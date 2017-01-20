#pragma once
#include <map>



typedef unsigned long unsigned32;
typedef unsigned long long unsigned64;
typedef signed long long signed64;

const unsigned long long emptyValueULL = -1;
const unsigned long emptyValueUL = -1;

const int MAX_PGW_QUEUES = 16;
const int MAIN_THREAD_NUM = 0;

const unsigned32 megabyteSizeInBytes = 1024 * 1024;
const int secondsToSleepWhenNothingToDo = 3;
const int MAX_PATH = 1000;

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

struct Config
{
    Config() :
      connectString("aggregator/aggregator@192.168.100.109:1521/irbistst"),
      //connectString("aggregator/AGGREGATOR@(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp)(HOST=idb-vip3)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME=irbis_n)(INSTANCE_NAME=irbis3)))"),
      sessionsNum(16)
    {}

    std::string connectString;
    int sessionsNum;
    static const unsigned32 homePlmnID = 25027;
    static const unsigned32 sessionIdlePeriod = 1; // TODO: debug value, change it
    static const unsigned32 exportRulesRefreshPeriodMin = 1; // TODO: this is for debug!
};
