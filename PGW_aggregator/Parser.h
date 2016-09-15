#pragma once
#include <string>
#include "Aggregator.h"

using namespace std;

class Parser
{
public:
	Parser(const Aggregator&);
	void ProcessDirectory(string cdrFilesDirectory, string cdrExtension, bool perFileAggregationTest);
	void RunPerFileAggregationTest(string sampleCdrDirectory, string cdrExtension);
	void SetPrintContents(bool);
private:
	Aggregator& m_aggregator;
	bool m_printFileContents;
	void ProcessCDRFile(string filename);
};
