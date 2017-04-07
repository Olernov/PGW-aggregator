#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"
#include "rdkafkacpp.h"


extern Config config;
extern LogWriter logWriter;


Parser::Parser(const std::string &connectString, const std::string &filesDirectory, const std::string &extension,
               const std::string &archDirectory, const std::string &badDirectory) :
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    stopFlag(false),
    shutdownFilePath(filesDirectory + "/" + shutdownFlagFilename),
    printFileContents(false),
    lastAlertTime(notInitialized)
{
}

bool Parser::IsReady()
{
    postponeReason.clear();
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
    catch(const std::exception& ex) {
        logWriter.Write("ERROR while ProcessFile:", mainThreadIndex, error);
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
            //TODO: unhandled exceptions
            throw std::invalid_argument("Error while decoding ASN file. Error code " + std::to_string(rval.code));
        }
        if (printFileContents && fileContents != NULL) {
            asn_fprint(fileContents, &asn_DEF_GPRSRecord, gprsRecord);
        }
        nextChunk += rval.consumed;
        if (gprsRecord->present == GPRSRecord_PR_pGWRecord && gprsRecord->choice.pGWRecord.servedIMSI &&
                gprsRecord->choice.pGWRecord.listOfServiceData) { // process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
            AccumulateStats(totals, gprsRecord->choice.pGWRecord);
//            auto& aggr = GetAppropiateAggregator(gprsRecord);
//            aggr.AddCdrToQueue(gprsRecord);
//            aggr.WakeUp();
        }
        ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
        recordCount++;
    }
    if (fileContents) {
        fclose(fileContents);
    }
    return totals;
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


void Parser::RegisterFileStats(const std::string& filename, CdrFileTotals totals)
{
//    int attemptCount = 0;
//    while (attemptCount++ < maxAttemptsToWriteToDB) {
//        try {
//            otl_stream dbStream;
//            dbStream.open(1, "call Billing.Mobile_Data_Charger.RegisterFileStats(:filename /*char[100]*/, "
//                          ":vol_uplink/*bigint*/, :vol_downlink/*bigint*/, :rec_count/*long*/, "
//                          ":earliest_time<timestamp>, :latest_time<timestamp> )", dbConnect);
//            dbStream
//                    << filename
//                    << static_cast<signed64>(totals.volumeUplink)
//                    << static_cast<signed64>(totals.volumeDownlink)
//                    << static_cast<long>(totals.recordCount)
//                    << Utils::Time_t_to_OTL_datetime(totals.earliestTime)
//                    << Utils::Time_t_to_OTL_datetime(totals.latestTime) ;
//            dbStream.close();
//            break;
//        }
//        catch(const otl_exception& ex) {
//            logWriter.LogOtlException("**** DB ERROR in main thread while RegisterFileStats: ****", ex, mainThreadIndex);
//            dbConnect.reconnect();
//        }
//    }
}


bool Parser::SendMissingCdrAlert(double diffMinutes)
{
//    otl_stream dbStream;
//    try {
//        dbStream.open(1, std::string("call BILLING.MOBILE_DATA_CHARGER.SendAlert(:mess/*char[" +
//                       std::to_string(maxAlertMessageLen) + "]*/)").c_str(), dbConnect);
//        dbStream << std::string("New CDR files are missing for ") + std::to_string(diffMinutes) + " min.";
//        dbStream.close();
//        return true;
//    }
//    catch(const otl_exception& ex) {
//        logWriter.LogOtlException("**** DB ERROR in main thread while SendMissingCdrAlert: ****", ex, mainThreadIndex);
//        dbConnect.reconnect();
//        return false;
//    }
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}



