



















#ifndef _MAC_Structures_H_
#define _MAC_Structures_H_

#include <linux/skbuff.h>




#define MAX_CHANNELS                        30
#define MAC_ADDR_LENGTH                     6
#define MAX_WEP_KEY_SIZE                    16  
#define	MAX_802_11_FRAGMENT_NUMBER		10 




#define MASK_PROTOCOL_VERSION_TYPE	0x0F
#define MASK_FRAGMENT_NUMBER		0x000F
#define SEQUENCE_NUMBER_SHIFT		4
#define DIFFER_11_TO_3				18
#define DOT_11_MAC_HEADER_SIZE		24
#define DOT_11_SNAP_SIZE			6
#define DOT_11_DURATION_OFFSET		2
#define DOT_11_SEQUENCE_OFFSET		22 
#define DOT_11_TYPE_OFFSET			30 
#define DOT_11_DATA_OFFSET          24
#define DOT_11_DA_OFFSET			4
#define DOT_3_TYPE_ARP				0x80F3
#define DOT_3_TYPE_IPX				0x8137
#define DOT_3_TYPE_OFFSET			12


#define ETHERNET_HEADER_SIZE			14
#define MAX_ETHERNET_PACKET_SIZE		1514



#define MAC_SUBTYPE_MNGMNT_ASSOC_REQUEST    0x00
#define MAC_SUBTYPE_MNGMNT_ASSOC_RESPONSE   0x10
#define MAC_SUBTYPE_MNGMNT_REASSOC_REQUEST  0x20
#define MAC_SUBTYPE_MNGMNT_REASSOC_RESPONSE 0x30
#define MAC_SUBTYPE_MNGMNT_PROBE_REQUEST    0x40
#define MAC_SUBTYPE_MNGMNT_PROBE_RESPONSE   0x50
#define MAC_SUBTYPE_MNGMNT_BEACON           0x80
#define MAC_SUBTYPE_MNGMNT_ATIM             0x90
#define MAC_SUBTYPE_MNGMNT_DISASSOCIATION   0xA0
#define MAC_SUBTYPE_MNGMNT_AUTHENTICATION   0xB0
#define MAC_SUBTYPE_MNGMNT_DEAUTHENTICATION 0xC0


#define MAC_SUBTYPE_CONTROL_PSPOLL          0xA4
#define MAC_SUBTYPE_CONTROL_RTS             0xB4
#define MAC_SUBTYPE_CONTROL_CTS             0xC4
#define MAC_SUBTYPE_CONTROL_ACK             0xD4
#define MAC_SUBTYPE_CONTROL_CFEND           0xE4
#define MAC_SUBTYPE_CONTROL_CFEND_CFACK     0xF4


#define MAC_SUBTYPE_DATA                    0x08
#define MAC_SUBTYPE_DATA_CFACK              0x18
#define MAC_SUBTYPE_DATA_CFPOLL             0x28
#define MAC_SUBTYPE_DATA_CFACK_CFPOLL       0x38
#define MAC_SUBTYPE_DATA_NULL               0x48
#define MAC_SUBTYPE_DATA_CFACK_NULL         0x58
#define MAC_SUBTYPE_DATA_CFPOLL_NULL        0x68
#define MAC_SUBTYPE_DATA_CFACK_CFPOLL_NULL  0x78


#define MAC_TYPE_MANAGEMENT                 0x00
#define MAC_TYPE_CONTROL                    0x04
#define MAC_TYPE_DATA                       0x08


#define ELEMENT_ID_SSID                     0
#define ELEMENT_ID_SUPPORTED_RATES          1
#define ELEMENT_ID_FH_PARAMETER_SET         2
#define ELEMENT_ID_DS_PARAMETER_SET         3
#define ELEMENT_ID_CF_PARAMETER_SET         4
#define ELEMENT_ID_TIM                      5
#define ELEMENT_ID_IBSS_PARAMETER_SET       6

