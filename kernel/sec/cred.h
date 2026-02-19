#ifndef POOS_SEC_CRED_H
#define POOS_SEC_CRED_H
#include "../types.h"
#define CRED_MAX_GROUPS 8
struct cred {
    u32 refs;
    u32 uid, euid, suid;
    u32 gid, egid, sgid;
    u32 groups[CRED_MAX_GROUPS];
    u32 ngroups;
    u32 umask;
    u32 cap_permitted;
    u32 cap_effective;
    u32 cap_inheritable;
};
struct cred* cred_alloc_root(void);
void cred_ref(struct cred* c);
void cred_put(struct cred* c);
struct cred* cred_clone(const struct cred* in);
int cred_setuid(struct cred* c, u32 uid);
int cred_setgid(struct cred* c, u32 gid);
int cred_in_group(const struct cred* c, u32 gid);
int cred_has_cap(const struct cred* c, u32 cap);
#endif
