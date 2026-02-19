#include "sys_defs.h"
#include "../proc/usercopy.h"
#include "../proc/task.h"
#include "../proc/proc.h"
#include "../sched/sched.h"
#include "../time/time.h"
#include "../vfs/vfs.h"
#include "../ipc/pipe.h"
#include "../mm/mmap.h"
#include "../sec/cred.h"
#include "../sec/caps.h"
#include "../sec/audit.h"
#include "../sec/auth.h"
#include "../vfs/vfs_perm.h"
#include "../crypto/rng.h"
#include "../seccomp/seccomp.h"
#include "../cgroup/cgroup.h"
#include "../ns/ns.h"
#include "../ns/ns_proxy.h"
#include "../ns/netns.h"

void vga_write(const char*);


static int copy_user_path(char* kbuf, const char* upath, u32 n) {
    u32 len = 0;
    if (strnlen_user(upath, n, &len) < 0 || len + 1 > n) return -1;
    if (copy_from_user(kbuf, upath, len + 1) < 0) return -1;
    return 0;
}

int sys_write_compat(const char* uptr, u32 len) {
    char buf[128];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1U;
    if (copy_from_user(buf, uptr, len) < 0) return -1;
    buf[len] = '\0';
    vga_write(buf);
    return (int)len;
}
int sys_exit(int code) { proc_kill_current(code); return 0; }
int sys_yield(void) { kthread_yield(); return 0; }
int sys_sleep(u32 ms) { kthread_sleep(time_ms_to_ticks(ms)); return 0; }
int sys_getpid(void) { struct task* t = task_current(); return (t && t->owner) ? (int)t->owner->pid_ns : 0; }

int sys_open(const char* upath, u32 flags) {
    char path[128];
    if (copy_user_path(path, upath, sizeof(path)) < 0) return -1;
    struct file* f = 0;
    if (vfs_open(path, flags, &f) < 0) return -1;
    struct task* t = task_current();
    if (!t || !t->owner) { file_put(f); return -1; }
    int fd = fdtable_alloc(&t->owner->fdt, f);
    file_put(f);
    return fd;
}

int sys_close(int fd) {
    struct task* t = task_current();
    if (!t || !t->owner) return -1;
    return fdtable_close(&t->owner->fdt, fd);
}

int sys_read(int fd, void* ubuf, u32 len) {
    struct task* t = task_current();
    if (!t || !t->owner) return -1;
    struct file* f = fdtable_get(&t->owner->fdt, fd);
    if (!f) return -1;
    u8 kbuf[256]; if (len > sizeof(kbuf)) len = sizeof(kbuf);
    int rc = -1;
    if (f->ops && f->ops->read) rc = f->ops->read(f, f->pos, kbuf, len);
    else if (f->vnode && f->vnode->ops && f->vnode->ops->read) rc = f->vnode->ops->read(f->vnode, f->pos, kbuf, len);
    if (rc > 0) { if (copy_to_user(ubuf, kbuf, (u32)rc) < 0) return -1; f->pos += (u32)rc; }
    return rc;
}

int sys_write(int fd, const void* ubuf, u32 len) {
    struct task* t = task_current();
    if (!t || !t->owner) return -1;
    struct file* f = fdtable_get(&t->owner->fdt, fd);
    if (!f) return (fd == 1 || fd == 2) ? sys_write_compat((const char*)ubuf, len) : -1;
    u8 kbuf[256]; if (len > sizeof(kbuf)) len = sizeof(kbuf);
    if (copy_from_user(kbuf, ubuf, len) < 0) return -1;
    int rc = -1;
    if (f->ops && f->ops->write) rc = f->ops->write(f, f->pos, kbuf, len);
    else if (f->vnode && f->vnode->ops && f->vnode->ops->write) rc = f->vnode->ops->write(f->vnode, f->pos, kbuf, len);
    if (rc > 0) f->pos += (u32)rc;
    return rc;
}

int sys_lseek(int fd, i32 off, int whence) {
    struct task* t = task_current();
    struct file* f = t && t->owner ? fdtable_get(&t->owner->fdt, fd) : 0;
    struct vstat st;
    if (!f || !f->vnode || !f->vnode->ops || !f->vnode->ops->getattr) return -1;
    if (f->vnode->ops->getattr(f->vnode, &st) < 0) return -1;
    i32 np = (whence == 0) ? off : ((whence == 1) ? ((i32)f->pos + off) : ((i32)st.size + off));
    if (np < 0) return -1;
    f->pos = (u32)np;
    return np;
}

int sys_stat(const char* upath, void* ust) {
    char path[128]; struct vstat st;
    if (copy_user_path(path, upath, sizeof(path)) < 0) return -1;
    if (vfs_stat(path, &st) < 0) return -1;
    return copy_to_user(ust, &st, sizeof(st));
}

