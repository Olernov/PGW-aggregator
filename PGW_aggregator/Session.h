#pragma once
#include <string>
#include "Common.h"

using namespace std;

class Session
{
public:
	Session();
	Session(unsigned long long m_IMSI,
		unsigned long long m_MSISDN,
		unsigned long long m_IMEI,
		string m_accessPointName,
		unsigned long m_duration,
		unsigned long m_servingNodeIP,
		unsigned long m_servingNodePLMNID,
		unsigned long m_ratingGroup,
		unsigned long m_dataVolumeUplink,
		unsigned long m_dataVolumeDownlink,
		time_t m_firstCDRTime
		);
//private:
	unsigned long long m_IMSI;
	unsigned long long m_MSISDN;
	unsigned long long m_IMEI;
	string m_accessPointName;
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
};
