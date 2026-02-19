#include "asn1.h"
int asn1_parse_tlv(const u8* p, const u8* end, asn1_tlv* out){
    if(!p||!out||p>=end) return -1;
    out->tag=*p++; if(p>=end) return -1;
    u32 len=0; u8 lb=*p++;
    if((lb&0x80U)==0){ len=lb; }
    else { u32 n=lb&0x7fU; if(n==0||n>4||(u32)(end-p)<n) return -1; for(u32 i=0;i<n;i++) len=(len<<8)|p[i]; p+=n; }
    if((u32)(end-p)<len) return -1;
    out->val=p; out->len=len; out->next=p+len; return 0;
}
