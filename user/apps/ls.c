#include "../libc_min/syscall.h"
extern void puts_min(const char*);
static int slen(const char* s){int i=0;while(i<32&&s[i])i++;return i;}
int main(void){
    int fd=sys_open("/",0); struct vdirent d;
    if(fd<0){ puts_min("ls: open / failed\n"); return 1; }
    while(sys_getdents(fd,&d,sizeof(d))>0){ int n=slen(d.name); if(n>0) sys_write(1,d.name,n); puts_min("\n"); }
    sys_close(fd);
    return 0;
}
