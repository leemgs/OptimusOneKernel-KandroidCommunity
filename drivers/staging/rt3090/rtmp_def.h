
#ifndef __RTMP_DEF_H__
#define __RTMP_DEF_H__

#include "oid.h"

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED


#ifdef CONFIG_STA_SUPPORT
#endif 

#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif



#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define RT_DEBUG_INFO       4
#define RT_DEBUG_LOUD       5

#define NIC_TAG             ((ULONG)'0682')
#define NIC_DBG_STRING      ("**RT28xx**")

#ifdef SNMP_SUPPORT


#define ManufacturerOUI_LEN			3
#define ManufacturerNAME			("Ralink Technology Company.")
#define	ResourceTypeIdName			("Ralink_ID")
#endif




#define RALINK_2883_VERSION		((UINT32)0x28830300)
#define RALINK_2880E_VERSION	((UINT32)0x28720200)
#define RALINK_3070_VERSION		((UINT32)0x30700200)

#define MAX_RX_PKT_LEN	1520





#ifdef RTMP_MAC_PCI
#define TX_RING_SIZE            64 
#define MGMT_RING_SIZE          128
#define RX_RING_SIZE            128 
#define MAX_TX_PROCESS          TX_RING_SIZE 
#define MAX_DMA_DONE_PROCESS    TX_RING_SIZE
#define MAX_TX_DONE_PROCESS     TX_RING_SIZE 
#define LOCAL_TXBUF_SIZE        2
#endif 

#define PCI_VIRT_TO_PHYS(__Addr)	(((UINT32)(__Addr)) & 0x0FFFFFFF)


#ifdef MULTIPLE_CARD_SUPPORT

#define MAX_NUM_OF_MULTIPLE_CARD		32
#endif 

#define MAX_RX_PROCESS          128 
#define NUM_OF_LOCAL_TXBUF      2
#define TXD_SIZE                16
#define TXWI_SIZE               16
#define RXD_SIZE		16
#define RXWI_SIZE		16

#define TX_DMA_1ST_BUFFER_SIZE  96    
#define MGMT_DMA_BUFFER_SIZE    1536 
#define RX_BUFFER_AGGRESIZE     3840 
#define RX_BUFFER_NORMSIZE      3840 
#define TX_BUFFER_NORMSIZE		RX_BUFFER_NORMSIZE
#define MAX_FRAME_SIZE          2346                    
#define MAX_AGGREGATION_SIZE    3840 
#define MAX_NUM_OF_TUPLE_CACHE  2
#define MAX_MCAST_LIST_SIZE     32
#define MAX_LEN_OF_VENDOR_DESC  64

#define MAX_SIZE_OF_MCAST_PSQ               32

#define MAX_RX_PROCESS_CNT	(RX_RING_SIZE)






#define MAX_PACKETS_IN_QUEUE				(512) 

#define MAX_PACKETS_IN_MCAST_PS_QUEUE		32
#define MAX_PACKETS_IN_PS_QUEUE				128	
#define WMM_NUM_OF_AC                       4  


#ifdef RTMP_EFUSE_SUPPORT

#define MAX_EEPROM_BIN_FILE_SIZE				1024
#define EFUSE_BUFFER_PATH						"/tmp/RT30xxEEPROM.bin"

#endif 


#define STANORMAL	 0x17f97
#define APNORMAL	 0x15f97
#define PSPXLINK	 0x17f93



#define fRTMP_ADAPTER_MAP_REGISTER          0x00000001
#define fRTMP_ADAPTER_INTERRUPT_IN_USE      0x00000002
#define fRTMP_ADAPTER_HARDWARE_ERROR        0x00000004
#define fRTMP_ADAPTER_SCATTER_GATHER        0x00000008
#define fRTMP_ADAPTER_SEND_PACKET_ERROR     0x00000010
#define fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS 0x00000020
#define fRTMP_ADAPTER_HALT_IN_PROGRESS      0x00000040
#define fRTMP_ADAPTER_RESET_IN_PROGRESS     0x00000080
#define fRTMP_ADAPTER_NIC_NOT_EXIST         0x00000100
#define fRTMP_ADAPTER_TX_RING_ALLOCATED     0x00000200
#define fRTMP_ADAPTER_REMOVE_IN_PROGRESS    0x00000400
#define fRTMP_ADAPTER_MIMORATE_INUSED       0x00000800
#define fRTMP_ADAPTER_RX_RING_ALLOCATED     0x00001000
#define fRTMP_ADAPTER_INTERRUPT_ACTIVE      0x00002000
#define fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS  0x00004000
#define	fRTMP_ADAPTER_REASSOC_IN_PROGRESS	0x00008000
#define	fRTMP_ADAPTER_MEDIA_STATE_PENDING	0x00010000
#define	fRTMP_ADAPTER_RADIO_OFF				0x00020000
#define fRTMP_ADAPTER_BULKOUT_RESET			0x00040000
#define	fRTMP_ADAPTER_BULKIN_RESET			0x00080000
#define fRTMP_ADAPTER_RDG_ACTIVE			0x00100000
#define fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE 0x00200000
#define fRTMP_ADAPTER_SCAN_2040				0x04000000
#define	fRTMP_ADAPTER_RADIO_MEASUREMENT		0x08000000

#define fRTMP_ADAPTER_START_UP			0x10000000	
#define fRTMP_ADAPTER_MEDIA_STATE_CHANGE    0x20000000
#define fRTMP_ADAPTER_IDLE_RADIO_OFF        0x40000000














#define fOP_STATUS_INFRA_ON                 0x00000001
#define fOP_STATUS_ADHOC_ON                 0x00000002
#define fOP_STATUS_BG_PROTECTION_INUSED     0x00000004
#define fOP_STATUS_SHORT_SLOT_INUSED        0x00000008
#define fOP_STATUS_SHORT_PREAMBLE_INUSED    0x00000010
#define fOP_STATUS_RECEIVE_DTIM             0x00000020

#define fOP_STATUS_MEDIA_STATE_CONNECTED    0x00000080
#define fOP_STATUS_WMM_INUSED               0x00000100
#define fOP_STATUS_AGGREGATION_INUSED       0x00000200
#define fOP_STATUS_DOZE                     0x00000400  
#define fOP_STATUS_PIGGYBACK_INUSED         0x00000800  
#define fOP_STATUS_APSD_INUSED				0x00001000
#define fOP_STATUS_TX_AMSDU_INUSED			0x00002000
#define fOP_STATUS_MAX_RETRY_ENABLED		0x00004000
#define fOP_STATUS_WAKEUP_NOW               0x00008000
#define fOP_STATUS_PCIE_DEVICE       0x00020000





#define fRTMP_PS_CAN_GO_SLEEP          0x00000001

#define fRTMP_PS_SET_PCI_CLK_OFF_COMMAND          0x00000002

#define fRTMP_PS_DISABLE_TX         0x00000004


#define fRTMP_PS_GO_TO_SLEEP_NOW         0x00000008
#define fRTMP_PS_TOGGLE_L1		0x00000010	
#ifdef RT3090
#define WAKE_MCU_CMD				0x31
#define SLEEP_MCU_CMD					0x30
#define RFOFF_MCU_CMD				0x35
#endif 
#ifdef DOT11N_DRAFT3
#define fOP_STATUS_SCAN_2040			    0x00040000
#endif 

