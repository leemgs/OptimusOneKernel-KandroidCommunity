
#ifndef _OID_H_
#define _OID_H_


#define TRUE				1
#define FALSE				0



#define MAX_TX_POWER_LEVEL              100   
#define MAX_RSSI_TRIGGER                -10    
#define MIN_RSSI_TRIGGER                -200   
#define MAX_FRAG_THRESHOLD              2346  
#define MIN_FRAG_THRESHOLD              256   
#define MAX_RTS_THRESHOLD               2347  



#define EXTCHA_NONE			0
#define EXTCHA_ABOVE		0x1
#define EXTCHA_BELOW		0x3


#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_BOTH		2
#define BAND_WIDTH_10		3	

#define GAP_INTERVAL_400	1	
#define GAP_INTERVAL_800	0
#define GAP_INTERVAL_BOTH	2

#define NdisMediaStateConnected			1
#define NdisMediaStateDisconnected		0

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16
#define MAC_ADDR_LENGTH                 6
#define MAX_NUM_OF_CHS					49 
#define MAX_NUMBER_OF_EVENT				10  
#define MAX_NUMBER_OF_MAC				32 
#define MAX_NUMBER_OF_ACL				64
#define MAX_LENGTH_OF_SUPPORT_RATES		12    
#define MAX_NUMBER_OF_DLS_ENTRY			4

#define OID_GEN_MACHINE_NAME               0x0001021A

#define RT_QUERY_SIGNAL_CONTEXT				0x0402
#define RT_SET_IAPP_PID                 	0x0404
#define RT_SET_APD_PID						0x0405
#define RT_SET_DEL_MAC_ENTRY				0x0406




#define	OID_GET_SET_TOGGLE			0x8000

#define OID_802_11_ADD_WEP			0x0112
#define OID_802_11_DISASSOCIATE			0x0114
#define OID_802_11_BSSID_LIST_SCAN		0x0508
#define OID_802_11_SSID				0x0509
#define OID_802_11_BSSID			0x050A
#define OID_802_11_MIC_FAILURE_REPORT_FRAME	0x0528

#define	RT_OID_DEVICE_NAME							0x0607
#define	RT_OID_VERSION_INFO							0x0608
#define	OID_GEN_MEDIA_CONNECT_STATUS				0x060B
#define	OID_GEN_RCV_OK								0x060F
#define	OID_GEN_RCV_NO_BUFFER						0x0610

#define OID_SET_COUNTERMEASURES                     0x0616
#define RT_OID_WPA_SUPPLICANT_SUPPORT               0x0621
#define RT_OID_WE_VERSION_COMPILED                  0x0622
#define RT_OID_NEW_DRIVER                           0x0623


#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_QUERY_MULTIPLE_CARD_SUPPORT          0x0647




typedef enum _NDIS_802_11_STATUS_TYPE
{
    Ndis802_11StatusType_Authentication,
    Ndis802_11StatusType_MediaStreamMode,
    Ndis802_11StatusType_PMKID_CandidateList,
    Ndis802_11StatusTypeMax    
} NDIS_802_11_STATUS_TYPE, *PNDIS_802_11_STATUS_TYPE;

typedef UCHAR   NDIS_802_11_MAC_ADDRESS[6];

typedef struct _NDIS_802_11_STATUS_INDICATION
{
    NDIS_802_11_STATUS_TYPE StatusType;
} NDIS_802_11_STATUS_INDICATION, *PNDIS_802_11_STATUS_INDICATION;


#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f

#define NDIS_802_11_AUTH_REQUEST_REAUTH             0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE          0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR     0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR        0x0E

typedef struct _NDIS_802_11_AUTHENTICATION_REQUEST
{
    ULONG Length;            
    NDIS_802_11_MAC_ADDRESS Bssid;
    ULONG Flags;
} NDIS_802_11_AUTHENTICATION_REQUEST, *PNDIS_802_11_AUTHENTICATION_REQUEST;


typedef struct _PMKID_CANDIDATE {
    NDIS_802_11_MAC_ADDRESS BSSID;
    ULONG Flags;
} PMKID_CANDIDATE, *PPMKID_CANDIDATE;

typedef struct _NDIS_802_11_PMKID_CANDIDATE_LIST
{
    ULONG Version;       
    ULONG NumCandidates; 
    PMKID_CANDIDATE CandidateList[1];
} NDIS_802_11_PMKID_CANDIDATE_LIST, *PNDIS_802_11_PMKID_CANDIDATE_LIST;


