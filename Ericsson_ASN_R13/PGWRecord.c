/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#include "PGWRecord.h"

static asn_TYPE_member_t asn_MBR_servingNodeAddress_6[] = {
	{ ATF_POINTER, 0, 0,
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_GSNAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_servingNodeAddress_tags_6[] = {
	(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_servingNodeAddress_specs_6 = {
	sizeof(struct servingNodeAddress),
	offsetof(struct servingNodeAddress, _asn_ctx),
	2,	/* XER encoding is XMLValueList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_servingNodeAddress_6 = {
	"servingNodeAddress",
	"servingNodeAddress",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_servingNodeAddress_tags_6,
	sizeof(asn_DEF_servingNodeAddress_tags_6)
		/sizeof(asn_DEF_servingNodeAddress_tags_6[0]) - 1, /* 1 */
	asn_DEF_servingNodeAddress_tags_6,	/* Same as above */
	sizeof(asn_DEF_servingNodeAddress_tags_6)
		/sizeof(asn_DEF_servingNodeAddress_tags_6[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_servingNodeAddress_6,
	1,	/* Single element */
	&asn_SPC_servingNodeAddress_specs_6	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_listOfTrafficVolumes_12[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ChangeOfCharCondition,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_listOfTrafficVolumes_tags_12[] = {
	(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_listOfTrafficVolumes_specs_12 = {
	sizeof(struct listOfTrafficVolumes),
	offsetof(struct listOfTrafficVolumes, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_listOfTrafficVolumes_12 = {
	"listOfTrafficVolumes",
	"listOfTrafficVolumes",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_listOfTrafficVolumes_tags_12,
	sizeof(asn_DEF_listOfTrafficVolumes_tags_12)
		/sizeof(asn_DEF_listOfTrafficVolumes_tags_12[0]) - 1, /* 1 */
	asn_DEF_listOfTrafficVolumes_tags_12,	/* Same as above */
	sizeof(asn_DEF_listOfTrafficVolumes_tags_12)
		/sizeof(asn_DEF_listOfTrafficVolumes_tags_12[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_listOfTrafficVolumes_12,
	1,	/* Single element */
	&asn_SPC_listOfTrafficVolumes_specs_12	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_listOfServiceData_32[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ChangeOfServiceCondition,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_listOfServiceData_tags_32[] = {
	(ASN_TAG_CLASS_CONTEXT | (34 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_listOfServiceData_specs_32 = {
	sizeof(struct listOfServiceData),
	offsetof(struct listOfServiceData, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_listOfServiceData_32 = {
	"listOfServiceData",
	"listOfServiceData",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_listOfServiceData_tags_32,
	sizeof(asn_DEF_listOfServiceData_tags_32)
		/sizeof(asn_DEF_listOfServiceData_tags_32[0]) - 1, /* 1 */
	asn_DEF_listOfServiceData_tags_32,	/* Same as above */
	sizeof(asn_DEF_listOfServiceData_tags_32)
		/sizeof(asn_DEF_listOfServiceData_tags_32[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_listOfServiceData_32,
	1,	/* Single element */
	&asn_SPC_listOfServiceData_specs_32	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_servingNodeType_34[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_ServingNodeType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_servingNodeType_tags_34[] = {
	(ASN_TAG_CLASS_CONTEXT | (35 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_servingNodeType_specs_34 = {
	sizeof(struct servingNodeType),
	offsetof(struct servingNodeType, _asn_ctx),
	1,	/* XER encoding is XMLValueList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_servingNodeType_34 = {
	"servingNodeType",
	"servingNodeType",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_servingNodeType_tags_34,
	sizeof(asn_DEF_servingNodeType_tags_34)
		/sizeof(asn_DEF_servingNodeType_tags_34[0]) - 1, /* 1 */
	asn_DEF_servingNodeType_tags_34,	/* Same as above */
	sizeof(asn_DEF_servingNodeType_tags_34)
		/sizeof(asn_DEF_servingNodeType_tags_34[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_servingNodeType_34,
	1,	/* Single element */
	&asn_SPC_servingNodeType_specs_34	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_PGWRecord_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, recordType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RecordType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"recordType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, servedIMSI),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servedIMSI"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, p_GWAddress),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_GSNAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"p-GWAddress"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, chargingID),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChargingID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"chargingID"
		},
	{ ATF_POINTER, 5, offsetof(struct PGWRecord, servingNodeAddress),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		0,
		&asn_DEF_servingNodeAddress_6,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servingNodeAddress"
		},
	{ ATF_POINTER, 4, offsetof(struct PGWRecord, accessPointNameNI),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AccessPointNameNI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"accessPointNameNI"
		},
	{ ATF_POINTER, 3, offsetof(struct PGWRecord, pdpPDNType),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDPType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pdpPDNType"
		},
	{ ATF_POINTER, 2, offsetof(struct PGWRecord, servedPDPPDNAddress),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_PDPAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servedPDPPDNAddress"
		},
	{ ATF_POINTER, 1, offsetof(struct PGWRecord, dynamicAddressFlag),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DynamicAddressFlag,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dynamicAddressFlag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, listOfTrafficVolumes),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		0,
		&asn_DEF_listOfTrafficVolumes_12,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"listOfTrafficVolumes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, recordOpeningTime),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"recordOpeningTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, duration),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallDuration,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"duration"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, causeForRecClosing),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CauseForRecClosing,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"causeForRecClosing"
		},
	{ ATF_POINTER, 6, offsetof(struct PGWRecord, recordSequenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"recordSequenceNumber"
		},
	{ ATF_POINTER, 5, offsetof(struct PGWRecord, nodeID),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NodeID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nodeID"
		},
	{ ATF_POINTER, 4, offsetof(struct PGWRecord, recordExtensions),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ManagementExtensions,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"recordExtensions"
		},
	{ ATF_POINTER, 3, offsetof(struct PGWRecord, localSequenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocalSequenceNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"localSequenceNumber"
		},
	{ ATF_POINTER, 2, offsetof(struct PGWRecord, apnSelectionMode),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_APNSelectionMode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apnSelectionMode"
		},
	{ ATF_POINTER, 1, offsetof(struct PGWRecord, servedMSISDN),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MSISDN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servedMSISDN"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, chargingCharacteristics),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChargingCharacteristics,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"chargingCharacteristics"
		},
	{ ATF_POINTER, 8, offsetof(struct PGWRecord, chChSelectionMode),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChChSelectionMode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"chChSelectionMode"
		},
	{ ATF_POINTER, 7, offsetof(struct PGWRecord, iMSsignalingContext),
		(ASN_TAG_CLASS_CONTEXT | (25 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"iMSsignalingContext"
		},
	{ ATF_POINTER, 6, offsetof(struct PGWRecord, servinggNodePLMNIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (27 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PLMN_Id,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servinggNodePLMNIdentifier"
		},
	{ ATF_POINTER, 5, offsetof(struct PGWRecord, pSFurnishChargingInformation),
		(ASN_TAG_CLASS_CONTEXT | (28 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PSFurnishChargingInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pSFurnishChargingInformation"
		},
	{ ATF_POINTER, 4, offsetof(struct PGWRecord, servedIMEISV),
		(ASN_TAG_CLASS_CONTEXT | (29 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMEI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servedIMEISV"
		},
	{ ATF_POINTER, 3, offsetof(struct PGWRecord, rATType),
		(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RATType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rATType"
		},
	{ ATF_POINTER, 2, offsetof(struct PGWRecord, mSTimeZone),
		(ASN_TAG_CLASS_CONTEXT | (31 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MSTimeZone,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mSTimeZone"
		},
	{ ATF_POINTER, 1, offsetof(struct PGWRecord, userLocationInformation),
		(ASN_TAG_CLASS_CONTEXT | (32 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"userLocationInformation"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PGWRecord, listOfServiceData),
		(ASN_TAG_CLASS_CONTEXT | (34 << 2)),
		0,
		&asn_DEF_listOfServiceData_32,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"listOfServiceData"
		},
	{ ATF_POINTER, 8, offsetof(struct PGWRecord, servingNodeType),
		(ASN_TAG_CLASS_CONTEXT | (35 << 2)),
		0,
		&asn_DEF_servingNodeType_34,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servingNodeType"
		},
	{ ATF_POINTER, 7, offsetof(struct PGWRecord, p_GWPLMNIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (37 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PLMN_Id,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"p-GWPLMNIdentifier"
		},
	{ ATF_POINTER, 6, offsetof(struct PGWRecord, startTime),
		(ASN_TAG_CLASS_CONTEXT | (38 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"startTime"
		},
	{ ATF_POINTER, 5, offsetof(struct PGWRecord, stopTime),
		(ASN_TAG_CLASS_CONTEXT | (39 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"stopTime"
		},
	{ ATF_POINTER, 4, offsetof(struct PGWRecord, pDNConnectionID),
		(ASN_TAG_CLASS_CONTEXT | (41 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChargingID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pDNConnectionID"
		},
	{ ATF_POINTER, 3, offsetof(struct PGWRecord, threeGPP2UserLocationInformation),
		(ASN_TAG_CLASS_CONTEXT | (44 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"threeGPP2UserLocationInformation"
		},
	{ ATF_POINTER, 2, offsetof(struct PGWRecord, servedPDPPDNAddressExt),
		(ASN_TAG_CLASS_CONTEXT | (45 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_PDPAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"servedPDPPDNAddressExt"
		},
	{ ATF_POINTER, 1, offsetof(struct PGWRecord, lowPriorityIndicator),
		(ASN_TAG_CLASS_CONTEXT | (46 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lowPriorityIndicator"
		},
};
static const ber_tlv_tag_t asn_DEF_PGWRecord_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (17 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PGWRecord_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* recordType */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 }, /* servedIMSI */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 }, /* p-GWAddress */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 }, /* chargingID */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 4, 0, 0 }, /* servingNodeAddress */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 5, 0, 0 }, /* accessPointNameNI */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 6, 0, 0 }, /* pdpPDNType */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 7, 0, 0 }, /* servedPDPPDNAddress */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 8, 0, 0 }, /* dynamicAddressFlag */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 9, 0, 0 }, /* listOfTrafficVolumes */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 10, 0, 0 }, /* recordOpeningTime */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 11, 0, 0 }, /* duration */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 12, 0, 0 }, /* causeForRecClosing */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 13, 0, 0 }, /* recordSequenceNumber */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 14, 0, 0 }, /* nodeID */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 15, 0, 0 }, /* recordExtensions */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 16, 0, 0 }, /* localSequenceNumber */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 17, 0, 0 }, /* apnSelectionMode */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 18, 0, 0 }, /* servedMSISDN */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 19, 0, 0 }, /* chargingCharacteristics */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 20, 0, 0 }, /* chChSelectionMode */
    { (ASN_TAG_CLASS_CONTEXT | (25 << 2)), 21, 0, 0 }, /* iMSsignalingContext */
    { (ASN_TAG_CLASS_CONTEXT | (27 << 2)), 22, 0, 0 }, /* servinggNodePLMNIdentifier */
    { (ASN_TAG_CLASS_CONTEXT | (28 << 2)), 23, 0, 0 }, /* pSFurnishChargingInformation */
    { (ASN_TAG_CLASS_CONTEXT | (29 << 2)), 24, 0, 0 }, /* servedIMEISV */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 25, 0, 0 }, /* rATType */
    { (ASN_TAG_CLASS_CONTEXT | (31 << 2)), 26, 0, 0 }, /* mSTimeZone */
    { (ASN_TAG_CLASS_CONTEXT | (32 << 2)), 27, 0, 0 }, /* userLocationInformation */
    { (ASN_TAG_CLASS_CONTEXT | (34 << 2)), 28, 0, 0 }, /* listOfServiceData */
    { (ASN_TAG_CLASS_CONTEXT | (35 << 2)), 29, 0, 0 }, /* servingNodeType */
    { (ASN_TAG_CLASS_CONTEXT | (37 << 2)), 30, 0, 0 }, /* p-GWPLMNIdentifier */
    { (ASN_TAG_CLASS_CONTEXT | (38 << 2)), 31, 0, 0 }, /* startTime */
    { (ASN_TAG_CLASS_CONTEXT | (39 << 2)), 32, 0, 0 }, /* stopTime */
    { (ASN_TAG_CLASS_CONTEXT | (41 << 2)), 33, 0, 0 }, /* pDNConnectionID */
    { (ASN_TAG_CLASS_CONTEXT | (44 << 2)), 34, 0, 0 }, /* threeGPP2UserLocationInformation */
    { (ASN_TAG_CLASS_CONTEXT | (45 << 2)), 35, 0, 0 }, /* servedPDPPDNAddressExt */
    { (ASN_TAG_CLASS_CONTEXT | (46 << 2)), 36, 0, 0 } /* lowPriorityIndicator */
};
static const uint8_t asn_MAP_PGWRecord_mmap_1[(37 + (8 * sizeof(unsigned int)) - 1) / 8] = {
	(1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0),
	(0 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (0 << 2) | (0 << 1) | (0 << 0),
	(0 << 7) | (0 << 6) | (0 << 5) | (1 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0),
	(0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) | (1 << 3) | (0 << 2) | (0 << 1) | (0 << 0),
	(0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) | (0 << 3)
};
static asn_SET_specifics_t asn_SPC_PGWRecord_specs_1 = {
	sizeof(struct PGWRecord),
	offsetof(struct PGWRecord, _asn_ctx),
	offsetof(struct PGWRecord, _presence_map),
	asn_MAP_PGWRecord_tag2el_1,
	37,	/* Count of tags in the map */
	asn_MAP_PGWRecord_tag2el_1,	/* Same as above */
	37,	/* Count of tags in the CXER map */
	0,	/* Whether extensible */
	(unsigned int *)asn_MAP_PGWRecord_mmap_1	/* Mandatory elements map */
};
asn_TYPE_descriptor_t asn_DEF_PGWRecord = {
	"PGWRecord",
	"PGWRecord",
	SET_free,
	SET_print,
	SET_constraint,
	SET_decode_ber,
	SET_encode_der,
	SET_decode_xer,
	SET_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_PGWRecord_tags_1,
	sizeof(asn_DEF_PGWRecord_tags_1)
		/sizeof(asn_DEF_PGWRecord_tags_1[0]), /* 1 */
	asn_DEF_PGWRecord_tags_1,	/* Same as above */
	sizeof(asn_DEF_PGWRecord_tags_1)
		/sizeof(asn_DEF_PGWRecord_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_PGWRecord_1,
	37,	/* Elements count */
	&asn_SPC_PGWRecord_specs_1	/* Additional specs */
};

