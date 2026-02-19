#include "../libc_min/syscall.h"
extern void puts_min(const char*);
extern int printf_min(const char*,...);
static int rd(char* b,int n){int r=sys_read(0,b,n-1); if(r<0) return r; b[r]=0; return r;}
static void trim(char*s){int i=0;while(s[i]&&s[i]!='\n'&&s[i]!='\r')i++;s[i]=0;}
int main(void){ char u[32],p[64]; unsigned int uid=0,gid=0; puts_min("login: "); if(rd(u,sizeof(u))<=0) return 1; trim(u); puts_min("password: "); if(rd(p,sizeof(p))<=0) return 1; trim(p); if(sys_auth(u,p,(int*)&uid,(int*)&gid)<0){ puts_min("Login incorrect\n"); return 1; } sys_setuid((int)uid); if(sys_execve("/bin/sh")<0) puts_min("exec shell failed\n"); return 0; }
