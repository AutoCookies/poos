#include "sock.h"
#include "udp.h"
#include "netif.h"
#include "../sched/spinlock.h"
#include "../sched/sched.h"
#include "../mem/mem.h"

#define MAX_SOCK 32
#define SOCK_Q 16
struct dgram{u32 sip;u16 sport;u16 len;u8 data[512];};
struct usock{u8 used;u16 lport;struct dgram q[SOCK_Q];u8 qh,qt,qc;};
static struct usock s[MAX_SOCK]; static spinlock_t lk; static u16 next_port=49152;

static int fd_to_idx(int fd){return fd-200;}
void sock_init(void){ mem_set(s,0,sizeof(s)); spinlock_init(&lk); }
int sock_socket(int domain,int type,int proto){ (void)proto; if(domain!=AF_INET||type!=SOCK_DGRAM) return -1; spinlock_guard_t g=spin_lock_irqsave(&lk); for(int i=0;i<MAX_SOCK;i++) if(!s[i].used){ s[i].used=1; s[i].lport=next_port++; spin_unlock_irqrestore(&lk,g); return i+200; } spin_unlock_irqrestore(&lk,g); return -1; }
int sock_bind(int fd,const struct sockaddr_in_k* sa){ int i=fd_to_idx(fd); if(i<0||i>=MAX_SOCK||!sa) return -1; spinlock_guard_t g=spin_lock_irqsave(&lk); if(!s[i].used){spin_unlock_irqrestore(&lk,g);return -1;} s[i].lport=sa->port; spin_unlock_irqrestore(&lk,g); return 0; }
int sock_sendto(int fd,const void* buf,u32 len,const struct sockaddr_in_k* sa){ int i=fd_to_idx(fd); netif_t* n=netif_default(); if(i<0||i>=MAX_SOCK||!s[i].used||!sa||!n) return -1; if(len>512) len=512; return udp_send(n,n->ip,s[i].lport,sa->addr,sa->port,buf,(u16)len); }
int sock_recvfrom(int fd,void* buf,u32 len,struct sockaddr_in_k* sa){ int i=fd_to_idx(fd); if(i<0||i>=MAX_SOCK||!s[i].used) return -1; for(;;){ spinlock_guard_t g=spin_lock_irqsave(&lk); if(s[i].qc){ struct dgram* d=&s[i].q[s[i].qh]; u32 n=d->len; if(n>len) n=len; mem_copy(buf,d->data,n); if(sa){sa->family=AF_INET;sa->addr=d->sip;sa->port=d->sport;} s[i].qh=(u8)((s[i].qh+1)%SOCK_Q); s[i].qc--; spin_unlock_irqrestore(&lk,g); return (int)n; } spin_unlock_irqrestore(&lk,g); kthread_sleep(1);} }
int sock_close(int fd){ int i=fd_to_idx(fd); if(i<0||i>=MAX_SOCK) return -1; spinlock_guard_t g=spin_lock_irqsave(&lk); mem_set(&s[i],0,sizeof(s[i])); spin_unlock_irqrestore(&lk,g); return 0; }
void sock_udp_deliver(u32 sip,u16 sport,u32 dip,u16 dport,pbuf_t* p){ (void)dip; spinlock_guard_t g=spin_lock_irqsave(&lk); for(int i=0;i<MAX_SOCK;i++) if(s[i].used && s[i].lport==dport){ if(s[i].qc<SOCK_Q){ struct dgram* d=&s[i].q[s[i].qt]; d->sip=sip; d->sport=sport; d->len=p->len>512?512:p->len; mem_copy(d->data,p->data,d->len); s[i].qt=(u8)((s[i].qt+1)%SOCK_Q); s[i].qc++; } break; } spin_unlock_irqrestore(&lk,g); }
u32 sock_udp_stats(void){ u32 c=0; for(int i=0;i<MAX_SOCK;i++) if(s[i].used) c++; return c; }
