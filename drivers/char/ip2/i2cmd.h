













#ifndef I2CMD_H      
#define I2CMD_H   1

#include "ip2types.h"












typedef struct _cmdSyntax
{
	UCHAR length;   
	UCHAR flags;    

	
	
	
	
	UCHAR cmd[2];
} cmdSyntax, *cmdSyntaxPtr;



#define INL 1           
#define BYP 2           
#define BTH (INL|BYP)   
#define END 4           
#define VIP 8           
						
						
#define VAR 0x10        











static UCHAR ct02[];
static UCHAR ct03[];
static UCHAR ct04[];
static UCHAR ct05[];
static UCHAR ct06[];
static UCHAR ct07[];
static UCHAR ct08[];
static UCHAR ct09[];
static UCHAR ct10[];
static UCHAR ct11[];
static UCHAR ct12[];
static UCHAR ct13[];
static UCHAR ct14[];
static UCHAR ct15[];
static UCHAR ct16[];
static UCHAR ct17[];
static UCHAR ct18[];
static UCHAR ct19[];
static UCHAR ct20[];
static UCHAR ct21[];
static UCHAR ct22[];
static UCHAR ct23[];
static UCHAR ct24[];
static UCHAR ct25[];
static UCHAR ct26[];
static UCHAR ct27[];
static UCHAR ct28[];
static UCHAR ct29[];
static UCHAR ct30[];
static UCHAR ct31[];
static UCHAR ct32[];
static UCHAR ct33[];
static UCHAR ct34[];
static UCHAR ct35[];
static UCHAR ct36[];
static UCHAR ct36a[];
static UCHAR ct41[];
static UCHAR ct42[];
static UCHAR ct43[];
static UCHAR ct44[];
static UCHAR ct45[];
static UCHAR ct46[];
static UCHAR ct48[];
static UCHAR ct49[];
static UCHAR ct50[];
static UCHAR ct51[];
static UCHAR ct52[];
static UCHAR ct56[];
static UCHAR ct57[];
static UCHAR ct58[];
static UCHAR ct59[];
static UCHAR ct60[];
static UCHAR ct61[];
static UCHAR ct62[];
static UCHAR ct63[];
static UCHAR ct64[];
static UCHAR ct65[];
static UCHAR ct66[];
static UCHAR ct67[];
static UCHAR ct68[];
static UCHAR ct69[];
static UCHAR ct70[];
static UCHAR ct71[];
static UCHAR ct72[];
static UCHAR ct73[];
static UCHAR ct74[];
static UCHAR ct75[];
static UCHAR ct76[];
static UCHAR ct77[];
static UCHAR ct78[];
static UCHAR ct79[];
static UCHAR ct80[];
static UCHAR ct81[];
static UCHAR ct82[];
static UCHAR ct83[];
static UCHAR ct84[];
static UCHAR ct85[];
static UCHAR ct86[];
static UCHAR ct87[];
static UCHAR ct88[];
static UCHAR ct89[];
static UCHAR ct90[];
static UCHAR ct91[];
static UCHAR cc01[];
static UCHAR cc02[];
















#define CMD_DTRUP	(cmdSyntaxPtr)(ct02)	
#define CMD_DTRDN	(cmdSyntaxPtr)(ct03)	
#define CMD_RTSUP	(cmdSyntaxPtr)(ct04)	
#define CMD_RTSDN	(cmdSyntaxPtr)(ct05)	
#define CMD_STARTFL	(cmdSyntaxPtr)(ct06)	

#define CMD_DTRRTS_UP (cmdSyntaxPtr)(cc01)	
#define CMD_DTRRTS_DN (cmdSyntaxPtr)(cc02)	


#define CMD_SETBAUD(arg) \
	(((cmdSyntaxPtr)(ct07))->cmd[1] = (arg),(cmdSyntaxPtr)(ct07))

#define CBR_50       1
#define CBR_75       2
#define CBR_110      3
#define CBR_134      4
#define CBR_150      5
#define CBR_200      6
#define CBR_300      7
#define CBR_600      8
#define CBR_1200     9
#define CBR_1800     10
#define CBR_2400     11
#define CBR_4800     12
#define CBR_9600     13
#define CBR_19200    14
#define CBR_38400    15
#define CBR_2000     16
#define CBR_3600     17
#define CBR_7200     18
#define CBR_56000    19
#define CBR_57600    20
#define CBR_64000    21
#define CBR_76800    22
#define CBR_115200   23
#define CBR_C1       24    
#define CBR_C2       25    
#define CBR_153600   26
#define CBR_230400   27
#define CBR_307200   28
#define CBR_460800   29
#define CBR_921600   30



