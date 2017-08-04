#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "LogWriter.h"
#include "Common.h"

struct Config
{
public:
    Config();
    Config(std::ifstream& cfgStream);

    void ReadConfigFile(std::ifstream& cfgStream);
    void ValidateParams();
    std::string DumpAllSettings();
    std::string kafkaBroker;
    std::string kafkaTopic;
    std::string kafkaTopicTest;
    unsigned32 kafkaPartition;
    std::string inputDir;
    std::string archiveDir;
    std::string badDir;
    std::string logDir;
    std::string cdrExtension;
    unsigned32 noCdrAlertPeriodMin;
    LogLevel logLevel;
private:
    const std::string kafkaBrokerParamName = "KAFKA_BROKER";
    const std::string kafkaTopicParamName = "KAFKA_TOPIC";
    const std::string kafkaTopicTestParamName = "KAFKA_TOPIC_TEST";
    const std::string kafkaPartitionParamName = "KAFKA_PARTITION";
    const std::string inputDirParamName = "INPUT_DIR";
    const std::string archiveDirParamName = "ARCHIVE_DIR";
    const std::string badDirParamName = "BAD_DIR";
    const std::string logDirParamName = "LOG_DIR";
    const std::string cdrExtensionParamName = "CDR_FILES_EXTENSION";
    const std::string noCdrAlertPeriodParamName = "NO_CDR_ALERT_PERIOD_MIN";
    const std::string logLevelParamName = "LOG_LEVEL";
    unsigned long ParseULongValue(const std::string& name, const std::string& value);
};

