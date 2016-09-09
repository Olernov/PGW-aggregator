#pragma once
#include <string>
#include "Aggregator.h"

using namespace std;

class Parser
{
public:
	Parser(const Aggregator&);
	void ProcessDirectory(string cdrFilesDirectory, bool perFileAggregationTest);
	void RunPerFileAggregationTest(string);
	void SetPrintContents(bool);
private:
	Aggregator& m_aggregator;
	bool m_printFileContents;
};
