/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_ManagementExtensions_H_
#define	_ManagementExtensions_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SET_OF.h>
#include <constr_SET_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ManagementExtension;

/* ManagementExtensions */
typedef struct ManagementExtensions {
	A_SET_OF(struct ManagementExtension) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ManagementExtensions_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ManagementExtensions;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ManagementExtension.h"

#endif	/* _ManagementExtensions_H_ */
#include <asn_internal.h>
