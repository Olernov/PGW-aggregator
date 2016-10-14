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
	static unsigned long long TBCDString_to_ULongLong(const TBCD_STRING_t* pTBCDString);
	static unsigned long IPAddress_to_ULong(const IPAddress* pIPAddress);
    static std::string BinIPAddress_to_Text(unsigned long ipAddress);
	static unsigned long PLMNID_to_ULong(const PLMN_Id_t* pPLMNID);
	static time_t Timestamp_to_time_t(const TimeStamp_t* pTimestamp);
    static std::string Time_t_to_String(time_t timeT);
    static std::map<unsigned long, DataVolumes> SumDataVolumesByRatingGroup(const PGWRecord& pGWRecord);
	static bool RunAllTests();
private:
	static bool TBCDString_to_ULongLong_Test();
	static bool IPAddress_to_ULong_Test();
	static bool PLMNID_to_ULong_Test();
    static std::string PrintBinaryDump(const OCTET_STRING* pOctetStr);
	static unsigned long BCDString_to_ULong(const uint8_t* pOctetStr, int size);
    bool SumDataVolumesByRatingGroup_Test();

};
