

#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/acpi_pmtmr.h>
#include <linux/efi.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/irq.h>
#include <linux/bootmem.h>
#include <linux/ioport.h>
#include <linux/pci.h>

#include <asm/pgtable.h>
#include <asm/io_apic.h>
#include <asm/apic.h>
#include <asm/io.h>
#include <asm/mpspec.h>
#include <asm/smp.h>

static int __initdata acpi_force = 0;
u32 acpi_rsdt_forced;
int acpi_disabled;
EXPORT_SYMBOL(acpi_disabled);

#ifdef	CONFIG_X86_64
# include <asm/proto.h>
#endif				

#define BAD_MADT_ENTRY(entry, end) (					    \
		(!entry) || (unsigned long)entry + sizeof(*entry) > end ||  \
		((struct acpi_subtable_header *)entry)->length < sizeof(*entry))

#define PREFIX			"ACPI: "

int acpi_noirq;				
int acpi_pci_disabled;		
EXPORT_SYMBOL(acpi_pci_disabled);
int acpi_ht __initdata = 1;	

int acpi_lapic;
int acpi_ioapic;
int acpi_strict;

u8 acpi_sci_flags __initdata;
int acpi_sci_override_gsi __initdata;
int acpi_skip_timer_override __initdata;
int acpi_use_timer_override __initdata;

#ifdef CONFIG_X86_LOCAL_APIC
static u64 acpi_lapic_addr __initdata = APIC_DEFAULT_PHYS_BASE;
#endif

#ifndef __HAVE_ARCH_CMPXCHG
#warning ACPI uses CMPXCHG, i486 and later hardware
#endif




enum acpi_irq_model_id acpi_irq_model = ACPI_IRQ_MODEL_PIC;



char *__init __acpi_map_table(unsigned long phys, unsigned long size)
{

	if (!phys || !size)
		return NULL;

	return early_ioremap(phys, size);
}
void __init __acpi_unmap_table(char *map, unsigned long size)
{
	if (!map || !size)
		return;

	early_iounmap(map, size);
}

#ifdef CONFIG_X86_LOCAL_APIC
static int __init acpi_parse_madt(struct acpi_table_header *table)
{
	struct acpi_table_madt *madt = NULL;

	if (!cpu_has_apic)
		return -EINVAL;

	madt = (struct acpi_table_madt *)table;
	if (!madt) {
		printk(KERN_WARNING PREFIX "Unable to map MADT\n");
		return -ENODEV;
	}

	if (madt->address) {
		acpi_lapic_addr = (u64) madt->address;

		printk(KERN_DEBUG PREFIX "Local APIC address 0x%08x\n",
		       madt->address);
	}

	default_acpi_madt_oem_check(madt->header.oem_id,
				    madt->header.oem_table_id);

	return 0;
}

static void __cpuinit acpi_register_lapic(int id, u8 enabled)
{
	unsigned int ver = 0;

	if (!enabled) {
		++disabled_cpus;
		return;
	}

	if (boot_cpu_physical_apicid != -1U)
		ver = apic_version[boot_cpu_physical_apicid];

	generic_processor_info(id, ver);
}

