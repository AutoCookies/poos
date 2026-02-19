#ifndef POOS_NS_MNTNS_H
#define POOS_NS_MNTNS_H
#include "../types.h"
struct vnode;
struct mnt_ns { u32 id; u32 refcnt; struct vnode* root; };
struct mnt_ns* mntns_create(struct vnode* root);
void mntns_get(struct mnt_ns* ns);
void mntns_put(struct mnt_ns* ns);
#endif
