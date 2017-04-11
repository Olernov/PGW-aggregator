#include "LogWriterOtl.h"

LogWriterOtl::LogWriterOtl()
{
}

void LogWriterOtl::LogOtlException(const std::string& header, const otl_exception& otlEx, short threadIndex)
{
    std::string message = reinterpret_cast<const char*>(otlEx.msg);
    Write(header, threadIndex, error);
    Write(message, threadIndex, error);
    Write(reinterpret_cast<const char*>(otlEx.stm_text), threadIndex, error);
    Write(reinterpret_cast<const char*>(otlEx.var_info), threadIndex, error);
}

