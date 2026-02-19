#include "seccomp.h"
#include "../sec/audit.h"
int seccomp_check(struct seccomp_filter* f, u32 nr){ if(!f||f->mode==SECCOMP_MODE_DISABLED) return 0; if(nr<128 && f->allow[nr]) return 0; audit_log("seccomp.deny"); return -1; }
