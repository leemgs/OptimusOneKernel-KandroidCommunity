

#ifndef __CRYPT_SHA2_H__
#define __CRYPT_SHA2_H__

#ifdef CRYPT_TESTPLAN
#include "crypt_testplan.h"
#else
#include "rt_config.h"
#endif 


#define SHA1_SUPPORT
#define SHA256_SUPPORT

#ifdef SHA1_SUPPORT
#define SHA1_BLOCK_SIZE    64 
#define SHA1_DIGEST_SIZE   20 
typedef struct _SHA1_CTX_STRUC {
    UINT32 HashValue[5];  
    UINT64 MessageLen;    
    UINT8  Block[SHA1_BLOCK_SIZE];
    UINT   BlockLen;
} SHA1_CTX_STRUC, *PSHA1_CTX_STRUC;

VOID SHA1_Init (
    IN  SHA1_CTX_STRUC *pSHA_CTX);
VOID SHA1_Hash (
    IN  SHA1_CTX_STRUC *pSHA_CTX);
VOID SHA1_Append (
    IN  SHA1_CTX_STRUC *pSHA_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen);
VOID SHA1_End (
    IN  SHA1_CTX_STRUC *pSHA_CTX,
    OUT UINT8 DigestMessage[]);
VOID RT_SHA1 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[]);
#endif 

#ifdef SHA256_SUPPORT
#define SHA256_BLOCK_SIZE   64 
#define SHA256_DIGEST_SIZE  32 
typedef struct _SHA256_CTX_STRUC {
    UINT32 HashValue[8];  
    UINT64 MessageLen;    
    UINT8  Block[SHA256_BLOCK_SIZE];
    UINT   BlockLen;
} SHA256_CTX_STRUC, *PSHA256_CTX_STRUC;

VOID SHA256_Init (
    IN  SHA256_CTX_STRUC *pSHA_CTX);
VOID SHA256_Hash (
    IN  SHA256_CTX_STRUC *pSHA_CTX);
VOID SHA256_Append (
    IN  SHA256_CTX_STRUC *pSHA_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen);
VOID SHA256_End (
    IN  SHA256_CTX_STRUC *pSHA_CTX,
    OUT UINT8 DigestMessage[]);
VOID RT_SHA256 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[]);
#endif 

#endif 
