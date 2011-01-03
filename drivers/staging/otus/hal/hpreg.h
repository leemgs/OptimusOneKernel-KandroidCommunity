










#ifndef _HPREG_H
#define _HPREG_H

typedef u16_t HAL_CTRY_CODE;		
typedef u16_t HAL_REG_DOMAIN;		
typedef enum {
	AH_FALSE = 0,		
	AH_TRUE  = 1,
} HAL_BOOL;



enum CountryCode {
    CTRY_ALBANIA              = 8,       
    CTRY_ALGERIA              = 12,      
    CTRY_ARGENTINA            = 32,      
    CTRY_ARMENIA              = 51,      
    CTRY_AUSTRALIA            = 36,      
    CTRY_AUSTRIA              = 40,      
    CTRY_AZERBAIJAN           = 31,      
    CTRY_BAHRAIN              = 48,      
    CTRY_BELARUS              = 112,     
    CTRY_BELGIUM              = 56,      
    CTRY_BELIZE               = 84,      
    CTRY_BOLIVIA              = 68,      
    CTRY_BOSNIA               = 70,      
    CTRY_BRAZIL               = 76,      
    CTRY_BRUNEI_DARUSSALAM    = 96,      
    CTRY_BULGARIA             = 100,     
    CTRY_CANADA               = 124,     
    CTRY_CHILE                = 152,     
    CTRY_CHINA                = 156,     
    CTRY_COLOMBIA             = 170,     
    CTRY_COSTA_RICA           = 188,     
    CTRY_CROATIA              = 191,     
    CTRY_CYPRUS               = 196,     
    CTRY_CZECH                = 203,     
    CTRY_DENMARK              = 208,     
    CTRY_DOMINICAN_REPUBLIC   = 214,     
    CTRY_ECUADOR              = 218,     
    CTRY_EGYPT                = 818,     
    CTRY_EL_SALVADOR          = 222,     
    CTRY_ESTONIA              = 233,     
    CTRY_FAEROE_ISLANDS       = 234,     
    CTRY_FINLAND              = 246,     
    CTRY_FRANCE               = 250,     
    CTRY_FRANCE2              = 255,     
    CTRY_GEORGIA              = 268,     
    CTRY_GERMANY              = 276,     
    CTRY_GREECE               = 300,     
    CTRY_GUATEMALA            = 320,     
    CTRY_HONDURAS             = 340,     
    CTRY_HONG_KONG            = 344,     
    CTRY_HUNGARY              = 348,     
    CTRY_ICELAND              = 352,     
    CTRY_INDIA                = 356,     
    CTRY_INDONESIA            = 360,     
    CTRY_IRAN                 = 364,     
    CTRY_IRAQ                 = 368,     
    CTRY_IRELAND              = 372,     
    CTRY_ISRAEL               = 376,     
    CTRY_ISRAEL2              = 377,     
    CTRY_ITALY                = 380,     
    CTRY_JAMAICA              = 388,     
    CTRY_JAPAN                = 392,     
    CTRY_JAPAN1               = 393,     
    CTRY_JAPAN2               = 394,     
    CTRY_JAPAN3               = 395,     
    CTRY_JAPAN4               = 396,     
    CTRY_JAPAN5               = 397,     
    CTRY_JAPAN6               = 399,     

    CTRY_JAPAN7	              = 4007,    
    CTRY_JAPAN8	              = 4008,    
    CTRY_JAPAN9               = 4009,    

    CTRY_JAPAN10              = 4010,    
    CTRY_JAPAN11              = 4011,    
    CTRY_JAPAN12              = 4012,    

    CTRY_JAPAN13              = 4013,    
    CTRY_JAPAN14              = 4014,    
    CTRY_JAPAN15              = 4015,    

    CTRY_JAPAN16              = 4016,    
    CTRY_JAPAN17              = 4017,    
    CTRY_JAPAN18              = 4018,    

    CTRY_JAPAN19              = 4019,    
    CTRY_JAPAN20              = 4020,    
    CTRY_JAPAN21              = 4021,    

    CTRY_JAPAN22              = 4022,    
    CTRY_JAPAN23              = 4023,    
    CTRY_JAPAN24              = 4024,    

