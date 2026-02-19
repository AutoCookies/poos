#include "crypto.h"

void hmac_sha256(const u8* key, u32 key_len, const u8* data, u32 data_len, u8 out[SHA256_DIGEST_LEN]) {
    u8 k0[64];
    u8 tmp[SHA256_DIGEST_LEN];
    for (u32 i=0;i<64;i++) k0[i]=0;
    if (key_len > 64) {
        sha256(key, key_len, tmp);
        for (u32 i=0;i<SHA256_DIGEST_LEN;i++) k0[i]=tmp[i];
    } else {
        for (u32 i=0;i<key_len;i++) k0[i]=key[i];
    }
    u8 ipad[64], opad[64];
    for (u32 i=0;i<64;i++) { ipad[i]=k0[i]^0x36U; opad[i]=k0[i]^0x5cU; }

    u8 inner[64 + data_len];
    for (u32 i=0;i<64;i++) inner[i]=ipad[i];
    for (u32 i=0;i<data_len;i++) inner[64+i]=data[i];
    sha256(inner, 64 + data_len, tmp);

    u8 outer[64 + SHA256_DIGEST_LEN];
    for (u32 i=0;i<64;i++) outer[i]=opad[i];
    for (u32 i=0;i<SHA256_DIGEST_LEN;i++) outer[64+i]=tmp[i];
    sha256(outer, sizeof(outer), out);

    crypto_memwipe(tmp, sizeof(tmp));
    crypto_memwipe(k0, sizeof(k0));
    crypto_memwipe(ipad, sizeof(ipad));
    crypto_memwipe(opad, sizeof(opad));
    crypto_memwipe(inner, sizeof(inner));
    crypto_memwipe(outer, sizeof(outer));
}
