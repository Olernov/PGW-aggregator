#pragma once
#include <map>



typedef unsigned long unsigned32;
typedef unsigned long long unsigned64;
typedef signed long long signed64;

const unsigned long long emptyValueULL = -1;
const unsigned long emptyValueUL = -1;

const std::string crlf = "\r\n";

const int mainThreadIndex = -1;

const unsigned32 megabyteSizeInBytes = 1024 * 1024;
const int secondsToSleepWhenNothingToDo = 3;
const int maxPath = 1000;

enum ExportResult
{
	erSuccess = 0,
	erDBError
};

enum AggregationTestType
{
	noTest = 0,
	perFileTest = 1,
	totalTest = 2
};

struct DataVolumes {
    DataVolumes (unsigned long uplink, unsigned long downlink) :
        volumeUplink(uplink),
        volumeDownlink(downlink)
    {}
    unsigned long volumeUplink;
    unsigned long volumeDownlink;
};

typedef std::map<unsigned long, DataVolumes> DataVolumesMap;


