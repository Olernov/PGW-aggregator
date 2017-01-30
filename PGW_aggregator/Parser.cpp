#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"



extern Config config;
extern LogWriter logWriter;

Parser::Parser() :
    stopFlag(false),
    printFileContents(false)
{
    aggregators.reserve(config.threadCount);
    for (int i =0; i < config.threadCount; i++) {
        aggregators.push_back(Aggregator_ptr(new Aggregator(i)));
    }
}


void Parser::ProcessDirectory(const std::string& cdrFilesDirectory, const std::string& cdrExtension,
                              const std::string& cdrArchiveDirectory, const std::string& cdrBadDirectory)
{
	filesystem::path cdrPath(cdrFilesDirectory);

    bool parserSuspend = false;
    while(!IsShutdownFlagSet()) {
        try {
            filesystem::directory_iterator endIterator;
            bool filesFound = false;
            for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
                if (filesystem::is_regular_file(dirIterator->status()) &&
                        dirIterator->path().extension() == cdrExtension) {
                    filesFound = true;
                    parserSuspend = false;
                    ProcessFile(dirIterator->path(), cdrArchiveDirectory, cdrBadDirectory);
                    if (IsShutdownFlagSet()) {
                        break;
                    }
                }
            }
            if (!filesFound) {
                if (!parserSuspend) {
                    parserSuspend = true;
                    logWriter << "All CDR files processed.";
                }
                std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
            }
        }
        catch(const otl_exception& ex) {
            if (lastExceptionText != reinterpret_cast<const char*>(ex.msg)) {
                logWriter << "*************  DB ERROR in aggregating thread: **********";
                logWriter << reinterpret_cast<const char*>(ex.msg);
                logWriter << reinterpret_cast<const char*>(ex.stm_text);
                logWriter << reinterpret_cast<const char*>(ex.var_info);
                lastExceptionText = reinterpret_cast<const char*>(ex.msg);
            }
            std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
        }
        catch(const std::exception& ex) {
            if (lastExceptionText != ex.what()) {
                logWriter << "Parser ERROR: ";
                logWriter << ex.what();
                lastExceptionText = ex.what();
            }
            std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
        }
    }
    logWriter << "Shutting down ...";
}


void Parser::ProcessFile(const filesystem::path& file, const std::string& cdrArchiveDirectory,
                         const std::string& cdrBadDirectory)
{
    FILE *pgwFile = fopen(file.string().c_str(), "rb");
    if(!pgwFile) {
        logWriter << "Unable to open input file " + file.string() + ". Skipping it.";
        return;
    }
    logWriter << "Processing file " + file.filename().string() + "...";
    try {
        ParseFile(pgwFile, file.string());
        filesystem::path archivePath(cdrArchiveDirectory);
        filesystem::path archiveFilename = archivePath / file.filename();
        logWriter << "File " + file.filename().string() + " processed.";
        filesystem::rename(file, archiveFilename);
    }
    catch(const parse_error& ex) {
        logWriter << "ERROR while processing:";
        logWriter << ex.what();
        if (!cdrBadDirectory.empty()) {
            filesystem::path badFilePath(cdrBadDirectory);
            filesystem::path badFilename = badFilePath / file.filename();
            filesystem::rename(file, badFilename);
            logWriter << "File " + file.filename().string() + " moved to bad files directory " + cdrBadDirectory;
        }
    }
}


void Parser::ParseFile(FILE *pgwFile, const std::string& filename)
{
    fseek(pgwFile, 0, SEEK_END);
    unsigned32 pgwFileLen = ftell(pgwFile);
    std::unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
    fseek(pgwFile, 0, SEEK_SET);
    size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
    fclose(pgwFile);
    if(bytesRead < pgwFileLen) {
        throw std::invalid_argument(std::string("Error reading file ") + filename);
    }

    FILE* fileContents = NULL;
    if (printFileContents) {
        fileContents = fopen (std::string(filename + "_contents.txt").c_str(), "w");
    }

    asn_dec_rval_t rval;
    unsigned32 nextChunk = 0;
    unsigned32 recordCount = 0;
    const int maxPGWRecordSize = 2000;
    while(nextChunk < bytesRead) {
        GPRSRecord* gprsRecord = nullptr;
        rval = ber_decode(0, &asn_DEF_GPRSRecord, (void**) &gprsRecord, buffer.get() + nextChunk, maxPGWRecordSize);
        if(rval.code != RC_OK) {
            if (fileContents) {
                fclose(fileContents);
            }
            throw std::invalid_argument("Error while decoding ASN file. Error code " + std::to_string(rval.code));
        }
        if (printFileContents && fileContents != NULL) {
            asn_fprint(fileContents, &asn_DEF_GPRSRecord, gprsRecord);
        }
        nextChunk += rval.consumed;
        if (gprsRecord->present == GPRSRecord_PR_pGWRecord) {
            auto& aggr = GetAppropiateAggregator(gprsRecord);
            std::exception_ptr exceptionPtr = aggr.PopException();
            if (exceptionPtr != nullptr) {
                std::rethrow_exception(exceptionPtr);
            }
            aggr.AddCdrToQueue(gprsRecord);
        }
        recordCount++;
    }
    if (fileContents) {
        fclose(fileContents);
    }
}


Aggregator& Parser::GetAppropiateAggregator(const GPRSRecord* gprsRecord)
{
    unsigned64 imsi = Utils::TBCDString_to_ULongLong(gprsRecord->choice.pGWRecord.servedIMSI);
    return *aggregators[imsi % config.threadCount].get();
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


bool Parser::IsShutdownFlagSet()
{
    if (filesystem::exists(shutdownFlagFilename)) {
        filesystem::remove(shutdownFlagFilename);
        return true;
    }
    else {
        return false;
    }
}


void Parser::SetStopFlag()
{
    stopFlag = true;
    for (auto& agr : aggregators) {
        agr.get()->SetStopFlag();
    }
}

Parser::~Parser()
{
    SetStopFlag();
}