#define ELEMENT_ID_CHALLENGE_TEXT           16



#define ELEMENT_ID_ERP_INFORMATION			42
#define ELEMENT_ID_EXTENDED_SUPPORTED_RATES 50



#define ELEMENT_ID_RSN_WPA					221
#ifdef _WPA2_
#define ELEMENT_ID_RSN_WPA2				    48
#endif 

#define WLAN_MAX_PAIRWISE_CIPHER_SUITE_COUNT    ((u16) 6)
#define WLAN_MAX_AUTH_KEY_MGT_SUITE_LIST_COUNT  ((u16) 2)


typedef enum enum_PowerManagementMode
{
    ACTIVE = 0,
    POWER_SAVE
} WB_PM_Mode, *PWB_PM_MODE;





#define REASON_REASERED             0
#define REASON_UNSPECIDIED          1
#define REASON_PREAUTH_INVALID      2
#define DEAUTH_REASON_LEFT_BSS      3
#define DISASS_REASON_AP_INACTIVE   4
#define DISASS_REASON_AP_BUSY       5
#define REASON_CLASS2_FRAME_FROM_NONAUTH_STA    6
#define REASON_CLASS3_FRAME_FROM_NONASSO_STA    7
#define DISASS_REASON_LEFT_BSS      8
#define REASON_NOT_AUTH_YET         9

#define REASON_INVALID_IE						13
#define REASON_MIC_ERROR						14
#define REASON_4WAY_HANDSHAKE_TIMEOUT			15
#define REASON_GROUPKEY_UPDATE_TIMEOUT			16
#define REASON_IE_DIFF_4WAY_ASSOC				17
#define REASON_INVALID_MULTICAST_CIPHER			18
#define REASON_INVALID_UNICAST_CIPHER			19
#define REASON_INVALID_AKMP						20
#define REASON_UNSUPPORTED_RSNIE_VERSION		21
#define REASON_INVALID_RSNIE_CAPABILITY			22
#define REASON_802_1X_AUTH_FAIL					23
#define	REASON_CIPHER_REJECT_PER_SEC_POLICY		14








typedef enum enum_TxRate
{
    TXRATE_1M               = 0,
    TXRATE_2MLONG           = 2,
    TXRATE_2MSHORT          = 3,
    TXRATE_55MLONG          = 4,
    TXRATE_55MSHORT         = 5,
    TXRATE_11MLONG          = 6,
    TXRATE_11MSHORT         = 7,
    TXRATE_AUTO             = 255           
} WB_TXRATE, *PWB_TXRATE;


#define	RATE_BITMAP_1M				1
#define	RATE_BITMAP_2M				2
#define	RATE_BITMAP_5dot5M			5
#define RATE_BITMAP_6M				6
#define RATE_BITMAP_9M				9
#define RATE_BITMAP_11M				11
#define RATE_BITMAP_12M				12
#define RATE_BITMAP_18M				18
#define RATE_BITMAP_22M				22
#define RATE_BITMAP_24M				24
#define RATE_BITMAP_33M				17
#define RATE_BITMAP_36M				19
#define RATE_BITMAP_48M				25
#define RATE_BITMAP_54M				28

#define RATE_AUTO					0
#define RATE_1M						2
#define RATE_2M						4
#define RATE_5dot5M					11
#define RATE_6M						12
#define RATE_9M						18
#define RATE_11M					22
#define RATE_12M					24
#define RATE_18M					36
#define RATE_22M					44
#define RATE_24M					48
#define RATE_33M					66
#define RATE_36M					72
#define RATE_48M					96
#define RATE_54M					108
#define RATE_MAX					255


#define CAPABILITY_ESS_BIT				0x0001
#define CAPABILITY_IBSS_BIT				0x0002
#define CAPABILITY_CF_POLL_BIT			0x0004
#define CAPABILITY_CF_POLL_REQ_BIT		0x0008
#define CAPABILITY_PRIVACY_BIT			0x0010
#define CAPABILITY_SHORT_PREAMBLE_BIT	0x0020
#define CAPABILITY_PBCC_BIT				0x0040
#define CAPABILITY_CHAN_AGILITY_BIT		0x0080
#define CAPABILITY_SHORT_SLOT_TIME_BIT	0x0400
#define CAPABILITY_DSSS_OFDM_BIT		0x2000


