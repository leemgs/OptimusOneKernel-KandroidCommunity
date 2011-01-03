
#include <asm/mach-types.h>


#define DC21285_BASE ((volatile unsigned int *)0x42000160)
#define SER0_BASE    ((volatile unsigned char *)0x7c0003f8)

static inline void putc(char c)
{
	if (machine_is_netwinder()) {
		while ((SER0_BASE[5] & 0x60) != 0x60)
			barrier();
		SER0_BASE[0] = c;
	} else {
		while (DC21285_BASE[6] & 8);
		DC21285_BASE[0] = c;
	}
}

static inline void flush(void)
{
}


#define arch_decomp_setup()
#define arch_decomp_wdog()
