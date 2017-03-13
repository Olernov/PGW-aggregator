#pragma once
#include <string>
#include <thread>
#include <boost/filesystem.hpp>
#include "Aggregator.h"
#include "ExportRules.h"
#include "Common.h"

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



class Parser
{
public:
    Parser(const std::string &connectString, const std::string &cdrFilesDirectory, const std::string &cdrExtension,
           const std::string &archiveDirectory, const std::string &cdrBadDirectory);
    void ProcessFile(const filesystem::path& file);
    void RefreshExportRulesIfNeeded();
    void SetStopFlag();
	void SetPrintContents(bool);
    bool IsReady();
    const std::string& GetPostponeReason() const { return postponeReason; }
    bool SendMissingCdrAlert(double diffMinutes);
private:
    const std::string shutdownFlagFilename = "pgw-aggregator.stop";
    std::string cdrArchiveDirectory;
    std::string cdrBadDirectory;
    std::string shutdownFilePath;
    DBConnect dbConnect;
    std::vector<Aggregator_ptr> aggregators;
    ExportRules exportRules;
    std::string lastExceptionText;
    std::string postponeReason;
    std::string lastAlertMessage;
    bool printFileContents;
    bool stopFlag;
    time_t lastAlertTime;

    CdrFileTotals ParseFile(FILE *pgwFile, const std::string& filename);
    Aggregator& GetAppropiateAggregator(const GPRSRecord*);
    bool ChargingAllowed();
    void AccumulateStats(CdrFileTotals& totalVolumes, const PGWRecord& pGWRecord);
    void RegisterFileStats(const std::string& filename, CdrFileTotals totals, long processTimeSec, time_t fileTime);
    void AlertAggregatorExceptions();
};


