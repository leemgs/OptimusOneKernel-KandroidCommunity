

#ifndef R819xU_H
#define R819xU_H

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include <linux/usb.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/rtnetlink.h>	
#include <linux/wireless.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>	
#include <linux/if_arp.h>
#include <linux/random.h>
#include <linux/version.h>
#include <asm/io.h>

#include "ieee80211/ieee80211.h"

#include "r8192S_firmware.h"


#define RTL819xU_MODULE_NAME "rtl819xU"

#define FALSE 0
#define TRUE 1
#define MAX_KEY_LEN     61
#define KEY_BUF_SIZE    5

#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000


#define	Rx_Smooth_Factor		20
#define DMESG(x,a...)
#define DMESGW(x,a...)
#define DMESGE(x,a...)
extern u32 rt_global_debug_component;
#define RT_TRACE(component, x, args...) \
do { if(rt_global_debug_component & component) \
	printk(KERN_DEBUG RTL819xU_MODULE_NAME ":" x "\n" , \
	       ##args);\
}while(0);



#define RX_DESC_SIZE 24
#define RX_DRV_INFO_SIZE_UNIT   8

#define IS_UNDER_11N_AES_MODE(_ieee)  ((_ieee->pHTInfo->bCurrentHTSupport==TRUE) &&\
									(_ieee->pairwise_key_type==KEY_TYPE_CCMP))

#define COMP_TRACE				BIT0		
#define COMP_DBG				BIT1		
#define COMP_INIT				BIT2		


#define COMP_RECV				BIT3		
#define COMP_SEND				BIT4		
#define COMP_IO					BIT5		
#define COMP_POWER				BIT6		
#define COMP_EPROM				BIT7		
#define COMP_SWBW				BIT8	
#define COMP_POWER_TRACKING			BIT9	
#define COMP_TURBO				BIT10	
#define COMP_QOS				BIT11	
#define COMP_RATE				BIT12	
#define COMP_LPS					BIT13	
#define COMP_DIG				BIT14	
#define COMP_PHY	 			BIT15
#define COMP_CH					BIT16	
#define COMP_TXAGC				BIT17	
#define COMP_HIPWR				BIT18	
#define COMP_HALDM				BIT19	
#define COMP_SEC			        BIT20	
#define COMP_LED				BIT21	
#define COMP_RF					BIT22	

#define COMP_RXDESC				BIT23	



#define COMP_FIRMWARE				BIT24	
#define COMP_HT					BIT25	
#define COMP_AMSDU				BIT26	

#define COMP_SCAN				BIT27
#define COMP_CMD				BIT28
#define COMP_DOWN				BIT29  
#define COMP_RESET				BIT30  
#define COMP_ERR				BIT31 

