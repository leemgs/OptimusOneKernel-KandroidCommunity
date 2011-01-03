

#include "tmacro.h"
#include "michael.h"







static VOID s_vClear(void);                       
                                              
static VOID s_vSetKey(DWORD dwK0, DWORD dwK1);
static VOID s_vAppendByte(BYTE b);            


static DWORD  L, R;           

static DWORD  K0, K1;         
static DWORD  M;              
static UINT   nBytesInM;      





static VOID s_vClear (void)
{
    
    L = K0;
    R = K1;
    nBytesInM = 0;
    M = 0;
}

static VOID s_vSetKey (DWORD dwK0, DWORD dwK1)
{
    
    K0 = dwK0;
    K1 = dwK1;
    
    s_vClear();
}

static VOID s_vAppendByte (BYTE b)
{
    
    M |= b << (8*nBytesInM);
    nBytesInM++;
    
    if( nBytesInM >= 4 )
    {
        L ^= M;
        R ^= ROL32( L, 17 );
        L += R;
        R ^= ((L & 0xff00ff00) >> 8) | ((L & 0x00ff00ff) << 8);
        L += R;
        R ^= ROL32( L, 3 );
        L += R;
        R ^= ROR32( L, 2 );
        L += R;
        
        M = 0;
        nBytesInM = 0;
    }
}

VOID MIC_vInit (DWORD dwK0, DWORD dwK1)
{
    
    s_vSetKey(dwK0, dwK1);
}


VOID MIC_vUnInit (void)
{
    
    K0 = 0;
    K1 = 0;

    
    
    s_vClear();
}

VOID MIC_vAppend (PBYTE src, UINT nBytes)
{
    
    while (nBytes > 0)
    {
        s_vAppendByte(*src++);
        nBytes--;
    }
}

VOID MIC_vGetMIC (PDWORD pdwL, PDWORD pdwR)
{
    
    s_vAppendByte(0x5a);
    s_vAppendByte(0);
    s_vAppendByte(0);
    s_vAppendByte(0);
    s_vAppendByte(0);
    
    while( nBytesInM != 0 )
    {
        s_vAppendByte(0);
    }
    
    *pdwL = L;
    *pdwR = R;
    
    s_vClear();
}