    CTRY_JAPAN25              = 4025,    
    CTRY_JAPAN26              = 4026,    
    CTRY_JAPAN27              = 4027,    

    CTRY_JAPAN28              = 4028,    
    CTRY_JAPAN29              = 4029,    
    CTRY_JAPAN30              = 4030,    

    CTRY_JAPAN31              = 4031,    
    CTRY_JAPAN32              = 4032,    
    CTRY_JAPAN33              = 4033,    

    CTRY_JAPAN34              = 4034,    
    CTRY_JAPAN35              = 4035,    
    CTRY_JAPAN36              = 4036,    

    CTRY_JAPAN37              = 4037,    
    CTRY_JAPAN38              = 4038,    
    CTRY_JAPAN39              = 4039,    

    CTRY_JAPAN40              = 4040,    
    CTRY_JAPAN41              = 4041,    
    CTRY_JAPAN42              = 4042,    
    CTRY_JAPAN43              = 4043,    
    CTRY_JAPAN44              = 4044,    
    CTRY_JAPAN45              = 4045,    
    CTRY_JAPAN46              = 4046,    
    CTRY_JAPAN47              = 4047,    
    CTRY_JAPAN48              = 4048,    
    CTRY_JAPAN49              = 4049,    

    CTRY_JAPAN50              = 4050,    
    CTRY_JAPAN51              = 4051,    
    CTRY_JAPAN52              = 4052,    
    CTRY_JAPAN53              = 4053,    
    CTRY_JAPAN54              = 4054,    

    CTRY_JORDAN               = 400,     
    CTRY_KAZAKHSTAN           = 398,     
    CTRY_KENYA                = 404,     
    CTRY_KOREA_NORTH          = 408,     
    CTRY_KOREA_ROC            = 410,     
    CTRY_KOREA_ROC2           = 411,     
    CTRY_KOREA_ROC3           = 412,     
    CTRY_KUWAIT               = 414,     
    CTRY_LATVIA               = 428,     
    CTRY_LEBANON              = 422,     
    CTRY_LIBYA                = 434,     
    CTRY_LIECHTENSTEIN        = 438,     
    CTRY_LITHUANIA            = 440,     
    CTRY_LUXEMBOURG           = 442,     
    CTRY_MACAU                = 446,     
    CTRY_MACEDONIA            = 807,     
    CTRY_MALAYSIA             = 458,     
    CTRY_MALTA	              = 470,     
    CTRY_MEXICO               = 484,     
    CTRY_MONACO               = 492,     
    CTRY_MOROCCO              = 504,     
    CTRY_NETHERLANDS          = 528,     
    CTRY_NETHERLANDS_ANT      = 530,     
    CTRY_NEW_ZEALAND          = 554,     
    CTRY_NICARAGUA            = 558,     
    CTRY_NORWAY               = 578,     
    CTRY_OMAN                 = 512,     
    CTRY_PAKISTAN             = 586,     
    CTRY_PANAMA               = 591,     
    CTRY_PARAGUAY             = 600,     
    CTRY_PERU                 = 604,     
    CTRY_PHILIPPINES          = 608,     
    CTRY_POLAND               = 616,     
    CTRY_PORTUGAL             = 620,     
    CTRY_PUERTO_RICO          = 630,     
    CTRY_QATAR                = 634,     
    CTRY_ROMANIA              = 642,     
    CTRY_RUSSIA               = 643,     
    CTRY_SAUDI_ARABIA         = 682,     
    CTRY_SERBIA_MONT          = 891,     
    CTRY_SINGAPORE            = 702,     
    CTRY_SLOVAKIA             = 703,     
    CTRY_SLOVENIA             = 705,     
    CTRY_SOUTH_AFRICA         = 710,     
    CTRY_SPAIN                = 724,     
    CTRY_SRILANKA             = 144,     
    CTRY_SWEDEN               = 752,     
    CTRY_SWITZERLAND          = 756,     
    CTRY_SYRIA                = 760,     
    CTRY_TAIWAN               = 158,     
    CTRY_THAILAND             = 764,     
    CTRY_TRINIDAD_Y_TOBAGO    = 780,     
    CTRY_TUNISIA              = 788,     
    CTRY_TURKEY               = 792,     
    CTRY_UAE                  = 784,     
    CTRY_UKRAINE              = 804,     
    CTRY_UNITED_KINGDOM       = 826,     
    CTRY_UNITED_STATES        = 840,     
    CTRY_UNITED_STATES_FCC49  = 842,     
    CTRY_URUGUAY              = 858,     
    CTRY_UZBEKISTAN           = 860,     
    CTRY_VENEZUELA            = 862,     
    CTRY_VIET_NAM             = 704,     
    CTRY_YEMEN                = 887,     
    CTRY_ZIMBABWE             = 716      
};


