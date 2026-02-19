#include "part.h"
#include "../mem/mem.h"

struct __attribute__((packed)) mbr_ent { u8 st[3]; u8 type; u8 en[3]; u32 lba_start; u32 lba_len; };
struct part_priv { struct blkdev* parent; u32 start; };
static struct blkdev g_parts[4];
static struct part_priv g_priv[4];
static char g_names[4][8];

static int part_read(struct blkdev* dev,u32 lba,u32 c,void* buf){ struct part_priv* p=(struct part_priv*)dev->priv; return blk_read(p->parent,p->start+lba,c,buf); }
static int part_write(struct blkdev* dev,u32 lba,u32 c,void* buf){ struct part_priv* p=(struct part_priv*)dev->priv; return blk_write(p->parent,p->start+lba,c,buf); }

int part_scan_mbr(struct blkdev* disk){
    u8 sec[512]; if(blk_read(disk,0,1,sec)<0) return -1;
    if(sec[510]!=0x55||sec[511]!=0xAA) return -1;
    struct mbr_ent* e=(struct mbr_ent*)(sec+446);
    for(u32 i=0;i<4;i++){
        if(!e[i].lba_len) continue;
        g_names[i][0]='h'; g_names[i][1]='d'; g_names[i][2]='0'; g_names[i][3]='p'; g_names[i][4]='1'+(char)i; g_names[i][5]=0;
        g_priv[i].parent=disk; g_priv[i].start=e[i].lba_start;
        g_parts[i].name=g_names[i]; g_parts[i].sector_size=512; g_parts[i].total_sectors=e[i].lba_len;
        g_parts[i].read=part_read; g_parts[i].write=part_write; g_parts[i].priv=&g_priv[i];
        blkdev_register(&g_parts[i]);
    }
    return 0;
}
