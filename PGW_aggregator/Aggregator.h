#pragma once
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"
#include <map>

using namespace std;


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
	void PrintCDRContents(const PGWRecord& pGWRecord);
	void ProcessCDR(const PGWRecord& pGWRecord);
	void PrintSessions();
	void ExportAllSessionsToDB(string filename);
	bool RunAllTests();
private:
	static const int maxSessionMapsNum = 32;
	static const int defaultSessionMapsNum = 32;
	int m_sessionMapsNum;
	multimap<ChargingID_t, Session> m_sessions[maxSessionMapsNum];
	map<RatingGroupId_t, DataVolumes> SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord) const;
	bool SumDataVolumesByRatingGroup_Test();
	otl_connect& m_dbConnect;
};
