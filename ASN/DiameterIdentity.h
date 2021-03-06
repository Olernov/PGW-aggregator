/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GPRSChargingDataTypes"
 * 	found in "./PGW_CDR_Format.asn"
 */

#ifndef	_DiameterIdentity_H_
#define	_DiameterIdentity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DiameterIdentity */
typedef OCTET_STRING_t	 DiameterIdentity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DiameterIdentity;
asn_struct_free_f DiameterIdentity_free;
asn_struct_print_f DiameterIdentity_print;
asn_constr_check_f DiameterIdentity_constraint;
ber_type_decoder_f DiameterIdentity_decode_ber;
der_type_encoder_f DiameterIdentity_encode_der;
xer_type_decoder_f DiameterIdentity_decode_xer;
xer_type_encoder_f DiameterIdentity_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _DiameterIdentity_H_ */
#include <asn_internal.h>
