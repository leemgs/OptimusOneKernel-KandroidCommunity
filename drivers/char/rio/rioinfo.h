

#ifndef __rioinfo_h
#define __rioinfo_h


struct RioHostInfo {
	long location;		
	long vector;		
	int bus;		
	int mode;		
	struct old_sgttyb
	*Sg;			
};



#define INTERRUPTED_MODE	0x01	
#define POLLED_MODE		0x02	
#define AUTO_MODE		0x03	

#define WORD_ACCESS_MODE	0x10	
#define BYTE_ACCESS_MODE	0x20	



#define ISA_BUS			0x01	
#define EISA_BUS		0x02	
#define MCA_BUS			0x04	
#define PCI_BUS			0x08	



#define DEF_TERM_CHARACTERISTICS \
{ \
	B19200, B19200,				 \
	'H' - '@',				 \
	-1,					 \
	'U' - '@',				 \
	ECHO | CRMOD,				 \
	'C' - '@',				 \
	'\\' - '@',				/* quit char */ \
	'Q' - '@',				/* start char */ \
	'S' - '@',				/* stop char */ \
	'D' - '@',				/* EOF */ \
	-1,					/* brk */ \
	(LCRTBS | LCRTERA | LCRTKIL | LCTLECH),	/* local mode word */ \
	'Z' - '@',				/* process stop */ \
	'Y' - '@',				/* delayed stop */ \
	'R' - '@',				/* reprint line */ \
	'O' - '@',				/* flush output */ \
	'W' - '@',				/* word erase */ \
	'V' - '@'				/* literal next char */ \
}

#endif				/* __rioinfo_h */
