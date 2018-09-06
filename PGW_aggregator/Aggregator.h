#pragma once
#include <map>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>
#include "Common.h"
#include "OTL_Header.h"
#include "GPRSRecord.h"
#include "PGWRecord.h"
#include "Session.h"
#include "ExportRules.h"

typedef std::unordered_multimap<unsigned32, Session_ptr> SessionMap;


class Aggregator
{
public:
    Aggregator(int index, const std::string &connectString, ExportRules& er);
    ~Aggregator();
    void AddCdrToQueue(GPRSRecord* gprsRecord);
    void AggregatorThreadFunc();
    void ProcessCDR(const PGWRecord &pGWRecord);
    std::string GetExceptionMessage() const;
    void CheckExportedData(AggregationTestType);
    void SetStopFlag();
    void WakeUp();
    void ProcessCDRQueue();
private:
    static const int cdrQueueSize = 500;
    static const int mapSizeReportPeriodMin = 5;

    int thisIndex;
    boost::lockfree::queue<GPRSRecord*, boost::lockfree::fixed_sized<true>> cdrQueue;
    std::mutex mutex;
    std::condition_variable conditionVar;
    SessionMap sessions;
    std::thread thread;
    bool stopFlag;
    std::string connectString;
    DBConnect dbConnect;
    std::string exceptionText;
    std::string lastExceptionText;
    ExportRules& exportRules;
    time_t lastIdleSessionsEject;
    time_t lastMapSizeReport;

    void CreateSessions(const PGWRecord& pGWRecord, const DataVolumesMap& dataVolumes);
    SessionMap::iterator CreateSession(const PGWRecord& pGWRecord, unsigned32 ratingGroup,
        time_t sessionStartTime, unsigned32 duration, unsigned32 volumeUplink, unsigned32 volumeDownlink);
    void ExportAllSessionsToDB();
    bool EjectOneIdleSession();
    void MapSizeReportIfNeeded();
    void SendAlertIfNeeded(const std::string&);
};

typedef std::shared_ptr<Aggregator> Aggregator_ptr;
