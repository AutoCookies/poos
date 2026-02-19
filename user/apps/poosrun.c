#include "../libc_min/syscall.h"
static int streq(const char*a,const char*b){int i=0; for(;a[i]&&b[i];++i) if(a[i]!=b[i]) return 0; return a[i]==b[i];}
static int parse_num(const char*s){int v=0; for(int i=0;s[i];++i){ if(s[i]<'0'||s[i]>'9') break; v=v*10+(s[i]-'0'); } return v; }
int main(int argc,char**argv){
    int flags=CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWNET;
    int mem=64*1024*1024,pids=64,cpu=20,sec=SECCOMP_MODE_STRICT; const char* root=0; const char* cmd="/bin/sh";
    for(int i=1;i<argc;i++){
        if(streq(argv[i],"--")){ if(i+1<argc) cmd=argv[i+1]; break; }
        if(streq(argv[i],"--net=host")) flags &= ~CLONE_NEWNET;
        else if(streq(argv[i],"--net=none")) flags |= CLONE_NEWNET;
        else if(streq(argv[i],"--seccomp=off")) sec=SECCOMP_MODE_DISABLED;
        else if(streq(argv[i],"--root") && i+1<argc) root=argv[++i];
        else if(streq(argv[i],"--mem") && i+1<argc) mem=parse_num(argv[++i])*1024*1024;
        else if(streq(argv[i],"--pids") && i+1<argc) pids=parse_num(argv[++i]);
        else if(streq(argv[i],"--cpu") && i+1<argc) cpu=parse_num(argv[++i]);
    }
    if(sys_unshare(flags)<0){ sys_write(2,"poosrun: unshare failed\n",24); return 1; }
    if(root && sys_chroot(root)<0){ sys_write(2,"poosrun: chroot failed\n",23); return 1; }
    if(sys_cgset(mem,pids,cpu)<0){ sys_write(2,"poosrun: cgroup set failed\n",27); return 1; }
    if(sys_seccomp(sec)<0){ sys_write(2,"poosrun: seccomp failed\n",23); return 1; }
    return sys_execve(cmd);
}
