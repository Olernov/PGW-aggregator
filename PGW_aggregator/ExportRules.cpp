#include "Utils.h"
#include "LogWriterOtl.h"
#include "Config.h"
#include "DBConnect.h"
#include "ExportRules.h"

extern Config config;
extern LogWriterOtl logWriter;

ExportRules::ExportRules(DBConnect &conn, unsigned long refreshPeriodMin) :
    refreshInProgress(false),
    refreshPeriodMin(refreshPeriodMin),
    lastRefresh(notInitialized),
    dbConnect(conn)
{}

void ExportRules::RefreshIfNeeded()
{
    if (Utils::DiffMinutes(time(nullptr), lastRefresh) < refreshPeriodMin) {
        return;
    }
    refreshInProgress = true;
    logWriter << "Refreshing export rules ...";

    try {
        otl_stream stream;
        stream.open(1, "select pcrf_rating_group /*long*/, direction /*long*/, "
                    "threshold_mb_home /*long*/, threshold_mb_roaming /*long*/, "
                    "threshold_min_home /*long*/, threshold_min_roaming /*long*/ "
                    "from Billing.PGW_Rating_Groups", dbConnect);
        ratingGroups.clear();
        const int thresholdParamsCount = 4;
        while(!stream.eof()) {
            long ratingGroup;
            std::string direction;
            std::vector<long> thresholds(thresholdParamsCount);
            stream
                >> ratingGroup
                >> direction
                >> thresholds[0]
                >> thresholds[1]
                >> thresholds[2]
                >> thresholds[3];
            // settings for uplink and downlink are stored separately in the database
            // but we store them togeteher, so we combine them here.
            auto iter = ratingGroups.find(ratingGroup);
            if (iter != ratingGroups.end()) {
               iter->second.set(direction, thresholds);
            }
            else {
                RatingGroupSetting rgs;
                rgs.set(direction, thresholds);
                ratingGroups.insert(std::make_pair(ratingGroup, rgs));
            }
        }
    }
    catch(const otl_exception& ex) {
        logWriter.LogOtlException("**** DB ERROR in main thread while refreshing export rules: ****",
                                  ex, mainThreadIndex);
        dbConnect.reconnect();
    }
    lastRefresh = time(nullptr);
    logWriter << "Export rules refreshed";
    refreshInProgress = false;
}


bool ExportRules::IsReadyForExport(Session* session)
{
    while (refreshInProgress) {
        std::this_thread::sleep_for(std::chrono::seconds(0));
    }

    std::map<unsigned32, RatingGroupSetting>::iterator iter = ratingGroups.find(session->ratingGroup);
    double thresholdUplinkMb, thresholdDownlinkMb, thresholdUplinkMin, thresholdDownlinkMin ;
    if (iter != ratingGroups.end()) {
        if (session->servingNodePLMNID == config.homePlmnID) {
            thresholdUplinkMb = iter->second.megabytesUplinkHome ;
            thresholdDownlinkMb = iter->second.megabytesDownlinkHome ;
            thresholdUplinkMin = iter->second.minutesUplinkHome;
            thresholdDownlinkMin = iter->second.minutesDownlinkHome ;
        }
        else {
            thresholdUplinkMb = iter->second.megabytesUplinkRoaming ;
            thresholdDownlinkMb = iter->second.megabytesDownlinkRoaming ;
            thresholdUplinkMin = iter->second.minutesUplinkRoaming;
            thresholdDownlinkMin = iter->second.minutesDownlinkRoaming ;
        }
    }
    else {
        if (session->servingNodePLMNID == config.homePlmnID) {
            thresholdUplinkMb = thresholdDownlinkMb = defaultMegabytesHome;
        }
        else {
            thresholdUplinkMb = thresholdDownlinkMb = defaultMegabytesRoaming;
        }
    }

    return (static_cast<double>(session->volumeDownlinkAggregated)/megabyteSizeInBytes > thresholdDownlinkMb) ||
            (static_cast<double>(session->volumeUplinkAggregated)/megabyteSizeInBytes > thresholdUplinkMb) ||
            (Utils::DiffMinutes(session->startTime, session->endTime) > thresholdUplinkMin) ||
            (Utils::DiffMinutes(session->startTime, session->endTime) > thresholdDownlinkMin);
}

