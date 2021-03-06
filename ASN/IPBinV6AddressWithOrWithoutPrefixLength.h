/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GPRSChargingDataTypes"
 * 	found in "./PGW_CDR_Format.asn"
 */

#ifndef	_IPBinV6AddressWithOrWithoutPrefixLength_H_
#define	_IPBinV6AddressWithOrWithoutPrefixLength_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IPBinV6Address.h"
#include "IPBinV6AddressWithPrefixLength.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum IPBinV6AddressWithOrWithoutPrefixLength_PR {
	IPBinV6AddressWithOrWithoutPrefixLength_PR_NOTHING,	/* No components present */
	IPBinV6AddressWithOrWithoutPrefixLength_PR_iPBinV6Address,
	IPBinV6AddressWithOrWithoutPrefixLength_PR_iPBinV6AddressWithPrefix
} IPBinV6AddressWithOrWithoutPrefixLength_PR;

/* IPBinV6AddressWithOrWithoutPrefixLength */
typedef struct IPBinV6AddressWithOrWithoutPrefixLength {
	IPBinV6AddressWithOrWithoutPrefixLength_PR present;
	union IPBinV6AddressWithOrWithoutPrefixLength_u {
		IPBinV6Address_t	 iPBinV6Address;
		IPBinV6AddressWithPrefixLength_t	 iPBinV6AddressWithPrefix;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IPBinV6AddressWithOrWithoutPrefixLength_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IPBinV6AddressWithOrWithoutPrefixLength;

#ifdef __cplusplus
}
#endif

#endif	/* _IPBinV6AddressWithOrWithoutPrefixLength_H_ */
#include <asn_internal.h>
