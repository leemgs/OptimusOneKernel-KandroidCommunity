#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/efi.h>
#include <linux/dmi.h>
#include <linux/sched.h>
#include <linux/tboot.h>
#include <acpi/reboot.h>
#include <asm/io.h>
#include <asm/apic.h>
#include <asm/desc.h>
#include <asm/hpet.h>
#include <asm/pgtable.h>
#include <asm/proto.h>
#include <asm/reboot_fixups.h>
#include <asm/reboot.h>
#include <asm/pci_x86.h>
#include <asm/virtext.h>
#include <asm/cpu.h>

#ifdef CONFIG_X86_32
# include <linux/ctype.h>
# include <linux/mc146818rtc.h>
#else
# include <asm/iommu.h>
#endif


void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

static const struct desc_ptr no_idt = {};
static int reboot_mode;
enum reboot_type reboot_type = BOOT_KBD;
int reboot_force;

#if defined(CONFIG_X86_32) && defined(CONFIG_SMP)
static int reboot_cpu = -1;
#endif


static int reboot_emergency;


bool port_cf9_safe = false;


static int __init reboot_setup(char *str)
{
	for (;;) {
		switch (*str) {
		case 'w':
			reboot_mode = 0x1234;
			break;

		case 'c':
			reboot_mode = 0;
			break;

#ifdef CONFIG_X86_32
#ifdef CONFIG_SMP
		case 's':
			if (isdigit(*(str+1))) {
				reboot_cpu = (int) (*(str+1) - '0');
				if (isdigit(*(str+2)))
					reboot_cpu = reboot_cpu*10 + (int)(*(str+2) - '0');
			}
				
			break;
#endif 

		case 'b':
#endif
		case 'a':
		case 'k':
		case 't':
		case 'e':
		case 'p':
			reboot_type = *str;
			break;

		case 'f':
			reboot_force = 1;
			break;
		}

		str = strchr(str, ',');
		if (str)
			str++;
		else
			break;
	}
	return 1;
}

__setup("reboot=", reboot_setup);


#ifdef CONFIG_X86_32



static int __init set_bios_reboot(const struct dmi_system_id *d)
{
	if (reboot_type != BOOT_BIOS) {
		reboot_type = BOOT_BIOS;
		printk(KERN_INFO "%s series board detected. Selecting BIOS-method for reboots.\n", d->ident);
	}
	return 0;
}

static struct dmi_system_id __initdata reboot_dmi_table[] = {
	{	
		.callback = set_bios_reboot,
		.ident = "Dell E520",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "Dell DM061"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell PowerEdge 1300",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Computer Corporation"),
			DMI_MATCH(DMI_PRODUCT_NAME, "PowerEdge 1300/"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell PowerEdge 300",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Computer Corporation"),
			DMI_MATCH(DMI_PRODUCT_NAME, "PowerEdge 300/"),
		},
	},
	{       
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 745",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 745"),
		},
	},
	{       
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 745",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 745"),
			DMI_MATCH(DMI_BOARD_NAME, "0MM599"),
		},
	},
	{       
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 745",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 745"),
			DMI_MATCH(DMI_BOARD_NAME, "0KW626"),
		},
	},
	{   
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 330",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 330"),
			DMI_MATCH(DMI_BOARD_NAME, "0KP561"),
		},
	},
	{   
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 360",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 360"),
			DMI_MATCH(DMI_BOARD_NAME, "0T656F"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell OptiPlex 760",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "OptiPlex 760"),
			DMI_MATCH(DMI_BOARD_NAME, "0G919G"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell PowerEdge 2400",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Computer Corporation"),
			DMI_MATCH(DMI_PRODUCT_NAME, "PowerEdge 2400"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell Precision T5400",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "Precision WorkStation T5400"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "HP Compaq Laptop",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
			DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell XPS710",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "Dell XPS710"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Dell DXP061",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "Dell DXP061"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "Sony VGN-Z540N",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Sony Corporation"),
			DMI_MATCH(DMI_PRODUCT_NAME, "VGN-Z540N"),
		},
	},
	{	
		.callback = set_bios_reboot,
		.ident = "CompuLab SBC-FITPC2",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "CompuLab"),
			DMI_MATCH(DMI_PRODUCT_NAME, "SBC-FITPC2"),
		},
	},
	{       
		.callback = set_bios_reboot,
		.ident = "ASUS P4S800",
		.matches = {
			DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC."),
			DMI_MATCH(DMI_BOARD_NAME, "P4S800"),
		},
	},
	{ }
};

static int __init reboot_init(void)
{
	dmi_check_system(reboot_dmi_table);
	return 0;
}
core_initcall(reboot_init);


static const unsigned long long
real_mode_gdt_entries [3] =
{
	0x0000000000000000ULL,	
	0x00009b000000ffffULL,	
	0x000093000100ffffULL	
};

static const struct desc_ptr
real_mode_gdt = { sizeof (real_mode_gdt_entries) - 1, (long)real_mode_gdt_entries },
real_mode_idt = { 0x3ff, 0 };


