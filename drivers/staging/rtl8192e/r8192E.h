

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
#include <linux/pci.h>

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
#include "ieee80211.h"




#define RTL819xE_MODULE_NAME "rtl819xE"

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

#define		PHY_RSSI_SLID_WIN_MAX				100
#define		PHY_Beacon_RSSI_SLID_WIN_MAX		10

#define IC_VersionCut_D	0x3
#define IC_VersionCut_E	0x4

#if 0 
#define DMESG(x,a...) printk(KERN_INFO RTL819xE_MODULE_NAME ": " x "\n", ## a)
#define DMESGW(x,a...) printk(KERN_WARNING RTL819xE_MODULE_NAME ": WW:" x "\n", ## a)
#define DMESGE(x,a...) printk(KERN_WARNING RTL819xE_MODULE_NAME ": EE:" x "\n", ## a)
#else
#define DMESG(x,a...)
#define DMESGW(x,a...)
#define DMESGE(x,a...)
extern u32 rt_global_debug_component;
#define RT_TRACE(component, x, args...) \
do { if(rt_global_debug_component & component) \
	printk(KERN_DEBUG RTL819xE_MODULE_NAME ":" x "\n" , \
	       ##args);\
}while(0);

#define COMP_TRACE				BIT0		
#define COMP_DBG				BIT1		
#define COMP_INIT				BIT2		


#define COMP_RECV				BIT3		
#define COMP_SEND				BIT4		
#define COMP_IO					BIT5		
#define COMP_POWER				BIT6		
#define COMP_EPROM				BIT7		
#define COMP_SWBW				BIT8	
#define COMP_SEC				BIT9


#define COMP_TURBO				BIT10	
#define COMP_QOS				BIT11	

#define COMP_RATE				BIT12	
#define COMP_RXDESC			        BIT13	
#define COMP_PHY				BIT14
#define COMP_DIG				BIT15	
#define COMP_TXAGC				BIT16	
#define COMP_HALDM				BIT17	
#define COMP_POWER_TRACKING	                BIT18	
#define COMP_EVENTS			        BIT19	

#define COMP_RF					BIT20	




#define COMP_FIRMWARE			        BIT21	
#define COMP_HT					BIT22	

#define COMP_RESET				BIT23
#define COMP_CMDPKT			        BIT24
#define COMP_SCAN				BIT25
#define COMP_IPS				BIT26
#define COMP_DOWN				BIT27  
#define COMP_INTR 				BIT28  
#define COMP_ERR				BIT31  
#endif

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
                        printk(KERN_DEBUG RTL819xE_MODULE_NAME ": %s()\n", __FUNCTION__);   \
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

#define RTL819X_DEFAULT_RF_TYPE RF_1T2R
#define EEPROM_Default_LegacyHTTxPowerDiff	0x4
#define IEEE80211_WATCH_DOG_TIME    2000


typedef struct _tx_desc_819x_pci {
        
        u16	PktSize;
        u8	Offset;
        u8	Reserved1:3;
        u8	CmdInit:1;
        u8	LastSeg:1;
        u8	FirstSeg:1;
        u8	LINIP:1;
        u8	OWN:1;

        
        u8	TxFWInfoSize;
        u8	RATid:3;
        u8	DISFB:1;
        u8	USERATE:1;
        u8	MOREFRAG:1;
        u8	NoEnc:1;
        u8	PIFS:1;
        u8	QueueSelect:5;
        u8	NoACM:1;
        u8	Resv:2;
        u8	SecCAMID:5;
        u8	SecDescAssign:1;
        u8	SecType:2;

        
        u16	TxBufferSize;
        u8	PktId:7;
        u8	Resv1:1;
        u8	Reserved2;

        
	u32 	TxBuffAddr;

	
	u32	NextDescAddress;

	
        u32	Reserved5;
        u32	Reserved6;
        u32	Reserved7;
}tx_desc_819x_pci, *ptx_desc_819x_pci;


typedef struct _tx_desc_cmd_819x_pci {
        
	u16	PktSize;
	u8	Reserved1;
	u8	CmdType:3;
	u8	CmdInit:1;
	u8	LastSeg:1;
	u8	FirstSeg:1;
	u8	LINIP:1;
	u8	OWN:1;

        
	u16	ElementReport;
	u16	Reserved2;

        
	u16 	TxBufferSize;
	u16	Reserved3;

       
	u32	TxBufferAddr;
	u32	NextDescAddress;
	u32	Reserved4;
	u32	Reserved5;
	u32	Reserved6;
}tx_desc_cmd_819x_pci, *ptx_desc_cmd_819x_pci;


typedef struct _tx_fwinfo_819x_pci {
        
        u8		TxRate:7;
        u8		CtsEnable:1;
        u8		RtsRate:7;
        u8		RtsEnable:1;
        u8		TxHT:1;
        u8		Short:1;                
        u8		TxBandwidth:1;          
        u8		TxSubCarrier:2;         
        u8		STBC:2;
        u8		AllowAggregation:1;
        u8		RtsHT:1;                
        u8		RtsShort:1;             
        u8		RtsBandwidth:1;         
        u8		RtsSubcarrier:2;        
        u8		RtsSTBC:2;
        u8		EnableCPUDur:1;         

        
        u8		RxMF:2;
        u8		RxAMD:3;
        u8		Reserved1:3;
        u8		Reserved2;
        u8		Reserved3;
        u8		Reserved4;

        
}tx_fwinfo_819x_pci, *ptx_fwinfo_819x_pci;

typedef struct rtl8192_rx_info {
	struct urb *urb;
	struct net_device *dev;
	u8 out_pipe;
}rtl8192_rx_info ;
typedef struct _rx_desc_819x_pci{
	
	u16			Length:14;
	u16			CRC32:1;
	u16			ICV:1;
	u8			RxDrvInfoSize;
	u8			Shift:2;
	u8			PHYStatus:1;
	u8			SWDec:1;
	u8					LastSeg:1;
	u8					FirstSeg:1;
	u8					EOR:1;
	u8					OWN:1;

	
	u32			Reserved2;

	
	u32			Reserved3;

	
	u32	BufferAddress;

}rx_desc_819x_pci, *prx_desc_819x_pci;

typedef struct _rx_fwinfo_819x_pci{
	
	u16			Reserved1:12;
	u16			PartAggr:1;
	u16			FirstAGGR:1;
	u16			Reserved2:2;

	u8			RxRate:7;
	u8			RxHT:1;

	u8			BW:1;
	u8			SPLCP:1;
	u8			Reserved3:2;
	u8			PAM:1;
	u8			Mcast:1;
	u8			Bcast:1;
	u8			Reserved4:1;

	
	u32			TSFL;

}rx_fwinfo_819x_pci, *prx_fwinfo_819x_pci;

#define MAX_DEV_ADDR_SIZE		8  
#define MAX_FIRMWARE_INFORMATION_SIZE   32 
#define MAX_802_11_HEADER_LENGTH        (40 + MAX_FIRMWARE_INFORMATION_SIZE)
#define ENCRYPTION_MAX_OVERHEAD		128


#define MAX_FRAGMENT_COUNT		8
#define MAX_TRANSMIT_BUFFER_SIZE  	(1600+(MAX_802_11_HEADER_LENGTH+ENCRYPTION_MAX_OVERHEAD)*MAX_FRAGMENT_COUNT)

#define scrclng					4		

typedef enum _rtl819x_loopback{
	RTL819X_NO_LOOPBACK = 0,
	RTL819X_MAC_LOOPBACK = 1,
	RTL819X_DMA_LOOPBACK = 2,
	RTL819X_CCK_LOOPBACK = 3,
}rtl819x_loopback_e;


typedef enum _desc_packet_type_e{
	DESC_PACKET_TYPE_INIT = 0,
	DESC_PACKET_TYPE_NORMAL = 1,
}desc_packet_type_e;

typedef enum _firmware_source{
	FW_SOURCE_IMG_FILE = 0,
	FW_SOURCE_HEADER_FILE = 1,		
}firmware_source_e, *pfirmware_source_e;

typedef enum _firmware_status{
	FW_STATUS_0_INIT = 0,
	FW_STATUS_1_MOVE_BOOT_CODE = 1,
	FW_STATUS_2_MOVE_MAIN_CODE = 2,
	FW_STATUS_3_TURNON_CPU = 3,
	FW_STATUS_4_MOVE_DATA_CODE = 4,
	FW_STATUS_5_READY = 5,
}firmware_status_e;

typedef struct _rt_firmare_seg_container {
	u16	seg_size;
	u8	*seg_ptr;
}fw_seg_container, *pfw_seg_container;

typedef struct _rt_firmware{
	firmware_status_e firmware_status;
	u16		  cmdpacket_frag_thresold;
#define RTL8190_MAX_FIRMWARE_CODE_SIZE	64000	
#define MAX_FW_INIT_STEP		3
	u8		  firmware_buf[MAX_FW_INIT_STEP][RTL8190_MAX_FIRMWARE_CODE_SIZE];
	u16		  firmware_buf_size[MAX_FW_INIT_STEP];
}rt_firmware, *prt_firmware;

#define MAX_RECEIVE_BUFFER_SIZE	9100	


#define NUM_OF_FIRMWARE_QUEUE		10
#define NUM_OF_PAGES_IN_FW		0x100
#define NUM_OF_PAGE_IN_FW_QUEUE_BE	0x0aa
#define NUM_OF_PAGE_IN_FW_QUEUE_BK	0x007
#define NUM_OF_PAGE_IN_FW_QUEUE_VI	0x024
#define NUM_OF_PAGE_IN_FW_QUEUE_VO	0x007
#define NUM_OF_PAGE_IN_FW_QUEUE_HCCA	0
#define NUM_OF_PAGE_IN_FW_QUEUE_CMD	0x2
#define NUM_OF_PAGE_IN_FW_QUEUE_MGNT	0x10
#define NUM_OF_PAGE_IN_FW_QUEUE_HIGH	0
#define NUM_OF_PAGE_IN_FW_QUEUE_BCN	0x4
#define NUM_OF_PAGE_IN_FW_QUEUE_PUB	0xd
#define APPLIED_RESERVED_QUEUE_IN_FW	0x80000000
#define RSVD_FW_QUEUE_PAGE_BK_SHIFT	0x00
#define RSVD_FW_QUEUE_PAGE_BE_SHIFT	0x08
#define RSVD_FW_QUEUE_PAGE_VI_SHIFT	0x10
#define RSVD_FW_QUEUE_PAGE_VO_SHIFT	0x18
#define RSVD_FW_QUEUE_PAGE_MGNT_SHIFT	0x10
#define RSVD_FW_QUEUE_PAGE_CMD_SHIFT	0x08
#define RSVD_FW_QUEUE_PAGE_BCN_SHIFT	0x00
#define RSVD_FW_QUEUE_PAGE_PUB_SHIFT	0x08





#define DCAM                    0xAC                    
#define AESMSK_FC               0xB2    


#define CAM_CONTENT_COUNT       8

#define CFG_VALID               BIT15
#if 0



#define SCR_UseDK                       0x01
#define SCR_TxSecEnable                 0x02
#define SCR_RxSecEnable                 0x04




#define CAM_VALID                               0x8000
#define CAM_NOTVALID                    0x0000
#define CAM_USEDK                               0x0020


#define CAM_NONE                                0x0
#define CAM_WEP40                               0x01
#define CAM_TKIP                                0x02
#define CAM_AES                                 0x04
#define CAM_WEP104                              0x05


#define TOTAL_CAM_ENTRY         16
#define CAM_ENTRY_LEN_IN_DW     6       
#define CAM_ENTRY_LEN_IN_BYTE   (CAM_ENTRY_LEN_IN_DW*sizeof(u32))    

#define CAM_CONFIG_USEDK                1
#define CAM_CONFIG_NO_USEDK             0

#define CAM_WRITE                               0x00010000
#define CAM_READ                                0x00000000
#define CAM_POLLINIG                    0x80000000




#endif
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
	dma_addr_t dma;

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

#if 0

typedef struct tx_pendingbuf
{
	struct ieee80211_txb *txb;
	short ispending;
	short descfrag;
} tx_pendigbuf;

#endif

typedef struct _rt_9x_tx_rate_history {
	u32             cck[4];
	u32             ofdm[8];
	
	
	
	
	u32             ht_mcs[4][16];
}rt_tx_rahis_t, *prt_tx_rahis_t;

typedef	struct _RT_SMOOTH_DATA_4RF {
	char	elements[4][100];
	u32	index;			
	u32	TotalNum;		
	u32	TotalVal[4];		
}RT_SMOOTH_DATA_4RF, *PRT_SMOOTH_DATA_4RF;

typedef enum _tag_TxCmd_Config_Index{
	TXCMD_TXRA_HISTORY_CTRL				= 0xFF900000,
	TXCMD_RESET_TX_PKT_BUFF				= 0xFF900001,
	TXCMD_RESET_RX_PKT_BUFF				= 0xFF900002,
	TXCMD_SET_TX_DURATION				= 0xFF900003,
	TXCMD_SET_RX_RSSI						= 0xFF900004,
	TXCMD_SET_TX_PWR_TRACKING			= 0xFF900005,
	TXCMD_XXXX_CTRL,
}DCMD_TXCMD_OP;

typedef struct Stats
{
	unsigned long txrdu;
	unsigned long rxrdu;
	
	


	unsigned long rxok;
	unsigned long rxframgment;
	unsigned long rxcmdpkt[4];		
	unsigned long rxurberr;
	unsigned long rxstaterr;
	unsigned long rxcrcerrmin;
	unsigned long rxcrcerrmid;
	unsigned long rxcrcerrmax;
	unsigned long received_rate_histogram[4][32];	
	unsigned long received_preamble_GI[2][32];		
	unsigned long	rx_AMPDUsize_histogram[5]; 
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

	unsigned long rxoverflow;
	unsigned long rxint;
	unsigned long txnpokint;


	unsigned long ints;
	unsigned long shints;
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
	unsigned long txcmdpktokint;
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
	unsigned long rxbytesunicast;
	unsigned long txfeedbackfail;
	unsigned long txerrtotal;
	unsigned long txerrbytestotal;
	unsigned long txerrmulticast;
	unsigned long txerrbroadcast;
	unsigned long txerrunicast;
	unsigned long txretrycount;
	unsigned long txfeedbackretry;
	u8			last_packet_rate;
	unsigned long slide_signal_strength[100];
	unsigned long slide_evm[100];
	unsigned long	slide_rssi_total;	
	unsigned long slide_evm_total;	
	long signal_strength; 
	long signal_quality;
	long last_signal_strength_inpercent;
	long	recv_signal_power;	
	u8 rx_rssi_percentage[4];
	u8 rx_evm_percentage[2];
	long rxSNRdB[4];
	rt_tx_rahis_t txrate;
	u32 Slide_Beacon_pwdb[100];	
	u32 Slide_Beacon_Total;		
	RT_SMOOTH_DATA_4RF		cck_adc_pwdb;
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
}BB_REGISTER_DEFINITION_T, *PBB_REGISTER_DEFINITION_T;

typedef enum _RT_RF_TYPE_819xU{
        RF_TYPE_MIN = 0,
        RF_8225,
        RF_8256,
        RF_8258,
        RF_PSEUDO_11N = 4,
}RT_RF_TYPE_819xU, *PRT_RF_TYPE_819xU;


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


typedef enum tag_Rf_Operatetion_State
{
    RF_STEP_INIT = 0,
    RF_STEP_NORMAL,
    RF_STEP_MAX
}RF_STEP_E;

typedef enum _RT_STATUS{
	RT_STATUS_SUCCESS,
	RT_STATUS_FAILURE,
	RT_STATUS_PENDING,
	RT_STATUS_RESOURCE
}RT_STATUS,*PRT_STATUS;

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
	RT_CID_COREGA = 14,
}RT_CUSTOMER_ID, *PRT_CUSTOMER_ID;





