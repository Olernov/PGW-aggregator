/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "GgsnPgwR13Ber"
 * 	found in "./pgw-r13.asn"
 */

#ifndef	_CreditRequestType_H_
#define	_CreditRequestType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CreditRequestType {
	CreditRequestType_start	= 0,
	CreditRequestType_interim	= 1,
	CreditRequestType_stop	= 2
} e_CreditRequestType;

/* CreditRequestType */
typedef long	 CreditRequestType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CreditRequestType;
asn_struct_free_f CreditRequestType_free;
asn_struct_print_f CreditRequestType_print;
asn_constr_check_f CreditRequestType_constraint;
ber_type_decoder_f CreditRequestType_decode_ber;
der_type_encoder_f CreditRequestType_encode_der;
xer_type_decoder_f CreditRequestType_decode_xer;
xer_type_encoder_f CreditRequestType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CreditRequestType_H_ */
#include <asn_internal.h>