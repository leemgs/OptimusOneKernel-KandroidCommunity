

#include "imx074.h"
const struct reg_struct_init imx074_reg_init[1] = {
	{
		
		0x2D,	
		0x02,	
		0x4B,	
		0x03,	
		0x03,	
		0x80,	
		0x08,	
		0x37,	
		0x01,	
		0x05,	
		0x26,	
		0x60,	
		0x24,	
		0x34,	
		0xc0,	
		0x09,	
		0x07,	
		0x30,	
		0x00,	
		0x08,	
		0x1c,	
		0x32,	
		0x83,	
		0x04,	
		0x78,	
		0x82,	
		0x05,	
		0x04,	
		0x11,	
		0x02,	
		0x0c,	
		0x06,	
		0x08,	
		0x04,	
		0x08,	
		0x06,	
		0x01,	
		0x00,	

	}
};


const struct reg_struct	imx074_reg_pat[2] = {
	
	{
		0x06, 
		0x48, 
		0x00, 
		0x2F, 
		0x08, 
		0x38, 
		0x06, 
		0x18, 
		0x01, 
		0x03, 
		0x01, 
		0x03, 
		0x80, 
		0x16, 
		0x24, 
		0x53, 
		0x00,
		0x80, 
		0x83, 
	},

	
	{
		0x0C, 
		0x90, 
		0x00, 
		0x2F, 
		0x10, 
		0x70, 
		0x0c, 
		0x30, 
		0x01, 
		0x01, 
		0x01, 
		0x01, 
		0x00, 
		0x06, 
		0x24, 
		0x53, 
		0x00, 
		0x00, 
		0x03, 
	}
};
struct imx074_reg imx074_regs = {
	.reg_pat_init = &imx074_reg_init[0],
	.reg_pat = &imx074_reg_pat[0],
};
