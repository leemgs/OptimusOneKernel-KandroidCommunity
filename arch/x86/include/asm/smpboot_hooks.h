

static inline void smpboot_clear_io_apic_irqs(void)
{
#ifdef CONFIG_X86_IO_APIC
	io_apic_irqs = 0;
#endif
}

static inline void smpboot_setup_warm_reset_vector(unsigned long start_eip)
{
	CMOS_WRITE(0xa, 0xf);
	local_flush_tlb();
	pr_debug("1.\n");
	*((volatile unsigned short *)phys_to_virt(apic->trampoline_phys_high)) =
								 start_eip >> 4;
	pr_debug("2.\n");
	*((volatile unsigned short *)phys_to_virt(apic->trampoline_phys_low)) =
							 start_eip & 0xf;
	pr_debug("3.\n");
}

static inline void smpboot_restore_warm_reset_vector(void)
{
	
	local_flush_tlb();

	
	CMOS_WRITE(0, 0xf);

	*((volatile long *)phys_to_virt(apic->trampoline_phys_low)) = 0;
}

static inline void __init smpboot_setup_io_apic(void)
{
#ifdef CONFIG_X86_IO_APIC
	
	if (!skip_ioapic_setup && nr_ioapics)
		setup_IO_APIC();
	else {
		nr_ioapics = 0;
		localise_nmi_watchdog();
	}
#endif
}

static inline void smpboot_clear_io_apic(void)
{
#ifdef CONFIG_X86_IO_APIC
	nr_ioapics = 0;
#endif
}
