

#include "../crypt_md5.h"


#ifdef MD5_SUPPORT

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTL(x,n,w) ((x << n) | (x >> (w - n)))
#define ROTL32(x,n) ROTL(x,n,32) 

#define ROUND1(a, b, c, d, x, s, ac) {          \
    (a) += F((b),(c),(d)) + (x) + (UINT32)(ac); \
    (a)  = ROTL32((a),(s));                     \
    (a) += (b);                                 \
}
#define ROUND2(a, b, c, d, x, s, ac) {          \
    (a) += G((b),(c),(d)) + (x) + (UINT32)(ac); \
    (a)  = ROTL32((a),(s));                     \
    (a) += (b);                                 \
}
#define ROUND3(a, b, c, d, x, s, ac) {          \
    (a) += H((b),(c),(d)) + (x) + (UINT32)(ac); \
    (a)  = ROTL32((a),(s));                     \
    (a) += (b);                                 \
}
#define ROUND4(a, b, c, d, x, s, ac) {          \
    (a) += I((b),(c),(d)) + (x) + (UINT32)(ac); \
    (a)  = ROTL32((a),(s));                     \
    (a) += (b);                                 \
}
static const UINT32 MD5_DefaultHashValue[4] = {
    0x67452301UL, 0xefcdab89UL, 0x98badcfeUL, 0x10325476UL
};
#endif 


#ifdef MD5_SUPPORT

VOID MD5_Init (
    IN  MD5_CTX_STRUC *pMD5_CTX)
{
    NdisMoveMemory(pMD5_CTX->HashValue, MD5_DefaultHashValue,
        sizeof(MD5_DefaultHashValue));
    NdisZeroMemory(pMD5_CTX->Block, MD5_BLOCK_SIZE);
    pMD5_CTX->BlockLen   = 0;
    pMD5_CTX->MessageLen = 0;
} 



VOID MD5_Hash (
    IN  MD5_CTX_STRUC *pMD5_CTX)
{
    UINT32 X_i;
    UINT32 X[16];
    UINT32 a,b,c,d;

    
    NdisMoveMemory(X, pMD5_CTX->Block, MD5_BLOCK_SIZE);
    for (X_i = 0; X_i < 16; X_i++)
        X[X_i] = cpu2le32(X[X_i]); 
        

    
    
    a = pMD5_CTX->HashValue[0];
    b = pMD5_CTX->HashValue[1];
    c = pMD5_CTX->HashValue[2];
    d = pMD5_CTX->HashValue[3];

    
    ROUND1(a, b, c, d, X[ 0],  7, 0xd76aa478); 
    ROUND1(d, a, b, c, X[ 1], 12, 0xe8c7b756); 
    ROUND1(c, d, a, b, X[ 2], 17, 0x242070db); 
    ROUND1(b, c, d, a, X[ 3], 22, 0xc1bdceee); 
    ROUND1(a, b, c, d, X[ 4],  7, 0xf57c0faf); 
    ROUND1(d, a, b, c, X[ 5], 12, 0x4787c62a); 
    ROUND1(c, d, a, b, X[ 6], 17, 0xa8304613); 
    ROUND1(b, c, d, a, X[ 7], 22, 0xfd469501); 
    ROUND1(a, b, c, d, X[ 8],  7, 0x698098d8); 
    ROUND1(d, a, b, c, X[ 9], 12, 0x8b44f7af); 
    ROUND1(c, d, a, b, X[10], 17, 0xffff5bb1); 
    ROUND1(b, c, d, a, X[11], 22, 0x895cd7be); 
    ROUND1(a, b, c, d, X[12],  7, 0x6b901122); 
    ROUND1(d, a, b, c, X[13], 12, 0xfd987193); 
    ROUND1(c, d, a, b, X[14], 17, 0xa679438e); 
    ROUND1(b, c, d, a, X[15], 22, 0x49b40821); 

    
    ROUND2(a, b, c, d, X[ 1],  5, 0xf61e2562); 
    ROUND2(d, a, b, c, X[ 6],  9, 0xc040b340); 
    ROUND2(c, d, a, b, X[11], 14, 0x265e5a51); 
    ROUND2(b, c, d, a, X[ 0], 20, 0xe9b6c7aa); 
    ROUND2(a, b, c, d, X[ 5],  5, 0xd62f105d); 
    ROUND2(d, a, b, c, X[10],  9,  0x2441453); 
    ROUND2(c, d, a, b, X[15], 14, 0xd8a1e681); 
    ROUND2(b, c, d, a, X[ 4], 20, 0xe7d3fbc8); 
    ROUND2(a, b, c, d, X[ 9],  5, 0x21e1cde6); 
    ROUND2(d, a, b, c, X[14],  9, 0xc33707d6); 
    ROUND2(c, d, a, b, X[ 3], 14, 0xf4d50d87); 
    ROUND2(b, c, d, a, X[ 8], 20, 0x455a14ed); 
    ROUND2(a, b, c, d, X[13],  5, 0xa9e3e905); 
    ROUND2(d, a, b, c, X[ 2],  9, 0xfcefa3f8); 
    ROUND2(c, d, a, b, X[ 7], 14, 0x676f02d9); 
    ROUND2(b, c, d, a, X[12], 20, 0x8d2a4c8a); 

    
    ROUND3(a, b, c, d, X[ 5],  4, 0xfffa3942); 
    ROUND3(d, a, b, c, X[ 8], 11, 0x8771f681); 
    ROUND3(c, d, a, b, X[11], 16, 0x6d9d6122); 
    ROUND3(b, c, d, a, X[14], 23, 0xfde5380c); 
    ROUND3(a, b, c, d, X[ 1],  4, 0xa4beea44); 
    ROUND3(d, a, b, c, X[ 4], 11, 0x4bdecfa9); 
    ROUND3(c, d, a, b, X[ 7], 16, 0xf6bb4b60); 
    ROUND3(b, c, d, a, X[10], 23, 0xbebfbc70); 
    ROUND3(a, b, c, d, X[13],  4, 0x289b7ec6); 
    ROUND3(d, a, b, c, X[ 0], 11, 0xeaa127fa); 
    ROUND3(c, d, a, b, X[ 3], 16, 0xd4ef3085); 
    ROUND3(b, c, d, a, X[ 6], 23,  0x4881d05); 
    ROUND3(a, b, c, d, X[ 9],  4, 0xd9d4d039); 
    ROUND3(d, a, b, c, X[12], 11, 0xe6db99e5); 
    ROUND3(c, d, a, b, X[15], 16, 0x1fa27cf8); 
    ROUND3(b, c, d, a, X[ 2], 23, 0xc4ac5665); 

    
    ROUND4(a, b, c, d, X[ 0],  6, 0xf4292244); 
    ROUND4(d, a, b, c, X[ 7], 10, 0x432aff97); 
    ROUND4(c, d, a, b, X[14], 15, 0xab9423a7); 
    ROUND4(b, c, d, a, X[ 5], 21, 0xfc93a039); 
    ROUND4(a, b, c, d, X[12],  6, 0x655b59c3); 
    ROUND4(d, a, b, c, X[ 3], 10, 0x8f0ccc92); 
    ROUND4(c, d, a, b, X[10], 15, 0xffeff47d); 
    ROUND4(b, c, d, a, X[ 1], 21, 0x85845dd1); 
    ROUND4(a, b, c, d, X[ 8],  6, 0x6fa87e4f); 
    ROUND4(d, a, b, c, X[15], 10, 0xfe2ce6e0); 
    ROUND4(c, d, a, b, X[ 6], 15, 0xa3014314); 
    ROUND4(b, c, d, a, X[13], 21, 0x4e0811a1); 
    ROUND4(a, b, c, d, X[ 4],  6, 0xf7537e82); 
    ROUND4(d, a, b, c, X[11], 10, 0xbd3af235); 
    ROUND4(c, d, a, b, X[ 2], 15, 0x2ad7d2bb); 
    ROUND4(b, c, d, a, X[ 9], 21, 0xeb86d391); 

    
    pMD5_CTX->HashValue[0] += a;
    pMD5_CTX->HashValue[1] += b;
    pMD5_CTX->HashValue[2] += c;
    pMD5_CTX->HashValue[3] += d;

    NdisZeroMemory(pMD5_CTX->Block, MD5_BLOCK_SIZE);
    pMD5_CTX->BlockLen = 0;
} 