enum EnumRd {
	
	NO_ENUMRD	= 0x00,
	NULL1_WORLD	= 0x03,		
	NULL1_ETSIB	= 0x07,		
	NULL1_ETSIC	= 0x08,
	FCC1_FCCA	= 0x10,		
	FCC1_WORLD	= 0x11,		
	FCC4_FCCA	= 0x12,		
	FCC5_FCCA	= 0x13,		
  FCC6_FCCA       = 0x14,         

	FCC2_FCCA	= 0x20,		
	FCC2_WORLD	= 0x21,		
	FCC2_ETSIC	= 0x22,
  FCC6_WORLD      = 0x23,         

	FRANCE_RES	= 0x31,		
	FCC3_FCCA	= 0x3A,		
	FCC3_WORLD	= 0x3B,		

	ETSI1_WORLD	= 0x37,
	ETSI3_ETSIA	= 0x32,		
	ETSI2_WORLD	= 0x35,		
	ETSI3_WORLD	= 0x36,		
	ETSI4_WORLD	= 0x30,
	ETSI4_ETSIC	= 0x38,
	ETSI5_WORLD	= 0x39,
	ETSI6_WORLD	= 0x34,		
	ETSI_RESERVED	= 0x33,		

	MKK1_MKKA	= 0x40,		
	MKK1_MKKB	= 0x41,		
	APL4_WORLD	= 0x42,		
	MKK2_MKKA	= 0x43,		
	APL_RESERVED	= 0x44,		
	APL2_WORLD	= 0x45,		
	APL2_APLC	= 0x46,
	APL3_WORLD	= 0x47,
	MKK1_FCCA	= 0x48,		
	APL2_APLD	= 0x49,		
	MKK1_MKKA1	= 0x4A,		
	MKK1_MKKA2	= 0x4B,		
	MKK1_MKKC	= 0x4C,		

	APL3_FCCA   = 0x50,
	APL1_WORLD	= 0x52,		
	APL1_FCCA	= 0x53,
	APL1_APLA	= 0x54,
	APL1_ETSIC	= 0x55,
	APL2_ETSIC	= 0x56,		
	APL2_FCCA   = 0x57, 	
	APL5_WORLD	= 0x58,		
	APL6_WORLD	= 0x5B,		
	APL7_FCCA   = 0x5C,     
	APL8_WORLD  = 0x5D,     
	APL9_WORLD  = 0x5E,     

	
	WOR0_WORLD	= 0x60,		
	WOR1_WORLD	= 0x61,		
	WOR2_WORLD	= 0x62,		
	WOR3_WORLD	= 0x63,		
	WOR4_WORLD	= 0x64,		
	WOR5_ETSIC	= 0x65,		

	WOR01_WORLD	= 0x66,		
	WOR02_WORLD	= 0x67,		
	EU1_WORLD	= 0x68,		

	WOR9_WORLD	= 0x69,		
	WORA_WORLD	= 0x6A,		

	MKK3_MKKB	= 0x80,		
	MKK3_MKKA2	= 0x81,		
	MKK3_MKKC	= 0x82,		

	MKK4_MKKB	= 0x83,		
	MKK4_MKKA2	= 0x84,		
	MKK4_MKKC	= 0x85,		

	MKK5_MKKB	= 0x86,		
	MKK5_MKKA2	= 0x87,		
	MKK5_MKKC	= 0x88,		

	MKK6_MKKB	= 0x89,		
	MKK6_MKKA2	= 0x8A,		
	MKK6_MKKC	= 0x8B,		

	MKK7_MKKB	= 0x8C,		
	MKK7_MKKA	= 0x8D,		
	MKK7_MKKC	= 0x8E,		

	MKK8_MKKB	= 0x8F,		
	MKK8_MKKA2	= 0x90,		
	MKK8_MKKC	= 0x91,		

