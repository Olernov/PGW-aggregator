#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "ExportRules.h"
#include "Common.h"


ExportRules exportRules;

Aggregator::Aggregator(otl_connect& dbConnect) :
	m_dbConnect(dbConnect),
    m_sessionMapsNum(defaultSessionMapsNum)
{

}


void Aggregator::SetSessionMapsNum(int num)
{
    if ((num < 1) || (num >= maxSessionMapsNum))
        throw std::string("Aggregator::SetSessionMapsNum: invalid num given");
	m_sessionMapsNum = num;
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
		auto eqRange = GetAppropriateMap(Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI)).equal_range(
					pGWRecord.chargingID); // equal_range is used here because of multimap. In case of map we could use find function here
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


SessionMap::iterator Aggregator::CreateSession(const PGWRecord& pGWRecord, unsigned long ratingGroup,
							   unsigned long volumeUplink, unsigned long volumeDownlink)
{
	unsigned long long imsi = Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI);
    return GetAppropriateMap(imsi).insert(std::make_pair(pGWRecord.chargingID,
        std::shared_ptr<Session> (new Session(
           pGWRecord.chargingID,
           imsi,
		   Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN),
		   Utils::TBCDString_to_ULongLong(pGWRecord.servedIMEISV),
		   (pGWRecord.accessPointNameNI ? (const char*) pGWRecord.accessPointNameNI->buf : ""),
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
    if (! GetExportQueue(sessionPtr).push(sessionPtr.get())) {
        // TODO: normal situation, add correct processing
        throw std::string("ExportQueue is full");
    }
}


SessionMap& Aggregator::GetAppropriateMap(unsigned long long imsi)
{
    return m_sessions[imsi % m_sessionMapsNum];
}


ExportQueue& Aggregator::GetExportQueue(Session_ptr sessionPtr)
{
    return m_exportQueue[sessionPtr.get()->GetIMSI() % m_sessionMapsNum];
}


void Aggregator::PrintSessions()
{
	for (int i = 0; i < m_sessionMapsNum; i++)
		for (auto& it : m_sessions[i]) {
            it.second.get()->PrintSessionData(std::cout);
		}
}


void Aggregator::ExportAllSessionsToDB(std::string filename)
{
	for (int i = 0; i < m_sessionMapsNum; i++) {
		for (auto& it : m_sessions[i]) {
            it.second.get()->ExportToTestTable(m_dbConnect, filename);
		}
	}
}


void Aggregator::EraseAllSessions()
{
	for(int i = 0; i < m_sessionMapsNum; i++)
		m_sessions[i].clear();
}


void Aggregator::CheckExportedData(AggregationTestType testType)
{
	otl_stream otlStream;
    otlStream.open(1, "call CHECK_TEST_EXPORT(:testType /*long,in*/)", m_dbConnect);
	otlStream << static_cast<long> (testType);    
	otlStream.close();
}



