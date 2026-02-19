#include "dns.h"
#include "sock.h"
#include "netif.h"
#include "../mem/mem.h"

int dns_lookup_a(const char* host,u32* out_ip){ if(!host||!out_ip) return -1; int s=sock_socket(AF_INET,SOCK_DGRAM,0); if(s<0) return -1; struct sockaddr_in_k d={AF_INET,53,netif_default()->dns}; u8 q[128]; u16 id=0x1234; mem_set(q,0,sizeof(q)); q[0]=id>>8; q[1]=id; q[2]=1; q[5]=1; int qp=12; const char* p=host; while(*p){ int l=0; const char* st=p; while(*p&&*p!='.'){l++;p++;} q[qp++]=(u8)l; for(int i=0;i<l;i++) q[qp++]=st[i]; if(*p=='.') p++; } q[qp++]=0; q[qp++]=0; q[qp++]=1; q[qp++]=0; q[qp++]=1; if(sock_sendto(s,q,qp,&d)<0){sock_close(s);return -1;} u8 r[256]; int n=sock_recvfrom(s,r,sizeof(r),0); sock_close(s); if(n<32) return -1; int an=(r[6]<<8)|r[7]; int off=12; while(off<n && r[off]) off += 1+r[off]; off+=5; for(int i=0;i<an && off+12<n;i++){ if((r[off]&0xC0)==0xC0) off+=2; else { while(off<n&&r[off]) off+=1+r[off]; off++; } u16 typ=(r[off]<<8)|r[off+1]; u16 cls=(r[off+2]<<8)|r[off+3]; u16 rdlen=(r[off+10]<<8)|r[off+11]; off+=12; if(typ==1&&cls==1&&rdlen==4&&off+4<=n){ *out_ip=((u32)r[off]<<24)|((u32)r[off+1]<<16)|((u32)r[off+2]<<8)|r[off+3]; return 0;} off+=rdlen; }
 return -1; }