typedef	enum _LED_STRATEGY_8190{
	SW_LED_MODE0, 
	SW_LED_MODE1, 
	SW_LED_MODE2, 
	SW_LED_MODE3, 
	SW_LED_MODE4, 
	SW_LED_MODE5, 
	SW_LED_MODE6, 
	HW_LED, 
}LED_STRATEGY_8190, *PLED_STRATEGY_8190;

#define CHANNEL_PLAN_LEN				10

#define sCrcLng 		4

typedef struct _TX_FWINFO_STRUCUTRE{
	
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
	u32			TxAGCOffset:4;
	u32			TxAGCSign:1;
	u32			Tx_INFO_RSVD:6;
	u32			PacketID:13;
}TX_FWINFO_T;


typedef struct _TX_FWINFO_8190PCI{
	
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
	u32			TxPerPktInfoFeedback:1; 	
	u32			Reserved1:2;
	u32			TxAGCOffset:4;		
	u32			TxAGCSign:1;		
	u32			RAW_TXD:1;			
	u32			Retry_Limit:4;		
	u32			Reserved2:1;
	u32			PacketID:13;

	

}TX_FWINFO_8190PCI, *PTX_FWINFO_8190PCI;

typedef struct _phy_ofdm_rx_status_report_819xpci
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
	u8	rxsc_sgien_exflg;
}phy_sts_ofdm_819xpci_t;