#define CMD_SETBITS(arg) \
	(((cmdSyntaxPtr)(ct08))->cmd[1] = (arg),(cmdSyntaxPtr)(ct08))

#define CSZ_5  0
#define CSZ_6  1
#define CSZ_7  2
#define CSZ_8  3



#define CMD_SETSTOP(arg) \
	(((cmdSyntaxPtr)(ct09))->cmd[1] = (arg),(cmdSyntaxPtr)(ct09))

#define CST_1  0
#define CST_15 1  
#define CST_2  2



#define CMD_SETPAR(arg) \
	(((cmdSyntaxPtr)(ct10))->cmd[1] = (arg),(cmdSyntaxPtr)(ct10))

#define CSP_NP 0  
#define CSP_OD 1  
#define CSP_EV 2  
#define CSP_SP 3  
#define CSP_MK 4  



#define CMD_DEF_IXON(arg) \
	(((cmdSyntaxPtr)(ct11))->cmd[1] = (arg),(cmdSyntaxPtr)(ct11))



#define CMD_DEF_IXOFF(arg) \
	(((cmdSyntaxPtr)(ct12))->cmd[1] = (arg),(cmdSyntaxPtr)(ct12))

#define CMD_STOPFL   (cmdSyntaxPtr)(ct13) 



#define CMD_HOTACK   (cmdSyntaxPtr)(ct14)




#define CMDVALUE_IRQ 15 
						
#define CMD_SET_IRQ(arg) \
	(((cmdSyntaxPtr)(ct15))->cmd[1] = (arg),(cmdSyntaxPtr)(ct15))

#define CIR_POLL  0  
#define CIR_3     3  
#define CIR_4     4  
#define CIR_5     5  
#define CIR_7     7  
#define CIR_10    10 
#define CIR_11    11 
#define CIR_12    12 
#define CIR_15    15 



#define CMD_IXON_OPT(arg) \
	(((cmdSyntaxPtr)(ct16))->cmd[1] = (arg),(cmdSyntaxPtr)(ct16))

#define CIX_NONE  0  
#define CIX_XON   1  
#define CIX_XANY  2  



#define CMD_OXON_OPT(arg) \
	(((cmdSyntaxPtr)(ct17))->cmd[1] = (arg),(cmdSyntaxPtr)(ct17))

#define COX_NONE  0  
#define COX_XON   1  


#define CMD_CTS_REP  (cmdSyntaxPtr)(ct18) 
#define CMD_CTS_NREP (cmdSyntaxPtr)(ct19) 

#define CMD_DCD_REP  (cmdSyntaxPtr)(ct20) 
#define CMD_DCD_NREP (cmdSyntaxPtr)(ct21) 

#define CMD_DSR_REP  (cmdSyntaxPtr)(ct22) 
#define CMD_DSR_NREP (cmdSyntaxPtr)(ct23) 

#define CMD_RI_REP   (cmdSyntaxPtr)(ct24) 
#define CMD_RI_NREP  (cmdSyntaxPtr)(ct25) 



#define CMD_BRK_REP(arg) \
	(((cmdSyntaxPtr)(ct26))->cmd[1] = (arg),(cmdSyntaxPtr)(ct26))

#define CBK_STAT     0x00  
#define CBK_NULL     0x01  
#define CBK_STAT_SEQ 0x02  
                           
#define CBK_SEQ      0x03  
						   
#define CBK_FLSH     0x04  
#define CBK_POSIX    0x08  
#define CBK_SINGLE   0x10  
						   

#define CMD_BRK_NREP (cmdSyntaxPtr)(ct27) 



#define CMD_MAX_BLOCK(arg) \
	(((cmdSyntaxPtr)(ct28))->cmd[1] = (arg),(cmdSyntaxPtr)(ct28))



#define CMD_CTSFL_ENAB  (cmdSyntaxPtr)(ct30) 
#define CMD_CTSFL_DSAB  (cmdSyntaxPtr)(ct31) 
#define CMD_RTSFL_ENAB  (cmdSyntaxPtr)(ct32) 
#define CMD_RTSFL_DSAB  (cmdSyntaxPtr)(ct33) 



#define CMD_ISTRIP_OPT(arg) \
	(((cmdSyntaxPtr)(ct34))->cmd[1] = (arg),(cmdSyntaxPtr)(ct34))

