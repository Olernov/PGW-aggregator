/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#include "URI.h"

static asn_TYPE_member_t asn_MBR_listOfUriTimeStamps_7[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_TimeStamp,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_listOfUriTimeStamps_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_listOfUriTimeStamps_specs_7 = {
	sizeof(struct listOfUriTimeStamps),
	offsetof(struct listOfUriTimeStamps, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_listOfUriTimeStamps_7 = {
	"listOfUriTimeStamps",
	"listOfUriTimeStamps",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_listOfUriTimeStamps_tags_7,
	sizeof(asn_DEF_listOfUriTimeStamps_tags_7)
		/sizeof(asn_DEF_listOfUriTimeStamps_tags_7[0]) - 1, /* 1 */
	asn_DEF_listOfUriTimeStamps_tags_7,	/* Same as above */
	sizeof(asn_DEF_listOfUriTimeStamps_tags_7)
		/sizeof(asn_DEF_listOfUriTimeStamps_tags_7[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_listOfUriTimeStamps_7,
	1,	/* Single element */
	&asn_SPC_listOfUriTimeStamps_specs_7	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_URI_1[] = {
	{ ATF_POINTER, 6, offsetof(struct URI, count),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"count"
		},
	{ ATF_POINTER, 5, offsetof(struct URI, uri),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IA5String,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"uri"
		},
	{ ATF_POINTER, 4, offsetof(struct URI, uriIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"uriIdentifier"
		},
	{ ATF_POINTER, 3, offsetof(struct URI, uriDataVolumeUplink),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"uriDataVolumeUplink"
		},
	{ ATF_POINTER, 2, offsetof(struct URI, uriDataVolumeDownlink),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"uriDataVolumeDownlink"
		},
	{ ATF_POINTER, 1, offsetof(struct URI, listOfUriTimeStamps),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		0,
		&asn_DEF_listOfUriTimeStamps_7,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"listOfUriTimeStamps"
		},
};
static const ber_tlv_tag_t asn_DEF_URI_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_URI_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* count */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* uri */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* uriIdentifier */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* uriDataVolumeUplink */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 }, /* uriDataVolumeDownlink */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 5, 0, 0 } /* listOfUriTimeStamps */
};
static asn_SEQUENCE_specifics_t asn_SPC_URI_specs_1 = {
	sizeof(struct URI),
	offsetof(struct URI, _asn_ctx),
	asn_MAP_URI_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_URI = {
	"URI",
	"URI",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_URI_tags_1,
	sizeof(asn_DEF_URI_tags_1)
		/sizeof(asn_DEF_URI_tags_1[0]), /* 1 */
	asn_DEF_URI_tags_1,	/* Same as above */
	sizeof(asn_DEF_URI_tags_1)
		/sizeof(asn_DEF_URI_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_URI_1,
	6,	/* Elements count */
	&asn_SPC_URI_specs_1	/* Additional specs */
};

