#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "ExportRules.h"
#include "Common.h"


extern ExportRules exportRules;

std::mutex mutex;

extern Config config;

Aggregator::Aggregator(int index) :
    sessionIndex(index),
    cdrQueue(cdrQueueSize),
    stopFlag(false),
    exportCount(0),
    lastIdleSessionsEject(0)
{
    thread = std::thread(&Aggregator::AggregateCDRsFromQueue, this);
}

Aggregator::~Aggregator()
{
    stopFlag = true;
    thread.join();
}


void Aggregator::AddCdrToQueue(const GPRSRecord *gprsRecord)
{
    while (!cdrQueue.push(const_cast<GPRSRecord*>(gprsRecord))) {
        std::cout << "Thread #" << sessionIndex << ": CDR queue is full. Sleeping 3 sec..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}


void Aggregator::AggregateCDRsFromQueue()
{
    dbConnect.rlogon(config.connectString.c_str());
    while (!(stopFlag && cdrQueue.empty())) {
        try {
            GPRSRecord* gprsRecord;
            if (cdrQueue.pop(gprsRecord)) {
                ProcessCDR(gprsRecord->choice.pGWRecord);
                ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
            }
            else {
                time_t now;
                if (Utils::DiffMinutes(time(&now), lastIdleSessionsEject) > idleSessionEjectPeriodMin) {
                    // TODO: start session map check procedures if needed
                    EjectIdleSessions();
                }
                else {
                    // nothing to do
                    std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenCdrQueueIsEmpty));
                }
            }
        }
        catch(const otl_exception& ex) {
            // TODO: add correct processing
            dbConnect.rollback();
            std::cout << "DB error: " << std::endl
                 << ex.msg << std::endl
                 << ex.stm_text << std::endl
                 << ex.var_info << std::endl;
        }
    }
    std::cout << "Thread #" << sessionIndex << " finished processing CDR queue. Export count: " << exportCount << std::endl;
    std::cout << "Thread #" << sessionIndex << ": exporting all sesions to DB..." << std::endl;
    try {
        ExportAllSessionsToDB();
    }
    catch(const otl_exception& ex) {
        // TODO: add correct processing
        dbConnect.rollback();
        std::cout << "DB error: " << std::endl
             << ex.msg << std::endl
             << ex.stm_text << std::endl
             << ex.var_info << std::endl;
    }
    std::cout << "Thread #" << sessionIndex << ": export count: " << exportCount << std::endl;
    dbConnect.commit();
    dbConnect.logoff();
}


void Aggregator::ProcessCDR(const PGWRecord& pGWRecord)
{
	// TODO: check all CDR record parameters existance and validity
	if (!pGWRecord.servedIMSI) {
        throw std::string("servedIMSI not present in CDR record, processing impossible");
	}
	if (!pGWRecord.servingNodePLMNIdentifier) {
        throw std::string("servingNodePLMNIdentifier not present in CDR record, processing impossible");
	}

    if (pGWRecord.listOfServiceData) { // process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
        DataVolumesMap dataVolumes = Utils::SumDataVolumesByRatingGroup(pGWRecord);
        auto eqRange = sessions.equal_range(pGWRecord.chargingID); // equal_range is used here because of multimap. In case of map we could use find function here
		if (eqRange.first == eqRange.second) {
			// not found
             CreateSessionsAndExport(pGWRecord, dataVolumes);
		}
		else {
			// one or more sessions having this Charging ID are found, try to find appropriate rating group
			for (auto sessionIter = eqRange.first; sessionIter != eqRange.second; ++sessionIter) {
                auto dataVolumeIter = dataVolumes.find(sessionIter->second.get()->GetRatingGroup());
				if (dataVolumeIter != dataVolumes.end()) {
					// session having same rating group found, update values
                    sessionIter->second.get()->UpdateData(dataVolumeIter->second.volumeUplink,
                                                      dataVolumeIter->second.volumeDownlink,
                                                      pGWRecord.duration,
                                                      Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime));
                    if (exportRules.IsReadyForExport(sessionIter->second)) {
                        ExportSession(sessionIter->second);
                        //sessions.erase(sessionIter);
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
            //sessions.erase(iter);
        }
    }
}


SessionMap::iterator Aggregator::CreateSession(const PGWRecord& pGWRecord, unsigned32 ratingGroup,
                               unsigned32 volumeUplink, unsigned32 volumeDownlink)
{
    //std::lock_guard<std::mutex> lock(mutex);
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
    //std::lock_guard<std::mutex> lock(mutex);
    sessionPtr.get()->ExportToDB(dbConnect);
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
    for (auto& it : sessions) {
        ExportSession(it.second);
    }
}


void Aggregator::EjectIdleSessions()
{
 // TODO
    std::cout << "Thread #" << sessionIndex << ": Ejecting idle sessions. Map size: " << sessions.size() << std::endl;
    time_t now;
    time(&now);
    for (auto& it : sessions) {
        if (Utils::DiffMinutes(it.second->GetLastUpdateTime(), now) > config.sessionIdlePeriod) {
            ExportSession(it.second);
            sessions.erase(it.first);
        }
    }
    std::cout << "Thread #" << sessionIndex << ": Idle sessions ejected. Map size: " << sessions.size() << std::endl;
}


void Aggregator::EraseAllSessions()
{
    sessions.clear();
}


void Aggregator::SetStopFlag()
{
    stopFlag = true;
}