int sys_getdents(int fd, void* ubuf, u32 len) {
    struct task* t = task_current();
    struct file* f = t && t->owner ? fdtable_get(&t->owner->fdt, fd) : 0;
    struct vdirent d;
    if (!f || !f->vnode || !f->vnode->ops || !f->vnode->ops->readdir || len < sizeof(d)) return -1;
    int rc = f->vnode->ops->readdir(f->vnode, &f->pos, &d);
    if (rc <= 0) return rc;
    if (copy_to_user(ubuf, &d, sizeof(d)) < 0) return -1;
    return (int)sizeof(d);
}

int sys_execve(const char* upath) { char path[128]; if (copy_user_path(path, upath, sizeof(path)) < 0) return -1; return proc_exec_path_current(path); }
int sys_waitpid(int pid, int* ustatus) { int st = 0; int r = proc_waitpid(pid, &st, 0); if (r > 0 && ustatus && copy_to_user(ustatus, &st, sizeof(st)) < 0) return -1; return r; }
int sys_spawn(const char* upath) { char path[128]; int pid = -1; if (copy_user_path(path, upath, sizeof(path)) < 0) return -1; if (proc_spawn_path(path, &pid) < 0) return -1; return pid; }
int sys_fork(struct trapframe* tf) { return proc_fork_from_tf(tf); }
int sys_pipe(int* ufds) {
    struct file* rd=0; struct file* wr=0; struct task* t=task_current(); int fds[2];
    if (!t || !t->owner) return -1;
    if (pipe_create_files(&rd, &wr) < 0) return -1;
    fds[0] = fdtable_alloc(&t->owner->fdt, rd);
    fds[1] = fdtable_alloc(&t->owner->fdt, wr);
    file_put(rd); file_put(wr);
    if (fds[0] < 0 || fds[1] < 0) return -1;
    return copy_to_user(ufds, fds, sizeof(fds));
}
int sys_dup2(int oldfd, int newfd) { struct task* t=task_current(); if(!t||!t->owner) return -1; return fdtable_dup2(&t->owner->fdt, oldfd, newfd); }
int sys_kill(int pid, int sig) { struct cred* c=cred_current(); struct proc* p=proc_find((u32)pid); if(!p||!c) return -1; if(c->uid!=p->cred->uid && !cred_has_cap(c,CAP_KILL)) return -1; return proc_send_signal((u32)pid, sig); }

int sys_mmap(void* addr, u32 len, int prot, int flags, int fd, u32 off) { return mm_mmap_sys(addr, len, prot, flags, fd, off); }
int sys_munmap(void* addr, u32 len) { return mm_munmap_sys(addr, len); }

int sys_mkdir(const char* upath){ char p[128]; if(copy_user_path(p,upath,sizeof(p))<0) return -1; return vfs_mkdir(p,0); }
int sys_unlink(const char* upath){ char p[128]; if(copy_user_path(p,upath,sizeof(p))<0) return -1; return vfs_unlink(p); }
int sys_rename(const char* uo,const char* un){ char o[128],n[128]; if(copy_user_path(o,uo,sizeof(o))<0||copy_user_path(n,un,sizeof(n))<0) return -1; return vfs_rename(o,n); }
int sys_sync(void){ return vfs_sync(); }

#include "../net/netif.h"
#include "../net/icmp.h"
#include "../net/dns.h"

struct netinfo_u {u8 mac[6];u32 ip,mask,gw,dns;u32 rx,tx,drops;};
struct netns_diag_u {
    u32 nsid;
    u32 bridge_ip;
    u32 bridge_mask;
    u32 container_ip;
    u32 nat_enabled;
    char veth_host[8];
    char veth_peer[8];
    char bridge[8];
    u32 bridge_rx;
    u32 bridge_tx;
    u32 nat_pkts;
    u32 nat_bytes;
    u32 pfwd_hits;
    u32 drops;
};
struct netns_pfwd_req { u16 host_port; u16 container_port; u32 container_ip; u8 proto; u8 _pad[3]; };

