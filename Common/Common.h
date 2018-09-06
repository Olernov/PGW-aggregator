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
const int secondsToSleepWhenNothingToDo = 1;
const int maxAttemptsToWriteToDB = 5;
const size_t maxAlertMessageLen = 2000;

const time_t notInitialized = 0;

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
    DataVolumes (time_t timeOfFirstUsage, unsigned long duration,
                 unsigned long uplink, unsigned long downlink) :
        timeOfFirstUsage(timeOfFirstUsage),
        duration(duration),
        volumeUplink(uplink),
        volumeDownlink(downlink)
    {}
    time_t timeOfFirstUsage;
    unsigned long duration;
    unsigned long volumeUplink;
    unsigned long volumeDownlink;
};

typedef std::map<unsigned long, DataVolumes> DataVolumesMap;

