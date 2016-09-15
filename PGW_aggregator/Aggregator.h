#pragma once
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"
#include <map>

using std::string;


struct DataVolumes {
	DataVolumes (unsigned long uplink, unsigned long downlink) : volumeUplink(uplink), volumeDownlink(downlink) {}
	unsigned long volumeUplink;
	unsigned long volumeDownlink;
};

class Aggregator
{
public:
	Aggregator(otl_connect&);
	void SetSessionMapsNum(int num);
	void ProcessCDR(const PGWRecord& pGWRecord);
	void PrintCDRContents(const PGWRecord& pGWRecord);
	void PrintSessions();
	void ExportAllSessionsToDB(string filename);
	void EraseAllSessions();
	bool RunAllTests();
private:
	static const int maxSessionMapsNum = 32;
	static const int defaultSessionMapsNum = 8;
	int m_sessionMapsNum;
	std::multimap<ChargingID_t, Session> m_sessions[maxSessionMapsNum];
	std::multimap<ChargingID_t, Session>& GetAppropriateMap(unsigned long long imsi);
	std::map<RatingGroupId_t, DataVolumes> SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord) const;
	bool SumDataVolumesByRatingGroup_Test();

	void CreateSession(const PGWRecord& pGWRecord, std::multimap<ChargingID_t, Session>::iterator insertPos,
					   unsigned long ratingGroup, unsigned long volumeUplink, unsigned long volumeDownlink);
	void CreateSession(const PGWRecord& pGWRecord,
					   unsigned long ratingGroup, unsigned long volumeUplink, unsigned long volumeDownlink);
	otl_connect& m_dbConnect;
};
