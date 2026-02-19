#include "crypto.h"

void crypto_memwipe(void* ptr, u32 len) {
    volatile u8* p = (volatile u8*)ptr;
    for (u32 i = 0; i < len; ++i) {
        p[i] = 0;
    }
}
