#pragma once
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <boost/lockfree/queue.hpp>
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"

typedef std::multimap<unsigned32, Session_ptr> SessionMap;


class Aggregator
{
public:
    Aggregator(int index);
    ~Aggregator();
    void AddCdrToQueue(const GPRSRecord* gprsRecord);
    void AggregatorThreadFunc();
    void ProcessCDR(const PGWRecord &pGWRecord);
    bool IsReady();
	void PrintSessions();
    void CheckExportedData(AggregationTestType);
    void SetStopFlag();
    std::exception_ptr PopException();
private:
    static const int cdrQueueSize = 100;

    int sessionIndex;
    unsigned long exportCount;
    boost::lockfree::queue<GPRSRecord*, boost::lockfree::fixed_sized<true>> cdrQueue;
    SessionMap sessions;
    std::thread thread;
    bool stopFlag;
    otl_connect dbConnect;
    std::exception_ptr exceptionPtr;
    std::string lastExceptionText;
    std::atomic<bool> refreshInProgress;

    time_t lastIdleSessionsEject;
    std::mutex setExceptionMutex;

    void ReconnectToDB();
    void CreateSessionsAndExport(const PGWRecord& pGWRecord, const DataVolumesMap& dataVolumes);
    SessionMap::iterator CreateSession(const PGWRecord& pGWRecord,
                       unsigned32 ratingGroup, unsigned32 volumeUplink, unsigned32 volumeDownlink);
    void ExportSession(Session_ptr sessionPtr);
    void ExportAllSessionsToDB();
    void EjectIdleSessions();
    void SetExceptionPtr();
    void ClearExceptionPtr();
};

typedef std::shared_ptr<Aggregator> Aggregator_ptr;
