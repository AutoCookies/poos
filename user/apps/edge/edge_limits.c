#include "edge_internal.h"

int edge_mem_free_low(void){
    struct memstat_u st;
    if(sys_memstat(&st, (int)sizeof(st)) < 0) return 0;
    unsigned int cap = st.cat[0].cap;
    unsigned int used = st.cat[0].used;
    if(cap == 0) return 0;
    return (cap > used) && ((cap - used) < (8u * 1024u * 1024u));
}
