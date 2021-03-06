#pragma once
#include <time.h>
#include <map>
#include "TBCD-STRING.h"
#include "IPAddress.h"
#include "PLMN-Id.h"
#include "TimeStamp.h"
#include "PGWRecord.h"
#include "Common.h"


class Utils
{
 public:
	static unsigned64 TBCDString_to_ULongLong(const TBCD_STRING_t* pTBCDString);
    static std::string TBCDString_to_String(const TBCD_STRING_t* pTBCDString);
	static unsigned32 IPAddress_to_ULong(const IPAddress* pIPAddress);
    static std::string BinIPAddress_to_Text(unsigned32 ipAddress);
	static unsigned32 PLMNID_to_ULong(const PLMN_Id_t* pPLMNID);
	static time_t Timestamp_to_time_t(const TimeStamp_t* pTimestamp);
    static std::string Time_t_to_String(time_t timeT);

    static inline double DiffMinutes(time_t start, time_t end)
        { return (end > start ? static_cast<double>(end - start) / 60 : static_cast<double>(start - end) / 60); }
    static std::map<unsigned32, DataVolumes> SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord);
    static void SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord, DataVolumesMap& dataVolumes);
    static std::string DumpCDRContents(const PGWRecord& pGWRecord);

	static bool RunAllTests();

private:
    static char DecodeTbcdDigit(uint8_t digit);
    static bool TBCDString_to_ULongLong_Test();
    static bool TBCDString_to_String_Test();
	static bool IPAddress_to_ULong_Test();
	static bool PLMNID_to_ULong_Test();
    static bool Timestamp_to_time_t_Test();
    static std::string PrintBinaryDump(const OCTET_STRING* pOctetStr);
	static unsigned32 BCDString_to_ULong(const uint8_t* pOctetStr, int size);
    static bool SumDataVolumesByRatingGroup_Test();
    static void AddToListOfServiceData(PGWRecord* rec, long ratingGroup, const char *timeOfFirstUsage,
                                       long duration, long volumeUplink, long volumeDownlink);
};
