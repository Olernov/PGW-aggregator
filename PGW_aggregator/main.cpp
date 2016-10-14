#include <iostream>
#include <cassert>
//#include <boost/lockfree/queue.hpp>

#include "ConfigContainer.h"
#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "Parser.h"

#define LOG_ERROR 3


void log(short msgType, string msgText)
{
    cout << msgText << endl;
}


void ClearTestExportTable(otl_connect& dbConnect)
{
    otl_stream dbStream;
    dbStream.open(1, "delete from TEST_SESSION_EXPORT", dbConnect);
    dbStream.close();
}


void RunAllTests(Parser& parser, Aggregator& aggregator, otl_connect& dbConnect)
{
    Utils::RunAllTests();

    std::string sampleCdrDirectory("../SampleCDR/");
    std::string cdrExtension(".dat");
    ClearTestExportTable(dbConnect);
    // uncomment if printing file contents neeeded:
    //parser.SetPrintContents(true);
    parser.ProcessDirectory(sampleCdrDirectory, cdrExtension, perFileTest);
    std::cout << "Checking exported data ..." << endl;
    aggregator.CheckExportedData(perFileTest);
    std::cout << "Per file aggregation test PASSED." << endl;

    ClearTestExportTable(dbConnect);
    parser.ProcessDirectory(sampleCdrDirectory, cdrExtension, totalTest);
    std::cout << "Exporting sessions ..." << endl;
    aggregator.ExportAllSessionsToDB("");
    std::cout << "Checking exported data ..." << endl;
    aggregator.CheckExportedData(totalTest);
    std::cout << "Total aggregation test PASSED." << endl;
}


int main(int argc, const char* argv[])
{
    otl_connect::otl_initialize();
	otl_connect dbConnect;
	try {
		dbConnect.rlogon("aggregator/aggregator@192.168.100.109:1521/irbistst");
		Aggregator aggregator(dbConnect);
        Parser parser(aggregator);
        RunAllTests(parser, aggregator, dbConnect);
        dbConnect.commit();
		dbConnect.logoff();
	}
	catch(otl_exception& otlEx) {
		dbConnect.rollback();
		// TODO: add correct processing
        std::cout << "DB error: " << std::endl
             << otlEx.msg << std::endl
             << otlEx.stm_text << std::endl
             << otlEx.var_info << std::endl;
		exit(-1);
	}
    return 0;
}
