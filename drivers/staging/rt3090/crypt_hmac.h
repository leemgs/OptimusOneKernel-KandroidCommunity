

#ifndef __CRYPT_HMAC_H__
#define __CRYPT_HMAC_H__

#ifdef CRYPT_TESTPLAN
#include "crypt_testplan.h"
#else
#include "rt_config.h"
#endif 

#ifdef SHA1_SUPPORT
#define HMAC_SHA1_SUPPORT
VOID HMAC_SHA1 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen);
#endif 

#ifdef SHA256_SUPPORT
#define HMAC_SHA256_SUPPORT
VOID HMAC_SHA256 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen);
#endif 

#ifdef MD5_SUPPORT
#define HMAC_MD5_SUPPORT
VOID HMAC_MD5 (
    IN  const UINT8 Key[],
    IN  UINT KeyLen,
    IN  const UINT8 Message[],
    IN  UINT MessageLen,
    OUT UINT8 MAC[],
    IN  UINT MACLen);
#endif 

#endif 
