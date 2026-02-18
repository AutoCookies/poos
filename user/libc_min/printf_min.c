#include "syscall.h"
static int slen(const char* s){int n=0;while(s[n])n++;return n;}
void puts_min(const char* s){sys_write(1,s,slen(s));}
