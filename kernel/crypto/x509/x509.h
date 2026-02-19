#ifndef POOS_X509_H
#define POOS_X509_H
#include "../../types.h"
typedef struct { char subject_cn[128]; char issuer_cn[128]; u32 not_before; u32 not_after; } x509_cert;
int x509_parse_der(const u8* der, u32 len, x509_cert* out);
#endif
