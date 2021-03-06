#ifndef LG_DIAGCMD_H
#define LG_DIAGCMD_H





#define DIAG_VERNO_F    0


#define DIAG_ESN_F      1


#define DIAG_PEEKB_F    2
  

#define DIAG_PEEKW_F    3


#define DIAG_PEEKD_F    4  


#define DIAG_POKEB_F    5  


#define DIAG_POKEW_F    6  


#define DIAG_POKED_F    7  


#define DIAG_OUTP_F     8


#define DIAG_OUTPW_F    9  


#define DIAG_INP_F      10 


#define DIAG_INPW_F     11 


#define DIAG_STATUS_F   12 




#define DIAG_LOGMASK_F  15 


#define DIAG_LOG_F      16 


#define DIAG_NV_PEEK_F  17 


#define DIAG_NV_POKE_F  18 


#define DIAG_BAD_CMD_F  19 


#define DIAG_BAD_PARM_F 20 


#define DIAG_BAD_LEN_F  21 




#define DIAG_BAD_MODE_F     24
                            

#define DIAG_TAGRAPH_F      25 


#define DIAG_MARKOV_F       26 


#define DIAG_MARKOV_RESET_F 27 


#define DIAG_DIAG_VER_F     28 
                            

#define DIAG_TS_F           29 


#define DIAG_TA_PARM_F      30 


#define DIAG_MSG_F          31 


#define DIAG_HS_KEY_F       32 


#define DIAG_HS_LOCK_F      33 


#define DIAG_HS_SCREEN_F    34 




#define DIAG_PARM_SET_F     36 




#define DIAG_NV_READ_F  38 

#define DIAG_NV_WRITE_F 39 



#define DIAG_CONTROL_F    41 


#define DIAG_ERR_READ_F   42 


#define DIAG_ERR_CLEAR_F  43 


#define DIAG_SER_RESET_F  44 


#define DIAG_SER_REPORT_F 45 


#define DIAG_TEST_F       46 


#define DIAG_GET_DIPSW_F  47 


#define DIAG_SET_DIPSW_F  48 


#define DIAG_VOC_PCM_LB_F 49 


#define DIAG_VOC_PKT_LB_F 50 




#define DIAG_ORIG_F 53 

#define DIAG_END_F  54 



#define DIAG_DLOAD_F 58 

#define DIAG_TMOB_F  59 

#define DIAG_FTM_CMD_F  59 


#ifdef FEATURE_HWTC
#define DIAG_TEST_STATE_F 61
#endif 


#define DIAG_STATE_F        63 


#define DIAG_PILOT_SETS_F   64 


#define DIAG_SPC_F          65 


#define DIAG_BAD_SPC_MODE_F 66 


#define DIAG_PARM_GET2_F    67 


#define DIAG_SERIAL_CHG_F   68 




#define DIAG_PASSWORD_F     70 


#define DIAG_BAD_SEC_MODE_F 71 


#define DIAG_PR_LIST_WR_F   72 


#define DIAG_PR_LIST_RD_F   73 




#define DIAG_SUBSYS_CMD_F   75 




#define DIAG_FEATURE_QUERY_F   81 




#define DIAG_SMS_READ_F        83 


#define DIAG_SMS_WRITE_F       84 


#define DIAG_SUP_FER_F         85 


#define DIAG_SUP_WALSH_CODES_F 86 


#define DIAG_SET_MAX_SUP_CH_F  87 


#define DIAG_PARM_GET_IS95B_F  88 


#define DIAG_FS_OP_F           89 


#define DIAG_AKEY_VERIFY_F     90 


#define DIAG_BMP_HS_SCREEN_F   91 


#define DIAG_CONFIG_COMM_F        92 


#define DIAG_EXT_LOGMASK_F        93 




#define DIAG_EVENT_REPORT_F       96 


#define DIAG_STREAMING_CONFIG_F   97 


#define DIAG_PARM_RETRIEVE_F      98 

 
#define DIAG_STATUS_SNAPSHOT_F    99
 

#define DIAG_RPC_F               100 


#define DIAG_GET_PROPERTY_F      101 


#define DIAG_PUT_PROPERTY_F      102 


