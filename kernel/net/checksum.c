#include "checksum.h"
static u32 sum16(const u8* d,u32 l,u32 s){ while(l>1){ s += ((u32)d[0]<<8)|d[1]; d+=2; l-=2;} if(l) s += (u32)d[0]<<8; while(s>>16) s=(s&0xFFFF)+(s>>16); return s; }
u16 net_checksum(const void* data, u32 len){ u32 s=sum16((const u8*)data,len,0); return (u16)~s; }
u16 net_checksum_udp(u32 sip,u32 dip,u8 proto,const void* data,u32 len){ u8 ph[12]={sip>>24,sip>>16,sip>>8,sip,dip>>24,dip>>16,dip>>8,dip,0,proto,len>>8,len}; u32 s=sum16(ph,12,0); s=sum16((const u8*)data,len,s); while(s>>16) s=(s&0xFFFF)+(s>>16); return (u16)~s; }
