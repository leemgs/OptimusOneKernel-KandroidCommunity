

#ifndef __WPA_H__
#define __WPA_H__

#include "ttype.h"
#include "80211hdr.h"



#define WPA_NONE            0
#define WPA_WEP40           1
#define WPA_TKIP            2
#define WPA_AESWRAP         3
#define WPA_AESCCMP         4
#define WPA_WEP104          5
#define WPA_AUTH_IEEE802_1X 1
#define WPA_AUTH_PSK        2

#define WPA_GROUPFLAG       0x02
#define WPA_REPLAYBITSSHIFT 2
#define WPA_REPLAYBITS      0x03










VOID
WPA_ClearRSN(
    IN PKnownBSS        pBSSList
    );

VOID
WPA_ParseRSN(
    IN PKnownBSS        pBSSList,
    IN PWLAN_IE_RSN_EXT pRSN
    );

BOOL
WPA_SearchRSN(
    BYTE                byCmd,
    BYTE                byEncrypt,
    IN PKnownBSS        pBSSList
    );

BOOL
WPAb_Is_RSN(
    IN PWLAN_IE_RSN_EXT pRSN
    );

#endif 