#define DIAG_GET_GUID_F          103 


#define DIAG_USER_CMD_F          104 


#define DIAG_GET_PERM_PROPERTY_F 105 


#define DIAG_PUT_PERM_PROPERTY_F 106 


#define DIAG_PERM_USER_CMD_F     107 


#define DIAG_GPS_SESS_CTRL_F     108 


#define DIAG_GPS_GRID_F          109 


#define DIAG_GPS_STATISTICS_F    110 


#define DIAG_ROUTE_F             111 


#define DIAG_IS2000_STATUS_F     112


#define DIAG_RLP_STAT_RESET_F    113


#define DIAG_TDSO_STAT_RESET_F   114


#define DIAG_LOG_CONFIG_F        115


#define DIAG_TRACE_EVENT_REPORT_F 116


#define DIAG_SBI_READ_F           117


#define DIAG_SBI_WRITE_F          118


#define DIAG_SSD_VERIFY_F         119


#define DIAG_LOG_ON_DEMAND_F      120


#define DIAG_EXT_MSG_F            121 


#define DIAG_ONCRPC_F             122


#define DIAG_PROTOCOL_LOOPBACK_F  123


#define DIAG_EXT_BUILD_ID_F       124


#define DIAG_EXT_MSG_CONFIG_F     125


#define DIAG_EXT_MSG_TERSE_F      126


#define DIAG_EXT_MSG_TERSE_XLATE_F 127


#define DIAG_SUBSYS_CMD_VER_2_F    128


#define DIAG_EVENT_MASK_GET_F      129


#define DIAG_EVENT_MASK_SET_F      130




#define DIAG_CHANGE_PORT_SETTINGS  140


#define DIAG_CNTRY_INFO_F          141


#define DIAG_SUPS_REQ_F            142


#define DIAG_MMS_ORIG_SMS_REQUEST_F 143


#define DIAG_MEAS_MODE_F           144


#define DIAG_MEAS_REQ_F            145


#define DIAG_QSR_EXT_MSG_TERSE_F   146


#define DIAG_LGF_SCREEN_SHOT_F     150



#if 1	
#define DIAG_MTC_F              240
#endif 



#ifdef CONFIG_LGE_DIAG_WMC
#define DIAG_WMCSYNC_MAPPING_F 	241
#endif

#define DIAG_WIFI_MAC_ADDR 214


#define DIAG_TEST_MODE_F          250  
#define DIAG_LCD_Q_TEST_F         253

#define DIAG_MAX_F                 255

