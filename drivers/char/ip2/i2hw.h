
#ifndef I2HW_H
#define I2HW_H 1

















#include "ip2types.h"











#define FIFO_DATA 0
















#define FIFO_STATUS 2



#define ST_OUT_FULL  0x40  
#define ST_IN_EMPTY  0x20  
#define ST_IN_MAIL   0x04  






#define STE_OUT_MAIL 0x80  















#define STN_MR       0x80




#define STN_OUT_AF  0x10  
#define STN_IN_AE   0x08  
#define STN_BD      0x02  
#define STN_PE      0x01  



#define STE_OUT_HF  0x10  
#define STE_IN_HF   0x08  
#define STE_IN_FULL 0x02  
#define STE_OUT_MT  0x01  







#define FIFO_PTR    0x02



#define SEL_COMMAND 0x1    



#define SEL_CMD_MR  0x80	
#define SEL_CMD_SH  0x40	
							
#define SEL_CMD_UNSH   0	
							
#define SEL_MASK     0x2	
							
							
							
#define SEL_BYTE_DET 0x3	
							
#define SEL_OUTMAIL  0x4	
							
							
							
							
							
#define SEL_AEAF     0x5	
#define SEL_INMAIL   0x6	






#define FIFO_MASK    0x2








#define  MX_OUTMAIL_RSEL   0x80

#define  MX_IN_MAIL  0x40	
							
#define  MX_IN_FULL  0x20	
							
#define  MX_IN_MT    0x08	
							
#define  MX_OUT_FULL 0x04	
							
#define  MX_OUT_MT   0x01	
							





















#define  FIFO_MAIL   0x3




#define  FIFO_RESET  0x7







#define  FIFO_NOP    0x7









typedef union _porStr		
{
	unsigned char  c[16];	
							

	struct					
	{
		
		
		
		
		
		
		
		

		unsigned char porMagic1;   
		unsigned char porMagic2;   

		
		

		unsigned char porVersion;  
		unsigned char porRevision; 
		unsigned char porSubRev;   

		unsigned char porID;	
								
								
								
								

		unsigned char porBus;	
								
								

		unsigned char porMemory;	

		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		unsigned char  porPorts1;

		unsigned char  porDiag1;	
		unsigned char  porDiag2;	
		unsigned char  porSpeed;	
									
		unsigned char  porFlags;	
									

		unsigned char  porPorts2;	
									
									

		
		
		
		
		
		unsigned char  porFifoSize;

		
		
		unsigned char  porNumBoxes;
	} e;
} porStr, *porStrPtr;









#define  POR_MAGIC_1    0x96  
#define  POR_MAGIC_2    0x35  
#define  POR_1_INDEX    0     
#define  POR_2_INDEX    1     





#define  POR_ID_FAMILY  0xc0	
								
#define  POR_ID_FII     0x00	
#define  POR_ID_FIIEX   0x40	




#define POR_ID_RESERVED 0x3c

#define POR_ID_SIZE     0x03	
								
#define POR_ID_II_8     0x00	
								
#define POR_ID_II_8R    0x01	
								
#define POR_ID_II_6     0x02	
								
#define POR_ID_II_4     0x03	
								
#define POR_ID_EX       0x00	
								







#define POR_BUS_SLOT16  0x20




#define POR_BUS_DIP16   0x10








#define  POR_BUS_TYPE   0x07




#define  POR_BUS_T_UNK  0




#define  POR_BUS_T_MCA  1  
#define  POR_BUS_T_EISA 2  
#define  POR_BUS_T_ISA  3  









#define  POR_BAD_MAPPER 0x80	



#define  POR_BAD_UART1  0x01	
#define  POR_BAD_UART2  0x02	





#define  POR_DEBUG_PORT 0x80	
#define  POR_DIAG_OK    0x00	
								
								




#define  POR_CPU     0x03	
#define  POR_CPU_8   0x01	
#define  POR_CPU_6   0x02	
#define  POR_CEX4    0x04	
							
							
#define POR_BOXES    0xf0	
							
#define POR_BOX_16   0x10	

















#define LOADWARE_BLOCK_SIZE   512   

typedef union _loadHdrStr
{
	unsigned char c[LOADWARE_BLOCK_SIZE];  

	struct	
	{
		unsigned char loadMagic;		
		unsigned char loadBlocksMore;	
		unsigned char loadCRC[2];		
		unsigned char loadVersion;		
		unsigned char loadRevision;		
		unsigned char loadSubRevision;	
		unsigned char loadSpares[9];	
		unsigned char loadDates[32];	
										
	} e;
} loadHdrStr, *loadHdrStrPtr;








#define  MAGIC_LOADFILE 0x3c





#define  LOADWARE_OK    0xc3        
#define  LOADWARE_BAD   0x5a        







#define  MAX_DLOAD_START_TIME 1000  
#define  MAX_DLOAD_READ_TIME  100   




#define  MAX_DLOAD_ACK_TIME   100   







#define ABS_MAX_BOXES   4     
#define ABS_BIGGEST_BOX 16    
#define ABS_MOST_PORTS  (ABS_MAX_BOXES * ABS_BIGGEST_BOX)

#define I2_OUTSW(port, addr, count)	outsw((port), (addr), (((count)+1)/2))
#define I2_OUTSB(port, addr, count)	outsb((port), (addr), (((count)+1))&-2)
#define I2_INSW(port, addr, count)	insw((port), (addr), (((count)+1)/2))
#define I2_INSB(port, addr, count)	insb((port), (addr), (((count)+1))&-2)

#endif   

