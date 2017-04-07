#include "otl_utils.h"
#include "Common.h"

otl_datetime OTL_Utils::Time_t_to_OTL_datetime(time_t timeT)
{
    otl_datetime otlDt;

    tm tmT ;
    localtime_r(&timeT, &tmT);
    otlDt.year = tmT.tm_year + 1900;
    otlDt.month = tmT.tm_mon + 1;
    otlDt.day = tmT.tm_mday;
    otlDt.hour = tmT.tm_hour;
    otlDt.minute = tmT.tm_min;
    otlDt.second = tmT.tm_sec;
    return otlDt;
}


std::string OTL_Utils::OtlExceptionToText(const otl_exception& otlEx)
{
    return std::string(reinterpret_cast<const char*>(otlEx.msg)) + crlf +
        reinterpret_cast<const char*>(otlEx.stm_text)+ crlf +
        reinterpret_cast<const char*>(otlEx.var_info);
}

