

#ifndef __KEY_H__
#define __KEY_H__

#include "ttype.h"
#include "tether.h"
#include "80211mgr.h"


#define MAX_GROUP_KEY       4
#define MAX_KEY_TABLE       11
#define MAX_KEY_LEN         32
#define AES_KEY_LEN         16


#define AUTHENTICATOR_KEY   0x10000000
#define USE_KEYRSC          0x20000000
#define PAIRWISE_KEY        0x40000000
#define TRANSMIT_KEY        0x80000000

#define GROUP_KEY           0x00000000

#define KEY_CTL_WEP         0x00
#define KEY_CTL_NONE        0x01
#define KEY_CTL_TKIP        0x02
#define KEY_CTL_CCMP        0x03
#define KEY_CTL_INVALID     0xFF


typedef struct tagSKeyItem
{
    BOOL        bKeyValid;
    ULONG       uKeyLength;
    BYTE        abyKey[MAX_KEY_LEN];
    QWORD       KeyRSC;
    DWORD       dwTSC47_16;
    WORD        wTSC15_0;
    BYTE        byCipherSuite;
    BYTE        byReserved0;
    DWORD       dwKeyIndex;
    PVOID       pvKeyTable;
} SKeyItem, *PSKeyItem; 

typedef struct tagSKeyTable
{
    BYTE        abyBSSID[U_ETHER_ADDR_LEN];  
    BYTE        byReserved0[2];              
    SKeyItem    PairwiseKey;
    SKeyItem    GroupKey[MAX_GROUP_KEY]; 
    DWORD       dwGTKeyIndex;            
    BOOL        bInUse;
    
    
    
    BOOL        bSoftWEP;
    WORD        wKeyCtl;      

    BYTE        byReserved1[6];
} SKeyTable, *PSKeyTable; 

typedef struct tagSKeyManagement
{
    SKeyTable   KeyTable[MAX_KEY_TABLE];
} SKeyManagement, * PSKeyManagement;











VOID KeyvInitTable(PSKeyManagement pTable, DWORD_PTR dwIoBase);

BOOL KeybGetKey(
    IN  PSKeyManagement pTable,
    IN  PBYTE           pbyBSSID,
    IN  DWORD           dwKeyIndex,
    OUT PSKeyItem       *pKey
    );

BOOL KeybSetKey(
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    );

BOOL KeybSetDefaultKey(
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    );

BOOL KeybRemoveKey(
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD           dwKeyIndex,
    DWORD_PTR       dwIoBase
    );

BOOL KeybGetTransmitKey(
    IN  PSKeyManagement pTable,
    IN  PBYTE           pbyBSSID,
    IN  DWORD           dwKeyType,
    OUT PSKeyItem       *pKey
    );

BOOL KeybCheckPairewiseKey(
    IN  PSKeyManagement pTable,
    OUT PSKeyItem       *pKey
    );

BOOL KeybRemoveAllKey(
    PSKeyManagement pTable,
    PBYTE           pbyBSSID,
    DWORD_PTR       dwIoBase
    );

VOID KeyvRemoveWEPKey(
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    DWORD_PTR       dwIoBase
    );

VOID KeyvRemoveAllWEPKey(
    PSKeyManagement pTable,
    DWORD_PTR       dwIoBase
    );

BOOL KeybSetAllGroupKey (
    PSKeyManagement pTable,
    DWORD           dwKeyIndex,
    ULONG           uKeyLength,
    PQWORD          pKeyRSC,
    PBYTE           pbyKey,
    BYTE            byKeyDecMode,
    DWORD_PTR       dwIoBase,
    BYTE            byLocalID
    );

#endif 

