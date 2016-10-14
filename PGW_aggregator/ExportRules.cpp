#include "ExportRules.h"

bool ExportRules::ReadyForExport(Session_ptr sessionPtr)
{
    // This is completely test code!
    return ((sessionPtr.get()->m_dataVolumeUplink > 1000000) || (sessionPtr.get()->m_dataVolumeDownlink > 1000000));


}

