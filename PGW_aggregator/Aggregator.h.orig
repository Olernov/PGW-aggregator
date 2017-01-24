#pragma once
#include <map>
#include <memory>
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"
#include "LockFreeQueueWithSize.h"

typedef std::multimap<unsigned long, Session_ptr> SessionMap;
//typedef LockFreeQueueWithSize<Session*> ExportQueue;
typedef LockFreeQueueFixedSize<Session*> ExportQueue;

class Aggregator
{
public:
	Aggregator(otl_connect&);
	void SetSessionMapsNum(int num);
	void ProcessCDR(const PGWRecord& pGWRecord);
	void PrintSessions();
    void ExportAllSessionsToDB(std::string filename);
	void EraseAllSessions();
	void CheckExportedData(AggregationTestType);
<<<<<<< HEAD
=======
	bool RunAllTests();
>>>>>>> 395dc8537034875dc9959803af5f032d74d9dcd3
private:
	static const int maxSessionMapsNum = 32;
	static const int defaultSessionMapsNum = 8;
    static const int exportQueueSize = 1000;
	int m_sessionMapsNum;
    SessionMap m_sessions[maxSessionMapsNum];
    ExportQueue m_exportQueue[maxSessionMapsNum];

    SessionMap& GetAppropriateMap(unsigned long long imsi);
    ExportQueue& GetExportQueue(Session_ptr sessionPtr);
    void CreateSessionsAndExport(const PGWRecord& pGWRecord, const DataVolumesMap& dataVolumes);
    SessionMap::iterator CreateSession(const PGWRecord& pGWRecord,
					   unsigned long ratingGroup, unsigned long volumeUplink, unsigned long volumeDownlink);
    void ExportSession(Session_ptr sessionPtr);
	otl_connect& m_dbConnect;
};
