#include "crypto.h"

int crypto_consttime_eq(const void* a, const void* b, u32 len) {
    const u8* pa = (const u8*)a;
    const u8* pb = (const u8*)b;
    u8 diff = 0;
    for (u32 i = 0; i < len; ++i) diff |= (u8)(pa[i] ^ pb[i]);
    return diff == 0;
}
