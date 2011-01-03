

#ifndef __RF_H__
#define __RF_H__

#include "ttype.h"
#include "device.h"





#define RF_RFMD2959             0x01
#define RF_MAXIMAG              0x02
#define RF_AIROHA               0x03


#define RF_UW2451               0x05
#define RF_MAXIMG               0x06
#define RF_MAXIM2829            0x07 
#define RF_UW2452               0x08 
#define RF_AIROHA7230           0x0a 
#define RF_UW2453               0x0b

#define RF_VT3226               0x09
#define RF_AL2230S              0x0e

#define RF_NOTHING              0x7E
#define RF_EMU                  0x80
#define RF_MASK                 0x7F

#define ZONE_FCC                0
#define ZONE_MKK1               1
#define ZONE_ETSI               2
#define ZONE_IC                 3
#define ZONE_SPAIN              4
#define ZONE_FRANCE             5
#define ZONE_MKK                6
#define ZONE_ISRAEL             7


#define CB_MAXIM2829_CHANNEL_5G_HIGH    41 
#define CB_UW2452_CHANNEL_5G_HIGH       41 








BOOL IFRFbWriteEmbeded(DWORD_PTR dwIoBase, DWORD dwData);
BOOL RFbSelectChannel(DWORD_PTR dwIoBase, BYTE byRFType, BYTE byChannel);
BOOL RFbInit (
    IN  PSDevice  pDevice
    );
BOOL RFvWriteWakeProgSyn(DWORD_PTR dwIoBase, BYTE byRFType, UINT uChannel);
BOOL RFbSetPower(PSDevice pDevice, UINT uRATE, UINT uCH);
BOOL RFbRawSetPower(
    IN  PSDevice  pDevice,
    IN  BYTE      byPwr,
    IN  UINT      uRATE
    );

VOID
RFvRSSITodBm(
    IN  PSDevice pDevice,
    IN  BYTE     byCurrRSSI,
    long    *pldBm
    );


BOOL RFbAL7230SelectChannelPostProcess(DWORD_PTR dwIoBase, BYTE byOldChannel, BYTE byNewChannel);


#endif 



