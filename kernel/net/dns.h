#ifndef POOS_DNS_H
#define POOS_DNS_H
#include "../types.h"
int dns_lookup_a(const char* host, u32* out_ip);
#endif
