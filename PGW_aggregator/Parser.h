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
        recordCount(0)
    {}
    unsigned64 volumeUplink;
    unsigned64 volumeDownlink;
    unsigned32 recordCount;
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
private:
    std::string cdrArchiveDirectory;
    std::string cdrBadDirectory;
    const std::string shutdownFlagFilename = "pgw-aggregator.stop";
    std::string shutdownFilePath;
    DBConnect dbConnect;
    std::vector<Aggregator_ptr> aggregators;
    ExportRules exportRules;

    bool printFileContents;
    bool stopFlag;
    std::string lastExceptionText;
    std::string postponeReason;


    std::string lastAlertMessage;
    time_t lastAlertTime;


    CdrFileTotals ParseFile(FILE *pgwFile, const std::string& filename);
    Aggregator& GetAppropiateAggregator(const GPRSRecord*);
    bool ChargingAllowed();
    void Accumulate(CdrFileTotals& totalVolumes, const PGWRecord& pGWRecord);
    void RegisterFileStats(const std::string& filename, CdrFileTotals totals);
    void AlertAggregatorExceptions();
};


