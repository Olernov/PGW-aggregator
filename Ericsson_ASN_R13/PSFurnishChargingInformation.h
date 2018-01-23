/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.txt"
 */

#ifndef	_PSFurnishChargingInformation_H_
#define	_PSFurnishChargingInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "FreeFormatData.h"
#include "FFDAppendIndicator.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PSFurnishChargingInformation */
typedef struct PSFurnishChargingInformation {
	FreeFormatData_t	 pSFreeFormatData;
	FFDAppendIndicator_t	*pSFFDAppendIndicator	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PSFurnishChargingInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PSFurnishChargingInformation;

#ifdef __cplusplus
}
#endif

#endif	/* _PSFurnishChargingInformation_H_ */
#include <asn_internal.h>
