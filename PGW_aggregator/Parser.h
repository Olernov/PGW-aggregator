#pragma once
#include <string>
#include <thread>
#include "Aggregator.h"
#include "LockFreeQueueWithSize.h"
#include "Common.h"

class Parser
{
public:
    Parser();
    ~Parser();
    void ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension, std::string archiveDirectory);
    void SetStopFlag();
	void SetPrintContents(bool);
    //void ExportAllSessionsToDB(std::string filename);
private:
    const char* shutdownFlagFilename = "PGW_aggregator.stop";
    std::vector<Aggregator_ptr> aggregators;
    bool printFileContents;
    bool stopFlag;

    void ParseFile(std::string filename);
    void AggregateCDRsFromQueue();
    Aggregator& GetAppropiateAggregator(const GPRSRecord*);
    bool IsShutdownFlagSet();
};