struct Capability_Information_Element
{
  union
  {
  	u16 __attribute__ ((packed)) wValue;
    #ifdef _BIG_ENDIAN_  
    struct _Capability
    {
        
	u8	Reserved3 : 2;
	u8	DSSS_OFDM : 1;
	u8	Reserved2 : 2;
	u8	Short_Slot_Time : 1;
	u8    Reserved1 : 2;
	u8    Channel_Agility : 1;
	u8    PBCC : 1;
	u8    ShortPreamble : 1;
	u8    CF_Privacy : 1;
	u8    CF_Poll_Request : 1;
	u8    CF_Pollable : 1;
	u8    IBSS : 1;
	u8    ESS : 1;
    } __attribute__ ((packed)) Capability;
    #else
    struct _Capability
    {
        u8    ESS : 1;
        u8    IBSS : 1;
        u8    CF_Pollable : 1;
        u8    CF_Poll_Request : 1;
        u8    CF_Privacy : 1;
        u8    ShortPreamble : 1;
        u8    PBCC : 1;
        u8    Channel_Agility : 1;
        u8    Reserved1 : 2;
		
		u8	Short_Slot_Time : 1;
		u8	Reserved2 : 2;
		u8	DSSS_OFDM : 1;
		u8	Reserved3 : 2;
    } __attribute__ ((packed)) Capability;
    #endif
  }__attribute__ ((packed)) ;
}__attribute__ ((packed));

struct FH_Parameter_Set_Element
{
    u8    Element_ID;
    u8    Length;
    u8    Dwell_Time[2];
    u8    Hop_Set;
    u8    Hop_Pattern;
    u8    Hop_Index;
};

struct DS_Parameter_Set_Element
{
    u8    Element_ID;
    u8    Length;
    u8    Current_Channel;
};

struct Supported_Rates_Element
{
    u8    Element_ID;
    u8    Length;
    u8    SupportedRates[8];
}__attribute__ ((packed));

struct SSID_Element
{
    u8    Element_ID;
    u8    Length;
    u8    SSID[32];
}__attribute__ ((packed)) ;

struct CF_Parameter_Set_Element
{
    u8    Element_ID;
    u8    Length;
    u8    CFP_Count;
    u8    CFP_Period;
    u8    CFP_MaxDuration[2];     
    u8    CFP_DurRemaining[2];    
};

struct TIM_Element
{
    u8    Element_ID;
    u8    Length;
    u8    DTIM_Count;
    u8    DTIM_Period;
    u8    Bitmap_Control;
    u8    Partial_Virtual_Bitmap[251];
};

struct IBSS_Parameter_Set_Element
{
    u8    Element_ID;
    u8    Length;
    u8    ATIM_Window[2];
};

struct Challenge_Text_Element
{
    u8    Element_ID;
    u8    Length;
    u8    Challenge_Text[253];
};

struct PHY_Parameter_Set_Element
{


    s32     aCCATime;
    s32     aRxTxTurnaroundTime;
    s32     aTxPLCPDelay;
    s32     RxPLCPDelay;
    s32     aRxTxSwitchTime;
    s32     aTxRampOntime;
    s32     aTxRampOffTime;
    s32     aTxRFDelay;
    s32     aRxRFDelay;
    s32     aAirPropagationTime;
    s32     aMACProcessingDelay;
    s32     aPreambleLength;
    s32     aPLCPHeaderLength;
    s32     aMPDUDurationFactor;
    s32     aMPDUMaxLength;


};


