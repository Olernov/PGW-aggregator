/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.txt"
 */

#include "TimeReport.h"

static asn_TYPE_member_t asn_MBR_TimeReport_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct TimeReport, ratingGroup),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RatingGroup,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ratingGroup"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TimeReport, startTime),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"startTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TimeReport, endTime),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"endTime"
		},
	{ ATF_POINTER, 2, offsetof(struct TimeReport, dataVolumeUplink),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DataVolumeGPRS,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dataVolumeUplink"
		},
	{ ATF_POINTER, 1, offsetof(struct TimeReport, dataVolumeDownlink),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DataVolumeGPRS,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dataVolumeDownlink"
		},
};
static const ber_tlv_tag_t asn_DEF_TimeReport_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_TimeReport_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* ratingGroup */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* startTime */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* endTime */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* dataVolumeUplink */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 } /* dataVolumeDownlink */
};
static asn_SEQUENCE_specifics_t asn_SPC_TimeReport_specs_1 = {
	sizeof(struct TimeReport),
	offsetof(struct TimeReport, _asn_ctx),
	asn_MAP_TimeReport_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_TimeReport = {
	"TimeReport",
	"TimeReport",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_TimeReport_tags_1,
	sizeof(asn_DEF_TimeReport_tags_1)
		/sizeof(asn_DEF_TimeReport_tags_1[0]), /* 1 */
	asn_DEF_TimeReport_tags_1,	/* Same as above */
	sizeof(asn_DEF_TimeReport_tags_1)
		/sizeof(asn_DEF_TimeReport_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_TimeReport_1,
	5,	/* Elements count */
	&asn_SPC_TimeReport_specs_1	/* Additional specs */
};