#define CCKSETPROTECT		0x1
#define OFDMSETPROTECT		0x2
#define MM20SETPROTECT		0x4
#define MM40SETPROTECT		0x8
#define GF20SETPROTECT		0x10
#define GR40SETPROTECT		0x20
#define ALLN_SETPROTECT		(GR40SETPROTECT | GF20SETPROTECT | MM40SETPROTECT | MM20SETPROTECT)




#define fCLIENT_STATUS_WMM_CAPABLE          0x00000001  
#define fCLIENT_STATUS_AGGREGATION_CAPABLE  0x00000002  
#define fCLIENT_STATUS_PIGGYBACK_CAPABLE    0x00000004  
#define fCLIENT_STATUS_AMSDU_INUSED			0x00000008
#define fCLIENT_STATUS_SGI20_CAPABLE		0x00000010
#define fCLIENT_STATUS_SGI40_CAPABLE		0x00000020
#define fCLIENT_STATUS_TxSTBC_CAPABLE		0x00000040
#define fCLIENT_STATUS_RxSTBC_CAPABLE		0x00000080
#define fCLIENT_STATUS_HTC_CAPABLE			0x00000100
#define fCLIENT_STATUS_RDG_CAPABLE			0x00000200
#define fCLIENT_STATUS_MCSFEEDBACK_CAPABLE  0x00000400
#define fCLIENT_STATUS_APSD_CAPABLE         0x00000800  

#ifdef DOT11N_DRAFT3
#define fCLIENT_STATUS_BSSCOEXIST_CAPABLE	0x00001000
#endif 

#define fCLIENT_STATUS_RALINK_CHIPSET		0x00100000






#define HT_NO_PROTECT	0
#define HT_LEGACY_PROTECT	1
#define HT_40_PROTECT	2
#define HT_2040_PROTECT	3
#define HT_RTSCTS_6M	7

#define HT_ATHEROS	8	
#define HT_FORCERTSCTS	9	




#define fRX_FILTER_ACCEPT_DIRECT            NDIS_PACKET_TYPE_DIRECTED
#define fRX_FILTER_ACCEPT_MULTICAST         NDIS_PACKET_TYPE_MULTICAST
#define fRX_FILTER_ACCEPT_BROADCAST         NDIS_PACKET_TYPE_BROADCAST
#define fRX_FILTER_ACCEPT_ALL_MULTICAST     NDIS_PACKET_TYPE_ALL_MULTICAST
#define fRX_FILTER_ACCEPT_PROMISCUOUS       NDIS_PACKET_TYPE_PROMISCUOUS





#define ERRLOG_READ_PCI_SLOT_FAILED     0x00000101L
#define ERRLOG_WRITE_PCI_SLOT_FAILED    0x00000102L
#define ERRLOG_VENDOR_DEVICE_NOMATCH    0x00000103L


#define ERRLOG_BUS_MASTER_DISABLED      0x00000201L


#define ERRLOG_INVALID_SPEED_DUPLEX     0x00000301L
#define ERRLOG_SET_SECONDARY_FAILED     0x00000302L


#define ERRLOG_OUT_OF_MEMORY            0x00000401L
#define ERRLOG_OUT_OF_SHARED_MEMORY     0x00000402L
#define ERRLOG_OUT_OF_MAP_REGISTERS     0x00000403L
#define ERRLOG_OUT_OF_BUFFER_POOL       0x00000404L
#define ERRLOG_OUT_OF_NDIS_BUFFER       0x00000405L
#define ERRLOG_OUT_OF_PACKET_POOL       0x00000406L
#define ERRLOG_OUT_OF_NDIS_PACKET       0x00000407L
#define ERRLOG_OUT_OF_LOOKASIDE_MEMORY  0x00000408L


#define ERRLOG_SELFTEST_FAILED          0x00000501L
#define ERRLOG_INITIALIZE_ADAPTER       0x00000502L
#define ERRLOG_REMOVE_MINIPORT          0x00000503L


#define ERRLOG_MAP_IO_SPACE             0x00000601L
#define ERRLOG_QUERY_ADAPTER_RESOURCES  0x00000602L
#define ERRLOG_NO_IO_RESOURCE           0x00000603L
#define ERRLOG_NO_INTERRUPT_RESOURCE    0x00000604L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L



#define	MAX_WDS_ENTRY               4
#define WDS_PAIRWISE_KEY_OFFSET     60    

#define	WDS_DISABLE_MODE            0
#define	WDS_RESTRICT_MODE           1
#define	WDS_BRIDGE_MODE             2
#define	WDS_REPEATER_MODE           3
#define	WDS_LAZY_MODE               4


#define MAX_MESH_NUM				0

#define MAX_APCLI_NUM				0
#ifdef APCLI_SUPPORT
#undef	MAX_APCLI_NUM
#define MAX_APCLI_NUM				1
#endif 

#define MAX_MBSSID_NUM				1
#ifdef MBSS_SUPPORT
#undef	MAX_MBSSID_NUM
#define MAX_MBSSID_NUM				(8 - MAX_MESH_NUM - MAX_APCLI_NUM)
#endif 


#define MBSS_MR_APIDX_SANITY_CHECK(apidx) \
    { if (apidx > MAX_MBSSID_NUM) { \
          DBGPRINT(RT_DEBUG_ERROR, ("%s> Error! apidx = %d > MAX_MBSSID_NUM!\n", __FUNCTION__, apidx)); \
	  apidx = MAIN_MBSSID; } }

#define VALID_WCID(_wcid)	((_wcid) > 0 && (_wcid) < MAX_LEN_OF_MAC_TABLE )

#define MAIN_MBSSID                 0
#define FIRST_MBSSID                1


#define MAX_BEACON_SIZE				512




#if defined(MAX_MBSSID_NUM) && (MAX_MBSSID_NUM == 8)
#define HW_RESERVED_WCID	222
#elif defined(MAX_MBSSID_NUM) && (MAX_MBSSID_NUM == 7)
#define HW_RESERVED_WCID	238
#else
#define HW_RESERVED_WCID	255
#endif


#define DFS_CTS_WCID		(HW_RESERVED_WCID - 1)
#define CS_CTS_WCID		(HW_RESERVED_WCID - 2)
#define LAST_SPECIFIC_WCID	(HW_RESERVED_WCID - 2)



#define MAX_AVAILABLE_CLIENT_WCID	(LAST_SPECIFIC_WCID - MAX_MBSSID_NUM - 1)



#define GET_GroupKey_WCID(__wcid, __bssidx) \
	{										\
		__wcid = LAST_SPECIFIC_WCID - (MAX_MBSSID_NUM) + __bssidx;	\
	}

#define IsGroupKeyWCID(__wcid) (((__wcid) < LAST_SPECIFIC_WCID) && ((__wcid) >= (LAST_SPECIFIC_WCID - (MAX_MBSSID_NUM))))



#define BSS0                            0
#define BSS1                            1
#define BSS2                            2
#define BSS3                            3
#define BSS4                            4
#define BSS5                            5
#define BSS6                            6
#define BSS7                            7