struct ERP_Information_Element
{
    u8	Element_ID;
    u8	Length;
    #ifdef _BIG_ENDIAN_ 
    	u8	Reserved:5;   
       u8	Barker_Preamble_Mode:1;
	u8	Use_Protection:1;
       u8	NonERP_Present:1;
    #else
	u8	NonERP_Present:1;
	u8	Use_Protection:1;
	u8	Barker_Preamble_Mode:1;
	u8	Reserved:5;
    #endif
};

struct Extended_Supported_Rates_Element
{
    u8	Element_ID;
    u8	Length;
    u8	ExtendedSupportedRates[255];
}__attribute__ ((packed));


#define VERSION_WPA				1
#ifdef _WPA2_
#define VERSION_WPA2            1
#endif 
#define OUI_WPA					0x00F25000	
#ifdef _WPA2_
#define OUI_WPA2				0x00AC0F00	
#endif 

#define OUI_WPA_ADDITIONAL		0x01
#define WLAN_MIN_RSN_WPA_LENGTH                 6 
#ifdef _WPA2_
#define WLAN_MIN_RSN_WPA2_LENGTH                2 
#endif 

#define oui_wpa                  (u32)(OUI_WPA|OUI_WPA_ADDITIONAL)

#define WPA_OUI_BIG    ((u32) 0x01F25000)
#define WPA_OUI_LITTLE  ((u32) 0x01F25001)

#define WPA_WPS_OUI				cpu_to_le32(0x04F25000) 


#ifdef _WPA2_
#define WPA2_OUI_BIG    ((u32)0x01AC0F00)
#define WPA2_OUI_LITTLE ((u32)0x01AC0F01)
#endif 


#define OUI_AUTH_WPA_NONE           0x00 
#define OUI_AUTH_8021X				0x01
#define OUI_AUTH_PSK				0x02

#define OUI_CIPHER_GROUP_KEY        0x00  
#define OUI_CIPHER_WEP_40			0x01
#define OUI_CIPHER_TKIP				0x02
#define OUI_CIPHER_CCMP				0x04
#define OUI_CIPHER_WEP_104			0x05

typedef struct _SUITE_SELECTOR_
{
	union
	{
		u8	Value[4];
		struct _SUIT_
		{
			u8	OUI[3];
			u8	Type;
		}SuitSelector;
	};
}SUITE_SELECTOR;


struct	RSN_Information_Element
{
	u8					Element_ID;
	u8					Length;
	SUITE_SELECTOR	OuiWPAAdditional;
	u16					Version;
	SUITE_SELECTOR		GroupKeySuite;
	u16					PairwiseKeySuiteCount;
	SUITE_SELECTOR		PairwiseKeySuite[1];
}__attribute__ ((packed));
struct RSN_Auth_Sub_Information_Element
{
	u16				AuthKeyMngtSuiteCount;
	SUITE_SELECTOR	AuthKeyMngtSuite[1];
}__attribute__ ((packed));


struct RSN_Capability_Element
{
  union
  {
	u16	__attribute__ ((packed))	wValue;
    #ifdef _BIG_ENDIAN_	 
    struct _RSN_Capability
    {
    	u16   __attribute__ ((packed))  Reserved2 : 8; 
	u16   __attribute__ ((packed))  Reserved1 : 2;
	u16   __attribute__ ((packed))  GTK_Replay_Counter : 2;
	u16   __attribute__ ((packed))  PTK_Replay_Counter : 2;
	u16   __attribute__ ((packed))  No_Pairwise : 1;
        u16   __attribute__ ((packed))  Pre_Auth : 1;
    }__attribute__ ((packed))  RSN_Capability;
    #else
    struct _RSN_Capability
    {
        u16   __attribute__ ((packed))  Pre_Auth : 1;
        u16   __attribute__ ((packed))  No_Pairwise : 1;
        u16   __attribute__ ((packed))  PTK_Replay_Counter : 2;
	    u16   __attribute__ ((packed))  GTK_Replay_Counter : 2;
	    u16   __attribute__ ((packed))  Reserved1 : 2;
	    u16   __attribute__ ((packed))  Reserved2 : 8; 
    }__attribute__ ((packed))  RSN_Capability;
    #endif

  }__attribute__ ((packed)) ;
}__attribute__ ((packed)) ;

