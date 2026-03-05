#include "fat.h"
#include "../../blk/bio.h"
#include "../../bcache/bcache.h"
#include "../../mem/mem.h"
#include "../../mem/heap.h"

#define FAT_ATTR_DIR 0x10
#define FAT_EOC 0xFFF8

struct __attribute__((packed)) bpb16 { u8 j[3]; u8 oem[8]; u16 bps; u8 spc; u16 rsvd; u8 fats; u16 root_cnt; u16 ts16; u8 media; u16 spf; u16 spt; u16 heads; u32 hidden; u32 ts32; u8 drive; u8 r1; u8 boot_sig; u32 vol_id; u8 label[11]; u8 fst[8]; };
struct __attribute__((packed)) de { u8 name[11]; u8 attr; u8 nt; u8 crt_tenth; u16 crt_t; u16 crt_d; u16 acc_d; u16 hi; u16 mod_t; u16 mod_d; u16 lo; u32 size; };

struct fat_node{ struct fat_fs* fs; u32 dir_cluster; u32 first_cluster; u32 size; u8 attr; struct vnode vn; };
struct fat_fs { struct blkdev* dev; u32 fat_start,root_start,data_start,root_secs,spc; u32 total_clusters; u16 root_cnt; u8 clean; struct vnode root_vn; struct fat_node root_node; };
static struct fat_fs gfs;

static u32 cl_to_lba(struct fat_fs*fs,u32 cl){ return fs->data_start + (cl-2)*fs->spc; }
static u32 fat_get(struct fat_fs*fs,u32 cl){ u32 off=cl*2; struct bcache_buf* b=bcache_get(fs->dev,(fs->fat_start*512+off)/4096); if(!b) return FAT_EOC; u32 in=(fs->fat_start*512+off)%4096; u16 v=*(u16*)(b->data+in); bcache_put(b); return v; }
static int fat_set(struct fat_fs*fs,u32 cl,u32 v){ u32 off=cl*2; struct bcache_buf* b=bcache_get(fs->dev,(fs->fat_start*512+off)/4096); if(!b) return -1; u32 in=(fs->fat_start*512+off)%4096; *(u16*)(b->data+in)=(u16)v; bcache_mark_dirty(b); bcache_put(b); return 0; }
static u32 fat_alloc(struct fat_fs* fs){ for(u32 c=2;c<fs->total_clusters+2;c++) if(fat_get(fs,c)==0){ fat_set(fs,c,FAT_EOC); return c; } return 0; }

static void name83(const char*name,u8 out[11]){ for(int i=0;i<11;i++) out[i]=' '; int oi=0,ext=8; for(int i=0;name[i];i++){ char c=name[i]; if(c=='.'){oi=ext; continue;} if(oi>=11) break; if(c>='a'&&c<='z') c-=32; out[oi++]=(u8)c; }}
static int namecmp(const u8 a[11], const char* n){ u8 b[11]; name83(n,b); for(int i=0;i<11;i++) if(a[i]!=b[i]) return 0; return 1; }

static int dir_iter(struct fat_node* dir, u32* idx, struct de* out, u32* lba_out, u32* off_out){
    struct fat_fs* fs=dir->fs; u32 entries_per_sec=512/sizeof(struct de); u32 pos=*idx;
    if(dir->first_cluster==0){ u32 total=fs->root_cnt; if(pos>=total) return 0; u32 sec=fs->root_start+pos/entries_per_sec; u32 off=(pos%entries_per_sec)*sizeof(struct de);
      struct bcache_buf* b=bcache_get(fs->dev,(sec*512)/4096); if(!b) return -1; u32 bi=(sec*512)%4096+off; mem_copy(out,b->data+bi,sizeof(*out)); bcache_put(b); *lba_out=sec; *off_out=off; *idx=pos+1; return 1; }
    u32 cl=dir->first_cluster; u32 idx_in=pos;
    while(idx_in >= fs->spc*entries_per_sec){ cl=fat_get(fs,cl); if(cl>=FAT_EOC) return 0; idx_in -= fs->spc*entries_per_sec; }
    u32 sec=cl_to_lba(fs,cl)+idx_in/entries_per_sec; u32 off=(idx_in%entries_per_sec)*sizeof(struct de);
    struct bcache_buf* b=bcache_get(fs->dev,(sec*512)/4096); if(!b) return -1; u32 bi=(sec*512)%4096+off; mem_copy(out,b->data+bi,sizeof(*out)); bcache_put(b); *lba_out=sec; *off_out=off; *idx=pos+1; return 1;
}