#define PEER_KEY_NO                     2
#define MAC_ADDR_LEN                    6
#define TIMESTAMP_LEN                   8
#define MAX_LEN_OF_SUPPORTED_RATES      MAX_LENGTH_OF_SUPPORT_RATES 
#define MAX_NUM_OF_REGULATORY_CLASS		16
#define MAX_LEN_OF_KEY                  32      
#define MAX_NUM_OF_CHANNELS             MAX_NUM_OF_CHS      
#define MAX_NUM_OF_11JCHANNELS             20      
#define MAX_LEN_OF_SSID                 32
#define CIPHER_TEXT_LEN                 128
#define HASH_TABLE_SIZE                 256
#define MAX_VIE_LEN                     1024   
#define MAX_SUPPORT_MCS             32
#define MAX_NUM_OF_BBP_LATCH             140



#define BSSID_WCID		1	
#define MCAST_WCID	0x0
#define BSS0Mcast_WCID	0x0
#define BSS1Mcast_WCID	0xf8
#define BSS2Mcast_WCID	0xf9
#define BSS3Mcast_WCID	0xfa
#define BSS4Mcast_WCID	0xfb
#define BSS5Mcast_WCID	0xfc
#define BSS6Mcast_WCID	0xfd
#define BSS7Mcast_WCID	0xfe
#define RESERVED_WCID		0xff

#define MAX_NUM_OF_ACL_LIST				MAX_NUMBER_OF_ACL

#define MAX_LEN_OF_MAC_TABLE            MAX_NUMBER_OF_MAC 

#if MAX_LEN_OF_MAC_TABLE>MAX_AVAILABLE_CLIENT_WCID
#error MAX_LEN_OF_MAC_TABLE can not be larger than MAX_AVAILABLE_CLIENT_WCID!!!!
#endif

#define MAX_NUM_OF_WDS_LINK_PERBSSID	            3
#define MAX_NUM_OF_WDS_LINK	            (MAX_NUM_OF_WDS_LINK_PERBSSID*MAX_MBSSID_NUM)
#define MAX_NUM_OF_EVENT                MAX_NUMBER_OF_EVENT
#define WDS_LINK_START_WCID				(MAX_LEN_OF_MAC_TABLE-1)

#define NUM_OF_TID			8
#define MAX_AID_BA                    4
#define MAX_LEN_OF_BA_REC_TABLE          ((NUM_OF_TID * MAX_LEN_OF_MAC_TABLE)/2)
#define MAX_LEN_OF_BA_ORI_TABLE          ((NUM_OF_TID * MAX_LEN_OF_MAC_TABLE)/2)
#define MAX_LEN_OF_BSS_TABLE             64
#define MAX_REORDERING_MPDU_NUM			 512


#define SHARE_KEY_NUM                   4
#define MAX_LEN_OF_SHARE_KEY            16    
#define MAX_LEN_OF_PEER_KEY             16    
#define PAIRWISE_KEY_NUM                64    
#define GROUP_KEY_NUM                   4
#define PMK_LEN                         32
#define WDS_PAIRWISE_KEY_OFFSET         60    
#define	PMKID_NO                        4     
#define MAX_LEN_OF_MLME_BUFFER          2048


#define PWR_ACTIVE                      0
#define PWR_SAVE                        1
#define PWR_MMPS                        2			



#define AUTH_MODE_OPEN                  0x00
#define AUTH_MODE_KEY                   0x01





#define BSS_ADHOC                       0  
#define BSS_INFRA                       1  
#define BSS_ANY                         2  
#define BSS_MONITOR			            3  



#define REASON_RESERVED                 0
#define REASON_UNSPECIFY                1
#define REASON_NO_LONGER_VALID          2
#define REASON_DEAUTH_STA_LEAVING       3
#define REASON_DISASSOC_INACTIVE        4
#define REASON_DISASSPC_AP_UNABLE       5
#define REASON_CLS2ERR                  6
#define REASON_CLS3ERR                  7
#define REASON_DISASSOC_STA_LEAVING     8
#define REASON_STA_REQ_ASSOC_NOT_AUTH   9
#define REASON_INVALID_IE               13
#define REASON_MIC_FAILURE              14
#define REASON_4_WAY_TIMEOUT            15
#define REASON_GROUP_KEY_HS_TIMEOUT     16
#define REASON_IE_DIFFERENT             17
#define REASON_MCIPHER_NOT_VALID        18
#define REASON_UCIPHER_NOT_VALID        19
#define REASON_AKMP_NOT_VALID           20
#define REASON_UNSUPPORT_RSNE_VER       21
#define REASON_INVALID_RSNE_CAP         22
#define REASON_8021X_AUTH_FAIL          23
#define REASON_CIPHER_SUITE_REJECTED    24
#define REASON_DECLINED                 37

#define REASON_QOS_UNSPECIFY              32
#define REASON_QOS_LACK_BANDWIDTH         33
#define REASON_POOR_CHANNEL_CONDITION     34
#define REASON_QOS_OUTSIDE_TXOP_LIMITION  35
#define REASON_QOS_QSTA_LEAVING_QBSS      36
#define REASON_QOS_UNWANTED_MECHANISM     37
#define REASON_QOS_MECH_SETUP_REQUIRED    38
#define REASON_QOS_REQUEST_TIMEOUT        39
#define REASON_QOS_CIPHER_NOT_SUPPORT     45


#define MLME_SUCCESS                    0
#define MLME_UNSPECIFY_FAIL             1
#define MLME_CANNOT_SUPPORT_CAP         10
#define MLME_REASSOC_DENY_ASSOC_EXIST   11
#define MLME_ASSOC_DENY_OUT_SCOPE       12
#define MLME_ALG_NOT_SUPPORT            13
#define MLME_SEQ_NR_OUT_OF_SEQUENCE     14
#define MLME_REJ_CHALLENGE_FAILURE      15
#define MLME_REJ_TIMEOUT                  16
#define MLME_ASSOC_REJ_UNABLE_HANDLE_STA  17
#define MLME_ASSOC_REJ_DATA_RATE          18

#define MLME_ASSOC_REJ_NO_EXT_RATE        22
#define MLME_ASSOC_REJ_NO_EXT_RATE_PBCC   23
#define MLME_ASSOC_REJ_NO_CCK_OFDM        24

#define MLME_QOS_UNSPECIFY                32
#define MLME_REQUEST_DECLINED             37
#define MLME_REQUEST_WITH_INVALID_PARAM   38
#define MLME_INVALID_GROUP_CIPHER	  41
#define MLME_INVALID_PAIRWISE_CIPHER	  42
#define MLME_INVALID_AKMP			  43
#define MLME_DLS_NOT_ALLOW_IN_QBSS        48
#define MLME_DEST_STA_NOT_IN_QBSS         49
#define MLME_DEST_STA_IS_NOT_A_QSTA       50

#define MLME_INVALID_FORMAT             0x51
#define MLME_FAIL_NO_RESOURCE           0x52
#define MLME_STATE_MACHINE_REJECT       0x53
#define MLME_MAC_TABLE_FAIL             0x54


