

#ifndef __RC4_H__
#define __RC4_H__

#include "ttype.h"



typedef struct {
    UINT ux;
    UINT uy;
    BYTE abystate[256];
} RC4Ext, *PRC4Ext;

VOID rc4_init(PRC4Ext pRC4, PBYTE pbyKey, UINT cbKey_len);
UINT rc4_byte(PRC4Ext pRC4);
void rc4_encrypt(PRC4Ext pRC4, PBYTE pbyDest, PBYTE pbySrc, UINT cbData_len);

#endif 
