#include "../libc_min/syscall.h"

static int strcmp2(const char* a,const char* b){while(*a&&*b&&*a==*b){a++;b++;}return (unsigned char)*a-(unsigned char)*b;}
static int strlen2(const char* s){int n=0;while(s[n])n++;return n;}

int main(void) {
    char* p = (char*)sys_mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    if ((int)p < 0) return 1;
    p[0]='p'; p[1]='a'; p[2]='r'; p[3]='e'; p[4]='n'; p[5]='t'; p[6]=0;
    int pid = sys_fork();
    if (pid == 0) {
        p[0]='c'; p[1]='h'; p[2]='i'; p[3]='l'; p[4]='d'; p[5]=0;
        if (strcmp2(p,"child") != 0) return 2;
        sys_write(1,"cow child ok\n",13);
        sys_exit(0);
    }
    int st=0; sys_waitpid(pid,&st);
    if (strcmp2(p,"parent") != 0) { sys_write(1,"cow parent mismatch\n",20); return 3; }
    sys_write(1,"cowtest success\n",16);
    return 0;
}
