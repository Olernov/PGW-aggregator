/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_GPRSRecord_H_
#define	_GPRSRecord_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PGWRecord.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GPRSRecord_PR {
	GPRSRecord_PR_NOTHING,	/* No components present */
	GPRSRecord_PR_pgwRecord
} GPRSRecord_PR;

/* GPRSRecord */
typedef struct GPRSRecord {
	GPRSRecord_PR present;
	union GPRSRecord_u {
		PGWRecord_t	 pgwRecord;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GPRSRecord_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRSRecord;

#ifdef __cplusplus
}
#endif

#endif	/* _GPRSRecord_H_ */
#include <asn_internal.h>
