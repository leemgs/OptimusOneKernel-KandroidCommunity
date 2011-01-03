

#include "../crypt_sha2.h"



#define SHR(x,n) (x >> n) 
#define ROTR(x,n,w) ((x >> n) | (x << (w - n))) 
#define ROTL(x,n,w) ((x << n) | (x >> (w - n))) 
#define ROTR32(x,n) ROTR(x,n,32) 
#define ROTL32(x,n) ROTL(x,n,32) 


#define Ch(x,y,z) ((x & y) ^ ((~x) & z))
#define Maj(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define Parity(x,y,z) (x ^ y ^ z)

#ifdef SHA1_SUPPORT

#define SHA1_MASK 0x0000000f
static const UINT32 SHA1_K[4] = {
    0x5a827999UL, 0x6ed9eba1UL, 0x8f1bbcdcUL, 0xca62c1d6UL
};
static const UINT32 SHA1_DefaultHashValue[5] = {
    0x67452301UL, 0xefcdab89UL, 0x98badcfeUL, 0x10325476UL, 0xc3d2e1f0UL
};
#endif 


#ifdef SHA256_SUPPORT

#define Zsigma_256_0(x) (ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22))
#define Zsigma_256_1(x) (ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25))
#define Sigma_256_0(x)  (ROTR32(x,7) ^ ROTR32(x,18) ^ SHR(x,3))
#define Sigma_256_1(x)  (ROTR32(x,17) ^ ROTR32(x,19) ^ SHR(x,10))

static const UINT32 SHA256_K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
static const UINT32 SHA256_DefaultHashValue[8] = {
    0x6a09e667UL, 0xbb67ae85UL, 0x3c6ef372UL, 0xa54ff53aUL,
    0x510e527fUL, 0x9b05688cUL, 0x1f83d9abUL, 0x5be0cd19UL
};
#endif 


#ifdef SHA1_SUPPORT

VOID SHA1_Init (
    IN  SHA1_CTX_STRUC *pSHA_CTX)
{
    NdisMoveMemory(pSHA_CTX->HashValue, SHA1_DefaultHashValue,
        sizeof(SHA1_DefaultHashValue));
    NdisZeroMemory(pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    pSHA_CTX->MessageLen = 0;
    pSHA_CTX->BlockLen   = 0;
} 



VOID SHA1_Hash (
    IN  SHA1_CTX_STRUC *pSHA_CTX)
{
    UINT32 W_i,t,s;
    UINT32 W[16];
    UINT32 a,b,c,d,e,T,f_t = 0;

    
    NdisMoveMemory(W, pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    for (W_i = 0; W_i < 16; W_i++)
        W[W_i] = cpu2be32(W[W_i]); 
        

    
    
    a = pSHA_CTX->HashValue[0];
    b = pSHA_CTX->HashValue[1];
    c = pSHA_CTX->HashValue[2];
    d = pSHA_CTX->HashValue[3];
    e = pSHA_CTX->HashValue[4];

    
    for (t = 0;t < 80;t++) {
        s = t & SHA1_MASK;
        if (t > 15) { 
            W[s] = (W[(s+13) & SHA1_MASK]) ^ (W[(s+8) & SHA1_MASK]) ^ (W[(s+2) & SHA1_MASK]) ^ W[s];
            W[s] = ROTL32(W[s],1);
        } 
        switch (t / 20) {
            case 0:
                 f_t = Ch(b,c,d);
                 break;
            case 1:
                 f_t = Parity(b,c,d);
                 break;
            case 2:
                 f_t = Maj(b,c,d);
                 break;
            case 3:
                 f_t = Parity(b,c,d);
                 break;
        } 
        T = ROTL32(a,5) + f_t + e + SHA1_K[t / 20] + W[s];
        e = d;
        d = c;
        c = ROTL32(b,30);
        b = a;
        a = T;
     } 

     
     pSHA_CTX->HashValue[0] += a;
     pSHA_CTX->HashValue[1] += b;
     pSHA_CTX->HashValue[2] += c;
     pSHA_CTX->HashValue[3] += d;
     pSHA_CTX->HashValue[4] += e;

    NdisZeroMemory(pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    pSHA_CTX->BlockLen = 0;
} 



VOID SHA1_Append (
    IN  SHA1_CTX_STRUC *pSHA_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    UINT diffLen   = 0;

    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pSHA_CTX->BlockLen + diffLen) <  SHA1_BLOCK_SIZE) {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
                Message + appendLen, diffLen);
            pSHA_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        }
        else
        {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
                Message + appendLen, SHA1_BLOCK_SIZE - pSHA_CTX->BlockLen);
            appendLen += (SHA1_BLOCK_SIZE - pSHA_CTX->BlockLen);
            pSHA_CTX->BlockLen = SHA1_BLOCK_SIZE;
            SHA1_Hash(pSHA_CTX);
        } 
    } 
    pSHA_CTX->MessageLen += MessageLen;
} 



