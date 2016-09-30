#include <iostream>
#include "Aggregator.h"
#include "Utils.h"
#include "Common.h"


Aggregator::Aggregator(otl_connect& dbConnect) :
	m_dbConnect(dbConnect),
	m_sessionMapsNum(defaultSessionMapsNum)
{}


void Aggregator::SetSessionMapsNum(int num)
{
	if (num < 1 || num >= maxSessionMapsNum)
		throw string("Aggregator::SetSessionMapsNum: invalid num given");
	m_sessionMapsNum = num;
}


void Aggregator::PrintCDRContents(const PGWRecord& pGWRecord)
{
	cout << "chargingID: " << pGWRecord.chargingID << endl;
	cout << "IMSI: " << Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI) << endl;
	if (pGWRecord.servedMSISDN)
		cout << "MSISDN: " << Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN) << endl;
	cout << "IMEI: " << Utils::TBCDString_to_ULongLong(pGWRecord.servedIMEISV) << endl;
	if (pGWRecord.accessPointNameNI)
		cout << "APN: " << pGWRecord.accessPointNameNI->buf << endl;
	time_t recOpeningTime = Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime);
	char* recOpeningTimeStr = ctime(&recOpeningTime);
	cout << "recOpeningTime: " << recOpeningTimeStr << endl;
	cout << "Duration: " << pGWRecord.duration << endl;
	cout << "servingNodeAddress: " << Utils::BinIPAddress_to_Text(
				Utils::IPAddress_to_ULong(pGWRecord.servingNodeAddress.list.array[0])) << endl;
	if (pGWRecord.servingNodePLMNIdentifier)
		cout << "PLMN-ID: " << Utils::PLMNID_to_ULong(pGWRecord.servingNodePLMNIdentifier) << endl;
	if (pGWRecord.listOfServiceData) {
		for(int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
			cout << "   rating group: " << pGWRecord.listOfServiceData->list.array[i]->ratingGroup << endl;
			cout << "   volume uplink: " << pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink << endl;
			cout << "   volume downlink: " << pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink << endl;
		}
	}
	else {
		cout << "   no service data present" << endl;
	}
}

map<RatingGroupId_t, DataVolumes> Aggregator::SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord) const
{
	map<RatingGroupId_t, DataVolumes> dataVolumes;
	for(int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
		auto it = dataVolumes.find(pGWRecord.listOfServiceData->list.array[i]->ratingGroup);
		if (it != dataVolumes.end()) {
			if (pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink)
				it->second.volumeUplink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink;
			if (pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink)
				it->second.volumeDownlink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink;
		}
		else {
			dataVolumes.insert(pair<RatingGroupId_t, DataVolumes> (pGWRecord.listOfServiceData->list.array[i]->ratingGroup,
				DataVolumes(
					(pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink ?
						*pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink : 0),
					(pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink ?
						*pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink : 0))
				));
		}
	}
	return dataVolumes;
}


bool Aggregator::SumDataVolumesByRatingGroup_Test()
{
	bool success = true;
	PGWRecord rec;
//	struct listOfServiceData {
//		A_SEQUENCE_OF(struct ChangeOfServiceCondition) list;

//		/* Context for parsing across buffer boundaries */
//		asn_struct_ctx_t _asn_ctx;
//	};
	//PGWRecord::listOfServiceData* p;
	void* ptr = &rec.listOfServiceData;
	ptr = calloc(1, sizeof(PGWRecord::listOfServiceData));
//	rec.recordExtensions = (PGWRecord_t::recordExtensions*) calloc(1, sizeof(PGWRecord::recordExtensions));

//	ChangeOfServiceCondition changeOfService;
//	ASN_SEQUENCE_ADD(rec.listOfServiceData, &changeOfService);
	return success;
}


