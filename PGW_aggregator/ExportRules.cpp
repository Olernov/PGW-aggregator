#include "ExportRules.h"

bool ExportRules::ReadyForExport(Session_ptr sessionPtr)
{
    return ((sessionPtr.get()->volumeUplinkAggregated > 10000000) ||
            (sessionPtr.get()->volumeDownlinkAggregated > 10000000));


}