#define RTL819x_DEBUG
#ifdef RTL819x_DEBUG
#define assert(expr) \
        if (!(expr)) {                                  \
                printk( "Assertion failed! %s,%s,%s,line=%d\n", \
                #expr,__FILE__,__FUNCTION__,__LINE__);          \
        }


#define RT_DEBUG_DATA(level, data, datalen)      \
        do{ if ((rt_global_debug_component & (level)) == (level))   \
                {       \
                        int i;                                  \
                        u8* pdata = (u8*) data;                 \
                        printk(KERN_DEBUG RTL819xU_MODULE_NAME ": %s()\n", __FUNCTION__);   \
                        for(i=0; i<(int)(datalen); i++)                 \
                        {                                               \
                                printk("%2x ", pdata[i]);               \
                                if ((i+1)%16 == 0) printk("\n");        \
                        }                               \
                        printk("\n");                   \
                }                                       \
        } while (0)
#else
#define assert(expr) do {} while (0)
#define RT_DEBUG_DATA(level, data, datalen) do {} while(0)
#endif 


	
	#define RTL819X_DEFAULT_RF_TYPE				RF_1T2R
	#define RTL819X_TOTAL_RF_PATH				2

	
	
	

	#define Rtl819XMACPHY_Array_PG				Rtl8192UsbMACPHY_Array_PG
	#define Rtl819XMACPHY_Array					Rtl8192UsbMACPHY_Array
	#define Rtl819XPHY_REGArray					Rtl8192UsbPHY_REGArray
	#define Rtl819XPHY_REG_1T2RArray				Rtl8192UsbPHY_REG_1T2RArray
	
	
	#define Rtl819XRadioC_Array					Rtl8192UsbRadioC_Array
	#define Rtl819XRadioD_Array					Rtl8192UsbRadioD_Array

	
	#define Rtl819XFwImageArray					Rtl8192SUFwImgArray
	#define Rtl819XMAC_Array						Rtl8192SUMAC_2T_Array
	#define Rtl819XAGCTAB_Array					Rtl8192SUAGCTAB_Array
	#define Rtl819XPHY_REG_Array					Rtl8192SUPHY_REG_2T2RArray
	#define Rtl819XPHY_REG_to1T1R_Array			Rtl8192SUPHY_ChangeTo_1T1RArray
	#define Rtl819XPHY_REG_to1T2R_Array			Rtl8192SUPHY_ChangeTo_1T2RArray
	#define Rtl819XPHY_REG_to2T2R_Array			Rtl8192SUPHY_ChangeTo_2T2RArray
	#define Rtl819XPHY_REG_Array_PG				Rtl8192SUPHY_REG_Array_PG
	#define Rtl819XRadioA_Array					Rtl8192SURadioA_1T_Array
	#define Rtl819XRadioB_Array					Rtl8192SURadioB_Array
	#define Rtl819XRadioB_GM_Array				Rtl8192SURadioB_GM_Array
	#define Rtl819XRadioA_to1T_Array				Rtl8192SURadioA_to1T_Array
	#define Rtl819XRadioA_to2T_Array				Rtl8192SURadioA_to2T_Array





#define QSLT_BK                                 0x1
#define QSLT_BE                                 0x0
#define QSLT_VI                                 0x4
#define QSLT_VO                                 0x6
#define QSLT_BEACON                             0x10
#define QSLT_HIGH                               0x11
#define QSLT_MGNT                               0x12
#define QSLT_CMD                                0x13

#define DESC90_RATE1M                           0x00
#define DESC90_RATE2M                           0x01
#define DESC90_RATE5_5M                         0x02
#define DESC90_RATE11M                          0x03
#define DESC90_RATE6M                           0x04
#define DESC90_RATE9M                           0x05
#define DESC90_RATE12M                          0x06
#define DESC90_RATE18M                          0x07
#define DESC90_RATE24M                          0x08
#define DESC90_RATE36M                          0x09
#define DESC90_RATE48M                          0x0a
#define DESC90_RATE54M                          0x0b
#define DESC90_RATEMCS0                         0x00
#define DESC90_RATEMCS1                         0x01
#define DESC90_RATEMCS2                         0x02
#define DESC90_RATEMCS3                         0x03
#define DESC90_RATEMCS4                         0x04
#define DESC90_RATEMCS5                         0x05
#define DESC90_RATEMCS6                         0x06
#define DESC90_RATEMCS7                         0x07
#define DESC90_RATEMCS8                         0x08
#define DESC90_RATEMCS9                         0x09
#define DESC90_RATEMCS10                        0x0a
#define DESC90_RATEMCS11                        0x0b
#define DESC90_RATEMCS12                        0x0c
#define DESC90_RATEMCS13                        0x0d
#define DESC90_RATEMCS14                        0x0e
#define DESC90_RATEMCS15                        0x0f
#define DESC90_RATEMCS32                        0x20



#define DESC92S_RATE1M					0x00
#define DESC92S_RATE2M					0x01
#define DESC92S_RATE5_5M				0x02
#define DESC92S_RATE11M					0x03


#define DESC92S_RATE6M					0x04
#define DESC92S_RATE9M					0x05
#define DESC92S_RATE12M					0x06
#define DESC92S_RATE18M					0x07
#define DESC92S_RATE24M					0x08
#define DESC92S_RATE36M					0x09
#define DESC92S_RATE48M					0x0a
#define DESC92S_RATE54M					0x0b


#define DESC92S_RATEMCS0				0x0c
#define DESC92S_RATEMCS1				0x0d
#define DESC92S_RATEMCS2				0x0e
#define DESC92S_RATEMCS3				0x0f
#define DESC92S_RATEMCS4				0x10
#define DESC92S_RATEMCS5				0x11
#define DESC92S_RATEMCS6				0x12
#define DESC92S_RATEMCS7				0x13
#define DESC92S_RATEMCS8				0x14
#define DESC92S_RATEMCS9				0x15
#define DESC92S_RATEMCS10				0x16
#define DESC92S_RATEMCS11				0x17
#define DESC92S_RATEMCS12				0x18
#define DESC92S_RATEMCS13				0x19
#define DESC92S_RATEMCS14				0x1a
#define DESC92S_RATEMCS15				0x1b
#define DESC92S_RATEMCS15_SG			0x1c
#define DESC92S_RATEMCS32				0x20


#define RTL819X_DEFAULT_RF_TYPE RF_1T2R

#define IEEE80211_WATCH_DOG_TIME    2000
#define		PHY_Beacon_RSSI_SLID_WIN_MAX		10

#define 	OFDM_Table_Length	19
#define	CCK_Table_length	12




typedef struct _tx_desc_819x_usb {
	
	u16		PktSize;
	u8		Offset;
	u8		Type:2;	
	u8		LastSeg:1;
	u8		FirstSeg:1;
	u8		LINIP:1;
	u8		AMSDU:1;
	u8		GF:1;
	u8		OWN:1;

	
	u8		MacID:5;
	u8		MoreData:1;
	u8		MOREFRAG:1;
	u8		PIFS:1;
	u8		QueueSelect:5;
	u8		AckPolicy:2;
	u8		NoACM:1;
	u8		NonQos:1;
	u8		KeyID:2;
	u8		OUI:1;
	u8		PktType:1;
	u8		EnDescID:1;
	u8		SecType:2;
	u8		HTC:1;	
	u8		WDS:1;	
	u8		PktOffset:5;	
	u8		HWPC:1;

	
	u32		DataRetryLmt:6;
	u32		RetryLmtEn:1;
	u32		TSFL:5;
	u32		RTSRC:6;	
	u32		DATARC:6;	
	u32		Rsvd1:5;
	u32		AllowAggregation:1;
	u32		BK:1;	
	u32		OwnMAC:1;

	
	u8		NextHeadPage;
	u8		TailPage;
	u16		Seq:12;
	u16		Frag:4;

	
	u32		RTSRate:6;
	u32		DisRTSFB:1;
	u32		RTSRateFBLmt:4;
	u32		CTS2Self:1;
	u32		RTSEn:1;
	u32		RaBRSRID:3;	
	u32		TxHT:1;
	u32		TxShort:1;
	u32		TxBandwidth:1;
	u32		TxSubCarrier:2;
	u32		STBC:2;
	u32		RD:1;
	u32		RTSHT:1;
	u32		RTSShort:1;
	u32		RTSBW:1;
	u32		RTSSubcarrier:2;
	u32		RTSSTBC:2;
	u32		USERATE:1;
	
	u32		PktID:9;
	u32		TxRate:6;
	u32		DISFB:1;
	u32		DataRateFBLmt:5;
	u32		TxAGC:11;

	
	u16		IPChkSum;
	u16		TCPChkSum;

	
	
        u16     	TxBufferSize;
	u16		IPHdrOffset:8;
	u16		Rsvd2:7;
	u16		TCPEn:1;
}tx_desc_819x_usb, *ptx_desc_819x_usb;
typedef struct _tx_status_desc_8192s_usb{

	
	u8		TxRate:6;
	u8		Rsvd1:1;
	u8		BandWidth:1;
	u8		RTSRate:6;
	u8		AGGLS:1;
	u8		AGG:1;
	u8		RTSRC:6;
	u8		DataRC:6;
	u8		FailCause:2;
	u8		TxOK:1;
	u8		Own:1;

	
	u16		Seq:12;
	u8		QueueSel:5;
	u8		MACID:5;
	u8		PwrMgt:1;
	u8		MoreData:1;
	u8		Rsvd2;

	
	u8		RxAGC1;
	u8		RxAGC2;
	u8		RxAGC3;
	u8		RxAGC4;
}tx_status_desc_8192s_usb, *ptx_status_desc_8192s_usb;






typedef struct _tx_desc_cmd_819x_usb{
       
	u16		PktSize;
	u8		Offset;
	u8		Rsvd0:4;
	u8		LINIP:1;
	u8		Rsvd1:2;
	u8		OWN:1;

	
	u32		Rsvd2;
	u32		Rsvd3;
	u32		Rsvd4;
	u32		Rsvd5;
	u32		Rsvd6;
	u32		Rsvd7;

	
	u16		TxBuffSize;
	u16		Rsvd8;
}tx_desc_cmd_819x_usb, *ptx_desc_cmd_819x_usb;



typedef struct _tx_h2c_desc_cmd_8192s_usb{
       
	u32		PktSize:16;
	u32		Offset:8;
	u32		Rsvd0:7;
	u32		OWN:1;

	
	u32		Rsvd1:8;
	u32		QSEL:5;
	u32		Rsvd2:19;

	
	u32		Rsvd3;

	
	u32		NextHeadPage:8;
	u32		TailPage:8;
	u32		Rsvd4:16;

	
	u32		Rsvd5;
	u32		Rsvd6;
	u32		Rsvd7;
	u32		Rsvd8;
}tx_h2c_desc_cmd_8192s_usb, *ptx_h2c_desc_cmd_8192s_usb;


typedef struct _tx_h2c_cmd_hdr_8192s_usb{
       
	u32		CmdLen:16;
	u32		ElementID:8;
	u32		CmdSeq:8;

	
	u32		Rsvd0;
}tx_h2c_cmd_hdr_8192s_usb, *ptx_h2c_cmd_hdr_8192s_usb;

typedef struct _tx_fwinfo_819x_usb{
	
	u8			TxRate:7;
	u8			CtsEnable:1;
	u8			RtsRate:7;
	u8			RtsEnable:1;
	u8			TxHT:1;
	u8			Short:1;						
	u8			TxBandwidth:1;				
	u8			TxSubCarrier:2;				
	u8			STBC:2;
	u8			AllowAggregation:1;
	u8			RtsHT:1;						
	u8			RtsShort:1;					
	u8			RtsBandwidth:1;				
	u8			RtsSubcarrier:2;				
	u8			RtsSTBC:2;
	u8			EnableCPUDur:1;				

	
	u32			RxMF:2;
	u32			RxAMD:3;
        u32			Reserved1:3;
	u32			TxAGCOffSet:4;
	u32			TxAGCSign:1;
	u32			Tx_INFO_RSVD:6;
	u32			PacketID:13;
}tx_fwinfo_819x_usb, *ptx_fwinfo_819x_usb;

typedef struct rtl8192_rx_info {
	struct urb *urb;
	struct net_device *dev;
	u8 out_pipe;
}rtl8192_rx_info ;


typedef struct rx_desc_819x_usb{
	
	u16		Length:14;
	u16		CRC32:1;
	u16		ICV:1;
	u8		RxDrvInfoSize:4;
	u8		Security:3;
	u8		Qos:1;
	u8		Shift:2;
	u8		PHYStatus:1;
	u8		SWDec:1;
	u8		LastSeg:1;
	u8		FirstSeg:1;
	u8		EOR:1;
	u8		Own:1;

	
	u16		MACID:5;
	u16		TID:4;
	u16		HwRsvd:5;
	u16		PAGGR:1;
	u16		FAGGR:1;
	u8		A1_FIT:4;
	u8		A2_FIT:4;
	u8		PAM:1;
	u8		PWR:1;
	u8		MoreData:1;
	u8		MoreFrag:1;
	u8		Type:2;
	u8		MC:1;
	u8		BC:1;

	
	u16		Seq:12;
	u16		Frag:4;
	u8		NextPktLen;
	u8		Rsvd0:6;
	u8		NextIND:1;
	u8		Rsvd1:1;

	
	u8		RxMCS:6;
	u8		RxHT:1;
	u8		AMSDU:1;
	u8		SPLCP:1;
	u8		BW:1;
	u8		HTC:1;
	u8		TCPChkRpt:1;
	u8		IPChkRpt:1;
	u8		TCPChkValID:1;
	u8		HwPCErr:1;
	u8		HwPCInd:1;
	u16		IV0;

	
	u32		IV1;

	
	u32		TSFL;

}rx_desc_819x_usb, *prx_desc_819x_usb;






typedef struct rx_drvinfo_819x_usb{
	
	
	
	

	
	
	u8			gain_trsw[4];

	
	
	u8			pwdb_all;
	u8			cfosho[4];

	
	
	u8			cfotail[4];

	
	
	char			        rxevm[2];
	char			        rxsnr[4];

	
	
	u8			pdsnr[2];

	
	
	u8			csi_current[2];
	u8			csi_target[2];

	
	
	u8			sigevm;
	u8			max_ex_pwr;
	u8			ex_intf_flag:1;
	u8			sgi_en:1;
	u8			rxsc:2;
	u8			reserve:4;

}rx_drvinfo_819x_usb, *prx_drvinfo_819x_usb;

	#define HWSET_MAX_SIZE_92S	128
	#define MAX_802_11_HEADER_LENGTH 40
	#define MAX_PKT_AGG_NUM		256
	#define TX_PACKET_SHIFT_BYTES USB_HWDESC_HEADER_LEN

#define MAX_DEV_ADDR_SIZE		8  
#define MAX_FIRMWARE_INFORMATION_SIZE   32 

#define ENCRYPTION_MAX_OVERHEAD		128
#define	USB_HWDESC_HEADER_LEN		sizeof(tx_desc_819x_usb)

#define MAX_FRAGMENT_COUNT		8
#ifdef RTL8192U
#define MAX_TRANSMIT_BUFFER_SIZE			8000
#else
#define MAX_TRANSMIT_BUFFER_SIZE  	(1600+(MAX_802_11_HEADER_LENGTH+ENCRYPTION_MAX_OVERHEAD)*MAX_FRAGMENT_COUNT)
#endif
#define scrclng					4		

typedef enum rf_optype
{
	RF_OP_By_SW_3wire = 0,
	RF_OP_By_FW,
	RF_OP_MAX
}rf_op_type;

typedef enum _rtl819xUsb_loopback{
	RTL819xU_NO_LOOPBACK = 0,
	RTL819xU_MAC_LOOPBACK = 1,
	RTL819xU_DMA_LOOPBACK = 2,
	RTL819xU_CCK_LOOPBACK = 3,
}rtl819xUsb_loopback_e;


typedef enum _RT_STATUS{
	RT_STATUS_SUCCESS = 0,
	RT_STATUS_FAILURE = 1,
	RT_STATUS_PENDING = 2,
	RT_STATUS_RESOURCE = 3
}RT_STATUS,*PRT_STATUS;


typedef enum _RTL8192SUSB_LOOPBACK{
	RTL8192SU_NO_LOOPBACK = 0,
	RTL8192SU_MAC_LOOPBACK = 1,
	RTL8192SU_DMA_LOOPBACK = 2,
	RTL8192SU_CCK_LOOPBACK = 3,
}RTL8192SUSB_LOOPBACK_E;



#define MAX_RECEIVE_BUFFER_SIZE	9100	



#define NUM_OF_FIRMWARE_QUEUE		10
#define NUM_OF_PAGES_IN_FW		0x100


#define NUM_OF_PAGE_IN_FW_QUEUE_BE	0x020
#define NUM_OF_PAGE_IN_FW_QUEUE_BK	0x020
#define NUM_OF_PAGE_IN_FW_QUEUE_VI	0x040
#define NUM_OF_PAGE_IN_FW_QUEUE_VO	0x040
#define NUM_OF_PAGE_IN_FW_QUEUE_HCCA	0
#define NUM_OF_PAGE_IN_FW_QUEUE_CMD	0x4
#define NUM_OF_PAGE_IN_FW_QUEUE_MGNT	0x20
#define NUM_OF_PAGE_IN_FW_QUEUE_HIGH	0
#define NUM_OF_PAGE_IN_FW_QUEUE_BCN	0x4
#define NUM_OF_PAGE_IN_FW_QUEUE_PUB	0x18


#define APPLIED_RESERVED_QUEUE_IN_FW	0x80000000
#define RSVD_FW_QUEUE_PAGE_BK_SHIFT	0x00
#define RSVD_FW_QUEUE_PAGE_BE_SHIFT	0x08
#define RSVD_FW_QUEUE_PAGE_VI_SHIFT	0x10
#define RSVD_FW_QUEUE_PAGE_VO_SHIFT	0x18
#define RSVD_FW_QUEUE_PAGE_MGNT_SHIFT	0x10
#define RSVD_FW_QUEUE_PAGE_CMD_SHIFT	0x08
#define RSVD_FW_QUEUE_PAGE_BCN_SHIFT	0x00
#define RSVD_FW_QUEUE_PAGE_PUB_SHIFT	0x08



#define EPROM_93c46 0
#define EPROM_93c56 1

#define DEFAULT_FRAG_THRESHOLD 2342U
#define MIN_FRAG_THRESHOLD     256U
#define DEFAULT_BEACONINTERVAL 0x64U
#define DEFAULT_BEACON_ESSID "Rtl819xU"

#define DEFAULT_SSID ""
#define DEFAULT_RETRY_RTS 7
#define DEFAULT_RETRY_DATA 7
#define PRISM_HDR_SIZE 64

#define		PHY_RSSI_SLID_WIN_MAX				100


typedef enum _WIRELESS_MODE {
	WIRELESS_MODE_UNKNOWN = 0x00,
	WIRELESS_MODE_A = 0x01,
	WIRELESS_MODE_B = 0x02,
	WIRELESS_MODE_G = 0x04,
	WIRELESS_MODE_AUTO = 0x08,
	WIRELESS_MODE_N_24G = 0x10,
	WIRELESS_MODE_N_5G = 0x20
} WIRELESS_MODE;


#define RTL_IOCTL_WPA_SUPPLICANT		SIOCIWFIRSTPRIV+30

typedef struct buffer
{
	struct buffer *next;
	u32 *buf;

} buffer;

typedef struct rtl_reg_debug{
        unsigned int  cmd;
        struct {
                unsigned char type;
                unsigned char addr;
                unsigned char page;
                unsigned char length;
        } head;
        unsigned char buf[0xff];
}rtl_reg_debug;

typedef struct _rt_9x_tx_rate_history {
	u32             cck[4];
	u32             ofdm[8];
	
	
	
	
	u32             ht_mcs[4][16];
}rt_tx_rahis_t, *prt_tx_rahis_t;
typedef struct _RT_SMOOTH_DATA_4RF {
	char    elements[4][100];
	u32     index;                  
	u32     TotalNum;               
	u32     TotalVal[4];            
}RT_SMOOTH_DATA_4RF, *PRT_SMOOTH_DATA_4RF;

#define MAX_8192U_RX_SIZE			8192    

typedef struct Stats
{
	unsigned long txrdu;

	
	


	unsigned long rxok;
	unsigned long rxframgment;
	unsigned long rxcmdpkt[4];		
	unsigned long rxurberr;
	unsigned long rxstaterr;
	unsigned long received_rate_histogram[4][32];	
	unsigned long received_preamble_GI[2][32];		
	unsigned long rx_AMPDUsize_histogram[5]; 
	unsigned long rx_AMPDUnum_histogram[5]; 
	unsigned long numpacket_matchbssid;	
	unsigned long numpacket_toself;		
	unsigned long num_process_phyinfo;		
	unsigned long numqry_phystatus;
	unsigned long numqry_phystatusCCK;
	unsigned long numqry_phystatusHT;
	unsigned long received_bwtype[5];              
	unsigned long txnperr;
	unsigned long txnpdrop;
	unsigned long txresumed;



	unsigned long txnpokint;




	unsigned long txoverflow;



	unsigned long txlpokint;
	unsigned long txlpdrop;
	unsigned long txlperr;
	unsigned long txbeokint;
	unsigned long txbedrop;
	unsigned long txbeerr;
	unsigned long txbkokint;
	unsigned long txbkdrop;
	unsigned long txbkerr;
	unsigned long txviokint;
	unsigned long txvidrop;
	unsigned long txvierr;
	unsigned long txvookint;
	unsigned long txvodrop;
	unsigned long txvoerr;
	unsigned long txbeaconokint;
	unsigned long txbeacondrop;
	unsigned long txbeaconerr;
	unsigned long txmanageokint;
	unsigned long txmanagedrop;
	unsigned long txmanageerr;
	unsigned long txdatapkt;
	unsigned long txfeedback;
	unsigned long txfeedbackok;

	unsigned long txoktotal;
	unsigned long txokbytestotal;
	unsigned long txokinperiod;
	unsigned long txmulticast;
	unsigned long txbytesmulticast;
	unsigned long txbroadcast;
	unsigned long txbytesbroadcast;
	unsigned long txunicast;
	unsigned long txbytesunicast;

	unsigned long rxoktotal;
	unsigned long rxbytesunicast;
	unsigned long txfeedbackfail;
	unsigned long txerrtotal;
	unsigned long txerrbytestotal;
	unsigned long txerrmulticast;
	unsigned long txerrbroadcast;
	unsigned long txerrunicast;
	unsigned long txretrycount;
	unsigned long txfeedbackretry;
	u8	      last_packet_rate;
	unsigned long slide_signal_strength[100];
	unsigned long slide_evm[100];
	unsigned long slide_rssi_total;	
	unsigned long slide_evm_total;	
	long signal_strength; 
	long signal_quality;
	long last_signal_strength_inpercent;
	long recv_signal_power;	
	u8 rx_rssi_percentage[4];
	u8 rx_evm_percentage[2];
	long rxSNRdB[4];
	rt_tx_rahis_t txrate;
	u32 Slide_Beacon_pwdb[100];     
	u32 Slide_Beacon_Total;         
	RT_SMOOTH_DATA_4RF              cck_adc_pwdb;

	u32	CurrentShowTxate;
} Stats;



#define HAL_PRIME_CHNL_OFFSET_DONT_CARE		0
#define HAL_PRIME_CHNL_OFFSET_LOWER			1
#define HAL_PRIME_CHNL_OFFSET_UPPER			2



typedef struct 	ChnlAccessSetting {
	u16 SIFS_Timer;
	u16 DIFS_Timer;
	u16 SlotTimeTimer;
	u16 EIFS_Timer;
	u16 CWminIndex;
	u16 CWmaxIndex;
}*PCHANNEL_ACCESS_SETTING,CHANNEL_ACCESS_SETTING;

typedef struct _BB_REGISTER_DEFINITION{
	u32 rfintfs; 			
	u32 rfintfi; 			
	u32 rfintfo; 			
	u32 rfintfe; 			
	u32 rf3wireOffset; 		
	u32 rfLSSI_Select; 		
	u32 rfTxGainStage;		
	u32 rfHSSIPara1; 		
	u32 rfHSSIPara2; 		
	u32 rfSwitchControl; 	
	u32 rfAGCControl1; 	
	u32 rfAGCControl2; 	
	u32 rfRxIQImbalance; 	
	u32 rfRxAFE;  			
	u32 rfTxIQImbalance; 	
	u32 rfTxAFE; 			
	u32 rfLSSIReadBack; 	
	u32 rfLSSIReadBackPi; 	
}BB_REGISTER_DEFINITION_T, *PBB_REGISTER_DEFINITION_T;

typedef enum _RT_RF_TYPE_819xU{
        RF_TYPE_MIN = 0,
        RF_8225,
        RF_8256,
        RF_8258,
	RF_6052=4,		
        RF_PSEUDO_11N = 5,
}RT_RF_TYPE_819xU, *PRT_RF_TYPE_819xU;


typedef enum _RF_POWER_STATE{
	RF_ON,
	RF_SLEEP,
	RF_OFF,
	RF_SHUT_DOWN,
}RF_POWER_STATE, *PRF_POWER_STATE;


typedef struct _rate_adaptive
{
	u8				rate_adaptive_disabled;
	u8				ratr_state;
	u16				reserve;

	u32				high_rssi_thresh_for_ra;
	u32				high2low_rssi_thresh_for_ra;
	u8				low2high_rssi_thresh_for_ra40M;
	u32				low_rssi_thresh_for_ra40M;
	u8				low2high_rssi_thresh_for_ra20M;
	u32				low_rssi_thresh_for_ra20M;
	u32				upper_rssi_threshold_ratr;
	u32				middle_rssi_threshold_ratr;
	u32				low_rssi_threshold_ratr;
	u32				low_rssi_threshold_ratr_40M;
	u32				low_rssi_threshold_ratr_20M;
	u8				ping_rssi_enable;	
	u32				ping_rssi_ratr;	
	u32				ping_rssi_thresh_for_ra;
	u32				last_ratr;

} rate_adaptive, *prate_adaptive;

#define TxBBGainTableLength 37
#define	CCKTxBBGainTableLength 23

typedef struct _txbbgain_struct
{
	long	txbb_iq_amplifygain;
	u32	txbbgain_value;
} txbbgain_struct, *ptxbbgain_struct;

typedef struct _ccktxbbgain_struct
{
	
	u8	ccktxbb_valuearray[8];
} ccktxbbgain_struct,*pccktxbbgain_struct;


typedef struct _init_gain
{
	u8				xaagccore1;
	u8				xbagccore1;
	u8				xcagccore1;
	u8				xdagccore1;
	u8				cca;

} init_gain, *pinit_gain;


typedef struct _phy_ofdm_rx_status_report_819xusb
{
	u8	trsw_gain_X[4];
	u8	pwdb_all;
	u8	cfosho_X[4];
	u8	cfotail_X[4];
	u8	rxevm_X[2];
	u8	rxsnr_X[4];
	u8	pdsnr_X[2];
	u8	csi_current_X[2];
	u8	csi_target_X[2];
	u8	sigevm;
	u8	max_ex_pwr;
	u8	sgi_en;
	u8  rxsc_sgien_exflg;
}phy_sts_ofdm_819xusb_t;

typedef struct _phy_cck_rx_status_report_819xusb
{
	
	u8	adc_pwdb_X[4];
	u8	sq_rpt;
	u8	cck_agc_rpt;
}phy_sts_cck_819xusb_t;


typedef struct _phy_ofdm_rx_status_rxsc_sgien_exintfflag{
	u8			reserved:4;
	u8			rxsc:2;
	u8			sgi_en:1;
	u8			ex_intf_flag:1;
}phy_ofdm_rx_status_rxsc_sgien_exintfflag;

typedef enum _RT_CUSTOMER_ID
{
	RT_CID_DEFAULT = 0,
	RT_CID_8187_ALPHA0 = 1,
	RT_CID_8187_SERCOMM_PS = 2,
	RT_CID_8187_HW_LED = 3,
	RT_CID_8187_NETGEAR = 4,
	RT_CID_WHQL = 5,
	RT_CID_819x_CAMEO  = 6,
	RT_CID_819x_RUNTOP = 7,
	RT_CID_819x_Senao = 8,
	RT_CID_TOSHIBA = 9,	
	RT_CID_819x_Netcore = 10,
	RT_CID_Nettronix = 11,
	RT_CID_DLINK = 12,
	RT_CID_PRONET = 13,
}RT_CUSTOMER_ID, *PRT_CUSTOMER_ID;





typedef	enum _LED_STRATEGY_8190{
	SW_LED_MODE0, 
	SW_LED_MODE1, 
	SW_LED_MODE2, 
	SW_LED_MODE3, 
	SW_LED_MODE4, 
	HW_LED, 
}LED_STRATEGY_8190, *PLED_STRATEGY_8190;

typedef enum _RESET_TYPE {
	RESET_TYPE_NORESET = 0x00,
	RESET_TYPE_NORMAL = 0x01,
	RESET_TYPE_SILENT = 0x02
} RESET_TYPE;


typedef enum _tag_TxCmd_Config_Index{
	TXCMD_TXRA_HISTORY_CTRL				= 0xFF900000,
	TXCMD_RESET_TX_PKT_BUFF				= 0xFF900001,
	TXCMD_RESET_RX_PKT_BUFF				= 0xFF900002,
	TXCMD_SET_TX_DURATION				= 0xFF900003,
	TXCMD_SET_RX_RSSI						= 0xFF900004,
	TXCMD_SET_TX_PWR_TRACKING			= 0xFF900005,
	TXCMD_XXXX_CTRL,
}DCMD_TXCMD_OP;

typedef enum{
	NIC_8192U = 1,
	NIC_8190P = 2,
	NIC_8192E = 3,
	NIC_8192SE = 4,
	NIC_8192SU = 5,
	} nic_t;



struct rtl819x_ops{
	nic_t nic_type;
	void (* rtl819x_read_eeprom_info)(struct net_device *dev);
	short (* rtl819x_tx)(struct net_device *dev, struct sk_buff* skb);
	short (* rtl819x_tx_cmd)(struct net_device *dev, struct sk_buff *skb);
	void (* rtl819x_rx_nomal)(struct sk_buff* skb);
	void (* rtl819x_rx_cmd)(struct sk_buff *skb);
	bool (*	rtl819x_adapter_start)(struct net_device *dev);
	void (* rtl819x_link_change)(struct net_device *dev);
	void (*	rtl819x_initial_gain)(struct net_device *dev,u8 Operation);
	void (*	rtl819x_query_rxdesc_status)(struct sk_buff *skb, struct ieee80211_rx_stats *stats, bool bIsRxAggrSubframe);
};

typedef struct r8192_priv
{
	struct rtl819x_ops* ops;
	struct usb_device *udev;
	
	short epromtype;
	u16 eeprom_vid;
	u16 eeprom_pid;
	u8  eeprom_CustomerID;
	u8  eeprom_SubCustomerID;
	u8  eeprom_ChannelPlan;
	RT_CUSTOMER_ID CustomerID;
	LED_STRATEGY_8190	LedStrategy;
	u8  txqueue_to_outpipemap[9];
	u8  RtOutPipes[16];
	u8  RtInPipes[16];
	u8  ep_in_num;
	u8  ep_out_num;
	u8  ep_num;
	int irq;
	struct ieee80211_device *ieee80211;

	short card_8192; 
	u8 card_8192_version; 

	short enable_gpio0;
	enum card_type {PCI,MINIPCI,CARDBUS,USB}card_type;
	short hw_plcp_len;
	short plcp_preamble_mode;

	spinlock_t irq_lock;

	spinlock_t tx_lock;
	spinlock_t ps_lock;
        struct mutex mutex;
	spinlock_t rf_lock; 

	u16 irq_mask;


	short chan;
	short sens;
	short max_sens;


	




	short up;
	short crcmon; 



	


	
	struct semaphore wx_sem;
	struct semaphore rf_sem; 







	u8 rf_type; 
	RT_RF_TYPE_819xU rf_chip;


	short (*rf_set_sens)(struct net_device *dev,short sens);
	u8 (*rf_set_chan)(struct net_device *dev,u8 ch);
	void (*rf_close)(struct net_device *dev);
	void (*rf_init)(struct net_device *dev);
	
	short promisc;
	
	struct Stats stats;
	struct iw_statistics wstats;
	struct proc_dir_entry *dir_dev;

	



	struct urb **rx_urb;
	struct urb **rx_cmd_urb;


       struct sk_buff_head rx_queue;
       struct sk_buff_head skb_queue;

       struct work_struct qos_activate;

	short  tx_urb_index;
	atomic_t tx_pending[0x10];


	struct tasklet_struct irq_rx_tasklet;
	struct urb *rxurb_task;

	
	u16	ShortRetryLimit;
	u16	LongRetryLimit;
	u32	TransmitConfig;
	u8	RegCWinMin;		

	u32     LastRxDescTSFHigh;
	u32     LastRxDescTSFLow;


	
	u16	EarlyRxThreshold;
	u32	ReceiveConfig;
	u8	AcmControl;

	u8	RFProgType;

	u8 retry_data;
	u8 retry_rts;
	u16 rts;

	struct 	ChnlAccessSetting  ChannelAccessSetting;

	struct work_struct reset_wq;


	
	u16     basic_rate;
	u8      short_preamble;
	u8      slot_time;
	bool 	bDcut;
	bool bCurrentRxAggrEnable;
	u8 Rf_Mode; 
	prt_firmware		pFirmware;
	rtl819xUsb_loopback_e	LoopbackMode;
	firmware_source_e	firmware_source;
	bool usb_error;

	u16 EEPROMTxPowerDiff;
	u8 EEPROMThermalMeter;
	u8 EEPROMPwDiff;
	u8 EEPROMCrystalCap;
	u8 EEPROM_Def_Ver;
	u8 EEPROMTxPowerLevelCCK;
	u8 EEPROMTxPowerLevelCCK_V1[3];
	u8 EEPROMTxPowerLevelOFDM24G[3]; 
	u8 EEPROMTxPowerLevelOFDM5G[24];	


	bool	bDmDisableProtect;
	bool	bIgnoreDiffRateTxPowerOffset;

	
	u8  EEPROMRfACCKChnl1TxPwLevel[3];	
	u8  EEPROMRfAOfdmChnlTxPwLevel[3];
	u8  EEPROMRfCCCKChnl1TxPwLevel[3];	
	u8  EEPROMRfCOfdmChnlTxPwLevel[3];

	
	
	u8  RfCckChnlAreaTxPwr[2][3];
	u8  RfOfdmChnlAreaTxPwr1T[2][3];
	u8  RfOfdmChnlAreaTxPwr2T[2][3];

	
	bool		EepromOrEfuse;
	bool		bBootFromEfuse;	
	u8  		EfuseMap[2][HWSET_MAX_SIZE_92S];

	u8  		EEPROMUsbOption;
	u8  		EEPROMUsbPhyParam[5];
	u8  		EEPROMTxPwrBase;
	u8  		EEPROMBoardType;
	bool		bBootFromEEPROM;   
	u8  		EEPROMTSSI_A;
	u8  		EEPROMTSSI_B;
	u8  		EEPROMHT2T_TxPwr[6];			
	u8  		EEPROMTxPwrTkMode;

	u8  		bTXPowerDataReadFromEEPORM;

	u8		EEPROMVersion;
	u8		EEPROMUsbEndPointNumber;

	bool		AutoloadFailFlag;
	u8	RfTxPwrLevelCck[2][14];
	u8	RfTxPwrLevelOfdm1T[2][14];
	u8	RfTxPwrLevelOfdm2T[2][14];
	
	u8					TxPwrHt20Diff[2][14];				
	u8					TxPwrLegacyHtDiff[2][14];		
	u8					TxPwrbandEdgeHt40[2][2];		
	u8					TxPwrbandEdgeHt20[2][2];		
	u8					TxPwrbandEdgeLegacyOfdm[2][2];	
	u8					TxPwrbandEdgeFlag;				

	
	u8 				MidHighPwrTHR_L1;
	u8 				MidHighPwrTHR_L2;
	u8				TxPwrSafetyFlag;				



	BB_REGISTER_DEFINITION_T	PHYRegDef[4];	
	
	u32	MCSTxPowerLevelOriginalOffset[7];
	u32	CCKTxPowerLevelOriginalOffset;
	u8	TxPowerLevelCCK[14];			
	u8	TxPowerLevelOFDM24G[14];		
	u8	TxPowerLevelOFDM5G[14];			
	u32	Pwr_Track;
	u8	TxPowerDiff;
	u8	AntennaTxPwDiff[2];				
	u8	CrystalCap;						
	u8	ThermalMeter[2];				

	u8	CckPwEnl;
	
	u8	bCckHighPower;
	long	undecorated_smoothed_pwdb;

	
	u8	SwChnlInProgress;
	u8 	SwChnlStage;
	u8	SwChnlStep;
	u8	SetBWModeInProgress;
	HT_CHANNEL_WIDTH		CurrentChannelBW;
	u8      ChannelPlan;
	u8      pwrGroupCnt;
	
	
	u8	nCur40MhzPrimeSC;	
	
	
	
	u32					RfReg0Value[4];
	u8 					NumTotalRFPath;
	bool 				brfpath_rxenable[4];
	
	bool				SetRFPowerStateInProgress;

	struct timer_list watch_dog_timer;


	
	bool	bdynamic_txpower;  
	bool	bDynamicTxHighPower;  
	bool	bDynamicTxLowPower;  
	bool	bLastDTPFlag_High;
	bool	bLastDTPFlag_Low;

	bool	bstore_last_dtpflag;
	bool	bstart_txctrl_bydtp;   
	
	rate_adaptive rate_adaptive;
	
	
       txbbgain_struct txbbgain_table[TxBBGainTableLength];
       u8	EEPROMTxPowerTrackEnable;
	u8			   txpower_count;
	bool			   btxpower_trackingInit;
	u8			   OFDM_index;
	u8			   CCK_index;
	
	ccktxbbgain_struct	cck_txbbgain_table[CCKTxBBGainTableLength];
	ccktxbbgain_struct	cck_txbbgain_ch14_table[CCKTxBBGainTableLength];
	u8 rfa_txpowertrackingindex;
	u8 rfa_txpowertrackingindex_real;
	u8 rfa_txpowertracking_default;
	u8 rfc_txpowertrackingindex;
	u8 rfc_txpowertrackingindex_real;

	s8 cck_present_attentuation;
	u8 cck_present_attentuation_20Mdefault;
	u8 cck_present_attentuation_40Mdefault;
	char cck_present_attentuation_difference;
	bool btxpower_tracking;
	bool bcck_in_ch14;
	bool btxpowerdata_readfromEEPORM;
	u16 	TSSI_13dBm;
	
	init_gain initgain_backup;
	u8 DefaultInitialGain[4];
	
	bool		bis_any_nonbepkts;
	bool		bcurrent_turbo_EDCA;
	bool		bis_cur_rdlstate;
	struct timer_list fsync_timer;
	bool bfsync_processing;	
	u32 	rate_record;
	u32 	rateCountDiffRecord;
	u32	ContiuneDiffCount;
	bool bswitch_fsync;

	u8	framesync;
	u32 	framesyncC34;
	u8   	framesyncMonitor;
        	
	u16 	nrxAMPDU_size;
	u8 	nrxAMPDU_aggr_num;

	
	 bool bHwRadioOff;

	
	u32 reset_count;
	bool bpbc_pressed;
	
	u32 txpower_checkcnt;
	u32 txpower_tracking_callback_cnt;
	u8 thermal_read_val[40];
	u8 thermal_readback_index;
	u32 ccktxpower_adjustcnt_not_ch14;
	u32 ccktxpower_adjustcnt_ch14;
	u8 tx_fwinfo_force_subcarriermode;
	u8 tx_fwinfo_force_subcarrierval;
	
	RESET_TYPE	ResetProgress;
	bool		bForcedSilentReset;
	bool		bDisableNormalResetCheck;
	u16		TxCounter;
	u16		RxCounter;
	int		IrpPendingCount;
	bool		bResetInProgress;
	bool		force_reset;
	u8		InitialGainOperateType;

	u16		SifsTime;

	
	struct delayed_work update_beacon_wq;
	struct delayed_work watch_dog_wq;
	struct delayed_work txpower_tracking_wq;
	struct delayed_work rfpath_check_wq;
	struct delayed_work gpio_change_rf_wq;
	struct delayed_work initialgain_operate_wq;

	struct workqueue_struct *priv_wq;

	
	 u32 			IntrMask;
	
	bool				bChangeBBInProgress; 
	bool				bChangeRFInProgress; 

	u32				CCKTxPowerAdjustCntCh14;		
	u32				CCKTxPowerAdjustCntNotCh14;	
	u32				TXPowerTrackingCallbackCnt;		
	u32				TxPowerCheckCnt;				
	u32				RFWritePageCnt[3];				
	u32				RFReadPageCnt[3];				
	u8				ThermalReadBackIndex;			
	u8				ThermalReadVal[40];				

	
	
	bool				bInHctTest;

	
	u8				CurrentCckTxPwrIdx;
	u8				CurrentOfdm24GTxPwrIdx;

	
	u8					TxPowerLevelCCK_A[14];			
	u8					TxPowerLevelOFDM24G_A[14];	
	u8					TxPowerLevelCCK_C[14];			
	u8					TxPowerLevelOFDM24G_C[14];	
	u8					LegacyHTTxPowerDiff;			
	char					RF_C_TxPwDiff;					

	bool	bRFSiOrPi;
	

	bool SetFwCmdInProgress; 
	u8 CurrentFwCmdIO;

	u8 MinSpaceCfg;

	u16 rf_pathmap;






}r8192_priv;





typedef enum{
	BULK_PRIORITY = 0x01,
	
	
	LOW_PRIORITY,
	NORM_PRIORITY,
	VO_PRIORITY,
	VI_PRIORITY, 
	BE_PRIORITY,
	BK_PRIORITY,
	RSVD2,
	RSVD3,
	BEACON_PRIORITY, 
	HIGH_PRIORITY,
	MANAGE_PRIORITY,
	RSVD4,
	RSVD5,
	UART_PRIORITY 
} priority_t;

#ifdef JOHN_HWSEC
struct ssid_thread {
	struct net_device *dev;
       	u8 name[IW_ESSID_MAX_SIZE + 1];
};
#endif

short rtl8192SU_tx_cmd(struct net_device *dev, struct sk_buff *skb);
short rtl8192SU_tx(struct net_device *dev, struct sk_buff* skb);
bool FirmwareDownload92S(struct net_device *dev);

short rtl819xU_tx_cmd(struct net_device *dev, struct sk_buff *skb);
short rtl8192_tx(struct net_device *dev, struct sk_buff* skb);

u32 read_cam(struct net_device *dev, u8 addr);
void write_cam(struct net_device *dev, u8 addr, u32 data);

u8 read_nic_byte(struct net_device *dev, int x);
u8 read_nic_byte_E(struct net_device *dev, int x);
u32 read_nic_dword(struct net_device *dev, int x);
u16 read_nic_word(struct net_device *dev, int x) ;
void write_nic_byte(struct net_device *dev, int x,u8 y);
void write_nic_byte_E(struct net_device *dev, int x,u8 y);
void write_nic_word(struct net_device *dev, int x,u16 y);
void write_nic_dword(struct net_device *dev, int x,u32 y);
void force_pci_posting(struct net_device *dev);

void rtl8192_rtx_disable(struct net_device *);
void rtl8192_rx_enable(struct net_device *);
void rtl8192_tx_enable(struct net_device *);

void rtl8192_disassociate(struct net_device *dev);

void rtl8185_set_rf_pins_enable(struct net_device *dev,u32 a);

void rtl8192_set_anaparam(struct net_device *dev,u32 a);
void rtl8185_set_anaparam2(struct net_device *dev,u32 a);
void rtl8192_update_msr(struct net_device *dev);
int rtl8192_down(struct net_device *dev);
int rtl8192_up(struct net_device *dev);
void rtl8192_commit(struct net_device *dev);
void rtl8192_set_chan(struct net_device *dev,short ch);
void write_phy(struct net_device *dev, u8 adr, u8 data);
void write_phy_cck(struct net_device *dev, u8 adr, u32 data);
void write_phy_ofdm(struct net_device *dev, u8 adr, u32 data);
void rtl8185_tx_antenna(struct net_device *dev, u8 ant);
void rtl8192_set_rxconf(struct net_device *dev);

extern void rtl819xusb_beacon_tx(struct net_device *dev,u16  tx_rate);
void CamResetAllEntry(struct net_device* dev);
void EnableHWSecurityConfig8192(struct net_device *dev);
void setKey(struct net_device *dev, u8 EntryNo, u8 KeyIndex, u16 KeyType, u8 *MacAddr, u8 DefaultKey, u32 *KeyContent );
short rtl8192_is_tx_queue_empty(struct net_device *dev);

#endif
