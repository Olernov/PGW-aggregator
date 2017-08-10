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


Parser::Parser(const std::string &kafkaBroker, const std::string &kafkaTopic, unsigned32 partition,
               const std::string &filesDirectory, const std::string &extension,
               const std::string &archDirectory, const std::string &badDirectory, bool runtest) :
    kafkaTopic(kafkaTopic),
    kafkaPartition(partition),
    cdrArchiveDirectory(archDirectory),
    cdrBadDirectory(badDirectory),
    printFileContents(false),
    runTest(runtest),
    stopFlag(false),
    lastAlertTime(notInitialized),
    kafkaGlobalConf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)),
    kafkaTopicConf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC))
{

    std::string errstr;
    if (kafkaGlobalConf->set("bootstrap.servers", kafkaBroker, errstr) != RdKafka::Conf::CONF_OK
            || kafkaGlobalConf->set("group.id", "pgw-emitter", errstr) != RdKafka::Conf::CONF_OK
            || kafkaGlobalConf->set("api.version.request", "true", errstr) != RdKafka::Conf::CONF_OK
            || kafkaGlobalConf->set("socket.keepalive.enable", "true", errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set kafka global conf: " + errstr);
    }
    if (runTest && kafkaGlobalConf->set("debug", "all", errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set kafka global conf: " + errstr);
    }
    kafkaGlobalConf->set("event_cb", &eventCb, errstr);

    kafkaProducer = std::unique_ptr<RdKafka::Producer>(RdKafka::Producer::create(kafkaGlobalConf.get(), errstr));
    if (!kafkaProducer) {
        throw std::runtime_error("Failed to create kafka producer: " + errstr);
    }
    if (runTest) {
        RunTests();
    }
}


bool Parser::IsReady()
{
    postponeReason.clear();
    kafkaProducer->poll(0);
    if (kafkaProducer->outq_len() > 5000) {
        postponeReason = std::to_string(kafkaProducer->outq_len()) + " unsent messages in Kafka queue";
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

    int64_t lowOffset = 0;
    int64_t highOffset = 0;
    if (runTest) {
        RdKafka::ErrorCode res = kafkaProducer->query_watermark_offsets (kafkaTopic, kafkaPartition, &lowOffset, &highOffset, 1000);
        if (res != RdKafka::ERR_NO_ERROR) {
            std::cout << "Kafka query_watermark_offsets failed: " + RdKafka::err2str(res) << std::endl;
            assert(false);
        }
        std::cout << "Kafka low watermark: " << lowOffset << std::endl;
        std::cout << "Kafka high watermark: " << highOffset << std::endl;
        sentAvroCdrs.clear();
    }

    logWriter << "Processing file " + file.filename().string() + "...";
    time_t processStartTime;
    time(&processStartTime);
    unsigned32 recordCount = 0;
    try {
        recordCount = ParseFile(pgwFile, file.string());
        if (!cdrArchiveDirectory.empty()) {
            filesystem::path archivePath(cdrArchiveDirectory);
            filesystem::path archiveFilename = archivePath / file.filename();
            filesystem::rename(file, archiveFilename);
        }
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
    long processTimeSec = Utils::DiffMinutes(processStartTime, time(nullptr)) * 60;
    logWriter << "File " + file.filename().string() + " having " +std::to_string(recordCount) +
                 " records processed in " +
                 std::to_string(processTimeSec) + " sec.";

    if (runTest) {
        assert(sentAvroCdrs.size() == recordCount);
        std::cout<< "Consuming sent records from Kafka ..." << std::endl;
        assert(CompareSentAndConsumedRecords(highOffset));
    }
}


unsigned32 Parser::ParseFile(FILE *pgwFile, const std::string& filename)
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
            recordCount += SendRecordToKafka(gprsRecord->choice.pGWRecord);
        }
        ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
    }
    kafkaProducer->poll(0);
    if (fileContents) {
        fclose(fileContents);
    }

    return recordCount;
}


void Parser::WaitForKafkaQueue()
{
    while (kafkaProducer->outq_len() > 0)   {
        std::string message = std::to_string(kafkaProducer->outq_len()) + " message(s) are in Kafka producer queue. "
                "Waiting to be sent...";
        if (message != lastErrorMessage) {
            logWriter << message;
            lastErrorMessage = message;
        }
        kafkaProducer->poll(producerPollTimeoutMs);
    }
}

void Parser::ConstructAvroCdr(const PGWRecord& pGWRecord, int listIndex, PGW_CDR& avroCdr)
{
    avroCdr.IMSI = Utils::TBCDString_to_ULongLong(pGWRecord.servedIMSI);
    avroCdr.MSISDN = Utils::TBCDString_to_ULongLong(pGWRecord.servedMSISDN);

    if (pGWRecord.servedIMEISV != nullptr) {
        avroCdr.IMEI.set_string(Utils::TBCDString_to_String(pGWRecord.servedIMEISV));
    }
    else {
        avroCdr.IMEI.set_null();
    }
    if (pGWRecord.servedPDPPDNAddress != nullptr
            && pGWRecord.servedPDPPDNAddress->present == PDPAddress_PR_iPAddress) {
        avroCdr.ServedPDPAddress = Utils::IPAddress_to_ULong(&pGWRecord.servedPDPPDNAddress->choice.iPAddress);
    }
    else {
        avroCdr.ServedPDPAddress = 0;
    }
    if (pGWRecord.listOfServiceData->list.array[listIndex]->timeOfFirstUsage != nullptr) {
        avroCdr.FirstUsageTime = Utils::Timestamp_to_time_t(
                    pGWRecord.listOfServiceData->list.array[listIndex]->timeOfFirstUsage) * 1000; // milliseconds
    }
    else if (pGWRecord.listOfServiceData->list.array[listIndex]->timeOfLastUsage != nullptr) {
        avroCdr.FirstUsageTime = Utils::Timestamp_to_time_t(
                    pGWRecord.listOfServiceData->list.array[listIndex]->timeOfLastUsage) * 1000;
    }
    else {
        avroCdr.FirstUsageTime = Utils::Timestamp_to_time_t(&pGWRecord.recordOpeningTime);
    }
    avroCdr.RatingGroup = pGWRecord.listOfServiceData->list.array[listIndex]->ratingGroup;
    if (pGWRecord.listOfServiceData->list.array[listIndex]->datavolumeFBCUplink) {
        avroCdr.VolumeUplink = *pGWRecord.listOfServiceData->list.array[listIndex]->datavolumeFBCUplink;
    }
    if (pGWRecord.listOfServiceData->list.array[listIndex]->datavolumeFBCDownlink) {
        avroCdr.VolumeDownlink = *pGWRecord.listOfServiceData->list.array[listIndex]->datavolumeFBCDownlink;
    }
    avroCdr.ChargingID = pGWRecord.chargingID;
    if (pGWRecord.recordSequenceNumber) {
        avroCdr.SequenceNumber.set_int(*pGWRecord.recordSequenceNumber);
    }
    else {
        avroCdr.SequenceNumber.set_null();
    }
    if (pGWRecord.listOfServiceData->list.array[listIndex]->timeUsage != nullptr) {
        avroCdr.TimeOfUsage = *pGWRecord.listOfServiceData->list.array[listIndex]->timeUsage;
    }
    else {
        avroCdr.TimeOfUsage = pGWRecord.duration;
    }
    if (pGWRecord.userLocationInformation != nullptr) {
        int locInfoSize = pGWRecord.userLocationInformation->size;
        avroCdr.UserLocationInfo.resize(locInfoSize);
        std::copy(&pGWRecord.userLocationInformation->buf[0], &pGWRecord.userLocationInformation->buf[locInfoSize],
            avroCdr.UserLocationInfo.begin());
    }
    else {
        avroCdr.UserLocationInfo.resize(0);
    }
}


int Parser::EncodeAvro(const PGW_CDR& avroCdr, avro::OutputStream* out)
{
    avro::EncoderPtr encoder(avro::binaryEncoder());
    encoder->init(*out);
    avro::encode(*encoder, avroCdr);
    encoder->flush();
    return out->byteCount();
}


void Parser::ReadEncodedAvroCdr(avro::OutputStream* out, size_t byteCount, std::vector<uint8_t>& rawData)
{
    std::unique_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
    avro::StreamReader reader(*in);
    reader.readBytes(&rawData[0], byteCount);
}


std::vector<uint8_t> Parser::EncodeCdr(const PGW_CDR& avroCdr)
{
    std::unique_ptr<avro::OutputStream> out(avro::memoryOutputStream());
    size_t byteCount = EncodeAvro(avroCdr, out.get());
    std::vector<uint8_t> rawData(byteCount);
    ReadEncodedAvroCdr(out.get(), byteCount, rawData);
    return rawData;
}


int Parser::SendRecordToKafka(const PGWRecord& pGWRecord)
{
    unsigned32 sent = 0;
    for (int i = 0; i < pGWRecord.listOfServiceData->list.count; i++) {
        PGW_CDR avroCdr;
        ConstructAvroCdr(pGWRecord, i, avroCdr);
        std::vector<uint8_t> rawData = EncodeCdr(avroCdr);
        RdKafka::ErrorCode resp;
        std::string errstr;
        do {
            resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                                   RdKafka::Producer::RK_MSG_COPY,
                                   rawData.data(), rawData.size(), nullptr, 0,
                                   time(nullptr) * 1000 /*milliseconds*/, nullptr);
            if (resp == RdKafka::ERR__QUEUE_FULL) {
                kafkaProducer->poll(producerPollTimeoutMs);
            }
        }
        while (resp == RdKafka::ERR__QUEUE_FULL);

        if (resp != RdKafka::ERR_NO_ERROR) {
            throw std::runtime_error("Kafka produce failed: " + RdKafka::err2str(resp));
        }
        else {
            sent++;
            if (runTest) {
                sentAvroCdrs.insert(avroCdr);
                auto it = sentAvroCdrs.find(avroCdr);
                assert (it != sentAvroCdrs.end());
            }
        }
    }
    return sent;
}


