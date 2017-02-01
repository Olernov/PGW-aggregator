#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"



extern Config config;
extern LogWriter logWriter;
extern void ReconnectToDB(otl_connect& dbConnect, short sessionIndex);

Parser::Parser(const std::string &filesDirectory, const std::string &extension, const std::string &archDirectory, 
               const std::string &badDirectory, otl_connect& connect) :
    cdrFilesDirectory(filesDirectory),
    cdrExtension(extension),
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    dbConnect(connect),
    stopFlag(false),
    printFileContents(false)
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
    while(!IsShutdownFlagSet()) {
        try {
            filesystem::directory_iterator endIterator;
            bool filesFound = false;
            for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
                if (filesystem::is_regular_file(dirIterator->status()) &&
                        dirIterator->path().extension() == cdrExtension) {
                    filesFound = true;
                    parserSuspend = false;
                    if (std::all_of(aggregators.begin(), aggregators.end(), [](Aggregator_ptr& aggr)
                                                                            { return aggr.get()->IsReady(); } )) {
                        ProcessFile(dirIterator->path(), cdrArchiveDirectory, cdrBadDirectory);
                    }
                    else {
                        logWriter.Write("Some aggregators are not ready. Processing postponed.", mainThreadIndex, debug);
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
        catch(const otl_exception& ex) {
            logWriter.LogOtlException("**** DB ERROR in main thread: ****", ex, mainThreadIndex);
            ReconnectToDB(dbConnect, mainThreadIndex);
        }
        catch(const std::exception& ex) {
            if (lastExceptionText != ex.what()) {
                logWriter.Write("Parser ERROR: ", mainThreadIndex, error);
                logWriter.Write(ex.what(), mainThreadIndex, error);
                lastExceptionText = ex.what();
            }
        }
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
        filesystem::path archivePath(cdrArchiveDirectory);
        filesystem::path archiveFilename = archivePath / file.filename();
        logWriter << "File " + file.filename().string() + " processed.";
        filesystem::rename(file, archiveFilename);
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
    // TODO: catch exceptions
    otl_stream dbStream;
    dbStream.open(1, "call Billing.Mobile_Data_Charger.RegisterFileStats(:filename /*char[100]*/, "
                  ":vol_uplink/*bigint*/, :vol_downlink/*bigint*/, :rec_count/*long*/ )", dbConnect);
    dbStream
            << filename
            << static_cast<signed64>(totals.volumeUplink)
            << static_cast<signed64>(totals.volumeDownlink)
            << static_cast<long>(totals.recordCount);
    dbStream.close();
    dbConnect.commit();
}

void Parser::AlertAggregatorExceptions()
{
    std::string alertMessage;
    for (auto& aggr : aggregators) {
        if (!aggr.get()->IsReady()) {
            std::exception_ptr exPtr = aggr.get()->GetException();
            if (exPtr != nullptr) {
                try {
                    std::rethrow_exception(exPtr);
                }
                catch(const otl_exception& ex) {
                    alertMessage += "**** DB ERROR in aggregating thread #" +
                        std::to_string(aggr.get()->sessionIndex) + crlf +
                        reinterpret_cast<const char*>(ex.msg) + crlf +
                        reinterpret_cast<const char*>(ex.stm_text) + crlf +
                        reinterpret_cast<const char*>(ex.var_info) + crlf;
                }
                catch(const std::exception& ex) {
                    alertMessage += "**** ERROR in aggregating thread #" +
                       std::to_string(aggr.get()->sessionIndex) + crlf +
                       ex.what() + crlf;
                }
            }
        }
    }
    if (!alertMessage.empty()) {
        logWriter.Write("Sending AlertAggregatorExceptions:", mainThreadIndex, debug);
        logWriter.Write(alertMessage, mainThreadIndex, debug);
        otl_stream dbStream;
        dbStream.open(1, "call Billing.Mobile_Data_Charger.SendAlert(:message /*char[5000]*/)", dbConnect);
        dbStream << alertMessage;
        dbStream.close();
        dbConnect.commit();
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

