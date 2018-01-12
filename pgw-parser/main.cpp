#include <iostream>
#include <cassert>
#include <boost/filesystem.hpp>
#include "GPRSRecord.h"
#include "Utils.h"
#include "Common.h"

using namespace boost;

typedef std::vector<filesystem::path> fileList;

unsigned64 imsiFilter = 0;
unsigned64 chargingIdFilter = 0;

void printUsage(const char* programName)
{
    std::cerr << "Usage: " << std::endl << programName << " <input-dir> [-o <output-dir>] [-I imsi] [-C chargingID]"
              << std::endl << std::endl
              << "Options: " << std::endl
              << "  -I imsi         prints contents only for given IMSI" << std::endl
              << "  -C chargingID   prints contents only for given chargingID" << std::endl
                 << std::endl << std::endl;
}

FILE* openOutputFile(std::string filename)
{
    filesystem::path outputFile(filename);
    char nextIndex[3];
    int i = 1;
    while (i < 256 && filesystem::exists(outputFile)) {
        sprintf(nextIndex, "%03d", i);
        outputFile.replace_extension(nextIndex);
        i++;
    }
    if (filesystem::exists(outputFile)) {
        std::cerr << "Choose another output filename to avoid overwiting existing" << std::endl;
        exit(EXIT_FAILURE);
    }
    FILE* f = fopen(outputFile.string().c_str(), "w");
    if (f == nullptr) {
        std::cerr << "Unable to open output file " << outputFile.string() << " for writing." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Writing output to " << outputFile.string() << std::endl;
    return f;
}

fileList ConstructSortedFileList(const std::string& inputDir)
{
    filesystem::path inputPath(inputDir);
    filesystem::directory_iterator endIterator;
    std::vector<filesystem::path> sourceFiles;
    for(filesystem::directory_iterator dirIterator(inputPath); dirIterator != endIterator; dirIterator++) {
        if (filesystem::is_regular_file(dirIterator->status())) {
            sourceFiles.push_back(dirIterator->path());
        }
    }
    std::sort(sourceFiles.begin(), sourceFiles.end());
    return sourceFiles;
}


bool filterSet()
{
    return (imsiFilter != 0 || chargingIdFilter != 0);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage(argv[0]);
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

    std::string outputFilename("./output.txt");
    int opt;

    while ((opt = getopt(argc - 1, argv + 1, "o:I:C:")) != -1) {
      switch (opt) {
      case 'o':
          outputFilename = optarg;
          break;
      case 'I':
          imsiFilter = strtoull(optarg, NULL, 10);
          break;
      case 'C':
          chargingIdFilter = strtoull(optarg, NULL, 10);
          break;
      default:
          printUsage(argv[1]);
          exit(EXIT_FAILURE);
      }
    }

    FILE* fileContents = openOutputFile(outputFilename);
    if (filterSet()) {
        fprintf(fileContents, "Using filter:\n");
        if (imsiFilter != 0) {
            fprintf(fileContents, "  IMSI: %llu\n", imsiFilter);
        }
        if (chargingIdFilter != 0) {
            fprintf(fileContents, "  chargingID: %llu\n", chargingIdFilter);
        }
    }

    fileList sourceFiles = ConstructSortedFileList(inputDir);
    DataVolumesMap dataVolumes;
    for (auto& file : sourceFiles) {
        FILE *pgwFile = fopen(file.string().c_str(), "rb");
        if(!pgwFile) {
            std::cerr << "Unable to open input file " << file.string() << std::endl;
            continue;
        }

        unsigned32 pgwFileLen = filesystem::file_size(file);
        std::unique_ptr<unsigned char[]> buffer (new unsigned char [pgwFileLen]);
        size_t bytesRead = fread(buffer.get(), 1, pgwFileLen, pgwFile);
        fclose(pgwFile);
        if(bytesRead < pgwFileLen) {
            std::cerr << "Error reading file " << file.string() << ". Skipping ..." << std::endl;
            continue;
        }

        fprintf(fileContents, "\n\n----***** Parsing file %s *****----\n\n", file.string().c_str());

        asn_dec_rval_t rval;
        unsigned32 nextChunk = 0;
        const int maxPGWRecordSize = 2000;
        bool decodeError = false;

        while(nextChunk < bytesRead && !decodeError) {
            GPRSRecord* gprsRecord = nullptr;
            rval = ber_decode(0, &asn_DEF_GPRSRecord, (void**) &gprsRecord, buffer.get() + nextChunk, maxPGWRecordSize);
            if(rval.code != RC_OK) {
                std::cerr << "Error while decoding file " << file.string() << ". Error code "
                          << std::to_string(rval.code) << std::endl;
                decodeError = true;
                continue;
            }

            bool filterFit = true;
            if (imsiFilter != 0 &&
                    Utils::TBCDString_to_ULongLong(&gprsRecord->choice.pgwRecord.servedIMSI) != imsiFilter) {
                filterFit = false;
            }
            if (filterFit && chargingIdFilter != 0 && gprsRecord->choice.pgwRecord.chargingID
                        != chargingIdFilter) {
                filterFit = false;
            }
            if (filterFit) {
                if (asn_fprint(fileContents, &asn_DEF_GPRSRecord, gprsRecord) != 0) {
                    std::cerr << "Error while calling asn_fprintf for " << file.string() << std::endl;
                }
                time_t recOpenTime = Utils::Timestamp_to_time_t(&gprsRecord->choice.pgwRecord.recordOpeningTime);
                tm* timeInfo = localtime(&recOpenTime);
                if (fprintf(fileContents, "---- Binary fields decode: ----\n"
                            "\tIMSI:\t\t\t%llu\n"
                            "\tMSISDN:\t\t\t%llu\n"
                            "\trecordOpeningTime:\t%s\n"
                            "--------------------------------------\n\n",
                            Utils::TBCDString_to_ULongLong(&gprsRecord->choice.pgwRecord.servedIMSI),
                            Utils::TBCDString_to_ULongLong(gprsRecord->choice.pgwRecord.servedMSISDN),
                            asctime(timeInfo)) < 0) {
                    std::cerr << "Error while calling fprintf for " << file.string() << std::endl;
                }
                if (filterSet()) {
                    Utils::SumDataVolumesByRatingGroup(gprsRecord->choice.pgwRecord, dataVolumes);
                }
            }
            nextChunk += rval.consumed;
            ASN_STRUCT_FREE(asn_DEF_GPRSRecord, gprsRecord);
        }
        std::cout << "File " << file.string() << " parsed successfully." << std::endl;
    }
    if (filterSet()) {
        fprintf(fileContents, "\n----==== Data volumes summary: ====-----\n");
        for (auto& dv : dataVolumes) {
            fprintf(fileContents, "\tRating group: %lu\n\t\tVolume uplink:\t\t%lu\n\t\tVolume downlink:\t%lu",
                    dv.first, dv.second.volumeUplink, dv.second.volumeDownlink);
        }
    }
    fclose(fileContents);

    return 0;
}
