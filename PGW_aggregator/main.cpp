#include <iostream>
#include <cassert>
#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "Parser.h"


Config config;

void log(short msgType, std::string msgText)
{
    std::cout << msgText << std::endl;
}


void ClearTestExportTable(otl_connect& dbConnect)
{
    otl_stream dbStream;
    dbStream.open(1, "call Billing.Mobile_Data_Charger.ClearMobileSessions()", dbConnect);
    dbStream.close();
}

void CheckExportedData(otl_connect& dbConnect)
{
    otl_stream otlStream;
    otlStream.open(1, "call Billing.Mobile_Data_Charger.CheckTestExport()", dbConnect);
    otlStream.close();
}

void RunStoredLogicTests(otl_connect& dbConnect)
{
    std::cout << "Running stored database logic tests ..." << std::endl;
    otl_stream otlStream;
    otlStream.open(1, "call Billing.Mobile_Data_Charger.RunAllTests()", dbConnect);
    otlStream.close();

}

void RunAllTests(otl_connect& dbConnect)
{
    Utils::RunAllTests();
    RunStoredLogicTests(dbConnect);

    std::string sampleCdrDirectory("../SampleCDR/");
    std::string cdrExtension(".dat");
    ClearTestExportTable(dbConnect);
    dbConnect.commit();

    time_t testStart = time(nullptr);

    {
        Parser parser;
        // uncomment if printing file contents neeeded:
        //parser.SetPrintContents(true);
        parser.ProcessDirectory(sampleCdrDirectory, cdrExtension);
    }
    std::cout << "Export consumed " << difftime(time(nullptr), testStart) << " seconds" << std::endl;
    std::cout << "Checking exported data ..." << std::endl;
    CheckExportedData(dbConnect);
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