static const unsigned char real_mode_switch [] =
{
	0x66, 0x0f, 0x20, 0xc0,			
	0x66, 0x83, 0xe0, 0x11,			
	0x66, 0x0d, 0x00, 0x00, 0x00, 0x60,	
	0x66, 0x0f, 0x22, 0xc0,			
	0x66, 0x0f, 0x22, 0xd8,			
	0x66, 0x0f, 0x20, 0xc3,			
	0x66, 0x81, 0xe3, 0x00, 0x00, 0x00, 0x60,	
	0x74, 0x02,				
	0x0f, 0x09,				
	0x24, 0x10,				
	0x66, 0x0f, 0x22, 0xc0			
};
static const unsigned char jump_to_bios [] =
{
	0xea, 0x00, 0x00, 0xff, 0xff		
};


void machine_real_restart(const unsigned char *code, int length)
{
	local_irq_disable();

	
	spin_lock(&rtc_lock);
	CMOS_WRITE(0x00, 0x8f);
	spin_unlock(&rtc_lock);

	
	memcpy(swapper_pg_dir, swapper_pg_dir + KERNEL_PGD_BOUNDARY,
		sizeof(swapper_pg_dir [0]) * KERNEL_PGD_PTRS);

	
	load_cr3(swapper_pg_dir);

	
	*((unsigned short *)0x472) = reboot_mode;

	
	memcpy((void *)(0x1000 - sizeof(real_mode_switch) - 100),
		real_mode_switch, sizeof (real_mode_switch));
	memcpy((void *)(0x1000 - 100), code, length);

	
	load_idt(&real_mode_idt);

	
	load_gdt(&real_mode_gdt);

	
	__asm__ __volatile__ ("movl $0x0010,%%eax\n"
				"\tmovl %%eax,%%ds\n"
				"\tmovl %%eax,%%es\n"
				"\tmovl %%eax,%%fs\n"
				"\tmovl %%eax,%%gs\n"
				"\tmovl %%eax,%%ss" : : : "eax");

	
	__asm__ __volatile__ ("ljmp $0x0008,%0"
				:
				: "i" ((void *)(0x1000 - sizeof (real_mode_switch) - 100)));
}
#ifdef CONFIG_APM_MODULE
EXPORT_SYMBOL(machine_real_restart);
#endif

#endif 


static int __init set_pci_reboot(const struct dmi_system_id *d)
{
	if (reboot_type != BOOT_CF9) {
		reboot_type = BOOT_CF9;
		printk(KERN_INFO "%s series board detected. "
		       "Selecting PCI-method for reboots.\n", d->ident);
	}
	return 0;
}

static struct dmi_system_id __initdata pci_reboot_dmi_table[] = {
	{	
		.callback = set_pci_reboot,
		.ident = "Apple MacBook5",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBook5"),
		},
	},
	{	
		.callback = set_pci_reboot,
		.ident = "Apple MacBookPro5",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro5"),
		},
	},
	{	
		.callback = set_pci_reboot,
		.ident = "Apple Macmini3,1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "Macmini3,1"),
		},
	},
	{ }
};

static int __init pci_reboot_init(void)
{
	dmi_check_system(pci_reboot_dmi_table);
	return 0;
}
core_initcall(pci_reboot_init);

static inline void kb_wait(void)
{
	int i;

	for (i = 0; i < 0x10000; i++) {
		if ((inb(0x64) & 0x02) == 0)
			break;
		udelay(2);
	}
}

static void vmxoff_nmi(int cpu, struct die_args *args)
{
	cpu_emergency_vmxoff();
}


static void emergency_vmx_disable_all(void)
{
	
	local_irq_disable();

	
	if (cpu_has_vmx() && cpu_vmx_enabled()) {
		
		cpu_vmxoff();

		
		nmi_shootdown_cpus(vmxoff_nmi);

	}
}


void __attribute__((weak)) mach_reboot_fixups(void)
{
}

static void native_machine_emergency_restart(void)
{
	int i;

	if (reboot_emergency)
		emergency_vmx_disable_all();

	tboot_shutdown(TB_SHUTDOWN_REBOOT);

	
	*((unsigned short *)__va(0x472)) = reboot_mode;

	for (;;) {
		
		switch (reboot_type) {
		case BOOT_KBD:
			mach_reboot_fixups(); 

			for (i = 0; i < 10; i++) {
				kb_wait();
				udelay(50);
				outb(0xfe, 0x64); 
				udelay(50);
			}

		case BOOT_TRIPLE:
			load_idt(&no_idt);
			__asm__ __volatile__("int3");

			reboot_type = BOOT_KBD;
			break;

#ifdef CONFIG_X86_32
		case BOOT_BIOS:
			machine_real_restart(jump_to_bios, sizeof(jump_to_bios));

			reboot_type = BOOT_KBD;
			break;
#endif

		case BOOT_ACPI:
			acpi_reboot();
			reboot_type = BOOT_KBD;
			break;

		case BOOT_EFI:
			if (efi_enabled)
				efi.reset_system(reboot_mode ?
						 EFI_RESET_WARM :
						 EFI_RESET_COLD,
						 EFI_SUCCESS, 0, NULL);
			reboot_type = BOOT_KBD;
			break;

		case BOOT_CF9:
			port_cf9_safe = true;
			

		case BOOT_CF9_COND:
			if (port_cf9_safe) {
				u8 cf9 = inb(0xcf9) & ~6;
				outb(cf9|2, 0xcf9); 
				udelay(50);
				outb(cf9|6, 0xcf9); 
				udelay(50);
			}
			reboot_type = BOOT_KBD;
			break;
		}
	}
}

