

#include "tmacro.h"
#include "key.h"
#include "mac.h"






static int          msglevel                =MSG_LEVEL_INFO;












static VOID
s_vCheckKeyTableValid (PSKeyManagement pTable, DWORD_PTR dwIoBase)
{
    int i;

    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            (pTable->KeyTable[i].PairwiseKey.bKeyValid == FALSE) &&
            (pTable->KeyTable[i].GroupKey[0].bKeyValid == FALSE) &&
            (pTable->KeyTable[i].GroupKey[1].bKeyValid == FALSE) &&
            (pTable->KeyTable[i].GroupKey[2].bKeyValid == FALSE) &&
            (pTable->KeyTable[i].GroupKey[3].bKeyValid == FALSE)
            ) {

            pTable->KeyTable[i].bInUse = FALSE;
            pTable->KeyTable[i].wKeyCtl = 0;
            pTable->KeyTable[i].bSoftWEP = FALSE;
            MACvDisableKeyEntry(dwIoBase, i);
        }
    }
}






VOID KeyvInitTable (PSKeyManagement pTable, DWORD_PTR dwIoBase)
{
    int i;
    int jj;

    for (i=0;i<MAX_KEY_TABLE;i++) {
        pTable->KeyTable[i].bInUse = FALSE;
        pTable->KeyTable[i].PairwiseKey.bKeyValid = FALSE;
        pTable->KeyTable[i].PairwiseKey.pvKeyTable = (PVOID)&pTable->KeyTable[i];
        for (jj=0; jj < MAX_GROUP_KEY; jj++) {
            pTable->KeyTable[i].GroupKey[jj].bKeyValid = FALSE;
            pTable->KeyTable[i].GroupKey[jj].pvKeyTable = (PVOID)&pTable->KeyTable[i];
        }
        pTable->KeyTable[i].wKeyCtl = 0;
        pTable->KeyTable[i].dwGTKeyIndex = 0;
        pTable->KeyTable[i].bSoftWEP = FALSE;
        MACvDisableKeyEntry(dwIoBase, i);
    }
}



BOOL KeybGetKey (
    IN  PSKeyManagement pTable,
    IN  PBYTE           pbyBSSID,
    IN  DWORD           dwKeyIndex,
    OUT PSKeyItem       *pKey
    )
{
    int i;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybGetKey() \n");

    *pKey = NULL;
    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            IS_ETH_ADDRESS_EQUAL(pTable->KeyTable[i].abyBSSID,pbyBSSID)) {
            if (dwKeyIndex == 0xFFFFFFFF) {
                if (pTable->KeyTable[i].PairwiseKey.bKeyValid == TRUE) {
                    *pKey = &(pTable->KeyTable[i].PairwiseKey);
                    return (TRUE);
                }
                else {
                    return (FALSE);
                }
            } else if (dwKeyIndex < MAX_GROUP_KEY) {
                if (pTable->KeyTable[i].GroupKey[dwKeyIndex].bKeyValid == TRUE) {
                    *pKey = &(pTable->KeyTable[i].GroupKey[dwKeyIndex]);
                    return (TRUE);
                }
                else {
                    return (FALSE);
                }
            }
            else {
                return (FALSE);
            }
        }
    }
    return (FALSE);
}