#define IE_SSID                         0
#define IE_SUPP_RATES                   1
#define IE_FH_PARM                      2
#define IE_DS_PARM                      3
#define IE_CF_PARM                      4
#define IE_TIM                          5
#define IE_IBSS_PARM                    6
#define IE_COUNTRY                      7     
#define IE_802_11D_REQUEST              10    
#define IE_QBSS_LOAD                    11    
#define IE_EDCA_PARAMETER               12    
#define IE_TSPEC                        13    
#define IE_TCLAS                        14    
#define IE_SCHEDULE                     15    
#define IE_CHALLENGE_TEXT               16
#define IE_POWER_CONSTRAINT             32    
#define IE_POWER_CAPABILITY             33    
#define IE_TPC_REQUEST                  34    
#define IE_TPC_REPORT                   35    
#define IE_SUPP_CHANNELS                36    
#define IE_CHANNEL_SWITCH_ANNOUNCEMENT  37    
#define IE_MEASUREMENT_REQUEST          38    
#define IE_MEASUREMENT_REPORT           39    
#define IE_QUIET                        40    
#define IE_IBSS_DFS                     41    
#define IE_ERP                          42    
#define IE_TS_DELAY                     43    
#define IE_TCLAS_PROCESSING             44    
#define IE_QOS_CAPABILITY               46    
#define IE_HT_CAP                       45    
#define IE_AP_CHANNEL_REPORT			51    
#define IE_HT_CAP2                         52    
#define IE_RSN                          48    
#define IE_WPA2                         48    
#define IE_EXT_SUPP_RATES               50    
#define IE_SUPP_REG_CLASS               59    
#define IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT	60	
#define IE_ADD_HT                         61    
#define IE_ADD_HT2                        53    




#define IE_SECONDARY_CH_OFFSET		62	
#define IE_WAPI							68		
#define IE_2040_BSS_COEXIST               72    
#define IE_2040_BSS_INTOLERANT_REPORT     73    
#define IE_OVERLAPBSS_SCAN_PARM           74    
#define IE_EXT_CAPABILITY                127   


#define IE_WPA                          221   
#define IE_VENDOR_SPECIFIC              221   

#define OUI_BROADCOM_HT              51   
#define OUI_BROADCOM_HTADD              52   
#define OUI_PREN_HT_CAP              51   
#define OUI_PREN_ADD_HT              52   


#define IE_AIRONET_CKIP                 133   
#define IE_AP_TX_POWER                  150   
#define IE_MEASUREMENT_CAPABILITY       221   
#define IE_CCX_V2                       221
#define IE_AIRONET_IPADDRESS            149   
#define IE_AIRONET_CCKMREASSOC          156   
#define CKIP_NEGOTIATION_LENGTH         30
#define AIRONET_IPADDRESS_LENGTH        10
#define AIRONET_CCKMREASSOC_LENGTH      24






#define ASSOC_STATE_MACHINE             1
#define AUTH_STATE_MACHINE              2
#define AUTH_RSP_STATE_MACHINE          3
#define SYNC_STATE_MACHINE              4
#define MLME_CNTL_STATE_MACHINE         5
#define WPA_PSK_STATE_MACHINE           6

#define AIRONET_STATE_MACHINE           8
#define ACTION_STATE_MACHINE           9


#define AP_ASSOC_STATE_MACHINE          11
#define AP_AUTH_STATE_MACHINE           12
#define AP_SYNC_STATE_MACHINE           14
#define AP_CNTL_STATE_MACHINE           15
#define WSC_STATE_MACHINE            17
#define WSC_UPNP_STATE_MACHINE		    18


#define WPA_STATE_MACHINE			23


#ifdef QOS_DLS_SUPPORT
#define DLS_STATE_MACHINE               26
#endif 




#define CNTL_IDLE                       0
#define CNTL_WAIT_DISASSOC              1
#define CNTL_WAIT_JOIN                  2
#define CNTL_WAIT_REASSOC               3
#define CNTL_WAIT_START                 4
#define CNTL_WAIT_AUTH                  5
#define CNTL_WAIT_ASSOC                 6
#define CNTL_WAIT_AUTH2                 7
#define CNTL_WAIT_OID_LIST_SCAN         8
#define CNTL_WAIT_OID_DISASSOC          9

#define MT2_ASSOC_CONF                  34
#define MT2_AUTH_CONF                   35
#define MT2_DEAUTH_CONF                 36
#define MT2_DISASSOC_CONF               37
#define MT2_REASSOC_CONF                38
#define MT2_PWR_MGMT_CONF               39
#define MT2_JOIN_CONF                   40
#define MT2_SCAN_CONF                   41
#define MT2_START_CONF                  42
#define MT2_GET_CONF                    43
#define MT2_SET_CONF                    44
#define MT2_RESET_CONF                  45
#define MT2_FT_OTD_CONF					46
#define MT2_MLME_ROAMING_REQ            52

#define CNTL_FUNC_SIZE                  1




#define ASSOC_IDLE                      0
#define ASSOC_WAIT_RSP                  1
#define REASSOC_WAIT_RSP                2
#define DISASSOC_WAIT_RSP               3
#define MAX_ASSOC_STATE                 4

#define ASSOC_MACHINE_BASE              0
#define MT2_MLME_ASSOC_REQ              0
#define MT2_MLME_REASSOC_REQ            1
#define MT2_MLME_DISASSOC_REQ           2
#define MT2_PEER_DISASSOC_REQ           3
#define MT2_PEER_ASSOC_REQ              4
#define MT2_PEER_ASSOC_RSP              5
#define MT2_PEER_REASSOC_REQ            6
#define MT2_PEER_REASSOC_RSP            7
#define MT2_DISASSOC_TIMEOUT            8
#define MT2_ASSOC_TIMEOUT               9
#define MT2_REASSOC_TIMEOUT             10
#define MAX_ASSOC_MSG                   11

#define ASSOC_FUNC_SIZE                 (MAX_ASSOC_STATE * MAX_ASSOC_MSG)




#define ACT_IDLE                      0
#define MAX_ACT_STATE                 1

#define ACT_MACHINE_BASE              0



#define MT2_PEER_SPECTRUM_CATE              0
#define MT2_PEER_QOS_CATE              1
#define MT2_PEER_DLS_CATE             2
#define MT2_PEER_BA_CATE             3
#define MT2_PEER_PUBLIC_CATE             4
#define MT2_PEER_RM_CATE             5

#define MT2_PEER_HT_CATE             7	
#define MAX_PEER_CATE_MSG                   7


#define MT2_MLME_ADD_BA_CATE             8
#define MT2_MLME_ORI_DELBA_CATE             9
#define MT2_MLME_REC_DELBA_CATE             10
#define MT2_MLME_QOS_CATE              11
#define MT2_MLME_DLS_CATE             12
#define MT2_ACT_INVALID             13

#define MAX_ACT_MSG                   14



#define CATEGORY_SPECTRUM		0
#define CATEGORY_QOS			1
#define CATEGORY_DLS			2
#define CATEGORY_BA			3
#define CATEGORY_PUBLIC		4
#define CATEGORY_RM			5
#define CATEGORY_HT			7



#define ACTION_DLS_REQUEST			0
#define ACTION_DLS_RESPONSE			1
#define ACTION_DLS_TEARDOWN			2


#define SPEC_MRQ	0	
#define SPEC_MRP	1	
#define SPEC_TPCRQ	2
#define SPEC_TPCRP	3
#define SPEC_CHANNEL_SWITCH	4



#define ADDBA_REQ	0
#define ADDBA_RESP	1
#define DELBA   2


#define ACTION_BSS_2040_COEXIST				0	
#define ACTION_DSE_ENABLEMENT					1	
#define ACTION_DSE_DEENABLEMENT				2	
#define ACTION_DSE_REG_LOCATION_ANNOUNCE	3	
#define ACTION_EXT_CH_SWITCH_ANNOUNCE		4	
#define ACTION_DSE_MEASUREMENT_REQ			5	
#define ACTION_DSE_MEASUREMENT_REPORT		6	
#define ACTION_MEASUREMENT_PILOT_ACTION		7	
#define ACTION_DSE_POWER_CONSTRAINT			8	