static int __init
acpi_parse_x2apic(struct acpi_subtable_header *header, const unsigned long end)
{
	struct acpi_madt_local_x2apic *processor = NULL;

	processor = (struct acpi_madt_local_x2apic *)header;

	if (BAD_MADT_ENTRY(processor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

#ifdef CONFIG_X86_X2APIC
	
	acpi_register_lapic(processor->local_apic_id,	
			    processor->lapic_flags & ACPI_MADT_ENABLED);
#else
	printk(KERN_WARNING PREFIX "x2apic entry ignored\n");
#endif

	return 0;
}

static int __init
acpi_parse_lapic(struct acpi_subtable_header * header, const unsigned long end)
{
	struct acpi_madt_local_apic *processor = NULL;

	processor = (struct acpi_madt_local_apic *)header;

	if (BAD_MADT_ENTRY(processor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	
	acpi_register_lapic(processor->id,	
			    processor->lapic_flags & ACPI_MADT_ENABLED);

	return 0;
}

static int __init
acpi_parse_sapic(struct acpi_subtable_header *header, const unsigned long end)
{
	struct acpi_madt_local_sapic *processor = NULL;

	processor = (struct acpi_madt_local_sapic *)header;

	if (BAD_MADT_ENTRY(processor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	acpi_register_lapic((processor->id << 8) | processor->eid,
			    processor->lapic_flags & ACPI_MADT_ENABLED);

	return 0;
}

static int __init
acpi_parse_lapic_addr_ovr(struct acpi_subtable_header * header,
			  const unsigned long end)
{
	struct acpi_madt_local_apic_override *lapic_addr_ovr = NULL;

	lapic_addr_ovr = (struct acpi_madt_local_apic_override *)header;

	if (BAD_MADT_ENTRY(lapic_addr_ovr, end))
		return -EINVAL;

	acpi_lapic_addr = lapic_addr_ovr->address;

	return 0;
}

static int __init
acpi_parse_x2apic_nmi(struct acpi_subtable_header *header,
		      const unsigned long end)
{
	struct acpi_madt_local_x2apic_nmi *x2apic_nmi = NULL;

	x2apic_nmi = (struct acpi_madt_local_x2apic_nmi *)header;

	if (BAD_MADT_ENTRY(x2apic_nmi, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	if (x2apic_nmi->lint != 1)
		printk(KERN_WARNING PREFIX "NMI not connected to LINT 1!\n");

	return 0;
}

static int __init
acpi_parse_lapic_nmi(struct acpi_subtable_header * header, const unsigned long end)
{
	struct acpi_madt_local_apic_nmi *lapic_nmi = NULL;

	lapic_nmi = (struct acpi_madt_local_apic_nmi *)header;

	if (BAD_MADT_ENTRY(lapic_nmi, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	if (lapic_nmi->lint != 1)
		printk(KERN_WARNING PREFIX "NMI not connected to LINT 1!\n");

	return 0;
}

#endif				

#ifdef CONFIG_X86_IO_APIC

static int __init
acpi_parse_ioapic(struct acpi_subtable_header * header, const unsigned long end)
{
	struct acpi_madt_io_apic *ioapic = NULL;

	ioapic = (struct acpi_madt_io_apic *)header;

	if (BAD_MADT_ENTRY(ioapic, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	mp_register_ioapic(ioapic->id,
			   ioapic->address, ioapic->global_irq_base);

	return 0;
}


static void __init acpi_sci_ioapic_setup(u32 gsi, u16 polarity, u16 trigger)
{
	if (trigger == 0)	
		trigger = 3;

	if (polarity == 0)	
		polarity = 3;

	
	if (acpi_sci_flags & ACPI_MADT_TRIGGER_MASK)
		trigger = (acpi_sci_flags & ACPI_MADT_TRIGGER_MASK) >> 2;

	if (acpi_sci_flags & ACPI_MADT_POLARITY_MASK)
		polarity = acpi_sci_flags & ACPI_MADT_POLARITY_MASK;

	
	mp_override_legacy_irq(gsi, polarity, trigger, gsi);

	
	acpi_sci_override_gsi = gsi;
	return;
}

static int __init
acpi_parse_int_src_ovr(struct acpi_subtable_header * header,
		       const unsigned long end)
{
	struct acpi_madt_interrupt_override *intsrc = NULL;

	intsrc = (struct acpi_madt_interrupt_override *)header;

	if (BAD_MADT_ENTRY(intsrc, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	if (intsrc->source_irq == acpi_gbl_FADT.sci_interrupt) {
		acpi_sci_ioapic_setup(intsrc->global_irq,
				      intsrc->inti_flags & ACPI_MADT_POLARITY_MASK,
				      (intsrc->inti_flags & ACPI_MADT_TRIGGER_MASK) >> 2);
		return 0;
	}

	if (acpi_skip_timer_override &&
	    intsrc->source_irq == 0 && intsrc->global_irq == 2) {
		printk(PREFIX "BIOS IRQ0 pin2 override ignored.\n");
		return 0;
	}

	mp_override_legacy_irq(intsrc->source_irq,
				intsrc->inti_flags & ACPI_MADT_POLARITY_MASK,
				(intsrc->inti_flags & ACPI_MADT_TRIGGER_MASK) >> 2,
				intsrc->global_irq);

	return 0;
}

static int __init
acpi_parse_nmi_src(struct acpi_subtable_header * header, const unsigned long end)
{
	struct acpi_madt_nmi_source *nmi_src = NULL;

	nmi_src = (struct acpi_madt_nmi_source *)header;

	if (BAD_MADT_ENTRY(nmi_src, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	

	return 0;
}

#endif				



void __init acpi_pic_sci_set_trigger(unsigned int irq, u16 trigger)
{
	unsigned int mask = 1 << irq;
	unsigned int old, new;

	
	old = inb(0x4d0) | (inb(0x4d1) << 8);

	
	new = acpi_noirq ? old : 0;

	
	switch (trigger) {
	case 1:		
		new &= ~mask;
		break;
	case 3:		
		new |= mask;
		break;
	}

	if (old == new)
		return;

	printk(PREFIX "setting ELCR to %04x (from %04x)\n", new, old);
	outb(new, 0x4d0);
	outb(new >> 8, 0x4d1);
}

int acpi_gsi_to_irq(u32 gsi, unsigned int *irq)
{
	*irq = gsi;
	return 0;
}


int acpi_register_gsi(struct device *dev, u32 gsi, int trigger, int polarity)
{
	unsigned int irq;
	unsigned int plat_gsi = gsi;

#ifdef CONFIG_PCI
	
	if (acpi_irq_model == ACPI_IRQ_MODEL_PIC) {
		if (trigger == ACPI_LEVEL_SENSITIVE)
			eisa_set_level_irq(gsi);
	}
#endif

#ifdef CONFIG_X86_IO_APIC
	if (acpi_irq_model == ACPI_IRQ_MODEL_IOAPIC) {
		plat_gsi = mp_register_gsi(dev, gsi, trigger, polarity);
	}
#endif
	acpi_gsi_to_irq(plat_gsi, &irq);
	return irq;
}


#ifdef CONFIG_ACPI_HOTPLUG_CPU

static int __cpuinit _acpi_map_lsapic(acpi_handle handle, int *pcpu)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct acpi_madt_local_apic *lapic;
	cpumask_var_t tmp_map, new_map;
	u8 physid;
	int cpu;
	int retval = -ENOMEM;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_MAT", NULL, &buffer)))
		return -EINVAL;

	if (!buffer.length || !buffer.pointer)
		return -EINVAL;

	obj = buffer.pointer;
	if (obj->type != ACPI_TYPE_BUFFER ||
	    obj->buffer.length < sizeof(*lapic)) {
		kfree(buffer.pointer);
		return -EINVAL;
	}

	lapic = (struct acpi_madt_local_apic *)obj->buffer.pointer;

	if (lapic->header.type != ACPI_MADT_TYPE_LOCAL_APIC ||
	    !(lapic->lapic_flags & ACPI_MADT_ENABLED)) {
		kfree(buffer.pointer);
		return -EINVAL;
	}

	physid = lapic->id;

	kfree(buffer.pointer);
	buffer.length = ACPI_ALLOCATE_BUFFER;
	buffer.pointer = NULL;

	if (!alloc_cpumask_var(&tmp_map, GFP_KERNEL))
		goto out;

	if (!alloc_cpumask_var(&new_map, GFP_KERNEL))
		goto free_tmp_map;

	cpumask_copy(tmp_map, cpu_present_mask);
	acpi_register_lapic(physid, lapic->lapic_flags & ACPI_MADT_ENABLED);

	
	cpumask_andnot(new_map, cpu_present_mask, tmp_map);
	if (cpumask_empty(new_map)) {
		printk ("Unable to map lapic to logical cpu number\n");
		retval = -EINVAL;
		goto free_new_map;
	}

	cpu = cpumask_first(new_map);

	*pcpu = cpu;
	retval = 0;

free_new_map:
	free_cpumask_var(new_map);
free_tmp_map:
	free_cpumask_var(tmp_map);
out:
	return retval;
}


int __ref acpi_map_lsapic(acpi_handle handle, int *pcpu)
{
	return _acpi_map_lsapic(handle, pcpu);
}
EXPORT_SYMBOL(acpi_map_lsapic);

int acpi_unmap_lsapic(int cpu)
{
	per_cpu(x86_cpu_to_apicid, cpu) = -1;
	set_cpu_present(cpu, false);
	num_processors--;

	return (0);
}

EXPORT_SYMBOL(acpi_unmap_lsapic);
#endif				

int acpi_register_ioapic(acpi_handle handle, u64 phys_addr, u32 gsi_base)
{
	
	return -EINVAL;
}

EXPORT_SYMBOL(acpi_register_ioapic);

int acpi_unregister_ioapic(acpi_handle handle, u32 gsi_base)
{
	
	return -EINVAL;
}

EXPORT_SYMBOL(acpi_unregister_ioapic);

static int __init acpi_parse_sbf(struct acpi_table_header *table)
{
	struct acpi_table_boot *sb;

	sb = (struct acpi_table_boot *)table;
	if (!sb) {
		printk(KERN_WARNING PREFIX "Unable to map SBF\n");
		return -ENODEV;
	}

	sbf_port = sb->cmos_index;	

	return 0;
}

#ifdef CONFIG_HPET_TIMER
#include <asm/hpet.h>

static struct __initdata resource *hpet_res;

static int __init acpi_parse_hpet(struct acpi_table_header *table)
{
	struct acpi_table_hpet *hpet_tbl;

	hpet_tbl = (struct acpi_table_hpet *)table;
	if (!hpet_tbl) {
		printk(KERN_WARNING PREFIX "Unable to map HPET\n");
		return -ENODEV;
	}

	if (hpet_tbl->address.space_id != ACPI_SPACE_MEM) {
		printk(KERN_WARNING PREFIX "HPET timers must be located in "
		       "memory.\n");
		return -1;
	}

	hpet_address = hpet_tbl->address.address;

	
	if (!hpet_address) {
		printk(KERN_WARNING PREFIX
		       "HPET id: %#x base: %#lx is invalid\n",
		       hpet_tbl->id, hpet_address);
		return 0;
	}
#ifdef CONFIG_X86_64
	
	if (hpet_address == 0xfed0000000000000UL) {
		if (!hpet_force_user) {
			printk(KERN_WARNING PREFIX "HPET id: %#x "
			       "base: 0xfed0000000000000 is bogus\n "
			       "try hpet=force on the kernel command line to "
			       "fix it up to 0xfed00000.\n", hpet_tbl->id);
			hpet_address = 0;
			return 0;
		}
		printk(KERN_WARNING PREFIX
		       "HPET id: %#x base: 0xfed0000000000000 fixed up "
		       "to 0xfed00000.\n", hpet_tbl->id);
		hpet_address >>= 32;
	}
#endif
	printk(KERN_INFO PREFIX "HPET id: %#x base: %#lx\n",
	       hpet_tbl->id, hpet_address);

	
#define HPET_RESOURCE_NAME_SIZE 9
	hpet_res = alloc_bootmem(sizeof(*hpet_res) + HPET_RESOURCE_NAME_SIZE);

	hpet_res->name = (void *)&hpet_res[1];
	hpet_res->flags = IORESOURCE_MEM;
	snprintf((char *)hpet_res->name, HPET_RESOURCE_NAME_SIZE, "HPET %u",
		 hpet_tbl->sequence);

	hpet_res->start = hpet_address;
	hpet_res->end = hpet_address + (1 * 1024) - 1;

	return 0;
}


static __init int hpet_insert_resource(void)
{
	if (!hpet_res)
		return 1;

	return insert_resource(&iomem_resource, hpet_res);
}

late_initcall(hpet_insert_resource);

#else
#define	acpi_parse_hpet	NULL
#endif

static int __init acpi_parse_fadt(struct acpi_table_header *table)
{

#ifdef CONFIG_X86_PM_TIMER
	
	if (acpi_gbl_FADT.header.revision >= FADT2_REVISION_ID) {
		
		if (acpi_gbl_FADT.xpm_timer_block.space_id !=
		    ACPI_ADR_SPACE_SYSTEM_IO)
			return 0;

		pmtmr_ioport = acpi_gbl_FADT.xpm_timer_block.address;
		
		if (!pmtmr_ioport)
			pmtmr_ioport = acpi_gbl_FADT.pm_timer_block;
	} else {
		
		pmtmr_ioport = acpi_gbl_FADT.pm_timer_block;
	}
	if (pmtmr_ioport)
		printk(KERN_INFO PREFIX "PM-Timer IO Port: %#x\n",
		       pmtmr_ioport);
#endif
	return 0;
}

#ifdef	CONFIG_X86_LOCAL_APIC


static void __init acpi_register_lapic_address(unsigned long address)
{
	mp_lapic_addr = address;

	set_fixmap_nocache(FIX_APIC_BASE, address);
	if (boot_cpu_physical_apicid == -1U) {
		boot_cpu_physical_apicid  = read_apic_id();
		apic_version[boot_cpu_physical_apicid] =
			 GET_APIC_VERSION(apic_read(APIC_LVR));
	}
}

static int __init early_acpi_parse_madt_lapic_addr_ovr(void)
{
	int count;

	if (!cpu_has_apic)
		return -ENODEV;

	

	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE,
				  acpi_parse_lapic_addr_ovr, 0);
	if (count < 0) {
		printk(KERN_ERR PREFIX
		       "Error parsing LAPIC address override entry\n");
		return count;
	}

	acpi_register_lapic_address(acpi_lapic_addr);

	return count;
}

static int __init acpi_parse_madt_lapic_entries(void)
{
	int count;
	int x2count = 0;

	if (!cpu_has_apic)
		return -ENODEV;

	

	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE,
				  acpi_parse_lapic_addr_ovr, 0);
	if (count < 0) {
		printk(KERN_ERR PREFIX
		       "Error parsing LAPIC address override entry\n");
		return count;
	}

	acpi_register_lapic_address(acpi_lapic_addr);

	count = acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_SAPIC,
				      acpi_parse_sapic, MAX_APICS);

	if (!count) {
		x2count = acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_X2APIC,
						acpi_parse_x2apic, MAX_APICS);
		count = acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_APIC,
					      acpi_parse_lapic, MAX_APICS);
	}
	if (!count && !x2count) {
		printk(KERN_ERR PREFIX "No LAPIC entries present\n");
		
		return -ENODEV;
	} else if (count < 0 || x2count < 0) {
		printk(KERN_ERR PREFIX "Error parsing LAPIC entry\n");
		
		return count;
	}

	x2count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_X2APIC_NMI,
				  acpi_parse_x2apic_nmi, 0);
	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_LOCAL_APIC_NMI, acpi_parse_lapic_nmi, 0);
	if (count < 0 || x2count < 0) {
		printk(KERN_ERR PREFIX "Error parsing LAPIC NMI entry\n");
		
		return count;
	}
	return 0;
}
#endif				

#ifdef	CONFIG_X86_IO_APIC
#define MP_ISA_BUS		0

#ifdef CONFIG_X86_ES7000
extern int es7000_plat;
#endif

int __init acpi_probe_gsi(void)
{
	int idx;
	int gsi;
	int max_gsi = 0;

	if (acpi_disabled)
		return 0;

	if (!acpi_ioapic)
		return 0;

	max_gsi = 0;
	for (idx = 0; idx < nr_ioapics; idx++) {
		gsi = mp_gsi_routing[idx].gsi_end;

		if (gsi > max_gsi)
			max_gsi = gsi;
	}

	return max_gsi + 1;
}

static void assign_to_mp_irq(struct mpc_intsrc *m,
				    struct mpc_intsrc *mp_irq)
{
	memcpy(mp_irq, m, sizeof(struct mpc_intsrc));
}

static int mp_irq_cmp(struct mpc_intsrc *mp_irq,
				struct mpc_intsrc *m)
{
	return memcmp(mp_irq, m, sizeof(struct mpc_intsrc));
}

static void save_mp_irq(struct mpc_intsrc *m)
{
	int i;

	for (i = 0; i < mp_irq_entries; i++) {
		if (!mp_irq_cmp(&mp_irqs[i], m))
			return;
	}

	assign_to_mp_irq(m, &mp_irqs[mp_irq_entries]);
	if (++mp_irq_entries == MAX_IRQ_SOURCES)
		panic("Max # of irq sources exceeded!!\n");
}

void __init mp_override_legacy_irq(u8 bus_irq, u8 polarity, u8 trigger, u32 gsi)
{
	int ioapic;
	int pin;
	struct mpc_intsrc mp_irq;

	
	ioapic = mp_find_ioapic(gsi);
	if (ioapic < 0)
		return;
	pin = mp_find_ioapic_pin(ioapic, gsi);

	
	if ((bus_irq == 0) && (trigger == 3))
		trigger = 1;

	mp_irq.type = MP_INTSRC;
	mp_irq.irqtype = mp_INT;
	mp_irq.irqflag = (trigger << 2) | polarity;
	mp_irq.srcbus = MP_ISA_BUS;
	mp_irq.srcbusirq = bus_irq;	
	mp_irq.dstapic = mp_ioapics[ioapic].apicid; 
	mp_irq.dstirq = pin;	

	save_mp_irq(&mp_irq);
}

void __init mp_config_acpi_legacy_irqs(void)
{
	int i;
	int ioapic;
	unsigned int dstapic;
	struct mpc_intsrc mp_irq;

#if defined (CONFIG_MCA) || defined (CONFIG_EISA)
	
	mp_bus_id_to_type[MP_ISA_BUS] = MP_BUS_ISA;
#endif
	set_bit(MP_ISA_BUS, mp_bus_not_pci);
	pr_debug("Bus #%d is ISA\n", MP_ISA_BUS);

#ifdef CONFIG_X86_ES7000
	
	if (es7000_plat == 1)
		return;
#endif

	
	ioapic = mp_find_ioapic(0);
	if (ioapic < 0)
		return;
	dstapic = mp_ioapics[ioapic].apicid;

	
	for (i = 0; i < 16; i++) {
		int idx;

		for (idx = 0; idx < mp_irq_entries; idx++) {
			struct mpc_intsrc *irq = mp_irqs + idx;

			
			if (irq->srcbus == MP_ISA_BUS && irq->srcbusirq == i)
				break;

			
			if (irq->dstapic == dstapic && irq->dstirq == i)
				break;
		}

		if (idx != mp_irq_entries) {
			printk(KERN_DEBUG "ACPI: IRQ%d used by override.\n", i);
			continue;	
		}

		mp_irq.type = MP_INTSRC;
		mp_irq.irqflag = 0;	
		mp_irq.srcbus = MP_ISA_BUS;
		mp_irq.dstapic = dstapic;
		mp_irq.irqtype = mp_INT;
		mp_irq.srcbusirq = i; 
		mp_irq.dstirq = i;

		save_mp_irq(&mp_irq);
	}
}

static int mp_config_acpi_gsi(struct device *dev, u32 gsi, int trigger,
			int polarity)
{
#ifdef CONFIG_X86_MPPARSE
	struct mpc_intsrc mp_irq;
	struct pci_dev *pdev;
	unsigned char number;
	unsigned int devfn;
	int ioapic;
	u8 pin;

	if (!acpi_ioapic)
		return 0;
	if (!dev)
		return 0;
	if (dev->bus != &pci_bus_type)
		return 0;

	pdev = to_pci_dev(dev);
	number = pdev->bus->number;
	devfn = pdev->devfn;
	pin = pdev->pin;
	
	mp_irq.type = MP_INTSRC;
	mp_irq.irqtype = mp_INT;
	mp_irq.irqflag = (trigger == ACPI_EDGE_SENSITIVE ? 4 : 0x0c) |
				(polarity == ACPI_ACTIVE_HIGH ? 1 : 3);
	mp_irq.srcbus = number;
	mp_irq.srcbusirq = (((devfn >> 3) & 0x1f) << 2) | ((pin - 1) & 3);
	ioapic = mp_find_ioapic(gsi);
	mp_irq.dstapic = mp_ioapics[ioapic].apicid;
	mp_irq.dstirq = mp_find_ioapic_pin(ioapic, gsi);

	save_mp_irq(&mp_irq);
#endif
	return 0;
}

int mp_register_gsi(struct device *dev, u32 gsi, int trigger, int polarity)
{
	int ioapic;
	int ioapic_pin;
	struct io_apic_irq_attr irq_attr;

	if (acpi_irq_model != ACPI_IRQ_MODEL_IOAPIC)
		return gsi;

	
	if (acpi_gbl_FADT.sci_interrupt == gsi)
		return gsi;

	ioapic = mp_find_ioapic(gsi);
	if (ioapic < 0) {
		printk(KERN_WARNING "No IOAPIC for GSI %u\n", gsi);
		return gsi;
	}

	ioapic_pin = mp_find_ioapic_pin(ioapic, gsi);

#ifdef CONFIG_X86_32
	if (ioapic_renumber_irq)
		gsi = ioapic_renumber_irq(ioapic, gsi);
#endif

	if (ioapic_pin > MP_MAX_IOAPIC_PIN) {
		printk(KERN_ERR "Invalid reference to IOAPIC pin "
		       "%d-%d\n", mp_ioapics[ioapic].apicid,
		       ioapic_pin);
		return gsi;
	}

	if (enable_update_mptable)
		mp_config_acpi_gsi(dev, gsi, trigger, polarity);

	set_io_apic_irq_attr(&irq_attr, ioapic, ioapic_pin,
			     trigger == ACPI_EDGE_SENSITIVE ? 0 : 1,
			     polarity == ACPI_ACTIVE_HIGH ? 0 : 1);
	io_apic_set_pci_routing(dev, gsi, &irq_attr);

	return gsi;
}


static int __init acpi_parse_madt_ioapic_entries(void)
{
	int count;

	
	if (acpi_disabled || acpi_noirq)
		return -ENODEV;

	if (!cpu_has_apic)
		return -ENODEV;

	
	if (skip_ioapic_setup) {
		printk(KERN_INFO PREFIX "Skipping IOAPIC probe "
		       "due to 'noapic' option.\n");
		return -ENODEV;
	}

	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_IO_APIC, acpi_parse_ioapic,
				  MAX_IO_APICS);
	if (!count) {
		printk(KERN_ERR PREFIX "No IOAPIC entries present\n");
		return -ENODEV;
	} else if (count < 0) {
		printk(KERN_ERR PREFIX "Error parsing IOAPIC entry\n");
		return count;
	}

	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_INTERRUPT_OVERRIDE, acpi_parse_int_src_ovr,
				  nr_irqs);
	if (count < 0) {
		printk(KERN_ERR PREFIX
		       "Error parsing interrupt source overrides entry\n");
		
		return count;
	}

	
	if (!acpi_sci_override_gsi)
		acpi_sci_ioapic_setup(acpi_gbl_FADT.sci_interrupt, 0, 0);

	
	mp_config_acpi_legacy_irqs();

	count =
	    acpi_table_parse_madt(ACPI_MADT_TYPE_NMI_SOURCE, acpi_parse_nmi_src,
				  nr_irqs);
	if (count < 0) {
		printk(KERN_ERR PREFIX "Error parsing NMI SRC entry\n");
		
		return count;
	}

	return 0;
}
#else
static inline int acpi_parse_madt_ioapic_entries(void)
{
	return -1;
}
#endif	

static void __init early_acpi_process_madt(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	int error;

	if (!acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt)) {

		
		error = early_acpi_parse_madt_lapic_addr_ovr();
		if (!error) {
			acpi_lapic = 1;
			smp_found_config = 1;
		}
		if (error == -EINVAL) {
			
			printk(KERN_ERR PREFIX
			       "Invalid BIOS MADT, disabling ACPI\n");
			disable_acpi();
		}
	}
#endif
}

static void __init acpi_process_madt(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	int error;

	if (!acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt)) {

		
		error = acpi_parse_madt_lapic_entries();
		if (!error) {
			acpi_lapic = 1;

#ifdef CONFIG_X86_BIGSMP
			generic_bigsmp_probe();
#endif
			
			error = acpi_parse_madt_ioapic_entries();
			if (!error) {
				acpi_irq_model = ACPI_IRQ_MODEL_IOAPIC;
				acpi_ioapic = 1;

				smp_found_config = 1;
				if (apic->setup_apic_routing)
					apic->setup_apic_routing();
			}
		}
		if (error == -EINVAL) {
			
			printk(KERN_ERR PREFIX
			       "Invalid BIOS MADT, disabling ACPI\n");
			disable_acpi();
		}
	} else {
		
		if (smp_found_config) {
			printk(KERN_WARNING PREFIX
				"No APIC-table, disabling MPS\n");
			smp_found_config = 0;
		}
	}

	
	if (acpi_lapic && acpi_ioapic)
		printk(KERN_INFO "Using ACPI (MADT) for SMP configuration "
		       "information\n");
	else if (acpi_lapic)
		printk(KERN_INFO "Using ACPI for processor (LAPIC) "
		       "configuration information\n");
#endif
	return;
}

static int __init disable_acpi_irq(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: force use of acpi=noirq\n",
		       d->ident);
		acpi_noirq_set();
	}
	return 0;
}

static int __init disable_acpi_pci(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: force use of pci=noacpi\n",
		       d->ident);
		acpi_disable_pci();
	}
	return 0;
}

