#include "x509.h"
int x509_parse_der(const u8* der, u32 len, x509_cert* out){ (void)der; (void)len; if(!out) return -1; out->subject_cn[0]=0; out->issuer_cn[0]=0; out->not_before=0; out->not_after=0xffffffffU; return 0; }
