#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"



extern Config config;
extern LogWriter logWriter;
extern void Reconnect(otl_connect& dbConnect, short sessionIndex);

Parser::Parser(const std::string &filesDirectory, const std::string &extension, const std::string &archDirectory, 
               const std::string &badDirectory, otl_connect& connect) :
    cdrFilesDirectory(filesDirectory),
    cdrExtension(extension),
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    dbConnect(connect),
    stopFlag(false),
    printFileContents(false),
    lastAlertTime(notInitialized)
{
    aggregators.reserve(config.threadCount);
    for (int i =0; i < config.threadCount; i++) {
        aggregators.push_back(Aggregator_ptr(new Aggregator(i)));
    }
}


void Parser::ProcessCdrFiles()
{
	filesystem::path cdrPath(cdrFilesDirectory);

    bool parserSuspend = false;
    try {
        while(!IsShutdownFlagSet()) {
            filesystem::directory_iterator endIterator;
            bool filesFound = false;
            for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
                if (filesystem::is_regular_file(dirIterator->status()) &&
                        dirIterator->path().extension() == cdrExtension) {
                    filesFound = true;
                    parserSuspend = false;
                    if (std::all_of(aggregators.begin(), aggregators.end(), [](Aggregator_ptr& aggr)
                                                                            { return aggr.get()->GetExceptionMessage().empty(); } )) {
                        ProcessFile(dirIterator->path(), cdrArchiveDirectory, cdrBadDirectory);
                    }
                    else {
                        logWriter.Write("Some aggregators have errors. Processing postponed.", mainThreadIndex, debug);
                        AlertAggregatorExceptions();
                        Sleep();
                    }
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
                Sleep();
            }
        }
    }
    catch(const std::exception& ex) {
            logWriter.Write("Parser ERROR: ", mainThreadIndex, error);
            logWriter.Write(ex.what(), mainThreadIndex, error);
            lastExceptionText = ex.what();
    }
    logWriter << "Shutting down ...";
}


void Parser::ProcessFile(const filesystem::path& file, const std::string& cdrArchiveDirectory,
                         const std::string& cdrBadDirectory)
{
    FILE *pgwFile = fopen(file.string().c_str(), "rb");
    if(!pgwFile) {
        logWriter.Write("Unable to open input file " + file.string() + ". Skipping it." , mainThreadIndex, error);
        return;
    }
    logWriter << "Processing file " + file.filename().string() + "...";
    try {
        auto totals = ParseFile(pgwFile, file.string());
        if (!cdrArchiveDirectory.empty()) {
            filesystem::path archivePath(cdrArchiveDirectory);
            filesystem::path archiveFilename = archivePath / file.filename();
            filesystem::rename(file, archiveFilename);
        }
        logWriter << "File " + file.filename().string() + " processed.";
        RegisterFileStats(file.filename().string(), totals);
    }
    catch(const parse_error& ex) {
        logWriter.Write("ERROR while processing:", mainThreadIndex, error);
        logWriter.Write(ex.what(), mainThreadIndex, error);
        if (!cdrBadDirectory.empty()) {
            filesystem::path badFilePath(cdrBadDirectory);
            filesystem::path badFilename = badFilePath / file.filename();
            filesystem::rename(file, badFilename);
            logWriter << "File " + file.filename().string() + " moved to bad files directory " + cdrBadDirectory;
        }
    }
}


CdrFileTotals Parser::ParseFile(FILE *pgwFile, const std::string& filename)
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
    CdrFileTotals totals;
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
        if (gprsRecord->present == GPRSRecord_PR_pGWRecord && gprsRecord->choice.pGWRecord.servedIMSI &&
                gprsRecord->choice.pGWRecord.listOfServiceData) { // process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
            auto& aggr = GetAppropiateAggregator(gprsRecord);
            aggr.AddCdrToQueue(gprsRecord);
            Accumulate(totals, gprsRecord->choice.pGWRecord);
        }
        recordCount++;
    }
    if (fileContents) {
        fclose(fileContents);
    }
    return totals;
}


Aggregator& Parser::GetAppropiateAggregator(const GPRSRecord* gprsRecord)
{
    unsigned64 imsi = Utils::TBCDString_to_ULongLong(gprsRecord->choice.pGWRecord.servedIMSI);
    return *aggregators[imsi % config.threadCount].get();
}


void Parser::Accumulate(CdrFileTotals& totals, const PGWRecord& pGWRecord)
{
    for(int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
        totals.volumeUplink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink;
        totals.volumeDownlink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink;
    }
    totals.recordCount++;
}


void Parser::RegisterFileStats(const std::string& filename, CdrFileTotals totals)
{
    int attemptCount = 0;
    while (attemptCount++ < maxAttemptsToWriteToDB) {
        try {
            otl_stream dbStream;
            dbStream.open(1, "call Billing.Mobile_Data_Charger.RegisterFileStats(:filename /*char[100]*/, "
                          ":vol_uplink/*bigint*/, :vol_downlink/*bigint*/, :rec_count/*long*/ )", dbConnect);
            dbStream
                    << filename
                    << static_cast<signed64>(totals.volumeUplink)
                    << static_cast<signed64>(totals.volumeDownlink)
                    << static_cast<long>(totals.recordCount);
            dbStream.close();
            break;
        }
        catch(const otl_exception& ex) {
            logWriter.LogOtlException("**** DB ERROR in main thread while RegisterFileStats: ****", ex, mainThreadIndex);
            Reconnect(dbConnect, mainThreadIndex);
        }
    }
}

void Parser::AlertAggregatorExceptions()
{
    std::string alertMessage;
    for (auto& aggr : aggregators) {
        std::string errorMessage = aggr.get()->GetExceptionMessage();
        if (!errorMessage.empty()) {
            alertMessage += "Error in aggregating thread #" + std::to_string(aggr.get()->thisIndex) + ": " +
                    errorMessage + crlf;
        }
        if (alertMessage.length() >= maxAlertMessageLen) {
            alertMessage.erase(maxAlertMessageLen-1);
            break;
        }
    }

    if (!alertMessage.empty()) {
        if (Utils::DiffMinutes(time(nullptr), lastAlertTime) >= config.alertRepeatPeriodMin) {
            int attemptCount = 0;
            while (attemptCount++ < maxAttemptsToWriteToDB) {
                try {
                    otl_stream dbStream;
                    dbStream.open(1, std::string("call BILLING.MOBILE_DATA_CHARGER.SendAlert(:mess/*char[" +
                                   std::to_string(maxAlertMessageLen) + "]*/)").c_str(), dbConnect);
                    dbStream << alertMessage;
                    dbStream.close();
                    break;
                }
                catch(const otl_exception& ex) {
                    logWriter.LogOtlException("**** DB ERROR in main thread while AlertAggregatorExceptions: ****",
                                              ex, mainThreadIndex);
                    Reconnect(dbConnect, mainThreadIndex);
                }
            }
            lastAlertMessage = alertMessage;
            lastAlertTime = time(nullptr);
        }
    }
}

void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


bool Parser::IsShutdownFlagSet()
{
    if (filesystem::exists(shutdownFlagFilename)) {
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


void Parser::Sleep()
{
    std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
}

Parser::~Parser()
{
    SetStopFlag();
    filesystem::remove(shutdownFlagFilename);
}


