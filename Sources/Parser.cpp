#include <memory>
#include <boost/filesystem.hpp>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"

using namespace boost;

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


Parser::~Parser()
{
    SetStopFlag();
}

void Parser::ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension, std::string cdrArchiveDirectory)
{
	filesystem::path cdrPath(cdrFilesDirectory);
    filesystem::path archivePath(cdrArchiveDirectory);

    while(!IsShutdownFlagSet()) {
        try {
            filesystem::directory_iterator endIterator;
            bool filesFound = false;
            for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
                if (filesystem::is_regular_file(dirIterator->status()) &&
                        dirIterator->path().extension() == cdrExtension) {
                    filesFound = true;
                    logWriter << "Parsing file " + dirIterator->path().filename().string() + "...";
                    ParseFile(dirIterator->path().string());
                    filesystem::path archiveFilename = archivePath / dirIterator->path().filename();
                    logWriter << "File " + dirIterator->path().filename().string() + " parsed.";
                    filesystem::rename(dirIterator->path(), archiveFilename);
                    if (IsShutdownFlagSet()) {
                        break;
                    }
                }
            }
            if (!filesFound) {
                std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
            }
        }
        catch(const std::exception& ex) {
            logWriter << "Parser ERROR: ";
            logWriter << ex.what();
        }
    }
    logWriter << "Shutting down ...";
}


void Parser::ParseFile(std::string filename)
{
    FILE *pgwFile = fopen(filename.c_str(), "rb");
    if(!pgwFile) {
        throw std::string ("Unable to open input file ") + filename;
    }

    fseek(pgwFile, 0, SEEK_END);
    unsigned32 pgwFileLen = ftell(pgwFile); // длина данных файла (без заголовка)
    std::unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
    fseek(pgwFile, 0, SEEK_SET);
    size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
    fclose(pgwFile);
    if(bytesRead < pgwFileLen) {
        throw std::string("Error reading file ") + filename;
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
            if (fileContents)
                fclose(fileContents);
            throw std::runtime_error("Error while decoding ASN file. Error code " + std::to_string(rval.code));
        }
        if (printFileContents && fileContents != NULL) {
            asn_fprint(fileContents, &asn_DEF_GPRSRecord, gprsRecord);
        }
        nextChunk += rval.consumed;
        if (gprsRecord->present == GPRSRecord_PR_pGWRecord) {
            GetAppropiateAggregator(gprsRecord).AddCdrToQueue(gprsRecord);
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