BOOL KeybSetKey (
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    )
{
    int         i,j;
    UINT        ii;
    PSKeyItem   pKey;
    UINT        uKeyIdx;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Enter KeybSetKey: %lX\n", dwKeyIndex);

    j = (MAX_KEY_TABLE-1);
    for (i=0;i<(MAX_KEY_TABLE-1);i++) {
        if ((pTable->KeyTable[i].bInUse == FALSE) &&
            (j == (MAX_KEY_TABLE-1))) {
            
            j = i;
        }
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            IS_ETH_ADDRESS_EQUAL(pTable->KeyTable[i].abyBSSID,pbyBSSID)) {
            
            if ((dwKeyIndex & PAIRWISE_KEY) != 0) {
                
                pKey = &(pTable->KeyTable[i].PairwiseKey);
                pTable->KeyTable[i].wKeyCtl &= 0xFFF0;          
                pTable->KeyTable[i].wKeyCtl |= byKeyDecMode;
                uKeyIdx = 4;                                    
            } else {
                
                if ((dwKeyIndex & 0x000000FF) >= MAX_GROUP_KEY)
                    return (FALSE);
                pKey = &(pTable->KeyTable[i].GroupKey[dwKeyIndex & 0x000000FF]);
                if ((dwKeyIndex & TRANSMIT_KEY) != 0)  {
                    
                    pTable->KeyTable[i].dwGTKeyIndex = dwKeyIndex;
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Group transmit key(R)[%lX]: %d\n", pTable->KeyTable[i].dwGTKeyIndex, i);
                }
                pTable->KeyTable[i].wKeyCtl &= 0xFF0F;          
                pTable->KeyTable[i].wKeyCtl |= (byKeyDecMode << 4);
                pTable->KeyTable[i].wKeyCtl |= 0x0040;          
                uKeyIdx = (dwKeyIndex & 0x000000FF);
            }
            pTable->KeyTable[i].wKeyCtl |= 0x8000;              

            pKey->bKeyValid = TRUE;
            pKey->uKeyLength = uKeyLength;
            pKey->dwKeyIndex = dwKeyIndex;
            pKey->byCipherSuite = byKeyDecMode;
            memcpy(pKey->abyKey, pbyKey, uKeyLength);
            if (byKeyDecMode == KEY_CTL_WEP) {
                if (uKeyLength == WLAN_WEP40_KEYLEN)
                    pKey->abyKey[15] &= 0x7F;
                if (uKeyLength == WLAN_WEP104_KEYLEN)
                    pKey->abyKey[15] |= 0x80;
            }
            MACvSetKeyEntry(dwIoBase, pTable->KeyTable[i].wKeyCtl, i, uKeyIdx, pbyBSSID, (PDWORD)pKey->abyKey, byLocalID);

            if ((dwKeyIndex & USE_KEYRSC) == 0) {
                
		    memset(&(pKey->KeyRSC), 0, sizeof(QWORD));
            }
            else {
                memcpy(&(pKey->KeyRSC), pKeyRSC,  sizeof(QWORD));
            }
            pKey->dwTSC47_16 = 0;
            pKey->wTSC15_0 = 0;

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybSetKey(R): \n");
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->bKeyValid: %d\n ", pKey->bKeyValid);
            
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->abyKey: ");
            for (ii = 0; ii < pKey->uKeyLength; ii++) {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "%02x ", pKey->abyKey[ii]);
            }
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwTSC47_16: %lx\n ", pKey->dwTSC47_16);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->wTSC15_0: %x\n ", pKey->wTSC15_0);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwKeyIndex: %lx\n ", pKey->dwKeyIndex);

            return (TRUE);
        }
    }
    if (j < (MAX_KEY_TABLE-1)) {
        memcpy(pTable->KeyTable[j].abyBSSID,pbyBSSID,U_ETHER_ADDR_LEN);
        pTable->KeyTable[j].bInUse = TRUE;
        if ((dwKeyIndex & PAIRWISE_KEY) != 0)  {
            
            pKey = &(pTable->KeyTable[j].PairwiseKey);
            pTable->KeyTable[j].wKeyCtl &= 0xFFF0;          
            pTable->KeyTable[j].wKeyCtl |= byKeyDecMode;
            uKeyIdx = 4;                                    
        } else {
            
            if ((dwKeyIndex & 0x000000FF) >= MAX_GROUP_KEY)
                return (FALSE);
            pKey = &(pTable->KeyTable[j].GroupKey[dwKeyIndex & 0x000000FF]);
            if ((dwKeyIndex & TRANSMIT_KEY) != 0)  {
                
                pTable->KeyTable[j].dwGTKeyIndex = dwKeyIndex;
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Group transmit key(N)[%lX]: %d\n", pTable->KeyTable[j].dwGTKeyIndex, j);
            }
            pTable->KeyTable[j].wKeyCtl &= 0xFF0F;          
            pTable->KeyTable[j].wKeyCtl |= (byKeyDecMode << 4);
            pTable->KeyTable[j].wKeyCtl |= 0x0040;          
            uKeyIdx = (dwKeyIndex & 0x000000FF);
        }
        pTable->KeyTable[j].wKeyCtl |= 0x8000;              

        pKey->bKeyValid = TRUE;
        pKey->uKeyLength = uKeyLength;
        pKey->dwKeyIndex = dwKeyIndex;
        pKey->byCipherSuite = byKeyDecMode;
        memcpy(pKey->abyKey, pbyKey, uKeyLength);
        if (byKeyDecMode == KEY_CTL_WEP) {
            if (uKeyLength == WLAN_WEP40_KEYLEN)
                pKey->abyKey[15] &= 0x7F;
            if (uKeyLength == WLAN_WEP104_KEYLEN)
                pKey->abyKey[15] |= 0x80;
        }
        MACvSetKeyEntry(dwIoBase, pTable->KeyTable[j].wKeyCtl, j, uKeyIdx, pbyBSSID, (PDWORD)pKey->abyKey, byLocalID);

        if ((dwKeyIndex & USE_KEYRSC) == 0) {
            
		memset(&(pKey->KeyRSC), 0, sizeof(QWORD));
        }
        else {
            memcpy(&(pKey->KeyRSC), pKeyRSC,  sizeof(QWORD));
        }
        pKey->dwTSC47_16 = 0;
        pKey->wTSC15_0 = 0;

        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybSetKey(N): \n");
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->bKeyValid: %d\n ", pKey->bKeyValid);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->uKeyLength: %d\n ", (int)pKey->uKeyLength);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->abyKey: ");
        for (ii = 0; ii < pKey->uKeyLength; ii++) {
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "%02x ", pKey->abyKey[ii]);
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");

        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwTSC47_16: %lx\n ", pKey->dwTSC47_16);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->wTSC15_0: %x\n ", pKey->wTSC15_0);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwKeyIndex: %lx\n ", pKey->dwKeyIndex);

        return (TRUE);
    }
    return (FALSE);
}



