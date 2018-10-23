/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.txt"
 */

#include "GPRSRecord.h"

static asn_TYPE_member_t asn_MBR_GPRSRecord_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GPRSRecord, choice.pgwRecord),
		(ASN_TAG_CLASS_CONTEXT | (79 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PGWRecord,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pgwRecord"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_GPRSRecord_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (79 << 2)), 0, 0, 0 } /* pgwRecord */
};
static asn_CHOICE_specifics_t asn_SPC_GPRSRecord_specs_1 = {
	sizeof(struct GPRSRecord),
	offsetof(struct GPRSRecord, _asn_ctx),
	offsetof(struct GPRSRecord, present),
	sizeof(((struct GPRSRecord *)0)->present),
	asn_MAP_GPRSRecord_tag2el_1,
	1,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_GPRSRecord = {
	"GPRSRecord",
	"GPRSRecord",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_GPRSRecord_1,
	1,	/* Elements count */
	&asn_SPC_GPRSRecord_specs_1	/* Additional specs */
};
