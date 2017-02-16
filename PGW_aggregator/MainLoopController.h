#pragma once
#include "Parser.h"

class MainLoopController
{
public:
    MainLoopController(const std::string &connectString, const std::string &cdrFilesDirectory,
                       const std::string &cdrExtension, const std::string &archiveDirectory,
                       const std::string &cdrBadDirectory);
    void Run();
    void SetPrintContents(bool printContents);
    ~MainLoopController();
private:
    Parser parser;
    std::string cdrFilesDirectory;
    std::string cdrExtension;

    const std::string shutdownFlagFilename = "pgw-aggregator.stop";
    std::string shutdownFilePath;

    bool printFileContents;
    bool stopFlag;
    std::string lastExceptionText;

    const size_t maxAlertMessageLen = 2000;
    std::string lastAlertMessage;
    time_t lastAlertTime;

    bool IsShutdownFlagSet();
    void Sleep();
};


