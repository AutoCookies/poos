#include "../libc_min/syscall.h"
extern int printf_min(const char*,...);
static int oct(const char*s){int v=0;for(int i=0;s[i];i++)v=(v<<3)+(s[i]-'0');return v;}
int main(int c,char**v){ if(c<3){ printf_min("usage: chmod <mode> <path>\n"); return 1;} return sys_chmod(v[2],oct(v[1]))<0; }
