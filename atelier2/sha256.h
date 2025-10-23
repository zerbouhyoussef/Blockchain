#ifndef MINI_SHA256_H
#define MINI_SHA256_H

#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>  

namespace SHA256 {
    using std::string;
    using std::stringstream;
    using std::hex;
    using std::setfill;
    using std::setw;

    typedef uint8_t  u8;
    typedef uint32_t u32;
    typedef uint64_t u64;

    static inline u32 rotr(u32 x, u32 n){ return (x>>n) | (x<<(32-n)); }
    static inline u32 ch(u32 x,u32 y,u32 z){ return (x&y)^(~x&z); }
    static inline u32 maj(u32 x,u32 y,u32 z){ return (x&y)^(x&z)^(y&z); }
    static inline u32 big0(u32 x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
    static inline u32 big1(u32 x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
    static inline u32 sm0(u32 x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
    static inline u32 sm1(u32 x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

    static const u32 K[64] = {
      0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
      0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
      0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
      0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
      0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
      0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
      0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
      0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    struct Ctx {
        u64 len = 0;
        u32 state[8] = {
            0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
            0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
        };
        u8 buf[64];
        size_t buf_len = 0;
    };

    static void process(Ctx& c, const u8* b) {
        u32 w[64];
        for (int i=0;i<16;++i)
            w[i]=(b[i*4]<<24)|(b[i*4+1]<<16)|(b[i*4+2]<<8)|b[i*4+3];
        for (int i=16;i<64;++i)
            w[i]=sm1(w[i-2])+w[i-7]+sm0(w[i-15])+w[i-16];
        u32 a=c.state[0],b2=c.state[1],c2=c.state[2],d=c.state[3];
        u32 e=c.state[4],f=c.state[5],g=c.state[6],h=c.state[7];
        for (int i=0;i<64;++i){
            u32 t1=h+big1(e)+ch(e,f,g)+K[i]+w[i];
            u32 t2=big0(a)+maj(a,b2,c2);
            h=g; g=f; f=e; e=d+t1; d=c2; c2=b2; b2=a; a=t1+t2;
        }
        u32 st[8]={a,b2,c2,d,e,f,g,h};
        for (int i=0;i<8;++i) c.state[i]+=st[i];
    }

    static void update(Ctx& c, const void* data, size_t len) {
        const u8* p=(const u8*)data;
        c.len += len;
        while (len) {
            size_t take = std::min(len, 64 - c.buf_len);   
            memcpy(c.buf + c.buf_len, p, take);
            c.buf_len += take; p += take; len -= take;
            if (c.buf_len == 64) {
                process(c, c.buf);
                c.buf_len = 0;
            }
        }
    }

    static std::string finalize(Ctx& c) {
        u64 bit_len = c.len * 8;
        u8 pad = 0x80;
        update(c, &pad, 1);
        u8 z = 0;
        while (c.buf_len != 56)
            update(c, &z, 1);
        u8 lenb[8];
        for (int i=0;i<8;++i) lenb[7-i] = (bit_len >> (8*i)) & 0xFF;
        update(c, lenb, 8);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i=0;i<8;++i)
            ss << std::setw(8) << c.state[i];
        return ss.str();
    }

    static std::string hash(const std::string& s) {
        Ctx c;
        update(c, s.data(), s.size());
        return finalize(c);
    }
}
#endif
