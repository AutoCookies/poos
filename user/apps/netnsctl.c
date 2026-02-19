#include "../libc_min/syscall.h"
#include "../libc_min/printf_min.h"

static int streq(const char*a,const char*b){int i=0; for(;a[i]&&b[i];++i) if(a[i]!=b[i]) return 0; return a[i]==b[i];}
static int parse_num(const char*s){ int v=0; for(int i=0;s[i];i++){ if(s[i]<'0'||s[i]>'9') break; v=v*10+(s[i]-'0'); } return v; }
static void ipprint(unsigned int ip){ printf_min("%d.%d.%d.%d",(ip>>24)&255,(ip>>16)&255,(ip>>8)&255,ip&255); }

int main(int argc,char**argv){
    if(argc<2 || streq(argv[1],"show")){
        struct netns_diag_u d;
        if(sys_netctl(10,&d,sizeof(d))<0){ printf_min("netnsctl: show failed\n"); return 1; }
        printf_min("ns=%d bridge=%s ",d.nsid,d.bridge); ipprint(d.bridge_ip);
        printf_min("/%d peer=%s host=%s ct=",24,d.veth_peer,d.veth_host); ipprint(d.container_ip);
        printf_min(" nat=%d\n",d.nat_enabled);
        printf_min("stats brx=%d btx=%d nat_pkts=%d nat_bytes=%d pfwd_hits=%d drops=%d\n",d.bridge_rx,d.bridge_tx,d.nat_pkts,d.nat_bytes,d.pfwd_hits,d.drops);
        return 0;
    }
    if(streq(argv[1],"nat") && argc>=3){
        unsigned int on = streq(argv[2],"on") ? 1U : 0U;
        if(sys_netctl(12,&on,4)<0){ printf_min("netnsctl: nat change failed\n"); return 1; }
        return 0;
    }
    if(streq(argv[1],"pfwd") && argc>=4){
        struct netns_pfwd_req r;
        r.host_port=(unsigned short)parse_num(argv[2]);
        r.container_port=(unsigned short)parse_num(argv[3]);
        struct netns_diag_u d;
        if(sys_netctl(10,&d,sizeof(d))<0){ printf_min("netnsctl: cannot query container ip\n"); return 1; }
        r.container_ip=d.container_ip;
        r.proto=6; r._pad[0]=r._pad[1]=r._pad[2]=0;
        if(sys_netctl(11,&r,sizeof(r))<0){ printf_min("netnsctl: pfwd add failed\n"); return 1; }
        return 0;
    }
    printf_min("usage: netnsctl [show|nat on|nat off|pfwd <host-port> <container-port>]\n");
    return 1;
}