	MKK6_MKKA1      = 0xF8,         
	MKK6_FCCA       = 0xF9,         
	MKK7_MKKA1      = 0xFA,         
	MKK7_FCCA       = 0xFB,         
	MKK9_FCCA       = 0xFC,         
	MKK9_MKKA1      = 0xFD,         
	MKK9_MKKC       = 0xFE,         
	MKK9_MKKA2      = 0xFF,         

	MKK10_FCCA      = 0xD0,         
	MKK10_MKKA1     = 0xD1,         
	MKK10_MKKC      = 0xD2,         
	MKK10_MKKA2     = 0xD3,         

	MKK11_MKKA      = 0xD4,         
	MKK11_FCCA      = 0xD5,         
	MKK11_MKKA1     = 0xD6,         
	MKK11_MKKC      = 0xD7,         
	MKK11_MKKA2     = 0xD8,         

	MKK12_MKKA      = 0xD9,         
	MKK12_FCCA      = 0xDA,         
	MKK12_MKKA1     = 0xDB,         
	MKK12_MKKC      = 0xDC,         
	MKK12_MKKA2     = 0xDD,         

	
	MKK3_MKKA       = 0xF0,         
	MKK3_MKKA1      = 0xF1,         
	MKK3_FCCA       = 0xF2,         
	MKK4_MKKA       = 0xF3,         
	MKK4_MKKA1      = 0xF4,         
	MKK4_FCCA       = 0xF5,         
	MKK9_MKKA       = 0xF6,         
	MKK10_MKKA      = 0xF7,         

	
	APL1		= 0x0150,	
	APL2		= 0x0250,	
	APL3		= 0x0350,	
	APL4		= 0x0450,	
	APL5		= 0x0550,	
	APL6		= 0x0650,	
	APL7		= 0x0750,	
	APL8		= 0x0850,	
	APL9		= 0x0950,	

	ETSI1		= 0x0130,	
	ETSI2		= 0x0230,	
	ETSI3		= 0x0330,	
	ETSI4		= 0x0430,	
	ETSI5		= 0x0530,	
	ETSI6		= 0x0630,	
	ETSIA		= 0x0A30,	
	ETSIB		= 0x0B30,	
	ETSIC		= 0x0C30,	

	FCC1		= 0x0110,	
	FCC2		= 0x0120,	
	FCC3		= 0x0160,	
	FCC4		= 0x0165,	
	FCC5		= 0x0510,	
    FCC6    = 0x0610, 

	FCCA		= 0x0A10,

	APLD		= 0x0D50,	

	MKK1		= 0x0140,	
	MKK2		= 0x0240,	
	MKK3		= 0x0340,	
	MKK4		= 0x0440,	
	MKK5		= 0x0540,	
	MKK6		= 0x0640,	
	MKK7		= 0x0740,	
	MKK8		= 0x0840,	
	MKK9		= 0x0940,   
	MKK10		= 0x0B40,   
	MKK11		= 0x1140,   
	MKK12		= 0x1240,   
	MKKA		= 0x0A40,	
	MKKC		= 0x0A50,

	NULL1		= 0x0198,
	WORLD		= 0x0199,
	DEBUG_REG_DMN	= 0x01ff,
};


#define	ZM_REG_FLAG_CHANNEL_CW_INT	0x0002	
#define	ZM_REG_FLAG_CHANNEL_TURBO	0x0010	
#define	ZM_REG_FLAG_CHANNEL_CCK	    0x0020	
#define	ZM_REG_FLAG_CHANNEL_OFDM	0x0040	
#define	ZM_REG_FLAG_CHANNEL_2GHZ	0x0080	
#define	ZM_REG_FLAG_CHANNEL_5GHZ	0x0100	
#define	ZM_REG_FLAG_CHANNEL_PASSIVE	0x0200	
#define	ZM_REG_FLAG_CHANNEL_DYN	    0x0400	
#define	ZM_REG_FLAG_CHANNEL_XR	    0x0800	
#define	ZM_REG_FLAG_CHANNEL_CSA 	0x1000	
#define	ZM_REG_FLAG_CHANNEL_STURBO	0x2000	
#define ZM_REG_FLAG_CHANNEL_HALF    0x4000 	
#define ZM_REG_FLAG_CHANNEL_QUARTER 0x8000 	


