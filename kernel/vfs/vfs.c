#include "../fs/memfs.h"
#include "../fs/fat/fat.h"
#include "vfs.h"
#include "mount.h"
#include "path.h"

static int split_parent(const char* path, char* parent, char* name){
    u32 len=0; while(path[len]) len++; if(len<2||path[0]!='/') return -1;
    while(len>1 && path[len-1]=='/') len--; u32 i=len; while(i>1 && path[i-1]!='/') i--; if(i==0||i>=len) return -1;
    u32 pn=i; if(pn==0) pn=1; for(u32 j=0;j<pn;j++) parent[j]=path[j]; parent[pn]=0;
    u32 n=0; for(u32 j=i;j<len && n<63;j++) name[n++]=path[j]; name[n]=0; return 0;
}

void vfs_init(void) { mount_init(); }
int vfs_mount(const char* path, struct vnode* root) { return mount_attach(path, root); }

int vfs_open(const char* path, u32 flags, struct file** out) {
    struct vnode* vn = 0;
    if (vfs_resolve(path, &vn) < 0) {
        char parent[128],name[64]; struct vnode* dir=0;
        if(split_parent(path,parent,name)<0 || vfs_resolve(parent,&dir)<0) {
            if (memfs_open_flat(path, flags, &vn) < 0) return -1;
        } else {
            if(!dir->ops || !dir->ops->create || !(flags&0x100) || dir->ops->create(dir,name,0,&vn)<0){ vnode_put(dir); return -1; }
            vnode_put(dir);
        }
    }
    if ((flags & 0x200) && vn->ops && vn->ops->truncate) vn->ops->truncate(vn, 0);
    struct file* f = file_create(vn, flags);
    vnode_put(vn);
    if (!f) return -1;
    *out = f;
    return 0;
}

int vfs_stat(const char* path, struct vstat* st) {
    struct vnode* vn = 0;
    if (vfs_resolve(path, &vn) < 0) return -1;
    if (!vn->ops || !vn->ops->getattr) { vnode_put(vn); return -1; }
    int rc = vn->ops->getattr(vn, st);
    vnode_put(vn);
    return rc;
}

int vfs_mkdir(const char* path, u32 mode){ char parent[128],name[64]; struct vnode* d=0; struct vnode* out=0; if(split_parent(path,parent,name)<0) return -1; if(vfs_resolve(parent,&d)<0) return -1; int rc=(d->ops&&d->ops->mkdir)?d->ops->mkdir(d,name,mode,&out):-1; if(out) vnode_put(out); vnode_put(d); return rc; }
int vfs_unlink(const char* path){ char parent[128],name[64]; struct vnode* d=0; if(split_parent(path,parent,name)<0) return -1; if(vfs_resolve(parent,&d)<0) return -1; int rc=(d->ops&&d->ops->unlink)?d->ops->unlink(d,name):-1; vnode_put(d); return rc; }
int vfs_rename(const char* o,const char* n){ (void)o;(void)n; return -1; }
int vfs_sync(void){ return fat_sync(); }
