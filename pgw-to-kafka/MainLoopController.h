#pragma once
#include "Parser.h"

class MainLoopController
{
public:
    MainLoopController(const std::string &kafkaBroker, const std::string &kafkaTopic, unsigned32 kafkaPartition,
                       const std::string &cdrFilesDirectory,
                       const std::string &cdrExtension, const std::string &archiveDirectory,
                       const std::string &cdrBadDirectory,
                       bool runTests);
    void Run();
    void SetPrintContents(bool printContents);
    ~MainLoopController();
private:
    Parser parser;
    std::string cdrFilesDirectory;
    std::string cdrExtension;

    const std::string shutdownFlagFilename = "pgw-to-kafka.stop";
    std::string shutdownFilePath;

    bool printFileContents;
    bool stopFlag;

    bool IsShutdownFlagSet();
    void Sleep();
};


