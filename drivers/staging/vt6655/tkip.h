

#ifndef __TKIP_H__
#define __TKIP_H__

#include "ttype.h"
#include "tether.h"


#define TKIP_KEY_LEN        16











VOID TKIPvMixKey(
    PBYTE   pbyTKey,
    PBYTE   pbyTA,
    WORD    wTSC15_0,
    DWORD   dwTSC47_16,
    PBYTE   pbyRC4Key
    );

#endif 



