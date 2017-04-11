#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "LogWriterOtl.h"

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
    unsigned short threadCount;
    unsigned long homePlmnID;
    unsigned long sessionEjectPeriodMin;
    unsigned long exportRulesRefreshPeriodMin;
    unsigned long noCdrAlertPeriodMin;
    LogLevel logLevel;
private:
    const std::string connectStringParamName = "CONNECT_STRING";
    const std::string inputDirParamName = "INPUT_DIR";
    const std::string archiveDirParamName = "ARCHIVE_DIR";
    const std::string badDirParamName = "BAD_DIR";
    const std::string logDirParamName = "LOG_DIR";
    const std::string cdrExtensionParamName = "CDR_FILES_EXTENSION";
    const std::string threadCountParamName = "THREAD_COUNT";
    const std::string homePlmnIdParamName = "HOME_PLMN_ID";
    const std::string sessionEjectPeriodParamName = "SESSION_EJECT_PERIOD";
    const std::string exportRulesRefreshPeriodParamName = "EXPORT_RULES_REFRESH_PERIOD";
    const std::string noCdrAlertPeriodParamName = "NO_CDR_ALERT_PERIOD_MIN";
    const std::string logLevelParamName = "LOG_LEVEL";
    const int minThreadCount = 1;
    const int maxThreadCount = 32;
    unsigned long ParseULongValue(const std::string& name, const std::string& value);
};
