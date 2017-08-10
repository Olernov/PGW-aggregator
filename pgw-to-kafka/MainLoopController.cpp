#include <boost/filesystem.hpp>
#include "MainLoopController.h"
#include "LogWriter.h"
#include "Config.h"
#include "Utils.h"

extern LogWriter logWriter;
extern Config config;

MainLoopController::MainLoopController(const std::string &kafkaBroker, const std::string &kafkaTopic,
                                       unsigned32 kafkaPartition, const std::string &filesDirectory,
                                       const std::string &extension, const std::string &archDirectory,
                                       const std::string &badDirectory, bool runTest) :
    parser(kafkaBroker, kafkaTopic, kafkaPartition, filesDirectory, extension,
           archDirectory, badDirectory, runTest),
    cdrFilesDirectory(filesDirectory),
    cdrExtension(extension),
    shutdownFilePath(filesDirectory + "/" + shutdownFlagFilename),
    printFileContents(false),
    stopFlag(false)
{
}


void MainLoopController::Run()
{
    bool allCdrProcessed = false;
    std::string lastPostponeReason;
    while(!IsShutdownFlagSet()) {
        try {
            fileList sourceFiles;
            ConstructSortedFileList(cdrFilesDirectory, cdrExtension, sourceFiles);
            if (sourceFiles.size() > 0) {
                allCdrProcessed = false;
                for (auto& file : sourceFiles) {
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
                }
            }
            else {
                if (!allCdrProcessed) {
                    allCdrProcessed = true;
                    logWriter << "All CDR files processed.";
                }
                Sleep();
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