BOOL KeybRemoveKey (
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD           dwKeyIndex,
    DWORD_PTR       dwIoBase
    )
{
    int  i;

    if (IS_BROADCAST_ADDRESS(pbyBSSID)) {
        
        if ((dwKeyIndex & PAIRWISE_KEY) != 0) {
            for (i=0;i<MAX_KEY_TABLE;i++) {
                pTable->KeyTable[i].PairwiseKey.bKeyValid = FALSE;
            }
            s_vCheckKeyTableValid(pTable, dwIoBase);
            return TRUE;
        }
        else if ((dwKeyIndex & 0x000000FF) < MAX_GROUP_KEY) {
            for (i=0;i<MAX_KEY_TABLE;i++) {
                pTable->KeyTable[i].GroupKey[dwKeyIndex & 0x000000FF].bKeyValid = FALSE;
                if ((dwKeyIndex & 0x7FFFFFFF) == (pTable->KeyTable[i].dwGTKeyIndex & 0x7FFFFFFF)) {
                    
                    pTable->KeyTable[i].dwGTKeyIndex = 0;
                }
            }
            s_vCheckKeyTableValid(pTable, dwIoBase);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            IS_ETH_ADDRESS_EQUAL(pTable->KeyTable[i].abyBSSID,pbyBSSID)) {
            if ((dwKeyIndex & PAIRWISE_KEY) != 0) {
                pTable->KeyTable[i].PairwiseKey.bKeyValid = FALSE;
                s_vCheckKeyTableValid(pTable, dwIoBase);
                return (TRUE);
            }
            else if ((dwKeyIndex & 0x000000FF) < MAX_GROUP_KEY) {
                pTable->KeyTable[i].GroupKey[dwKeyIndex & 0x000000FF].bKeyValid = FALSE;
                if ((dwKeyIndex & 0x7FFFFFFF) == (pTable->KeyTable[i].dwGTKeyIndex & 0x7FFFFFFF)) {
                    
                    pTable->KeyTable[i].dwGTKeyIndex = 0;
                }
                s_vCheckKeyTableValid(pTable, dwIoBase);
                return (TRUE);
            }
            else {
                return (FALSE);
            }
        }
    }
    return (FALSE);
}