int sys_netctl(int cmd, void* ubuf, u32 len){
    netif_t* n=netif_default();
    struct net_ns* ns = netns_current();
    if(!n || !ns) return -1;
    if(cmd==1){
        if(!netns_allowed()) return -1;
        if(len<sizeof(struct netinfo_u)) return -1;
        struct netinfo_u i;
        for(int k=0;k<6;k++) i.mac[k]=n->mac[k];
        i.ip=n->ip; i.mask=n->netmask; i.gw=n->gw; i.dns=n->dns;
        i.rx=n->stats.rx_packets; i.tx=n->stats.tx_packets; i.drops=n->stats.rx_drops+n->stats.tx_drops;
        return copy_to_user(ubuf,&i,sizeof(i));
    }
    if(cmd==2){
        if(!netns_allowed()) return -1;
        struct cred* c=cred_current(); if(!c||!cred_has_cap(c,CAP_NET_RAW)) return -1;
        if(len<4) return -1; u32 ip; if(copy_from_user(&ip,ubuf,4)<0) return -1;
        return icmp_ping(ip,0x55AA,1,1000);
    }
    if(cmd==3){
        if(!netns_allowed()) return -1;
        char host[64]; if(len>=sizeof(host)) len=sizeof(host)-1;
        if(copy_from_user(host,ubuf,len)<0) return -1; host[len]=0;
        u32 ip=0; if(dns_lookup_a(host,&ip)<0) return -1;
        return copy_to_user(ubuf,&ip,4);
    }
    if(cmd==10){
        struct netns_diag_u d;
        if(netns_fill_diag(ns,&d,sizeof(d))<0) return -1;
        return copy_to_user(ubuf,&d,sizeof(d));
    }
    if(cmd==11){
        struct cred* c=cred_current(); if(!c||!cred_has_cap(c,CAP_NET_ADMIN)) return -1;
        struct netns_pfwd_req r;
        if(len<sizeof(r)) return -1;
        if(copy_from_user(&r,ubuf,sizeof(r))<0) return -1;
        return netns_add_port_forward(ns,r.host_port,r.container_ip,r.container_port,r.proto);
    }
    if(cmd==12){
        struct cred* c=cred_current(); if(!c||!cred_has_cap(c,CAP_NET_ADMIN)) return -1;
        u32 on=0; if(len<4) return -1;
        if(copy_from_user(&on,ubuf,4)<0) return -1;
        ns->nat_enabled = on ? 1 : 0;
        return 0;
    }
    return -1;
}

int sys_getuid(void){ struct cred* c=cred_current(); return c?(int)c->uid:-1; }
int sys_geteuid(void){ struct cred* c=cred_current(); return c?(int)c->euid:-1; }
int sys_setuid(int uid){ return proc_setuid((u32)uid); }
int sys_umask(u32 mask){ struct cred* c=cred_current(); if(!c) return -1; u32 old=c->umask; c->umask=mask & 0777U; return (int)old; }
int sys_chmod(const char* upath,u32 mode){ char p[128]; struct vnode* vn=0; struct cred* c=cred_current(); if(copy_user_path(p,upath,sizeof(p))<0) return -1; if(vfs_resolve(p,&vn)<0) return -1; if(!c || (c->euid!=0 && c->euid!=vn->uid)){ vnode_put(vn); audit_log("deny chmod"); return -1; } vn->mode = (vn->mode & ~07777U) | (mode & 07777U); vnode_put(vn); audit_log("chmod"); return 0; }
int sys_chown(const char* upath,u32 uid,u32 gid){ char p[128]; struct vnode* vn=0; struct cred* c=cred_current(); if(copy_user_path(p,upath,sizeof(p))<0) return -1; if(vfs_resolve(p,&vn)<0) return -1; if(!c || (!cred_has_cap(c,CAP_CHOWN) && c->euid!=0)){ vnode_put(vn); audit_log("deny chown"); return -1; } vn->uid=uid; vn->gid=gid; vnode_put(vn); audit_log("chown"); return 0; }
int sys_chroot(const char* upath){ char p[128]; if(copy_user_path(p,upath,sizeof(p))<0) return -1; return proc_chroot(p); }
int sys_capget(void){ struct cred* c=cred_current(); return c?(int)c->cap_effective:-1; }
int sys_capset(int pid,u32 caps){ return proc_capset((u32)pid,caps); }
int sys_auth(const char* uuser,const char* upass,u32* uuid,u32* ugid){ char user[32],pass[64]; u32 uid=0,gid=0; if(copy_user_path(user,uuser,sizeof(user))<0) return -1; if(copy_user_path(pass,upass,sizeof(pass))<0) return -1; if(auth_verify_password(user,pass)<0) return -1; if(auth_lookup_user(user,&uid,&gid)<0) return -1; if(uuid && copy_to_user(uuid,&uid,sizeof(uid))<0) return -1; if(ugid && copy_to_user(ugid,&gid,sizeof(gid))<0) return -1; audit_log("login ok"); return 0; }

int sys_getrandom(void* ubuf, u32 len, u32 flags){ (void)flags; if(len>256) len=256; u8 kbuf[256]; if(rng_get_bytes(kbuf,len)<0) return -1; if(copy_to_user(ubuf,kbuf,len)<0) return -1; return (int)len; }
int sys_time(void){ return (int)time_epoch(); }
int sys_settime(u32 epoch){ struct cred* c=cred_current(); if(!c||!cred_has_cap(c,CAP_SYS_ADMIN)) return -1; time_set_epoch(epoch); return 0; }

int sys_clone(u32 flags, struct trapframe* tf){ return proc_clone(flags, tf); }
int sys_unshare(u32 flags){ return proc_unshare(flags); }
int sys_seccomp(u32 mode){ struct task* t=task_current(); if(!t||!t->owner||!t->owner->seccomp) return -1; seccomp_init_filter(t->owner->seccomp, mode); audit_log("seccomp.set"); return 0; }
int sys_cgset(u32 mem,u32 pids,u32 cpu){ struct task* t=task_current(); if(!t||!t->owner||!t->owner->cgrp) return -1; return cgroup_set_limits(t->owner->cgrp,mem,pids,cpu); }
