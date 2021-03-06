/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GPRSChargingDataTypes"
 * 	found in "./PGW_CDR_Format.asn"
 */

#ifndef	_ChangeLocation_H_
#define	_ChangeLocation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "LocationAreaCode.h"
#include "RoutingAreaCode.h"
#include "CellId.h"
#include "TimeStamp.h"
#include "PLMN-Id.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChangeLocation */
typedef struct ChangeLocation {
	LocationAreaCode_t	 locationAreaCode;
	RoutingAreaCode_t	 routingAreaCode;
	CellId_t	*cellId	/* OPTIONAL */;
	TimeStamp_t	 changeTime;
	PLMN_Id_t	*mCC_MNC	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChangeLocation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChangeLocation;

#ifdef __cplusplus
}
#endif

#endif	/* _ChangeLocation_H_ */
#include <asn_internal.h>