BOOL KeybRemoveAllKey (
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD_PTR       dwIoBase
    )
{
    int  i,u;

    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            IS_ETH_ADDRESS_EQUAL(pTable->KeyTable[i].abyBSSID,pbyBSSID)) {
            pTable->KeyTable[i].PairwiseKey.bKeyValid = FALSE;
            for(u=0;u<MAX_GROUP_KEY;u++) {
                pTable->KeyTable[i].GroupKey[u].bKeyValid = FALSE;
            }
            pTable->KeyTable[i].dwGTKeyIndex = 0;
            s_vCheckKeyTableValid(pTable, dwIoBase);
            return (TRUE);
        }
    }
    return (FALSE);
}


VOID KeyvRemoveWEPKey (
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    DWORD_PTR       dwIoBase
    )
{

   if ((dwKeyIndex & 0x000000FF) < MAX_GROUP_KEY) {
        if (pTable->KeyTable[MAX_KEY_TABLE-1].bInUse == TRUE) {
            if (pTable->KeyTable[MAX_KEY_TABLE-1].GroupKey[dwKeyIndex & 0x000000FF].byCipherSuite == KEY_CTL_WEP) {
                pTable->KeyTable[MAX_KEY_TABLE-1].GroupKey[dwKeyIndex & 0x000000FF].bKeyValid = FALSE;
                if ((dwKeyIndex & 0x7FFFFFFF) == (pTable->KeyTable[MAX_KEY_TABLE-1].dwGTKeyIndex & 0x7FFFFFFF)) {
                    
                    pTable->KeyTable[MAX_KEY_TABLE-1].dwGTKeyIndex = 0;
                }
            }
        }
        s_vCheckKeyTableValid(pTable, dwIoBase);
    }
    return;
}

VOID KeyvRemoveAllWEPKey (
    PSKeyManagement pTable,
    DWORD_PTR       dwIoBase
    )
{
    int i;

    for(i=0;i<MAX_GROUP_KEY;i++) {
        KeyvRemoveWEPKey(pTable, i, dwIoBase);
    }
}


BOOL KeybGetTransmitKey (
    IN  PSKeyManagement pTable,
    IN  PBYTE           pbyBSSID,
    IN  DWORD           dwKeyType,
    OUT PSKeyItem       *pKey
    )
{
    int i, ii;

    *pKey = NULL;
    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            IS_ETH_ADDRESS_EQUAL(pTable->KeyTable[i].abyBSSID,pbyBSSID)) {

            if (dwKeyType == PAIRWISE_KEY) {

                if (pTable->KeyTable[i].PairwiseKey.bKeyValid == TRUE) {
                    *pKey = &(pTable->KeyTable[i].PairwiseKey);

                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybGetTransmitKey:");
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"PAIRWISE_KEY: KeyTable.abyBSSID: ");
                    for (ii = 0; ii < 6; ii++) {
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"%x ", pTable->KeyTable[i].abyBSSID[ii]);
                    }
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");


                    return (TRUE);
                }
                else {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"PairwiseKey.bKeyValid == FALSE\n");
                    return (FALSE);
                }
            } 
            else {
                if (pTable->KeyTable[i].dwGTKeyIndex == 0) {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ERROR: dwGTKeyIndex == 0 !!!\n");
                    return FALSE;
                }
                if (pTable->KeyTable[i].GroupKey[(pTable->KeyTable[i].dwGTKeyIndex&0x000000FF)].bKeyValid == TRUE) {
                    *pKey = &(pTable->KeyTable[i].GroupKey[(pTable->KeyTable[i].dwGTKeyIndex&0x000000FF)]);

                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybGetTransmitKey:");
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"GROUP_KEY: KeyTable.abyBSSID\n");
                        for (ii = 0; ii < 6; ii++) {
                            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"%x ", pTable->KeyTable[i].abyBSSID[ii]);
                        }
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"dwGTKeyIndex: %lX\n", pTable->KeyTable[i].dwGTKeyIndex);

                    return (TRUE);
                }
                else {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"GroupKey.bKeyValid == FALSE\n");
                    return (FALSE);
                }
            } 
        } 
    }
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ERROR: NO Match BSSID !!! ");
    for (ii = 0; ii < 6; ii++) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"%02x ", *(pbyBSSID+ii));
    }
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");
    return (FALSE);
}



