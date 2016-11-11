#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "ExportRules.h"
#include "Common.h"


ExportRules exportRules;

extern Config config;

Aggregator::Aggregator() :
    cdrQueue(cdrQueueSize),
    stopFlag(false)
{
    //otl_connect::otl_initialize();
    dbConnect.rlogon(config.connectString.c_str());
    thread = std::thread(&Aggregator::AggregateCDRsFromQueue, this);
}

Aggregator::~Aggregator()
{
    stopFlag = true;
    thread.join();
    ExportAllSessionsToDB();
    dbConnect.commit();
    dbConnect.logoff();
}


void Aggregator::AddCdrToQueue(const GPRSRecord *gprsRecord)
{
    if (!cdrQueue.push(const_cast<GPRSRecord*>(gprsRecord))) {
        // TODO: for fixed size queue it's normal, just means that the queue is full
        // Process it correctly (sleep or something)

        //throw std::runtime_error("Unable to push CDR to queue");
        std::cout << "CDR queue is full. Sleeping 3 sec..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}


void Aggregator::AggregateCDRsFromQueue()
{
    while (!(stopFlag && cdrQueue.empty())) {
        try {
            GPRSRecord* gprsRecord;
            if (cdrQueue.pop(gprsRecord)) {
                ProcessCDR(gprsRecord->choice.pGWRecord);
                ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
            }
            else {
                std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenCdrQueueIsEmpty));
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
        catch(const std::exception& ex) {

        }
    }
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

	if (pGWRecord.listOfServiceData) {
		// process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
        DataVolumesMap dataVolumes = Utils::SumDataVolumesByRatingGroup(pGWRecord);
		// try to find sessions having this Charging ID in appropriate map (by hash of IMSI)
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
                                                      pGWRecord.duration);
                    if (exportRules.ReadyForExport(sessionIter->second)) {
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
        if (exportRules.ReadyForExport(iter->second)) {
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
    sessionPtr.get()->ExportToDB(dbConnect);
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
        it.second.get()->ExportToDB(dbConnect);
    }
}


void Aggregator::EraseAllSessions()
{
    sessions.clear();
}


void Aggregator::SetStopFlag()
{
    stopFlag = true;
}
