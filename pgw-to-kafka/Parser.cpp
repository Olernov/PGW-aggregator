#include <memory>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"
#include "LogWriter.h"
#include "Config.h"


extern Config config;
extern LogWriter logWriter;


KafkaEventCallback::KafkaEventCallback() :
    allBrokersDown(false)
{}

void KafkaEventCallback::event_cb (RdKafka::Event &event)
{
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        logWriter << "Kafka ERROR (" + RdKafka::err2str(event.err()) + "): " + event.str();
        if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
            allBrokersDown = true;
        break;
      case RdKafka::Event::EVENT_STATS:
        logWriter << "Kafka STATS: " + event.str();
        break;
      case RdKafka::Event::EVENT_LOG:
        logWriter << "Kafka LOG-" + std::to_string(event.severity()) + "-" + event.fac() + ":" + event.str();
        break;
      default:
        logWriter << "Kafka EVENT " + std::to_string(event.type()) + " (" +
                     RdKafka::err2str(event.err()) + "): " + event.str();
        break;
    }
}


void KafkaDeliveryReportCallback::dr_cb (RdKafka::Message &message)
{
    std::cout << "Message delivery for (" << message.len() << " bytes): " <<
        message.errstr() << std::endl;
    if (message.key())
      std::cout << "Key: " << *(message.key()) << ";" << std::endl;
}


Parser::Parser(const std::string &kafkaBroker, const std::string &kafkaTopic, const std::string &filesDirectory, const std::string &extension,
               const std::string &archDirectory, const std::string &badDirectory) :
    kafkaTopic(kafkaTopic),
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    printFileContents(false),
    stopFlag(false),
    lastAlertTime(notInitialized)
{
    kafkaGlobalConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    kafkaTopicConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    std::string errstr;
    if (kafkaGlobalConf->set("metadata.broker.list", kafkaBroker, errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set kafka glocal conf: " + errstr);
    }
    //kafkaGlobalConf->set("dr_cb", &deliveryReportCb, errstr);
    kafkaGlobalConf->set("event_cb", &eventCb, errstr);

    kafkaProducer = RdKafka::Producer::create(kafkaGlobalConf, errstr);
    if (!kafkaProducer) {
        throw std::runtime_error("Failed to create kafka producer: " + errstr);
    }
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
        ParseFile(pgwFile, file.string());
        if (!cdrArchiveDirectory.empty()) {
            filesystem::path archivePath(cdrArchiveDirectory);
            filesystem::path archiveFilename = archivePath / file.filename();
            filesystem::rename(file, archiveFilename);
        }
        logWriter << "File " + file.filename().string() + " processed.";
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
                gprsRecord->choice.pGWRecord.listOfServiceData) {
            // process only CDRs having service data i.e. data volume. Otherwise just ignore CDR record
            WaitForKafkaQueue();
            SendRecordToKafka(gprsRecord->choice.pGWRecord);
        }
        ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
        recordCount++;
    }
    if (fileContents) {
        fclose(fileContents);
    }
}


void Parser::WaitForKafkaQueue()
{
    while (kafkaProducer->outq_len() >= queueSizeThreshold)   {
        std::string message = std::to_string(kafkaProducer->outq_len()) + " message(s) are in Kafka producer queue. "
                "Waiting to be sent...";
        if (message != lastErrorMessage) {
            logWriter << message;
            lastErrorMessage = message;
        }
        kafkaProducer->poll(producerPollTimeoutMs);
    }
}

void Parser::SendRecordToKafka(const PGWRecord& pGWRecord)
{

    for (int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
        PGW_CDR avroCdr;
        avroCdr.IMSI = Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI);
        avroCdr.MSISDN = Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN);
        avroCdr.IMEI.set_string(Utils::TBCDString_to_String(pGWRecord.servedIMEISV));
        avroCdr.PGWNodeExportTime = Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime) * 1000, // milliseconds
        avroCdr.RatingGroup = pGWRecord.listOfServiceData->list.array[i]->ratingGroup;
        if (pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink) {
            avroCdr.VolumeUplink = *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCUplink;
        }
        if (pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink) {
            avroCdr.VolumeDownlink = *pGWRecord.listOfServiceData->list.array[i]->datavolumeFBCDownlink;
        }
        avroCdr.ChargingID = pGWRecord.chargingID;
        if (pGWRecord.recordSequenceNumber) {
            avroCdr.SequenceNumber.set_int(*pGWRecord.recordSequenceNumber);
        }
        avroCdr.Duration = pGWRecord.duration;
        int locInfoSize = pGWRecord.userLocationInformation->size;
        avroCdr.UserLocationInfo.resize(locInfoSize);
        std::copy(&pGWRecord.userLocationInformation->buf[0], &pGWRecord.userLocationInformation->buf[locInfoSize],
                avroCdr.UserLocationInfo.begin());
        std::unique_ptr<avro::OutputStream> out(avro::memoryOutputStream());
        avro::EncoderPtr encoder(avro::binaryEncoder());
        encoder->init(*out);
        avro::encode(*encoder, avroCdr);
        encoder->flush();
        size_t byteCount = out->byteCount();
        std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
        avro::StreamReader reader(*in);
        std::vector<uint8_t> rawData(byteCount);
        reader.readBytes(&rawData[0], byteCount);

        RdKafka::ErrorCode resp =
            kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
                                   rawData.data(), byteCount, nullptr, 0, 0, nullptr);
        if (resp != RdKafka::ERR_NO_ERROR) {
            // TODO: log and process error
            logWriter << "Kafka produce failed: " + RdKafka::err2str(resp);
        }
        kafkaProducer->poll(0);
    }
    //std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


Parser::~Parser()
{
    delete kafkaProducer;
    delete kafkaGlobalConf;
    delete kafkaTopicConf;
}
