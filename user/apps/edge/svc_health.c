#include "edge_internal.h"

static int parse_i(const char* s){ int n=0,neg=0,i=0; if(s[0]=='-'){neg=1;i=1;} for(;s[i]>='0'&&s[i]<='9';i++) n=n*10+(s[i]-'0'); return neg?-n:n; }

int edge_status_write(const struct edge_service_info* svcs, int n, const struct edge_health_state* h){
    int fd = sys_open("/tmp/edge.status.tmp", O_WRONLY|O_CREAT|O_TRUNC);
    if(fd<0) return -1;
    char line[160];
    int l = 0;
    line[l++]='H'; line[l++]=' '; line[l++]='0'+h->status; line[l++]=' ';
    line[l++]='0'+(h->proxyd_up?1:0); line[l++]=' '; line[l++]='0'+(h->restart_storm?1:0); line[l++]=' '; line[l++]='0'+(h->mem_low?1:0); line[l++]=' ';
    unsigned int t=h->low_mem_ticks; char tmp[12]; int tp=0; do{ tmp[tp++]=(char)('0'+(t%10)); t/=10; }while(t&&tp<11); for(int i=tp-1;i>=0;i--) line[l++]=tmp[i]; line[l++]='\n';
    sys_write(fd,line,l);
    for(int i=0;i<n;i++){
        l=0; line[l++]='S'; line[l++]=' ';
        for(int j=0;svcs[i].name[j]&&l<150;j++) line[l++]=svcs[i].name[j];
        line[l++]=' '; 
        int vals[7]={svcs[i].pid,svcs[i].state,svcs[i].restarts,svcs[i].last_exit,svcs[i].restart_minute,(int)svcs[i].backoff_ms,(int)svcs[i].last_crash_at};
        for(int k=0;k<7;k++){
            int v=vals[k],neg=0; char num[12]; int np=0; if(v<0){neg=1;v=-v;} if(neg) line[l++]='-'; do{ num[np++]=(char)('0'+(v%10)); v/=10; }while(v&&np<11); for(int x=np-1;x>=0;x--) line[l++]=num[x]; line[l++]=(k==6)?'\n':' ';
        }
        sys_write(fd,line,l);
    }
    sys_close(fd);
    sys_rename("/tmp/edge.status.tmp","/tmp/edge.status");
    return 0;
}

int edge_status_read(struct edge_service_info* svcs, int cap, int* n, struct edge_health_state* h){
    int fd = sys_open("/tmp/edge.status", O_RDONLY);
    if(fd<0) return -1;
    char buf[1024]; int r=sys_read(fd,buf,sizeof(buf)-1); sys_close(fd); if(r<=0) return -1; buf[r]=0;
    int i=0,idx=0;
    h->status=2; h->proxyd_up=0;
    while(i<r){
        int j=i; while(j<r&&buf[j]!='\n') j++; if(j<=i){ i=j+1; continue; }
        if(buf[i]=='H'){
            int p=i+2; h->status=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++; h->proxyd_up=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++; h->restart_storm=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++; h->mem_low=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++; h->low_mem_ticks=(unsigned int)parse_i(&buf[p]);
        } else if(buf[i]=='S' && idx<cap){
            int p=i+2,sp=0; while(p<j&&buf[p]!=' '&&sp<15) svcs[idx].name[sp++]=buf[p++]; svcs[idx].name[sp]=0; p++;
            svcs[idx].pid=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].state=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].restarts=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].last_exit=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].restart_minute=parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].backoff_ms=(unsigned int)parse_i(&buf[p]); while(p<j&&buf[p]!=' ')p++; p++;
            svcs[idx].last_crash_at=(unsigned int)parse_i(&buf[p]);
            idx++;
        }
        i=j+1;
    }
    *n=idx;
    return 0;
}
