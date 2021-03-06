


















#ifndef	_sxboards_h				
#define	_sxboards_h    1




#define		BUS_ISA		0
#define		BUS_MCA		1
#define		BUS_EISA	2
#define		BUS_PCI		3


#define		SI1_Z280	1
#define		SI2_Z280	2
#define		SI3_T225	3


#define		CARD_TYPE(bus,phase)	(bus<<4|phase)
#define		CARD_BUS(type)		((type>>4)&0xF)
#define		CARD_PHASE(type)	(type&0xF)

#define		TYPE_SI1_ISA		CARD_TYPE(BUS_ISA,SI1_Z280)
#define		TYPE_SI2_ISA		CARD_TYPE(BUS_ISA,SI2_Z280)
#define		TYPE_SI2_EISA		CARD_TYPE(BUS_EISA,SI2_Z280)
#define		TYPE_SI2_PCI		CARD_TYPE(BUS_PCI,SI2_Z280)

#define		TYPE_SX_ISA		CARD_TYPE(BUS_ISA,SI3_T225)
#define		TYPE_SX_PCI		CARD_TYPE(BUS_PCI,SI3_T225)



#define		SI1_ISA_WINDOW_LEN	0x10000		








#define		SI1_ISA_RESET		0x8000		
#define		SI1_ISA_RESET_CLEAR	0xc000		
#define		SI1_ISA_WAIT	        0x9000		
#define		SI1_ISA_WAIT_CLEAR	0xd000		
#define		SI1_ISA_INTCL        	0xa000		
#define		SI1_ISA_INTCL_CLEAR	0xe000		





#define		SI2_ISA_WINDOW_LEN	0x8000		
#define 	SI2_ISA_MEMORY_LEN	0x7FF8		
#define		SI2_ISA_ADDR_LOW	0x0A0000	
#define		SI2_ISA_ADDR_HIGH	0xFF8000	
#define		SI2_ISA_ADDR_STEP	SI2_ISA_WINDOW_LEN
#define		SI2_ISA_IRQ_MASK	0x9800		


#define		SI2_ISA_ID_BASE		0x7FF8			
#define		SI2_ISA_RESET		SI2_ISA_ID_BASE		
#define		SI2_ISA_IRQ11		(SI2_ISA_ID_BASE+1)	
#define		SI2_ISA_IRQ12		(SI2_ISA_ID_BASE+2)	
#define		SI2_ISA_IRQ15		(SI2_ISA_ID_BASE+3)	
#define		SI2_ISA_IRQSET		(SI2_ISA_ID_BASE+4)	
#define		SI2_ISA_INTCLEAR	(SI2_ISA_ID_BASE+5)	

#define		SI2_ISA_IRQ11_SET	0x10
#define		SI2_ISA_IRQ11_CLEAR	0x00
#define		SI2_ISA_IRQ12_SET	0x10
#define		SI2_ISA_IRQ12_CLEAR	0x00
#define		SI2_ISA_IRQ15_SET	0x10
#define		SI2_ISA_IRQ15_CLEAR	0x00
#define		SI2_ISA_INTCLEAR_SET	0x10
#define		SI2_ISA_INTCLEAR_CLEAR	0x00
#define		SI2_ISA_IRQSET_CLEAR	0x10
#define		SI2_ISA_IRQSET_SET	0x00
#define		SI2_ISA_RESET_SET	0x00
#define		SI2_ISA_RESET_CLEAR	0x10


#define		SI2_PCI_WINDOW_LEN	0x100000	


#define		SI2_PCI_SET_IRQ		0x40001		
#define		SI2_PCI_RESET		0xC0001		




#define		SX_WINDOW_LEN		64*1024		


#define		SX_ISA_ADDR_LOW		0x0A0000	
#define		SX_ISA_ADDR_HIGH	0xFF8000	
#define		SX_ISA_ADDR_STEP	SX_WINDOW_LEN	
#define		SX_ISA_IRQ_MASK		0x9E00		


#define		SX_EVENT_STATUS		0x7800		
#define		SX_EVENT_STROBE		0x7800		
#define		SX_EVENT_ENABLE		0x7880		
#define		SX_VPD_ROM		0x7C00		
#define		SX_CONFIG		0x7C00		
#define		SX_IRQ_STATUS		0x7C80		
#define		SX_SET_IRQ		0x7C80		
#define		SX_RESET_STATUS		0x7D00		
#define		SX_RESET		0x7D00		
#define		SX_RESET_IRQ		0x7D80		


#define		SX_VPD_SLX_ID1		0x00
#define		SX_VPD_SLX_ID2		0x01
#define		SX_VPD_HW_REV		0x02
#define		SX_VPD_HW_ASSEM		0x03
#define		SX_VPD_UNIQUEID4	0x04
#define		SX_VPD_UNIQUEID3	0x05
#define		SX_VPD_UNIQUEID2	0x06
#define		SX_VPD_UNIQUEID1	0x07
#define		SX_VPD_MANU_YEAR	0x08
#define		SX_VPD_MANU_WEEK	0x09
#define		SX_VPD_IDENT		0x10
#define		SX_VPD_IDENT_STRING	"JET HOST BY KEV#"


#define		SX_UNIQUEID_MASK	0xF0
#define		SX_ISA_UNIQUEID1	0x20
#define		SX_PCI_UNIQUEID1	0x50


#define		SX_CONF_BUSEN		0x02		
#define		SX_CONF_HOSTIRQ		0x04		


#define		SX_BOOTSTRAP		"\x28\x20\x21\x02\x60\x0a"
#define		SX_BOOTSTRAP_SIZE	6
#define		SX_BOOTSTRAP_ADDR	(0x8000-SX_BOOTSTRAP_SIZE)



#define		SI2_EISA_OFF	 	0x42
#define		SI2_EISA_VAL	 	0x01
#define		SI2_EISA_WINDOW_LEN     0x10000





#define		SPX_VENDOR_ID		0x11CB		
#define		SPX_DEVICE_ID		0x4000		
#define		SPX_PLXDEVICE_ID	0x2000		

#define		SPX_SUB_VENDOR_ID	SPX_VENDOR_ID	
#define		SI2_SUB_SYS_ID		0x400		
#define		SX_SUB_SYS_ID		0x200		

#endif						


