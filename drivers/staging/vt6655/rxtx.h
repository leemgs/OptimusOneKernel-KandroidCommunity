

#ifndef __RXTX_H__
#define __RXTX_H__

#include "ttype.h"
#include "device.h"
#include "wcmd.h"










#ifdef __cplusplus
extern "C" {                            
#endif 




VOID
vGenerateMACHeader (
    IN PSDevice         pDevice,
    IN PBYTE            pbyBufferAddr,
    IN WORD             wDuration,
    IN PSEthernetHeader psEthHeader,
    IN BOOL             bNeedEncrypt,
    IN WORD             wFragType,
    IN UINT             uDMAIdx,
    IN UINT             uFragIdx
    );


UINT
cbGetFragCount(
    IN  PSDevice         pDevice,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             cbFrameBodySize,
    IN  PSEthernetHeader psEthHeader
    );


VOID
vGenerateFIFOHeader (
    IN  PSDevice         pDevice,
    IN  BYTE             byPktTyp,
    IN  PBYTE            pbyTxBufferAddr,
    IN  BOOL             bNeedEncrypt,
    IN  UINT             cbPayloadSize,
    IN  UINT             uDMAIdx,
    IN  PSTxDesc         pHeadTD,
    IN  PSEthernetHeader psEthHeader,
    IN  PBYTE            pPacket,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             uNodeIndex,
    OUT PUINT            puMACfragNum,
    OUT PUINT            pcbHeaderSize
    );


VOID vDMA0_tx_80211(PSDevice  pDevice, struct sk_buff *skb, PBYTE pbMPDU, UINT cbMPDULen);
CMD_STATUS csMgmt_xmit(PSDevice pDevice, PSTxMgmtPacket pPacket);
CMD_STATUS csBeacon_xmit(PSDevice pDevice, PSTxMgmtPacket pPacket);

#ifdef __cplusplus
}                                       
#endif 




#endif 



