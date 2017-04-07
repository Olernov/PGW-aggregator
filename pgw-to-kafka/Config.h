#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "LogWriter.h"

struct Config
{
public:
    Config();
    Config(std::ifstream& cfgStream);

    void ReadConfigFile(std::ifstream& cfgStream);
    void ValidateParams();
    std::string DumpAllSettings();
    std::string connectString;
    std::string inputDir;
    std::string archiveDir;
    std::string badDir;
    std::string logDir;
    std::string cdrExtension;
    unsigned long noCdrAlertPeriodMin;
    LogLevel logLevel;
private:
    const std::string connectStringParamName = "CONNECT_STRING";
    const std::string inputDirParamName = "INPUT_DIR";
    const std::string archiveDirParamName = "ARCHIVE_DIR";
    const std::string badDirParamName = "BAD_DIR";
    const std::string logDirParamName = "LOG_DIR";
    const std::string cdrExtensionParamName = "CDR_FILES_EXTENSION";
    const std::string noCdrAlertPeriodParamName = "NO_CDR_ALERT_PERIOD_MIN";
    const std::string logLevelParamName = "LOG_LEVEL";
    unsigned long ParseULongValue(const std::string& name, const std::string& value);
};
