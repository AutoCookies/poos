#ifndef POOS_RNG_H
#define POOS_RNG_H

#include "../types.h"

void rng_init(void);
void rng_mix_entropy(const void* data, u32 len);
int rng_ready(void);
int rng_get_bytes(void* out, u32 len);

#endif
