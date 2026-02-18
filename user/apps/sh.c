#include "../libc_min/syscall.h"
extern void puts_min(const char*);
extern int printf_min(const char*, ...);

static int str_len(const char* s){int i=0;while(s[i])i++;return i;}
static int streq(const char* a,const char* b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return 0;i++;}return a[i]==b[i];}
static void trim(char* s){ int i=0,j=0; while(s[i]==' '||s[i]=='\t')i++; for(;s[i];++i) s[j++]=s[i]; while(j>0&&(s[j-1]==' '||s[j-1]=='\t'||s[j-1]=='\n'))j--; s[j]=0; }

static void parse_cmd(char* s, char** cmd, char** in, char** out, int* bg){ *in=*out=0; *bg=0; for(int i=0;s[i];++i){ if(s[i]=='&'){ *bg=1; s[i]=0; }
 if(s[i]=='<'){ s[i]=0; *in=s+i+1; }
 if(s[i]=='>'){ s[i]=0; *out=s+i+1; }} trim(s); if(*in) trim(*in); if(*out) trim(*out); *cmd=s; }

static void run_simple(char* line){
    char *cmd,*in,*out; int bg; parse_cmd(line,&cmd,&in,&out,&bg);
    if(!cmd[0]) return;
    if(streq(cmd,"help")){ puts_min("help ls cat echo\n"); return; }
    if(cmd[0]=='c'&&cmd[1]=='a'&&cmd[2]=='t'&&cmd[3]==' '){
        char* f=cmd+4; trim(f); int fd=sys_open(f,O_RDONLY); if(fd<0){ puts_min("cat: open failed\n"); return;}
        char b[64]; for(;;){int n=sys_read(fd,b,sizeof(b)); if(n<=0) break; sys_write(1,b,n);} sys_close(fd); return;
    }
    if(cmd[0]=='e'&&cmd[1]=='c'&&cmd[2]=='h'&&cmd[3]=='o'&&cmd[4]==' '){ sys_write(1,cmd+5,str_len(cmd+5)); sys_write(1,"\n",1); return; }
    int pid=sys_fork();
    if(pid==0){
        if(in){ int fd=sys_open(in,O_RDONLY); if(fd>=0){ sys_dup2(fd,0); sys_close(fd);} }
        if(out){ int fd=sys_open(out,O_CREAT|O_TRUNC|O_WRONLY); if(fd>=0){ sys_dup2(fd,1); sys_close(fd);} }
        char path[64]; if(cmd[0]=='/') { int i=0; for(;cmd[i]&&i<63;i++) path[i]=cmd[i]; path[i]=0; }
        else { int i=0; path[i++]='/';path[i++]='b';path[i++]='i';path[i++]='n';path[i++]='/'; int j=0; while(cmd[j]&&i<63) path[i++]=cmd[j++]; path[i]=0; }
        if(sys_execve(path)<0) puts_min("sh: exec failed\n");
        sys_exit(127);
    } else if(pid>0){ if(!bg) sys_waitpid(pid,0); }
}

static void run_line(char* line){
    trim(line); if(!line[0]) return;
    for(int i=0;line[i];++i) if(line[i]=='|'){
        line[i]=0; char* left=line; char* right=line+i+1; trim(left); trim(right);
        int p[2]; if(sys_pipe(p)<0){ puts_min("pipe fail\n"); return; }
        int c1=sys_fork(); if(c1==0){ sys_dup2(p[1],1); sys_close(p[0]); sys_close(p[1]); run_simple(left); sys_exit(0);}
        int c2=sys_fork(); if(c2==0){ sys_dup2(p[0],0); sys_close(p[1]); sys_close(p[0]); run_simple(right); sys_exit(0);}
        sys_close(p[0]); sys_close(p[1]); sys_waitpid(c1,0); sys_waitpid(c2,0); return;
    }
    run_simple(line);
}

int main(void){
    char line[128];
    puts_min("PoOS v0.6 shell\n");
    for(;;){ puts_min("poos> "); int n=sys_read(0,line,sizeof(line)-1); if(n<=0) continue; line[n]=0; run_line(line);} return 0;
}
