

#include <linux/ioport.h>
#include <linux/mtd/mtd.h>




#define MS02NV_CSR		0x400000	


#define MS02NV_CSR_BATT_OK	0x01		
#define MS02NV_CSR_BATT_OFF	0x02		



#define MS02NV_DIAG		0x0003f8	
#define MS02NV_MAGIC		0x0003fc	
#define MS02NV_VALID		0x000400	
#define MS02NV_RAM		0x001000	


#define MS02NV_DIAG_TEST	0x01		
#define MS02NV_DIAG_RO		0x02		
#define MS02NV_DIAG_RW		0x04		
#define MS02NV_DIAG_FAIL	0x08		
#define MS02NV_DIAG_SIZE_MASK	0xf0		
#define MS02NV_DIAG_SIZE_SHIFT	0x10		


#define MS02NV_ID		0x03021966	
#define MS02NV_VALID_ID		0xbd100248	
#define MS02NV_SLOT_SIZE	0x800000	


typedef volatile u32 ms02nv_uint;

struct ms02nv_private {
	struct mtd_info *next;
	struct {
		struct resource *module;
		struct resource *diag_ram;
		struct resource *user_ram;
		struct resource *csr;
	} resource;
	u_char *addr;
	size_t size;
	u_char *uaddr;
};