typedef struct _phy_cck_rx_status_report_819xpci
{
	
	u8	adc_pwdb_X[4];
	u8	sq_rpt;
	u8	cck_agc_rpt;
}phy_sts_cck_819xpci_t;

typedef struct _phy_ofdm_rx_status_rxsc_sgien_exintfflag{
	u8			reserved:4;
	u8			rxsc:2;
	u8			sgi_en:1;
	u8			ex_intf_flag:1;
}phy_ofdm_rx_status_rxsc_sgien_exintfflag;

typedef enum _RT_OP_MODE{
	RT_OP_MODE_AP,
	RT_OP_MODE_INFRASTRUCTURE,
	RT_OP_MODE_IBSS,
	RT_OP_MODE_NO_LINK,
}RT_OP_MODE, *PRT_OP_MODE;



typedef enum tag_Rf_OpType
{
    RF_OP_By_SW_3wire = 0,
    RF_OP_By_FW,
    RF_OP_MAX
}RF_OpType_E;

typedef enum _RESET_TYPE {
	RESET_TYPE_NORESET = 0x00,
	RESET_TYPE_NORMAL = 0x01,
	RESET_TYPE_SILENT = 0x02
} RESET_TYPE;

typedef struct _tx_ring{
	u32 * desc;
	u8 nStuckCount;
	struct _tx_ring * next;
}__attribute__ ((packed)) tx_ring, * ptx_ring;

