#include <memory>
#include <boost/filesystem.hpp>
#include "Utils.h"
#include "Parser.h"
#include "GPRSRecord.h"

using namespace boost;

extern Config config;

Parser::Parser() :
    stopFlag(false),
    printFileContents(false)
{
    aggregators.reserve(config.sessionsNum);
    for (int i =0; i < config.sessionsNum; i++) {
        aggregators.push_back(Aggregator_ptr(new Aggregator(i)));
    }
}


Parser::~Parser()
{
    SetStopFlag();
}

void Parser::ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension)
{
	filesystem::path cdrPath(cdrFilesDirectory);

	try {
		if (!filesystem::exists(cdrPath))
            throw std::string("CDR files directory ") + cdrFilesDirectory + " does not exist";

		if (!filesystem::is_directory(cdrPath))
            throw std::string("Given CDR files directory ") + cdrFilesDirectory + " exists, but is not a directory";

		filesystem::directory_iterator endIterator;
		for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
			if (filesystem::is_regular_file(dirIterator->status()) &&
					dirIterator->path().extension() == cdrExtension) {
                std::cout << "Parsing file " << dirIterator->path().filename().string() << "..." << std::endl;
                ParseFile(dirIterator->path().string());
                std::cout << "File " << dirIterator->path().filename().string() << " parsed" << std::endl;
			}
		}
	}
	catch(const boost::filesystem::filesystem_error& ex) {
		// TODO: process correctly
        throw std::string(ex.what());
	}
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
    return *aggregators[imsi % config.sessionsNum].get();
}


void Parser::SetPrintContents(bool printContents)
{
    printFileContents = printContents;
}


//void Parser::ExportAllSessionsToDB()
//{
//    for (auto& agr : aggregators) {
//        agr.get()->ExportAllSessionsToDB(filename);
//    }
//}


void Parser::SetStopFlag()
{
    stopFlag = true;
    for (auto& agr : aggregators) {
        agr.get()->SetStopFlag();
    }
}
