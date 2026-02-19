#include "net.h"
#include "eth.h"
#include "arp.h"
#include "sock.h"
#include "dhcp.h"
#include "../sched/sched.h"
#include "../sched/spinlock.h"

void vga_write(const char*);
void vga_write_u32(u32);

#define NET_RXQ_MAX 64
static pbuf_t* rxq_head; static pbuf_t* rxq_tail; static u32 rxq_len;
static spinlock_t rxq_lock;
static netif_t* g_if;

static void net_thread(void* arg){ (void)arg; dhcp_start(); for(;;){ pbuf_t* p=0; spinlock_guard_t g=spin_lock_irqsave(&rxq_lock); if(rxq_head){ p=rxq_head; rxq_head=p->next; if(!rxq_head) rxq_tail=0; rxq_len--; } spin_unlock_irqrestore(&rxq_lock,g); if(!p){ kthread_sleep(1); continue; } p->next=0; eth_input(g_if,p); pbuf_free(p);} }

void net_init(void){ spinlock_init(&rxq_lock); arp_init(); sock_init(); kthread_create("knetd",net_thread,0,0); }
int netif_register(netif_t* n){ g_if=n; netif_set_default(n); return 0; }
void net_input(netif_t* n,pbuf_t* p){ (void)n; if(!p) return; spinlock_guard_t g=spin_lock_irqsave(&rxq_lock); if(rxq_len>=NET_RXQ_MAX){ if(g_if) g_if->stats.rxq_drop++; pbuf_free(p); } else { p->next=0; if(!rxq_tail) rxq_head=p; else rxq_tail->next=p; rxq_tail=p; rxq_len++; } spin_unlock_irqrestore(&rxq_lock,g); }
void net_kick(void){}
void net_print_stats(void){ if(!g_if) return; vga_write("net: rx=");vga_write_u32(g_if->stats.rx_packets);vga_write(" tx=");vga_write_u32(g_if->stats.tx_packets);vga_write(" drops=");vga_write_u32(g_if->stats.rx_drops+g_if->stats.tx_drops);vga_write(" arp=");vga_write_u32(g_if->stats.arp_entries);vga_write(" udp=");vga_write_u32(sock_udp_stats());vga_write("\n"); }
