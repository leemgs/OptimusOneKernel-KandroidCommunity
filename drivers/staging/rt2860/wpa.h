

#ifndef	__WPA_H__
#define	__WPA_H__


#define LEN_KEY_DESC_NONCE			32
#define LEN_KEY_DESC_IV				16
#define LEN_KEY_DESC_RSC			8
#define LEN_KEY_DESC_ID				8
#define LEN_KEY_DESC_REPLAY			8
#define LEN_KEY_DESC_MIC			16



#define LEN_EAPOL_KEY_MSG			(sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE)


#define EAP_CODE_REQUEST	1
#define EAP_CODE_RESPONSE	2
#define EAP_CODE_SUCCESS    3
#define EAP_CODE_FAILURE    4


#define	EAPOL_VER					1
#define	EAPOL_VER2					2


#define	WPA1_KEY_DESC				0xfe
#define WPA2_KEY_DESC               0x02


#define	DESC_TYPE_TKIP				1
#define	DESC_TYPE_AES				2
#define DESC_TYPE_MESH				3

#define LEN_MSG1_2WAY               0x7f
#define MAX_LEN_OF_EAP_HS           256

#define LEN_MASTER_KEY				32


#define LEN_EAP_EK					16
#define LEN_EAP_MICK				16
#define LEN_EAP_KEY					((LEN_EAP_EK)+(LEN_EAP_MICK))

#define LEN_PMKID					16
#define LEN_TKIP_EK					16
#define LEN_TKIP_RXMICK				8
#define LEN_TKIP_TXMICK				8
#define LEN_AES_EK					16
#define LEN_AES_KEY					LEN_AES_EK
#define LEN_TKIP_KEY				((LEN_TKIP_EK)+(LEN_TKIP_RXMICK)+(LEN_TKIP_TXMICK))
#define TKIP_AP_TXMICK_OFFSET		((LEN_EAP_KEY)+(LEN_TKIP_EK))
#define TKIP_AP_RXMICK_OFFSET		(TKIP_AP_TXMICK_OFFSET+LEN_TKIP_TXMICK)
#define TKIP_GTK_LENGTH				((LEN_TKIP_EK)+(LEN_TKIP_RXMICK)+(LEN_TKIP_TXMICK))
#define LEN_PTK						((LEN_EAP_KEY)+(LEN_TKIP_KEY))


#define MAX_LEN_OF_RSNIE         	90
#define MIN_LEN_OF_RSNIE         	8


#define	EAPPacket		0
#define	EAPOLStart		1
#define	EAPOLLogoff		2
#define	EAPOLKey		3
#define	EAPOLASFAlert	4
#define	EAPTtypeMax		5

#define	EAPOL_MSG_INVALID	0
#define	EAPOL_PAIR_MSG_1	1
#define	EAPOL_PAIR_MSG_2	2
#define	EAPOL_PAIR_MSG_3	3
#define	EAPOL_PAIR_MSG_4	4
#define	EAPOL_GROUP_MSG_1	5
#define	EAPOL_GROUP_MSG_2	6

#define PAIRWISEKEY					1
#define GROUPKEY					0


#define PEER_MSG1_RETRY_TIMER_CTR           0
#define PEER_MSG3_RETRY_TIMER_CTR           10
#define GROUP_MSG1_RETRY_TIMER_CTR          20


#define EAPOL_START_DISABLE					0
#define EAPOL_START_PSK						1
#define EAPOL_START_1X						2

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)

#define ROUND_UP(__x, __y) \
	(((ULONG)((__x)+((__y)-1))) & ((ULONG)~((__y)-1)))

#define	ADD_ONE_To_64BIT_VAR(_V)		\
{										\
	UCHAR	cnt = LEN_KEY_DESC_REPLAY;	\
	do									\
	{									\
		cnt--;							\
		_V[cnt]++;						\
		if (cnt == 0)					\
			break;						\
	}while (_V[cnt] == 0);				\
}

#define IS_WPA_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))


typedef	struct PACKED _KEY_INFO
{
	UCHAR	KeyMic:1;
	UCHAR	Secure:1;
	UCHAR	Error:1;
	UCHAR	Request:1;
	UCHAR	EKD_DL:1;       
	UCHAR	Rsvd:3;
	UCHAR	KeyDescVer:3;
	UCHAR	KeyType:1;
	UCHAR	KeyIndex:2;
	UCHAR	Install:1;
	UCHAR	KeyAck:1;
}	KEY_INFO, *PKEY_INFO;


