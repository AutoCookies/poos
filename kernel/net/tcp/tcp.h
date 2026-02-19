#ifndef POOS_TCP_H
#define POOS_TCP_H

#include "../netif.h"
#include "../pbuf.h"
#include "tcp_state.h"
#include "../../types.h"

#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_RST 0x04
#define TCP_FLAG_PSH 0x08
#define TCP_FLAG_ACK 0x10

#define TCP_MAX_CONN 64
#define TCP_RX_BUF 8192
#define TCP_MSS 1460

struct tcp_sock;

struct tcp_segq {
    u32 seq;
    u16 len;
    u8 flags;
    u32 tx_deadline;
    u8 retries;
    u8 data[TCP_MSS];
    struct tcp_segq* next;
};

struct tcp_conn {
    u8 used;
    enum tcp_state state;
    u32 local_ip, remote_ip;
    u16 local_port, remote_port;
    u32 snd_una, snd_nxt;
    u32 rcv_nxt;
    u32 iss, irs;
    u16 send_window;
    u16 recv_window;
    u32 timewait_deadline;
    u32 delayed_ack_deadline;
    struct tcp_segq* retransmit_q;
    struct tcp_segq* ooo_q;
    struct tcp_sock* sk;
    u8 rxbuf[TCP_RX_BUF];
    u16 rx_head, rx_tail, rx_count;
};

struct tcp_stats {
    u32 active;
    u32 seg_tx;
    u32 seg_rx;
    u32 retransmits;
    u32 drops;
    u32 rst;
    u32 ooo;
};

void tcp_init(void);
void tcp_input(netif_t* n, u32 sip, u32 dip, pbuf_t* p);
void tcp_timer_tick(void);
void tcp_set_now_ticks(u32 now);
int tcp_connect(struct tcp_conn* c, u32 rip, u16 rport);
int tcp_send(struct tcp_conn* c, const u8* buf, u32 len);
int tcp_recv(struct tcp_conn* c, u8* buf, u32 len);
int tcp_close(struct tcp_conn* c);
struct tcp_conn* tcp_conn_alloc(void);
void tcp_conn_put(struct tcp_conn* c);
struct tcp_conn* tcp_conn_lookup(u32 lip,u16 lport,u32 rip,u16 rport);
u16 tcp_alloc_ephemeral_port(void);
const struct tcp_stats* tcp_stats_get(void);

#endif
