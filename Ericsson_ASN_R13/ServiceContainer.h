/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_ServiceContainer_H_
#define	_ServiceContainer_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RatingGroup.h"
#include "ServiceIdentifier.h"
#include "LocalSequenceNumber.h"
#include "ActiveTimeMethod.h"
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct URI;

/* ServiceContainer */
typedef struct ServiceContainer {
	RatingGroup_t	 ratingGroup;
	ServiceIdentifier_t	*serviceIdentifier	/* OPTIONAL */;
	LocalSequenceNumber_t	*localSequenceNumber	/* OPTIONAL */;
	ActiveTimeMethod_t	*method	/* OPTIONAL */;
	long	*inactivity	/* OPTIONAL */;
	long	*resolution	/* OPTIONAL */;
	long	*ccRequestNumber	/* OPTIONAL */;
	long	*serviceSpecificUnits	/* OPTIONAL */;
	struct listOfURI {
		A_SEQUENCE_OF(struct URI) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *listOfURI;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ServiceContainer_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ServiceContainer;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "URI.h"

#endif	/* _ServiceContainer_H_ */
#include <asn_internal.h>