#define CHANNEL_CW_INT  0x0002  
#define CHANNEL_TURBO   0x0010  
#define CHANNEL_CCK 0x0020  
#define CHANNEL_OFDM    0x0040  
#define CHANNEL_2GHZ    0x0080  
#define CHANNEL_5GHZ    0x0100  
#define CHANNEL_PASSIVE 0x0200  
#define CHANNEL_DYN 0x0400  
#define CHANNEL_XR  0x0800  
#define CHANNEL_STURBO  0x2000  
#define CHANNEL_HALF    0x4000  
#define CHANNEL_QUARTER 0x8000  
#define CHANNEL_HT20    0x10000 
#define CHANNEL_HT40    0x20000 
#define CHANNEL_HT40U 	0x40000 
#define CHANNEL_HT40L 	0x80000 


#define ZM_REG_FLAG_CHANNEL_INTERFERENCE   	0x01 
#define ZM_REG_FLAG_CHANNEL_DFS		0x02 
#define ZM_REG_FLAG_CHANNEL_4MS_LIMIT	0x04 
#define ZM_REG_FLAG_CHANNEL_DFS_CLEAR       0x08 

#define CHANNEL_A   (CHANNEL_5GHZ|CHANNEL_OFDM)
#define CHANNEL_B   (CHANNEL_2GHZ|CHANNEL_CCK)
#define CHANNEL_PUREG   (CHANNEL_2GHZ|CHANNEL_OFDM)
#ifdef notdef
#define CHANNEL_G   (CHANNEL_2GHZ|CHANNEL_DYN)
#else
#define CHANNEL_G   (CHANNEL_2GHZ|CHANNEL_OFDM)
#endif
#define CHANNEL_T   (CHANNEL_5GHZ|CHANNEL_OFDM|CHANNEL_TURBO)
#define CHANNEL_ST  (CHANNEL_T|CHANNEL_STURBO)
#define CHANNEL_108G    (CHANNEL_2GHZ|CHANNEL_OFDM|CHANNEL_TURBO)
#define CHANNEL_108A    CHANNEL_T
#define CHANNEL_X   (CHANNEL_5GHZ|CHANNEL_OFDM|CHANNEL_XR)
#define CHANNEL_G_HT      (CHANNEL_2GHZ | CHANNEL_OFDM | CHANNEL_HT20)
#define CHANNEL_A_HT      (CHANNEL_5GHZ | CHANNEL_OFDM | CHANNEL_HT20)

#define CHANNEL_G_HT20  (CHANNEL_2GHZ|CHANNEL_HT20)
#define CHANNEL_A_HT20  (CHANNEL_5GHZ|CHANNEL_HT20)
#define CHANNEL_G_HT40  (CHANNEL_2GHZ|CHANNEL_HT20|CHANNEL_HT40)
#define CHANNEL_A_HT40  (CHANNEL_5GHZ|CHANNEL_HT20|CHANNEL_HT40)
#define CHANNEL_ALL \
    (CHANNEL_OFDM|CHANNEL_CCK| CHANNEL_2GHZ | CHANNEL_5GHZ | CHANNEL_TURBO | CHANNEL_HT20 | CHANNEL_HT40)
#define CHANNEL_ALL_NOTURBO     (CHANNEL_ALL &~ CHANNEL_TURBO)

enum {
    HAL_MODE_11A    = 0x001,        
    HAL_MODE_TURBO  = 0x002,        
    HAL_MODE_11B    = 0x004,        
    HAL_MODE_PUREG  = 0x008,        
#ifdef notdef
    HAL_MODE_11G    = 0x010,        
#else
    HAL_MODE_11G    = 0x008,        
#endif
    HAL_MODE_108G   = 0x020,        
    HAL_MODE_108A   = 0x040,        
    HAL_MODE_XR     = 0x100,        
    HAL_MODE_11A_HALF_RATE = 0x200,     
    HAL_MODE_11A_QUARTER_RATE = 0x400,  
    HAL_MODE_11NG   = 0x4000,           
    HAL_MODE_11NA   = 0x8000,           
    HAL_MODE_ALL    = 0xffff
};

#endif 
