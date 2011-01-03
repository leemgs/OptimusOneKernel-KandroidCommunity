#include <linux/io.h>

#include <asm/trampoline.h>
#include <asm/e820.h>

#if defined(CONFIG_X86_64) && defined(CONFIG_ACPI_SLEEP)
#define __trampinit
#define __trampinitdata
#else
#define __trampinit __cpuinit
#define __trampinitdata __cpuinitdata
#endif


unsigned char *__trampinitdata trampoline_base = __va(TRAMPOLINE_BASE);

void __init reserve_trampoline_memory(void)
{
#ifdef CONFIG_X86_32
	
	reserve_early(PAGE_SIZE, PAGE_SIZE + PAGE_SIZE, "EX TRAMPOLINE");
#endif
	
	reserve_early(TRAMPOLINE_BASE, TRAMPOLINE_BASE + TRAMPOLINE_SIZE,
			"TRAMPOLINE");
}


unsigned long __trampinit setup_trampoline(void)
{
	memcpy(trampoline_base, trampoline_data, TRAMPOLINE_SIZE);
	return virt_to_phys(trampoline_base);
}
