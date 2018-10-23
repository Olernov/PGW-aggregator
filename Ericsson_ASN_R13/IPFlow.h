/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.txt"
 */

#ifndef	_IPFlow_H_
#define	_IPFlow_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IPPort.h"
#include "IPProtocol.h"
#include <NativeInteger.h>
#include <UTF8String.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct IPBinaryAddress;

/* IPFlow */
typedef struct IPFlow {
	struct IPBinaryAddress	*uEAddress	/* OPTIONAL */;
	IPPort_t	*uEPort	/* OPTIONAL */;
	struct IPBinaryAddress	*networkAddress	/* OPTIONAL */;
	IPPort_t	*networkPort	/* OPTIONAL */;
	IPProtocol_t	*protocol	/* OPTIONAL */;
	long	*dataVolumeUplink	/* OPTIONAL */;
	long	*dataVolumeDownlink	/* OPTIONAL */;
	struct listOfURIIPFlow {
		A_SEQUENCE_OF(UTF8String_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *listOfURIIPFlow;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IPFlow_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IPFlow;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "IPAddress.h"

#endif	/* _IPFlow_H_ */
#include <asn_internal.h>