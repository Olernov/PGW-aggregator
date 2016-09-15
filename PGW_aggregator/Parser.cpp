#include <memory>
#include <boost/filesystem.hpp>
#include "Parser.h"
#include "GPRSRecord.h"

using namespace boost;

Parser::Parser(const Aggregator& aggregator) :
	m_aggregator(const_cast<Aggregator&> (aggregator)),
	m_printFileContents(false)
{}


void Parser::ProcessCDRFile(string filename)
{
	FILE *pgwFile = fopen(filename.c_str(), "rb");
	if(!pgwFile)
		throw string ("Unable to open input file ") + filename;

	fseek(pgwFile, 0, SEEK_END);
	unsigned long pgwFileLen = ftell(pgwFile); // длина данных файла (без заголовка)
	unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
	fseek(pgwFile, 0, SEEK_SET);
	size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
	fclose(pgwFile);
	if(bytesRead < pgwFileLen) {
		throw string("Error reading file ") + filename;
	}

	FILE* fileContents = NULL;
	if (m_printFileContents)
		fileContents = fopen (string(filename + "_contents.txt").c_str(), "w");

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
			throw string("Error while decoding ASN file. Error code ") + std::to_string(rval.code);
		}

		if (m_printFileContents && fileContents != NULL) {
			asn_fprint(fileContents, &asn_DEF_GPRSRecord, pGprsRecord);

		}
		nextChunk += rval.consumed;
		recordCount++;

		//const PGWRecord& pGWRecord = pGprsRecord->choice.pGWRecord;

		m_aggregator.ProcessCDR(pGprsRecord->choice.pGWRecord);

		ASN_STRUCT_FREE(asn_DEF_GPRSRecord, pGprsRecord);
	}
	if (fileContents)
		fclose(fileContents);
}




void Parser::ProcessDirectory(string cdrFilesDirectory, string cdrExtension, bool perFileAggregationTest = false)
{
	filesystem::path cdrPath(cdrFilesDirectory);

	try {
		if (!filesystem::exists(cdrPath))
			throw string("CDR files directory ") + cdrFilesDirectory + " does not exist";

		if (!filesystem::is_directory(cdrPath))
			throw string("Given CDR files directory ") + cdrFilesDirectory + " exists, but is not a directory";

		filesystem::directory_iterator endIterator;
		for(filesystem::directory_iterator dirIterator(cdrPath); dirIterator != endIterator; dirIterator++) {
			if (filesystem::is_regular_file(dirIterator->status()) &&
					dirIterator->path().extension() == cdrExtension) {
				cout << "Parsing file " << dirIterator->path().filename().string() << "..." << endl;
				ProcessCDRFile(dirIterator->path().string());
				cout << "File " << dirIterator->path().filename().string() << "parsed, exporting sessions..." << endl;

				if (perFileAggregationTest) {
					m_aggregator.ExportAllSessionsToDB(dirIterator->path().filename().string());
					m_aggregator.EraseAllSessions();
				}
			}
		}
	}
	catch(const boost::filesystem::filesystem_error& ex) {
		// TODO: process correctly
		throw string(ex.what());
	}
}


void Parser::RunPerFileAggregationTest(string sampleCdrDirectory, string cdrExtension)
{
	ProcessDirectory(sampleCdrDirectory, cdrExtension, true);
}


void Parser::SetPrintContents(bool printContents)
{
	m_printFileContents = printContents;
}
