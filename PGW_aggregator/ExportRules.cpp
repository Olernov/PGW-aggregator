#include "Utils.h"
#include "ExportRules.h"
#include "LogWriter.h"
#include "Config.h"

extern Config config;
extern LogWriter logWriter;

ExportRules::ExportRules() :
    refreshInProgress(false),
    lastRefresh(0),
    dbConnect(nullptr)
{}

void ExportRules::ReadSettingsFromDatabase(otl_connect& connect)
{
    if (std::atomic_exchange(&refreshInProgress, true)) {
        // another thread acquired the locking flag
        return;
    }
    //std::cout << "Refreshing export rules ..." << std::endl;
    logWriter << "Refreshing export rules ...";
    // store DB connection for further settings auto refresh
    dbConnect = &connect;
    otl_stream stream;
    stream.open(1, "select pcrf_rating_group /*long*/, direction /*long*/, "
                "threshold_mb_home /*long*/, threshold_mb_roaming /*long*/, "
                "threshold_min_home /*long*/, threshold_min_roaming /*long*/ "
                "from Billing.PGW_Rating_Groups", *dbConnect);
    ratingGroups.clear();
    while(!stream.eof()) {
        long ratingGroup;
        std::string direction;
        std::vector<long> thresholds(4);
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
    lastRefresh = time(nullptr);
    //std::cout << "Export rules refreshed" << std::endl;
    logWriter << "Export rules refreshed";
    refreshInProgress = false;
}


bool ExportRules::IsReadyForExport(Session_ptr sessionPtr)
{
    while (refreshInProgress) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    time_t now;
    if (Utils::DiffMinutes(time(&now), lastRefresh) > config.exportRulesRefreshPeriodMin) {
        ReadSettingsFromDatabase(*dbConnect);
    }
    Session* session = sessionPtr.get();
    auto iter = ratingGroups.find(session->ratingGroup);
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

