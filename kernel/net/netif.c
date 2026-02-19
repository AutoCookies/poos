#include "netif.h"
static netif_t* g;
netif_t* netif_default(void){ return g; }
void netif_set_default(netif_t* n){ g=n; }