#define NOTIFY_BW_ACTION				0
#define SMPS_ACTION						1
#define PSMP_ACTION					2
#define SETPCO_ACTION					3
#define MIMO_CHA_MEASURE_ACTION			4
#define MIMO_N_BEACONFORM				5
#define MIMO_BEACONFORM					6
#define ANTENNA_SELECT					7
#define HT_INFO_EXCHANGE				8

#define ACT_FUNC_SIZE                 (MAX_ACT_STATE * MAX_ACT_MSG)



#define AUTH_REQ_IDLE                   0
#define AUTH_WAIT_SEQ2                  1
#define AUTH_WAIT_SEQ4                  2
#define MAX_AUTH_STATE                  3

#define AUTH_MACHINE_BASE               0
#define MT2_MLME_AUTH_REQ               0
#define MT2_PEER_AUTH_EVEN              1
#define MT2_AUTH_TIMEOUT                2
#define MAX_AUTH_MSG                    3

#define AUTH_FUNC_SIZE                  (MAX_AUTH_STATE * MAX_AUTH_MSG)




#define AUTH_RSP_IDLE                   0
#define AUTH_RSP_WAIT_CHAL              1
#define MAX_AUTH_RSP_STATE              2

#define AUTH_RSP_MACHINE_BASE           0
#define MT2_AUTH_CHALLENGE_TIMEOUT      0
#define MT2_PEER_AUTH_ODD               1
#define MT2_PEER_DEAUTH                 2
#define MAX_AUTH_RSP_MSG                3

#define AUTH_RSP_FUNC_SIZE              (MAX_AUTH_RSP_STATE * MAX_AUTH_RSP_MSG)




#define SYNC_IDLE                       0  
#define JOIN_WAIT_BEACON                1
#define SCAN_LISTEN                     2
#define MAX_SYNC_STATE                  3

#define SYNC_MACHINE_BASE               0
#define MT2_MLME_SCAN_REQ               0
#define MT2_MLME_JOIN_REQ               1
#define MT2_MLME_START_REQ              2
#define MT2_PEER_BEACON                 3
#define MT2_PEER_PROBE_RSP              4
#define MT2_PEER_ATIM                   5
#define MT2_SCAN_TIMEOUT                6
#define MT2_BEACON_TIMEOUT              7
#define MT2_ATIM_TIMEOUT                8
#define MT2_PEER_PROBE_REQ              9
#define MAX_SYNC_MSG                    10

#define SYNC_FUNC_SIZE                  (MAX_SYNC_STATE * MAX_SYNC_MSG)


#define DLS_IDLE						0
#define MAX_DLS_STATE					1

#define DLS_MACHINE_BASE				0
#define MT2_MLME_DLS_REQ			    0
#define MT2_PEER_DLS_REQ			    1
#define MT2_PEER_DLS_RSP			    2
#define MT2_MLME_DLS_TEAR_DOWN		    3
#define MT2_PEER_DLS_TEAR_DOWN		    4
#define MAX_DLS_MSG				        5

#define DLS_FUNC_SIZE					(MAX_DLS_STATE * MAX_DLS_MSG)








#define AP_CNTL_FUNC_SIZE               1




#define AP_ASSOC_IDLE                   0
#define AP_MAX_ASSOC_STATE              1

#define AP_ASSOC_MACHINE_BASE           0
#define APMT2_MLME_DISASSOC_REQ         0
#define APMT2_PEER_DISASSOC_REQ         1
#define APMT2_PEER_ASSOC_REQ            2
#define APMT2_PEER_REASSOC_REQ          3
#define APMT2_CLS3ERR                   4
#define AP_MAX_ASSOC_MSG                5

#define AP_ASSOC_FUNC_SIZE              (AP_MAX_ASSOC_STATE * AP_MAX_ASSOC_MSG)




#define AP_AUTH_REQ_IDLE                0
#define AP_MAX_AUTH_STATE               1

#define AP_AUTH_MACHINE_BASE            0
#define APMT2_MLME_DEAUTH_REQ           0
#define APMT2_CLS2ERR                   1
#define APMT2_PEER_DEAUTH				2
#define APMT2_PEER_AUTH_REQ				3
#define APMT2_PEER_AUTH_CONFIRM			4
#define AP_MAX_AUTH_MSG                 5

#define AP_AUTH_FUNC_SIZE               (AP_MAX_AUTH_STATE * AP_MAX_AUTH_MSG)




#define AP_SYNC_IDLE                    0
#define AP_SCAN_LISTEN					1
#define AP_MAX_SYNC_STATE               2

#define AP_SYNC_MACHINE_BASE            0
#define APMT2_PEER_PROBE_REQ            0
#define APMT2_PEER_BEACON               1
#define APMT2_MLME_SCAN_REQ				2
#define APMT2_PEER_PROBE_RSP			3
#define APMT2_SCAN_TIMEOUT				4
#define APMT2_MLME_SCAN_CNCL			5
#define AP_MAX_SYNC_MSG                 6

#define AP_SYNC_FUNC_SIZE               (AP_MAX_SYNC_STATE * AP_MAX_SYNC_MSG)




#define WPA_PTK                      0
#define MAX_WPA_PTK_STATE            1

#define WPA_MACHINE_BASE             0
#define MT2_EAPPacket                0
#define MT2_EAPOLStart               1
#define MT2_EAPOLLogoff              2
#define MT2_EAPOLKey                 3
#define MT2_EAPOLASFAlert            4
#define MAX_WPA_MSG                  5

#define WPA_FUNC_SIZE                (MAX_WPA_PTK_STATE * MAX_WPA_MSG)

#ifdef APCLI_SUPPORT

#define APCLI_AUTH_REQ_IDLE                0
#define APCLI_AUTH_WAIT_SEQ2               1
#define APCLI_AUTH_WAIT_SEQ4               2
#define APCLI_MAX_AUTH_STATE               3

#define APCLI_AUTH_MACHINE_BASE            0
#define APCLI_MT2_MLME_AUTH_REQ            0
#define APCLI_MT2_MLME_DEAUTH_REQ          1
#define APCLI_MT2_PEER_AUTH_EVEN           2
#define APCLI_MT2_PEER_DEAUTH              3
#define APCLI_MT2_AUTH_TIMEOUT             4
#define APCLI_MAX_AUTH_MSG                 5

#define APCLI_AUTH_FUNC_SIZE               (APCLI_MAX_AUTH_STATE * APCLI_MAX_AUTH_MSG)


#define APCLI_ASSOC_IDLE                   0
#define APCLI_ASSOC_WAIT_RSP               1
#define APCLI_MAX_ASSOC_STATE              2

#define APCLI_ASSOC_MACHINE_BASE           0
#define APCLI_MT2_MLME_ASSOC_REQ           0
#define APCLI_MT2_MLME_DISASSOC_REQ        1
#define APCLI_MT2_PEER_DISASSOC_REQ        2
#define APCLI_MT2_PEER_ASSOC_RSP           3
#define APCLI_MT2_ASSOC_TIMEOUT            4
#define APCLI_MAX_ASSOC_MSG                5

#define APCLI_ASSOC_FUNC_SIZE              (APCLI_MAX_ASSOC_STATE * APCLI_MAX_ASSOC_MSG)


