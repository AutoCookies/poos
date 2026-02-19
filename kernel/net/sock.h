#ifndef POOS_SOCK_H
#define POOS_SOCK_H
#include "../types.h"
#include "pbuf.h"

#define AF_INET 2
#define SOCK_DGRAM 2

struct sockaddr_in_k { u16 family; u16 port; u32 addr; };

void sock_init(void);
int sock_socket(int domain, int type, int proto);
int sock_bind(int fd, const struct sockaddr_in_k* sa);
int sock_sendto(int fd, const void* buf, u32 len, const struct sockaddr_in_k* sa);
int sock_recvfrom(int fd, void* buf, u32 len, struct sockaddr_in_k* sa);
int sock_close(int fd);
void sock_udp_deliver(u32 sip, u16 sport, u32 dip, u16 dport, pbuf_t* payload);
u32 sock_udp_stats(void);
#endif
