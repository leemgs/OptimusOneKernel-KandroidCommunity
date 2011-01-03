

#include <linux/acpi.h>
#include <linux/bootmem.h>
#include <linux/dmi.h>
#include <linux/cpumask.h>
#include <asm/segment.h>
#include <asm/desc.h>

#include "realmode/wakeup.h"
#include "sleep.h"

unsigned long acpi_wakeup_address;
unsigned long acpi_realmode_flags;


static unsigned long acpi_realmode;

#if defined(CONFIG_SMP) && defined(CONFIG_64BIT)
static char temp_stack[4096];
#endif


int acpi_save_state_mem(void)
{
	struct wakeup_header *header;

	if (!acpi_realmode) {
		printk(KERN_ERR "Could not allocate memory during boot, "
		       "S3 disabled\n");
		return -ENOMEM;
	}
	memcpy((void *)acpi_realmode, &wakeup_code_start, WAKEUP_SIZE);

	header = (struct wakeup_header *)(acpi_realmode + HEADER_OFFSET);
	if (header->signature != 0x51ee1111) {
		printk(KERN_ERR "wakeup header does not match\n");
		return -EINVAL;
	}

	header->video_mode = saved_video_mode;

	header->wakeup_jmp_seg = acpi_wakeup_address >> 4;

	

	
	header->wakeup_gdt[0] =
		(u64)(sizeof(header->wakeup_gdt) - 1) +
		((u64)(acpi_wakeup_address +
			((char *)&header->wakeup_gdt - (char *)acpi_realmode))
				<< 16);
	
	header->wakeup_gdt[1] =
		GDT_ENTRY(0x809b, acpi_wakeup_address, 0xfffff);
	
	header->wakeup_gdt[2] =
		GDT_ENTRY(0x8093, acpi_wakeup_address, 0xfffff);

#ifndef CONFIG_64BIT
	store_gdt((struct desc_ptr *)&header->pmode_gdt);

	header->pmode_efer_low = nx_enabled;
	if (header->pmode_efer_low & 1) {
		
		rdmsr(MSR_EFER, header->pmode_efer_low,
			header->pmode_efer_high);
	}
#endif 

	header->pmode_cr0 = read_cr0();
	header->pmode_cr4 = read_cr4_safe();
	header->realmode_flags = acpi_realmode_flags;
	header->real_magic = 0x12345678;

#ifndef CONFIG_64BIT
	header->pmode_entry = (u32)&wakeup_pmode_return;
	header->pmode_cr3 = (u32)(swsusp_pg_dir - __PAGE_OFFSET);
	saved_magic = 0x12345678;
#else 
	header->trampoline_segment = setup_trampoline() >> 4;
#ifdef CONFIG_SMP
	stack_start.sp = temp_stack + sizeof(temp_stack);
	early_gdt_descr.address =
			(unsigned long)get_cpu_gdt_table(smp_processor_id());
	initial_gs = per_cpu_offset(smp_processor_id());
#endif
	initial_code = (unsigned long)wakeup_long64;
       saved_magic = 0x123456789abcdef0L;
#endif 

	return 0;
}


void acpi_restore_state_mem(void)
{
}



void __init acpi_reserve_bootmem(void)
{
	if ((&wakeup_code_end - &wakeup_code_start) > WAKEUP_SIZE) {
		printk(KERN_ERR
		       "ACPI: Wakeup code way too big, S3 disabled.\n");
		return;
	}

	acpi_realmode = (unsigned long)alloc_bootmem_low(WAKEUP_SIZE);

	if (!acpi_realmode) {
		printk(KERN_ERR "ACPI: Cannot allocate lowmem, S3 disabled.\n");
		return;
	}

	acpi_wakeup_address = virt_to_phys((void *)acpi_realmode);
}


static int __init acpi_sleep_setup(char *str)
{
	while ((str != NULL) && (*str != '\0')) {
		if (strncmp(str, "s3_bios", 7) == 0)
			acpi_realmode_flags |= 1;
		if (strncmp(str, "s3_mode", 7) == 0)
			acpi_realmode_flags |= 2;
		if (strncmp(str, "s3_beep", 7) == 0)
			acpi_realmode_flags |= 4;
#ifdef CONFIG_HIBERNATION
		if (strncmp(str, "s4_nohwsig", 10) == 0)
			acpi_no_s4_hw_signature();
		if (strncmp(str, "s4_nonvs", 8) == 0)
			acpi_s4_no_nvs();
#endif
		if (strncmp(str, "old_ordering", 12) == 0)
			acpi_old_suspend_ordering();
		str = strchr(str, ',');
		if (str != NULL)
			str += strspn(str, ", \t");
	}
	return 1;
}

__setup("acpi_sleep=", acpi_sleep_setup);
