#include "sock.h"
#include "../proc/usercopy.h"

int sys_socket(int domain,int type,int proto){ return sock_socket(domain,type,proto); }
int sys_bind(int fd,const void* usa,u32 len){ struct sockaddr_in_k sa; if(len<sizeof(sa)||copy_from_user(&sa,usa,sizeof(sa))<0) return -1; return sock_bind(fd,&sa); }
int sys_connect(int fd,const void* usa,u32 len){ struct sockaddr_in_k sa; if(len<sizeof(sa)||copy_from_user(&sa,usa,sizeof(sa))<0) return -1; return sock_connect(fd,&sa); }
int sys_send(int fd,const void* ubuf,u32 len,u32 flags){ (void)flags; u8 b[512]; if(len>sizeof(b)) len=sizeof(b); if(copy_from_user(b,ubuf,len)<0) return -1; return sock_send(fd,b,len); }
int sys_recv(int fd,void* ubuf,u32 len,u32 flags){ (void)flags; u8 b[512]; if(len>sizeof(b)) len=sizeof(b); int rc=sock_recv(fd,b,len); if(rc<0) return -1; if(rc && copy_to_user(ubuf,b,(u32)rc)<0) return -1; return rc; }
int sys_sendto(int fd,const void* ubuf,u32 len,u32 flags,const void* usa,u32 alen){ (void)flags; struct sockaddr_in_k sa; u8 b[512]; if(alen<sizeof(sa)||len>sizeof(b)) return -1; if(copy_from_user(&sa,usa,sizeof(sa))<0||copy_from_user(b,ubuf,len)<0) return -1; return sock_sendto(fd,b,len,&sa); }
int sys_recvfrom(int fd,void* ubuf,u32 len,u32 flags,void* usa,u32* ualen){ (void)flags; u8 b[512]; struct sockaddr_in_k sa; if(len>sizeof(b)) len=sizeof(b); int rc=sock_recvfrom(fd,b,len,&sa); if(rc<0) return -1; if(copy_to_user(ubuf,b,(u32)rc)<0) return -1; if(usa) copy_to_user(usa,&sa,sizeof(sa)); if(ualen){ u32 l=sizeof(sa); copy_to_user(ualen,&l,sizeof(l)); } return rc; }
int sys_sockclose(int fd){ return sock_close(fd); }
