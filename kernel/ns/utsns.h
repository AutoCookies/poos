#ifndef POOS_NS_UTSNS_H
#define POOS_NS_UTSNS_H
#include "../types.h"
struct uts_ns { u32 id; u32 refcnt; char hostname[64]; };
struct uts_ns* utsns_create(const char* host);
void utsns_get(struct uts_ns* ns);
void utsns_put(struct uts_ns* ns);
int utsns_sethostname(struct uts_ns* ns,const char* host);
#endif