typedef	struct PACKED _KEY_DESCRIPTER
{
	UCHAR		Type;
	KEY_INFO	KeyInfo;
	UCHAR		KeyLength[2];
	UCHAR		ReplayCounter[LEN_KEY_DESC_REPLAY];
	UCHAR		KeyNonce[LEN_KEY_DESC_NONCE];
	UCHAR		KeyIv[LEN_KEY_DESC_IV];
	UCHAR		KeyRsc[LEN_KEY_DESC_RSC];
	UCHAR		KeyId[LEN_KEY_DESC_ID];
	UCHAR		KeyMic[LEN_KEY_DESC_MIC];
	UCHAR		KeyDataLen[2];
	UCHAR		KeyData[MAX_LEN_OF_RSNIE];
}	KEY_DESCRIPTER, *PKEY_DESCRIPTER;

typedef	struct PACKED _EAPOL_PACKET
{
	UCHAR	 			ProVer;
	UCHAR	 			ProType;
	UCHAR	 			Body_Len[2];
	KEY_DESCRIPTER		KeyDesc;
}	EAPOL_PACKET, *PEAPOL_PACKET;


typedef struct PACKED _GTK_ENCAP
{
    UCHAR               Kid:2;
    UCHAR               tx:1;
    UCHAR               rsv:5;
    UCHAR               rsv1;
    UCHAR               GTK[TKIP_GTK_LENGTH];
}   GTK_ENCAP, *PGTK_ENCAP;

typedef struct PACKED _KDE_ENCAP
{
    UCHAR               Type;
    UCHAR               Len;
    UCHAR               OUI[3];
    UCHAR               DataType;
    GTK_ENCAP      GTKEncap;
}   KDE_ENCAP, *PKDE_ENCAP;


typedef struct PACKED _RSNIE {
    UCHAR   oui[4];
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct PACKED {
        UCHAR oui[4];
    }ucast[1];
} RSNIE, *PRSNIE;


typedef struct PACKED _RSNIE2 {
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct PACKED {
        UCHAR oui[4];
    }ucast[1];
} RSNIE2, *PRSNIE2;


typedef struct PACKED _RSNIE_AUTH {
    USHORT acount;
    struct PACKED {
        UCHAR oui[4];
    }auth[1];
} RSNIE_AUTH,*PRSNIE_AUTH;

typedef	union PACKED _RSN_CAPABILITIES	{
	struct	PACKED {
        USHORT		PreAuth:1;
		USHORT		No_Pairwise:1;
		USHORT		PTKSA_R_Counter:2;
		USHORT		GTKSA_R_Counter:2;
		USHORT		Rsvd:10;
	}	field;
	USHORT			word;
}	RSN_CAPABILITIES, *PRSN_CAPABILITIES;

typedef struct PACKED _EAP_HDR {
    UCHAR   ProVer;
    UCHAR   ProType;
    UCHAR   Body_Len[2];
    UCHAR   code;
    UCHAR   identifier;
    UCHAR   length[2]; 
} EAP_HDR, *PEAP_HDR;



typedef	enum	_WpaState
{
	SS_NOTUSE,				
	SS_START,				
	SS_WAIT_MSG_3,			
	SS_WAIT_GROUP,			
	SS_FINISH,  			
	SS_KEYUPDATE,			
}	WPA_STATE;


















typedef	enum	_WpaMixPairCipher
{
	MIX_CIPHER_NOTUSE 			= 0x00,
	WPA_NONE_WPA2_TKIPAES		= 0x03,		
	WPA_AES_WPA2_TKIP 			= 0x06,
	WPA_AES_WPA2_TKIPAES		= 0x07,
	WPA_TKIP_WPA2_AES			= 0x09,
	WPA_TKIP_WPA2_TKIPAES		= 0x0B,
	WPA_TKIPAES_WPA2_NONE		= 0x0C,		
	WPA_TKIPAES_WPA2_AES		= 0x0D,
	WPA_TKIPAES_WPA2_TKIP		= 0x0E,
	WPA_TKIPAES_WPA2_TKIPAES	= 0x0F,
}	WPA_MIX_PAIR_CIPHER;

typedef struct PACKED _RSN_IE_HEADER_STRUCT	{
	UCHAR		Eid;
	UCHAR		Length;
	USHORT		Version;	
}	RSN_IE_HEADER_STRUCT, *PRSN_IE_HEADER_STRUCT;


typedef struct PACKED _CIPHER_SUITE_STRUCT	{
	UCHAR		Oui[3];
	UCHAR		Type;
}	CIPHER_SUITE_STRUCT, *PCIPHER_SUITE_STRUCT;


typedef struct PACKED _AKM_SUITE_STRUCT	{
	UCHAR		Oui[3];
	UCHAR		Type;
}	AKM_SUITE_STRUCT, *PAKM_SUITE_STRUCT;


typedef struct	PACKED _RSN_CAPABILITY	{
	USHORT		Rsv:10;
	USHORT		GTKSAReplayCnt:2;
	USHORT		PTKSAReplayCnt:2;
	USHORT		NoPairwise:1;
	USHORT		PreAuth:1;
}	RSN_CAPABILITY, *PRSN_CAPABILITY;

#endif