#define APCLI_SYNC_IDLE                   0  
#define APCLI_JOIN_WAIT_PROBE_RSP         1
#define APCLI_MAX_SYNC_STATE              2

#define APCLI_SYNC_MACHINE_BASE           0
#define APCLI_MT2_MLME_PROBE_REQ          0
#define APCLI_MT2_PEER_PROBE_RSP          1
#define APCLI_MT2_PROBE_TIMEOUT           2
#define APCLI_MAX_SYNC_MSG                3

#define APCLI_SYNC_FUNC_SIZE              (APCLI_MAX_SYNC_STATE * APCLI_MAX_SYNC_MSG)


#define APCLI_CTRL_DISCONNECTED           0  
#define APCLI_CTRL_PROBE                  1
#define APCLI_CTRL_AUTH                   2
#define APCLI_CTRL_AUTH_2                 3
#define APCLI_CTRL_ASSOC                  4
#define APCLI_CTRL_DEASSOC                5
#define APCLI_CTRL_CONNECTED              6
#define APCLI_MAX_CTRL_STATE              7

#define APCLI_CTRL_MACHINE_BASE           0
#define APCLI_CTRL_JOIN_REQ               0
#define APCLI_CTRL_PROBE_RSP              1
#define APCLI_CTRL_AUTH_RSP               2
#define APCLI_CTRL_DISCONNECT_REQ         3
#define APCLI_CTRL_PEER_DISCONNECT_REQ    4
#define APCLI_CTRL_ASSOC_RSP              5
#define APCLI_CTRL_DEASSOC_RSP            6
#define APCLI_CTRL_JOIN_REQ_TIMEOUT       7
#define APCLI_CTRL_AUTH_REQ_TIMEOUT       8
#define APCLI_CTRL_ASSOC_REQ_TIMEOUT      9
#define APCLI_MAX_CTRL_MSG                10

#define APCLI_CTRL_FUNC_SIZE              (APCLI_MAX_CTRL_STATE * APCLI_MAX_CTRL_MSG)


#endif 





#define BTYPE_MGMT                  0
#define BTYPE_CNTL                  1
#define BTYPE_DATA                  2


#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK              14


#define SUBTYPE_WRAPPER		7
#define SUBTYPE_BLOCK_ACK_REQ       8
#define SUBTYPE_BLOCK_ACK           9
#define SUBTYPE_PS_POLL             10
#define SUBTYPE_RTS                 11
#define SUBTYPE_CTS                 12
#define SUBTYPE_ACK                 13
#define SUBTYPE_CFEND               14
#define SUBTYPE_CFEND_CFACK         15


#define SUBTYPE_DATA                0
#define SUBTYPE_DATA_CFACK          1
#define SUBTYPE_DATA_CFPOLL         2
#define SUBTYPE_DATA_CFACK_CFPOLL   3
#define SUBTYPE_NULL_FUNC           4
#define SUBTYPE_CFACK               5
#define SUBTYPE_CFPOLL              6
#define SUBTYPE_CFACK_CFPOLL        7
#define SUBTYPE_QDATA               8
#define SUBTYPE_QDATA_CFACK         9
#define SUBTYPE_QDATA_CFPOLL        10
#define SUBTYPE_QDATA_CFACK_CFPOLL  11
#define SUBTYPE_QOS_NULL            12
#define SUBTYPE_QOS_CFACK           13
#define SUBTYPE_QOS_CFPOLL          14
#define SUBTYPE_QOS_CFACK_CFPOLL    15


#define NORMAL_ACK                  0x00  
#define NO_ACK                      0x20  
#define NO_EXPLICIT_ACK             0x40  
#define BLOCK_ACK                   0x60  




#define LENGTH_802_11               24
#define LENGTH_802_11_AND_H         30
#define LENGTH_802_11_CRC_H         34
#define LENGTH_802_11_CRC           28
#define LENGTH_802_11_WITH_ADDR4    30
#define LENGTH_802_3                14
#define LENGTH_802_3_TYPE           2
#define LENGTH_802_1_H              8
#define LENGTH_EAPOL_H              4
#define LENGTH_WMMQOS_H				2
#define LENGTH_CRC                  4
#define MAX_SEQ_NUMBER              0x0fff
#define LENGTH_802_3_NO_TYPE		12
#define LENGTH_802_1Q				4 


#define TX_RESULT_SUCCESS           0
#define TX_RESULT_ZERO_LENGTH       1
#define TX_RESULT_UNDER_RUN         2
#define TX_RESULT_OHY_ERROR         4
#define TX_RESULT_RETRY_FAIL        6



#define MODE_CCK	0
#define MODE_OFDM   1
#ifdef DOT11_N_SUPPORT
#define MODE_HTMIX	2
#define MODE_HTGREENFIELD	3
#endif 

#define MCS_LONGP_RATE_1                      0	 
#define MCS_LONGP_RATE_2                      1	
#define MCS_LONGP_RATE_5_5                    2
#define MCS_LONGP_RATE_11                     3
#define MCS_SHORTP_RATE_1                      4	 
#define MCS_SHORTP_RATE_2                      5	
#define MCS_SHORTP_RATE_5_5                    6
#define MCS_SHORTP_RATE_11                     7

#define MCS_RATE_6                      0   
#define MCS_RATE_9                      1   
#define MCS_RATE_12                     2   
#define MCS_RATE_18                     3   
#define MCS_RATE_24                     4  
#define MCS_RATE_36                     5   
#define MCS_RATE_48                     6  
#define MCS_RATE_54                     7 

#define MCS_0		0	
#define MCS_1		1
#define MCS_2		2
#define MCS_3		3
#define MCS_4		4
#define MCS_5		5
#define MCS_6		6
#define MCS_7		7
#define MCS_8		8	
#define MCS_9		9
#define MCS_10		10
#define MCS_11		11
#define MCS_12		12
#define MCS_13		13
#define MCS_14		14
#define MCS_15		15
#define MCS_16		16	
#define MCS_17		17
#define MCS_18		18
#define MCS_19		19
#define MCS_20		20
#define MCS_21		21
#define MCS_22		22
#define MCS_23		23
#define MCS_32		32
#define MCS_AUTO		33

#ifdef DOT11_N_SUPPORT


#define HTMODE_MM	0
#define HTMODE_GF	1
#endif 


#define FIXED_TXMODE_HT		0
#define FIXED_TXMODE_CCK	1
#define FIXED_TXMODE_OFDM	2

#define BW_20		BAND_WIDTH_20
#define BW_40		BAND_WIDTH_40
#define BW_BOTH		BAND_WIDTH_BOTH
#define BW_10		BAND_WIDTH_10	

#ifdef DOT11_N_SUPPORT

#define GI_400		GAP_INTERVAL_400	
#define GI_BOTH		GAP_INTERVAL_BOTH
#endif 
#define GI_800		GAP_INTERVAL_800

#define STBC_NONE	0
#ifdef DOT11_N_SUPPORT
#define STBC_USE	1	
#define RXSTBC_ONE	1	
#define RXSTBC_TWO	2	
#define RXSTBC_THR	3	

#define MCSFBK_NONE	0  
#define MCSFBK_RSV	1	
#define MCSFBK_UNSOLICIT	2	
#define MCSFBK_MRQ	3	


