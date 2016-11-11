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
            unsigned32 durationAggregated,
            unsigned32 servingNodeIP,
            unsigned32 servingNodePLMNID,
            unsigned32 ratingGroup,
            unsigned32 volumeUplinkAggregated,
            unsigned32 volumeDownlinkAggregated,
            time_t firstCDRTime
		);
    Session(const Session& rhs);

    unsigned32 GetRatingGroup();
    void ExportToDB(otl_connect& dbConnect);
    void ExportToTestTable(otl_connect& dbConnect, const std::string& filename);
    void PrintSessionData(std::ostream& outStream);
    void UpdateData(unsigned32 volumeUplink, unsigned32 volumeDownlink, unsigned32 durationAggregated);
private:
    unsigned32 chargingID;
    unsigned64 iMSI;
    unsigned64 mSISDN;
    std::string iMEI;
    std::string accessPointName;
    unsigned32 durationAggregated;
    unsigned32 durationExported;
    unsigned32 servingNodeIP;
    unsigned32 servingNodePLMNID;
    unsigned32 ratingGroup;
    unsigned64 volumeUplinkAggregated;
    unsigned64 volumeDownlinkAggregated;
    unsigned64 volumeUplinkExported;
    unsigned64 volumeDownlinkExported;
    time_t firstCDRTime;
    time_t lastUpdateTime;
    time_t lastExportTime;
    time_t lastExportErrorTime;
    ExportResult exportErrorCode;
    std::atomic<SessionStatus> status;
    bool HaveDataToExport();

    friend class ExportRules;
};

typedef std::shared_ptr<Session> Session_ptr;
