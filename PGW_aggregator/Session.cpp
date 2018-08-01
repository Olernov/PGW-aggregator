#include "OTL_Header.h"
#include "Utils.h"
#include "otl_utils.h"
#include "ExportRules.h"
#include "Session.h"
#include "LogWriterOtl.h"
#include "Config.h"

extern LogWriterOtl logWriter;
extern Config config;

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
    time_t cdrTime	,
    ExportRules &er,
    DBConnect &db) :
        chargingID(chargingID),
        iMSI(iMSI),
        ratingGroup(ratingGroup),
        lastUpdateTime(time(nullptr)),
        lastExportTime(notInitialized),
        mSISDN(mSISDN),
        iMEI(iMEI),
        accessPointName(accessPointName),
        servingNodeIP(servingNodeIP),
        servingNodePLMNID(servingNodePLMNID),
        volumeUplinkAggregated(dataVolumeUplink),
        volumeDownlinkAggregated(dataVolumeDownlink),
        volumeUplinkExported(0),
        volumeDownlinkExported(0),
        startTime(cdrTime),
        endTime(cdrTime + duration),
        tollFreeSign(false),
        exportRules(er),
        dbConnect(db)
{
    ExportIfNeeded();
}


void Session::UpdateData(unsigned32 volumeUplinkIncrease, unsigned32 volumeDownlinkIncrease,
                         unsigned32 durationIncrease, time_t newCdrTime)
{
    volumeUplinkAggregated += volumeUplinkIncrease;
    volumeDownlinkAggregated += volumeDownlinkIncrease;
    if (startTime == notInitialized || newCdrTime < startTime) {
        startTime = newCdrTime;
    }
    if (newCdrTime + static_cast<time_t>(durationIncrease) > endTime) {
        endTime = newCdrTime + durationIncrease;
    }
    lastUpdateTime = time(nullptr);
    ExportIfNeeded();
}


void Session::ExportIfNeeded()
{
    if (!tollFreeSign) {
        ForceExport();
    }
    else {
        if (exportRules.IsReadyForExport(this)) {
            ForceExport();
        }
    }
}


void Session::ForceExport()
{
    if (HaveDataToExport()) {
        try {
            otl_stream dbStream;
            dbStream.open(1,
                    "call BILLING.MOBILE_DATA_CHARGER.ExportSession(:charging_id /*bigint,in*/, :imsi /*bigint,in*/, :msisdn /*bigint,in*/, "
                    ":imei /*char[20],in*/, :access_point_name /*char[64],in*/, :start_time /*timestamp,in*/, :end_time /*timestamp,in*/,"
                    ":serving_node_ip /*bigint,in*/, :plmn_id /*long,in*/, "
                    ":rating_group /*long,in*/, :data_volume_uplink /*bigint,in*/, :data_volume_downlink /*bigint,in*/,"
                    ":source_id /*long,in*/) "
                    " into :rate /*double,out*/",
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
                        << OTL_Utils::Time_t_to_OTL_datetime(startTime)
                        << OTL_Utils::Time_t_to_OTL_datetime(endTime)
                        << static_cast<signed64>(servingNodeIP)
                        << static_cast<long>(servingNodePLMNID)
                        << static_cast<long>(ratingGroup)
                        << static_cast<signed64>(volumeUplinkAggregated)
                        << static_cast<signed64>(volumeDownlinkAggregated)
                        << static_cast<long>(config.sourceId);
                double rate = 0;
                dbStream >> rate;
            dbStream.close();

            volumeUplinkExported += volumeUplinkAggregated;
            volumeUplinkAggregated = 0;
            volumeDownlinkExported += volumeDownlinkAggregated;
            volumeDownlinkAggregated = 0;
            startTime = notInitialized;
            endTime = notInitialized;
            lastExportTime = time(nullptr);
            tollFreeSign = rate < tollFreeBound;
        }
        catch(const otl_exception& ex) {
            logWriter << "**** DB ERROR while exporting chargingID " + std::to_string(chargingID) + " ****"
                         + crlf + OTL_Utils::OtlExceptionToText(ex) + crlf + SessionDataDump();
            if (ex.code != deadlockExceptionCode) {
                throw std::runtime_error("**** DB ERROR while exporting ****"
                                     + crlf + OTL_Utils::OtlExceptionToText(ex));
            }
        }
    }
}


std::string Session::SessionDataDump()
{
    return "Session data dump:" + crlf
        + "chargingID: " + std::to_string(chargingID) + crlf
        + "ratingGroup: " + std::to_string(ratingGroup) + crlf
        + "IMSI: " + std::to_string(iMSI) + crlf
        + "MSISDN: " + std::to_string(mSISDN) + crlf
        + "IMEI: " + iMEI + crlf
        + "APN: " + accessPointName + crlf
        + "startTime: " + Utils::Time_t_to_String(startTime) + crlf
        + "endTime: " + Utils::Time_t_to_String(endTime) + crlf
        + "servingNodeAddress: " + std::to_string(servingNodeIP) + crlf
        + "PLMN-ID: " + std::to_string(servingNodePLMNID) + crlf
        + "volume uplink aggregated: " + std::to_string(volumeUplinkAggregated) + crlf
        + "volume downlink aggregated: " + std::to_string(volumeDownlinkAggregated) + crlf
        + "volume uplink exported: " + std::to_string(volumeUplinkExported) + crlf
        + "volume downlink exported: " + std::to_string(volumeDownlinkExported) + crlf
        + "lastUpdateTime: " + Utils::Time_t_to_String(lastUpdateTime) + crlf
        + "lastExportTime: " + Utils::Time_t_to_String(lastExportTime);
}

