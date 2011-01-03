

#include "../crypt_hmac.h"


#ifdef HMAC_SHA1_SUPPORT

VOID HMAC_SHA1 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    SHA1_CTX_STRUC sha_ctx1;
    SHA1_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA1_BLOCK_SIZE];
    UINT8 Digest[SHA1_DIGEST_SIZE];
    UINT index;

    NdisZeroMemory(&sha_ctx1, sizeof(SHA1_CTX_STRUC));
    NdisZeroMemory(&sha_ctx2, sizeof(SHA1_CTX_STRUC));
    
    NdisZeroMemory(K0, SHA1_BLOCK_SIZE);
    if (KeyLen <= SHA1_BLOCK_SIZE)
        NdisMoveMemory(K0, Key, KeyLen);
    else
        RT_SHA1(Key, KeyLen, K0);
    

    
    
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        

    SHA1_Init(&sha_ctx1);
    
    SHA1_Append(&sha_ctx1, K0, sizeof(K0));
    
    SHA1_Append(&sha_ctx1, Message, MessageLen);
    SHA1_End(&sha_ctx1, Digest);

    
    
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        

    SHA1_Init(&sha_ctx2);
    
    SHA1_Append(&sha_ctx2, K0, sizeof(K0));
    
    SHA1_Append(&sha_ctx2, Digest, SHA1_DIGEST_SIZE);
    SHA1_End(&sha_ctx2, Digest);

    if (MACLen > SHA1_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest, SHA1_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);
} 
#endif 


#ifdef HMAC_SHA256_SUPPORT

VOID HMAC_SHA256 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    SHA256_CTX_STRUC sha_ctx1;
    SHA256_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA256_BLOCK_SIZE];
    UINT8 Digest[SHA256_DIGEST_SIZE];
    UINT index;

    NdisZeroMemory(&sha_ctx1, sizeof(SHA256_CTX_STRUC));
    NdisZeroMemory(&sha_ctx2, sizeof(SHA256_CTX_STRUC));
    
    NdisZeroMemory(K0, SHA256_BLOCK_SIZE);
    if (KeyLen <= SHA256_BLOCK_SIZE) {
        NdisMoveMemory(K0, Key, KeyLen);
    } else {
        RT_SHA256(Key, KeyLen, K0);
    }

    
    
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        

    SHA256_Init(&sha_ctx1);
    
    SHA256_Append(&sha_ctx1, K0, sizeof(K0));
    
    SHA256_Append(&sha_ctx1, Message, MessageLen);
    SHA256_End(&sha_ctx1, Digest);

    
    
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        

    SHA256_Init(&sha_ctx2);
    
    SHA256_Append(&sha_ctx2, K0, sizeof(K0));
    
    SHA256_Append(&sha_ctx2, Digest, SHA256_DIGEST_SIZE);
    SHA256_End(&sha_ctx2, Digest);

    if (MACLen > SHA256_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest,SHA256_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);

} 
#endif 


#ifdef HMAC_MD5_SUPPORT

VOID HMAC_MD5(
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    MD5_CTX_STRUC md5_ctx1;
    MD5_CTX_STRUC md5_ctx2;
    UINT8 K0[MD5_BLOCK_SIZE];
    UINT8 Digest[MD5_DIGEST_SIZE];
    UINT index;

    NdisZeroMemory(&md5_ctx1, sizeof(MD5_CTX_STRUC));
    NdisZeroMemory(&md5_ctx2, sizeof(MD5_CTX_STRUC));
    
    NdisZeroMemory(K0, MD5_BLOCK_SIZE);
    if (KeyLen <= MD5_BLOCK_SIZE) {
        NdisMoveMemory(K0, Key, KeyLen);
    } else {
        RT_MD5(Key, KeyLen, K0);
    }

    
    
    for (index = 0; index < MD5_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        

    MD5_Init(&md5_ctx1);
    
    MD5_Append(&md5_ctx1, K0, sizeof(K0));
    
    MD5_Append(&md5_ctx1, Message, MessageLen);
    MD5_End(&md5_ctx1, Digest);

    
    
    for (index = 0; index < MD5_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        

    MD5_Init(&md5_ctx2);
    
    MD5_Append(&md5_ctx2, K0, sizeof(K0));
    
    MD5_Append(&md5_ctx2, Digest, MD5_DIGEST_SIZE);
    MD5_End(&md5_ctx2, Digest);

    if (MACLen > MD5_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest, MD5_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);
} 
#endif 


