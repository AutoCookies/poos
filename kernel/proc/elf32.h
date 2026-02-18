#ifndef POOS_PROC_ELF32_H
#define POOS_PROC_ELF32_H

#include "../types.h"
#include "proc.h"

int elf32_load_image(struct proc* p, const u8* image, u32 size, u32* out_entry);

#endif
