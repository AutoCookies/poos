#include "auth.h"
#include "../vfs/vfs.h"
#include "../mem/mem.h"
static int str_eq(const char* a,const char* b){u32 i=0;while(a[i]&&b[i]){if(a[i]!=b[i])return 0;i++;}return a[i]==b[i];}
static int split3(const char* s,char* a,char* b,char* c){u32 i=0,j=0,k=0,p=0; while(s[p]&&s[p]!=':') a[i++]=s[p++]; if(s[p++]!=':') return -1; while(s[p]&&s[p]!=':') b[j++]=s[p++]; if(s[p++]!=':') return -1; while(s[p]&&s[p]!='\n'&&s[p]!=':') c[k++]=s[p++]; a[i]=b[j]=c[k]=0; return 0;}
static u32 atou(const char* s){u32 v=0; for(u32 i=0;s[i]>='0'&&s[i]<='9';i++) v=v*10+(u32)(s[i]-'0'); return v;}
int auth_lookup_user(const char* user, u32* uid, u32* gid){ struct file* f=0; if(vfs_open("/etc/passwd",0,&f)<0) return -1; char buf[1024]; int n=f->vnode->ops->read(f->vnode,0,buf,sizeof(buf)-1); file_put(f); if(n<=0) return -1; buf[n]=0; u32 i=0; while(i<(u32)n){ char line[128]; u32 l=0; while(i<(u32)n&&buf[i]!='\n'&&l<sizeof(line)-1) line[l++]=buf[i++]; if(i<(u32)n&&buf[i]=='\n') i++; line[l]=0; char a[32],b[32],c[32]; if(split3(line,a,b,c)==0 && str_eq(a,user)){ if(uid) *uid=atou(b); if(gid) *gid=atou(c); return 0; } } return -1; }
int auth_verify_password(const char* user, const char* pass){ (void)pass; return auth_lookup_user(user,0,0); }