#define NDIS_802_11_PMKID_CANDIDATE_PREAUTH_ENABLED	0x01


typedef enum _NDIS_802_11_NETWORK_TYPE
{
   Ndis802_11FH,
   Ndis802_11DS,
    Ndis802_11OFDM5,
    Ndis802_11OFDM5_N,
    Ndis802_11OFDM24,
    Ndis802_11OFDM24_N,
   Ndis802_11Automode,
    Ndis802_11NetworkTypeMax    
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST
{
    UINT                       NumberOfItems;  
   NDIS_802_11_NETWORK_TYPE    NetworkType [1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

typedef enum _NDIS_802_11_POWER_MODE
{
    Ndis802_11PowerModeCAM,
    Ndis802_11PowerModeMAX_PSP,
    Ndis802_11PowerModeFast_PSP,
    Ndis802_11PowerModeLegacy_PSP,
    Ndis802_11PowerModeMax      
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef ULONG   NDIS_802_11_TX_POWER_LEVEL; 




typedef LONG    NDIS_802_11_RSSI;           

typedef struct _NDIS_802_11_CONFIGURATION_FH
{
   ULONG           Length;            
   ULONG           HopPattern;        
   ULONG           HopSet;            
   ULONG           DwellTime;         
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION
{
   ULONG                           Length;             
   ULONG                           BeaconPeriod;       
   ULONG                           ATIMWindow;         
   ULONG                           DSConfig;           
   NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef struct _NDIS_802_11_STATISTICS
{
   ULONG           Length;             
   LARGE_INTEGER   TransmittedFragmentCount;
   LARGE_INTEGER   MulticastTransmittedFrameCount;
   LARGE_INTEGER   FailedCount;
   LARGE_INTEGER   RetryCount;
   LARGE_INTEGER   MultipleRetryCount;
   LARGE_INTEGER   RTSSuccessCount;
   LARGE_INTEGER   RTSFailureCount;
   LARGE_INTEGER   ACKFailureCount;
   LARGE_INTEGER   FrameDuplicateCount;
   LARGE_INTEGER   ReceivedFragmentCount;
   LARGE_INTEGER   MulticastReceivedFrameCount;
   LARGE_INTEGER   FCSErrorCount;
   LARGE_INTEGER   TKIPLocalMICFailures;
   LARGE_INTEGER   TKIPRemoteMICErrors;
   LARGE_INTEGER   TKIPICVErrors;
   LARGE_INTEGER   TKIPCounterMeasuresInvoked;
   LARGE_INTEGER   TKIPReplays;
   LARGE_INTEGER   CCMPFormatErrors;
   LARGE_INTEGER   CCMPReplays;
   LARGE_INTEGER   CCMPDecryptErrors;
   LARGE_INTEGER   FourWayHandshakeFailures;
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;

typedef  ULONG  NDIS_802_11_KEY_INDEX;
typedef ULONGLONG   NDIS_802_11_KEY_RSC;

#define MAX_RADIUS_SRV_NUM			2	  

typedef struct PACKED _RADIUS_SRV_INFO {
	UINT32			radius_ip;
	UINT32			radius_port;
	UCHAR			radius_key[64];
	UCHAR			radius_key_len;
} RADIUS_SRV_INFO, *PRADIUS_SRV_INFO;

typedef struct PACKED _RADIUS_KEY_INFO
{
	UCHAR			radius_srv_num;
	RADIUS_SRV_INFO	radius_srv_info[MAX_RADIUS_SRV_NUM];
	UCHAR			ieee8021xWEP;		 
    UCHAR           key_index;
    UCHAR           key_length;          
    UCHAR           key_material[13];
} RADIUS_KEY_INFO, *PRADIUS_KEY_INFO;


typedef struct PACKED _RADIUS_CONF
{
    UINT32          Length;             
    UCHAR			mbss_num;			
	UINT32			own_ip_addr;
	UINT32			retry_interval;
	UINT32			session_timeout_interval;
	UCHAR			EAPifname[IFNAMSIZ];
	UCHAR			EAPifname_len;
	UCHAR 			PreAuthifname[IFNAMSIZ];
	UCHAR			PreAuthifname_len;
	RADIUS_KEY_INFO	RadiusInfo[8];
} RADIUS_CONF, *PRADIUS_CONF;


typedef struct _NDIS_802_11_KEY
{
    UINT           Length;             
    UINT           KeyIndex;
    UINT           KeyLength;          
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_KEY_RSC KeyRSC;
    UCHAR           KeyMaterial[1];     
} NDIS_802_11_KEY, *PNDIS_802_11_KEY;

typedef struct _NDIS_802_11_REMOVE_KEY
{
    UINT           Length;             
    UINT           KeyIndex;
    NDIS_802_11_MAC_ADDRESS BSSID;
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

typedef struct _NDIS_802_11_WEP
{
   UINT     Length;        
   UINT     KeyIndex;           
                                        
   UINT     KeyLength;     
   UCHAR     KeyMaterial[1];
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;


typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
   Ndis802_11IBSS,
   Ndis802_11Infrastructure,
   Ndis802_11AutoUnknown,
   Ndis802_11Monitor,
   Ndis802_11InfrastructureMax     
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;


typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
   Ndis802_11AuthModeOpen,
   Ndis802_11AuthModeShared,
   Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeWPA,
    Ndis802_11AuthModeWPAPSK,
    Ndis802_11AuthModeWPANone,
   Ndis802_11AuthModeWPA2,
   Ndis802_11AuthModeWPA2PSK,
   	Ndis802_11AuthModeWPA1WPA2,
	Ndis802_11AuthModeWPA1PSKWPA2PSK,
   Ndis802_11AuthModeMax           
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef UCHAR   NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];        
typedef UCHAR   NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];  

typedef struct PACKED _NDIS_802_11_SSID
{
    UINT   SsidLength;         
                                
    UCHAR   Ssid[NDIS_802_11_LENGTH_SSID];           
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;


typedef struct PACKED _NDIS_WLAN_BSSID
{
   ULONG                               Length;     
   NDIS_802_11_MAC_ADDRESS             MacAddress; 
   UCHAR                               Reserved[2];
   NDIS_802_11_SSID                    Ssid;       
   ULONG                               Privacy;    
   NDIS_802_11_RSSI                    Rssi;       
   NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
   NDIS_802_11_CONFIGURATION           Configuration;
   NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
   NDIS_802_11_RATES                   SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct PACKED _NDIS_802_11_BSSID_LIST
{
   UINT           NumberOfItems;      
   NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;


typedef struct PACKED _NDIS_WLAN_BSSID_EX
{
    ULONG                               Length;             
    NDIS_802_11_MAC_ADDRESS             MacAddress;         
    UCHAR                               Reserved[2];
    NDIS_802_11_SSID                    Ssid;               
    UINT                                Privacy;            
    NDIS_802_11_RSSI                    Rssi;               
                                                            
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION           Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    NDIS_802_11_RATES_EX                SupportedRates;
    ULONG                               IELength;
    UCHAR                               IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

typedef struct PACKED _NDIS_802_11_BSSID_LIST_EX
{
    UINT                   NumberOfItems;      
    NDIS_WLAN_BSSID_EX      Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

typedef struct PACKED _NDIS_802_11_FIXED_IEs
{
    UCHAR Timestamp[8];
    USHORT BeaconInterval;
    USHORT Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;

typedef struct _NDIS_802_11_VARIABLE_IEs
{
    UCHAR ElementID;
    UCHAR Length;    
    UCHAR data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

typedef  ULONG   NDIS_802_11_FRAGMENTATION_THRESHOLD;

typedef  ULONG   NDIS_802_11_RTS_THRESHOLD;

typedef  ULONG   NDIS_802_11_ANTENNA;

typedef enum _NDIS_802_11_PRIVACY_FILTER
{
   Ndis802_11PrivFilterAcceptAll,
   Ndis802_11PrivFilter8021xWEP
} NDIS_802_11_PRIVACY_FILTER, *PNDIS_802_11_PRIVACY_FILTER;



typedef enum _NDIS_802_11_WEP_STATUS
{
   Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
   Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
   Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
   Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent,
    Ndis802_11Encryption4Enabled,	
    Ndis802_11Encryption4KeyAbsent,
    Ndis802_11GroupWEP40Enabled,
	Ndis802_11GroupWEP104Enabled,
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS,
  NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef enum _NDIS_802_11_RELOAD_DEFAULTS
{
   Ndis802_11ReloadWEPKeys
} NDIS_802_11_RELOAD_DEFAULTS, *PNDIS_802_11_RELOAD_DEFAULTS;

#define NDIS_802_11_AI_REQFI_CAPABILITIES      1
#define NDIS_802_11_AI_REQFI_LISTENINTERVAL    2
#define NDIS_802_11_AI_REQFI_CURRENTAPADDRESS  4

#define NDIS_802_11_AI_RESFI_CAPABILITIES      1
#define NDIS_802_11_AI_RESFI_STATUSCODE        2
#define NDIS_802_11_AI_RESFI_ASSOCIATIONID     4

typedef struct _NDIS_802_11_AI_REQFI
{
    USHORT Capabilities;
    USHORT ListenInterval;
    NDIS_802_11_MAC_ADDRESS  CurrentAPAddress;
} NDIS_802_11_AI_REQFI, *PNDIS_802_11_AI_REQFI;

typedef struct _NDIS_802_11_AI_RESFI
{
    USHORT Capabilities;
    USHORT StatusCode;
    USHORT AssociationId;
} NDIS_802_11_AI_RESFI, *PNDIS_802_11_AI_RESFI;

typedef struct _NDIS_802_11_ASSOCIATION_INFORMATION
{
    ULONG                   Length;
    USHORT                  AvailableRequestFixedIEs;
    NDIS_802_11_AI_REQFI    RequestFixedIEs;
    ULONG                   RequestIELength;
    ULONG                   OffsetRequestIEs;
    USHORT                  AvailableResponseFixedIEs;
    NDIS_802_11_AI_RESFI    ResponseFixedIEs;
    ULONG                   ResponseIELength;
    ULONG                   OffsetResponseIEs;
} NDIS_802_11_ASSOCIATION_INFORMATION, *PNDIS_802_11_ASSOCIATION_INFORMATION;

typedef struct _NDIS_802_11_AUTHENTICATION_EVENT
{
    NDIS_802_11_STATUS_INDICATION       Status;
    NDIS_802_11_AUTHENTICATION_REQUEST  Request[1];
} NDIS_802_11_AUTHENTICATION_EVENT, *PNDIS_802_11_AUTHENTICATION_EVENT;


typedef enum _NDIS_802_11_MEDIA_STREAM_MODE
{
    Ndis802_11MediaStreamOff,
    Ndis802_11MediaStreamOn,
} NDIS_802_11_MEDIA_STREAM_MODE, *PNDIS_802_11_MEDIA_STREAM_MODE;


typedef UCHAR   NDIS_802_11_PMKID_VALUE[16];

typedef struct _BSSID_INFO
{
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_PMKID_VALUE PMKID;
} BSSID_INFO, *PBSSID_INFO;

typedef struct _NDIS_802_11_PMKID
{
    UINT    Length;
    UINT    BSSIDInfoCount;
    BSSID_INFO BSSIDInfo[1];
} NDIS_802_11_PMKID, *PNDIS_802_11_PMKID;

typedef struct _NDIS_802_11_AUTHENTICATION_ENCRYPTION
{
    NDIS_802_11_AUTHENTICATION_MODE AuthModeSupported;
    NDIS_802_11_ENCRYPTION_STATUS EncryptStatusSupported;
} NDIS_802_11_AUTHENTICATION_ENCRYPTION, *PNDIS_802_11_AUTHENTICATION_ENCRYPTION;

typedef struct _NDIS_802_11_CAPABILITY
{
     ULONG Length;
     ULONG Version;
     ULONG NoOfPMKIDs;
     ULONG NoOfAuthEncryptPairsSupported;
     NDIS_802_11_AUTHENTICATION_ENCRYPTION AuthenticationEncryptionSupported[1];
} NDIS_802_11_CAPABILITY, *PNDIS_802_11_CAPABILITY;

#define RT_PRIV_IOCTL_EXT							(SIOCIWFIRSTPRIV + 0x01) 
#define RTPRIV_IOCTL_SET							(SIOCIWFIRSTPRIV + 0x02)

#define RTPRIV_IOCTL_STATISTICS                     (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE                (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA                    (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)
#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x0E) 
#define RTPRIV_IOCTL_GET_MAC_TABLE					(SIOCIWFIRSTPRIV + 0x0F)

#define RTPRIV_IOCTL_SHOW							(SIOCIWFIRSTPRIV + 0x11)
enum {
    SHOW_CONN_STATUS = 4,
    SHOW_DRVIER_VERION = 5,
    SHOW_BA_INFO = 6,
	SHOW_DESC_INFO = 7,
#ifdef RT2870
	SHOW_RXBULK_INFO = 8,
	SHOW_TXBULK_INFO = 9,
#endif 
    RAIO_OFF = 10,
    RAIO_ON = 11,
	SHOW_CFG_VALUE = 20,
#if !defined(RT2860)
	SHOW_ADHOC_ENTRY_INFO = 21,
#endif
};

#ifdef LLTD_SUPPORT

#define RT_OID_GET_PHY_MODE                         0x761
#endif 

#if defined(RT2860) || defined(RT30xx)

#define OID_MH_802_1X_SUPPORTED               0xFFEDC100
#endif


typedef union  _HTTRANSMIT_SETTING {
	struct	{
	USHORT   	MCS:7;                 
	USHORT		BW:1;	
	USHORT		ShortGI:1;
	USHORT		STBC:2;	
	USHORT		rsv:2;
	USHORT		TxBF:1;
	USHORT		MODE:2;	
	}	field;
	USHORT		word;
 } HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

typedef enum _RT_802_11_PREAMBLE {
    Rt802_11PreambleLong,
    Rt802_11PreambleShort,
    Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;


typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B,
	PHY_11A,
	PHY_11ABG_MIXED,
	PHY_11G,
	PHY_11ABGN_MIXED,	
	PHY_11N_2_4G,		
	PHY_11GN_MIXED,	
	PHY_11AN_MIXED,	
	PHY_11BGN_MIXED,	
	PHY_11AGN_MIXED,	
	PHY_11N_5G,			
} RT_802_11_PHY_MODE;


typedef struct _RT_802_11_LINK_STATUS {
    ULONG   CurrTxRate;         
    ULONG   ChannelQuality;     
    ULONG   TxByteCount;        
    ULONG   RxByteCount;        
    ULONG	CentralChannel;		
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

typedef struct _RT_802_11_EVENT_LOG {
    LARGE_INTEGER   SystemTime;  
    UCHAR           Addr[MAC_ADDR_LENGTH];
    USHORT          Event;       
} RT_802_11_EVENT_LOG, *PRT_802_11_EVENT_LOG;

typedef struct _RT_802_11_EVENT_TABLE {
    ULONG       Num;
    ULONG       Rsv;     
    RT_802_11_EVENT_LOG   Log[MAX_NUMBER_OF_EVENT];
} RT_802_11_EVENT_TABLE, PRT_802_11_EVENT_TABLE;


typedef union  _MACHTTRANSMIT_SETTING {
	struct	{
	USHORT   	MCS:7;                 
	USHORT		BW:1;	
	USHORT		ShortGI:1;
	USHORT		STBC:2;	
	USHORT		rsv:3;
	USHORT		MODE:2;	
	}	field;
	USHORT		word;
 } MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
    UCHAR       Addr[MAC_ADDR_LENGTH];
    UCHAR       Aid;
    UCHAR       Psm;     
    UCHAR		MimoPs;  
    CHAR		AvgRssi0;
	CHAR		AvgRssi1;
	CHAR		AvgRssi2;
	UINT32		ConnectedTime;
    MACHTTRANSMIT_SETTING	TxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
    ULONG       Num;
    RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;


typedef struct _RT_802_11_HARDWARE_REGISTER {
    ULONG   HardwareType;       
    ULONG   Offset;             
    ULONG   Data;               
} RT_802_11_HARDWARE_REGISTER, *PRT_802_11_HARDWARE_REGISTER;

typedef struct _RT_802_11_AP_CONFIG {
    ULONG   EnableTxBurst;      
    ULONG   EnableTurboRate;    
    ULONG   IsolateInterStaTraffic;     
    ULONG   HideSsid;           
    ULONG   UseBGProtection;    
    ULONG   UseShortSlotTime;   
    ULONG   Rsv1;               
    ULONG   SystemErrorBitmap;  
} RT_802_11_AP_CONFIG, *PRT_802_11_AP_CONFIG;


typedef struct _RT_802_11_STA_CONFIG {
    ULONG   EnableTxBurst;      
    ULONG   EnableTurboRate;    
    ULONG   UseBGProtection;    
    ULONG   UseShortSlotTime;   
    ULONG   AdhocMode; 			
    ULONG   HwRadioStatus;      
    ULONG   Rsv1;               
    ULONG   SystemErrorBitmap;  
} RT_802_11_STA_CONFIG, *PRT_802_11_STA_CONFIG;




typedef	struct	_OID_BACAP_STRUC	{
		UCHAR		RxBAWinLimit;
		UCHAR		TxBAWinLimit;
		UCHAR		Policy;	
		UCHAR		MpduDensity;	
		UCHAR       	AmsduEnable;	
		UCHAR       	AmsduSize;	
		UCHAR       	MMPSmode;	
		BOOLEAN		AutoBA;	
} OID_BACAP_STRUC, *POID_BACAP_STRUC;

typedef struct _RT_802_11_ACL_ENTRY {
    UCHAR   Addr[MAC_ADDR_LENGTH];
    USHORT  Rsv;
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

typedef struct PACKED _RT_802_11_ACL {
    ULONG   Policy;             
    ULONG   Num;
    RT_802_11_ACL_ENTRY Entry[MAX_NUMBER_OF_ACL];
} RT_802_11_ACL, *PRT_802_11_ACL;

typedef struct _RT_802_11_WDS {
    ULONG						Num;
    NDIS_802_11_MAC_ADDRESS		Entry[24];
	ULONG						KeyLength;
	UCHAR						KeyMaterial[32];
} RT_802_11_WDS, *PRT_802_11_WDS;

typedef struct _RT_802_11_TX_RATES_ {
    UCHAR       SupRateLen;
    UCHAR       SupRate[MAX_LENGTH_OF_SUPPORT_RATES];
    UCHAR       ExtRateLen;
    UCHAR       ExtRate[MAX_LENGTH_OF_SUPPORT_RATES];
} RT_802_11_TX_RATES, *PRT_802_11_TX_RATES;



#define	GENERAL_LINK_UP			0x0			
#define	GENERAL_LINK_DOWN		0x1			
#define	HW_RADIO_OFF			0x2			
#define	SW_RADIO_OFF			0x3			
#define	AUTH_FAIL				0x4			
#define	AUTH_FAIL_KEYS			0x5			
#define	ASSOC_FAIL				0x6			
#define	EAP_MIC_FAILURE			0x7			
#define	EAP_4WAY_TIMEOUT		0x8			
#define	EAP_GROUP_KEY_TIMEOUT	0x9			
#define	EAP_SUCCESS				0xa			
#define	DETECT_RADAR_SIGNAL		0xb         
#define EXTRA_INFO_MAX			0xb			

#define EXTRA_INFO_CLEAR		0xffffffff


typedef struct {
	RT_802_11_PHY_MODE		PhyMode; 	
	UCHAR		TransmitNo;
	UCHAR		HtMode; 	
	UCHAR		ExtOffset;	
	UCHAR		MCS;
	UCHAR   	BW;
	UCHAR		STBC;
	UCHAR		SHORTGI;
	UCHAR		rsv;
} OID_SET_HT_PHYMODE, *POID_SET_HT_PHYMODE;

#ifdef LLTD_SUPPORT
typedef struct _RT_LLTD_ASSOICATION_ENTRY {
    UCHAR           Addr[ETH_LENGTH_OF_ADDRESS];
    unsigned short  MOR;        
    UCHAR           phyMode;
} RT_LLTD_ASSOICATION_ENTRY, *PRT_LLTD_ASSOICATION_ENTRY;

typedef struct _RT_LLTD_ASSOICATION_TABLE {
    unsigned int                Num;
    RT_LLTD_ASSOICATION_ENTRY   Entry[MAX_NUMBER_OF_MAC];
} RT_LLTD_ASSOICATION_TABLE, *PRT_LLTD_ASSOICATION_TABLE;
#endif 

#define MAX_CUSTOM_LEN 128

typedef enum _RT_802_11_D_CLIENT_MODE
{
   Rt802_11_D_None,
   Rt802_11_D_Flexible,
   Rt802_11_D_Strict,
} RT_802_11_D_CLIENT_MODE, *PRT_802_11_D_CLIENT_MODE;

typedef struct _RT_CHANNEL_LIST_INFO
{
	UCHAR ChannelList[MAX_NUM_OF_CHS];   
	UCHAR ChannelListNum; 
} RT_CHANNEL_LIST_INFO, *PRT_CHANNEL_LIST_INFO;

#ifdef RT2870

typedef	struct	_WSC_CREDENTIAL
{
	NDIS_802_11_SSID	SSID;				
	USHORT				AuthType;			
	USHORT				EncrType;			
	UCHAR				Key[64];			
	USHORT				KeyLength;
	UCHAR				MacAddr[6];			
	UCHAR				KeyIndex;			
	UCHAR				Rsvd[3];			
}	WSC_CREDENTIAL, *PWSC_CREDENTIAL;


typedef	struct	_WSC_PROFILE
{
	UINT			ProfileCnt;
	WSC_CREDENTIAL	Profile[8];				
}	WSC_PROFILE, *PWSC_PROFILE;
#endif

#endif 

