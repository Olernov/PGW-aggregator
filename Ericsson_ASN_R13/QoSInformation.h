/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_QoSInformation_H_
#define	_QoSInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* QoSInformation */
typedef OCTET_STRING_t	 QoSInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_QoSInformation;
asn_struct_free_f QoSInformation_free;
asn_struct_print_f QoSInformation_print;
asn_constr_check_f QoSInformation_constraint;
ber_type_decoder_f QoSInformation_decode_ber;
der_type_encoder_f QoSInformation_encode_der;
xer_type_decoder_f QoSInformation_decode_xer;
xer_type_encoder_f QoSInformation_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _QoSInformation_H_ */
#include <asn_internal.h>