static int read_cluster_chain(struct fat_node* n,u32 off,void*buf,u32 len){ struct fat_fs*fs=n->fs; if(off>=n->size) return 0; if(off+len>n->size) len=n->size-off; u8*o=buf; u32 done=0; while(done<len){ u32 pos=off+done; u32 cidx=pos/(fs->spc*512), in=pos%(fs->spc*512), cl=n->first_cluster; for(u32 i=0;i<cidx;i++) cl=fat_get(fs,cl); if(cl>=FAT_EOC) break; u32 sec=cl_to_lba(fs,cl)+in/512, so=in%512, ncpy=512-so; if(ncpy>len-done) ncpy=len-done; struct bcache_buf* b=bcache_get(fs->dev,(sec*512)/4096); if(!b) break; mem_copy(o+done,b->data+((sec*512)%4096)+so,ncpy); bcache_put(b); done+=ncpy; } return (int)done; }
static int write_cluster_chain(struct fat_node* n,u32 off,const void*buf,u32 len){ struct fat_fs*fs=n->fs; const u8*i=buf; u32 done=0; while(done<len){ u32 pos=off+done; u32 cidx=pos/(fs->spc*512), in=pos%(fs->spc*512); if(n->first_cluster==0){ n->first_cluster=fat_alloc(fs); if(!n->first_cluster) return (int)done; }
        u32 cl=n->first_cluster; for(u32 k=0;k<cidx;k++){ u32 nx=fat_get(fs,cl); if(nx>=FAT_EOC){ nx=fat_alloc(fs); if(!nx) return (int)done; fat_set(fs,cl,nx); } cl=nx; }
        u32 sec=cl_to_lba(fs,cl)+in/512, so=in%512, ncpy=512-so; if(ncpy>len-done) ncpy=len-done; struct bcache_buf* b=bcache_get(fs->dev,(sec*512)/4096); if(!b) return (int)done; mem_copy(b->data+((sec*512)%4096)+so,i+done,ncpy); bcache_mark_dirty(b); bcache_put(b); done+=ncpy; }
    if(off+done>n->size) n->size=off+done;
    return (int)done;
}

static int fat_lookup(struct vnode* d,const char*name,struct vnode**out){ struct fat_node* dir=d->data; u32 i=0,lba,off; struct de e; while(1){ int rc=dir_iter(dir,&i,&e,&lba,&off); if(rc<=0) return -1; if(e.name[0]==0x00) return -1; if(e.name[0]==0xE5||e.attr==0x0F) continue; if(namecmp(e.name,name)){ struct fat_node* n=kmalloc(sizeof(*n),8); if(!n) return -1; n->fs=dir->fs; n->dir_cluster=dir->first_cluster; n->first_cluster=e.lo; n->size=e.size; n->attr=e.attr; vnode_init(&n->vn,(e.attr&FAT_ATTR_DIR)?VNODE_DIR:VNODE_REG,d->ops,n); *out=&n->vn; vnode_ref(*out); return 0; }} }
static int fat_read(struct vnode* vn,u32 off,void*buf,u32 len){ return read_cluster_chain(vn->data,off,buf,len); }
static int fat_write(struct vnode* vn,u32 off,const void*buf,u32 len){ return write_cluster_chain(vn->data,off,buf,len); }
static int fat_getattr(struct vnode*vn,struct vstat*out){ struct fat_node*n=vn->data; out->type=vn->type; out->size=n->size; out->mode=vn->mode; out->uid=vn->uid; out->gid=vn->gid; return 0; }
static int fat_readdir(struct vnode*vn,u32*cookie,struct vdirent*out){ struct fat_node* dir=vn->data; struct de e; u32 lba,off,idx=*cookie; while(1){ int rc=dir_iter(dir,&idx,&e,&lba,&off); if(rc<=0) return 0; if(e.name[0]==0x00) return 0; if(e.name[0]==0xE5||e.attr==0x0F) continue; for(int i=0;i<11;i++) out->name[i]=(char)e.name[i]; out->name[11]=0; out->type=(e.attr&FAT_ATTR_DIR)?VNODE_DIR:VNODE_REG; *cookie=idx; return 1; } }

