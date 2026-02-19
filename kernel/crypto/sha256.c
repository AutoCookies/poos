#include "crypto.h"

typedef struct {
    u32 h[8];
    u64 bits;
    u8 buf[64];
    u32 used;
} sha256_ctx;

static const u32 K[64] = {
  0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
  0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
  0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
  0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
  0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
  0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
  0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
  0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static u32 rr(u32 x, u32 n){ return (x >> n) | (x << (32 - n)); }
static u32 be32(const u8* p){ return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3]; }
static void st32(u8* p, u32 v){ p[0]=(u8)(v>>24); p[1]=(u8)(v>>16); p[2]=(u8)(v>>8); p[3]=(u8)v; }

static void compress(sha256_ctx* c, const u8 blk[64]) {
    u32 w[64];
    for (u32 i = 0; i < 16; ++i) w[i] = be32(blk + 4*i);
    for (u32 i = 16; i < 64; ++i) {
        u32 s0 = rr(w[i-15],7) ^ rr(w[i-15],18) ^ (w[i-15] >> 3);
        u32 s1 = rr(w[i-2],17) ^ rr(w[i-2],19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    u32 a=c->h[0],b=c->h[1],d,e,f,g,h,cc;
    cc=c->h[2]; d=c->h[3]; e=c->h[4]; f=c->h[5]; g=c->h[6]; h=c->h[7];
    for (u32 i = 0; i < 64; ++i) {
        u32 S1 = rr(e,6) ^ rr(e,11) ^ rr(e,25);
        u32 ch = (e & f) ^ ((~e) & g);
        u32 t1 = h + S1 + ch + K[i] + w[i];
        u32 S0 = rr(a,2) ^ rr(a,13) ^ rr(a,22);
        u32 maj = (a & b) ^ (a & cc) ^ (b & cc);
        u32 t2 = S0 + maj;
        h=g; g=f; f=e; e=d+t1; d=cc; cc=b; b=a; a=t1+t2;
    }
    c->h[0]+=a; c->h[1]+=b; c->h[2]+=cc; c->h[3]+=d;
    c->h[4]+=e; c->h[5]+=f; c->h[6]+=g; c->h[7]+=h;
}

static void init(sha256_ctx* c){
    c->h[0]=0x6a09e667U; c->h[1]=0xbb67ae85U; c->h[2]=0x3c6ef372U; c->h[3]=0xa54ff53aU;
    c->h[4]=0x510e527fU; c->h[5]=0x9b05688cU; c->h[6]=0x1f83d9abU; c->h[7]=0x5be0cd19U;
    c->bits=0; c->used=0;
}
static void update(sha256_ctx* c,const u8* data,u32 len){
    c->bits += (u64)len * 8ULL;
    for(u32 i=0;i<len;i++){
        c->buf[c->used++] = data[i];
        if(c->used==64){ compress(c,c->buf); c->used=0; }
    }
}
static void final(sha256_ctx* c,u8 out[32]){
    c->buf[c->used++] = 0x80;
    if(c->used > 56){ while(c->used<64) c->buf[c->used++]=0; compress(c,c->buf); c->used=0; }
    while(c->used<56) c->buf[c->used++]=0;
    for(int i=7;i>=0;--i) c->buf[c->used++] = (u8)(c->bits >> (i*8));
    compress(c,c->buf);
    for(u32 i=0;i<8;++i) st32(out+4*i,c->h[i]);
    crypto_memwipe(c,sizeof(*c));
}

void sha256(const u8* data, u32 len, u8 out[SHA256_DIGEST_LEN]){
    sha256_ctx c; init(&c); update(&c,data,len); final(&c,out);
}
