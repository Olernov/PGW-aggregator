#include <memory>
#include <boost/filesystem.hpp>
#include "Parser.h"
#include "GPRSRecord.h"

using namespace boost;

Parser::Parser(const Aggregator& aggregator) :
	m_aggregator(const_cast<Aggregator&> (aggregator)),
	m_printFileContents(false)
{}


void Parser::ProcessDirectory(string cdrFilesDirectory, bool perFileAggregationTest = false)
{
	//string filename = "../CDR/b00666877.dat";
	filesystem::path cdrPath(cdrFilesDirectory);

	try {
		if (!filesystem::exists(cdrPath))
			throw string("CDR files directory ") + cdrFilesDirectory + " does not exist";

		if (!filesystem::is_directory(cdrPath))
			throw string("Given CDR files directory ") + cdrFilesDirectory + " exists, but is not a directory";

		filesystem::directory_iterator endIterator;
		for (/*filesystem::directory_entry*/auto&& nextFile : filesystem::directory_iterator(cdrPath)) {
		//for (filesystem::directory_iterator dirIterator(path); dirIterator != endIterator; dirIterator++) {
			FILE *pgwFile = fopen(dirIterator->leaf().c_str(), "rb");
			if(!pgwFile)
				throw string ("Unable to open input file ") + nextFile;

			fseek(pgwFile, 0, SEEK_END);
			unsigned long pgwFileLen = ftell(pgwFile); // длина данных файла (без заголовка)
			//unsigned char* buffer = new unsigned char [pgwFileLen];
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
			if (perFileAggregationTest) {
				m_aggregator.ExportAllSessionsToDB(filename);
			}
		}
	}
	catch(const boost::filesystem::filesystem_error& ex) {
		// TODO: process correctly
		throw string(ex.what());
	}
}


void Parser::RunPerFileAggregationTest(string sampleCdrDirectory)
{
	ProcessDirectory(sampleCdrDirectory, true);
}


void Parser::SetPrintContents(bool printContents)
{
	m_printFileContents = printContents;
}