void Aggregator::ProcessCDR(const PGWRecord& pGWRecord)
{
//	PrintCDRContents(pGWRecord);

	// TODO: check all CDR record parameters existance and validity
	if (!pGWRecord.servedIMSI) {
		throw string("servedIMSI not present in CDR record, processing impossible");
	}
	if (!pGWRecord.servingNodePLMNIdentifier) {
		throw string("servingNodePLMNIdentifier not present in CDR record, processing impossible");
	}

	if (pGWRecord.listOfServiceData) {
		// process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
		map<RatingGroupId_t, DataVolumes> dataVolumes = SumDataVolumesByRatingGroup(pGWRecord);
		// try to find sessions having this Charging ID in appropriate map (by hash of IMSI)
		auto eqRange = GetAppropriateMap(Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI)).equal_range(
					pGWRecord.chargingID); // equal_range is used here because of multimap. In case of map we could use find function here
		if (eqRange.first == eqRange.second) {
			// not found
			// create new session for every rating group in CDR record
			for (auto dataVolumeIter : dataVolumes) {
				CreateSession(pGWRecord,
							  dataVolumeIter.first /* rating group */,
							  dataVolumeIter.second.volumeUplink, dataVolumeIter.second.volumeDownlink);
//				m_sessions[imsi % m_sessionMapsNum].insert(pair<ChargingID_t, Session> (pGWRecord.chargingID,
//					Session(imsi,
//					   Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN),
//					   Utils::TBCDString_to_ULongLong(pGWRecord.servedIMEISV),
//					   (pGWRecord.accessPointNameNI ? (const char*) pGWRecord.accessPointNameNI->buf : ""),
//					   pGWRecord.duration,
//					   Utils::IPAddress_to_ULong(pGWRecord.servingNodeAddress.list.array[0]),
//					   Utils::PLMNID_to_ULong(pGWRecord.servingNodePLMNIdentifier),
//					   dataVolumeIter.first, // rating group
//					   dataVolumeIter.second.volumeUplink,
//					   dataVolumeIter.second.volumeDownlink,
//					   Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime))));
			}
		}
		else {
			// one or more sessions having this Charging ID are found, try to find appropriate rating group
			for (auto sessionIter = eqRange.first; sessionIter != eqRange.second; ++sessionIter) {
				auto dataVolumeIter = dataVolumes.find(sessionIter->second.m_ratingGroup);
				if (dataVolumeIter != dataVolumes.end()) {
					// session having same rating group found, update values
					sessionIter->second.m_dataVolumeUplink += dataVolumeIter->second.volumeUplink;
					sessionIter->second.m_dataVolumeDownlink += dataVolumeIter->second.volumeDownlink;
					sessionIter->second.m_duration += pGWRecord.duration;
					dataVolumes.erase(dataVolumeIter);
				}
			}
			// create new sessions for all data volumes left in map after updating (i.e. new rating groups)
			for (auto dataVolumeIter : dataVolumes) {
				CreateSession(pGWRecord,
							  dataVolumeIter.first /* rating group */,
							  dataVolumeIter.second.volumeUplink, dataVolumeIter.second.volumeDownlink);
			}
		}
	}
}


std::multimap<ChargingID_t, Session>& Aggregator::GetAppropriateMap(unsigned long long imsi)
{
	return m_sessions[imsi % m_sessionMapsNum];
}


void Aggregator::CreateSession(const PGWRecord& pGWRecord, std::multimap<ChargingID_t, Session>::iterator insertPos,
							   unsigned long ratingGroup, unsigned long volumeUplink, unsigned long volumeDownlink)
{
	unsigned long long imsi = Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI);
	m_sessions[imsi % m_sessionMapsNum].insert(/*insertPos,*/ pair<ChargingID_t, Session> (pGWRecord.chargingID,
		Session(imsi,
		   Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN),
		   Utils::TBCDString_to_ULongLong(pGWRecord.servedIMEISV),
		   (pGWRecord.accessPointNameNI ? (const char*) pGWRecord.accessPointNameNI->buf : ""),
		   pGWRecord.duration,
		   Utils::IPAddress_to_ULong(pGWRecord.servingNodeAddress.list.array[0]),
		   Utils::PLMNID_to_ULong(pGWRecord.servingNodePLMNIdentifier),
		   ratingGroup,
		   volumeUplink,
		   volumeDownlink,
		   Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime))));
}


