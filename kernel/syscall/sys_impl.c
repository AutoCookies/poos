#include "sys_defs.h"
#include "../proc/usercopy.h"
#include "../proc/task.h"
#include "../proc/proc.h"
#include "../sched/sched.h"
#include "../time/time.h"
#include "../vfs/vfs.h"
#include "../ipc/pipe.h"
#include "../mm/mmap.h"

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
int sys_getpid(void) { struct task* t = task_current(); return (t && t->owner) ? (int)t->owner->pid : 0; }

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
int sys_kill(int pid, int sig) { return proc_send_signal((u32)pid, sig); }

int sys_mmap(void* addr, u32 len, int prot, int flags, int fd, u32 off) { return mm_mmap_sys(addr, len, prot, flags, fd, off); }
int sys_munmap(void* addr, u32 len) { return mm_munmap_sys(addr, len); }
