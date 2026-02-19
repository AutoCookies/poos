#ifndef POOS_NS_PROXY_H
#define POOS_NS_PROXY_H
#include "mntns.h"
#include "pidns.h"
#include "netns.h"
#include "utsns.h"
#include "userns.h"
struct nsproxy { struct mnt_ns* mnt; struct pid_ns* pid; struct net_ns* net; struct uts_ns* uts; struct user_ns* user; u32 refcnt; };
struct nsproxy* nsproxy_create_host(struct vnode* root);
struct nsproxy* nsproxy_clone(struct nsproxy* src, u32 clone_flags, struct vnode* root);
void nsproxy_get(struct nsproxy* nsp);
void nsproxy_put(struct nsproxy* nsp);
#endif
