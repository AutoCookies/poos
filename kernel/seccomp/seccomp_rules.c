#include "seccomp.h"
void seccomp_init_filter(struct seccomp_filter* f, u32 mode){ if(!f) return; for(u32 i=0;i<128;i++) f->allow[i]=0; f->mode=mode; f->allow_count=0; if(mode==SECCOMP_MODE_STRICT){ u32 base[]={1,2,3,4,5,6,7,8,9,10,11,12,13,15,16,17,19,20}; for(u32 i=0;i<sizeof(base)/sizeof(base[0]);i++) seccomp_allow_syscall(f,base[i]); } }
int seccomp_allow_syscall(struct seccomp_filter* f, u32 nr){ if(!f||nr>=128) return -1; if(!f->allow[nr]){ f->allow[nr]=1; f->allow_count++; } return 0; }
