


#ifndef __RNDIS_H__
#define __RNDIS_H__


#define MESSAGE_TYPE_READ               0x01
#define MESSAGE_TYPE_WRITE              0x00
#define MESSAGE_TYPE_LOCK_OR            0x02
#define MESSAGE_TYPE_LOCK_AND           0x03
#define MESSAGE_TYPE_WRITE_MASK         0x04
#define MESSAGE_TYPE_CARDINIT           0x05
#define MESSAGE_TYPE_INIT_RSP           0x06
#define MESSAGE_TYPE_MACSHUTDOWN        0x07
#define MESSAGE_TYPE_SETKEY             0x08
#define MESSAGE_TYPE_CLRKEYENTRY        0x09
#define MESSAGE_TYPE_WRITE_MISCFF       0x0A
#define MESSAGE_TYPE_SET_ANTMD          0x0B
#define MESSAGE_TYPE_SELECT_CHANNLE     0x0C
#define MESSAGE_TYPE_SET_TSFTBTT        0x0D
#define MESSAGE_TYPE_SET_SSTIFS         0x0E
#define MESSAGE_TYPE_CHANGE_BBTYPE      0x0F
#define MESSAGE_TYPE_DISABLE_PS         0x10
#define MESSAGE_TYPE_WRITE_IFRF         0x11


#define MESSAGE_REQUEST_MEM             0x01
#define MESSAGE_REQUEST_BBREG           0x02
#define MESSAGE_REQUEST_MACREG          0x03
#define MESSAGE_REQUEST_EEPROM          0x04
#define MESSAGE_REQUEST_TSF             0x05
#define MESSAGE_REQUEST_TBTT            0x06
#define MESSAGE_REQUEST_BBAGC           0x07
#define MESSAGE_REQUEST_VERSION         0x08
#define MESSAGE_REQUEST_RF_INIT         0x09
#define MESSAGE_REQUEST_RF_INIT2        0x0A
#define MESSAGE_REQUEST_RF_CH0          0x0B
#define MESSAGE_REQUEST_RF_CH1          0x0C
#define MESSAGE_REQUEST_RF_CH2          0x0D


#define VIAUSB20_PACKET_HEADER          0x04




typedef struct _CMD_MESSAGE
{
    BYTE        byData[256];
} CMD_MESSAGE, *PCMD_MESSAGE;

typedef struct _CMD_WRITE_MASK
{
    BYTE        byData;
    BYTE        byMask;
} CMD_WRITE_MASK, *PCMD_WRITE_MASK;

typedef struct _CMD_CARD_INIT
{
    BYTE        byInitClass;
    BYTE        bExistSWNetAddr;
    BYTE        bySWNetAddr[6];
    BYTE        byShortRetryLimit;
    BYTE        byLongRetryLimit;
} CMD_CARD_INIT, *PCMD_CARD_INIT;

typedef struct _RSP_CARD_INIT
{
    BYTE        byStatus;
    BYTE        byNetAddr[6];
    BYTE        byRFType;
    BYTE        byMinChannel;
    BYTE        byMaxChannel;
} RSP_CARD_INIT, *PRSP_CARD_INIT;

typedef struct _CMD_SET_KEY
{
    WORD        wKCTL;
    BYTE        abyMacAddr[6];
    BYTE        abyKey[16];
} CMD_SET_KEY, *PCMD_SET_KEY;

typedef struct _CMD_CLRKEY_ENTRY
{
    BYTE        abyKeyEntry[11];
} CMD_CLRKEY_ENTRY, *PCMD_CLRKEY_ENTRY;

typedef struct _CMD_WRITE_MISCFF
{
    DWORD       adwMiscFFData[22][4];  
} CMD_WRITE_MISCFF, *PCMD_WRITE_MISCFF;

typedef struct _CMD_SET_TSFTBTT
{
    BYTE        abyTSF_TBTT[8];
} CMD_SET_TSFTBTT, *PCMD_SET_TSFTBTT;

typedef struct _CMD_SET_SSTIFS
{
    BYTE        bySIFS;
    BYTE        byDIFS;
    BYTE        byEIFS;
    BYTE        bySlotTime;
    BYTE        byCwMax_Min;
    BYTE        byBBCR10;
} CMD_SET_SSTIFS, *PCMD_SET_SSTIFS;

typedef struct _CMD_CHANGE_BBTYPE
{
    BYTE        bySIFS;
    BYTE        byDIFS;
    BYTE        byEIFS;
    BYTE        bySlotTime;
    BYTE        byCwMax_Min;
    BYTE        byBBCR10;
    BYTE        byBB_BBType;    
    BYTE        byMAC_BBType;
    DWORD       dwRSPINF_b_1;
    DWORD       dwRSPINF_b_2;
    DWORD       dwRSPINF_b_55;
    DWORD       dwRSPINF_b_11;
    WORD        wRSPINF_a[9];
} CMD_CHANGE_BBTYPE, *PCMD_CHANGE_BBTYPE;



#define EXCH_WORD(w)        ( (WORD)((WORD)(w)<<8) | (WORD)((WORD)(w)>>8) )






#endif 