VOID SHA1_End (
    IN  SHA1_CTX_STRUC *pSHA_CTX,
    OUT UINT8 DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    
    NdisFillMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 1, 0x80);

    
    if (pSHA_CTX->BlockLen > 55)
        SHA1_Hash(pSHA_CTX);
        

    
    message_length_bits = pSHA_CTX->MessageLen*8;
    message_length_bits = cpu2be64(message_length_bits);
    NdisMoveMemory(&pSHA_CTX->Block[56], &message_length_bits, 8);
    SHA1_Hash(pSHA_CTX);

    
    for (index = 0; index < 5;index++)
        pSHA_CTX->HashValue[index] = cpu2be32(pSHA_CTX->HashValue[index]);
        
    NdisMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA1_DIGEST_SIZE);
} 



VOID RT_SHA1 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[])
{

    SHA1_CTX_STRUC sha_ctx;

    NdisZeroMemory(&sha_ctx, sizeof(SHA1_CTX_STRUC));
    SHA1_Init(&sha_ctx);
    SHA1_Append(&sha_ctx, Message, MessageLen);
    SHA1_End(&sha_ctx, DigestMessage);
} 
#endif 


#ifdef SHA256_SUPPORT

VOID SHA256_Init (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    NdisMoveMemory(pSHA_CTX->HashValue, SHA256_DefaultHashValue,
        sizeof(SHA256_DefaultHashValue));
    NdisZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->MessageLen = 0;
    pSHA_CTX->BlockLen   = 0;
} 



VOID SHA256_Hash (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    UINT32 W_i,t;
    UINT32 W[64];
    UINT32 a,b,c,d,e,f,g,h,T1,T2;

    
    NdisMoveMemory(W, pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    for (W_i = 0; W_i < 16; W_i++)
        W[W_i] = cpu2be32(W[W_i]); 
        

    
    
    a = pSHA_CTX->HashValue[0];
    b = pSHA_CTX->HashValue[1];
    c = pSHA_CTX->HashValue[2];
    d = pSHA_CTX->HashValue[3];
    e = pSHA_CTX->HashValue[4];
    f = pSHA_CTX->HashValue[5];
    g = pSHA_CTX->HashValue[6];
    h = pSHA_CTX->HashValue[7];

    
    for (t = 0;t < 64;t++) {
        if (t > 15) 
            W[t] = Sigma_256_1(W[t-2]) + W[t-7] + Sigma_256_0(W[t-15]) + W[t-16];
            
        T1 = h + Zsigma_256_1(e) + Ch(e,f,g) + SHA256_K[t] + W[t];
        T2 = Zsigma_256_0(a) + Maj(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
     } 

     
     pSHA_CTX->HashValue[0] += a;
     pSHA_CTX->HashValue[1] += b;
     pSHA_CTX->HashValue[2] += c;
     pSHA_CTX->HashValue[3] += d;
     pSHA_CTX->HashValue[4] += e;
     pSHA_CTX->HashValue[5] += f;
     pSHA_CTX->HashValue[6] += g;
     pSHA_CTX->HashValue[7] += h;

    NdisZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->BlockLen = 0;
} 



VOID SHA256_Append (
    IN  SHA256_CTX_STRUC *pSHA_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    UINT diffLen   = 0;

    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pSHA_CTX->BlockLen + diffLen) <  SHA256_BLOCK_SIZE) {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
                Message + appendLen, diffLen);
            pSHA_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        }
        else
        {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
                Message + appendLen, SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            appendLen += (SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            pSHA_CTX->BlockLen = SHA256_BLOCK_SIZE;
            SHA256_Hash(pSHA_CTX);
        } 
    } 
    pSHA_CTX->MessageLen += MessageLen;
} 



VOID SHA256_End (
    IN  SHA256_CTX_STRUC *pSHA_CTX,
    OUT UINT8 DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    
    NdisFillMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 1, 0x80);

    
    if (pSHA_CTX->BlockLen > 55)
        SHA256_Hash(pSHA_CTX);
        

    
    message_length_bits = pSHA_CTX->MessageLen*8;
    message_length_bits = cpu2be64(message_length_bits);
    NdisMoveMemory(&pSHA_CTX->Block[56], &message_length_bits, 8);
    SHA256_Hash(pSHA_CTX);

    
    for (index = 0; index < 8;index++)
        pSHA_CTX->HashValue[index] = cpu2be32(pSHA_CTX->HashValue[index]);
        
    NdisMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA256_DIGEST_SIZE);
} 



VOID RT_SHA256 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[])
{
    SHA256_CTX_STRUC sha_ctx;

    NdisZeroMemory(&sha_ctx, sizeof(SHA256_CTX_STRUC));
    SHA256_Init(&sha_ctx);
    SHA256_Append(&sha_ctx, Message, MessageLen);
    SHA256_End(&sha_ctx, DigestMessage);
} 
#endif 


