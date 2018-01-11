/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#include "GprsCdrExtensions.h"

static asn_TYPE_member_t asn_MBR_serviceContainers_6[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ServiceContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_serviceContainers_tags_6[] = {
	(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_serviceContainers_specs_6 = {
	sizeof(struct serviceContainers),
	offsetof(struct serviceContainers, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_serviceContainers_6 = {
	"serviceContainers",
	"serviceContainers",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_serviceContainers_tags_6,
	sizeof(asn_DEF_serviceContainers_tags_6)
		/sizeof(asn_DEF_serviceContainers_tags_6[0]) - 1, /* 1 */
	asn_DEF_serviceContainers_tags_6,	/* Same as above */
	sizeof(asn_DEF_serviceContainers_tags_6)
		/sizeof(asn_DEF_serviceContainers_tags_6[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_serviceContainers_6,
	1,	/* Single element */
	&asn_SPC_serviceContainers_specs_6	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_timeReports_8[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_TimeReport,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_timeReports_tags_8[] = {
	(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_timeReports_specs_8 = {
	sizeof(struct timeReports),
	offsetof(struct timeReports, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_timeReports_8 = {
	"timeReports",
	"timeReports",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_timeReports_tags_8,
	sizeof(asn_DEF_timeReports_tags_8)
		/sizeof(asn_DEF_timeReports_tags_8[0]) - 1, /* 1 */
	asn_DEF_timeReports_tags_8,	/* Same as above */
	sizeof(asn_DEF_timeReports_tags_8)
		/sizeof(asn_DEF_timeReports_tags_8[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_timeReports_8,
	1,	/* Single element */
	&asn_SPC_timeReports_specs_8	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_GprsCdrExtensions_1[] = {
	{ ATF_POINTER, 7, offsetof(struct GprsCdrExtensions, creditControlInfo),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CreditControlInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"creditControlInfo"
		},
	{ ATF_POINTER, 6, offsetof(struct GprsCdrExtensions, policyControlInfo),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PolicyControlInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"policyControlInfo"
		},
	{ ATF_POINTER, 5, offsetof(struct GprsCdrExtensions, userCategory),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"userCategory"
		},
	{ ATF_POINTER, 4, offsetof(struct GprsCdrExtensions, ruleSpaceId),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IA5String,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ruleSpaceId"
		},
	{ ATF_POINTER, 3, offsetof(struct GprsCdrExtensions, serviceContainers),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		0,
		&asn_DEF_serviceContainers_6,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceContainers"
		},
	{ ATF_POINTER, 2, offsetof(struct GprsCdrExtensions, timeReports),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		0,
		&asn_DEF_timeReports_8,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"timeReports"
		},
	{ ATF_POINTER, 1, offsetof(struct GprsCdrExtensions, dataZoneInfo),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IA5String,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dataZoneInfo"
		},
};
static const ber_tlv_tag_t asn_DEF_GprsCdrExtensions_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (17 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_GprsCdrExtensions_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* creditControlInfo */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 }, /* policyControlInfo */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 2, 0, 0 }, /* userCategory */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 3, 0, 0 }, /* ruleSpaceId */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 4, 0, 0 }, /* serviceContainers */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 5, 0, 0 }, /* timeReports */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 6, 0, 0 } /* dataZoneInfo */
};
static const uint8_t asn_MAP_GprsCdrExtensions_mmap_1[(7 + (8 * sizeof(unsigned int)) - 1) / 8] = {
	(0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) | (0 << 3) | (0 << 2) | (0 << 1)
};
static asn_SET_specifics_t asn_SPC_GprsCdrExtensions_specs_1 = {
	sizeof(struct GprsCdrExtensions),
	offsetof(struct GprsCdrExtensions, _asn_ctx),
	offsetof(struct GprsCdrExtensions, _presence_map),
	asn_MAP_GprsCdrExtensions_tag2el_1,
	7,	/* Count of tags in the map */
	asn_MAP_GprsCdrExtensions_tag2el_1,	/* Same as above */
	7,	/* Count of tags in the CXER map */
	0,	/* Whether extensible */
	(unsigned int *)asn_MAP_GprsCdrExtensions_mmap_1	/* Mandatory elements map */
};
asn_TYPE_descriptor_t asn_DEF_GprsCdrExtensions = {
	"GprsCdrExtensions",
	"GprsCdrExtensions",
	SET_free,
	SET_print,
	SET_constraint,
	SET_decode_ber,
	SET_encode_der,
	SET_decode_xer,
	SET_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_GprsCdrExtensions_tags_1,
	sizeof(asn_DEF_GprsCdrExtensions_tags_1)
		/sizeof(asn_DEF_GprsCdrExtensions_tags_1[0]), /* 1 */
	asn_DEF_GprsCdrExtensions_tags_1,	/* Same as above */
	sizeof(asn_DEF_GprsCdrExtensions_tags_1)
		/sizeof(asn_DEF_GprsCdrExtensions_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_GprsCdrExtensions_1,
	7,	/* Elements count */
	&asn_SPC_GprsCdrExtensions_specs_1	/* Additional specs */
};
