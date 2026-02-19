#include "blkdev.h"

#define MAX_BLKDEVS 8
static struct blkdev* g_devs[MAX_BLKDEVS];
static u32 g_dev_count;

static int str_eq(const char* a, const char* b){u32 i=0; while(a[i]&&b[i]){ if(a[i]!=b[i]) return 0; i++; } return a[i]==b[i];}

int blkdev_register(struct blkdev* dev){ if(!dev||!dev->name||!dev->read||!dev->write||g_dev_count>=MAX_BLKDEVS) return -1; g_devs[g_dev_count++]=dev; return 0; }
struct blkdev* blkdev_get(const char* name){ for(u32 i=0;i<g_dev_count;i++) if(str_eq(g_devs[i]->name,name)) return g_devs[i]; return 0; }
int blk_read(struct blkdev* dev, u32 lba, u32 count, void* buf){ if(!dev||!buf||!count) return -1; if(lba+count>dev->total_sectors) return -1; return dev->read(dev,lba,count,buf); }
int blk_write(struct blkdev* dev, u32 lba, u32 count, const void* buf){ if(!dev||!buf||!count) return -1; if(lba+count>dev->total_sectors) return -1; return dev->write(dev,lba,count,(void*)buf); }