BOOL KeybCheckPairewiseKey (
    IN  PSKeyManagement pTable,
    OUT PSKeyItem       *pKey
    )
{
    int i;

    *pKey = NULL;
    for (i=0;i<MAX_KEY_TABLE;i++) {
        if ((pTable->KeyTable[i].bInUse == TRUE) &&
            (pTable->KeyTable[i].PairwiseKey.bKeyValid == TRUE)) {
            *pKey = &(pTable->KeyTable[i].PairwiseKey);
            return (TRUE);
        }
    }
    return (FALSE);
}


BOOL KeybSetDefaultKey (
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    )
{
    UINT        ii;
    PSKeyItem   pKey;
    UINT        uKeyIdx;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Enter KeybSetDefaultKey: %1x, %d \n", (int)dwKeyIndex, (int)uKeyLength);


    if ((dwKeyIndex & PAIRWISE_KEY) != 0) {                  
        return (FALSE);
    } else if ((dwKeyIndex & 0x000000FF) >= MAX_GROUP_KEY) {
        return (FALSE);
    }

    pTable->KeyTable[MAX_KEY_TABLE-1].bInUse = TRUE;
    for(ii=0;ii<U_ETHER_ADDR_LEN;ii++)
        pTable->KeyTable[MAX_KEY_TABLE-1].abyBSSID[ii] = 0xFF;

    
    pKey = &(pTable->KeyTable[MAX_KEY_TABLE-1].GroupKey[dwKeyIndex & 0x000000FF]);
    if ((dwKeyIndex & TRANSMIT_KEY) != 0)  {
        
        pTable->KeyTable[MAX_KEY_TABLE-1].dwGTKeyIndex = dwKeyIndex;
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Group transmit key(R)[%lX]: %d\n", pTable->KeyTable[MAX_KEY_TABLE-1].dwGTKeyIndex, MAX_KEY_TABLE-1);

    }
    pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl &= 0x7F00;          
    pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl |= (byKeyDecMode << 4);
    pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl |= (byKeyDecMode);
    pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl |= 0x0044;          
    uKeyIdx = (dwKeyIndex & 0x000000FF);

    if ((uKeyLength == WLAN_WEP232_KEYLEN) &&
        (byKeyDecMode == KEY_CTL_WEP)) {
        pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl |= 0x4000;              
        pTable->KeyTable[MAX_KEY_TABLE-1].bSoftWEP = TRUE;
    } else {
        if (pTable->KeyTable[MAX_KEY_TABLE-1].bSoftWEP == FALSE)
            pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl |= 0xC000;          
    }

    pKey->bKeyValid = TRUE;
    pKey->uKeyLength = uKeyLength;
    pKey->dwKeyIndex = dwKeyIndex;
    pKey->byCipherSuite = byKeyDecMode;
    memcpy(pKey->abyKey, pbyKey, uKeyLength);
    if (byKeyDecMode == KEY_CTL_WEP) {
        if (uKeyLength == WLAN_WEP40_KEYLEN)
            pKey->abyKey[15] &= 0x7F;
        if (uKeyLength == WLAN_WEP104_KEYLEN)
            pKey->abyKey[15] |= 0x80;
    }
    MACvSetKeyEntry(dwIoBase, pTable->KeyTable[MAX_KEY_TABLE-1].wKeyCtl, MAX_KEY_TABLE-1, uKeyIdx, pTable->KeyTable[MAX_KEY_TABLE-1].abyBSSID, (PDWORD)pKey->abyKey, byLocalID);

    if ((dwKeyIndex & USE_KEYRSC) == 0) {
        
	    memset(&(pKey->KeyRSC), 0, sizeof(QWORD));
    } else {
        memcpy(&(pKey->KeyRSC), pKeyRSC,  sizeof(QWORD));
    }
    pKey->dwTSC47_16 = 0;
    pKey->wTSC15_0 = 0;


    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybSetKey(R): \n");
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->bKeyValid: %d\n", pKey->bKeyValid);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->uKeyLength: %d\n", (int)pKey->uKeyLength);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->abyKey: \n");
    for (ii = 0; ii < pKey->uKeyLength; ii++) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"%x", pKey->abyKey[ii]);
    }
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwTSC47_16: %lx\n", pKey->dwTSC47_16);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->wTSC15_0: %x\n", pKey->wTSC15_0);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->dwKeyIndex: %lx\n", pKey->dwKeyIndex);

    return (TRUE);
}



