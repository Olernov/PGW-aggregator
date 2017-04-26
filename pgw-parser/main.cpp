#include <iostream>
#include <cassert>
#include <boost/filesystem.hpp>
#include "GPRSRecord.h"
#include "Utils.h"
#include "Common.h"

using namespace boost;

void printUsage()
{
    std::cerr << "Usage: " << std::endl << "pgw-parser <input-dir> [<output-dir>]" << std::endl;
}


int main(int argc, const char* argv[])
{
    if (argc < 2) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    std::string inputDir(argv[1]);
    if (!filesystem::exists(inputDir)) {
        std::cerr << "Input directory " << inputDir << " does not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!filesystem::is_directory(inputDir)) {
        std::cerr << inputDir << " is not a directory!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string outputDir;
    if (argc > 2) {
        outputDir = argv[2];
    }
    else {
        outputDir = inputDir;
    }

    filesystem::path inputPath(inputDir);
    filesystem::path outputPath(outputDir);
    filesystem::directory_iterator endIterator;
    for(filesystem::directory_iterator dirIterator(inputPath); dirIterator != endIterator; dirIterator++) {
        if (filesystem::is_regular_file(dirIterator->status())) {
            filesystem::path file = dirIterator->path();
            FILE *pgwFile = fopen(file.string().c_str(), "rb");
            if(!pgwFile) {
                std::cerr << "Unable to open input file " << file.string() << std::endl;
                continue;
            }

            fseek(pgwFile, 0, SEEK_END);
            unsigned32 pgwFileLen = ftell(pgwFile);
            std::unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
            fseek(pgwFile, 0, SEEK_SET);
            size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
            fclose(pgwFile);
            if(bytesRead < pgwFileLen) {
                std::cerr << "Error reading file " << file.string() << ". Skipping ..." << std::endl;
                continue;
            }

            // TODO: add output dir
            filesystem::path outputFile = outputPath / file.filename();
            outputFile.replace_extension(".txt");
            //std::string outputFilename = file.string() + ".txt";
            const char* s = outputFile.string().c_str();
            FILE* fileContents = fopen(outputFile.string().c_str(), "w");
            if (!fileContents) {
                std::cerr << "Unable to open output file " << outputFile.filename() << " for writing." << std::endl;
                continue;
            }

            asn_dec_rval_t rval;
            unsigned32 nextChunk = 0;
            const int maxPGWRecordSize = 2000;
            while(nextChunk < bytesRead) {
                GPRSRecord* gprsRecord = nullptr;
                rval = ber_decode(0, &asn_DEF_GPRSRecord, (void**) &gprsRecord, buffer.get() + nextChunk, maxPGWRecordSize);
                if(rval.code != RC_OK) {
                    fclose(fileContents);
                    std::cerr << "Error while decoding file " << file.string() << ". Error code "
                              << std::to_string(rval.code) << std::endl;
                    continue;
                }
                if (asn_fprint(fileContents, &asn_DEF_GPRSRecord, gprsRecord) != 0) {
                    std::cerr << "Error while calling asn_fprintf for " << file.string() << std::endl;
                }
                nextChunk += rval.consumed;
                ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
            }
            std::cout << "File " << file.string() << " parsed successfully." << std::endl;
            fclose(fileContents);
        }
    }
    return 0;
}