bool Parser::CompareSentAndConsumedRecords(int64_t startOffset)
{
    std::string errstr;

    std::unique_ptr<RdKafka::Consumer> consumer(RdKafka::Consumer::create(kafkaGlobalConf.get(), errstr));
    if (!consumer) {
      std::cout << "Failed to create consumer: " << errstr << std::endl;
      return false;
    }

    std::unique_ptr<RdKafka::Topic> topic(RdKafka::Topic::create(consumer.get(), kafkaTopic,
                           kafkaTopicConf.get(), errstr));
    if (!topic) {
      std::cout << "Failed to create topic: " << errstr << std::endl;
      return false;
    }

    RdKafka::ErrorCode resp = consumer->start(topic.get(), kafkaPartition, startOffset);
    if (resp != RdKafka::ERR_NO_ERROR) {
      std::cout << "Failed to start consumer: " << RdKafka::err2str(resp) << std::endl;
      return false;
    }

    unsigned32 consumed = 0;
    bool failedToFindCdr = false;
    while(true) {
        std::unique_ptr<RdKafka::Message> message(consumer->consume(topic.get(), kafkaPartition, 5000));
        if (message->err() == RdKafka::ERR__TIMED_OUT) {
            if (consumed > 0) {
                // consider we have read all records
                break;
            }
        }
        if (message->err() == RdKafka::ERR_NO_ERROR) {
            consumed++;
            //std::cout << "Read msg #" << consumed << " at offset " << message->offset() << std::endl;
            PGW_CDR avroCdr;
            std::unique_ptr<avro::InputStream> in(avro::memoryInputStream(
                                         static_cast<uint8_t*>(message->payload()), message->len()));
            avro::DecoderPtr decoder(avro::binaryDecoder());
            decoder->init(*in);
            avro::decode(*decoder, avroCdr);
            auto it = sentAvroCdrs.find(avroCdr);
            if (it != sentAvroCdrs.end()) {
                sentAvroCdrs.erase(it);

            }
            else {
                std::cout << std::endl << "CDR NOT FOUND in sent records:" << std::endl;
                PrintAvroCdrContents(avroCdr);
                failedToFindCdr = true;
            }
        }
        else {
           switch (message->err()) {
              case RdKafka::ERR__PARTITION_EOF:
                break;

              case RdKafka::ERR__UNKNOWN_TOPIC:
              case RdKafka::ERR__UNKNOWN_PARTITION:
                std::cout << "Consume failed: " << message->errstr() << std::endl;
                break;

              default:
                /* Errors */
                std::cout << "Consume failed: " << message->errstr() << std::endl;
            }
        }
        consumer->poll(0);
    }

    consumer->stop(topic.get(), kafkaPartition);
    consumer->poll(1000);
    std::cout << "Consumed " << consumed << " records from Kafka." << std::endl;
    std::cout << "sentAvroCdrs left: " << sentAvroCdrs.size() << std::endl;
    std::cout << (sentAvroCdrs.size() > 0 ? "CompareSentAndConsumedRecords test FAILED" :
                                            "CompareSentAndConsumedRecords test PASSED") << std::endl;
    return !failedToFindCdr && (sentAvroCdrs.size() == 0);
}


