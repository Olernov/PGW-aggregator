#include <iostream>
#include <string>
#include "Utils.h"

unsigned long long Utils::TBCDString_to_ULongLong(const TBCD_STRING_t* pTBCDString)
{
	//TBCD-STRING ::= OCTET STRING
	//	-- This type (Telephony Binary Coded Decimal String) is used to
	//	-- represent several digits from 0 through 9, *, #, a, b, c, two
	//	-- digits per octet, each digit encoded 0000 to 1001 (0 to 9),
	//	-- 1010 (*), 1011 (#), 1100 (a), 1101 (b) or 1110 (c); 1111 used
	//	-- as filler when there is an odd number of digits.

	//	-- bits 8765 of octet n encoding digit 2n
	//	-- bits 4321 of octet n encoding digit 2(n-1) +1

	if (!pTBCDString)
		return 0;
	unsigned long long res = 0;
    for(int i = 0; i < pTBCDString->size; i++) {
		uint8_t next = pTBCDString->buf[i] & 0x0F;
		if (next >= 0 && next <= 9) {
			// ignore non numeric characters
			res *= 10;
			res += next;
		}
		next = (pTBCDString->buf[i] & 0xF0) >> 4;
		if (next >= 0 && next <= 9) {
			// ignore non numeric characters
			res *= 10;
			res += next;
		}
    }
    return res;
}


bool Utils::TBCDString_to_ULongLong_Test()
{
	const int test_string_len = 8;
	const char* test_strings[] =
		{ "\x52\x00\x70\x72\x86\x45\x38\xf0",
		  "\x52\x00\x70\x72\x86\x45\x38\x80",
		  "\x52\xa0\x7b\xc2\x89\x45\x38\x10",
		  "\x00\xab\xcd\x72\xf1\xd3\x38\xf0",
		  NULL };
	const unsigned long long correct_results [] =
		{ 250007276854830ULL,
		  2500072768548308ULL,
		  2507298548301ULL,
		  2713830ULL,
		  0ULL };
	bool success = true;

	int i = 0;
	for (const char* testStr : test_strings) {
		TBCD_STRING_t* pTBCDString = OCTET_STRING_new_fromBuf(&asn_DEF_TBCD_STRING,
			testStr, test_string_len);
		unsigned long long res = TBCDString_to_ULongLong(pTBCDString);
		if (res != correct_results[i]) {
			cout << "TBCDString_to_LongLong_Test #" << i + 1 << " FAILED. correct result: " << correct_results[i]
			   << " but returned " << res << endl;
			success = false;
		}
		else {
			cout << "TBCDString_to_LongLong_Test #" << i + 1 << " PASSED. " << endl;
		}
		ASN_STRUCT_FREE(asn_DEF_TBCD_STRING, pTBCDString);
		i++;
	}
	return success;
}


unsigned long Utils::IPAddress_to_ULong(const IPAddress* pIPAddress)
{
	if (!pIPAddress)
		throw string("Empty IP address given (pIPAddress == NULL)");
	unsigned long ip_addr_ulong = 0;
	string textIP;
	size_t prev_pos = 0, next_pos = 0;
	unsigned int next_octet; // not uint8_t cause we will control 8-bit overflow
	uint8_t num_octets;
	switch(pIPAddress->present) {
	case IPAddress_PR_iPBinaryAddress:
		switch (pIPAddress->choice.iPBinaryAddress.present) {
		case IPBinaryAddress_PR_iPBinV4Address:
			if (pIPAddress->choice.iPBinaryAddress.choice.iPBinV4Address.size > 4)
				throw string("IPv4 address consists more than of 4 bytes");
			for (int i =0; i < pIPAddress->choice.iPBinaryAddress.choice.iPBinV4Address.size; i++) {
				ip_addr_ulong <<= 8;
				ip_addr_ulong |= pIPAddress->choice.iPBinaryAddress.choice.iPBinV4Address.buf[i];
			}
			break;
		case IPBinaryAddress_PR_iPBinV6Address:
			//if ()
			throw string("IPv6 parsing not implemented.");
		case IPBinaryAddress_PR_NOTHING:
			return 0UL;
		}
		break;
	case IPAddress_PR_iPTextRepresentedAddress:
		switch (pIPAddress->choice.iPTextRepresentedAddress.present) {
		case IPTextRepresentedAddress_PR_iPTextV4Address:
			textIP = (const char*) pIPAddress->choice.iPTextRepresentedAddress.choice.iPTextV4Address.buf;
			next_octet = 0;
			num_octets = 0;
			for (char c : textIP) {
				if (c >= '0' && c <= '9') {
					next_octet *= 10;
					next_octet += c - '0';
					if (next_octet > 0xFF)
						throw string("Wrong text represented IP address given: ") + textIP;
				}
				else if (c == '.') {
					ip_addr_ulong <<= 8;
					ip_addr_ulong |= next_octet;
					if(++num_octets > 4)
						throw string("Wrong text represented IP address given: ") + textIP;
					next_octet = 0;
				}
				else
					throw string("Wrong text represented IP address given: ") + textIP;
			}
			ip_addr_ulong <<= 8;
			ip_addr_ulong |= next_octet;
			if(++num_octets != 4)
				throw string("Wrong text represented IP address given: ") + textIP;
			break;
		case IPTextRepresentedAddress_PR_iPTextV6Address:
			throw string("IPv6 parsing not implemented.");
		}
		break;
	case IPAddress_PR_NOTHING:
		throw string("Empty IP address given (pIPAddress->present == IPAddress_PR_NOTHING)");
	}
	return ip_addr_ulong;
}


