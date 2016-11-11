#include <iostream>
#include <cassert>
//#include <boost/lockfree/queue.hpp>

#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "Parser.h"

#define LOG_ERROR 3


Config config;

void log(short msgType, std::string msgText)
{
    std::cout << msgText << std::endl;
}


void ClearTestExportTable(otl_connect& dbConnect)
{
    otl_stream dbStream;
    dbStream.open(1, "delete from TEST_SESSION_EXPORT", dbConnect);
    dbStream.close();
}

void CheckExportedData(otl_connect& dbConnect, AggregationTestType testType)
{
    otl_stream otlStream;
    otlStream.open(1, "call CHECK_TEST_EXPORT(:testType /*long,in*/)", dbConnect);
    otlStream << static_cast<long> (testType);
    otlStream.close();
}



void RunAllTests(otl_connect& dbConnect)
{
    Utils::RunAllTests();

    std::string sampleCdrDirectory("../SampleCDR/");
    std::string cdrExtension(".dat");
    //ClearTestExportTable(dbConnect);
    // uncomment if printing file contents neeeded:
    //parser.SetPrintContents(true);

    {
        Parser parser;
        parser.ProcessDirectory(sampleCdrDirectory, cdrExtension, perFileTest);
    }

    //std::cout << "Checking exported data ..." << endl;
    //CheckExportedData(dbConnect, perFileTest);
//    std::cout << "Per file aggregation test PASSED." << std::endl;

//    ClearTestExportTable(dbConnect);
//    {
//        Parser parser;
//        parser.ProcessDirectory(sampleCdrDirectory, cdrExtension, totalTest);
//        std::cout << "Exporting sessions ..." << std::endl;
//        parser.ExportAllSessionsToDB("");
//    }
    //std::cout << "Checking exported data ..." << std::endl;
    CheckExportedData(dbConnect,totalTest);
    std::cout << "Total aggregation test PASSED." << std::endl;
}


int main(int argc, const char* argv[])
{
    const int OTL_MULTITHREADED_MODE = 1;
    otl_connect::otl_initialize(OTL_MULTITHREADED_MODE);
	otl_connect dbConnect;
	try {
        dbConnect.rlogon(config.connectString.c_str());
        RunAllTests(dbConnect);
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