void native_machine_shutdown(void)
{
	
#ifdef CONFIG_SMP

	
	int reboot_cpu_id = 0;

#ifdef CONFIG_X86_32
	
	if ((reboot_cpu != -1) && (reboot_cpu < nr_cpu_ids) &&
		cpu_online(reboot_cpu))
		reboot_cpu_id = reboot_cpu;
#endif

	
	if (!cpu_online(reboot_cpu_id))
		reboot_cpu_id = smp_processor_id();

	
	set_cpus_allowed_ptr(current, cpumask_of(reboot_cpu_id));

	
	smp_send_stop();
#endif

	lapic_shutdown();

#ifdef CONFIG_X86_IO_APIC
	disable_IO_APIC();
#endif

#ifdef CONFIG_HPET_TIMER
	hpet_disable();
#endif

#ifdef CONFIG_X86_64
	pci_iommu_shutdown();
#endif
}

static void __machine_emergency_restart(int emergency)
{
	reboot_emergency = emergency;
	machine_ops.emergency_restart();
}

static void native_machine_restart(char *__unused)
{
	printk("machine restart\n");

	if (!reboot_force)
		machine_shutdown();
	__machine_emergency_restart(0);
}

static void native_machine_halt(void)
{
	
	machine_shutdown();

	tboot_shutdown(TB_SHUTDOWN_HALT);

	
	stop_this_cpu(NULL);
}

static void native_machine_power_off(void)
{
	if (pm_power_off) {
		if (!reboot_force)
			machine_shutdown();
		pm_power_off();
	}
	
	tboot_shutdown(TB_SHUTDOWN_HALT);
}

struct machine_ops machine_ops = {
	.power_off = native_machine_power_off,
	.shutdown = native_machine_shutdown,
	.emergency_restart = native_machine_emergency_restart,
	.restart = native_machine_restart,
	.halt = native_machine_halt,
#ifdef CONFIG_KEXEC
	.crash_shutdown = native_machine_crash_shutdown,
#endif
};

void machine_power_off(void)
{
	machine_ops.power_off();
}

void machine_shutdown(void)
{
	machine_ops.shutdown();
}

void machine_emergency_restart(void)
{
	__machine_emergency_restart(1);
}

void machine_restart(char *cmd)
{
	machine_ops.restart(cmd);
}

void machine_halt(void)
{
	machine_ops.halt();
}

#ifdef CONFIG_KEXEC
void machine_crash_shutdown(struct pt_regs *regs)
{
	machine_ops.crash_shutdown(regs);
}
#endif


#if defined(CONFIG_SMP)


static int crashing_cpu;
static nmi_shootdown_cb shootdown_callback;

static atomic_t waiting_for_crash_ipi;

static int crash_nmi_callback(struct notifier_block *self,
			unsigned long val, void *data)
{
	int cpu;

	if (val != DIE_NMI_IPI)
		return NOTIFY_OK;

	cpu = raw_smp_processor_id();

	
	if (cpu == crashing_cpu)
		return NOTIFY_STOP;
	local_irq_disable();

	shootdown_callback(cpu, (struct die_args *)data);

	atomic_dec(&waiting_for_crash_ipi);
	
	halt();
	for (;;)
		cpu_relax();

	return 1;
}

static void smp_send_nmi_allbutself(void)
{
	apic->send_IPI_allbutself(NMI_VECTOR);
}

static struct notifier_block crash_nmi_nb = {
	.notifier_call = crash_nmi_callback,
};


void nmi_shootdown_cpus(nmi_shootdown_cb callback)
{
	unsigned long msecs;
	local_irq_disable();

	
	crashing_cpu = safe_smp_processor_id();

	shootdown_callback = callback;

	atomic_set(&waiting_for_crash_ipi, num_online_cpus() - 1);
	
	if (register_die_notifier(&crash_nmi_nb))
		return;		
	
	wmb();

	smp_send_nmi_allbutself();

	msecs = 1000; 
	while ((atomic_read(&waiting_for_crash_ipi) > 0) && msecs) {
		mdelay(1);
		msecs--;
	}

	
}
#else 
void nmi_shootdown_cpus(nmi_shootdown_cb callback)
{
	
}
#endif
