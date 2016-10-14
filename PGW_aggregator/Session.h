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
    Session(unsigned long chargingID,
            unsigned long long iMSI,
            unsigned long long mSISDN,
            unsigned long long iMEI,
            std::string accessPointName,
            unsigned long duration,
            unsigned long servingNodeIP,
            unsigned long servingNodePLMNID,
            unsigned long ratingGroup,
            unsigned long dataVolumeUplink,
            unsigned long dataVolumeDownlink,
            time_t firstCDRTime
		);
    Session(const Session& rhs);

    unsigned long GetRatingGroup();
    unsigned long long GetIMSI();
    void ExportToDB(otl_connect& dbConnect);
    void ExportToTestTable(otl_connect& dbConnect, const std::string& filename);
    void PrintSessionData(std::ostream& outStream);
    void UpdateData(unsigned long volumeUplink, unsigned long volumeDownlink, unsigned long duration);
private:
    unsigned long m_chargingID;
    unsigned long long m_IMSI;
	unsigned long long m_MSISDN;
	unsigned long long m_IMEI;
	std::string m_accessPointName;
	unsigned long m_duration;
	unsigned long m_servingNodeIP;
	unsigned long m_servingNodePLMNID;
	unsigned long m_ratingGroup;
	unsigned long m_dataVolumeUplink;
	unsigned long m_dataVolumeDownlink;
	time_t m_firstCDRTime;
	time_t m_lastUpdateTime;
	time_t m_lastExportTime;
	time_t m_lastExportErrorTime;
	ExportResult m_exportErrorCode;
	std::atomic<SessionStatus> m_status;

    friend class ExportRules;
};

typedef std::shared_ptr<Session> Session_ptr;
