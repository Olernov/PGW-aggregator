#include <iostream>
#include <cassert>
#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "Parser.h"
#include "ExportRules.h"
#include "LogWriter.h"
#include "Config.h"

Config config;
ExportRules exportRules;
LogWriter logWriter;

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



void printUsage()
{
    std::cerr << "Usage: " << std::endl << "pgw-aggregator <config-file> [-test]" << std::endl;
}


void Reconnect(otl_connect& dbConnect, short sessionIndex)
{
    try {
        dbConnect.logoff();
    }
    catch(const otl_exception& ex) {
        // don't react on possible exception
    }
    dbConnect.connected = false;
    try {
        dbConnect.rlogon(config.connectString.c_str());
        logWriter.Write("(Re)Connected successfully", sessionIndex);
        dbConnect.connected = true;
    }
    catch(const otl_exception& ex) {
        logWriter.Write("**** DB ERROR while logging to DB: **** " + crlf +
            Utils::OtlExceptionToText(ex) + ": ****", sessionIndex);
    }
}


int main(int argc, const char* argv[])
{
    if (argc < 2) {
        printUsage();
        exit(EXIT_FAILURE);
    }
    const char* confFilename = argv[1];
    bool runTests = false;
    if (argc > 2 && !strncasecmp(argv[2], "-test", 5)) {
        runTests = true;
    }
    std::ifstream confFile(confFilename, std::ifstream::in);
    if (!confFile.is_open()) {
        std::cerr << "Unable to open config file " << confFilename << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        config.ReadConfigFile(confFile);
        config.ValidateParams();
    }
    catch(const std::exception& ex) {
        std::cerr << "Error when parsing config file " << confFilename << " " << std::endl;
        std::cerr << ex.what() <<std::endl;
        exit(EXIT_FAILURE);
    }

    logWriter.Initialize(config.logDir, config.logLevel);
    logWriter << "PGW aggregator start";
    logWriter << config.DumpAllSettings();

    const int OTL_MULTITHREADED_MODE = 1;
    otl_connect::otl_initialize(OTL_MULTITHREADED_MODE);
	otl_connect dbConnect;
    Reconnect(dbConnect, mainThreadIndex);
    if (!dbConnect.connected) {
        std::cerr << "Unable to log to DB. " << std::endl;
    }

    time_t testStart;
    try {
        exportRules.ReadSettingsFromDatabase(dbConnect);
        if (runTests) {
            Utils::RunAllTests();
            RunStoredLogicTests(dbConnect);
            std::cout << "Further sample CDR test requires deletion from Mobile_Session table. Proceed (y/n)?"
                    << std::endl << "> ";
            char confirmation;
//            std::cin >> confirmation;
//            if (confirmation != 'y' && confirmation != 'Y') {
//                exit(EXIT_SUCCESS);
//            }
            ClearTestExportTable(dbConnect);
            dbConnect.commit();
                        std::cout << "Put 33 files from SampleCDR directory to your INPUT_DIR and type y to proceed"
                                << std::endl << "> ";
            //            std::cin >> confirmation;
            std::cout << "Loading sample CDR files ..." << std::endl;
            std::cout << "After all sample CDR files are loaded please stop PGW aggregator as usual. "
                         "Then export checks would start automatically" << std::endl;
            testStart = time(nullptr);
        }
        Parser parser(config.inputDir, config.cdrExtension, config.archiveDir, config.badDir, dbConnect);
        parser.ProcessCdrFiles();
        if (runTests) {
            std::cout << "Test consumed " << Utils::DiffMinutes(time(nullptr), testStart) << " minutes" << std::endl;
            std::cout << "Checking exported data ..." << std::endl;
            CheckExportedData(dbConnect);
            std::cout << "All tests PASSED." << std::endl;
        }

        dbConnect.commit();
		dbConnect.logoff();
	}
    catch(otl_exception& ex) {
        std::cerr << Utils::OtlExceptionToText(ex) << std::endl;
	}
    logWriter << "PGW aggregator shutdown";
    return 0;
}
