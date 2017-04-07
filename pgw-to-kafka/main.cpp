#include <iostream>
#include <cassert>
#include "GPRSRecord.h"
#include "Utils.h"
#include "Common.h"
#include "MainLoopController.h"
#include "LogWriter.h"
#include "Config.h"

Config config;
LogWriter logWriter;

void log(short msgType, std::string msgText)
{
    std::cout << msgText << std::endl;
}



void printUsage()
{
    std::cerr << "Usage: " << std::endl << "pgw-to-kafka <config-file> [-test]" << std::endl;
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
    const std::string pidFilename = "/var/run/pgw-to-kafka.pid";
    std::ofstream pidFile(pidFilename, std::ofstream::out);
    if (pidFile.is_open()) {
        pidFile << getpid();
    }
    pidFile.close();

    logWriter.Initialize(config.logDir, config.logLevel);
    logWriter << "PGW-to-Kafka start";
    logWriter << config.DumpAllSettings();

    try {
        if (runTests) {
            Utils::RunAllTests();
        }

        // Common part both for tests and production
        {
            MainLoopController mlc(config.connectString, config.inputDir, config.cdrExtension,
                      config.archiveDir, config.badDir);
            mlc.Run();
        }
    }
    catch(std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        logWriter.Write(ex.what(), mainThreadIndex, error);
    }
    logWriter << "PGW-to-Kafka shutdown";
    filesystem::remove(pidFilename);
    return 0;
}
