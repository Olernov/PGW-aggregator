#pragma once
#include <map>

const unsigned long long emptyValueULL = -1;
const unsigned long emptyValueUL = -1;

const int MAX_PGW_QUEUES = 16;


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

typedef unsigned long unsigned32;
typedef unsigned long long unsigned64;
typedef signed long long signed64;

struct Config
{
    Config() :
      connectString("aggregator/aggregator@192.168.100.109:1521/irbistst"),
      //connectString("aggregator/AGGREGATOR@(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp)(HOST=idb-vip3)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME=irbis_n)(INSTANCE_NAME=irbis3)))"),
      sessionsNum(16)
    {}

    std::string connectString;
    int sessionsNum;
};
