#ifndef POOS_SECCOMP_H
#define POOS_SECCOMP_H
#include "../types.h"
#define SECCOMP_MODE_DISABLED 0
#define SECCOMP_MODE_STRICT 1
#define SECCOMP_MODE_FILTER 2
struct seccomp_filter { u32 mode; u8 allow[128]; u32 allow_count; };
void seccomp_init_filter(struct seccomp_filter* f, u32 mode);
int seccomp_allow_syscall(struct seccomp_filter* f, u32 nr);
int seccomp_check(struct seccomp_filter* f, u32 nr);
#endif
