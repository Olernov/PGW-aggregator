#pragma once
#include <vector>
#include <thread>
#include "Common.h"
#include "Session.h"


struct RatingGroupSetting
{
    static const long NOT_SET = -1;
    long megabytesUplinkHome;
    long megabytesDownlinkHome;
    long megabytesUplinkRoaming;
    long megabytesDownlinkRoaming;
    long minutesUplinkHome;
    long minutesDownlinkHome;
    long minutesUplinkRoaming;
    long minutesDownlinkRoaming;
    RatingGroupSetting():
        megabytesUplinkHome(NOT_SET),
        megabytesDownlinkHome(NOT_SET),
        megabytesUplinkRoaming(NOT_SET),
        megabytesDownlinkRoaming(NOT_SET),
        minutesUplinkHome(NOT_SET),
        minutesDownlinkHome(NOT_SET),
        minutesUplinkRoaming(NOT_SET),
        minutesDownlinkRoaming(NOT_SET)
    {}

    void set(std::string direction, const std::vector<long>& thresholds) {
        if (direction == "uplink") {
            megabytesUplinkHome = thresholds[0];
            megabytesUplinkRoaming = thresholds[1];
            minutesUplinkHome = thresholds[2];
            minutesUplinkRoaming = thresholds[3];
        }
        else if (direction == "downlink") {
            megabytesDownlinkHome = thresholds[0];
            megabytesDownlinkRoaming = thresholds[1];
            minutesDownlinkHome = thresholds[2];
            minutesDownlinkRoaming = thresholds[3];
        }
    }
};

class ExportRules
{
public:
    ExportRules(DBConnect& conn, unsigned long refreshPeriodMin);
    bool IsReadyForExport(Session_ptr sessionPtr);
    void RefreshIfNeeded();
private:
    static const unsigned32 defaultMegabytesHome = 10;
    static const unsigned32 defaultMegabytesRoaming = 0;
    static const unsigned32 defaultMinuteHome = 60;
    static const unsigned32 defaultMinuteRoaming = 60;
    std::map<unsigned32, RatingGroupSetting> ratingGroups;

    bool refreshInProgress;
    unsigned long refreshPeriodMin;
    time_t lastRefresh;
    DBConnect& dbConnect;
};