bool Utils::IPAddress_to_ULong_Test()
{
	const IPAddress test_ips[] = {
		{IPAddress_PR_iPBinaryAddress, {IPBinaryAddress_PR_iPBinV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPBinV4Address, "\xB9\x06\x50\x0A", 4)}}},
		{IPAddress_PR_iPBinaryAddress, {IPBinaryAddress_PR_iPBinV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPBinV4Address, "\xFF\xFF\xFF\xFF", 4)}}},
		{IPAddress_PR_iPBinaryAddress, {IPBinaryAddress_PR_iPBinV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPBinV4Address, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8)}}},
		{IPAddress_PR_iPBinaryAddress, {IPBinaryAddress_PR_iPBinV6Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPBinV4Address, "\xFF\xFF\xFF\xFF", 4)}}},
		{IPAddress_PR_NOTHING, {IPBinaryAddress_PR_iPBinV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPBinV4Address, "\xB9\x06\x50\x0A", 4)}}},
		{IPAddress_PR_iPTextRepresentedAddress, { .iPTextRepresentedAddress = {IPTextRepresentedAddress_PR_iPTextV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPTextRepresentedAddress, "172.18.1.0", 10)}}}},
		{IPAddress_PR_iPTextRepresentedAddress, { .iPTextRepresentedAddress = {IPTextRepresentedAddress_PR_iPTextV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPTextRepresentedAddress, "1.18.99.255", 11)}}}},
		{IPAddress_PR_iPTextRepresentedAddress, { .iPTextRepresentedAddress = {IPTextRepresentedAddress_PR_iPTextV4Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPTextRepresentedAddress, "11899.255", 9)}}}},
		{IPAddress_PR_iPTextRepresentedAddress, { .iPTextRepresentedAddress = {IPTextRepresentedAddress_PR_iPTextV6Address,
										{*OCTET_STRING_new_fromBuf(&asn_DEF_IPTextRepresentedAddress, "172.18.1.0", 10)}}}}
	};
	const unsigned long long exception_sign = 0xFFFFFFFFFFFFFFFF;
	const unsigned long long correct_results [] =
		{ 0xB906500A, 0xFFFFFFFF, exception_sign, exception_sign, exception_sign, 0xAC120100, 0x011263FF, exception_sign, exception_sign };
	bool success = true;

	int i = 0;
	for (/*int i = 0; i < test_num; i++*/const IPAddress& testIPAddr : test_ips) {
		try {
			unsigned long res = IPAddress_to_ULong(&testIPAddr);
			if (res != correct_results[i]) {
				cout << "IPAddress_to_ULong_Test #" << i + 1 << " FAILED. correct result: " << correct_results[i]
				   << " but returned " << res << endl;
				success = false;
			}
			else {
				cout << "IPAddress_to_ULong_Test #" << i + 1 << " PASSED. " << endl;
			}
		}
		catch(const string& exc_text) {
			if (correct_results[i] == exception_sign) {
				cout << "IPAddress_to_ULong_Test #" << i + 1 << " PASSED (exception caught: " << exc_text << ") " << endl;
			}
			else {
				cout << "IPAddress_to_ULong_Test #" << i + 1 << " FAILED. (exception caught: " << exc_text << ") " << endl;
			}
		}
		i++;
	}
	return success;
}


