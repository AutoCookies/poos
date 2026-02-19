#ifndef POOS_CRYPTO_H
#define POOS_CRYPTO_H

#include "../types.h"

void crypto_memwipe(void* ptr, u32 len);
int crypto_consttime_eq(const void* a, const void* b, u32 len);

#define SHA256_DIGEST_LEN 32
void sha256(const u8* data, u32 len, u8 out[SHA256_DIGEST_LEN]);

void hmac_sha256(const u8* key, u32 key_len, const u8* data, u32 data_len, u8 out[SHA256_DIGEST_LEN]);

int hkdf_sha256_extract(const u8* salt, u32 salt_len, const u8* ikm, u32 ikm_len, u8 prk[SHA256_DIGEST_LEN]);
int hkdf_sha256_expand(const u8 prk[SHA256_DIGEST_LEN], const u8* info, u32 info_len, u8* out, u32 out_len);

#endif
