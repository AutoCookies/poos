#ifndef POOS_ASN1_H
#define POOS_ASN1_H
#include "../../types.h"

typedef struct { u8 tag; const u8* val; u32 len; const u8* next; } asn1_tlv;
int asn1_parse_tlv(const u8* p, const u8* end, asn1_tlv* out);

#endif
