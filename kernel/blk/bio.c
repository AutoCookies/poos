#include "bio.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

int bio_read_bytes(struct blkdev* dev, u32 off, void* buf, u32 len){
    u8* out=(u8*)buf; u8* sec=(u8*)kmalloc(512,16); if(!sec) return -1;
    u32 done=0;
    while(done<len){
        u32 lba=(off+done)/512, so=(off+done)%512, n=512-so; if(n>len-done) n=len-done;
        if(blk_read(dev,lba,1,sec)<0){ kfree(sec); return -1; }
        mem_copy(out+done,sec+so,n); done+=n;
    }
    kfree(sec); return 0;
}
int bio_write_bytes(struct blkdev* dev, u32 off, const void* buf, u32 len){
    const u8* in=(const u8*)buf; u8* sec=(u8*)kmalloc(512,16); if(!sec) return -1;
    u32 done=0;
    while(done<len){
        u32 lba=(off+done)/512, so=(off+done)%512, n=512-so; if(n>len-done) n=len-done;
        if((so||n!=512) && blk_read(dev,lba,1,sec)<0){ kfree(sec); return -1; }
        if(!so&&n==512) mem_copy(sec,in+done,512); else mem_copy(sec+so,in+done,n);
        if(blk_write(dev,lba,1,sec)<0){ kfree(sec); return -1; }
        done+=n;
    }
    kfree(sec); return 0;
}