typedef enum {
  DIAG_SUBSYS_OEM                = 0,       
  DIAG_SUBSYS_ZREX               = 1,       
  DIAG_SUBSYS_SD                 = 2,       
  DIAG_SUBSYS_BT                 = 3,       
  DIAG_SUBSYS_WCDMA              = 4,       
  DIAG_SUBSYS_HDR                = 5,       
  DIAG_SUBSYS_DIABLO             = 6,       
  DIAG_SUBSYS_TREX               = 7,       
  DIAG_SUBSYS_GSM                = 8,       
  DIAG_SUBSYS_UMTS               = 9,       
  DIAG_SUBSYS_HWTC               = 10,      
  DIAG_SUBSYS_FTM                = 11,      
  DIAG_SUBSYS_REX                = 12,      
  DIAG_SUBSYS_OS                 = DIAG_SUBSYS_REX,
  DIAG_SUBSYS_GPS                = 13,      
  DIAG_SUBSYS_WMS                = 14,      
  DIAG_SUBSYS_CM                 = 15,      
  DIAG_SUBSYS_HS                 = 16,      
  DIAG_SUBSYS_AUDIO_SETTINGS     = 17,      
  DIAG_SUBSYS_DIAG_SERV          = 18,      
  DIAG_SUBSYS_FS                 = 19,      
  DIAG_SUBSYS_PORT_MAP_SETTINGS  = 20,      
  DIAG_SUBSYS_MEDIAPLAYER        = 21,      
  DIAG_SUBSYS_QCAMERA            = 22,      
  DIAG_SUBSYS_MOBIMON            = 23,      
  DIAG_SUBSYS_GUNIMON            = 24,      
  DIAG_SUBSYS_LSM                = 25,      
  DIAG_SUBSYS_QCAMCORDER         = 26,      
  DIAG_SUBSYS_MUX1X              = 27,      
  DIAG_SUBSYS_DATA1X             = 28,      
  DIAG_SUBSYS_SRCH1X             = 29,      
  DIAG_SUBSYS_CALLP1X            = 30,      
  DIAG_SUBSYS_APPS               = 31,      
  DIAG_SUBSYS_SETTINGS           = 32,      
  DIAG_SUBSYS_GSDI               = 33,      
  DIAG_SUBSYS_UIMDIAG            = DIAG_SUBSYS_GSDI,
  DIAG_SUBSYS_TMC                = 34,      
  DIAG_SUBSYS_USB                = 35,      
  DIAG_SUBSYS_PM                 = 36,      
  DIAG_SUBSYS_DEBUG              = 37,
  DIAG_SUBSYS_QTV                = 38,
  DIAG_SUBSYS_CLKRGM             = 39,      
  DIAG_SUBSYS_DEVICES            = 40,
  DIAG_SUBSYS_WLAN               = 41,      
  DIAG_SUBSYS_PS_DATA_LOGGING    = 42,      
  DIAG_SUBSYS_PS                 = DIAG_SUBSYS_PS_DATA_LOGGING,
  DIAG_SUBSYS_MFLO               = 43,      
  DIAG_SUBSYS_DTV                = 44,      
  DIAG_SUBSYS_RRC                = 45,      
  DIAG_SUBSYS_PROF               = 46,      
  DIAG_SUBSYS_TCXOMGR            = 47,
  DIAG_SUBSYS_NV                 = 48,      
  DIAG_SUBSYS_AUTOCONFIG         = 49,
  DIAG_SUBSYS_PARAMS             = 50,      
  DIAG_SUBSYS_MDDI               = 51,      
  DIAG_SUBSYS_DS_ATCOP           = 52,
  DIAG_SUBSYS_L4LINUX            = 53,      
  DIAG_SUBSYS_MVS                = 54,      
  DIAG_SUBSYS_CNV                = 55,      
  DIAG_SUBSYS_APIONE_PROGRAM     = 56,      
  DIAG_SUBSYS_HIT                = 57,      
  DIAG_SUBSYS_DRM                = 58,      
  DIAG_SUBSYS_DM                 = 59,      
  DIAG_SUBSYS_FC                 = 60,      
  DIAG_SUBSYS_MEMORY             = 61,      
  DIAG_SUBSYS_FS_ALTERNATE       = 62,      
  DIAG_SUBSYS_REGRESSION         = 63,      
  DIAG_SUBSYS_SENSORS            = 64,      
  DIAG_SUBSYS_FLUTE              = 65,      
  DIAG_SUBSYS_ANALOG             = 66,      
  DIAG_SUBSYS_APIONE_PROGRAM_MODEM = 67,    
  DIAG_SUBSYS_LTE                = 68,      
  DIAG_SUBSYS_BREW               = 69,      
  DIAG_SUBSYS_PWRDB              = 70,      
  DIAG_SUBSYS_CHORD              = 71,      
  DIAG_SUBSYS_SEC                = 72,      
  DIAG_SUBSYS_TIME               = 73,      
  DIAG_SUBSYS_Q6_CORE            = 74,		  

  DIAG_SUBSYS_LAST,

  
  DIAG_SUBSYS_RESERVED_OEM_0     = 250,
  DIAG_SUBSYS_RESERVED_OEM_1     = 251,
  DIAG_SUBSYS_RESERVED_OEM_2     = 252,
  DIAG_SUBSYS_RESERVED_OEM_3     = 253,
  DIAG_SUBSYS_RESERVED_OEM_4     = 254,
  DIAG_SUBSYS_LEGACY             = 255
} diagpkt_subsys_cmd_enum_type;

#define LG_DIAG_CMD_LINE_LEN 256

struct lg_diag_cmd_dev {
	char	*name;
	struct device *dev;
	int 	index;
	int 	state;
};
#endif 
