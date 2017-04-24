#pragma once
#include <string>
#include <set>
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


class AvroCdrCompare
{
public:
    bool operator() (const PGW_CDR& lhs, const PGW_CDR& rhs) const {
//        return (
//            lhs.IMSI < rhs.IMSI ||
//            lhs.MSISDN < rhs.MSISDN ||
//            ((lhs.IMEI.is_null() ? std::string("<null>") : lhs.IMEI.get_string()) <
//                (rhs.IMEI.is_null() ? std::string("<null>") : rhs.IMEI.get_string())) ||
//            lhs.ServedPDPAddress < rhs.ServedPDPAddress ||
//            lhs.FirstUsageTime < rhs.FirstUsageTime ||
//            lhs.RatingGroup < rhs.RatingGroup ||
//            lhs.VolumeUplink < rhs.VolumeUplink ||
//            lhs.VolumeDownlink < rhs.VolumeDownlink ||
//            lhs.ChargingID < rhs.ChargingID ||
//            ((lhs.SequenceNumber.is_null() ? -1 : lhs.SequenceNumber.get_int()) <
//                (rhs.SequenceNumber.is_null() ? -1 : rhs.SequenceNumber.get_int())) ||
//            lhs.TimeOfUsage < rhs.TimeOfUsage ||
//            lhs.UserLocationInfo < rhs.UserLocationInfo
//                    );
        if (lhs.IMSI < rhs.IMSI) return true;
        else if (lhs.IMSI > rhs.IMSI) return false;

        if (lhs.MSISDN < rhs.MSISDN) return true;
        else if (lhs.MSISDN > rhs.MSISDN) return false;

        if ((lhs.IMEI.is_null() ? std::string("<null>") : lhs.IMEI.get_string()) <
            (rhs.IMEI.is_null() ? std::string("<null>") : rhs.IMEI.get_string())) return true;
        else if ((lhs.IMEI.is_null() ? std::string("<null>") : lhs.IMEI.get_string()) >
                 (rhs.IMEI.is_null() ? std::string("<null>") : rhs.IMEI.get_string())) return false;

        if (lhs.ServedPDPAddress < rhs.ServedPDPAddress) return true;
        else if (lhs.ServedPDPAddress > rhs.ServedPDPAddress) return false;

        if (lhs.FirstUsageTime < rhs.FirstUsageTime) return true;
        else if (lhs.FirstUsageTime > rhs.FirstUsageTime) return false;

        if (lhs.RatingGroup < rhs.RatingGroup) return true;
        else if (lhs.RatingGroup > rhs.RatingGroup) return false;

        if (lhs.VolumeUplink < rhs.VolumeUplink) return true;
        else if (lhs.VolumeUplink > rhs.VolumeUplink) return false;

        if (lhs.VolumeDownlink < rhs.VolumeDownlink) return true;
        else if (lhs.VolumeDownlink > rhs.VolumeDownlink) return true;

        if (lhs.ChargingID < rhs.ChargingID) return true;
        else if (lhs.ChargingID > rhs.ChargingID) return false;

        if ((lhs.SequenceNumber.is_null() ? -1 : lhs.SequenceNumber.get_int()) <
               (rhs.SequenceNumber.is_null() ? -1 : rhs.SequenceNumber.get_int())) return true;
        else if ((lhs.SequenceNumber.is_null() ? -1 : lhs.SequenceNumber.get_int()) >
                 (rhs.SequenceNumber.is_null() ? -1 : rhs.SequenceNumber.get_int())) return false;

        if (lhs.TimeOfUsage < rhs.TimeOfUsage) return true;
        else if (lhs.TimeOfUsage > rhs.TimeOfUsage) return false;

        if (lhs.UserLocationInfo < rhs.UserLocationInfo) return true;
        else if (lhs.UserLocationInfo > rhs.UserLocationInfo) return false;

        return false;
    }
};


//class KafkaDeliveryReportCallback : public RdKafka::DeliveryReportCb
//{
// public:
//  void dr_cb (RdKafka::Message &message);
//};


class Parser
{
public:
    Parser(const std::string &kafkaBroker, const std::string &kafkaTopic, unsigned32 partition,
           const std::string &cdrFilesDirectory, const std::string &cdrExtension,
           const std::string &archiveDirectory, const std::string &cdrBadDirectory,
           bool runtest);
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
    unsigned32 kafkaPartition;
    std::string cdrArchiveDirectory;
    std::string cdrBadDirectory;
    std::string postponeReason;
    const int producerPollTimeoutMs = 3000;
    const int queueSizeThreshold = 20;
    std::string shutdownFilePath;

    bool printFileContents;
    bool runTest;
    bool stopFlag;
    std::string lastErrorMessage;
    time_t lastAlertTime;
    std::unique_ptr<RdKafka::Conf> kafkaGlobalConf;
    std::unique_ptr<RdKafka::Conf> kafkaTopicConf;
    std::unique_ptr<RdKafka::Producer> kafkaProducer;
    //KafkaDeliveryReportCallback deliveryReportCb;
    KafkaEventCallback eventCb;
    std::set<PGW_CDR, AvroCdrCompare> sentAvroCdrs;

    unsigned32 ParseFile(FILE *pgwFile, const std::string& filename);
    void WaitForKafkaQueue();
    int SendRecordToKafka(const PGWRecord& pGWRecord);
    bool CompareSentAndConsumedRecords(int64_t startOffset);
    void PrintAvroCdrContents(const PGW_CDR& cdr) const;
    void RunTests();
};