#define	MMPS_STATIC	0
#define	MMPS_DYNAMIC		1
#define   MMPS_RSV		2
#define MMPS_ENABLE		3



#define	AMSDU_0	0
#define	AMSDU_1		1

#endif 


#define TXRATEMIMO		0x80
#define TXRATEMCS		0x7F
#define TXRATEOFDM		0x7F
#define RATE_1                      0
#define RATE_2                      1
#define RATE_5_5                    2
#define RATE_11                     3
#define RATE_6                      4   
#define RATE_9                      5   
#define RATE_12                     6   
#define RATE_18                     7   
#define RATE_24                     8   
#define RATE_36                     9   
#define RATE_48                     10  
#define RATE_54                     11  
#define RATE_FIRST_OFDM_RATE        RATE_6
#define RATE_LAST_OFDM_RATE		RATE_54
#define RATE_6_5                    12  
#define RATE_13                     13  
#define RATE_19_5                   14  
#define RATE_26                     15  
#define RATE_39                     16  
#define RATE_52                     17  
#define RATE_58_5                   18  
#define RATE_65                     19  
#define RATE_78                     20  
#define RATE_104                    21  
#define RATE_117                    22  
#define RATE_130                    23  

#define HTRATE_0                      12
#define RATE_FIRST_MM_RATE        HTRATE_0
#define RATE_FIRST_HT_RATE        HTRATE_0
#define RATE_LAST_HT_RATE        HTRATE_0


#define IFS_HTTXOP                 0	
#define IFS_PIFS                    1
#define IFS_SIFS                    2
#define IFS_BACKOFF                 3


#define LONG_RETRY                  1
#define SHORT_RETRY                 0


#define REGION_MINIMUM_BG_BAND            0
#define REGION_0_BG_BAND                  0       
#define REGION_1_BG_BAND                  1       
#define REGION_2_BG_BAND                  2       
#define REGION_3_BG_BAND                  3       
#define REGION_4_BG_BAND                  4       
#define REGION_5_BG_BAND                  5       
#define REGION_6_BG_BAND                  6       
#define REGION_7_BG_BAND                  7       
#define REGION_31_BG_BAND                 31       
#define REGION_MAXIMUM_BG_BAND            7

#define REGION_MINIMUM_A_BAND             0
#define REGION_0_A_BAND                   0       
#define REGION_1_A_BAND                   1       
#define REGION_2_A_BAND                   2       
#define REGION_3_A_BAND                   3       
#define REGION_4_A_BAND                   4       
#define REGION_5_A_BAND                   5       
#define REGION_6_A_BAND                   6       
#define REGION_7_A_BAND                   7       
#define REGION_8_A_BAND                   8       
#define REGION_9_A_BAND                   9       
#define REGION_10_A_BAND                  10	  
#define REGION_11_A_BAND                  11	  
#define REGION_12_A_BAND                  12       
#define REGION_13_A_BAND                  13       
#define REGION_14_A_BAND                  14       
#define REGION_15_A_BAND                  15       
#define REGION_MAXIMUM_A_BAND             15


#define CIPHER_NONE                 0
#define CIPHER_WEP64                1
#define CIPHER_WEP128               2
#define CIPHER_TKIP                 3
#define CIPHER_AES                  4
#define CIPHER_CKIP64               5
#define CIPHER_CKIP128              6
#define CIPHER_TKIP_NO_MIC          7       
#define CIPHER_SMS4					8



#define LED_LINK_DOWN               0
#define LED_LINK_UP                 1
#define LED_RADIO_OFF               2
#define LED_RADIO_ON                3
#define LED_HALT                    4
#define LED_WPS                     5
#define LED_ON_SITE_SURVEY          6
#define LED_POWER_UP                7



#define LED_MODE_DEFAULT            0
#define LED_MODE_TWO_LED			1

#define LED_MODE_SIGNAL_STREGTH		0x40 


#define PPPINITFCS32                0xffffffff   


#define WPA_802_1X_PORT_SECURED     1
#define WPA_802_1X_PORT_NOT_SECURED 2

#define PAIRWISE_KEY                1
#define GROUP_KEY                   2


#define MAX_STEP_OF_TX_RATE_SWITCH	32



#define MAX_NUM_OF_FREE_NDIS_PACKET 128


#define MAX_TX_REORDERBUF   64
#define MAX_RX_REORDERBUF   64
#define DEFAULT_TX_TIMEOUT   30
#define DEFAULT_RX_TIMEOUT   30


#define I_RECIPIENT                  TRUE
#define I_ORIGINATOR                   FALSE

#define DEFAULT_BBP_TX_POWER        0
#define DEFAULT_RF_TX_POWER         5

#define MAX_INI_BUFFER_SIZE		4096
#define MAX_PARAM_BUFFER_SIZE		(2048) 
											
											

#define OPMODE_STA                  0
#define OPMODE_AP                   1


#ifdef RT_BIG_ENDIAN
#define DIR_READ                    0
#define DIR_WRITE                   1
#define TYPE_TXD                    0
#define TYPE_RXD                    1
#define TYPE_TXINFO					0
#define TYPE_RXINFO					1
#define TYPE_TXWI					0
#define TYPE_RXWI					1
#endif



#define EVENT_RESET_ACCESS_POINT    0 
#define EVENT_ASSOCIATED            1 
#define EVENT_DISASSOCIATED         2 
#define EVENT_AGED_OUT              3 
#define EVENT_COUNTER_M             4
#define EVENT_INVALID_PSK           5
#define EVENT_MAX_EVENT_TYPE        6



#define RSSI_0					0
#define RSSI_1					1
#define RSSI_2					2


#define RD_NORMAL_MODE				0	
#define RD_SWITCHING_MODE			1	
#define RD_SILENCE_MODE				2	


#define  SLEEPCID	0x11
#define  WAKECID	0x22
#define  QUERYPOWERCID	0x33
#define  OWNERMCU	0x1
#define  OWNERCPU	0x0


#define ENTRY_NOT_FOUND             0xFF



#define INT_MAIN			0x0100
#define INT_MBSSID			0x0200
#define INT_WDS				0x0300
#define INT_APCLI			0x0400
#define INT_MESH			0x0500

#define INF_MAIN_DEV_NAME		"wlan"
#define INF_MBSSID_DEV_NAME		"wlan"
#define INF_WDS_DEV_NAME		"wds"
#define INF_APCLI_DEV_NAME		"apcli"
#define INF_MESH_DEV_NAME		"mesh"


#ifdef RALINK_ATE
#define	ATE_START                   0x00   
#define	ATE_STOP                    0x80   
#define	ATE_TXCONT                  0x05   
#define	ATE_TXCARR                  0x09   
#define	ATE_TXCARRSUPP              0x11   
#define	ATE_TXFRAME                 0x01   
#define	ATE_RXFRAME                 0x02   
#ifdef RALINK_28xx_QA
#define ATE_TXSTOP                  0xe2   
#define ATE_RXSTOP					0xfd   
#define	BBP22_TXFRAME				0x00   
#define	BBP22_TXCONT_OR_CARRSUPP    0x80   
#define	BBP22_TXCARR                0xc1   
#define	BBP24_TXCONT                0x00   
#define	BBP24_CARRSUPP              0x01   
#endif 
#endif 


#define WEP_HEXADECIMAL_TYPE    0
#define WEP_ASCII_TYPE          1





#define IW_CUSTOM_MAX_LEN							255	


