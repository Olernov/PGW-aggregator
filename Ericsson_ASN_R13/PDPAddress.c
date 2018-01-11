/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#include "PDPAddress.h"

static asn_TYPE_member_t asn_MBR_PDPAddress_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PDPAddress, choice.iPAddress),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_IPAddress,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"iPAddress"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_PDPAddress_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* iPAddress */
};
static asn_CHOICE_specifics_t asn_SPC_PDPAddress_specs_1 = {
	sizeof(struct PDPAddress),
	offsetof(struct PDPAddress, _asn_ctx),
	offsetof(struct PDPAddress, present),
	sizeof(((struct PDPAddress *)0)->present),
	asn_MAP_PDPAddress_tag2el_1,
	1,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_PDPAddress = {
	"PDPAddress",
	"PDPAddress",
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
	asn_MBR_PDPAddress_1,
	1,	/* Elements count */
	&asn_SPC_PDPAddress_specs_1	/* Additional specs */
};