static int dir_write_entry(struct fat_node*dir, const struct de*in, int want_free, const char*name){ u32 i=0,lba,off; struct de e; while(1){ int rc=dir_iter(dir,&i,&e,&lba,&off); if(rc<=0) return -1; if((want_free && (e.name[0]==0x00||e.name[0]==0xE5)) || (!want_free && namecmp(e.name,name))){ struct bcache_buf* b=bcache_get(dir->fs->dev,(lba*512)/4096); if(!b) return -1; mem_copy(b->data+((lba*512)%4096)+off,in,sizeof(*in)); bcache_mark_dirty(b); bcache_put(b); return 0; }} }

static int fat_create(struct vnode* d,const char*name,u32 mode,struct vnode**out){ (void)mode; struct fat_node*dir=d->data; struct de e; mem_set(&e,0,sizeof(e)); name83(name,e.name); e.attr=0; e.lo=(u16)fat_alloc(dir->fs); if(!e.lo) return -1; if(dir_write_entry(dir,&e,1,0)<0) return -1; return fat_lookup(d,name,out); }
static int fat_unlink(struct vnode* d,const char*name){ struct fat_node*dir=d->data; u32 i=0,lba,off; struct de e; while(1){ int rc=dir_iter(dir,&i,&e,&lba,&off); if(rc<=0) return -1; if(e.name[0]==0x00) return -1; if(namecmp(e.name,name)){ e.name[0]=0xE5; return dir_write_entry(dir,&e,0,name); }} }
static int fat_mkdir(struct vnode* d,const char*name,u32 mode,struct vnode**out){ (void)mode; struct vnode* vn=0; if(fat_create(d,name,0,&vn)<0) return -1; ((struct fat_node*)vn->data)->attr=FAT_ATTR_DIR; *out=vn; return 0; }
static int fat_trunc(struct vnode* vn,u32 sz){ struct fat_node*n=vn->data; if(sz==0){ n->size=0; return 0; } return -1; }

static const struct vnode_ops g_ops = {
    .lookup = fat_lookup,
    .read = fat_read,
    .write = fat_write,
    .readdir = fat_readdir,
    .getattr = fat_getattr,
    .create = fat_create,
    .mkdir = fat_mkdir,
    .unlink = fat_unlink,
    .truncate = fat_trunc
};

int fat_mount(struct blkdev* dev, struct vnode** out_root){
    struct bpb16 b; if(bio_read_bytes(dev,0,&b,sizeof(b))<0) return -1; if(b.bps!=512 || b.spc==0 || b.spf==0) return -1;
    gfs.dev=dev; gfs.spc=b.spc; gfs.fat_start=b.rsvd; gfs.root_cnt=b.root_cnt; gfs.root_secs=((b.root_cnt*32)+511)/512; gfs.root_start=gfs.fat_start+b.fats*b.spf; gfs.data_start=gfs.root_start+gfs.root_secs;
    u32 ts=b.ts16?b.ts16:b.ts32; u32 data_secs=ts-gfs.data_start; gfs.total_clusters=data_secs/gfs.spc;
    gfs.root_node.fs=&gfs; gfs.root_node.first_cluster=0; gfs.root_node.size=0; gfs.root_node.attr=FAT_ATTR_DIR; vnode_init(&gfs.root_vn,VNODE_DIR,&g_ops,&gfs.root_node);
    *out_root=&gfs.root_vn; vnode_ref(*out_root); return 0;
}
int fat_sync(void){ return bcache_sync(gfs.dev); }
