#pragma once
#include <string>
#include <atomic>
#include <memory>
#include "Common.h"
#include "OTL_Header.h"
#include "DBConnect.h"

//enum SessionStatus
//{
//    idle,
//    updating,
//    readyForExport,
//    exporting
//};

class ExportRules;

class Session
{
public:
    Session(unsigned32 chargingID,
            unsigned64 iMSI,
            unsigned64 mSISDN,
            std::string iMEI,
            std::string accessPointName,
            unsigned32 duration,
            unsigned32 servingNodeIP,
            unsigned32 servingNodePLMNID,
            unsigned32 ratingGroup,
            unsigned32 volumeUplinkAggregated,
            unsigned32 volumeDownlinkAggregated,
            time_t startTime,
            ExportRules& er,
            DBConnect& db
		);
    unsigned32 chargingID;
    unsigned64 iMSI;
    unsigned32 ratingGroup;
    time_t lastUpdateTime;
    time_t lastExportTime;

    void ForceExport();
    std::string SessionDataDump();
    void UpdateData(unsigned32 volumeUplinkIncrease, unsigned32 volumeDownlinkIncrease,
                    unsigned32 durationIncrease, time_t newcdrTime);

private:
    const double tollFreeBound = 0.001;

    unsigned64 mSISDN;
    std::string iMEI;
    std::string accessPointName;
    unsigned32 servingNodeIP;
    unsigned32 servingNodePLMNID;

    unsigned64 volumeUplinkAggregated;
    unsigned64 volumeDownlinkAggregated;
    unsigned64 volumeUplinkExported;
    unsigned64 volumeDownlinkExported;
    time_t startTime;
    time_t endTime;
    bool tollFreeSign;
    ExportRules& exportRules;
    DBConnect& dbConnect;

    void ExportIfNeeded();
    inline bool HaveDataToExport()
        { return volumeUplinkAggregated>0 || volumeDownlinkAggregated>0 || endTime>startTime; }

    friend class ExportRules;
};

typedef std::shared_ptr<Session> Session_ptr;
