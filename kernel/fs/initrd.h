#ifndef POOS_FS_INITRD_H
#define POOS_FS_INITRD_H

#include "../types.h"

void initrd_set_region(u32 phys_start, u32 size);
const u8* initrd_data(void);
u32 initrd_size(void);

#endif
