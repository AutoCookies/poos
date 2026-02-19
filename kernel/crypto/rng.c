#include "rng.h"
#include "crypto.h"
#include "../time/time.h"

struct drbg_state {
    u8 key[SHA256_DIGEST_LEN];
    u8 v[SHA256_DIGEST_LEN];
    u32 seeded;
    u64 reseed_counter;
};

static struct drbg_state g;

static u64 rdtsc_now(void) {
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}

static void drbg_update(const u8* provided, u32 len) {
    u8 tmp[SHA256_DIGEST_LEN];
    hmac_sha256(g.key, sizeof(g.key), g.v, sizeof(g.v), tmp);
    for (u32 i=0;i<SHA256_DIGEST_LEN;i++) g.v[i]=tmp[i];
    if (provided && len) {
        hmac_sha256(g.key, sizeof(g.key), provided, len, g.key);
    } else {
        hmac_sha256(g.key, sizeof(g.key), g.v, sizeof(g.v), g.key);
    }
    crypto_memwipe(tmp, sizeof(tmp));
}

void rng_init(void) {
    for (u32 i=0;i<SHA256_DIGEST_LEN;i++) { g.key[i]=0; g.v[i]=1; }
    g.seeded = 0;
    g.reseed_counter = 1;
}

void rng_mix_entropy(const void* data, u32 len) {
    u8 seed[64];
    u64 t = rdtsc_now();
    u32 tick = time_ticks();
    u32 n = 0;
    for (u32 i=0;i<8;i++) seed[n++] = (u8)(t >> (i*8));
    for (u32 i=0;i<4;i++) seed[n++] = (u8)(tick >> (i*8));
    const u8* p = (const u8*)data;
    for (u32 i=0;i<len && n<sizeof(seed);i++) seed[n++] = p[i];
    hmac_sha256(g.key, sizeof(g.key), seed, n, g.key);
    drbg_update(seed, n);
    g.seeded = 1;
    crypto_memwipe(seed, sizeof(seed));
}

int rng_ready(void) { return g.seeded; }

int rng_get_bytes(void* out, u32 len) {
    if (!g.seeded || !out) return -1;
    u8* o = (u8*)out;
    u8 block[SHA256_DIGEST_LEN];
    for (u32 i=0;i<len;) {
        hmac_sha256(g.key, sizeof(g.key), g.v, sizeof(g.v), block);
        for (u32 j=0;j<SHA256_DIGEST_LEN;j++) g.v[j]=block[j];
        u32 take = (len - i > SHA256_DIGEST_LEN) ? SHA256_DIGEST_LEN : (len - i);
        for (u32 j=0;j<take;j++) o[i+j]=block[j];
        i += take;
    }
    drbg_update(0,0);
    g.reseed_counter++;
    crypto_memwipe(block, sizeof(block));
    return 0;
}
