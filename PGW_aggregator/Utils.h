#pragma once
#include "TBCD-STRING.h"
#include "IPAddress.h"
#include "PLMN-Id.h"

using namespace std;

class Utils
{
 public:
	static unsigned long long TBCDString_to_ULongLong(const TBCD_STRING_t* pTBCDString);
	static bool TBCDString_to_ULongLong_Test();
	static unsigned long IPAddress_to_ULong(const IPAddress* pIPAddress);
	static bool IPAddress_to_ULong_Test();
	static string BinIPAddress_to_Text(unsigned long ipAddress);
	static unsigned long PLMNID_to_ULong(const PLMN_Id_t* pPLMNID);
	static bool PLMNID_to_ULong_Test();
	static string PrintBinaryDump(const OCTET_STRING* pOctetStr);
};
