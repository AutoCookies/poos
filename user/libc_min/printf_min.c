#include "syscall.h"
#include <stdarg.h>

static int slen(const char* s){int n=0;while(s&&s[n])n++;return n;}
void puts_min(const char* s){sys_write(1,s,slen(s));}

static void putc_min(char c){ sys_write(1,&c,1); }

static void putu_base(unsigned int v, unsigned int base, int lower){
    char buf[32]; int i=0;
    const char* digs = lower ? "0123456789abcdef" : "0123456789ABCDEF";
    if(v==0){ putc_min('0'); return; }
    while(v && i < (int)sizeof(buf)){ buf[i++]=digs[v%base]; v/=base; }
    while(i--) putc_min(buf[i]);
}

static void puti(int v){
    if(v<0){ putc_min('-'); putu_base((unsigned int)(-v),10,1); }
    else putu_base((unsigned int)v,10,1);
}

int printf_min(const char* fmt, ...){
    va_list ap; va_start(ap, fmt);
    int out=0;
    for(int i=0; fmt && fmt[i]; ++i){
        if(fmt[i] != '%'){ putc_min(fmt[i]); out++; continue; }
        char c = fmt[++i]; if(!c) break;
        switch(c){
            case '%': putc_min('%'); out++; break;
            case 's': { const char* s = va_arg(ap,const char*); if(!s) s="(null)"; int n=slen(s); sys_write(1,s,n); out+=n; break; }
            case 'd': { int v=va_arg(ap,int); puti(v); break; }
            case 'u': { unsigned int v=va_arg(ap,unsigned int); putu_base(v,10,1); break; }
            case 'x': { unsigned int v=va_arg(ap,unsigned int); putu_base(v,16,1); break; }
            case 'o': { unsigned int v=va_arg(ap,unsigned int); putu_base(v,8,1); break; }
            case 'c': { char ch=(char)va_arg(ap,int); putc_min(ch); out++; break; }
            default: putc_min('%'); putc_min(c); out+=2; break;
        }
    }
    va_end(ap);
    return out;
}
