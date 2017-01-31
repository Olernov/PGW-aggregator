#pragma once
#include <string>
#include <thread>
#include <boost/filesystem.hpp>
#include "Aggregator.h"
//#include "LockFreeQueueWithSize.h"
#include "Common.h"

using namespace boost;

class parse_error : public std::logic_error {};

class Parser
{
public:
    Parser(const std::string &cdrFilesDirectory, const std::string &cdrExtension,
           const std::string &archiveDirectory, const std::string &cdrBadDirectory);
    ~Parser();
    void ProcessCdrFiles();
    void SetStopFlag();
	void SetPrintContents(bool);
    //void ExportAllSessionsToDB(std::string filename);
private:
    std::string cdrFilesDirectory; 
    std::string cdrExtension;
    std::string cdrArchiveDirectory;
    std::string cdrBadDirectory;
    const char* shutdownFlagFilename = "pgw-aggregator.stop";
    std::vector<Aggregator_ptr> aggregators;
    bool printFileContents;
    bool stopFlag;
    std::string lastExceptionText;

    void ProcessFile(const filesystem::path& file, const std::string& cdrArchiveDirectory,
                     const std::string &cdrBadDirectory);
    void ParseFile(FILE *pgwFile, const std::string& filename);
    void AggregateCDRsFromQueue();
    Aggregator& GetAppropiateAggregator(const GPRSRecord*);
    bool IsShutdownFlagSet();
    void Sleep();
};
