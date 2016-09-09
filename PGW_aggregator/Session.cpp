#include "Session.h"

Session::Session()
{}


Session::Session(unsigned long long iMSI,
	unsigned long long mSISDN,
	unsigned long long iMEI,
	string accessPointName,
	unsigned long duration,
	unsigned long servingNodeIP,
	unsigned long servingNodePLMNID,
	unsigned long ratingGroup,
	unsigned long dataVolumeUplink,
	unsigned long dataVolumeDownlink,
	time_t firstCDRTime	) :
		m_IMSI(iMSI),
		m_MSISDN(mSISDN),
		m_IMEI(iMEI),
		m_accessPointName(accessPointName),
		m_duration(duration),
		m_servingNodeIP(servingNodeIP),
		m_servingNodePLMNID(servingNodePLMNID),
		m_ratingGroup(ratingGroup),
		m_dataVolumeUplink(dataVolumeUplink),
		m_dataVolumeDownlink(dataVolumeDownlink),
		m_firstCDRTime(firstCDRTime),
		m_lastUpdateTime(time(NULL)),
		m_lastExportTime(0),
		m_lastExportErrorTime(0)
{}
