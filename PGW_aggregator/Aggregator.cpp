#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "Common.h"
#include "LogWriter.h"
#include "Config.h"


extern LogWriter logWriter;
//extern void Reconnect(otl_connect& dbConnect, short sessionIndex);

std::mutex mutex;

extern Config config;

Aggregator::Aggregator(int index, const std::string& connectString, ExportRules &er) :
    thisIndex(index),
    connectString(connectString),
    exportRules(er),
    cdrQueue(cdrQueueSize),
    stopFlag(false)
{
    time(&lastIdleSessionsEject);
    thread = std::thread(&Aggregator::AggregatorThreadFunc, this);
}


void Aggregator::AddCdrToQueue(const GPRSRecord *gprsRecord)
{
    bool queueIsFull = false;
    while (!cdrQueue.push(const_cast<GPRSRecord*>(gprsRecord))) {
        if (!queueIsFull) {
            logWriter.Write("AddCdrToQueue: CDR queue max size reached (" + std::to_string(cdrQueueSize) + ")",
                            thisIndex, debug);
            queueIsFull = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
    }
}


void Aggregator::AggregatorThreadFunc()
{
    try {
        dbConnect.rlogon(connectString.c_str());
    }
    catch(const otl_exception& ex) {
        SendAlertIfNeeded(Utils::OtlExceptionToText(ex));
        logWriter.Write("**** DB ERROR while logging to DB: **** " +
            crlf + exceptionText, thisIndex);
    }
    bool cdrFound = false;
    while (!(stopFlag && cdrQueue.empty())) {
        GPRSRecord* gprsRecord;
        if (cdrQueue.pop(gprsRecord)) {
            if (!cdrFound) {
                cdrFound = true;
            }
            ProcessCDR(gprsRecord->choice.pGWRecord);
            ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
        }
        else {
            if (cdrFound) {
                logWriter.Write("CDR queue has been processed. Sessions count: " + std::to_string(sessions.size()),
                                thisIndex);
                cdrFound = false;
            }
            if (Utils::DiffMinutes(time(nullptr), lastIdleSessionsEject) > config.sessionEjectPeriodMin) {
                EjectIdleSessions();
            }
            else {
                // nothing to do
                std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
            }
        }
    }
    logWriter.Write("Shutdown flag set.", thisIndex);
    ExportAllSessionsToDB();
    logWriter.Write("Thread finish", thisIndex);
    if (dbConnect.connected) {
        dbConnect.commit();
        dbConnect.logoff();
    }
}


void Aggregator::ProcessCDR(const PGWRecord& pGWRecord)
{
    DataVolumesMap dataVolumes = Utils::SumDataVolumesByRatingGroup(pGWRecord);
    auto eqRange = sessions.equal_range(pGWRecord.chargingID); // equal_range is used here because of multimap. In case of map we could use find function here
    if (eqRange.first == eqRange.second) {
        // not found
         CreateSessionsAndExport(pGWRecord, dataVolumes);
    }
    else {
        // one or more sessions having this Charging ID are found, try to find appropriate rating group
        for (auto sessionIter = eqRange.first; sessionIter != eqRange.second; ++sessionIter) {
            auto dataVolumeIter = dataVolumes.find(sessionIter->second.get()->ratingGroup);
            if (dataVolumeIter != dataVolumes.end()) {
                // session having same rating group found, update values
                sessionIter->second.get()->UpdateData(dataVolumeIter->second.volumeUplink,
                                                  dataVolumeIter->second.volumeDownlink,
                                                  pGWRecord.duration,
                                                  Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime));
                if (exportRules.IsReadyForExport(sessionIter->second)) {
                    ExportSession(sessionIter->second);
                }
                dataVolumes.erase(dataVolumeIter);
            }
        }
        CreateSessionsAndExport(pGWRecord, dataVolumes);
    }
}


void Aggregator::CreateSessionsAndExport(const PGWRecord& pGWRecord, const DataVolumesMap& dataVolumes)
{
    for (auto dataVolumeIter : dataVolumes) {
        auto iter = CreateSession(pGWRecord,
                      dataVolumeIter.first /* rating group */,
                      dataVolumeIter.second.volumeUplink,
                      dataVolumeIter.second.volumeDownlink);
        if (exportRules.IsReadyForExport(iter->second)) {
            ExportSession(iter->second);
        }
    }
}


