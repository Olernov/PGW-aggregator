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
    std::string sampleCdrDir;
    std::string cdrExtension;
    unsigned short threadCount;
    unsigned long homePlmnID;
    unsigned long sessionEjectPeriodMin;
    unsigned long exportRulesRefreshPeriodMin;
    LogLevel logLevel;
private:
    const std::string connectStringParamName = "CONNECT_STRING";
    const std::string inputDirParamName = "INPUT_DIR";
    const std::string archiveDirParamName = "ARCHIVE_DIR";
    const std::string badDirParamName = "BAD_DIR";
    const std::string logDirParamName = "LOG_DIR";
    const std::string sampleCdrDirParamName = "SAMPLE_CDR_DIR";
    const std::string cdrExtensionParamName = "CDR_FILES_EXTENSION";
    const std::string threadCountParamName = "THREAD_COUNT";
    const std::string homePlmnIdParamName = "HOME_PLMN_ID";
    const std::string sessionEjectPeriodParamName = "SESSION_EJECT_PERIOD";
    const std::string exportRulesRefreshPeriodParamName = "EXPORT_RULES_REFRESH_PERIOD";
    const std::string logLevelParamName = "LOG_LEVEL";
    const int minThreadCount = 1;
    const int maxThreadCount = 32;
    //void SetDefaults();
    unsigned long ParseULongValue(const std::string& name, const std::string& value);
};
