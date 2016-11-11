#include "ExportRules.h"

bool ExportRules::ReadyForExport(Session_ptr sessionPtr)
{
    // This is completely test code!
    return ((sessionPtr.get()->volumeUplinkAggregated > 1000000) || (sessionPtr.get()->volumeDownlinkAggregated > 1000000));


}

