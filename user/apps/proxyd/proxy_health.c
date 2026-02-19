#include "proxy_cache.h"
#include "../../libc_min/syscall.h"
int proxy_shed_load(void){
    struct memstat_u st;
    if(sys_memstat(&st,sizeof(st))<0) return 0;
    if(st.cat[0].cap > st.cat[0].used && (st.cat[0].cap-st.cat[0].used) < 6u*1024u*1024u) return 1;
    return 0;
}
