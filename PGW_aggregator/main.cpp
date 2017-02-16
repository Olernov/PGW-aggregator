#include <iostream>
#include <cassert>
#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "DBConnect.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "MainLoopController.h"
#include "ExportRules.h"
#include "LogWriter.h"
#include "Config.h"

Config config;
LogWriter logWriter;

void log(short msgType, std::string msgText)
{
    std::cout << msgText << std::endl;
}


void ClearTestExportTable(DBConnect& dbConnect)
{
    otl_stream dbStream;
    dbStream.open(1, "call Billing.Mobile_Data_Charger.ClearMobileSessions()", dbConnect);
    dbStream.close();
}

void CheckExportedData(DBConnect& dbConnect)
{
    otl_stream otlStream;
    otlStream.open(1, "call Billing.Mobile_Data_Charger.CheckTestExport()", dbConnect);
    otlStream.close();
}

void RunStoredLogicTests(DBConnect& dbConnect)
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
    const std::string pidFilename = "/var/run/pgw-aggregator.pid";
    std::ofstream pidFile(pidFilename, std::ofstream::out);
    if (pidFile.is_open()) {
        pidFile << getpid();
    }
    pidFile.close();

    logWriter.Initialize(config.logDir, config.logLevel);
    logWriter << "PGW aggregator start";
    logWriter << config.DumpAllSettings();

    const int OTL_MULTITHREADED_MODE = 1;
    otl_connect::otl_initialize(OTL_MULTITHREADED_MODE);

    time_t testStart = notInitialized;
    DBConnect dbConnect;
    try {
        if (runTests) {
            Utils::RunAllTests();
            dbConnect.rlogon(config.connectString.c_str());
            RunStoredLogicTests(dbConnect);
            std::cout << "Further sample CDR test requires deletion from Mobile_Session table. Proceed (y/n)?"
                    << std::endl << "> ";
            char confirmation;
            std::cin >> confirmation;
            if (confirmation != 'y' && confirmation != 'Y') {
                exit(EXIT_SUCCESS);
            }
            ClearTestExportTable(dbConnect);
            dbConnect.commit();
            std::cout << "Put 33 files from SampleCDR directory to your INPUT_DIR and type y to proceed"
                << std::endl << "> ";
            std::cin >> confirmation;
            std::cout << "Loading sample CDR files ..." << std::endl;
            std::cout << "After all sample CDR files are loaded please stop PGW aggregator as usual. "
                         "Then export checks would start automatically" << std::endl;
            testStart = time(nullptr);
        }

        // Common part both for tests and production
        {
            MainLoopController mlc(config.connectString, config.inputDir, config.cdrExtension,
                      config.archiveDir, config.badDir);
            mlc.Run();
        }

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
    filesystem::remove(pidFilename);
    return 0;
}
