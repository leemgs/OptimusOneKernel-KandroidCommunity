

#ifndef __SROM_H__
#define __SROM_H__

#include "ttype.h"



#define EEP_MAX_CONTEXT_SIZE    256

#define CB_EEPROM_READBYTE_WAIT 900     

#define W_MAX_I2CRETRY          0x0fff




#define EEP_OFS_PAR         0x00        
#define EEP_OFS_ANTENNA     0x16
#define EEP_OFS_RADIOCTL    0x17
#define EEP_OFS_RFTYPE      0x1B        
#define EEP_OFS_MINCHANNEL  0x1C        
#define EEP_OFS_MAXCHANNEL  0x1D        
#define EEP_OFS_SIGNATURE   0x1E        
#define EEP_OFS_ZONETYPE    0x1F        
#define EEP_OFS_RFTABLE     0x20        
#define EEP_OFS_PWR_CCK     0x20
#define EEP_OFS_SETPT_CCK   0x21
#define EEP_OFS_PWR_OFDMG   0x23
#define EEP_OFS_SETPT_OFDMG 0x24
#define EEP_OFS_PWR_FORMULA_OST  0x26   
#define EEP_OFS_MAJOR_VER 0x2E
#define EEP_OFS_MINOR_VER 0x2F
#define EEP_OFS_CCK_PWR_TBL     0x30
#define EEP_OFS_CCK_PWR_dBm     0x3F
#define EEP_OFS_OFDM_PWR_TBL    0x40
#define EEP_OFS_OFDM_PWR_dBm    0x4F

#define EEP_OFS_SETPT_OFDMA         0x4E
#define EEP_OFS_OFDMA_PWR_TBL       0x50

#define EEP_OFS_OFDMA_PWR_dBm       0xD2



#define EEP_OFS_BBTAB_LEN   0x70        
#define EEP_OFS_BBTAB_ADR   0x71        
#define EEP_OFS_CHECKSUM    0xFF        

#define EEP_I2C_DEV_ID      0x50        





#define EEP_ANTENNA_MAIN    0x01
#define EEP_ANTENNA_AUX     0x02
#define EEP_ANTINV          0x04




#define EEP_RADIOCTL_ENABLE 0x80
#define EEP_RADIOCTL_INV    0x01






typedef struct tagSSromReg {
    BYTE    abyPAR[6];                  

    WORD    wSUB_VID;                   
    WORD    wSUB_SID;

    BYTE    byBCFG0;                    
    BYTE    byBCFG1;

    BYTE    byFCR0;                     
    BYTE    byFCR1;
    BYTE    byPMC0;                     
    BYTE    byPMC1;
    BYTE    byMAXLAT;                   
    BYTE    byMINGNT;
    BYTE    byCFG0;                     
    BYTE    byCFG1;
    WORD    wCISPTR;                    
    WORD    wRsv0;                      
    WORD    wRsv1;                      
    BYTE    byBBPAIR;                   
    BYTE    byRFTYPE;
    BYTE    byMinChannel;               
    BYTE    byMaxChannel;
    BYTE    bySignature;                
    BYTE    byCheckSum;

    BYTE    abyReserved0[96];           
    BYTE    abyCIS[128];                
} SSromReg, *PSSromReg;









BYTE SROMbyReadEmbedded(DWORD_PTR dwIoBase, BYTE byContntOffset);
BOOL SROMbWriteEmbedded(DWORD_PTR dwIoBase, BYTE byContntOffset, BYTE byData);

void SROMvRegBitsOn(DWORD_PTR dwIoBase, BYTE byContntOffset, BYTE byBits);
void SROMvRegBitsOff(DWORD_PTR dwIoBase, BYTE byContntOffset, BYTE byBits);

BOOL SROMbIsRegBitsOn(DWORD_PTR dwIoBase, BYTE byContntOffset, BYTE byTestBits);
BOOL SROMbIsRegBitsOff(DWORD_PTR dwIoBase, BYTE byContntOffset, BYTE byTestBits);

void SROMvReadAllContents(DWORD_PTR dwIoBase, PBYTE pbyEepromRegs);
void SROMvWriteAllContents(DWORD_PTR dwIoBase, PBYTE pbyEepromRegs);

void SROMvReadEtherAddress(DWORD_PTR dwIoBase, PBYTE pbyEtherAddress);
void SROMvWriteEtherAddress(DWORD_PTR dwIoBase, PBYTE pbyEtherAddress);

VOID SROMvReadSubSysVenId(DWORD_PTR dwIoBase, PDWORD pdwSubSysVenId);

BOOL SROMbAutoLoad (DWORD_PTR dwIoBase);

#endif 
