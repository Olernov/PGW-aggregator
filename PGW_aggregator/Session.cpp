#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"


Session::Session(unsigned32 chargingID,
    unsigned64 iMSI,
    unsigned64 mSISDN,
    std::string iMEI,
	std::string accessPointName,
    unsigned32 duration,
    unsigned32 servingNodeIP,
    unsigned32 servingNodePLMNID,
    unsigned32 ratingGroup,
    unsigned32 dataVolumeUplink,
    unsigned32 dataVolumeDownlink,
	time_t firstCDRTime	) :
        chargingID(chargingID),
        iMSI(iMSI),
        mSISDN(mSISDN),
        iMEI(iMEI),
        accessPointName(accessPointName),
        durationAggregated(duration),
        servingNodeIP(servingNodeIP),
        servingNodePLMNID(servingNodePLMNID),
        ratingGroup(ratingGroup),
        volumeUplinkAggregated(dataVolumeUplink),
        volumeDownlinkAggregated(dataVolumeDownlink),
        volumeUplinkExported(0),
        volumeDownlinkExported(0),
        firstCDRTime(firstCDRTime),
        lastUpdateTime(time(NULL)),
        lastExportTime(0),
        lastExportErrorTime(0),
        status(idle)
{}

Session::Session(const Session& rhs) :
    chargingID(rhs.chargingID),
    iMSI(rhs.iMSI),
    mSISDN(rhs.mSISDN),
    iMEI(rhs.iMEI),
    accessPointName(rhs.accessPointName),
    durationAggregated(rhs.durationAggregated),
    servingNodeIP(rhs.servingNodeIP),
    servingNodePLMNID(rhs.servingNodePLMNID),
    ratingGroup(rhs.ratingGroup),
    volumeUplinkAggregated(rhs.volumeUplinkAggregated),
    volumeDownlinkAggregated(rhs.volumeDownlinkAggregated),
    firstCDRTime(rhs.firstCDRTime),
    lastUpdateTime(rhs.lastUpdateTime),
    lastExportTime(rhs.lastExportTime),
    lastExportErrorTime(rhs.lastExportErrorTime),
    status(rhs.status.load())
{}


void Session::UpdateData(unsigned32 volumeUplink, unsigned32 volumeDownlink, unsigned32 duration)
{
    volumeUplinkAggregated += volumeUplink;
    volumeDownlinkAggregated += volumeDownlink;
    duration += duration;
}


void Session::ExportToDB(otl_connect& dbConnect)
{
    if (HaveDataToExport()) {
        otl_stream dbStream;
        dbStream.open(1,
                "call PGW_AGGREGATOR.ExportSession(:charging_id /*bigint*/, :imsi /*bigint*/, :msisdn /*bigint*/, "
                ":imei /*char[20]*/, :access_point_name /*char[64]*/, to_date(:start_time /*char[20]*/, 'yyyymmddhh24miss'), "
                ":duration /*long*/, :serving_node_ip /*long*/, :plmn_id /*long*/, "
                ":rating_group /*long*/, :data_volume_uplink /*bigint*/, :data_volume_downlink /*bigint*/)",
                dbConnect);
            // WARNING: OTL library does not support unsigned long and unsigned long long datatypes
            // for Oracle versions lower than ORA_R11_G2, so we cast to signed64 type.
            // Long long (64-bit) type has range from -9 223 372 036 854 775 808 to 9 223 372 036 854 775 807,
            // (19 digits) so this should be enough to store IMSI and MSISDN.
            dbStream
                    << static_cast<signed64>(chargingID)
                    << static_cast<signed64>(iMSI)
                    << static_cast<signed64>(mSISDN)
                    << iMEI
                    << accessPointName
                    << Utils::Time_t_to_String(firstCDRTime)
                    << static_cast<long>(durationAggregated)
                    << static_cast<long>(servingNodeIP)
                    << static_cast<long>(servingNodePLMNID)
                    << static_cast<long>(ratingGroup)
                    << static_cast<signed64>(volumeUplinkAggregated)
                    << static_cast<signed64>(volumeDownlinkAggregated);

        dbStream.close();


        volumeUplinkExported += volumeUplinkAggregated;
        volumeUplinkAggregated = 0;
        volumeDownlinkExported += volumeDownlinkAggregated;
        volumeDownlinkAggregated = 0;
        durationExported += durationAggregated;
        durationAggregated = 0;
        lastExportTime = time(nullptr);
    }
}


bool Session::HaveDataToExport()
{
    return volumeUplinkAggregated>0 || volumeDownlinkAggregated>0 || durationAggregated>0;
}


void Session::ExportToTestTable(otl_connect& dbConnect, const std::string& filename)
{
    otl_stream dbStream;
    dbStream.open(1,
        "insert into TEST_SESSION_EXPORT (filename , charging_id, imsi, msisdn, "
        "imei, access_point_name, duration, serving_node_ip, serving_node_plmnid, rating_group, "
        "data_volume_uplink, data_volume_downlink, first_cdr_time) values ("
        ":filename /*char[30]*/, :charging_id /*bigint*/, :imsi /*bigint*/, :msisdn /*bigint*/, "
        ":imei /*char[20]*/, :access_point_name /*char[30]*/, :duration /*bigint*/, "
        ":serving_node_ip /*bigint*/, :serving_node_plmnid /*bigint*/, "
        ":rating_group /*bigint*/, :data_volume_uplink /*bigint*/,"
        ":data_volume_downlink /*bigint*/, to_date(:first_cdr_time /*char[20]*/, 'yyyymmddhh24miss'))",
        dbConnect);

    // see comment about casting to signed types at ExportToDB function
    dbStream
            << filename
            << static_cast<signed64>(chargingID)
            << static_cast<signed64>(iMSI)
            << static_cast<signed64>(mSISDN)
            << iMEI
            << accessPointName
            << static_cast<long>(durationAggregated)
            << static_cast<long>(servingNodeIP)
            << static_cast<long>(servingNodePLMNID)
            << static_cast<long>(ratingGroup)
            << static_cast<signed64>(volumeUplinkAggregated)
            << static_cast<signed64>(volumeDownlinkAggregated)
            << Utils::Time_t_to_String(firstCDRTime);
    dbStream.close();
}


void Session::PrintSessionData(std::ostream& outStream)
{
    outStream << "------------------------" << std::endl;
    outStream << "chargingID: " << chargingID << std::endl;
    outStream << "IMSI: " << iMSI << std::endl;
    outStream << "MSISDN: " << mSISDN << std::endl;
    outStream << "IMEI: " << iMEI << std::endl;
    outStream << "APN: " << accessPointName << std::endl;
    outStream << "recOpeningTime: " << ctime(&firstCDRTime) << std::endl;
    outStream << "Duration: " << durationAggregated << std::endl;
    outStream << "servingNodeAddress: " << servingNodeIP << std::endl;
    outStream << "PLMN-ID: " << servingNodePLMNID << std::endl;
    outStream << "rating group: " << ratingGroup << std::endl;
    outStream << "volume uplink: " << volumeUplinkAggregated << std::endl;
    outStream << "volume downlink: " << volumeDownlinkAggregated << std::endl;
}


unsigned32 Session::GetRatingGroup()
{
    return ratingGroup;
}


