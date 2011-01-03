

#include "device.h"
#include "tmacro.h"
#include "tcrc.h"
#include "tether.h"














BYTE ETHbyGetHashIndexByCrc32 (PBYTE pbyMultiAddr)
{
    int     ii;
    BYTE    byTmpHash;
    BYTE    byHash = 0;

    
    byTmpHash = (BYTE)(CRCdwCrc32(pbyMultiAddr, U_ETHER_ADDR_LEN,
            0xFFFFFFFFL) & 0x3F);
    
    for (ii = 0; ii < (sizeof(byTmpHash) * 8); ii++) {
        byHash <<= 1;
        if (byTmpHash & 0x01)
            byHash |= 1;
        byTmpHash >>= 1;
    }

    
    return (byHash >> 2);
}



BOOL ETHbIsBufferCrc32Ok (PBYTE pbyBuffer, UINT cbFrameLength)
{
    DWORD dwCRC;

    dwCRC = CRCdwGetCrc32(pbyBuffer, cbFrameLength - 4);
    if (cpu_to_le32(*((PDWORD)(pbyBuffer + cbFrameLength - 4))) != dwCRC) {
        return FALSE;
    }
    return TRUE;
}

