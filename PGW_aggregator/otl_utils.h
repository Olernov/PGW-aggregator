#pragma once
#include "OTL_Header.h"

class OTL_Utils
{
public:
    static otl_datetime Time_t_to_OTL_datetime(time_t timeT);
    static std::string OtlExceptionToText(const otl_exception &otlEx);
};