#define	IW_SYS_EVENT_FLAG_START                     0x0200
#define	IW_ASSOC_EVENT_FLAG                         0x0200
#define	IW_DISASSOC_EVENT_FLAG                      0x0201
#define	IW_DEAUTH_EVENT_FLAG				0x0202
#define	IW_AGEOUT_EVENT_FLAG				0x0203
#define	IW_COUNTER_MEASURES_EVENT_FLAG              0x0204
#define	IW_REPLAY_COUNTER_DIFF_EVENT_FLAG           0x0205
#define	IW_RSNIE_DIFF_EVENT_FLAG				0x0206
#define	IW_MIC_DIFF_EVENT_FLAG					0x0207
#define IW_ICV_ERROR_EVENT_FLAG						0x0208
#define IW_MIC_ERROR_EVENT_FLAG						0x0209
#define IW_GROUP_HS_TIMEOUT_EVENT_FLAG				0x020A
#define	IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG			0x020B
#define IW_RSNIE_SANITY_FAIL_EVENT_FLAG				0x020C
#define IW_SET_KEY_DONE_WPA1_EVENT_FLAG				0x020D
#define IW_SET_KEY_DONE_WPA2_EVENT_FLAG				0x020E
#define IW_STA_LINKUP_EVENT_FLAG					0x020F
#define IW_STA_LINKDOWN_EVENT_FLAG					0x0210
#define IW_SCAN_COMPLETED_EVENT_FLAG				0x0211
#define IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG				0x0212

#define	IW_SYS_EVENT_FLAG_END                       0x0212
#define	IW_SYS_EVENT_TYPE_NUM						(IW_SYS_EVENT_FLAG_END - IW_SYS_EVENT_FLAG_START + 1)



#define	IW_SPOOF_EVENT_FLAG_START                   0x0300
#define IW_CONFLICT_SSID_EVENT_FLAG					0x0300
#define IW_SPOOF_ASSOC_RESP_EVENT_FLAG				0x0301
#define IW_SPOOF_REASSOC_RESP_EVENT_FLAG			0x0302
#define IW_SPOOF_PROBE_RESP_EVENT_FLAG				0x0303
#define IW_SPOOF_BEACON_EVENT_FLAG					0x0304
#define IW_SPOOF_DISASSOC_EVENT_FLAG				0x0305
#define IW_SPOOF_AUTH_EVENT_FLAG					0x0306
#define IW_SPOOF_DEAUTH_EVENT_FLAG					0x0307
#define IW_SPOOF_UNKNOWN_MGMT_EVENT_FLAG			0x0308
#define IW_REPLAY_ATTACK_EVENT_FLAG					0x0309

#define	IW_SPOOF_EVENT_FLAG_END                     0x0309
#define	IW_SPOOF_EVENT_TYPE_NUM						(IW_SPOOF_EVENT_FLAG_END - IW_SPOOF_EVENT_FLAG_START + 1)



#define	IW_FLOOD_EVENT_FLAG_START                   0x0400
#define IW_FLOOD_AUTH_EVENT_FLAG					0x0400
#define IW_FLOOD_ASSOC_REQ_EVENT_FLAG				0x0401
#define IW_FLOOD_REASSOC_REQ_EVENT_FLAG				0x0402
#define IW_FLOOD_PROBE_REQ_EVENT_FLAG				0x0403
#define IW_FLOOD_DISASSOC_EVENT_FLAG				0x0404
#define IW_FLOOD_DEAUTH_EVENT_FLAG					0x0405
#define IW_FLOOD_EAP_REQ_EVENT_FLAG					0x0406

#define	IW_FLOOD_EVENT_FLAG_END				0x0406
#define	IW_FLOOD_EVENT_TYPE_NUM						(IW_FLOOD_EVENT_FLAG_END - IW_FLOOD_EVENT_FLAG_START + 1)




#ifdef CONFIG_STA_SUPPORT

#define	MAX_NUM_OF_INIT_DLS_ENTRY   1
#define	MAX_NUM_OF_DLS_ENTRY        MAX_NUMBER_OF_DLS_ENTRY


#define MAX_TX_REORDERBUF		64
#define MAX_RX_REORDERBUF		64
#define DEFAULT_TX_TIMEOUT		30
#define DEFAULT_RX_TIMEOUT		30
#define MAX_BARECI_SESSION		8

#ifndef IW_ESSID_MAX_SIZE

#define IW_ESSID_MAX_SIZE		32
#endif
#endif 

#ifdef MCAST_RATE_SPECIFIC
#define MCAST_DISABLE	0
#define MCAST_CCK		1
#define MCAST_OFDM		2
#define MCAST_HTMIX		3
#endif 


#define DOT11POWERSAVE		0
#define GUIRADIO_OFF		1
#define RTMP_HALT		    2
#define GUI_IDLE_POWER_SAVE		3




#define WPA_SUPPLICANT_DISABLE				0
#define WPA_SUPPLICANT_ENABLE				1
#define	WPA_SUPPLICANT_ENABLE_WITH_WEB_UI	2


#ifdef ANT_DIVERSITY_SUPPORT
enum ANT_DIVERSITY_TYPE {
    ANT_DIVERSITY_DISABLE = 0,
    ANT_DIVERSITY_ENABLE = 1,
    ANT_FIX_ANT1 = 2,
    ANT_FIX_ANT2 = 3
};
#endif 


#define SWAP16(x) \
    ((UINT16)( \
    (((UINT16)(x) & (UINT16) 0x00ffU) << 8) | \
    (((UINT16)(x) & (UINT16) 0xff00U) >> 8) ))

#define SWAP32(x) \
    ((UINT32)( \
    (((UINT32)(x) & (UINT32) 0x000000ffUL) << 24) | \
    (((UINT32)(x) & (UINT32) 0x0000ff00UL) <<  8) | \
    (((UINT32)(x) & (UINT32) 0x00ff0000UL) >>  8) | \
    (((UINT32)(x) & (UINT32) 0xff000000UL) >> 24) ))

#define SWAP64(x) \
    ((UINT64)( \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
    (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))

#ifdef RT_BIG_ENDIAN

#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))

#else   

#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))

#endif  

#define ABS(_x, _y) ((_x) > (_y)) ? ((_x) -(_y)) : ((_y) -(_x))


#define A2Dec(_X, _p)				\
{									\
	UCHAR *p;						\
	_X = 0;							\
	p = _p;							\
	while (((*p >= '0') && (*p <= '9')))		\
	{												\
		if ((*p >= '0') && (*p <= '9'))		\
			_X = _X * 10 + *p - 48;					\
		p++;										\
	}												\
}


#define A2Hex(_X, _p)				\
do{									\
	char *__p;						\
	(_X) = 0;							\
	__p = (char *)(_p);							\
	while (((*__p >= 'a') && (*__p <= 'f')) || ((*__p >= 'A') && (*__p <= 'F')) || ((*__p >= '0') && (*__p <= '9')))		\
	{												\
		if ((*__p >= 'a') && (*__p <= 'f'))				\
			(_X) = (_X) * 16 + *__p - 87;					\
		else if ((*__p >= 'A') && (*__p <= 'F'))		\
			(_X) = (_X) * 16 + *__p - 55;					\
		else if ((*__p >= '0') && (*__p <= '9'))		\
			(_X) = (_X) * 16 + *__p - 48;					\
		__p++;										\
	}												\
}while(0)

#endif  
