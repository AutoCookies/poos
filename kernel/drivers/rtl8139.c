#include "../types.h"
#include "../pci/pci.h"
#include "../pci/pci_ids.h"
#include "../net/net.h"
#include "../net/netif.h"
#include "../net/pbuf.h"
#include "../arch/x86/idt.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

void vga_write(const char*); void vga_write_hex(u32); void vga_write_u32(u32);

#define RTL_IDR0 0x00
#define RTL_RBSTART 0x30
#define RTL_CMD 0x37
#define RTL_IMR 0x3C
#define RTL_ISR 0x3E
#define RTL_RCR 0x44
#define RTL_TCR 0x40
#define RTL_CONFIG1 0x52
#define RTL_TSAD0 0x20
#define RTL_TSD0 0x10

static inline void outw(u16 p,u16 v){__asm__ volatile("outw %0,%1"::"a"(v),"Nd"(p));}
static inline u16 inw(u16 p){u16 v;__asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outl(u16 p,u32 v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}
static inline u32 inl(u16 p){u32 v;__asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p));return v;}

static struct { u16 io; u8 irq; netif_t nif; u8* rx; u32 rxp; u8 txi; u8 txb[4][1600]; } g;

static void rtl_irq(struct trapframe* tf){ (void)tf; u16 st=inw(g.io+RTL_ISR); outw(g.io+RTL_ISR,st); if(!(st&1)) return; for(;;){ u32 cap=inw(g.io+RTL_CMD); if(cap&1) break; u8* p=g.rx+g.rxp; u16 pktst=*(u16*)p; u16 len=*(u16*)(p+2); if(!(pktst&1) || len<64 || len>1518){ g.nif.stats.rx_drops++; break; } pbuf_t* pb=pbuf_alloc((u16)(len-4),32); if(pb){ mem_copy(pb->data,p+4,pb->len); net_input(&g.nif,pb);} g.rxp=(g.rxp+len+4+3)&~3U; g.rxp%=8192; outw(g.io+0x38,(u16)(g.rxp-16)); }
}

static int rtl_tx(netif_t* n,pbuf_t* p){ (void)n; if(p->len>1600){pbuf_free(p);return -1;} mem_copy(g.txb[g.txi],p->data,p->len); outl(g.io+RTL_TSAD0+g.txi*4,(u32)g.txb[g.txi]-KERNEL_VIRT_BASE); outl(g.io+RTL_TSD0+g.txi*4,p->len); g.txi=(u8)((g.txi+1)&3); pbuf_free(p); return 0; }

int rtl8139_init(void){ pci_dev_t d; if(pci_find(PCI_VENDOR_REALTEK,PCI_DEVICE_RTL8139,&d)<0) return -1; bool io=false; g.io=(u16)pci_map_bar(&d,0,&io); g.irq=d.irq_line; pci_enable_bus_master(&d); outb(g.io+RTL_CONFIG1,0x0); outb(g.io+RTL_CMD,0x10); for(u32 i=0;i<100000;i++) if(!(inb(g.io+RTL_CMD)&0x10)) break; g.rx=(u8*)kmalloc(8192+16+1500,16); if(!g.rx) return -1; outl(g.io+RTL_RBSTART,(u32)g.rx-KERNEL_VIRT_BASE); outl(g.io+RTL_RCR,0x0000E68A); outl(g.io+RTL_TCR,0x03000700); outw(g.io+RTL_IMR,0x0005); outb(g.io+RTL_CMD,0x0C);
 for(int i=0;i<6;i++) g.nif.mac[i]=inb(g.io+RTL_IDR0+i); g.nif.name[0]='e';g.nif.name[1]='t';g.nif.name[2]='h';g.nif.name[3]='0';g.nif.name[4]=0; g.nif.mtu=1500; g.nif.tx=rtl_tx; netif_register(&g.nif); idt_register_handler((u8)(32+d.irq_line),rtl_irq);
 vga_write("rtl8139 up irq=");vga_write_u32(d.irq_line);vga_write(" mac="); for(int i=0;i<6;i++){ vga_write_hex(g.nif.mac[i]); if(i<5) vga_write(":"); } vga_write("\n"); return 0; }