#define CIS_NOSTRIP  0  
#define CIS_STRIP    1  



#define CMD_SEND_BRK(arg) \
	(((cmdSyntaxPtr)(ct35))->cmd[1] = (arg),(cmdSyntaxPtr)(ct35))



#define CMD_SET_ERROR(arg) \
	(((cmdSyntaxPtr)(ct36))->cmd[1] = (arg),(cmdSyntaxPtr)(ct36))

#define CSE_ESTAT 0  
#define CSE_NOREP 1  
#define CSE_DROP  2  
#define CSE_NULL  3  
#define CSE_MARK  4  

#define CSE_REPLACE  0x8	
							

#define CSE_STAT_REPLACE   0x18	
								
								
								















#define CMD_OPOST_ON(oflag)   \
	(*(USHORT *)(((cmdSyntaxPtr)(ct39))->cmd[1]) = (oflag), \
		(cmdSyntaxPtr)(ct39))

#define CMD_OPOST_OFF   (cmdSyntaxPtr)(ct40) 

#define CMD_RESUME   (cmdSyntaxPtr)(ct41)	
											



#define CMD_SETBAUD_TX(arg) \
	(((cmdSyntaxPtr)(ct42))->cmd[1] = (arg),(cmdSyntaxPtr)(ct42))



#define CMD_SETBAUD_RX(arg) \
	(((cmdSyntaxPtr)(ct43))->cmd[1] = (arg),(cmdSyntaxPtr)(ct43))





#define CMD_PING_REQ(arg) \
	(((cmdSyntaxPtr)(ct44))->cmd[1] = (arg),(cmdSyntaxPtr)(ct44))

#define CMD_HOT_ENAB (cmdSyntaxPtr)(ct45) 
#define CMD_HOT_DSAB (cmdSyntaxPtr)(ct46) 

#if 0







#define CMD_UNIX_FLAGS(iflag,cflag,lflag) i2cmdUnixFlags(iflag,cflag,lflag)
#endif  

#define CMD_DSRFL_ENAB  (cmdSyntaxPtr)(ct48) 
#define CMD_DSRFL_DSAB  (cmdSyntaxPtr)(ct49) 
#define CMD_DTRFL_ENAB  (cmdSyntaxPtr)(ct50) 
#define CMD_DTRFL_DSAB  (cmdSyntaxPtr)(ct51) 
#define CMD_BAUD_RESET  (cmdSyntaxPtr)(ct52) 




#define CMD_BAUD_DEF1(rate) i2cmdBaudDef(1,rate)




#define CMD_BAUD_DEF2(rate) i2cmdBaudDef(2,rate)



#define CMD_PAUSE(arg) \
	(((cmdSyntaxPtr)(ct56))->cmd[1] = (arg),(cmdSyntaxPtr)(ct56))

#define CMD_SUSPEND     (cmdSyntaxPtr)(ct57) 
#define CMD_UNSUSPEND   (cmdSyntaxPtr)(ct58) 



#define CMD_PARCHK(arg) \
	(((cmdSyntaxPtr)(ct59))->cmd[1] = (arg),(cmdSyntaxPtr)(ct59))

#define CPK_ENAB  0     
#define CPK_DSAB  1     

#define CMD_BMARK_REQ   (cmdSyntaxPtr)(ct60) 




#define CMD_INLOOP(arg) \
	(((cmdSyntaxPtr)(ct61))->cmd[1] = (arg),(cmdSyntaxPtr)(ct61))

#define CIN_DISABLE  0  
#define CIN_ENABLE   1  
#define CIN_REMOTE   2  




#define CMD_HOT_TIME(arg) \
	(((cmdSyntaxPtr)(ct62))->cmd[1] = (arg),(cmdSyntaxPtr)(ct62))




#define CMD_DEF_OXON(arg) \
	(((cmdSyntaxPtr)(ct63))->cmd[1] = (arg),(cmdSyntaxPtr)(ct63))



#define CMD_DEF_OXOFF(arg) \
	(((cmdSyntaxPtr)(ct64))->cmd[1] = (arg),(cmdSyntaxPtr)(ct64))



#define CMD_RTS_XMIT(arg) \
	(((cmdSyntaxPtr)(ct65))->cmd[1] = (arg),(cmdSyntaxPtr)(ct65))

#define CHD_DISABLE  0
#define CHD_ENABLE   1



