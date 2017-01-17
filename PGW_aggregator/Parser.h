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
    void ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension);
    void SetStopFlag();
	void SetPrintContents(bool);
    //void ExportAllSessionsToDB(std::string filename);
private:
    std::vector<Aggregator_ptr> aggregators;
    bool printFileContents;
    bool stopFlag;

    void ParseFile(std::string filename);
    void AggregateCDRsFromQueue();
    Aggregator& GetAppropiateAggregator(const GPRSRecord*);
};