SessionMap::iterator Aggregator::CreateSession(const PGWRecord& pGWRecord, unsigned32 ratingGroup,
                               unsigned32 volumeUplink, unsigned32 volumeDownlink)
{
    unsigned64 imsi = Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI);
    return sessions.insert(std::make_pair(pGWRecord.chargingID,
        Session_ptr(new Session(
           pGWRecord.chargingID,
           imsi,
		   Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN),
           Utils::TBCDString_to_String(pGWRecord.servedIMEISV),
           (pGWRecord.accessPointNameNI ? reinterpret_cast<const char*>(pGWRecord.accessPointNameNI->buf) : ""),
		   pGWRecord.duration,
		   Utils::IPAddress_to_ULong(pGWRecord.servingNodeAddress.list.array[0]),
		   Utils::PLMNID_to_ULong(pGWRecord.servingNodePLMNIdentifier),
		   ratingGroup,
		   volumeUplink,
		   volumeDownlink,
           Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime)))));
}


void Aggregator::ExportSession(Session_ptr sessionPtr)
{
    try {
        sessionPtr.get()->ExportToDB(dbConnect);
        ClearExceptionText();
    }
    catch(const otl_exception& ex) {
        exceptionText = Utils::OtlExceptionToText(ex);
        logWriter.Write("**** DB ERROR while exporting chargingID " +
            std::to_string(sessionPtr.get()->chargingID) + ": ****" + crlf + exceptionText, thisIndex);
        logWriter.Write(sessionPtr.get()->SessionDataDump(), thisIndex);
        dbConnect.reconnect();
        SendAlertIfNeeded(exceptionText);
    }
}


void Aggregator::ExportAllSessionsToDB()
{
    while (std::any_of(sessions.begin(), sessions.end(),
                       [](std::pair<unsigned32, Session_ptr> mp) { return mp.second.get()->HaveDataToExport(); })) {
        logWriter.Write("Exporting all sessions: " + std::to_string(sessions.size()), thisIndex);
        for (auto& it : sessions) {
            ExportSession(it.second);
        }
    }
    logWriter.Write("All sessions exported.", thisIndex);
}


void Aggregator::EjectIdleSessions()
{
    logWriter.Write("Start of ejecting idle sessions. Map size: " + std::to_string(sessions.size()), thisIndex);
    time_t now;
    time(&now);
    for (auto it = sessions.begin(); it != sessions.end(); it++) {
        if (Utils::DiffMinutes(it->second->lastUpdateTime, now) > config.sessionEjectPeriodMin) {
            ExportSession(it->second);
            if (!it->second->HaveDataToExport()) {
                sessions.erase(it);
            }
        }
    }
    time(&lastIdleSessionsEject);
    logWriter.Write("Finish of ejecting idle sessions. Map size: " + std::to_string(sessions.size()), thisIndex);
}


void Aggregator::SendAlertIfNeeded(const std::string& excText)
{
    if (exceptionText != lastExceptionText && dbConnect.connected) {
        otl_stream dbStream;
        try {
            dbStream.open(1, std::string("call BILLING.MOBILE_DATA_CHARGER.SendAlert(:mess/*char[" +
                           std::to_string(maxAlertMessageLen) + "]*/)").c_str(), dbConnect);
            dbStream << exceptionText.substr(0, maxAlertMessageLen-1);
            dbStream.close();
            lastExceptionText = exceptionText;
        }
        catch(const otl_exception& ex) {
        }
    }
}


void Aggregator::ClearExceptionText()
{
    exceptionText.clear();
}


std::string Aggregator::GetExceptionMessage() const
{
    if (!exceptionText.empty()) {
        return exceptionText;
    }
    if(!dbConnect.connected) {
        return "Not connected to DB";
    }
    return "";
}


void Aggregator::SetStopFlag()
{
    stopFlag = true;
}

Aggregator::~Aggregator()
{
    stopFlag = true;
    thread.join();
}
