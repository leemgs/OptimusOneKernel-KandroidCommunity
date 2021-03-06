

#include <mach/hardware.h>
#include <asm/mach-types.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	local_irq_disable();

	
	if (machine_is_ixdp2401()) {
		ixp2000_reg_write(IXDP2X01_CPLD_FLASH_REG,
					((0 >> IXDP2X01_FLASH_WINDOW_BITS)
						| IXDP2X01_CPLD_FLASH_INTERN));
		ixp2000_reg_wrb(IXDP2X01_CPLD_RESET_REG, 0xffffffff);
	}

	
	if (machine_is_ixdp2801() || machine_is_ixdp28x5()) {
		unsigned long reset_reg = *IXDP2X01_CPLD_RESET_REG;

		reset_reg = 0x55AA0000 | (reset_reg & 0x0000FFFF);
		ixp2000_reg_write(IXDP2X01_CPLD_RESET_REG, reset_reg);
		ixp2000_reg_wrb(IXDP2X01_CPLD_RESET_REG, 0x80000000);
	}

	ixp2000_reg_wrb(IXP2000_RESET0, RSTALL);
}
