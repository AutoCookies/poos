#include "crypto.h"

int hkdf_sha256_extract(const u8* salt, u32 salt_len, const u8* ikm, u32 ikm_len, u8 prk[SHA256_DIGEST_LEN]) {
    u8 zeros[SHA256_DIGEST_LEN];
    for (u32 i=0;i<SHA256_DIGEST_LEN;i++) zeros[i]=0;
    hmac_sha256(salt ? salt : zeros, salt ? salt_len : SHA256_DIGEST_LEN, ikm, ikm_len, prk);
    crypto_memwipe(zeros, sizeof(zeros));
    return 0;
}

int hkdf_sha256_expand(const u8 prk[SHA256_DIGEST_LEN], const u8* info, u32 info_len, u8* out, u32 out_len) {
    if (!out || out_len == 0 || out_len > 255U * SHA256_DIGEST_LEN) return -1;
    u8 t[SHA256_DIGEST_LEN];
    u8 buf[SHA256_DIGEST_LEN + 128 + 1];
    if (info_len > 128) return -1;
    u32 pos = 0;
    u8 ctr = 1;
    u32 tlen = 0;
    while (pos < out_len) {
        u32 b = 0;
        for (u32 i=0;i<tlen;i++) buf[b++] = t[i];
        for (u32 i=0;i<info_len;i++) buf[b++] = info[i];
        buf[b++] = ctr++;
        hmac_sha256(prk, SHA256_DIGEST_LEN, buf, b, t);
        tlen = SHA256_DIGEST_LEN;
        u32 take = (out_len - pos > SHA256_DIGEST_LEN) ? SHA256_DIGEST_LEN : (out_len - pos);
        for (u32 i=0;i<take;i++) out[pos+i]=t[i];
        pos += take;
    }
    crypto_memwipe(t, sizeof(t));
    crypto_memwipe(buf, sizeof(buf));
    return 0;
}
