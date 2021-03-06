/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GPRSChargingDataTypes"
 * 	found in "./PGW_CDR_Format.asn"
 */

#ifndef	_ChangeOfServiceCondition_H_
#define	_ChangeOfServiceCondition_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RatingGroupId.h"
#include "ChargingRuleBaseName.h"
#include "ResultCode.h"
#include "LocalSequenceNumber.h"
#include "TimeStamp.h"
#include "CallDuration.h"
#include "ServiceConditionChange.h"
#include "DataVolumeGPRS.h"
#include "FailureHandlingContinue.h"
#include "ServiceIdentifier.h"
#include <OCTET_STRING.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EPCQoSInformation;
struct IPAddress;
struct PSFurnishChargingInformation;
struct EventBasedChargingInformation;
struct TimeQuotaMechanism;
struct AFRecordInformation;
struct ServiceSpecificInfo;

/* ChangeOfServiceCondition */
typedef struct ChangeOfServiceCondition {
	RatingGroupId_t	 ratingGroup;
	ChargingRuleBaseName_t	*chargingRuleBaseName	/* OPTIONAL */;
	ResultCode_t	*resultCode	/* OPTIONAL */;
	LocalSequenceNumber_t	*localSequenceNumber	/* OPTIONAL */;
	TimeStamp_t	*timeOfFirstUsage	/* OPTIONAL */;
	TimeStamp_t	*timeOfLastUsage	/* OPTIONAL */;
	CallDuration_t	*timeUsage	/* OPTIONAL */;
	ServiceConditionChange_t	 serviceConditionChange;
	struct EPCQoSInformation	*qoSInformationNeg	/* OPTIONAL */;
	struct IPAddress	*servingNodeAddress	/* OPTIONAL */;
	DataVolumeGPRS_t	*datavolumeFBCUplink	/* OPTIONAL */;
	DataVolumeGPRS_t	*datavolumeFBCDownlink	/* OPTIONAL */;
	TimeStamp_t	 timeOfReport;
	FailureHandlingContinue_t	*failureHandlingContinue	/* OPTIONAL */;
	ServiceIdentifier_t	*serviceIdentifier	/* OPTIONAL */;
	struct PSFurnishChargingInformation	*pSFurnishChargingInformation	/* OPTIONAL */;
	struct aFRecordInformation {
		A_SEQUENCE_OF(struct AFRecordInformation) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *aFRecordInformation;
	OCTET_STRING_t	*userLocationInformation	/* OPTIONAL */;
	struct EventBasedChargingInformation	*eventBasedChargingInformation	/* OPTIONAL */;
	struct TimeQuotaMechanism	*timeQuotaMechanism	/* OPTIONAL */;
	struct serviceSpecificInfo {
		A_SEQUENCE_OF(struct ServiceSpecificInfo) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *serviceSpecificInfo;
	OCTET_STRING_t	*threeGPP2UserLocationInformation	/* OPTIONAL */;
	OCTET_STRING_t	*sponsorIdentity	/* OPTIONAL */;
	OCTET_STRING_t	*applicationServiceProviderIdentity	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChangeOfServiceCondition_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChangeOfServiceCondition;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "EPCQoSInformation.h"
#include "GSNAddress.h"
#include "PSFurnishChargingInformation.h"
#include "EventBasedChargingInformation.h"
#include "TimeQuotaMechanism.h"
#include "AFRecordInformation.h"
#include "ServiceSpecificInfo.h"

#endif	/* _ChangeOfServiceCondition_H_ */
#include <asn_internal.h>
