#ifndef POOS_PROC_USERCOPY_H
#define POOS_PROC_USERCOPY_H

#include "../types.h"

int copy_from_user(void* dst, const void* usrc, u32 n);
int copy_to_user(void* udst, const void* src, u32 n);
int strnlen_user(const char* ustr, u32 max, u32* out_len);

#endif
