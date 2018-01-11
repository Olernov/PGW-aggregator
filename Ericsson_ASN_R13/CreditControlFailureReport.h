/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_CreditControlFailureReport_H_
#define	_CreditControlFailureReport_H_


#include <asn_application.h>

/* Including external dependencies */
#include "CreditRequestType.h"
#include "CreditRequestStatus.h"
#include "CreditResultCode.h"
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CreditControlFailureReport */
typedef struct CreditControlFailureReport {
	CreditRequestType_t	 requestType;
	CreditRequestStatus_t	 requestStatus;
	CreditResultCode_t	*resultCode	/* OPTIONAL */;
	long	*ccRequestNumber	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CreditControlFailureReport_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CreditControlFailureReport;

#ifdef __cplusplus
}
#endif

#endif	/* _CreditControlFailureReport_H_ */
#include <asn_internal.h>
