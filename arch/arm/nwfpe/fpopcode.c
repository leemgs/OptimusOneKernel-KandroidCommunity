

#include "fpa11.h"
#include "softfloat.h"
#include "fpopcode.h"
#include "fpsr.h"
#include "fpmodule.h"
#include "fpmodule.inl"

#ifdef CONFIG_FPE_NWFPE_XP
const floatx80 floatx80Constant[] = {
	{ .high = 0x0000, .low = 0x0000000000000000ULL},
	{ .high = 0x3fff, .low = 0x8000000000000000ULL},
	{ .high = 0x4000, .low = 0x8000000000000000ULL},
	{ .high = 0x4000, .low = 0xc000000000000000ULL},
	{ .high = 0x4001, .low = 0x8000000000000000ULL},
	{ .high = 0x4001, .low = 0xa000000000000000ULL},
	{ .high = 0x3ffe, .low = 0x8000000000000000ULL},
	{ .high = 0x4002, .low = 0xa000000000000000ULL},
};
#endif

const float64 float64Constant[] = {
	0x0000000000000000ULL,	
	0x3ff0000000000000ULL,	
	0x4000000000000000ULL,	
	0x4008000000000000ULL,	
	0x4010000000000000ULL,	
	0x4014000000000000ULL,	
	0x3fe0000000000000ULL,	
	0x4024000000000000ULL	
};

const float32 float32Constant[] = {
	0x00000000,		
	0x3f800000,		
	0x40000000,		
	0x40400000,		
	0x40800000,		
	0x40a00000,		
	0x3f000000,		
	0x41200000		
};


static const unsigned short aCC[16] = {
	0xF0F0,			
	0x0F0F,			
	0xCCCC,			
	0x3333,			
	0xFF00,			
	0x00FF,			
	0xAAAA,			
	0x5555,			
	0x0C0C,			
	0xF3F3,			
	0xAA55,			
	0x55AA,			
	0x0A05,			
	0xF5FA,			
	0xFFFF,			
	0			
};

unsigned int checkCondition(const unsigned int opcode, const unsigned int ccodes)
{
	return (aCC[opcode >> 28] >> (ccodes >> 28)) & 1;
}