static int __init dmi_disable_acpi(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: acpi off\n", d->ident);
		disable_acpi();
	} else {
		printk(KERN_NOTICE
		       "Warning: DMI blacklist says broken, but acpi forced\n");
	}
	return 0;
}


static int __init force_acpi_ht(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: force use of acpi=ht\n",
		       d->ident);
		disable_acpi();
		acpi_ht = 1;
	} else {
		printk(KERN_NOTICE
		       "Warning: acpi=force overrules DMI blacklist: acpi=ht\n");
	}
	return 0;
}


static int __init dmi_ignore_irq0_timer_override(const struct dmi_system_id *d)
{
	
	if (!acpi_skip_timer_override) {
		WARN(1, KERN_ERR "ati_ixp4x0 quirk not complete.\n");
		pr_notice("%s detected: Ignoring BIOS IRQ0 pin2 override\n",
			d->ident);
		acpi_skip_timer_override = 1;
	}
	return 0;
}


static struct dmi_system_id __initdata acpi_dmi_table[] = {
	
	{
	 .callback = dmi_disable_acpi,
	 .ident = "IBM Thinkpad",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2629H1G"),
		     },
	 },

	
	{
	 .callback = force_acpi_ht,
	 .ident = "FSC Primergy T850",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "FUJITSU SIEMENS"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "PRIMERGY T850"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "HP VISUALIZE NT Workstation",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP VISUALIZE NT Workstation"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "Compaq Workstation W8000",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Compaq"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "Workstation W8000"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "ASUS P2B-DS",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC."),
		     DMI_MATCH(DMI_BOARD_NAME, "P2B-DS"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "ASUS CUR-DLS",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC."),
		     DMI_MATCH(DMI_BOARD_NAME, "CUR-DLS"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "ABIT i440BX-W83977",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ABIT <http://www.abit.com>"),
		     DMI_MATCH(DMI_BOARD_NAME, "i440BX-W83977 (BP6)"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "IBM Bladecenter",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "IBM eServer BladeCenter HS20"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "IBM eServer xSeries 360",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "eServer xSeries 360"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "IBM eserver xSeries 330",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "eserver xSeries 330"),
		     },
	 },
	{
	 .callback = force_acpi_ht,
	 .ident = "IBM eserver xSeries 440",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "eserver xSeries 440"),
		     },
	 },

	
	{
	 .callback = disable_acpi_irq,
	 .ident = "ASUS A7V",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC"),
		     DMI_MATCH(DMI_BOARD_NAME, "<A7V>"),
		     
		     DMI_MATCH(DMI_BIOS_VERSION,
			       "ASUS A7V ACPI BIOS Revision 1007"),
		     },
	 },
	{
		
	 .callback = disable_acpi_irq,
	 .ident = "IBM Thinkpad 600 Series 2645",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2645"),
		     },
	 },
	{
	 .callback = disable_acpi_irq,
	 .ident = "IBM Thinkpad 600 Series 2646",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2646"),
		     },
	 },
	
	{			
	 .callback = disable_acpi_pci,
	 .ident = "ASUS PR-DLS",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC."),
		     DMI_MATCH(DMI_BOARD_NAME, "PR-DLS"),
		     DMI_MATCH(DMI_BIOS_VERSION,
			       "ASUS PR-DLS ACPI BIOS Revision 1010"),
		     DMI_MATCH(DMI_BIOS_DATE, "03/21/2003")
		     },
	 },
	{
	 .callback = disable_acpi_pci,
	 .ident = "Acer TravelMate 36x Laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "TravelMate 360"),
		     },
	 },
	{}
};


