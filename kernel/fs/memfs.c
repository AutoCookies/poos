#include "memfs.h"
#include "../mem/mem.h"
#include "../mem/heap.h"

#define MEMFS_MAX_FILES 16
#define MEMFS_MAX_SIZE 65536
struct mem_file { char name[32]; u8 data[MEMFS_MAX_SIZE]; u32 size; int used; struct vnode vn; };
static struct mem_file g_files[MEMFS_MAX_FILES];
static struct vnode g_root;

static int str_eq(const char*a,const char*b){u32 i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return 0;i++;}return a[i]==b[i];}
static u32 str_len(const char* s){u32 i=0;while(s[i])i++;return i;}

static int mf_read(struct vnode* vn,u32 off,void* buf,u32 len){ struct mem_file* f=(struct mem_file*)vn->data; if(off>=f->size) return 0; if(off+len>f->size) len=f->size-off; mem_copy(buf,f->data+off,len); return (int)len; }
static int mf_write(struct vnode* vn,u32 off,const void* buf,u32 len){ struct mem_file* f=(struct mem_file*)vn->data; if(off>=MEMFS_MAX_SIZE) return 0; if(off+len>MEMFS_MAX_SIZE) len=MEMFS_MAX_SIZE-off; mem_copy(f->data+off,buf,len); if(off+len>f->size) f->size=off+len; return (int)len; }
static int mf_getattr(struct vnode* vn,struct vstat* out){ struct mem_file* f=(struct mem_file*)vn->data; out->type=vn->type; out->mode=vn->mode; out->uid=vn->uid; out->gid=vn->gid; out->size=f->size; return 0; }
static int mdir_lookup(struct vnode* dir,const char* name,struct vnode** out){ (void)dir; for(u32 i=0;i<MEMFS_MAX_FILES;i++) if(g_files[i].used&&str_eq(g_files[i].name,name)){ *out=&g_files[i].vn; vnode_ref(*out); return 0; } return -1; }
static int mdir_readdir(struct vnode* vn,u32* cookie,struct vdirent* out){ (void)vn; u32 seen=0; for(u32 i=0;i<MEMFS_MAX_FILES;i++) if(g_files[i].used){ if(seen++<*cookie) continue; mem_copy(out->name,g_files[i].name,32); out->name[31]=0; out->type=VNODE_REG; *cookie=seen; return 1; } return 0; }
static int mdir_getattr(struct vnode* vn,struct vstat* out){ (void)vn; out->type=VNODE_DIR; out->mode=vn->mode; out->uid=vn->uid; out->gid=vn->gid; out->size=0; return 0; }

static const struct vnode_ops g_file_ops={0,mf_read,mf_write,0,mf_getattr};
static const struct vnode_ops g_dir_ops={mdir_lookup,0,0,mdir_readdir,mdir_getattr};

struct vnode* memfs_root(void){ vnode_init(&g_root,VNODE_DIR,&g_dir_ops,0); return &g_root; }

int memfs_open_flat(const char* path, u32 flags, struct vnode** out){
    const char* n=path; if(path[0]=='/'&&path[1]=='t'&&path[2]=='m'&&path[3]=='p'&&path[4]=='/') n=path+5; if(!*n) return -1;
    for(u32 i=0;i<MEMFS_MAX_FILES;i++) if(g_files[i].used&&str_eq(g_files[i].name,n)){ if(flags&0x200) g_files[i].size=0; *out=&g_files[i].vn; vnode_ref(*out); return 0; }
    if(!(flags&0x100)) return -1;
    for(u32 i=0;i<MEMFS_MAX_FILES;i++) if(!g_files[i].used){ g_files[i].used=1; mem_set(g_files[i].data,0,sizeof(g_files[i].data)); g_files[i].size=0; mem_set(g_files[i].name,0,sizeof(g_files[i].name)); u32 l=str_len(n); if(l>31) l=31; mem_copy(g_files[i].name,n,l); vnode_init(&g_files[i].vn,VNODE_REG,&g_file_ops,&g_files[i]); *out=&g_files[i].vn; vnode_ref(*out); return 0; }
    return -1;
}
