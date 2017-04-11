#pragma once
#include <string>
#include <thread>
#include <boost/filesystem.hpp>
#include "Common.h"
#include "GPRSRecord.h"
#include "pgw_cdr_avro.hh"
#include "rdkafkacpp.h"

using namespace boost;

struct CdrFileTotals
{
    CdrFileTotals() :
        volumeUplink(0),
        volumeDownlink(0),
        recordCount(0),
        earliestTime(notInitialized),
        latestTime(notInitialized)
    {}
    unsigned64 volumeUplink;
    unsigned64 volumeDownlink;
    unsigned32 recordCount;
    time_t earliestTime;
    time_t latestTime;
};

class parse_error : public std::logic_error {};


class KafkaEventCallback : public RdKafka::EventCb
{
public:
    KafkaEventCallback();
    void event_cb (RdKafka::Event &event);
private:
    bool allBrokersDown;
};


class KafkaDeliveryReportCallback : public RdKafka::DeliveryReportCb
{
 public:
  void dr_cb (RdKafka::Message &message);
};


class Parser
{
public:
    Parser(const std::string &kafkaBroker, const std::string &kafkaTopic, const std::string &cdrFilesDirectory, const std::string &cdrExtension,
           const std::string &archiveDirectory, const std::string &cdrBadDirectory);
    ~Parser();
    void ProcessFile(const filesystem::path& file);
    void SetStopFlag() { stopFlag = true; }
	void SetPrintContents(bool);
    bool IsReady();
    const std::string& GetPostponeReason() const { return postponeReason; }
    bool SendMissingCdrAlert(double diffMinutes);

private:
    std::string kafkaBroker;
    std::string kafkaTopic;
    std::string cdrArchiveDirectory;
    std::string cdrBadDirectory;
    std::string postponeReason;
    const int producerPollTimeoutMs = 1000;
    const int queueSizeThreshold = 20;
    std::string shutdownFilePath;

    bool printFileContents;
    bool stopFlag;
    std::string lastErrorMessage;
    time_t lastAlertTime;
    RdKafka::Conf *kafkaGlobalConf;
    RdKafka::Conf *kafkaTopicConf;
    RdKafka::Producer *kafkaProducer;
    KafkaDeliveryReportCallback deliveryReportCb;
    KafkaEventCallback eventCb;

    void ParseFile(FILE *pgwFile, const std::string& filename);
    void WaitForKafkaQueue();
    void SendRecordToKafka(const PGWRecord& pGWRecord);
};