BOOL KeybSetAllGroupKey (
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    )
{
    int         i;
    UINT        ii;
    PSKeyItem   pKey;
    UINT        uKeyIdx;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Enter KeybSetAllGroupKey: %lX\n", dwKeyIndex);


    if ((dwKeyIndex & PAIRWISE_KEY) != 0) {                  
        return (FALSE);
    } else if ((dwKeyIndex & 0x000000FF) >= MAX_GROUP_KEY) {
        return (FALSE);
    }

    for (i=0; i < MAX_KEY_TABLE-1; i++) {
        if (pTable->KeyTable[i].bInUse == TRUE) {
            
            
            pKey = &(pTable->KeyTable[i].GroupKey[dwKeyIndex & 0x000000FF]);
            if ((dwKeyIndex & TRANSMIT_KEY) != 0)  {
                
                pTable->KeyTable[i].dwGTKeyIndex = dwKeyIndex;
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Group transmit key(R)[%lX]: %d\n", pTable->KeyTable[i].dwGTKeyIndex, i);

            }
            pTable->KeyTable[i].wKeyCtl &= 0xFF0F;          
            pTable->KeyTable[i].wKeyCtl |= (byKeyDecMode << 4);
            pTable->KeyTable[i].wKeyCtl |= 0x0040;          
            uKeyIdx = (dwKeyIndex & 0x000000FF);

            pTable->KeyTable[i].wKeyCtl |= 0x8000;              

            pKey->bKeyValid = TRUE;
            pKey->uKeyLength = uKeyLength;
            pKey->dwKeyIndex = dwKeyIndex;
            pKey->byCipherSuite = byKeyDecMode;
            memcpy(pKey->abyKey, pbyKey, uKeyLength);
            if (byKeyDecMode == KEY_CTL_WEP) {
                if (uKeyLength == WLAN_WEP40_KEYLEN)
                    pKey->abyKey[15] &= 0x7F;
                if (uKeyLength == WLAN_WEP104_KEYLEN)
                    pKey->abyKey[15] |= 0x80;
            }
            MACvSetKeyEntry(dwIoBase, pTable->KeyTable[i].wKeyCtl, i, uKeyIdx, pTable->KeyTable[i].abyBSSID, (PDWORD)pKey->abyKey, byLocalID);

            if ((dwKeyIndex & USE_KEYRSC) == 0) {
                
		    memset(&(pKey->KeyRSC), 0, sizeof(QWORD));
            }
            else {
                memcpy(&(pKey->KeyRSC), pKeyRSC,  sizeof(QWORD));
            }
            pKey->dwTSC47_16 = 0;
            pKey->wTSC15_0 = 0;

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KeybSetKey(R): \n");
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->bKeyValid: %d\n ", pKey->bKeyValid);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->uKeyLength: %d\n ", (int)pKey->uKeyLength);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey->abyKey: ");
            for (ii = 0; ii < pKey->uKeyLength; ii++) {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"%02x ", pKey->abyKey[ii]);
            }
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\n");

            
            
            

        } 
    }
    return (TRUE);
}
