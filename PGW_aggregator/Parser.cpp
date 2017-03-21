#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"


extern Config config;
extern LogWriter logWriter;


Parser::Parser(const std::string &connectString, const std::string &filesDirectory, const std::string &extension,
               const std::string &archDirectory, const std::string &badDirectory) :
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    shutdownFilePath(filesDirectory + "/" + shutdownFlagFilename),
    exportRules(dbConnect, config.exportRulesRefreshPeriodMin),
    printFileContents(false),
    stopFlag(false),
    lastAlertTime(notInitialized)
{
    dbConnect.rlogon(connectString.c_str());
    aggregators.reserve(config.threadCount);
    for (int i =0; i < config.threadCount; i++) {
        aggregators.push_back(Aggregator_ptr(new Aggregator(i, connectString, exportRules)));
    }
}

bool Parser::IsReady()
{
    postponeReason.clear();
    if (std::any_of(aggregators.begin(), aggregators.end(),
                     [](Aggregator_ptr& aggr) { return !aggr.get()->GetExceptionMessage().empty(); } )) {
        postponeReason = "Aggregator exceptions";
    }
    else if (!ChargingAllowed()) {
        postponeReason = "Charging forbidden";
    }
    return postponeReason.empty();
}

void Parser::ProcessFile(const filesystem::path& file)
{
    FILE *pgwFile = fopen(file.string().c_str(), "rb");
    if(!pgwFile) {
        logWriter.Write("Unable to open input file " + file.string() + ". Skipping it." , mainThreadIndex, error);
        return;
    }
    logWriter << "Processing file " + file.filename().string() + "...";
    time_t processStartTime;
    time(&processStartTime);
    bool parseError = false;
    CdrFileTotals totals;
    try {
        ParseFile(pgwFile, file.string(), totals);
    }
    catch(const std::exception& ex) {
        logWriter.Write("ERROR while ProcessFile:", mainThreadIndex, error);
        logWriter.Write(ex.what(), mainThreadIndex, error);
        parseError = true;
    }
    long processTimeSec = Utils::DiffMinutes(processStartTime, time(nullptr)) * 60;
    logWriter << "File " + file.filename().string() + " processed in " +
                 std::to_string(processTimeSec) + " sec.";
    RegisterFileStats(file.filename().string(), totals, processTimeSec, filesystem::last_write_time(file), parseError);
    if (!parseError && !cdrArchiveDirectory.empty()) {
        filesystem::path archivePath(cdrArchiveDirectory);
        filesystem::path archiveFilename = archivePath / file.filename();
        filesystem::rename(file, archiveFilename);
    }
    else if (parseError && !cdrBadDirectory.empty()) {
        filesystem::path badFilePath(cdrBadDirectory);
        filesystem::path badFilename = badFilePath / file.filename();
        filesystem::rename(file, badFilename);
        logWriter << "File " + file.filename().string() + " moved to bad files directory " + cdrBadDirectory;
    }
}


void Parser::ParseFile(FILE *pgwFile, const std::string& filename, CdrFileTotals& totals)
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
    const int maxPGWRecordSize = 5000;
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
            AccumulateStats(totals, gprsRecord->choice.pGWRecord);
            auto& aggr = GetAppropiateAggregator(gprsRecord);
            aggr.AddCdrToQueue(gprsRecord);
            aggr.WakeUp();
        }
        else {
            ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
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


void Parser::AccumulateStats(CdrFileTotals& totals, const PGWRecord& pGWRecord)
{
    for(int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
        totals.volumeUplink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink;
        totals.volumeDownlink += *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink;
    }
    totals.recordCount++;
    time_t cdrTime = Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime);
    if (totals.earliestTime == notInitialized || cdrTime < totals.earliestTime) {
        totals.earliestTime = cdrTime;
    }
    if (totals.latestTime == notInitialized || cdrTime > totals.latestTime) {
        totals.latestTime = cdrTime;
    }
}


void Parser::RefreshExportRulesIfNeeded()
{
    exportRules.RefreshIfNeeded();
}


bool Parser::ChargingAllowed()
{
    long res = 0;
    try {
        otl_stream stream;
        stream.open(1, "call Billing.Mobile_Data_Charger.ChargingAllowed() into :res /*long,out*/", dbConnect);
        stream >> res;
    }
    catch(const otl_exception& ex) {
        logWriter.LogOtlException("**** DB ERROR in main thread while ChargingAllowed: ****", ex, mainThreadIndex);
        dbConnect.reconnect();
    }

    return res > 0;
}

void Parser::RegisterFileStats(const std::string& filename, CdrFileTotals totals, long processTimeSec,
                               time_t fileTimestamp, bool parseError)
{
    int attemptCount = 0;
    while (attemptCount++ < maxAttemptsToWriteToDB) {
        try {
            otl_stream dbStream;
            dbStream.open(1, "call Billing.Mobile_Data_Charger.RegisterFileStats(:filename /*char[100]*/, "
                          ":vol_uplink/*bigint*/, :vol_downlink/*bigint*/, :rec_count/*long*/, "
                          ":earliest_time/*timestamp*/, :latest_time/*timestamp*/, :file_timestamp/*timestamp*/, "
                          ":process_time /*long*/, :parse_error /*long*/)", dbConnect);
            dbStream
                    << filename
                    << static_cast<signed64>(totals.volumeUplink)
                    << static_cast<signed64>(totals.volumeDownlink)
                    << static_cast<long>(totals.recordCount)
                    << Utils::Time_t_to_OTL_datetime(totals.earliestTime)
                    << Utils::Time_t_to_OTL_datetime(totals.latestTime)
                    << Utils::Time_t_to_OTL_datetime(fileTimestamp)
                    << processTimeSec
                    << (parseError ? 1L : 0L);
            dbStream.close();
            break;
        }
        catch(const otl_exception& ex) {
            logWriter.LogOtlException("**** DB ERROR in main thread while RegisterFileStats: ****", ex, mainThreadIndex);
            dbConnect.reconnect();
        }
    }
}


bool Parser::SendMissingCdrAlert(double diffMinutes)
{
    otl_stream dbStream;
    try {
        dbStream.open(1, std::string("call BILLING.MOBILE_DATA_CHARGER.SendAlert(:mess/*char[" +
                       std::to_string(maxAlertMessageLen) + "]*/)").c_str(), dbConnect);
        dbStream << std::string("New CDR files are missing for ") + std::to_string(diffMinutes) + " min.";
        dbStream.close();
        return true;
    }
    catch(const otl_exception& ex) {
        logWriter.LogOtlException("**** DB ERROR in main thread while SendMissingCdrAlert: ****", ex, mainThreadIndex);
        dbConnect.reconnect();
        return false;
    }
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


void Parser::SetStopFlag()
{
    stopFlag = true;
    for (auto& agr : aggregators) {
        agr.get()->SetStopFlag();
        agr.get()->WakeUp();
    }
}



