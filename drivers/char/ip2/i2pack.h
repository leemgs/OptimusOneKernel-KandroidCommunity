
#ifndef I2PACK_H
#define I2PACK_H  1








































#pragma pack(1)





typedef struct _i2DataHeader
{
	unsigned char i2sChannel;  

	

	
	
	unsigned i2sId : 2 __attribute__ ((__packed__));

	
	
	
	
	unsigned i2sTag : 4;

	
	unsigned i2sType : 2;

	
	
	unsigned short i2sCount;

} i2DataHeader, *i2DataHeaderPtr;







typedef struct _i2CmdHeader
{
	unsigned char i2sChannel;	
								

	
	unsigned i2sCount : 6;

	
	unsigned i2sType : 2;

} i2CmdHeader, *i2CmdHeaderPtr;
















typedef struct _flowIn
{
	i2CmdHeader    hd;      
	unsigned char  fcmd;    
	unsigned short asof;    
							
	unsigned short room;
} flowIn, *flowInPtr;











#define STAT_CTS_UP     0  
#define STAT_CTS_DN     1  
#define STAT_DCD_UP     2  
#define STAT_DCD_DN     3  
#define STAT_DSR_UP     4  
#define STAT_DSR_DN     5  
#define STAT_RI_UP      6  
#define STAT_RI_DN      7  
#define STAT_BRK_DET    8  
#define STAT_FLOW       9  
#define STAT_BMARK      10 
#define STAT_STATUS     11 
#define STAT_TXCNT      12 
#define STAT_RXCNT      13 
#define STAT_BOXIDS     14 
#define STAT_HWFAIL     15 

#define STAT_MOD_ERROR  0xc0
#define STAT_MODEM      0xc0
#define STAT_MOD_DCD    0x8
#define STAT_MOD_RI     0x4
#define STAT_MOD_DSR    0x2
#define STAT_MOD_CTS    0x1

#define STAT_ERROR      0x80


#define STAT_E_PARITY   4     
#define STAT_E_FRAMING  2     
#define STAT_E_OVERRUN  1     





typedef struct _flowStat
{
	unsigned short asof;
	unsigned short room;
}flowStat, *flowStatPtr;












#define CREDIT_USAGE(size) (((size) + 3) & ~1)





typedef  struct   _debugStat
{
	unsigned char d_ccsr;
	unsigned char d_txinh;
	unsigned char d_stat1;
	unsigned char d_stat2;
} debugStat, *debugStatPtr;




#define D_CCSR_XON      2     
#define D_CCSR_XOFF     4     
#define D_CCSR_TXENAB   8     
#define D_CCSR_RXENAB   0x80  

#define D_TXINH_BREAK   1     
#define D_TXINH_EMPTY   2     
#define D_TXINH_SUSP    4     
#define D_TXINH_CMD     8     
#define D_TXINH_LCD     0x10  
#define D_TXINH_PAUSE   0x20  
#define D_TXINH_DCD     0x40  
#define D_TXINH_DSR     0x80  

#define D_STAT1_TXEN    1     
#define D_STAT1_RXEN    2     
#define D_STAT1_MDEN    4     
#define D_STAT1_RLM     8     
#define D_STAT1_LLM     0x10  
#define D_STAT1_CTS     0x20  
#define D_STAT1_DTR     0x40  
#define D_STAT1_RTS     0x80  

#define D_STAT2_TXMT    1     
#define D_STAT2_RXMT    2     
#define D_STAT2_RXINH   4     
#define D_STAT2_RXFLO   8     




typedef  struct   _cntStat
{
	unsigned short cs_time;    
	unsigned short cs_count;
} cntStat, *cntStatPtr;










typedef struct _failStat
{
	unsigned char fs_written;
	unsigned char fs_read;
	unsigned short fs_address;
} failStat, *failStatPtr;

















#define CHANNEL_OF(pP)  ((i2DataHeaderPtr)(pP))->i2sChannel



#define PTYPE_OF(pP) ((i2DataHeaderPtr)(pP))->i2sType



#define PTYPE_DATA   0  
#define PTYPE_BYPASS 1  
#define PTYPE_INLINE 2  
#define PTYPE_STATUS 2  



#define TAG_OF(pP) ((i2DataHeaderPtr)(pP))->i2sTag



#define ID_OF(pP)  ((i2DataHeaderPtr)(pP))->i2sId



#define ID_ORDINARY_DATA   0
#define ID_HOT_KEY         1



#define DATA_COUNT_OF(pP) ((i2DataHeaderPtr)(pP))->i2sCount



#define DATA_OF(pP) &((unsigned char *)(pP))[4] 



#define CMD_COUNT_OF(pP) ((i2CmdHeaderPtr)(pP))->i2sCount

#define MAX_CMD_PACK_SIZE  62 



#define CMD_OF(pP) &((unsigned char *)(pP))[2]  









#define MB_OUT_STUFFED     0x80  
#define MB_IN_STRIPPED     0x40  





#define MB_IN_STUFFED      0x80  
#define MB_OUT_STRIPPED    0x40  
#define MB_FATAL_ERROR     0x20  

#pragma pack()                  

#endif      