#define CMD_SETHIGHWAT(arg) \
	(((cmdSyntaxPtr)(ct66))->cmd[1] = (arg),(cmdSyntaxPtr)(ct66))



#define CMD_START_SELFL(tag) \
	(((cmdSyntaxPtr)(ct67))->cmd[1] = (tag),(cmdSyntaxPtr)(ct67))



#define CMD_END_SELFL(tag) \
	(((cmdSyntaxPtr)(ct68))->cmd[1] = (tag),(cmdSyntaxPtr)(ct68))

#define CMD_HWFLOW_OFF  (cmdSyntaxPtr)(ct69) 
#define CMD_ODSRFL_ENAB (cmdSyntaxPtr)(ct70) 
#define CMD_ODSRFL_DSAB (cmdSyntaxPtr)(ct71) 
#define CMD_ODCDFL_ENAB (cmdSyntaxPtr)(ct72) 
#define CMD_ODCDFL_DSAB (cmdSyntaxPtr)(ct73) 



#define CMD_LOADLEVEL(count) \
	(((cmdSyntaxPtr)(ct74))->cmd[1] = (count),(cmdSyntaxPtr)(ct74))



#define CMD_STATDATA(arg) \
	(((cmdSyntaxPtr)(ct75))->cmd[1] = (arg),(cmdSyntaxPtr)(ct75))

#define CSTD_DISABLE
#define CSTD_ENABLE	
					

#define CMD_BREAK_ON    (cmdSyntaxPtr)(ct76)
#define CMD_BREAK_OFF   (cmdSyntaxPtr)(ct77)
#define CMD_GETFC       (cmdSyntaxPtr)(ct78)
											



#define CMD_XMIT_NOW(ch) \
	(((cmdSyntaxPtr)(ct79))->cmd[1] = (ch),(cmdSyntaxPtr)(ct79))



#define CMD_DIVISOR_LATCH(which,value) \
			(((cmdSyntaxPtr)(ct80))->cmd[1] = (which), \
			*(USHORT *)(((cmdSyntaxPtr)(ct80))->cmd[2]) = (value), \
			(cmdSyntaxPtr)(ct80))

#define CDL_RX 1	
#define CDL_TX 2	
					



#define CMD_GET_STATUS (cmdSyntaxPtr)(ct81)



#define CMD_GET_TXCNT  (cmdSyntaxPtr)(ct82)



#define CMD_GET_RXCNT  (cmdSyntaxPtr)(ct83)


#define CMD_GET_BOXIDS (cmdSyntaxPtr)(ct84)



#define CMD_ENAB_MULT(enable, box1, box2, box3, box4)    \
			(((cmdSytaxPtr)(ct85))->cmd[1] = (enable),            \
			*(USHORT *)(((cmdSyntaxPtr)(ct85))->cmd[2]) = (box1), \
			*(USHORT *)(((cmdSyntaxPtr)(ct85))->cmd[4]) = (box2), \
			*(USHORT *)(((cmdSyntaxPtr)(ct85))->cmd[6]) = (box3), \
			*(USHORT *)(((cmdSyntaxPtr)(ct85))->cmd[8]) = (box4), \
			(cmdSyntaxPtr)(ct85))

#define CEM_DISABLE  0
#define CEM_ENABLE   1



#define CMD_RCV_ENABLE(ch) \
	(((cmdSyntaxPtr)(ct86))->cmd[1] = (ch),(cmdSyntaxPtr)(ct86))

#define CRE_OFF      0  
#define CRE_ON       1  
#define CRE_INTOFF   2  
#define CRE_INTON    3  




#define CMD_HW_TEST  (cmdSyntaxPtr)(ct87)






#define CMD_RCV_THRESHOLD(count,ms) \
			(((cmdSyntaxPtr)(ct88))->cmd[1] = (count), \
			((cmdSyntaxPtr)(ct88))->cmd[2] = (ms), \
			(cmdSyntaxPtr)(ct88))



#define CMD_DSS_NOW (cmdSyntaxPtr)(ct89)
	




#define CMD_SET_SILO(timeout,threshold) \
			(((cmdSyntaxPtr)(ct90))->cmd[1] = (timeout), \
			((cmdSyntaxPtr)(ct90))->cmd[2]  = (threshold), \
			(cmdSyntaxPtr)(ct90))



#define CMD_LBREAK(ds) \
	(((cmdSyntaxPtr)(ct91))->cmd[1] = (ds),(cmdSyntaxPtr)(ct66))



#endif 
