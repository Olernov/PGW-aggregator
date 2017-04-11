#pragma once
#include "Parser.h"

class MainLoopController
{
public:
    MainLoopController(const std::string &kafkaBroker, const std::string &kafkaTopic,
                       const std::string &cdrFilesDirectory,
                       const std::string &cdrExtension, const std::string &archiveDirectory,
                       const std::string &cdrBadDirectory);
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


