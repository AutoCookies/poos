#include "../libc_min/syscall.h"
int main(void){
    unsigned char* p=(unsigned char*)sys_mmap(0, 64*1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    if ((int)p<0){ sys_write(1,"mmap fail\n",10); return 1; }
    for (int i=0;i<64*1024;i+=4096){ if (p[i]!=0){ sys_write(1,"nonzero\n",8); return 2; } p[i]=(unsigned char)(i/4096); }
    sys_write(1,"mmaptest success\n",17);
    return 0;
}
