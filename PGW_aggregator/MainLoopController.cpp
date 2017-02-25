#include <boost/filesystem.hpp>
#include "MainLoopController.h"
#include "LogWriter.h"
#include "Config.h"
#include "Utils.h"

extern LogWriter logWriter;
extern Config config;

MainLoopController::MainLoopController(const std::string &connectString, const std::string &filesDirectory,
                                       const std::string &extension, const std::string &archDirectory, const std::string &badDirectory) :
    parser(connectString, filesDirectory, extension, archDirectory, badDirectory),
    cdrFilesDirectory(filesDirectory),
    cdrExtension(extension),
    stopFlag(false),
    shutdownFilePath(filesDirectory + "/" + shutdownFlagFilename),
    printFileContents(false),
    lastAlertTime(notInitialized)
{
}


void MainLoopController::Run()
{
    filesystem::path cdrPath(cdrFilesDirectory);
    bool allCdrProcessed = false;
    std::string lastPostponeReason;
    time_t lastCdrFileTime = time(nullptr);
    bool missingCdrAlertSent = false;
    while(!IsShutdownFlagSet()) {
        try {
            parser.RefreshExportRulesIfNeeded();
            filesystem::directory_iterator endIterator;
            bool filesFound = false;
            for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
                if (filesystem::is_regular_file(dirIterator->status()) &&
                        dirIterator->path().extension() == cdrExtension) {
                    filesFound = true;
                    allCdrProcessed = false;
                    lastCdrFileTime = time(nullptr);
                    if (parser.IsReady()) {
                        lastPostponeReason.clear();
                        parser.ProcessFile(dirIterator->path());
                    }
                    else {
                        if (lastPostponeReason != parser.GetPostponeReason()) {
                            lastPostponeReason = parser.GetPostponeReason();
                            logWriter.Write("CDR processing postponed due to: " + lastPostponeReason, mainThreadIndex);
                        }
                        Sleep();
                    }
                    if (IsShutdownFlagSet()) {
                        break;
                    }
                }
            }
            if (!filesFound) {
                if (!allCdrProcessed) {
                    allCdrProcessed = true;
                    logWriter << "All CDR files processed.";
                }
                double diff = Utils::DiffMinutes(time(nullptr), lastCdrFileTime);
                if (diff >= config.noCdrAlertPeriodMin && !missingCdrAlertSent) {
                    logWriter << "Sending missing CDR alert";
                    missingCdrAlertSent = parser.SendMissingCdrAlert(diff);
                }
                Sleep();
            }
            else {
                missingCdrAlertSent = false;
            }
        }
        catch(const std::exception& ex) {
            logWriter.Write("ERROR in MainLoopProcessor::Run:", mainThreadIndex, error);
            logWriter.Write(ex.what(), mainThreadIndex, error);
            Sleep();
        }
    }
    logWriter << "Shutting down ...";
}


void MainLoopController::Sleep()
{
    std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
}

bool MainLoopController::IsShutdownFlagSet()
{
    if (filesystem::exists(shutdownFilePath)) {
        return true;
    }
    else {
        return false;
    }
}



void MainLoopController::SetPrintContents(bool printContents)
{
    parser.SetPrintContents(printContents);
}



MainLoopController::~MainLoopController()
{
    parser.SetStopFlag();
    filesystem::remove(shutdownFilePath);
}