void Parser::PrintAvroCdrContents(const PGW_CDR& cdr) const
{
    std::cout << "**** Avro CDR contents: ****" << std::endl
              << "IMSI:" << cdr.IMSI << std::endl
              << "MSISDN: " << cdr.MSISDN << std::endl
             << "IMEI: " << (cdr.IMEI.is_null() ? std::string("<null>") : cdr.IMEI.get_string()) << std::endl
            << "ServedPDPAddress: " << cdr.ServedPDPAddress << std::endl
           << "FirstUsageTime: " << cdr.FirstUsageTime << std::endl
            << "RatingGroup: " << cdr.RatingGroup << std::endl
            << "VolumeUplink: " << cdr.VolumeUplink << std::endl
            << "VolumeDownlink: " << cdr.VolumeDownlink << std::endl
            << "ChargingID: " << cdr.ChargingID << std::endl
            << "SequenceNumber: " <<
               (cdr.SequenceNumber.is_null() ? std::string("<null>") :
                                               std::to_string(cdr.SequenceNumber.get_int())) << std::endl
            << "TimeOfUsage: " << cdr.TimeOfUsage << std::endl;
    for (int i = 0; i < cdr.UserLocationInfo.size(); i++) {
        std::cout << std::to_string(cdr.UserLocationInfo[i]) << " ";
    }
    std::cout << std::endl;
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


Parser::~Parser()
{
    WaitForKafkaQueue();
    RdKafka::wait_destroyed(5000);
}


void Parser::RunTests()
{
    sentAvroCdrs.clear();
    PGW_CDR emptyCdr, cdr, cdr2, cdr3;
    auto res = sentAvroCdrs.insert(emptyCdr);
    //assert(res.second);
    auto it = sentAvroCdrs.find(emptyCdr);
    assert(it != sentAvroCdrs.end());

    cdr.IMSI = 25027000011111;
    cdr.MSISDN = 79083407531;
    cdr.IMEI.set_string("860159159159");
    cdr.ServedPDPAddress = 145789123;
    cdr.FirstUsageTime = 456123789;
    cdr.RatingGroup = 3;
    cdr.VolumeDownlink = 789456321;
    cdr.VolumeUplink = 56321;
    cdr.ChargingID = 87600101;
    cdr.SequenceNumber.set_null();
    //cdr.SequenceNumber.set_int(32);
    cdr.TimeOfUsage = 654;
    cdr.UserLocationInfo = { 23, 254, 0, 12, 54, 68, 1, 255, 23, 254, 0, 12, 54 };
    it = sentAvroCdrs.find(cdr);
    assert(it == sentAvroCdrs.end());

    sentAvroCdrs.insert(cdr);
    it = sentAvroCdrs.find(cdr);
    assert(it != sentAvroCdrs.end());

    cdr2 = cdr;
    it = sentAvroCdrs.find(cdr2);
    assert(it != sentAvroCdrs.end());

    cdr2.ServedPDPAddress = 987654321;
    cdr2.FirstUsageTime = 456123789;
    cdr2.RatingGroup = 1;
    cdr2.VolumeDownlink = 500500;
    cdr2.VolumeUplink = 100100;
    cdr2.ChargingID = 87600101;
    cdr2.SequenceNumber.set_int(21);
    //cdr2.SequenceNumber.set_null();
    cdr2.TimeOfUsage = 700;
    cdr2.UserLocationInfo = { 55, 12, 0, 12, 54, 68, 1, 255, 23 };
    sentAvroCdrs.insert(cdr2);
    it = sentAvroCdrs.find(cdr2);
    assert(it != sentAvroCdrs.end());
    assert(sentAvroCdrs.size() == 3);

    cdr2.UserLocationInfo = { 113, 2, 0, 12, 54, 68, 1, 255, 23, 254 };
    res = sentAvroCdrs.insert(cdr2);
    //assert(res.second);
    it = sentAvroCdrs.find(cdr2);
    assert(it != sentAvroCdrs.end());

    std::unique_ptr<avro::OutputStream> out(avro::memoryOutputStream());
    avro::EncoderPtr encoder(avro::binaryEncoder());
    encoder->init(*out);
    avro::encode(*encoder, cdr);
    encoder->flush();

    std::unique_ptr<avro::InputStream> in(avro::memoryInputStream(*out));
    avro::DecoderPtr decoder(avro::binaryDecoder());
    decoder->init(*in);
    avro::decode(*decoder, cdr3);
    it = sentAvroCdrs.find(cdr3);
    assert(it != sentAvroCdrs.end());

    cdr.SequenceNumber.set_int(1234);
    it = sentAvroCdrs.find(cdr);
    assert(it == sentAvroCdrs.end());

    PGW_CDR cdr4;
    cdr4.ChargingID = 1149416178;
    cdr4.FirstUsageTime = time(nullptr);
    cdr4.IMEI.set_string(std::string("3528110418064903"));
    cdr4.IMSI = 250270100113797;
    cdr4.MSISDN = 79047166648;
    cdr4.SequenceNumber.set_int(357);
    cdr4.ServedPDPAddress = 179861293;
    cdr4.TimeOfUsage = 0;
    cdr4.RatingGroup = 1;
    cdr4.UserLocationInfo = { 24, 82, 240, 114, 0, 213, 82, 240, 114, 0, 0, 17, 1 };
    cdr4.VolumeDownlink = 200;
    cdr4.VolumeUplink = 100;
    sentAvroCdrs.insert(cdr4);
    it = sentAvroCdrs.find(cdr4);
    assert(it != sentAvroCdrs.end());
    sentAvroCdrs.clear();
    std::cout << "Parser tests PASSED. Starting to send sample CDR set. " << std::endl;

    // Sending preset cdrs to consuming service
    std::vector<uint8_t> rawData = EncodeCdr(cdr4);
    RdKafka::ErrorCode resp;
    std::string errstr;
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);

    std::cout << "1st cdr 100/200 sent at " << Utils::Time_t_to_String(time(nullptr)) << ", sleeping 15 sec .... " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));

    std::cout << "Sending 1000 CDRs 200/400 in 10 seconds started at " << Utils::Time_t_to_String(time(nullptr)) << std::endl;
    for (int i = 0; i < 1000; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cdr4.FirstUsageTime = time(nullptr);
        cdr4.VolumeDownlink = 400000;
        cdr4.VolumeUplink = 200000;
        rawData = EncodeCdr(cdr4);
        resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                                   RdKafka::Producer::RK_MSG_COPY,
                                   rawData.data(), rawData.size(), nullptr, 0,
                                   time(nullptr) * 1000 /*milliseconds*/, nullptr);
        assert(resp == RdKafka::ERR_NO_ERROR);
    }

    // send unregistered IMSI
    std::cout << "Sending unregistered IMSI at " << Utils::Time_t_to_String(time(nullptr)) << std::endl;
    cdr4.IMSI = 250270100426286;
    rawData = EncodeCdr(cdr4);
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);

    // registered IMSI again
    std::cout << "Sleeping 35 sec .... " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(35));
    cdr4.IMSI = 250270100113797;
    cdr4.IMEI.set_string("3528110418064903");
    cdr4.FirstUsageTime = time(nullptr);
    cdr4.VolumeDownlink = 600;
    cdr4.VolumeUplink = 400;
    rawData = EncodeCdr(cdr4);
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);

    // new aggregation should start for this CDR
    std::cout << "Cdr 400/600 sent at " << Utils::Time_t_to_String(time(nullptr)) << ", sleeping 15 sec .... " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));
    cdr4.IMSI = 250270100113797;
    cdr4.FirstUsageTime = time(nullptr);
    cdr4.VolumeDownlink = 250;
    cdr4.VolumeUplink = 150;
    rawData = EncodeCdr(cdr4);
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);
    std::cout << "Cdr 150/250 sent at " << Utils::Time_t_to_String(time(nullptr)) << std::endl;

    // another registered IMSI
    std::cout << "Sending another IMSI 710/810 at " << Utils::Time_t_to_String(time(nullptr)) << std::endl;
    cdr4.IMSI = 250270700274584;
    cdr4.FirstUsageTime = time(nullptr);
    cdr4.VolumeDownlink = 810;
    cdr4.VolumeUplink = 710;
    rawData = EncodeCdr(cdr4);
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);

    std::cout << "Sleeping 15 sec .... " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));
    cdr4.IMSI = 250270100113797;
    cdr4.FirstUsageTime = time(nullptr);
    cdr4.VolumeDownlink = 75;
    cdr4.VolumeUplink = 25;
    rawData = EncodeCdr(cdr4);
    resp = kafkaProducer->produce(kafkaTopic, RdKafka::Topic::PARTITION_UA,
                               RdKafka::Producer::RK_MSG_COPY,
                               rawData.data(), rawData.size(), nullptr, 0,
                               time(nullptr) * 1000 /*milliseconds*/, nullptr);
    assert(resp == RdKafka::ERR_NO_ERROR);
    std::cout << "CDR 25/75 sent at " << Utils::Time_t_to_String(time(nullptr)) << std::endl;

    std::cout << "Sample CDR set set is sent to topic: " << kafkaTopic << std::endl
        << "Check aggregation results at consuming service." << std::endl
        << "There must not be unregistered IMSI 250270100426286 in consuming service output" << std::endl << std::endl
        << "Put some files to INPUT_DIR to perform produce/consume tests." << std::endl;
}
