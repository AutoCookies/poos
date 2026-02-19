#ifndef POOS_BIO_H
#define POOS_BIO_H
#include "blkdev.h"

int bio_read_bytes(struct blkdev* dev, u32 off, void* buf, u32 len);
int bio_write_bytes(struct blkdev* dev, u32 off, const void* buf, u32 len);

#endif