void Aggregator::CreateSession(const PGWRecord& pGWRecord,
							   unsigned long ratingGroup, unsigned long volumeUplink, unsigned long volumeDownlink)
{
	CreateSession(pGWRecord, GetAppropriateMap(Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI)).begin(),
				   ratingGroup, volumeUplink, volumeDownlink);
}


void Aggregator::PrintSessions()
{
	for (int i = 0; i < m_sessionMapsNum; i++)
		for (auto it : m_sessions[i]) {
			cout << "------------------------" << endl;
			cout << "chargingID: " << it.first << endl;
			cout << "IMSI: " << it.second.m_IMSI << endl;
			cout << "MSISDN: " << it.second.m_MSISDN << endl;
			cout << "IMEI: " << it.second.m_IMEI << endl;
			cout << "APN: " << it.second.m_accessPointName << endl;
			cout << "recOpeningTime: " << ctime(&it.second.m_firstCDRTime) << endl;
			cout << "Duration: " << it.second.m_duration << endl;
			cout << "servingNodeAddress: " << it.second.m_servingNodeIP << endl;
			cout << "PLMN-ID: " << it.second.m_servingNodePLMNID << endl;
			cout << "rating group: " << it.second.m_ratingGroup << endl;
			cout << "volume uplink: " << it.second.m_dataVolumeUplink << endl;
			cout << "volume downlink: " << it.second.m_dataVolumeDownlink << endl;
		}
}


void Aggregator::ExportAllSessionsToDB(string filename)
{
	otl_nocommit_stream dbStream;
	if (!filename.empty()) {
		dbStream.open(1,
			"delete from TEST_SESSION_EXPORT where filename = :filename /*char[30]*/", m_dbConnect);
		dbStream << filename;
	}
	else {
		dbStream.open(1,
			"delete from TEST_SESSION_EXPORT", m_dbConnect);
	}
	dbStream.close();
	for (int i = 0; i < m_sessionMapsNum; i++) {
		for (auto it : m_sessions[i]) {
			dbStream.open(1,
				"insert into TEST_SESSION_EXPORT (filename , charging_id, imsi, msisdn, "
				"imei, access_point_name, duration, serving_node_ip, serving_node_plmnid, rating_group, "
				"data_volume_uplink, data_volume_downlink, first_cdr_time) values ("
				":filename /*char[30]*/, :charging_id /*bigint*/, :imsi /*bigint*/, :msisdn /*bigint*/, "
				":imei /*bigint*/, :access_point_name /*char[30]*/, :duration /*bigint*/, "
				":serving_node_ip /*bigint*/, :serving_node_plmnid /*bigint*/, "
				":rating_group /*bigint*/, :data_volume_uplink /*bigint*/,"
				":data_volume_downlink /*bigint*/, to_date(:first_cdr_time /*char[20]*/, 'yyyymmddhh24miss'))",
				m_dbConnect);
			// WARNING: OTL library does not support unsigned long and unsigned long long datatypes
			// for Oracle versions lower than ORA_R11_G2, so we cast to long long type.
			// Long long (64-bit) type has range from -9 223 372 036 854 775 808 to 9 223 372 036 854 775 807,
			// (19 digits) so this should be enough to store IMSI and IMEI.
			dbStream
					<< filename
					<< (long long) it.first // chargingID
					<< (long long) it.second.m_IMSI
					<< (long long) it.second.m_MSISDN
					<< (long long) it.second.m_IMEI
					<< it.second.m_accessPointName
					<< (long long) it.second.m_duration
					<< (long long) it.second.m_servingNodeIP
					<< (long long) it.second.m_servingNodePLMNID
					<< (long long) it.second.m_ratingGroup
					<< (long long) it.second.m_dataVolumeUplink
					<< (long long) it.second.m_dataVolumeDownlink
					<< Utils::Time_t_to_String(it.second.m_firstCDRTime);
			dbStream.close();
		}
	}
	m_dbConnect.commit();
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


bool Aggregator::RunAllTests()
{
	assert(SumDataVolumesByRatingGroup_Test());
}