string Utils::BinIPAddress_to_Text(unsigned long ipAddress)
{
	std::string textIP;
	while(ipAddress > 0) {
		uint8_t next_octet = ipAddress & 0xFF;
		if (!textIP.empty())
			textIP = '.' + textIP;
		textIP = to_string(next_octet) + textIP;
		ipAddress >>= 8;
	}
	return textIP;
}


string Utils::PrintBinaryDump(const OCTET_STRING* pOctetStr)
{
	const size_t buffer_size = 256;
	char buffer[buffer_size];
	int i = 0;
	for (; i < pOctetStr->size; i++) {
		if (3 * (i + 1) >= buffer_size - 1)
			break;
		sprintf(&buffer[3 * i], "%02x ", pOctetStr->buf[i]);
	}
	buffer[3 * (i + 1)] = '\0';
	return string(buffer);

}

unsigned long Utils::PLMNID_to_ULong(const PLMN_Id_t* pPLMNID)
{
	//PLMN-Id ::= OCTET STRING (SIZE (3))
	//	-- The internal structure is defined as follows:
	//	-- octet 1 bits 4321	Mobile Country Code 1st digit
	//	--         bits 8765	Mobile Country Code 2nd digit
	//	-- octet 2 bits 4321	Mobile Country Code 3rd digit
	//	--         bits 8765	Mobile Network Code 3rd digit
	//	--			or filler (1111) for 2 digit MNCs
	//	-- octet 3 bits 4321	Mobile Network Code 1st digit
	//	--         bits 8765	Mobile Network Code 2nd digit

	if (pPLMNID->size != 3) {
		throw string("Wrong PLMN-ID given: ") + PrintBinaryDump(pPLMNID);
	}
	unsigned long plmnID =
		(pPLMNID->buf[0] & 0x0F) * 10000 + ((pPLMNID->buf[0] & 0xF0) >> 4) * 1000 +
		(pPLMNID->buf[1] & 0x0F) * 100// MCC
		+  (pPLMNID->buf[2] & 0x0F) * 10 + ((pPLMNID->buf[2] & 0xF0) >> 4); // MNC

	if (pPLMNID->buf[1] & 0xF0 != 0xF0) { // if 3rd digit of MNC is present
		plmnID *= 10;
		plmnID += ((pPLMNID->buf[1] & 0xF0) >> 4);
	}
	return plmnID;
}


bool Utils::PLMNID_to_ULong_Test()
{
	const int test_string_len = 8;
	const unsigned long exception_sign = 0xFFFFFFFF;
	const char* test_strings[] =
		{ "\x52\xF0\x27\x72\x86\x45\x38\xf0",
		  "\x52\x00\x70\x72\x86\x45\x38\x80",
		  "\x52\xa0\x7b\xc2\x89\x45\x38\x10",
		  "\x00\xab\xcd\x72\xf1\xd3\x38\xf0",
		  NULL };
	const unsigned long long correct_results [] =
		{ 250007276854830ULL,
		  2500072768548308ULL,
		  2507298548301ULL,
		  2713830ULL,
		  0ULL };
	bool success = true;

	int i = 0;
	for (const char* testStr : test_strings) {
		TBCD_STRING_t* pTBCDString = OCTET_STRING_new_fromBuf(&asn_DEF_TBCD_STRING,
			testStr, test_string_len);
		unsigned long long res = TBCDString_to_ULongLong(pTBCDString);
		if (res != correct_results[i]) {
			cout << "TBCDString_to_LongLong_Test #" << i + 1 << " FAILED. correct result: " << correct_results[i]
			   << " but returned " << res << endl;
			success = false;
		}
		else {
			cout << "TBCDString_to_LongLong_Test #" << i + 1 << " PASSED. " << endl;
		}
		ASN_STRUCT_FREE(asn_DEF_TBCD_STRING, pTBCDString);
		i++;
	}
	return success;

}
