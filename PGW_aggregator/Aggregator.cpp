#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "ExportRules.h"
#include "Common.h"
#include "LogWriter.h"
#include "Config.h"

extern ExportRules exportRules;
extern LogWriter logWriter;

std::mutex mutex;

extern Config config;

Aggregator::Aggregator(int index) :
    sessionIndex(index),
    cdrQueue(cdrQueueSize),
    stopFlag(false),
    exportCount(0),
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
            logWriter.Write("AddCdrToQueue failure: CDR queue max size reached (" + std::to_string(cdrQueueSize) + ")",
                            sessionIndex, debug);
            queueIsFull = true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
    }
    if (queueIsFull) {
        logWriter.Write("AddCdrToQueue success", sessionIndex, debug);
    }
}


void Aggregator::AggregatorThreadFunc()
{
    ReconnectToDB();
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
    logWriter.Write("Shutdown flag set. Exporting all remaining sessions: " + std::to_string(sessions.size()), sessionIndex);
    ExportAllSessionsToDB();
    logWriter.Write("All sessions exported. Thread finish", sessionIndex);
    if (dbConnect.connected) {
        dbConnect.commit();
        dbConnect.logoff();
    }
}


void Aggregator::ReconnectToDB()
{
    try {
        if (dbConnect.connected) {
            dbConnect.logoff();
        }
    }
    catch(const otl_exception& ex) {
    }

    try {
        dbConnect.rlogon(config.connectString.c_str());
        ClearExceptionPtr();
        lastExceptionText.clear();
        logWriter.Write("Connected successfully", sessionIndex);
    }
    catch(const otl_exception& ex) {
        if (lastExceptionText != reinterpret_cast<const char*>(ex.msg)) {
            logWriter.Write("**** DB ERROR while logging in ****", sessionIndex);
            logWriter.Write(reinterpret_cast<const char*>(ex.msg), sessionIndex);
            logWriter.Write(reinterpret_cast<const char*>(ex.stm_text), sessionIndex);
            logWriter.Write(reinterpret_cast<const char*>(ex.var_info), sessionIndex);
            lastExceptionText = reinterpret_cast<const char*>(ex.msg);
        }
        dbConnect.connected = false;
        SetExceptionPtr();
    }
}


void Aggregator::ProcessCDR(const PGWRecord& pGWRecord)
{
    if (pGWRecord.servedIMSI && pGWRecord.listOfServiceData) { // process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
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
        //logWriter.Write("Not connected to DB, trying to connect ...", sessionIndex, debug);
        ReconnectToDB();
    }
    if (dbConnect.connected) {
        try {
            sessionPtr.get()->ExportToDB(dbConnect);
            ClearExceptionPtr();
            lastExceptionText.clear();
        }
        catch(const otl_exception& ex) {
            if (lastExceptionText != reinterpret_cast<const char*>(ex.msg)) {
                logWriter.Write("**** DB ERROR while exporting session ****", sessionIndex);
                logWriter.Write(reinterpret_cast<const char*>(ex.msg), sessionIndex);
                logWriter.Write(reinterpret_cast<const char*>(ex.stm_text), sessionIndex);
                logWriter.Write(reinterpret_cast<const char*>(ex.var_info), sessionIndex);
                lastExceptionText = reinterpret_cast<const char*>(ex.msg);
            }
            dbConnect.connected = false;
            SetExceptionPtr();
            ReconnectToDB();
        }
    }
    exportCount++;
}


void Aggregator::PrintSessions()
{
    for (auto& it : sessions) {
        it.second.get()->PrintSessionData(std::cout);
    }
}


void Aggregator::ExportAllSessionsToDB()
{
    for (auto it = sessions.begin(); it != sessions.end(); it++) {
        ExportSession(it->second);
    }
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


std::exception_ptr Aggregator::PopException()
{
    std::lock_guard<std::mutex> lock(setExceptionMutex);
    std::exception_ptr ex = exceptionPtr;
    //exceptionPtr = nullptr;
    return ex;
}

bool Aggregator::IsReady()
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
