#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"


Session::Session(unsigned long chargingID,
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
	time_t firstCDRTime	) :
        m_chargingID(chargingID),
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
		m_lastExportErrorTime(0),
		m_status(idle)
{}

Session::Session(const Session& rhs) :
    m_chargingID(rhs.m_chargingID),
    m_IMSI(rhs.m_IMSI),
    m_MSISDN(rhs.m_MSISDN),
    m_IMEI(rhs.m_IMEI),
    m_accessPointName(rhs.m_accessPointName),
    m_duration(rhs.m_duration),
    m_servingNodeIP(rhs.m_servingNodeIP),
    m_servingNodePLMNID(rhs.m_servingNodePLMNID),
    m_ratingGroup(rhs.m_ratingGroup),
    m_dataVolumeUplink(rhs.m_dataVolumeUplink),
    m_dataVolumeDownlink(rhs.m_dataVolumeDownlink),
    m_firstCDRTime(rhs.m_firstCDRTime),
    m_lastUpdateTime(rhs.m_lastUpdateTime),
    m_lastExportTime(rhs.m_lastExportTime),
    m_lastExportErrorTime(rhs.m_lastExportErrorTime),
    m_status(rhs.m_status.load())
{}


void Session::UpdateData(unsigned long volumeUplink, unsigned long volumeDownlink, unsigned long duration)
{
    m_dataVolumeUplink += volumeUplink;
    m_dataVolumeDownlink += volumeDownlink;
    m_duration += duration;
}


void ExportToDB(otl_connect& dbConnect)
{


}

void Session::ExportToTestTable(otl_connect& dbConnect, const std::string& filename)
{
    otl_stream dbStream;
    dbStream.open(1,
        "insert into TEST_SESSION_EXPORT (filename , charging_id, imsi, msisdn, "
        "imei, access_point_name, duration, serving_node_ip, serving_node_plmnid, rating_group, "
        "data_volume_uplink, data_volume_downlink, first_cdr_time) values ("
        ":filename /*char[30]*/, :charging_id /*bigint*/, :imsi /*bigint*/, :msisdn /*bigint*/, "
        ":imei /*bigint*/, :access_point_name /*char[30]*/, :duration /*bigint*/, "
        ":serving_node_ip /*bigint*/, :serving_node_plmnid /*bigint*/, "
        ":rating_group /*bigint*/, :data_volume_uplink /*bigint*/,"
        ":data_volume_downlink /*bigint*/, to_date(:first_cdr_time /*char[20]*/, 'yyyymmddhh24miss'))",
        dbConnect);

    // WARNING: OTL library does not support unsigned long and unsigned long long datatypes
    // for Oracle versions lower than ORA_R11_G2, so we cast to long long type.
    // Long long (64-bit) type has range from -9 223 372 036 854 775 808 to 9 223 372 036 854 775 807,
    // (19 digits) so this should be enough to store IMSI and IMEI.
    dbStream
            << filename
            << (long long) m_chargingID
            << (long long) m_IMSI
            << (long long) m_MSISDN
            << (long long) m_IMEI
            << m_accessPointName
            << (long long) m_duration
            << (long long) m_servingNodeIP
            << (long long) m_servingNodePLMNID
            << (long long) m_ratingGroup
            << (long long) m_dataVolumeUplink
            << (long long) m_dataVolumeDownlink
            << Utils::Time_t_to_String(m_firstCDRTime);
    dbStream.close();
}


void Session::PrintSessionData(std::ostream& outStream)
{
    outStream << "------------------------" << std::endl;
    outStream << "chargingID: " << m_chargingID << std::endl;
    outStream << "IMSI: " << m_IMSI << std::endl;
    outStream << "MSISDN: " << m_MSISDN << std::endl;
    outStream << "IMEI: " << m_IMEI << std::endl;
    outStream << "APN: " << m_accessPointName << std::endl;
    outStream << "recOpeningTime: " << ctime(&m_firstCDRTime) << std::endl;
    outStream << "Duration: " << m_duration << std::endl;
    outStream << "servingNodeAddress: " << m_servingNodeIP << std::endl;
    outStream << "PLMN-ID: " << m_servingNodePLMNID << std::endl;
    outStream << "rating group: " << m_ratingGroup << std::endl;
    outStream << "volume uplink: " << m_dataVolumeUplink << std::endl;
    outStream << "volume downlink: " << m_dataVolumeDownlink << std::endl;
}


unsigned long Session::GetRatingGroup()
{
    return m_ratingGroup;
}


unsigned long long Session::GetIMSI()
{
    return m_IMSI;
}
