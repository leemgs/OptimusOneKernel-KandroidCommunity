

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "ttype.h"




typedef struct tagSChannelTblElement {
    BYTE    byChannelNumber;
    UINT    uFrequency;
    BOOL    bValid;
}SChannelTblElement, *PSChannelTblElement;




BOOL    ChannelValid(UINT CountryCode, UINT ChannelNum);
VOID    CHvInitChannelTable (PVOID pDeviceHandler);
BYTE    CHbyGetChannelMapping(BYTE byChannelNumber);

BOOL
CHvChannelGetList (
    IN  UINT       uCountryCodeIdx,
    OUT PBYTE      pbyChannelTable
    );

#endif  
