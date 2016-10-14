#include <memory>
#include <boost/filesystem.hpp>
#include "Parser.h"
#include "GPRSRecord.h"

using namespace boost;

Parser::Parser(const Aggregator& aggregator) :
	m_aggregator(const_cast<Aggregator&> (aggregator)),
    m_cdrQueue(cdrQueueSize),
	m_printFileContents(false)
{}


void Parser::ProcessDirectory(std::string cdrFilesDirectory, std::string cdrExtension,
							  AggregationTestType testType = noTest)
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
                if (testType != noTest) {
                    std::cout << "Parsing file " << dirIterator->path().filename().string() << "..." << std::endl;
                }
                ParseFile(dirIterator->path().string());
                ProcessCDRQueue();
                if (testType != noTest) {
                    std::cout << "File " << dirIterator->path().filename().string() << " parsed" << std::endl;
                }
				if (testType == perFileTest) {
                    std::cout << "Exporting sessions ..." << std::endl;
					m_aggregator.ExportAllSessionsToDB(dirIterator->path().filename().string());
					m_aggregator.EraseAllSessions();
				}
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
    if(!pgwFile)
        throw std::string ("Unable to open input file ") + filename;

    fseek(pgwFile, 0, SEEK_END);
    unsigned long pgwFileLen = ftell(pgwFile); // длина данных файла (без заголовка)
    std::unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
    fseek(pgwFile, 0, SEEK_SET);
    size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
    fclose(pgwFile);
    if(bytesRead < pgwFileLen) {
        throw std::string("Error reading file ") + filename;
    }

    FILE* fileContents = NULL;
    if (m_printFileContents)
        fileContents = fopen (std::string(filename + "_contents.txt").c_str(), "w");

    asn_dec_rval_t rval;
    unsigned long nextChunk = 0;
    unsigned long recordCount = 0;
    const int maxPGWRecordSize = 2000;
    while(nextChunk < bytesRead) {
        GPRSRecord* pGprsRecord = NULL;
        rval = ber_decode(0, &asn_DEF_GPRSRecord, (void**) &pGprsRecord, buffer.get() + nextChunk, maxPGWRecordSize);
        if(rval.code != RC_OK) {
            if (fileContents)
                fclose(fileContents);
            throw std::string("Error while decoding ASN file. Error code ") + std::to_string(rval.code);
        }

        if (m_printFileContents && fileContents != NULL) {
            asn_fprint(fileContents, &asn_DEF_GPRSRecord, pGprsRecord);

        }
        nextChunk += rval.consumed;
        recordCount++;


        if (!m_cdrQueue.push(pGprsRecord)) {
            // TODO: for fixed size queue it's normal, just means that the queue is full
            // Process it correctly (sleep or something)
            throw std::string("Unable to add CDR to queue");
        }
    }
    if (fileContents)
        fclose(fileContents);

}


void Parser::ProcessCDRQueue()
{
    while (!m_cdrQueue.empty()) {
        GPRSRecord* gprsRecord;
        if (m_cdrQueue.pop(gprsRecord)) {
            m_aggregator.ProcessCDR(gprsRecord->choice.pGWRecord);
            ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
        }
    }
}


void Parser::SetPrintContents(bool printContents)
{
	m_printFileContents = printContents;
}
