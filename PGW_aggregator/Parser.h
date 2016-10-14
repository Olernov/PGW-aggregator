#pragma once
#include <string>
#include "Aggregator.h"
#include "LockFreeQueueWithSize.h"



class Parser
{
public:
	Parser(const Aggregator&);
    void ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension, AggregationTestType testType);
	void SetPrintContents(bool);
private:
    static const int cdrQueueSize = 50000;
    boost::lockfree::queue<GPRSRecord*, boost::lockfree::fixed_sized<true>> m_cdrQueue;
    //LockFreeQueueWithSize<GPRSRecord*> m_cdrQueue;
	Aggregator& m_aggregator;
	bool m_printFileContents;
    void ParseFile(std::string filename);
    void ProcessCDRQueue();
};
