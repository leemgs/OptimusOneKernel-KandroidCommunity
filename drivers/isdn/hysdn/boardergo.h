





#define ERG_DPRAM_PAGE_SIZE 0x2000	
#define BOOT_IMG_SIZE 4096
#define ERG_DPRAM_FILL_SIZE (ERG_DPRAM_PAGE_SIZE - BOOT_IMG_SIZE)

#define ERG_TO_HY_BUF_SIZE  0x0E00	
#define ERG_TO_PC_BUF_SIZE  0x0E00	


typedef struct ErgDpram_tag {
 unsigned char ToHyBuf[ERG_TO_HY_BUF_SIZE];
 unsigned char ToPcBuf[ERG_TO_PC_BUF_SIZE];

	 unsigned char bSoftUart[SIZE_RSV_SOFT_UART];
	

	 unsigned char volatile ErrLogMsg[64];
	
	
	
	
	
	

 unsigned short volatile ToHyChannel;
 unsigned short volatile ToHySize;
	 unsigned char volatile ToHyFlag;
	
	 unsigned char volatile ToPcFlag;
	
 unsigned short volatile ToPcChannel;
 unsigned short volatile ToPcSize;
	 unsigned char bRes1DBA[0x1E00 - 0x1DFA];
	

 unsigned char bRestOfEntryTbl[0x1F00 - 0x1E00];
 unsigned long TrapTable[62];
	 unsigned char bRes1FF8[0x1FFB - 0x1FF8];
	
 unsigned char ToPcIntMetro;
	
 unsigned char volatile ToHyNoDpramErrLog;
	
 unsigned char bRes1FFD;
	 unsigned char ToPcInt;
	
	 unsigned char ToHyInt;
	
} tErgDpram;





#define PCI9050_INTR_REG    0x4C	
#define PCI9050_USER_IO     0x51	

				    
#define PCI9050_INTR_REG_EN1    0x01	
#define PCI9050_INTR_REG_POL1   0x02	
#define PCI9050_INTR_REG_STAT1  0x04	
#define PCI9050_INTR_REG_ENPCI  0x40	

				    
#define PCI9050_USER_IO_EN3     0x02	
#define PCI9050_USER_IO_DIR3    0x04	
#define PCI9050_USER_IO_DAT3    0x08	

#define PCI9050_E1_RESET    (                     PCI9050_USER_IO_DIR3)		
#define PCI9050_E1_RUN      (PCI9050_USER_IO_DAT3|PCI9050_USER_IO_DIR3)		
