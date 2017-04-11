#pragma once
#include "OTL_Header.h"
#include "LogWriter.h"

class LogWriterOtl : public LogWriter
{
public:
    LogWriterOtl();
    void LogOtlException(const std::string& header, const otl_exception& otlEx, short threadIndex);
};

