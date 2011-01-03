#ifndef _LINUX_AT24_H
#define _LINUX_AT24_H

#include <linux/types.h>
#include <linux/memory.h>



struct at24_platform_data {
	u32		byte_len;		
	u16		page_size;		
	u8		flags;
#define AT24_FLAG_ADDR16	0x80	
#define AT24_FLAG_READONLY	0x40	
#define AT24_FLAG_IRUGO		0x20	
#define AT24_FLAG_TAKE8ADDR	0x10	

	void		(*setup)(struct memory_accessor *, void *context);
	void		*context;
};

#endif 
