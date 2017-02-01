#include <boost/filesystem.hpp>
#include "Config.h"

using namespace boost;

Config::Config() :
    threadCount(8),
    homePlmnID(25027),
    sessionEjectPeriodMin(30),
    exportRulesRefreshPeriodMin(30),
    cdrExtension(".dat"),
    logLevel(notice)
{
}


Config::Config(std::ifstream& configStream) :
    Config()
{
    ReadConfigFile(configStream);
}

//void Config::SetDefaults()
//{
//    threadCount = 8;
//    homePlmnID = 25027;
//    sessionEjectPeriodMin = 30;
//    exportRulesRefreshPeriodMin = 30;
//    cdrExtension = ".dat";
//}


void Config::ReadConfigFile(std::ifstream& configStream)
{
    std::string line;
    while (getline(configStream, line))
	{
		size_t pos = line.find_first_not_of(" \t\r\n");
        if (pos != std::string::npos) {
            if (line[pos] == '#' || line[pos] == '\0') {
				continue;
            }
        }
		size_t delim_pos = line.find_first_of(" \t=", pos);
        std::string option_name;
        if (delim_pos != std::string::npos) {
			option_name = line.substr(pos, delim_pos - pos);
        }
        else {
			option_name = line;
        }
		
        std::transform(option_name.begin(), option_name.end(), option_name.begin(), ::toupper);

		size_t value_pos = line.find_first_not_of(" \t=", delim_pos);
        std::string option_value;
        if (value_pos != std::string::npos) {
			option_value = line.substr(value_pos);
			size_t comment_pos = option_value.find_first_of(" \t#");
            if (comment_pos != std::string::npos)
				option_value = option_value.substr(0, comment_pos);
		}

        if (option_name == connectStringParamName) {
            connectString = option_value;
        }
        else if (option_name == inputDirParamName) {
            inputDir = option_value;
        }
        else if (option_name == archiveDirParamName) {
            archiveDir = option_value;
        }
        else if (option_name == badDirParamName) {
            badDir = option_value;
        }
        else if (option_name == logDirParamName) {
            logDir = option_value;
        }
        else if (option_name == sampleCdrDirParamName) {
            sampleCdrDir = option_value;
        }
        else if (option_name == cdrExtensionParamName) {
            cdrExtension == option_value;
        }
        else if (option_name == threadCountParamName) {
            threadCount = ParseULongValue(option_name, option_value);
        }
        else if (option_name == homePlmnIdParamName) {
            homePlmnID = ParseULongValue(option_name, option_value);
        }
        else if (option_name == sessionEjectPeriodParamName) {
            sessionEjectPeriodMin = ParseULongValue(option_name, option_value);
        }
        else if (option_name == exportRulesRefreshPeriodParamName) {
            exportRulesRefreshPeriodMin = ParseULongValue(option_name, option_value);
        }
        else if (option_name == logLevelParamName) {
            if (option_value == "error") {
                logLevel = error;
            }
            else if (option_value == "notice") {
                logLevel = notice;
            }
            else if (option_value == "debug") {
                logLevel = debug;
            }
            else {
                throw std::runtime_error("Wrong value passed for " + option_name + ".");
            }
        }
        else if (!option_name.empty()){
            throw std::runtime_error("Unknown parameter " + option_name + " found");
        }
	}	
}


unsigned long Config::ParseULongValue(const std::string& name, const std::string& value)
{
    try {
        return std::stoul(value);
    }
    catch(const std::invalid_argument&) {
        throw std::runtime_error("Wrong value given for numeric config parameter " + name);
    }
}

void Config::ValidateParams()
{
    if (connectString.empty()) {
        throw std::runtime_error(connectStringParamName + " parameter is not set.");
    }
    if (inputDir.empty()) {
        throw std::runtime_error(inputDirParamName + " parameter is not set.");
    }
    if (archiveDir.empty()) {
        throw std::runtime_error(archiveDirParamName + " parameter is not set.");
    }

    filesystem::path inputPath(inputDir);
    filesystem::path archivePath(archiveDir);

    if (!filesystem::exists(inputPath)) {
        throw std::runtime_error(std::string("Input directory ") + inputDir + " does not exist");
    }
    if (!filesystem::exists(archivePath)) {
        throw std::runtime_error(std::string("Archive directory ") + archiveDir + " does not exist");
    }
    if (!filesystem::is_directory(inputPath)) {
        throw std::runtime_error(inputDir + " is not a directory");
    }
    if (!filesystem::is_directory(archivePath)) {
        throw std::runtime_error(archiveDir + " is not a directory");
    }
    if (!(threadCount >= minThreadCount && threadCount <= maxThreadCount)) {
        throw std::runtime_error(threadCountParamName + " must have value from " +
                                 std::to_string(minThreadCount) + " to " + std::to_string(maxThreadCount));
    }
}

std::string Config::DumpAllSettings()
{
    return connectStringParamName + ": " + connectString + crlf +
           inputDirParamName + ": " + inputDir + crlf +
            archiveDirParamName + ": " + archiveDir + crlf +
            logDirParamName + ": " + logDir + crlf +
            cdrExtensionParamName + ": " + cdrExtension + crlf +
            homePlmnIdParamName + ": " + std::to_string(homePlmnID) + crlf +
            sessionEjectPeriodParamName + ": " + std::to_string(sessionEjectPeriodMin) + crlf +
            exportRulesRefreshPeriodParamName + ": " + std::to_string(exportRulesRefreshPeriodMin) + crlf +
            logLevelParamName + ": " + (logLevel == error ? "error" : (logLevel == debug ? "debug" : "notice"));
}

