#include "../libc_min/syscall.h"
int main(void){
    int fd=sys_open("/etc/motd", O_RDONLY); if(fd<0){sys_write(1,"open fail\n",10);return 1;}
    char* m=(char*)sys_mmap(0,4096,PROT_READ,MAP_PRIVATE,fd,0); if((int)m<0){sys_write(1,"mmap fail\n",10);return 2;}
    char b[32]; int n=sys_read(fd,b,31); if(n<0){sys_write(1,"read fail\n",10);return 3;} b[n]=0;
    for(int i=0;i<n;i++){ if(m[i]!=b[i]){sys_write(1,"cmp fail\n",9);return 4;} }
    char* m2=(char*)sys_mmap(0,4096,PROT_READ,MAP_PRIVATE,fd,0); if((int)m2<0)return 5;
    if(m2[0]!=m[0]) return 6;
    sys_write(1,"filemaptest success\n",20);
    return 0;
}
