#include <boost/filesystem.hpp>
#include "MainLoopController.h"
#include "LogWriterOtl.h"
#include "Config.h"
#include "Utils.h"

extern LogWriterOtl logWriter;
extern Config config;

MainLoopController::MainLoopController(const std::string &connectString, const std::string &filesDirectory,
                                       const std::string &extension, const std::string &archDirectory, const std::string &badDirectory) :
    parser(connectString, filesDirectory, extension, archDirectory, badDirectory),
    cdrFilesDirectory(filesDirectory),
    cdrExtension(extension),
    shutdownFilePath(filesDirectory + "/" + shutdownFlagFilename),
    printFileContents(false),
    stopFlag(false),
    lastAlertTime(notInitialized)
{
}


void MainLoopController::Run()
{
    bool allCdrProcessed = false;
    std::string lastPostponeReason;
    time_t lastCdrFileTime = time(nullptr);
    bool missingCdrAlertSent = false;
    while(!IsShutdownFlagSet()) {
        try {
            parser.RefreshExportRulesIfNeeded();
            fileList sourceFiles;
            ConstructSortedFileList(cdrFilesDirectory, cdrExtension, sourceFiles);
            if (sourceFiles.size() > 0) {
                allCdrProcessed = false;
                for (auto& file : sourceFiles) {
                    lastCdrFileTime = time(nullptr);
                    if (parser.IsReady()) {
                        lastPostponeReason.clear();
                        parser.ProcessFile(file);
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
                    parser.RefreshExportRulesIfNeeded();
                }
            }
            if (sourceFiles.empty()) {
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

void MainLoopController::ConstructSortedFileList(const std::string& inputDir,
                                                 const std::string& cdrExtension,
                                                 fileList& sourceFiles)
{
    filesystem::path inputPath(inputDir);
    filesystem::directory_iterator endIterator;
    for(filesystem::directory_iterator iter(inputPath); iter != endIterator; iter++) {
        if (filesystem::is_regular_file(iter->status())
                && iter->path().extension() == cdrExtension) {
            sourceFiles.push_back(iter->path());
        }
    }
    std::sort(sourceFiles.begin(), sourceFiles.end());
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


