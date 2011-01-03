

#include "cprecomp.h"



















zbuf_t *zfGetAmsduSubFrame(zdev_t *dev, zbuf_t *buf, u16_t *offset)
{
	u16_t subframeLen;
	u16_t amsduLen = zfwBufGetSize(dev, buf);
	zbuf_t *newBuf;

	ZM_PERFORMANCE_RX_AMSDU(dev, buf, amsduLen);

	
	if (amsduLen < (*offset + 14))
		return NULL;

	
	subframeLen = (zmw_buf_readb(dev, buf, *offset + 12) << 8) +
		zmw_buf_readb(dev, buf, *offset + 13);

	if (subframeLen == 0)
		return NULL;

	
	if ((*offset+14+subframeLen) <= amsduLen) {
		
		newBuf = zfwBufAllocate(dev, 24+2+subframeLen);
		if (newBuf != NULL) {
			#ifdef ZM_ENABLE_NATIVE_WIFI
			
			zfRxBufferCopy(dev, newBuf, buf, 0, 0, 24);
			zfRxBufferCopy(dev, newBuf, buf, 24, *offset+14,
					subframeLen);
			zfwBufSetSize(dev, newBuf, 24+subframeLen);
			#else
			
			zfRxBufferCopy(dev, newBuf, buf, 0, *offset,
					14+subframeLen);
			zfwBufSetSize(dev, newBuf, 14+subframeLen);
			#endif
			
			*offset += (((14+subframeLen)+3) & 0xfffc);

			
			return newBuf;
		}
	}
	return NULL;
}



















void zfDeAmsdu(zdev_t *dev, zbuf_t *buf, u16_t vap, u8_t encryMode)
{
	u16_t offset = ZM_SIZE_OF_WLAN_DATA_HEADER+ZM_SIZE_OF_QOS_CTRL;
	zbuf_t *subframeBuf;
	zmw_get_wlan_dev(dev);

	ZM_BUFFER_TRACE(dev, buf)

	if (encryMode == ZM_AES || encryMode == ZM_TKIP)
		offset += (ZM_SIZE_OF_IV + ZM_SIZE_OF_EXT_IV);
	else if (encryMode == ZM_WEP64 || encryMode == ZM_WEP128)
		offset += ZM_SIZE_OF_IV;


	
	while ((subframeBuf = zfGetAmsduSubFrame(dev, buf, &offset)) != NULL) {
		wd->commTally.NotifyNDISRxFrmCnt++;
		if (wd->zfcbRecvEth != NULL) {
			wd->zfcbRecvEth(dev, subframeBuf, (u8_t)vap);
			ZM_PERFORMANCE_RX_MSDU(dev, wd->tick);
		}
	}
	zfwBufFree(dev, buf, 0);

	return;
}