#ifdef _WPA2_
typedef struct _PMKID
{
  u8 pValue[16];
}PMKID;

struct	WPA2_RSN_Information_Element
{
	u8					Element_ID;
	u8					Length;
	u16					Version;
	SUITE_SELECTOR		GroupKeySuite;
	u16					PairwiseKeySuiteCount;
	SUITE_SELECTOR		PairwiseKeySuite[1];

}__attribute__ ((packed));

struct WPA2_RSN_Auth_Sub_Information_Element
{
	u16				AuthKeyMngtSuiteCount;
	SUITE_SELECTOR	AuthKeyMngtSuite[1];
}__attribute__ ((packed));


struct PMKID_Information_Element
{
	u16				PMKID_Count;
	PMKID pmkid [16] ;
}__attribute__ ((packed));

#endif 



struct MAC_frame_control
{
    u8    mac_frame_info; 
    #ifdef _BIG_ENDIAN_ 
    u8    order:1;
    u8    WEP:1;
    u8    more_data:1;
    u8    pwr_mgt:1;
    u8    retry:1;
    u8    more_frag:1;
    u8    from_ds:1;
    u8    to_ds:1;
    #else
    u8    to_ds:1;
    u8    from_ds:1;
    u8    more_frag:1;
    u8    retry:1;
    u8    pwr_mgt:1;
    u8    more_data:1;
    u8    WEP:1;
    u8    order:1;
    #endif
} __attribute__ ((packed));

struct Management_Frame {
    struct MAC_frame_control frame_control; 
    u16		duration;
    u8		DA[MAC_ADDR_LENGTH];			
    u8		SA[MAC_ADDR_LENGTH];			
    u8		BSSID[MAC_ADDR_LENGTH];			
    u16		Sequence_Control;
    
    
}__attribute__ ((packed));


struct Control_Frame {
    struct MAC_frame_control frame_control; 
    u16		duration;
    u8		RA[MAC_ADDR_LENGTH];
    u8		TA[MAC_ADDR_LENGTH];
    u16		FCS;
}__attribute__ ((packed));

struct Data_Frame {
    struct MAC_frame_control frame_control;
    u16		duration;
    u8		Addr1[MAC_ADDR_LENGTH];
    u8		Addr2[MAC_ADDR_LENGTH];
    u8		Addr3[MAC_ADDR_LENGTH];
    u16		Sequence_Control;
    u8		Addr4[MAC_ADDR_LENGTH]; 
    
    
}__attribute__ ((packed));

struct Disassociation_Frame_Body
{
    u16    reasonCode;
}__attribute__ ((packed));

struct Association_Request_Frame_Body
{
    u16    capability_information;
    u16    listenInterval;
    u8     Current_AP_Address[MAC_ADDR_LENGTH];
    
    
}__attribute__ ((packed));

struct Association_Response_Frame_Body
{
    u16    capability_information;
    u16    statusCode;
    u16    Association_ID;
    struct Supported_Rates_Element supportedRates;
}__attribute__ ((packed));




struct Reassociation_Response_Frame_Body
{
    u16    capability_information;
    u16    statusCode;
    u16    Association_ID;
    struct Supported_Rates_Element supportedRates;
}__attribute__ ((packed));

struct Deauthentication_Frame_Body
{
    u16    reasonCode;
}__attribute__ ((packed));


struct Probe_Response_Frame_Body
{
    u16    Timestamp;
    u16    Beacon_Interval;
    u16    Capability_Information;
    
    
    
    
    
}__attribute__ ((packed));

struct Authentication_Frame_Body
{
    u16    algorithmNumber;
    u16    sequenceNumber;
    u16    statusCode;
    
	
}__attribute__ ((packed));


#endif 


