#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "ExportRules.h"
#include "Common.h"
#include "LogWriter.h"
#include "Config.h"

extern ExportRules exportRules;
extern LogWriter logWriter;
extern void ReconnectToDB(otl_connect& dbConnect, short sessionIndex);

std::mutex mutex;

extern Config config;

Aggregator::Aggregator(int index) :
    sessionIndex(index),
    cdrQueue(cdrQueueSize),
    stopFlag(false),
    exceptionPtr(nullptr)
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
                            sessionIndex, debug);
            queueIsFull = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
    }
}


void Aggregator::AggregatorThreadFunc()
{
    ReconnectToDB(dbConnect, sessionIndex);
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
                                sessionIndex);
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
    logWriter.Write("Shutdown flag set.", sessionIndex);
    ExportAllSessionsToDB();
    logWriter.Write("Thread finish", sessionIndex);
    if (dbConnect.connected) {
        dbConnect.commit();
        dbConnect.logoff();
    }
}


//void Aggregator::ReconnectToDB()
//{
//    try {
//        dbConnect.logoff();
//    }
//    catch(const otl_exception& ex) {
//        // no reaction for possible exception
//    }

//    try {
//        dbConnect.rlogon(config.connectString.c_str());
//        //ClearExceptionPtr();
//        logWriter.Write("(Re)Connected successfully", sessionIndex);
//    }
//    catch(const otl_exception& ex) {
//        //LogOtlException(ex);
//        logWriter.LogOtlException("**** DB ERROR while connecting to DB: ****", ex, sessionIndex);
//        dbConnect.connected = false;
//        SetExceptionPtr();
//    }
//}


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
    if (!dbConnect.connected) {
        ReconnectToDB(dbConnect, sessionIndex);
    }
    if (dbConnect.connected) {
        try {
            sessionPtr.get()->ExportToDB(dbConnect);
            ClearExceptionPtr();
        }
        catch(const otl_exception& ex) {
            logWriter.LogOtlException("**** DB ERROR while exporting chargingID " +
                std::to_string(sessionPtr.get()->chargingID) + ": ****", ex, sessionIndex);
            logWriter.Write(sessionPtr.get()->SessionDataDump(), sessionIndex);
            dbConnect.connected = false;
            SetExceptionPtr();
            ReconnectToDB(dbConnect, sessionIndex);
        }
    }
}


void Aggregator::ExportAllSessionsToDB()
{
    while (std::any_of(sessions.begin(), sessions.end(),
                       [](std::pair<unsigned32, Session_ptr> mp) { return mp.second.get()->HaveDataToExport(); })) {
        logWriter.Write("Exporting all sessions: " + std::to_string(sessions.size()), sessionIndex);
        for (auto& it : sessions) {
            ExportSession(it.second);
        }
    }
    logWriter.Write("All sessions exported.", sessionIndex);
}


void Aggregator::EjectIdleSessions()
{
    logWriter.Write("Start of ejecting idle sessions. Map size: " + std::to_string(sessions.size()), sessionIndex);
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
    logWriter.Write("Finish of ejecting idle sessions. Map size: " + std::to_string(sessions.size()), sessionIndex);
}


void Aggregator::SetExceptionPtr()
{
    std::lock_guard<std::mutex> lock(setExceptionMutex);
    exceptionPtr = std::current_exception();
}


void Aggregator::ClearExceptionPtr()
{
    std::lock_guard<std::mutex> lock(setExceptionMutex);
    exceptionPtr = nullptr;
}


bool Aggregator::IsReady() const
{
    return dbConnect.connected && (exceptionPtr == nullptr);
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