static struct dmi_system_id __initdata acpi_dmi_table_late[] = {
	
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP nx6115 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6115"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP NX6125 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6125"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP NX6325 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6325"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP 6715b laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq 6715b"),
		     },
	 },
	{}
};



int __init acpi_boot_table_init(void)
{
	int error;

	dmi_check_system(acpi_dmi_table);

	
	if (acpi_disabled && !acpi_ht)
		return 1;

	
	error = acpi_table_init();
	if (error) {
		disable_acpi();
		return error;
	}

	acpi_table_parse(ACPI_SIG_BOOT, acpi_parse_sbf);

	
	error = acpi_blacklisted();
	if (error) {
		if (acpi_force) {
			printk(KERN_WARNING PREFIX "acpi=force override\n");
		} else {
			printk(KERN_WARNING PREFIX "Disabling ACPI support\n");
			disable_acpi();
			return error;
		}
	}

	return 0;
}

int __init early_acpi_boot_init(void)
{
	
	if (acpi_disabled && !acpi_ht)
		return 1;

	
	early_acpi_process_madt();

	return 0;
}

int __init acpi_boot_init(void)
{
	
	dmi_check_system(acpi_dmi_table_late);

	
	if (acpi_disabled && !acpi_ht)
		return 1;

	acpi_table_parse(ACPI_SIG_BOOT, acpi_parse_sbf);

	
	acpi_table_parse(ACPI_SIG_FADT, acpi_parse_fadt);

	
	acpi_process_madt();

	acpi_table_parse(ACPI_SIG_HPET, acpi_parse_hpet);

	return 0;
}