struct rtl8192_tx_ring {
    tx_desc_819x_pci *desc;
    dma_addr_t dma;
    unsigned int idx;
    unsigned int entries;
    struct sk_buff_head queue;
};

#define NIC_SEND_HANG_THRESHOLD_NORMAL		4
#define NIC_SEND_HANG_THRESHOLD_POWERSAVE 	8
#define MAX_TX_QUEUE				9	

#define MAX_RX_COUNT                            64
#define MAX_TX_QUEUE_COUNT                      9

typedef struct r8192_priv
{
	struct pci_dev *pdev;
	
	short epromtype;
	u16 eeprom_vid;
	u16 eeprom_did;
	u8  eeprom_CustomerID;
	u16  eeprom_ChannelPlan;
	RT_CUSTOMER_ID CustomerID;
	LED_STRATEGY_8190	LedStrategy;
	
	u8	IC_Cut;
	int irq;
	short irq_enabled;
	struct ieee80211_device *ieee80211;
	bool being_init_adapter;
	u8 Rf_Mode;
	short card_8192; 
	u8 card_8192_version; 

	short enable_gpio0;
	enum card_type {PCI,MINIPCI,CARDBUS,USB}card_type;
	short hw_plcp_len;
	short plcp_preamble_mode;
	u8 ScanDelay;
	spinlock_t irq_lock;
	spinlock_t irq_th_lock;
	spinlock_t tx_lock;
	spinlock_t rf_ps_lock;
        struct mutex mutex;
	spinlock_t rf_lock; 
	spinlock_t ps_lock;

	u32 irq_mask;


	short chan;
	short sens;
	short max_sens;
	u32 rx_prevlen;

        rx_desc_819x_pci *rx_ring;
        dma_addr_t rx_ring_dma;
        unsigned int rx_idx;
        struct sk_buff *rx_buf[MAX_RX_COUNT];
	int rxringcount;
	u16 rxbuffersize;


	struct sk_buff *rx_skb;
	u32 *rxring;
	u32 *rxringtail;
	dma_addr_t rxringdma;
	struct buffer *rxbuffer;
	struct buffer *rxbufferhead;
	short rx_skb_complete;

        struct rtl8192_tx_ring tx_ring[MAX_TX_QUEUE_COUNT];
	int txringcount;

	int txbuffsize;
	int txfwbuffersize;
	
	
	struct tasklet_struct irq_rx_tasklet;
	struct tasklet_struct irq_tx_tasklet;
        struct tasklet_struct irq_prepare_beacon_tasklet;
	struct buffer *txmapbufs;
	struct buffer *txbkpbufs;
	struct buffer *txbepbufs;
	struct buffer *txvipbufs;
	struct buffer *txvopbufs;
	struct buffer *txcmdbufs;
	struct buffer *txmapbufstail;
	struct buffer *txbkpbufstail;
	struct buffer *txbepbufstail;
	struct buffer *txvipbufstail;
	struct buffer *txvopbufstail;
	struct buffer *txcmdbufstail;
	
	ptx_ring txbeaconringtail;
	dma_addr_t txbeaconringdma;
	ptx_ring txbeaconring;
	int txbeaconcount;
	struct buffer *txbeaconbufs;
	struct buffer *txbeaconbufstail;
	ptx_ring txmapring;
	ptx_ring txbkpring;
	ptx_ring txbepring;
	ptx_ring txvipring;
	ptx_ring txvopring;
	ptx_ring txcmdring;
	ptx_ring txmapringtail;
	ptx_ring txbkpringtail;
	ptx_ring txbepringtail;
	ptx_ring txvipringtail;
	ptx_ring txvopringtail;
	ptx_ring txcmdringtail;
	ptx_ring txmapringhead;
	ptx_ring txbkpringhead;
	ptx_ring txbepringhead;
	ptx_ring txvipringhead;
	ptx_ring txvopringhead;
	ptx_ring txcmdringhead;
	dma_addr_t txmapringdma;
	dma_addr_t txbkpringdma;
	dma_addr_t txbepringdma;
	dma_addr_t txvipringdma;
	dma_addr_t txvopringdma;
	dma_addr_t txcmdringdma;
	




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

	




#ifdef THOMAS_BEACON
	u32 *oldaddr;
#endif
#ifdef THOMAS_TASKLET
	atomic_t irt_counter;
#endif
#ifdef JACKSON_NEW_RX
        struct sk_buff **pp_rxskb;
        int     rx_inx;
#endif


       struct sk_buff_head rx_queue;
       struct sk_buff_head skb_queue;
       struct work_struct qos_activate;
	short  tx_urb_index;
	atomic_t tx_pending[0x10];

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



	
	u16	basic_rate;
	u8	short_preamble;
	u8 	slot_time;
	u16 SifsTime;

	u8 RegWirelessMode;

	prt_firmware		pFirmware;
	rtl819x_loopback_e	LoopbackMode;
	firmware_source_e	firmware_source;
	bool AutoloadFailFlag;
	u16 EEPROMTxPowerDiff;
	u16 EEPROMAntPwDiff;		
	u8 EEPROMThermalMeter;
	u8 EEPROMPwDiff;
	u8 EEPROMCrystalCap;
	u8 EEPROM_Def_Ver;
	u8 EEPROMTxPowerLevelCCK[14];
	
	u8 EEPROMRfACCKChnl1TxPwLevel[3];	
	u8 EEPROMRfAOfdmChnlTxPwLevel[3];
	u8 EEPROMRfCCCKChnl1TxPwLevel[3];	
	u8 EEPROMRfCOfdmChnlTxPwLevel[3];
	u8 EEPROMTxPowerLevelCCK_V1[3];
	u8 EEPROMTxPowerLevelOFDM24G[14]; 
	u8 EEPROMTxPowerLevelOFDM5G[24];	
	u8 EEPROMLegacyHTTxPowerDiff;	
	bool bTXPowerDataReadFromEEPORM;

	u16 RegChannelPlan; 
	u16 ChannelPlan;

	bool RegRfOff;
	
	u8	bHwRfOffAction;	

	BB_REGISTER_DEFINITION_T	PHYRegDef[4];	
	
	u32	MCSTxPowerLevelOriginalOffset[6];
	u32	CCKTxPowerLevelOriginalOffset;
	u8	TxPowerLevelCCK[14];			
	u8	TxPowerLevelCCK_A[14];			
	u8 	TxPowerLevelCCK_C[14];
	u8	TxPowerLevelOFDM24G[14];		
	u8	TxPowerLevelOFDM5G[14];			
	u8	TxPowerLevelOFDM24G_A[14];	
	u8	TxPowerLevelOFDM24G_C[14];	
	u8	LegacyHTTxPowerDiff;			
	u8	TxPowerDiff;
	char	RF_C_TxPwDiff;					
	u8	AntennaTxPwDiff[3];				
	u8	CrystalCap;						
	u8	ThermalMeter[2];				
	
	u8	CckPwEnl;
	u16	TSSI_13dBm;
	u32 	Pwr_Track;
	u8				CCKPresentAttentuation_20Mdefault;
	u8				CCKPresentAttentuation_40Mdefault;
	char				CCKPresentAttentuation_difference;
	char				CCKPresentAttentuation;
	
	u8	bCckHighPower;
	long	undecorated_smoothed_pwdb;
	long	undecorated_smoothed_cck_adc_pwdb[4];
	
	u8	SwChnlInProgress;
	u8 	SwChnlStage;
	u8	SwChnlStep;
	u8	SetBWModeInProgress;
	HT_CHANNEL_WIDTH		CurrentChannelBW;

	
	
	u8	nCur40MhzPrimeSC;	
	
	
	
	u32					RfReg0Value[4];
	u8 					NumTotalRFPath;
	bool 				brfpath_rxenable[4];

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
	u8			   txpower_count;
	bool			   btxpower_trackingInit;
	u8			   OFDM_index;
	u8			   CCK_index;
	u8			   Record_CCK_20Mindex;
	u8			   Record_CCK_40Mindex;
	
	ccktxbbgain_struct	cck_txbbgain_table[CCKTxBBGainTableLength];
	ccktxbbgain_struct	cck_txbbgain_ch14_table[CCKTxBBGainTableLength];
	u8 rfa_txpowertrackingindex;
	u8 rfa_txpowertrackingindex_real;
	u8 rfa_txpowertracking_default;
	u8 rfc_txpowertrackingindex;
	u8 rfc_txpowertrackingindex_real;
	u8 rfc_txpowertracking_default;
	bool btxpower_tracking;
	bool bcck_in_ch14;

	
	init_gain initgain_backup;
	u8 		DefaultInitialGain[4];
	
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

	
	u32 last_rxdesc_tsf_high;
	u32 last_rxdesc_tsf_low;

	
	bool bHwRadioOff;
	
	bool RFChangeInProgress; 
	bool SetRFPowerStateInProgress;
	RT_OP_MODE OpMode;
	
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

	
	struct delayed_work update_beacon_wq;
	struct delayed_work watch_dog_wq;
	struct delayed_work txpower_tracking_wq;
	struct delayed_work rfpath_check_wq;
	struct delayed_work gpio_change_rf_wq;
	struct delayed_work initialgain_operate_wq;
	struct workqueue_struct *priv_wq;
}r8192_priv;





