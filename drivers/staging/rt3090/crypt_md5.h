

#ifndef __CRYPT_MD5_H__
#define __CRYPT_MD5_H__

#ifdef CRYPT_TESTPLAN
#include "crypt_testplan.h"
#else
#include "rt_config.h"
#endif 


#define MD5_SUPPORT

#ifdef MD5_SUPPORT
#define MD5_BLOCK_SIZE    64 
#define MD5_DIGEST_SIZE   16 
typedef struct {
    UINT32 HashValue[4];
    UINT64 MessageLen;
    UINT8  Block[MD5_BLOCK_SIZE];
    UINT   BlockLen;
} MD5_CTX_STRUC, *PMD5_CTX_STRUC;

VOID MD5_Init (
    IN  MD5_CTX_STRUC *pMD5_CTX);
VOID MD5_Hash (
    IN  MD5_CTX_STRUC *pMD5_CTX);
VOID MD5_Append (
    IN  MD5_CTX_STRUC *pMD5_CTX,
    IN  const UINT8 Message[],
    IN  UINT MessageLen);
VOID MD5_End (
    IN  MD5_CTX_STRUC *pMD5_CTX,
    OUT UINT8 DigestMessage[]);
VOID RT_MD5 (
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 DigestMessage[]);
#endif 

#endif 
