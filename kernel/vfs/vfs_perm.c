#include "vfs_perm.h"
#include "../proc/proc.h"
#include "../sec/cred.h"
#include "../sec/caps.h"

static u32 perm_bits(struct vnode* vn, const struct cred* c){
    if(!vn||!c) return 0;
    if(c->euid==vn->uid) return (vn->mode>>6)&7U;
    if(cred_in_group(c,vn->gid)) return (vn->mode>>3)&7U;
    return vn->mode&7U;
}
int vfs_check_perm(struct vnode* vn, int want){
    struct cred* c=cred_current();
    if(!vn||!c) return -1;
    if(cred_has_cap(c,CAP_DAC_OVERRIDE)) return 0;
    u32 b=perm_bits(vn,c);
    if((want&VFS_PERM_READ) && !(b&4U)) return -1;
    if((want&VFS_PERM_WRITE)&& !(b&2U)) return -1;
    if((want&VFS_PERM_EXEC) && !(b&1U)) return -1;
    return 0;
}
int vfs_check_dir_op(struct vnode* dir, int want_write){ return vfs_check_perm(dir, VFS_PERM_EXEC | (want_write?VFS_PERM_WRITE:0)); }