static int __init parse_acpi(char *arg)
{
	if (!arg)
		return -EINVAL;

	
	if (strcmp(arg, "off") == 0) {
		disable_acpi();
	}
	
	else if (strcmp(arg, "force") == 0) {
		acpi_force = 1;
		acpi_ht = 1;
		acpi_disabled = 0;
	}
	
	else if (strcmp(arg, "strict") == 0) {
		acpi_strict = 1;
	}
	
	else if (strcmp(arg, "ht") == 0) {
		if (!acpi_force)
			disable_acpi();
		acpi_ht = 1;
	}
	
	else if (strcmp(arg, "rsdt") == 0) {
		acpi_rsdt_forced = 1;
	}
	
	else if (strcmp(arg, "noirq") == 0) {
		acpi_noirq_set();
	} else {
		
		return -EINVAL;
	}
	return 0;
}
early_param("acpi", parse_acpi);


static int __init parse_pci(char *arg)
{
	if (arg && strcmp(arg, "noacpi") == 0)
		acpi_disable_pci();
	return 0;
}
early_param("pci", parse_pci);

int __init acpi_mps_check(void)
{
#if defined(CONFIG_X86_LOCAL_APIC) && !defined(CONFIG_X86_MPPARSE)

	if (acpi_disabled || acpi_noirq) {
		printk(KERN_WARNING "MPS support code is not built-in.\n"
		       "Using acpi=off or acpi=noirq or pci=noacpi "
		       "may have problem\n");
		return 1;
	}
#endif
	return 0;
}

