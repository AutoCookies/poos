#include "../libc_min/syscall.h"
extern void puts_min(const char*);

static int str_eq(const char* a,const char* b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return 0;i++;}return a[i]==b[i];}
static int str_len(const char* s){int i=0;while(s[i])i++;return i;}
static void str_copy(char* d,const char* s){while(*s)*d++=*s++;*d=0;}

static void run_external(const char* cmd){
    char path[64];
    if (cmd[0] == '/') str_copy(path, cmd);
    else { path[0]='/'; path[1]='b'; path[2]='i'; path[3]='n'; path[4]='/'; path[5]=0; int l=5; for(int i=0; cmd[i]&&l<63; ++i) path[l++]=cmd[i]; path[l]=0; }
    int pid = sys_spawn(path);
    if (pid > 0) sys_waitpid(pid, 0);
    else { puts_min("sh: exec failed\n"); }
}

static void exec_line(const char* line){
    if (str_eq(line, "help")) { puts_min("help ls cat echo\n"); return; }
    if (str_eq(line, "ls /") || str_eq(line, "ls")) { run_external("ls"); return; }
    if (str_eq(line, "cat /etc/motd") || str_eq(line, "cat")) { run_external("cat"); return; }
    if (line[0]=='e'&&line[1]=='c'&&line[2]=='h'&&line[3]=='o'&&line[4]==' ') { sys_write(1,line+5,str_len(line+5)); puts_min("\n"); return; }
    run_external(line);
}

int main(void){
    const char* script[] = { "help", "ls /", "cat /etc/motd", "/bin/hello", 0 };
    puts_min("poos> shell online\n");
    for (int i=0; script[i]; ++i) { puts_min("poos> "); puts_min(script[i]); puts_min("\n"); exec_line(script[i]); }
    for(;;){ sys_sleep(1000); }
    return 0;
}
