/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GPRSChargingDataTypes"
 * 	found in "./PGW_CDR_Format.asn"
 */

#include "CAMELInformationSMS.h"

static asn_TYPE_member_t asn_MBR_CAMELInformationSMS_1[] = {
	{ ATF_POINTER, 8, offsetof(struct CAMELInformationSMS, sCFAddress),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SCFAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sCFAddress"
		},
	{ ATF_POINTER, 7, offsetof(struct CAMELInformationSMS, serviceKey),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceKey"
		},
	{ ATF_POINTER, 6, offsetof(struct CAMELInformationSMS, defaultSMSHandling),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DefaultSMS_Handling,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"defaultSMSHandling"
		},
	{ ATF_POINTER, 5, offsetof(struct CAMELInformationSMS, cAMELCallingPartyNumber),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallingNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cAMELCallingPartyNumber"
		},
	{ ATF_POINTER, 4, offsetof(struct CAMELInformationSMS, cAMELDestinationSubscriberNumber),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SmsTpDestinationNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cAMELDestinationSubscriberNumber"
		},
	{ ATF_POINTER, 3, offsetof(struct CAMELInformationSMS, cAMELSMSCAddress),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cAMELSMSCAddress"
		},
	{ ATF_POINTER, 2, offsetof(struct CAMELInformationSMS, freeFormatData),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FreeFormatData,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"freeFormatData"
		},
	{ ATF_POINTER, 1, offsetof(struct CAMELInformationSMS, smsReferenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallReferenceNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"smsReferenceNumber"
		},
};
static const ber_tlv_tag_t asn_DEF_CAMELInformationSMS_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (17 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CAMELInformationSMS_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* sCFAddress */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* serviceKey */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* defaultSMSHandling */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* cAMELCallingPartyNumber */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 }, /* cAMELDestinationSubscriberNumber */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 5, 0, 0 }, /* cAMELSMSCAddress */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 6, 0, 0 }, /* freeFormatData */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 7, 0, 0 } /* smsReferenceNumber */
};
static const uint8_t asn_MAP_CAMELInformationSMS_mmap_1[(8 + (8 * sizeof(unsigned int)) - 1) / 8] = {
	(0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0)
};
static asn_SET_specifics_t asn_SPC_CAMELInformationSMS_specs_1 = {
	sizeof(struct CAMELInformationSMS),
	offsetof(struct CAMELInformationSMS, _asn_ctx),
	offsetof(struct CAMELInformationSMS, _presence_map),
	asn_MAP_CAMELInformationSMS_tag2el_1,
	8,	/* Count of tags in the map */
	asn_MAP_CAMELInformationSMS_tag2el_1,	/* Same as above */
	8,	/* Count of tags in the CXER map */
	0,	/* Whether extensible */
	(unsigned int *)asn_MAP_CAMELInformationSMS_mmap_1	/* Mandatory elements map */
};
asn_TYPE_descriptor_t asn_DEF_CAMELInformationSMS = {
	"CAMELInformationSMS",
	"CAMELInformationSMS",
	SET_free,
	SET_print,
	SET_constraint,
	SET_decode_ber,
	SET_encode_der,
	SET_decode_xer,
	SET_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CAMELInformationSMS_tags_1,
	sizeof(asn_DEF_CAMELInformationSMS_tags_1)
		/sizeof(asn_DEF_CAMELInformationSMS_tags_1[0]), /* 1 */
	asn_DEF_CAMELInformationSMS_tags_1,	/* Same as above */
	sizeof(asn_DEF_CAMELInformationSMS_tags_1)
		/sizeof(asn_DEF_CAMELInformationSMS_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CAMELInformationSMS_1,
	8,	/* Elements count */
	&asn_SPC_CAMELInformationSMS_specs_1	/* Additional specs */
};

