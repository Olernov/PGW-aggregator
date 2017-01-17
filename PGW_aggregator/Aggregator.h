#pragma once
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"
#include "LockFreeQueueWithSize.h"

typedef std::multimap<unsigned32, Session_ptr> SessionMap;


class Aggregator
{
public:
    Aggregator(int index);
    ~Aggregator();
    void AddCdrToQueue(const GPRSRecord* gprsRecord);
    void AggregateCDRsFromQueue();
    void ProcessCDR(const PGWRecord &pGWRecord);
	void PrintSessions();
    void EraseAllSessions();
	void CheckExportedData(AggregationTestType);
    void SetStopFlag();
private:
    static const int cdrQueueSize = 5000;
    const int secondsToSleepWhenCdrQueueIsEmpty = 3;

    int sessionIndex;
    unsigned long exportCount;
    boost::lockfree::queue<GPRSRecord*, boost::lockfree::fixed_sized<true>> cdrQueue;
    SessionMap sessions;
    std::thread thread;
    bool stopFlag;
    otl_connect dbConnect;

    void CreateSessionsAndExport(const PGWRecord& pGWRecord, const DataVolumesMap& dataVolumes);
    SessionMap::iterator CreateSession(const PGWRecord& pGWRecord,
                       unsigned32 ratingGroup, unsigned32 volumeUplink, unsigned32 volumeDownlink);
    void ExportSession(Session_ptr sessionPtr);
    void ExportAllSessionsToDB();
};

typedef std::shared_ptr<Aggregator> Aggregator_ptr;
