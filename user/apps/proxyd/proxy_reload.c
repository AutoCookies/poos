#include "proxy_cache.h"
#include "../../libc_min/syscall.h"
int proxy_reload_requested(void){
    int fd = sys_open("/tmp/proxyd.reload", O_RDONLY);
    if(fd<0) return 0;
    sys_close(fd);
    sys_unlink("/tmp/proxyd.reload");
    return 1;
}
