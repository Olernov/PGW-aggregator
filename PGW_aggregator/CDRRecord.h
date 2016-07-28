#include <string>
#include "GPRSRecord.h"

using namespace std;

enum ExportErrorCode
{
    OK = 0,
    error = 1
};

typedef unsigned long IPAddressV4;

struct CDRRecord
{
    // long chargingID; // this would be key in map, so we don't include in in struct
    unsigned long long imsi;
    unsigned long long msisdn;
    string imei;
    string accessPointName;
    unsigned long duration;
    IPAddress servingNode;
    string plmnID;
    unsigned long ratingGroup;
    unsigned long long volumeUplink;
    unsigned long long volumeDownlink;
//    time_t aggregationStartTime;
//    time_t lastUpdateTime;
//    time_t lastExportTime;
//    time_t lastExportErrorTime;
//    ExportErrorCode exportErrorCode;
    CDRRecord(GPRSRecord* pGprsRecord);
};
