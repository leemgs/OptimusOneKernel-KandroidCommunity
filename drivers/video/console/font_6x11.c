





#include <linux/font.h>

#define FONTDATAMAX (11*256)

static const unsigned char fontdata_6x11[FONTDATAMAX] = {

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0x84, 
	0xcc, 
	0x84, 
	0xb4, 
	0x84, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0xfc, 
	0xb4, 
	0xfc, 
	0xcc, 
	0xfc, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x28, 
	0x7c, 
	0x7c, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x38, 
	0x7c, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x38, 
	0x38, 
	0x6c, 
	0x6c, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x38, 
	0x7c, 
	0x7c, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x30, 
	0x78, 
	0x30, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0xff, 
	0xff, 
	0xff, 
	0xcf, 
	0x87, 
	0xcf, 
	0xff, 
	0xff, 
	0xff, 
	0xff, 
	0xff, 

	
	0x00, 
	0x00, 
	0x30, 
	0x48, 
	0x84, 
	0x48, 
	0x30, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0xff, 
	0xff, 
	0xcf, 
	0xb7, 
	0x7b, 
	0xb7, 
	0xcf, 
	0xff, 
	0xff, 
	0xff, 
	0xff, 

	
	0x00, 
	0x3c, 
	0x14, 
	0x20, 
	0x78, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x38, 
	0x10, 
	0x7c, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x3c, 
	0x24, 
	0x3c, 
	0x20, 
	0x20, 
	0xe0, 
	0xc0, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x44, 
	0x7c, 
	0x44, 
	0x44, 
	0xcc, 
	0xcc, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x54, 
	0x38, 
	0x6c, 
	0x38, 
	0x54, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x40, 
	0x60, 
	0x70, 
	0x7c, 
	0x70, 
	0x60, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x04, 
	0x0c, 
	0x1c, 
	0x7c, 
	0x1c, 
	0x0c, 
	0x04, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x38, 
	0x54, 
	0x10, 
	0x54, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x48, 
	0x48, 
	0x48, 
	0x48, 
	0x48, 
	0x00, 
	0x48, 
	0x00, 
	0x00, 
	0x00, 

	
	0x3c, 
	0x54, 
	0x54, 
	0x54, 
	0x3c, 
	0x14, 
	0x14, 
	0x14, 
	0x00, 
	0x00, 
	0x00, 

	
	0x38, 
	0x44, 
	0x24, 
	0x50, 
	0x48, 
	0x24, 
	0x14, 
	0x48, 
	0x44, 
	0x38, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xf8, 
	0xf8, 
	0xf8, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x38, 
	0x54, 
	0x10, 
	0x54, 
	0x38, 
	0x10, 
	0x7c, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x38, 
	0x54, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x54, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x08, 
	0x7c, 
	0x08, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x20, 
	0x7c, 
	0x20, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x40, 
	0x40, 
	0x40, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x48, 
	0x84, 
	0xfc, 
	0x84, 
	0x48, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x10, 
	0x38, 
	0x38, 
	0x7c, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x7c, 
	0x7c, 
	0x38, 
	0x38, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x28, 
	0x28, 
	0x28, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x28, 
	0x7c, 
	0x28, 
	0x28, 
	0x7c, 
	0x28, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x38, 
	0x54, 
	0x50, 
	0x38, 
	0x14, 
	0x54, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 

	
	0x00, 
	0x64, 
	0x64, 
	0x08, 
	0x10, 
	0x20, 
	0x4c, 
	0x4c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x30, 
	0x48, 
	0x50, 
	0x20, 
	0x54, 
	0x48, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x04, 
	0x08, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x08, 
	0x04, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x10, 
	0x20, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x54, 
	0x38, 
	0x54, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x10, 
	0x7c, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x30, 
	0x30, 
	0x10, 
	0x20, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x18, 
	0x18, 
	0x00, 
	0x00, 
	0x00, 

	
	0x04, 
	0x04, 
	0x08, 
	0x08, 
	0x10, 
	0x10, 
	0x20, 
	0x20, 
	0x40, 
	0x40, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x4c, 
	0x54, 
	0x64, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x08, 
	0x18, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x1c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x04, 
	0x08, 
	0x10, 
	0x20, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x04, 
	0x18, 
	0x04, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x08, 
	0x18, 
	0x28, 
	0x48, 
	0x7c, 
	0x08, 
	0x1c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x40, 
	0x78, 
	0x04, 
	0x04, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x40, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x04, 
	0x04, 
	0x08, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x38, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x04, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x18, 
	0x18, 
	0x00, 
	0x18, 
	0x18, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x30, 
	0x30, 
	0x00, 
	0x30, 
	0x30, 
	0x10, 
	0x20, 
	0x00, 

	
	0x00, 
	0x04, 
	0x08, 
	0x10, 
	0x20, 
	0x10, 
	0x08, 
	0x04, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x00, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x20, 
	0x10, 
	0x08, 
	0x04, 
	0x08, 
	0x10, 
	0x20, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x04, 
	0x08, 
	0x10, 
	0x00, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x74, 
	0x54, 
	0x78, 
	0x40, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x7c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0x44, 
	0x44, 
	0x78, 
	0x44, 
	0x44, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x40, 
	0x40, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x40, 
	0x40, 
	0x78, 
	0x40, 
	0x40, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x40, 
	0x40, 
	0x78, 
	0x40, 
	0x40, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x4c, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x7c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x04, 
	0x04, 
	0x04, 
	0x04, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x48, 
	0x50, 
	0x60, 
	0x50, 
	0x48, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x40, 
	0x40, 
	0x40, 
	0x40, 
	0x40, 
	0x40, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x6c, 
	0x54, 
	0x54, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x64, 
	0x54, 
	0x4c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0x44, 
	0x44, 
	0x78, 
	0x40, 
	0x40, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x54, 
	0x38, 
	0x04, 
	0x00, 
	0x00, 

	
	0x00, 
	0x78, 
	0x44, 
	0x44, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x38, 
	0x04, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x28, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x54, 
	0x54, 
	0x6c, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x28, 
	0x10, 
	0x28, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x28, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x04, 
	0x08, 
	0x10, 
	0x20, 
	0x40, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x0c, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x0c, 
	0x00, 
	0x00, 

	
	0x40, 
	0x40, 
	0x20, 
	0x20, 
	0x10, 
	0x10, 
	0x08, 
	0x08, 
	0x04, 
	0x04, 
	0x00, 

	
	0x30, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x30, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x28, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x08, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x40, 
	0x40, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x04, 
	0x04, 
	0x3c, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x44, 
	0x7c, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x0c, 
	0x10, 
	0x38, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x04, 
	0x38, 
	0x00, 

	
	0x00, 
	0x40, 
	0x40, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x00, 
	0x30, 
	0x10, 
	0x10, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x00, 
	0x30, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x60, 
	0x00, 

	
	0x00, 
	0x40, 
	0x40, 
	0x48, 
	0x50, 
	0x70, 
	0x48, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x30, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x78, 
	0x54, 
	0x54, 
	0x54, 
	0x54, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x58, 
	0x64, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x78, 
	0x44, 
	0x44, 
	0x44, 
	0x78, 
	0x40, 
	0x40, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x04, 
	0x04, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x58, 
	0x64, 
	0x40, 
	0x40, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x40, 
	0x38, 
	0x04, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x10, 
	0x38, 
	0x10, 
	0x10, 
	0x10, 
	0x0c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x28, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x54, 
	0x54, 
	0x54, 
	0x54, 
	0x28, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x44, 
	0x28, 
	0x10, 
	0x28, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x04, 
	0x38, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x08, 
	0x10, 
	0x20, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x04, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x10, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x04, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x08, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x20, 

	
	0x00, 
	0x34, 
	0x58, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x10, 
	0x28, 
	0x44, 
	0x44, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x40, 
	0x40, 
	0x44, 
	0x38, 
	0x10, 
	0x20, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x08, 
	0x10, 
	0x00, 
	0x38, 
	0x44, 
	0x7c, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x28, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x08, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x18, 
	0x24, 
	0x18, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x44, 
	0x40, 
	0x40, 
	0x3c, 
	0x10, 
	0x20, 
	0x00, 

	
	0x10, 
	0x28, 
	0x00, 
	0x38, 
	0x44, 
	0x7c, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x38, 
	0x44, 
	0x7c, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x00, 
	0x38, 
	0x44, 
	0x7c, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x28, 
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x84, 
	0x38, 
	0x44, 
	0x44, 
	0x7c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x58, 
	0x38, 
	0x44, 
	0x44, 
	0x7c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x7c, 
	0x40, 
	0x40, 
	0x78, 
	0x40, 
	0x40, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x54, 
	0x5c, 
	0x50, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x3c, 
	0x50, 
	0x50, 
	0x78, 
	0x50, 
	0x50, 
	0x5c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x28, 
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x28, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x10, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x28, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x3c, 
	0x04, 
	0x38, 
	0x00, 

	
	0x84, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x88, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x38, 
	0x54, 
	0x50, 
	0x54, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 

	
	0x30, 
	0x48, 
	0x40, 
	0x70, 
	0x40, 
	0x40, 
	0x44, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x44, 
	0x28, 
	0x7c, 
	0x10, 
	0x7c, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x70, 
	0x48, 
	0x70, 
	0x48, 
	0x5c, 
	0x48, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x0c, 
	0x10, 
	0x10, 
	0x38, 
	0x10, 
	0x10, 
	0x60, 
	0x00, 
	0x00, 
	0x00, 

	
	0x08, 
	0x10, 
	0x00, 
	0x3c, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x08, 
	0x10, 
	0x00, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x08, 
	0x10, 
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x08, 
	0x10, 
	0x00, 
	0x44, 
	0x44, 
	0x44, 
	0x4c, 
	0x34, 
	0x00, 
	0x00, 
	0x00, 

	
	0x34, 
	0x58, 
	0x00, 
	0x58, 
	0x64, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x58, 
	0x44, 
	0x64, 
	0x54, 
	0x4c, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x1c, 
	0x24, 
	0x24, 
	0x1c, 
	0x00, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x18, 
	0x24, 
	0x24, 
	0x18, 
	0x00, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x00, 
	0x10, 
	0x20, 
	0x40, 
	0x44, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x40, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x04, 
	0x04, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x20, 
	0x60, 
	0x24, 
	0x28, 
	0x10, 
	0x28, 
	0x44, 
	0x08, 
	0x1c, 
	0x00, 
	0x00, 

	
	0x20, 
	0x60, 
	0x24, 
	0x28, 
	0x10, 
	0x28, 
	0x58, 
	0x3c, 
	0x08, 
	0x00, 
	0x00, 

	
	0x00, 
	0x08, 
	0x00, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x08, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x24, 
	0x48, 
	0x48, 
	0x24, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x48, 
	0x24, 
	0x24, 
	0x48, 
	0x00, 
	0x00, 
	0x00, 

	
	0x11, 
	0x44, 
	0x11, 
	0x44, 
	0x11, 
	0x44, 
	0x11, 
	0x44, 
	0x11, 
	0x44, 
	0x11, 

	
	0x55, 
	0xaa, 
	0x55, 
	0xaa, 
	0x55, 
	0xaa, 
	0x55, 
	0xaa, 
	0x55, 
	0xaa, 
	0x55, 

	
	0xdd, 
	0x77, 
	0xdd, 
	0x77, 
	0xdd, 
	0x77, 
	0xdd, 
	0x77, 
	0xdd, 
	0x77, 
	0xdd, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0xf0, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0xf0, 
	0x10, 
	0xf0, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0xe8, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xf8, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x00, 
	0x00, 
	0x00, 
	0xf0, 
	0x10, 
	0xf0, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x28, 
	0x28, 
	0x28, 
	0xe8, 
	0x08, 
	0xe8, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x00, 
	0x00, 
	0x00, 
	0xf8, 
	0x08, 
	0xe8, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0xe8, 
	0x08, 
	0xf8, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0xf8, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x10, 
	0x10, 
	0xf0, 
	0x10, 
	0xf0, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xf0, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x1c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x1c, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0xfc, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x1c, 
	0x10, 
	0x1c, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x2c, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0x2c, 
	0x20, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x20, 
	0x2c, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0xec, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0xec, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0x2c, 
	0x20, 
	0x2c, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x28, 
	0x28, 
	0x28, 
	0xec, 
	0x00, 
	0xec, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x10, 
	0x10, 
	0x10, 
	0xfc, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0xfc, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x10, 
	0x10, 
	0x10, 
	0x1c, 
	0x10, 
	0x1c, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x1c, 
	0x10, 
	0x1c, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0xfc, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 

	
	0x10, 
	0x10, 
	0x10, 
	0xfc, 
	0x10, 
	0xfc, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0xf0, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x1f, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 

	
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 
	0xe0, 

	
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 
	0x1c, 

	
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x24, 
	0x58, 
	0x50, 
	0x54, 
	0x2c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x18, 
	0x24, 
	0x44, 
	0x48, 
	0x48, 
	0x44, 
	0x44, 
	0x58, 
	0x40, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x44, 
	0x44, 
	0x40, 
	0x40, 
	0x40, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x7c, 
	0x28, 
	0x28, 
	0x28, 
	0x28, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x7c, 
	0x24, 
	0x10, 
	0x08, 
	0x10, 
	0x24, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x3c, 
	0x48, 
	0x48, 
	0x48, 
	0x30, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x48, 
	0x48, 
	0x48, 
	0x48, 
	0x74, 
	0x40, 
	0x40, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x6c, 
	0x98, 
	0x10, 
	0x10, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x10, 
	0x38, 
	0x44, 
	0x38, 
	0x10, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x4c, 
	0x54, 
	0x64, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x28, 
	0x6c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x08, 
	0x0c, 
	0x14, 
	0x24, 
	0x24, 
	0x18, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x38, 
	0x54, 
	0x54, 
	0x54, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x04, 
	0x38, 
	0x44, 
	0x38, 
	0x40, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x3c, 
	0x40, 
	0x40, 
	0x78, 
	0x40, 
	0x40, 
	0x3c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x38, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x44, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0xfc, 
	0x00, 
	0xfc, 
	0x00, 
	0xfc, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x10, 
	0x7c, 
	0x10, 
	0x10, 
	0x7c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x10, 
	0x08, 
	0x04, 
	0x08, 
	0x10, 
	0x00, 
	0x1c, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x08, 
	0x10, 
	0x20, 
	0x10, 
	0x08, 
	0x00, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x0c, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 

	
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x10, 
	0x60, 
	0x00, 

	
	0x00, 
	0x00, 
	0x10, 
	0x00, 
	0x7c, 
	0x00, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x34, 
	0x48, 
	0x00, 
	0x34, 
	0x48, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x18, 
	0x24, 
	0x24, 
	0x18, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x10, 
	0x38, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x10, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x0c, 
	0x08, 
	0x10, 
	0x50, 
	0x20, 
	0x20, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x50, 
	0x28, 
	0x28, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x70, 
	0x08, 
	0x20, 
	0x78, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x38, 
	0x38, 
	0x38, 
	0x38, 
	0x38, 
	0x38, 
	0x00, 
	0x00, 
	0x00, 

	
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 

};


const struct font_desc font_vga_6x11 = {
	.idx	= VGA6x11_IDX,
	.name	= "ProFont6x11",
	.width	= 6,
	.height	= 11,
	.data	= fontdata_6x11,
	
	.pref	= -2000,
};
