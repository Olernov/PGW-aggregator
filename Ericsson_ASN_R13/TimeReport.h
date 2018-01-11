/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_TimeReport_H_
#define	_TimeReport_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RatingGroup.h"
#include "TimeStamp.h"
#include "DataVolumeGPRS.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TimeReport */
typedef struct TimeReport {
	RatingGroup_t	 ratingGroup;
	TimeStamp_t	 startTime;
	TimeStamp_t	 endTime;
	DataVolumeGPRS_t	*dataVolumeUplink	/* OPTIONAL */;
	DataVolumeGPRS_t	*dataVolumeDownlink	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TimeReport_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TimeReport;

#ifdef __cplusplus
}
#endif

#endif	/* _TimeReport_H_ */
#include <asn_internal.h>
