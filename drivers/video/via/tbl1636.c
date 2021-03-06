

#include "global.h"
struct IODATA COMMON_INIT_TBL_VT1636[] = {

	
	{0x10, 0xC0, 0x00},
	
	{0x0B, 0xFF, 0x40},
	
	{0x0C, 0xFF, 0x31},
	
	{0x0D, 0xFF, 0x31},
	
	{0x0E, 0xFF, 0x68},
	
	{0x0F, 0xFF, 0x68},
	
	{0x09, 0xA0, 0xA0},
	
	{0x10, 0x33, 0x13}
};

struct IODATA DUAL_CHANNEL_ENABLE_TBL_VT1636[] = {

	{0x08, 0xF0, 0xE0}	
};

struct IODATA SINGLE_CHANNEL_ENABLE_TBL_VT1636[] = {

	{0x08, 0xF0, 0x00}	
};

struct IODATA DITHERING_ENABLE_TBL_VT1636[] = {

	{0x0A, 0x70, 0x50}
};

struct IODATA DITHERING_DISABLE_TBL_VT1636[] = {

	{0x0A, 0x70, 0x00}
};

struct IODATA VDD_ON_TBL_VT1636[] = {

	{0x10, 0x20, 0x20}
};

struct IODATA VDD_OFF_TBL_VT1636[] = {

	{0x10, 0x20, 0x00}
};
