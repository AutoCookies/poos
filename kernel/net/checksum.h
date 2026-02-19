#ifndef POOS_CSUM_H
#define POOS_CSUM_H
#include "../types.h"
u16 net_checksum(const void* data, u32 len);
u16 net_checksum_udp(u32 sip, u32 dip, u8 proto, const void* data, u32 len);
#endif