#ifdef CONFIG_X86_IO_APIC
static int __init parse_acpi_skip_timer_override(char *arg)
{
	acpi_skip_timer_override = 1;
	return 0;
}
early_param("acpi_skip_timer_override", parse_acpi_skip_timer_override);

static int __init parse_acpi_use_timer_override(char *arg)
{
	acpi_use_timer_override = 1;
	return 0;
}
early_param("acpi_use_timer_override", parse_acpi_use_timer_override);
#endif 

static int __init setup_acpi_sci(char *s)
{
	if (!s)
		return -EINVAL;
	if (!strcmp(s, "edge"))
		acpi_sci_flags =  ACPI_MADT_TRIGGER_EDGE |
			(acpi_sci_flags & ~ACPI_MADT_TRIGGER_MASK);
	else if (!strcmp(s, "level"))
		acpi_sci_flags = ACPI_MADT_TRIGGER_LEVEL |
			(acpi_sci_flags & ~ACPI_MADT_TRIGGER_MASK);
	else if (!strcmp(s, "high"))
		acpi_sci_flags = ACPI_MADT_POLARITY_ACTIVE_HIGH |
			(acpi_sci_flags & ~ACPI_MADT_POLARITY_MASK);
	else if (!strcmp(s, "low"))
		acpi_sci_flags = ACPI_MADT_POLARITY_ACTIVE_LOW |
			(acpi_sci_flags & ~ACPI_MADT_POLARITY_MASK);
	else
		return -EINVAL;
	return 0;
}
early_param("acpi_sci", setup_acpi_sci);

int __acpi_acquire_global_lock(unsigned int *lock)
{
	unsigned int old, new, val;
	do {
		old = *lock;
		new = (((old & ~0x3) + 2) + ((old >> 1) & 0x1));
		val = cmpxchg(lock, old, new);
	} while (unlikely (val != old));
	return (new < 3) ? -1 : 0;
}

int __acpi_release_global_lock(unsigned int *lock)
{
	unsigned int old, new, val;
	do {
		old = *lock;
		new = old & ~0x3;
		val = cmpxchg(lock, old, new);
	} while (unlikely (val != old));
	return old & 0x1;
}
