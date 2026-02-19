#include "../libc_min/syscall.h"
extern int printf_min(const char*,...);
static int dec(const char*s){int v=0;for(int i=0;s[i]>='0'&&s[i]<='9';i++)v=v*10+s[i]-'0';return v;}
int main(int c,char**v){ if(c<3){ printf_min("usage: chown <uid>:<gid> <path>\n"); return 1;} char* p=v[1]; int i=0; while(p[i]&&p[i]!=':')i++; if(!p[i]) return 1; p[i]=0; return sys_chown(v[2],dec(p),dec(p+i+1))<0; }
