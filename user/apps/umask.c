#include "../libc_min/syscall.h"
extern int printf_min(const char*,...);
static int oct(const char*s){int v=0;for(int i=0;s[i];i++)v=(v<<3)+(s[i]-'0');return v;}
int main(int c,char**v){ if(c<2){ printf_min("%o\n",sys_umask(022)); return 0;} int old=sys_umask(oct(v[1])); printf_min("%o\n",old); return 0; }
