#pragma once
#include <string>
#include <atomic>
#include <memory>
#include "Common.h"
#include "OTL_Header.h"

enum SessionStatus
{
	idle,
	updating,
    readyForExport,
	exporting
};


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
            time_t startTime
		);
    unsigned32 ratingGroup;
    time_t lastUpdateTime;
    time_t lastExportTime;

    //inline unsigned32 GetRatingGroup() const
    //    { return ratingGroup; }
    void ExportToDB(otl_connect& dbConnect);
    void PrintSessionData(std::ostream& outStream);
    void UpdateData(unsigned32 volumeUplinkIncrease, unsigned32 volumeDownlinkIncrease,
                    unsigned32 durationIncrease, time_t newcdrTime);
    inline bool HaveDataToExport()
        { return volumeUplinkAggregated>0 || volumeDownlinkAggregated>0 || endTime>startTime; }
    //inline time_t GetLastUpdateTime() const
    //    { return lastUpdateTime; }
private:
    unsigned32 chargingID;
    unsigned64 iMSI;
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

    //time_t lastExportErrorTime;
    //ExportResult exportErrorCode;


    const time_t notInitialized = 0;
    friend class ExportRules;
};

typedef std::shared_ptr<Session> Session_ptr;
