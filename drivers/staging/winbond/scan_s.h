#ifndef __WINBOND_SCAN_S_H
#define __WINBOND_SCAN_S_H

#include <linux/types.h>
#include "localpara.h"







#define SCANMSG_SCAN_REQ			0x01
#define SCANMSG_BEACON				0x02
#define SCANMSG_PROBE_RESPONSE		0x03
#define SCANMSG_TIMEOUT				0x04
#define SCANMSG_TXPROBE_FAIL		0x05
#define SCANMSG_ENABLE_BGSCAN		0x06
#define SCANMSG_STOP_SCAN			0x07




#define IBSS_NET			0
#define ESS_NET				1
#define ANYBSS_NET			2


#define ACTIVE_SCAN			0
#define PASSIVE_SCAN		1



typedef struct _SCAN_REQ_PARA	
{
	u32				ScanType;			

	CHAN_LIST		sChannelList;	
	u8			reserved_1[2];

	struct SSID_Element	sSSID; 
	u8			reserved_2[2];

} SCAN_REQ_PARA, *psSCAN_REQ_PARA;

typedef struct _SCAN_PARAMETERS
{
	u16				wState;
	u16				iCurrentChannelIndex;

	SCAN_REQ_PARA	sScanReq;

	u8				BSSID[MAC_ADDR_LENGTH + 2];		

	u32				BssType;						

	
	u16				ProbeDelay;
	u16				MinChannelTime;

	u16				MaxChannelTime;
	u16				reserved_1;

    s32				iBgScanPeriod;				

    u8				boBgScan;					
    u8				boFastScan;					
	u8				boCCAbusy;					
	u8				reserved_2;

	struct timer_list timer;

	u32				ScanTimeStamp;			
	u32				BssTimeStamp;			
	u32				RxNumPerAntenna[2];		

	u8				AntennaToggle;			
	u8				boInTimerHandler;
	u8 				boTimerActive;				
	u8				boSave;

	u32				BScanEnable; 

} SCAN_PARAMETERS, *psSCAN_PARAMETERS;


#define psSCAN			(&(adapter->sScanPara))
#define psSCANREQ			(&(adapter->sScanPara.sScanReq))









#define SCAN_INACTIVE						0
#define WAIT_PROBE_DELAY					1
#define WAIT_RESPONSE_MIN					2
#define WAIT_RESPONSE_MAX_ACTIVE			3
#define WAIT_BEACON_MAX_PASSIVE				4
#define SCAN_COMPLETE						5
#define BG_SCAN								6
#define BG_SCANNING							7















#endif
