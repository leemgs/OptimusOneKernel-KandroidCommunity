#ifndef __WINBOND_CORE_H
#define __WINBOND_CORE_H

#include <linux/wireless.h>

#include "mlme_s.h"
#include "wbhal_s.h"
#include "mto.h"

#define WBLINUX_PACKET_ARRAY_SIZE (ETHERNET_TX_DESCRIPTORS*4)

#define WB_MAX_LINK_NAME_LEN 40

struct wbsoft_priv {
	u32 adapterIndex;	

	WB_LOCALDESCRIPT sLocalPara;	

	MLME_FRAME sMlmeFrame;	

	struct wb35_mto_params sMtoPara;	
	struct hw_data sHwData;	
	struct wb35_mds Mds;

	spinlock_t SpinLock;

	atomic_t ThreadCount;

	u32 RxByteCount;
	u32 TxByteCount;

	struct sk_buff *packet_return;
	s32 netif_state_stop;	
	struct iw_statistics iw_stats;

	u8 LinkName[WB_MAX_LINK_NAME_LEN];

	bool enabled;
};

#endif 
