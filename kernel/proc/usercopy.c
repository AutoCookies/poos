#include "usercopy.h"
#include "proc.h"
#include "../mem/vmm.h"

static bool user_ptr_ok(const void* p, u32 n) {
    u32 a = (u32)p;
    if (a >= KERNEL_BASE || a + n >= KERNEL_BASE) return false;
    for (u32 off = 0; off < n; off += 4096U) {
        if (!vmm_user_accessible(a + off)) return false;
    }
    return n == 0 || vmm_user_accessible(a + n - 1U);
}

int copy_from_user(void* dst, const void* usrc, u32 n) {
    if (!user_ptr_ok(usrc, n)) return -1;
    for (u32 i = 0; i < n; ++i) ((u8*)dst)[i] = ((const u8*)usrc)[i];
    return 0;
}
int copy_to_user(void* udst, const void* src, u32 n) {
    if (!user_ptr_ok(udst, n)) return -1;
    for (u32 i = 0; i < n; ++i) ((u8*)udst)[i] = ((const u8*)src)[i];
    return 0;
}
int strnlen_user(const char* ustr, u32 max, u32* out_len) {
    if (!user_ptr_ok(ustr, 1)) return -1;
    for (u32 i=0;i<max;++i){ if(!user_ptr_ok(ustr+i,1)) return -1; if(ustr[i]=='\0'){*out_len=i;return 0;}}
    return -1;
}