#if 0
typedef enum{
	BULK_PRIORITY = 0x01,
	
	
	LOW_PRIORITY,
	NORM_PRIORITY,
	VO_PRIORITY,
	VI_PRIORITY, 
	BE_PRIORITY,
	BK_PRIORITY,
	CMD_PRIORITY,
	RSVD3,
	BEACON_PRIORITY, 
	HIGH_PRIORITY,
	MANAGE_PRIORITY,
	RSVD4,
	RSVD5,
	UART_PRIORITY 
} priority_t;
#endif
typedef enum{
	NIC_8192E = 1,
	} nic_t;


#if 0 

#define AC0_BE	0		
#define AC1_BK	1		
#define AC2_VI	2		
#define AC3_VO	3		
#define AC_MAX	4		





typedef	union _ECW{
	u8	charData;
	struct
	{
		u8	ECWmin:4;
		u8	ECWmax:4;
	}f;	
}ECW, *PECW;





typedef	union _ACI_AIFSN{
	u8	charData;

	struct
	{
		u8	AIFSN:4;
		u8	ACM:1;
		u8	ACI:2;
		u8	Reserved:1;
	}f;	
}ACI_AIFSN, *PACI_AIFSN;





typedef	union _AC_PARAM{
	u32	longData;
	u8	charData[4];

	struct
	{
		ACI_AIFSN	AciAifsn;
		ECW		Ecw;
		u16		TXOPLimit;
	}f;	
}AC_PARAM, *PAC_PARAM;

#endif
bool init_firmware(struct net_device *dev);
void rtl819xE_tx_cmd(struct net_device *dev, struct sk_buff *skb);
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
void rtl8187_set_rxconf(struct net_device *dev);

void rtl8192_start_beacon(struct net_device *dev);
void CamResetAllEntry(struct net_device* dev);
void EnableHWSecurityConfig8192(struct net_device *dev);
void setKey(struct net_device *dev, u8 EntryNo, u8 KeyIndex, u16 KeyType, u8 *MacAddr, u8 DefaultKey, u32 *KeyContent );
void CamPrintDbgReg(struct net_device* dev);
extern	void	dm_cck_txpower_adjust(struct net_device *dev,bool  binch14);
extern void firmware_init_param(struct net_device *dev);
extern RT_STATUS cmpk_message_handle_tx(struct net_device *dev, u8* codevirtualaddress, u32 packettype, u32 buffer_len);
void rtl8192_hw_wakeup_wq (struct work_struct *work);

short rtl8192_is_tx_queue_empty(struct net_device *dev);
#ifdef ENABLE_IPS
void IPSEnter(struct net_device *dev);
void IPSLeave(struct net_device *dev);
#endif
#endif