VOID MD5_Append (
    IN  MD5_CTX_STRUC *pMD5_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    UINT diffLen = 0;

    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pMD5_CTX->BlockLen + diffLen) < MD5_BLOCK_SIZE) {
            NdisMoveMemory(pMD5_CTX->Block + pMD5_CTX->BlockLen,
                Message + appendLen, diffLen);
            pMD5_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        }
        else
        {
            NdisMoveMemory(pMD5_CTX->Block + pMD5_CTX->BlockLen,
                Message + appendLen, MD5_BLOCK_SIZE - pMD5_CTX->BlockLen);
            appendLen += (MD5_BLOCK_SIZE - pMD5_CTX->BlockLen);
            pMD5_CTX->BlockLen = MD5_BLOCK_SIZE;
            MD5_Hash(pMD5_CTX);
        } 
    } 
    pMD5_CTX->MessageLen += MessageLen;
} 



VOID MD5_End (
    IN  MD5_CTX_STRUC *pMD5_CTX,
    OUT UINT8 DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    
    NdisFillMemory(pMD5_CTX->Block + pMD5_CTX->BlockLen, 1, 0x80);

    
    if (pMD5_CTX->BlockLen > 55)
        MD5_Hash(pMD5_CTX);
        

    
    message_length_bits = pMD5_CTX->MessageLen*8;
    message_length_bits = cpu2le64(message_length_bits);
    NdisMoveMemory(&pMD5_CTX->Block[56], &message_length_bits, 8);
    MD5_Hash(pMD5_CTX);

    
    for (index = 0; index < 4;index++)
        pMD5_CTX->HashValue[index] = cpu2le32(pMD5_CTX->HashValue[index]);
        
    NdisMoveMemory(DigestMessage, pMD5_CTX->HashValue, MD5_DIGEST_SIZE);
} 



VOID RT_MD5 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[])
{
    MD5_CTX_STRUC md5_ctx;

    NdisZeroMemory(&md5_ctx, sizeof(MD5_CTX_STRUC));
    MD5_Init(&md5_ctx);
    MD5_Append(&md5_ctx, Message, MessageLen);
    MD5_End(&md5_ctx, DigestMessage);
} 

#endif 


