#include "../blk/blkdev.h"
#include "../types.h"

void vga_write(const char*);
void vga_write_u32(u32);

#define ATA_IO 0x1F0
#define ATA_CTRL 0x3F6
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_FLUSH 0xE7

static struct blkdev g_hd0;

static inline u16 inw(u16 p){u16 v; __asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p)); return v;}
static inline void outw(u16 p,u16 v){ __asm__ volatile("outw %0,%1"::"a"(v),"Nd"(p)); }

static int ata_wait(u8 mask,u8 val){ for(u32 i=0;i<1000000;i++){ u8 s=inb(ATA_IO+7); if((s&ATA_SR_BSY)==0 && (s&mask)==val) return 0; } return -1; }

static int ata_rw28(int wr,u32 lba,u8 cnt,void* buf){
    if(ata_wait(0,0)<0) return -1;
    outb(ATA_CTRL,0);
    outb(ATA_IO+6,0xE0|((lba>>24)&0x0F));
    outb(ATA_IO+2,cnt); outb(ATA_IO+3,lba&0xFF); outb(ATA_IO+4,(lba>>8)&0xFF); outb(ATA_IO+5,(lba>>16)&0xFF);
    outb(ATA_IO+7, wr?ATA_CMD_WRITE:ATA_CMD_READ);
    u16* w=(u16*)buf;
    for(u32 s=0;s<cnt;s++){
        if(ata_wait(ATA_SR_DRQ,ATA_SR_DRQ)<0) return -1;
        for(u32 i=0;i<256;i++){
            if(wr) outw(ATA_IO,w[s*256+i]); else w[s*256+i]=inw(ATA_IO);
        }
    }
    if(wr){ outb(ATA_IO+7,ATA_CMD_FLUSH); if(ata_wait(0,0)<0) return -1; }
    return 0;
}

static int hd_read(struct blkdev* d,u32 l,u32 c,void* b){ (void)d; while(c){u8 n=c>255?255:(u8)c; if(ata_rw28(0,l,n,b)<0) return -1; l+=n; c-=n; b=(u8*)b+n*512;} return 0; }
static int hd_write(struct blkdev* d,u32 l,u32 c,void* b){ (void)d; while(c){u8 n=c>255?255:(u8)c; if(ata_rw28(1,l,n,b)<0) return -1; l+=n; c-=n; b=(u8*)b+n*512;} return 0; }

int ata_pio_init(void){
    outb(ATA_IO+6,0xA0); outb(ATA_IO+2,0); outb(ATA_IO+3,0); outb(ATA_IO+4,0); outb(ATA_IO+5,0); outb(ATA_IO+7,ATA_CMD_IDENTIFY);
    if(inb(ATA_IO+7)==0) return -1;
    if(ata_wait(ATA_SR_DRQ,ATA_SR_DRQ)<0) return -1;
    u16 id[256]; for(u32 i=0;i<256;i++) id[i]=inw(ATA_IO);
    u32 sectors=((u32)id[61]<<16)|id[60];
    g_hd0.name="hd0"; g_hd0.sector_size=512; g_hd0.total_sectors=sectors; g_hd0.read=hd_read; g_hd0.write=hd_write; g_hd0.priv=0;
    if(blkdev_register(&g_hd0)<0) return -1;
    vga_write("ata: hd0 sectors="); vga_write_u32(sectors); vga_write("\n");
    return 0;
}
