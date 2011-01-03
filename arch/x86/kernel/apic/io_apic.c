

#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/mc146818rtc.h>
#include <linux/compiler.h>
#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/sysdev.h>
#include <linux/msi.h>
#include <linux/htirq.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>	
#ifdef CONFIG_ACPI
#include <acpi/acpi_bus.h>
#endif
#include <linux/bootmem.h>
#include <linux/dmar.h>
#include <linux/hpet.h>

#include <asm/idle.h>
#include <asm/io.h>
#include <asm/smp.h>
#include <asm/cpu.h>
#include <asm/desc.h>
#include <asm/proto.h>
#include <asm/acpi.h>
#include <asm/dma.h>
#include <asm/timer.h>
#include <asm/i8259.h>
#include <asm/nmi.h>
#include <asm/msidef.h>
#include <asm/hypertransport.h>
#include <asm/setup.h>
#include <asm/irq_remapping.h>
#include <asm/hpet.h>
#include <asm/hw_irq.h>
#include <asm/uv/uv_hub.h>
#include <asm/uv/uv_irq.h>

#include <asm/apic.h>

#define __apicdebuginit(type) static type __init
#define for_each_irq_pin(entry, head) \
	for (entry = head; entry; entry = entry->next)


int sis_apic_bug = -1;

static DEFINE_SPINLOCK(ioapic_lock);
static DEFINE_SPINLOCK(vector_lock);


int nr_ioapic_registers[MAX_IO_APICS];


struct mpc_ioapic mp_ioapics[MAX_IO_APICS];
int nr_ioapics;


struct mp_ioapic_gsi  mp_gsi_routing[MAX_IO_APICS];


struct mpc_intsrc mp_irqs[MAX_IRQ_SOURCES];


int mp_irq_entries;


static int nr_legacy_irqs __read_mostly = NR_IRQS_LEGACY;

static int nr_irqs_gsi = NR_IRQS_LEGACY;

#if defined (CONFIG_MCA) || defined (CONFIG_EISA)
int mp_bus_id_to_type[MAX_MP_BUSSES];
#endif

DECLARE_BITMAP(mp_bus_not_pci, MAX_MP_BUSSES);

int skip_ioapic_setup;

void arch_disable_smp_support(void)
{
#ifdef CONFIG_PCI
	noioapicquirk = 1;
	noioapicreroute = -1;
#endif
	skip_ioapic_setup = 1;
}

static int __init parse_noapic(char *str)
{
	
	arch_disable_smp_support();
	return 0;
}
early_param("noapic", parse_noapic);

struct irq_pin_list {
	int apic, pin;
	struct irq_pin_list *next;
};

static struct irq_pin_list *get_one_free_irq_2_pin(int node)
{
	struct irq_pin_list *pin;

	pin = kzalloc_node(sizeof(*pin), GFP_ATOMIC, node);

	return pin;
}


struct irq_cfg {
	struct irq_pin_list *irq_2_pin;
	cpumask_var_t domain;
	cpumask_var_t old_domain;
	unsigned move_cleanup_count;
	u8 vector;
	u8 move_in_progress : 1;
};


#ifdef CONFIG_SPARSE_IRQ
static struct irq_cfg irq_cfgx[] = {
#else
static struct irq_cfg irq_cfgx[NR_IRQS] = {
#endif
	[0]  = { .vector = IRQ0_VECTOR,  },
	[1]  = { .vector = IRQ1_VECTOR,  },
	[2]  = { .vector = IRQ2_VECTOR,  },
	[3]  = { .vector = IRQ3_VECTOR,  },
	[4]  = { .vector = IRQ4_VECTOR,  },
	[5]  = { .vector = IRQ5_VECTOR,  },
	[6]  = { .vector = IRQ6_VECTOR,  },
	[7]  = { .vector = IRQ7_VECTOR,  },
	[8]  = { .vector = IRQ8_VECTOR,  },
	[9]  = { .vector = IRQ9_VECTOR,  },
	[10] = { .vector = IRQ10_VECTOR, },
	[11] = { .vector = IRQ11_VECTOR, },
	[12] = { .vector = IRQ12_VECTOR, },
	[13] = { .vector = IRQ13_VECTOR, },
	[14] = { .vector = IRQ14_VECTOR, },
	[15] = { .vector = IRQ15_VECTOR, },
};

void __init io_apic_disable_legacy(void)
{
	nr_legacy_irqs = 0;
	nr_irqs_gsi = 0;
}

int __init arch_early_irq_init(void)
{
	struct irq_cfg *cfg;
	struct irq_desc *desc;
	int count;
	int node;
	int i;

	cfg = irq_cfgx;
	count = ARRAY_SIZE(irq_cfgx);
	node= cpu_to_node(boot_cpu_id);

	for (i = 0; i < count; i++) {
		desc = irq_to_desc(i);
		desc->chip_data = &cfg[i];
		zalloc_cpumask_var_node(&cfg[i].domain, GFP_NOWAIT, node);
		zalloc_cpumask_var_node(&cfg[i].old_domain, GFP_NOWAIT, node);
		if (i < nr_legacy_irqs)
			cpumask_setall(cfg[i].domain);
	}

	return 0;
}

#ifdef CONFIG_SPARSE_IRQ
static struct irq_cfg *irq_cfg(unsigned int irq)
{
	struct irq_cfg *cfg = NULL;
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	if (desc)
		cfg = desc->chip_data;

	return cfg;
}

static struct irq_cfg *get_one_free_irq_cfg(int node)
{
	struct irq_cfg *cfg;

	cfg = kzalloc_node(sizeof(*cfg), GFP_ATOMIC, node);
	if (cfg) {
		if (!zalloc_cpumask_var_node(&cfg->domain, GFP_ATOMIC, node)) {
			kfree(cfg);
			cfg = NULL;
		} else if (!zalloc_cpumask_var_node(&cfg->old_domain,
							  GFP_ATOMIC, node)) {
			free_cpumask_var(cfg->domain);
			kfree(cfg);
			cfg = NULL;
		}
	}

	return cfg;
}

int arch_init_chip_data(struct irq_desc *desc, int node)
{
	struct irq_cfg *cfg;

	cfg = desc->chip_data;
	if (!cfg) {
		desc->chip_data = get_one_free_irq_cfg(node);
		if (!desc->chip_data) {
			printk(KERN_ERR "can not alloc irq_cfg\n");
			BUG_ON(1);
		}
	}

	return 0;
}


static void
init_copy_irq_2_pin(struct irq_cfg *old_cfg, struct irq_cfg *cfg, int node)
{
	struct irq_pin_list *old_entry, *head, *tail, *entry;

	cfg->irq_2_pin = NULL;
	old_entry = old_cfg->irq_2_pin;
	if (!old_entry)
		return;

	entry = get_one_free_irq_2_pin(node);
	if (!entry)
		return;

	entry->apic	= old_entry->apic;
	entry->pin	= old_entry->pin;
	head		= entry;
	tail		= entry;
	old_entry	= old_entry->next;
	while (old_entry) {
		entry = get_one_free_irq_2_pin(node);
		if (!entry) {
			entry = head;
			while (entry) {
				head = entry->next;
				kfree(entry);
				entry = head;
			}
			
			return;
		}
		entry->apic	= old_entry->apic;
		entry->pin	= old_entry->pin;
		tail->next	= entry;
		tail		= entry;
		old_entry	= old_entry->next;
	}

	tail->next = NULL;
	cfg->irq_2_pin = head;
}

static void free_irq_2_pin(struct irq_cfg *old_cfg, struct irq_cfg *cfg)
{
	struct irq_pin_list *entry, *next;

	if (old_cfg->irq_2_pin == cfg->irq_2_pin)
		return;

	entry = old_cfg->irq_2_pin;

	while (entry) {
		next = entry->next;
		kfree(entry);
		entry = next;
	}
	old_cfg->irq_2_pin = NULL;
}

void arch_init_copy_chip_data(struct irq_desc *old_desc,
				 struct irq_desc *desc, int node)
{
	struct irq_cfg *cfg;
	struct irq_cfg *old_cfg;

	cfg = get_one_free_irq_cfg(node);

	if (!cfg)
		return;

	desc->chip_data = cfg;

	old_cfg = old_desc->chip_data;

	memcpy(cfg, old_cfg, sizeof(struct irq_cfg));

	init_copy_irq_2_pin(old_cfg, cfg, node);
}

static void free_irq_cfg(struct irq_cfg *old_cfg)
{
	kfree(old_cfg);
}

void arch_free_chip_data(struct irq_desc *old_desc, struct irq_desc *desc)
{
	struct irq_cfg *old_cfg, *cfg;

	old_cfg = old_desc->chip_data;
	cfg = desc->chip_data;

	if (old_cfg == cfg)
		return;

	if (old_cfg) {
		free_irq_2_pin(old_cfg, cfg);
		free_irq_cfg(old_cfg);
		old_desc->chip_data = NULL;
	}
}


#else
static struct irq_cfg *irq_cfg(unsigned int irq)
{
	return irq < nr_irqs ? irq_cfgx + irq : NULL;
}

#endif

struct io_apic {
	unsigned int index;
	unsigned int unused[3];
	unsigned int data;
	unsigned int unused2[11];
	unsigned int eoi;
};

static __attribute_const__ struct io_apic __iomem *io_apic_base(int idx)
{
	return (void __iomem *) __fix_to_virt(FIX_IO_APIC_BASE_0 + idx)
		+ (mp_ioapics[idx].apicaddr & ~PAGE_MASK);
}

static inline void io_apic_eoi(unsigned int apic, unsigned int vector)
{
	struct io_apic __iomem *io_apic = io_apic_base(apic);
	writel(vector, &io_apic->eoi);
}

static inline unsigned int io_apic_read(unsigned int apic, unsigned int reg)
{
	struct io_apic __iomem *io_apic = io_apic_base(apic);
	writel(reg, &io_apic->index);
	return readl(&io_apic->data);
}

static inline void io_apic_write(unsigned int apic, unsigned int reg, unsigned int value)
{
	struct io_apic __iomem *io_apic = io_apic_base(apic);
	writel(reg, &io_apic->index);
	writel(value, &io_apic->data);
}


static inline void io_apic_modify(unsigned int apic, unsigned int reg, unsigned int value)
{
	struct io_apic __iomem *io_apic = io_apic_base(apic);

	if (sis_apic_bug)
		writel(reg, &io_apic->index);
	writel(value, &io_apic->data);
}

static bool io_apic_level_ack_pending(struct irq_cfg *cfg)
{
	struct irq_pin_list *entry;
	unsigned long flags;

	spin_lock_irqsave(&ioapic_lock, flags);
	for_each_irq_pin(entry, cfg->irq_2_pin) {
		unsigned int reg;
		int pin;

		pin = entry->pin;
		reg = io_apic_read(entry->apic, 0x10 + pin*2);
		
		if (reg & IO_APIC_REDIR_REMOTE_IRR) {
			spin_unlock_irqrestore(&ioapic_lock, flags);
			return true;
		}
	}
	spin_unlock_irqrestore(&ioapic_lock, flags);

	return false;
}

union entry_union {
	struct { u32 w1, w2; };
	struct IO_APIC_route_entry entry;
};

static struct IO_APIC_route_entry ioapic_read_entry(int apic, int pin)
{
	union entry_union eu;
	unsigned long flags;
	spin_lock_irqsave(&ioapic_lock, flags);
	eu.w1 = io_apic_read(apic, 0x10 + 2 * pin);
	eu.w2 = io_apic_read(apic, 0x11 + 2 * pin);
	spin_unlock_irqrestore(&ioapic_lock, flags);
	return eu.entry;
}


static void
__ioapic_write_entry(int apic, int pin, struct IO_APIC_route_entry e)
{
	union entry_union eu = {{0, 0}};

	eu.entry = e;
	io_apic_write(apic, 0x11 + 2*pin, eu.w2);
	io_apic_write(apic, 0x10 + 2*pin, eu.w1);
}

void ioapic_write_entry(int apic, int pin, struct IO_APIC_route_entry e)
{
	unsigned long flags;
	spin_lock_irqsave(&ioapic_lock, flags);
	__ioapic_write_entry(apic, pin, e);
	spin_unlock_irqrestore(&ioapic_lock, flags);
}


static void ioapic_mask_entry(int apic, int pin)
{
	unsigned long flags;
	union entry_union eu = { .entry.mask = 1 };

	spin_lock_irqsave(&ioapic_lock, flags);
	io_apic_write(apic, 0x10 + 2*pin, eu.w1);
	io_apic_write(apic, 0x11 + 2*pin, eu.w2);
	spin_unlock_irqrestore(&ioapic_lock, flags);
}


static int
add_pin_to_irq_node_nopanic(struct irq_cfg *cfg, int node, int apic, int pin)
{
	struct irq_pin_list **last, *entry;

	
	last = &cfg->irq_2_pin;
	for_each_irq_pin(entry, cfg->irq_2_pin) {
		if (entry->apic == apic && entry->pin == pin)
			return 0;
		last = &entry->next;
	}

	entry = get_one_free_irq_2_pin(node);
	if (!entry) {
		printk(KERN_ERR "can not alloc irq_pin_list (%d,%d,%d)\n",
				node, apic, pin);
		return -ENOMEM;
	}
	entry->apic = apic;
	entry->pin = pin;

	*last = entry;
	return 0;
}

static void add_pin_to_irq_node(struct irq_cfg *cfg, int node, int apic, int pin)
{
	if (add_pin_to_irq_node_nopanic(cfg, node, apic, pin))
		panic("IO-APIC: failed to add irq-pin. Can not proceed\n");
}


static void __init replace_pin_at_irq_node(struct irq_cfg *cfg, int node,
					   int oldapic, int oldpin,
					   int newapic, int newpin)
{
	struct irq_pin_list *entry;

	for_each_irq_pin(entry, cfg->irq_2_pin) {
		if (entry->apic == oldapic && entry->pin == oldpin) {
			entry->apic = newapic;
			entry->pin = newpin;
			
			return;
		}
	}

	
	add_pin_to_irq_node(cfg, node, newapic, newpin);
}

static void io_apic_modify_irq(struct irq_cfg *cfg,
			       int mask_and, int mask_or,
			       void (*final)(struct irq_pin_list *entry))
{
	int pin;
	struct irq_pin_list *entry;

	for_each_irq_pin(entry, cfg->irq_2_pin) {
		unsigned int reg;
		pin = entry->pin;
		reg = io_apic_read(entry->apic, 0x10 + pin * 2);
		reg &= mask_and;
		reg |= mask_or;
		io_apic_modify(entry->apic, 0x10 + pin * 2, reg);
		if (final)
			final(entry);
	}
}

static void __unmask_IO_APIC_irq(struct irq_cfg *cfg)
{
	io_apic_modify_irq(cfg, ~IO_APIC_REDIR_MASKED, 0, NULL);
}

static void io_apic_sync(struct irq_pin_list *entry)
{
	
	struct io_apic __iomem *io_apic;
	io_apic = io_apic_base(entry->apic);
	readl(&io_apic->data);
}

static void __mask_IO_APIC_irq(struct irq_cfg *cfg)
{
	io_apic_modify_irq(cfg, ~0, IO_APIC_REDIR_MASKED, &io_apic_sync);
}

static void __mask_and_edge_IO_APIC_irq(struct irq_cfg *cfg)
{
	io_apic_modify_irq(cfg, ~IO_APIC_REDIR_LEVEL_TRIGGER,
			IO_APIC_REDIR_MASKED, NULL);
}

static void __unmask_and_level_IO_APIC_irq(struct irq_cfg *cfg)
{
	io_apic_modify_irq(cfg, ~IO_APIC_REDIR_MASKED,
			IO_APIC_REDIR_LEVEL_TRIGGER, NULL);
}

static void mask_IO_APIC_irq_desc(struct irq_desc *desc)
{
	struct irq_cfg *cfg = desc->chip_data;
	unsigned long flags;

	BUG_ON(!cfg);

	spin_lock_irqsave(&ioapic_lock, flags);
	__mask_IO_APIC_irq(cfg);
	spin_unlock_irqrestore(&ioapic_lock, flags);
}

static void unmask_IO_APIC_irq_desc(struct irq_desc *desc)
{
	struct irq_cfg *cfg = desc->chip_data;
	unsigned long flags;

	spin_lock_irqsave(&ioapic_lock, flags);
	__unmask_IO_APIC_irq(cfg);
	spin_unlock_irqrestore(&ioapic_lock, flags);
}

static void mask_IO_APIC_irq(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	mask_IO_APIC_irq_desc(desc);
}
static void unmask_IO_APIC_irq(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	unmask_IO_APIC_irq_desc(desc);
}

static void clear_IO_APIC_pin(unsigned int apic, unsigned int pin)
{
	struct IO_APIC_route_entry entry;

	
	entry = ioapic_read_entry(apic, pin);
	if (entry.delivery_mode == dest_SMI)
		return;
	
	ioapic_mask_entry(apic, pin);
}

static void clear_IO_APIC (void)
{
	int apic, pin;

	for (apic = 0; apic < nr_ioapics; apic++)
		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++)
			clear_IO_APIC_pin(apic, pin);
}

#ifdef CONFIG_X86_32


#define MAX_PIRQS 8
static int pirq_entries[MAX_PIRQS] = {
	[0 ... MAX_PIRQS - 1] = -1
};

static int __init ioapic_pirq_setup(char *str)
{
	int i, max;
	int ints[MAX_PIRQS+1];

	get_options(str, ARRAY_SIZE(ints), ints);

	apic_printk(APIC_VERBOSE, KERN_INFO
			"PIRQ redirection, working around broken MP-BIOS.\n");
	max = MAX_PIRQS;
	if (ints[0] < MAX_PIRQS)
		max = ints[0];

	for (i = 0; i < max; i++) {
		apic_printk(APIC_VERBOSE, KERN_DEBUG
				"... PIRQ%d -> IRQ %d\n", i, ints[i+1]);
		
		pirq_entries[MAX_PIRQS-i-1] = ints[i+1];
	}
	return 1;
}

__setup("pirq=", ioapic_pirq_setup);
#endif 

struct IO_APIC_route_entry **alloc_ioapic_entries(void)
{
	int apic;
	struct IO_APIC_route_entry **ioapic_entries;

	ioapic_entries = kzalloc(sizeof(*ioapic_entries) * nr_ioapics,
				GFP_ATOMIC);
	if (!ioapic_entries)
		return 0;

	for (apic = 0; apic < nr_ioapics; apic++) {
		ioapic_entries[apic] =
			kzalloc(sizeof(struct IO_APIC_route_entry) *
				nr_ioapic_registers[apic], GFP_ATOMIC);
		if (!ioapic_entries[apic])
			goto nomem;
	}

	return ioapic_entries;

nomem:
	while (--apic >= 0)
		kfree(ioapic_entries[apic]);
	kfree(ioapic_entries);

	return 0;
}


int save_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries)
{
	int apic, pin;

	if (!ioapic_entries)
		return -ENOMEM;

	for (apic = 0; apic < nr_ioapics; apic++) {
		if (!ioapic_entries[apic])
			return -ENOMEM;

		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++)
			ioapic_entries[apic][pin] =
				ioapic_read_entry(apic, pin);
	}

	return 0;
}


void mask_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries)
{
	int apic, pin;

	if (!ioapic_entries)
		return;

	for (apic = 0; apic < nr_ioapics; apic++) {
		if (!ioapic_entries[apic])
			break;

		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++) {
			struct IO_APIC_route_entry entry;

			entry = ioapic_entries[apic][pin];
			if (!entry.mask) {
				entry.mask = 1;
				ioapic_write_entry(apic, pin, entry);
			}
		}
	}
}


int restore_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries)
{
	int apic, pin;

	if (!ioapic_entries)
		return -ENOMEM;

	for (apic = 0; apic < nr_ioapics; apic++) {
		if (!ioapic_entries[apic])
			return -ENOMEM;

		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++)
			ioapic_write_entry(apic, pin,
					ioapic_entries[apic][pin]);
	}
	return 0;
}

void free_ioapic_entries(struct IO_APIC_route_entry **ioapic_entries)
{
	int apic;

	for (apic = 0; apic < nr_ioapics; apic++)
		kfree(ioapic_entries[apic]);

	kfree(ioapic_entries);
}


static int find_irq_entry(int apic, int pin, int type)
{
	int i;

	for (i = 0; i < mp_irq_entries; i++)
		if (mp_irqs[i].irqtype == type &&
		    (mp_irqs[i].dstapic == mp_ioapics[apic].apicid ||
		     mp_irqs[i].dstapic == MP_APIC_ALL) &&
		    mp_irqs[i].dstirq == pin)
			return i;

	return -1;
}


static int __init find_isa_irq_pin(int irq, int type)
{
	int i;

	for (i = 0; i < mp_irq_entries; i++) {
		int lbus = mp_irqs[i].srcbus;

		if (test_bit(lbus, mp_bus_not_pci) &&
		    (mp_irqs[i].irqtype == type) &&
		    (mp_irqs[i].srcbusirq == irq))

			return mp_irqs[i].dstirq;
	}
	return -1;
}

static int __init find_isa_irq_apic(int irq, int type)
{
	int i;

	for (i = 0; i < mp_irq_entries; i++) {
		int lbus = mp_irqs[i].srcbus;

		if (test_bit(lbus, mp_bus_not_pci) &&
		    (mp_irqs[i].irqtype == type) &&
		    (mp_irqs[i].srcbusirq == irq))
			break;
	}
	if (i < mp_irq_entries) {
		int apic;
		for(apic = 0; apic < nr_ioapics; apic++) {
			if (mp_ioapics[apic].apicid == mp_irqs[i].dstapic)
				return apic;
		}
	}

	return -1;
}

#if defined(CONFIG_EISA) || defined(CONFIG_MCA)

static int EISA_ELCR(unsigned int irq)
{
	if (irq < nr_legacy_irqs) {
		unsigned int port = 0x4d0 + (irq >> 3);
		return (inb(port) >> (irq & 7)) & 1;
	}
	apic_printk(APIC_VERBOSE, KERN_INFO
			"Broken MPtable reports ISA irq %d\n", irq);
	return 0;
}

#endif



#define default_ISA_trigger(idx)	(0)
#define default_ISA_polarity(idx)	(0)



#define default_EISA_trigger(idx)	(EISA_ELCR(mp_irqs[idx].srcbusirq))
#define default_EISA_polarity(idx)	default_ISA_polarity(idx)



#define default_PCI_trigger(idx)	(1)
#define default_PCI_polarity(idx)	(1)



#define default_MCA_trigger(idx)	(1)
#define default_MCA_polarity(idx)	default_ISA_polarity(idx)

static int MPBIOS_polarity(int idx)
{
	int bus = mp_irqs[idx].srcbus;
	int polarity;

	
	switch (mp_irqs[idx].irqflag & 3)
	{
		case 0: 
			if (test_bit(bus, mp_bus_not_pci))
				polarity = default_ISA_polarity(idx);
			else
				polarity = default_PCI_polarity(idx);
			break;
		case 1: 
		{
			polarity = 0;
			break;
		}
		case 2: 
		{
			printk(KERN_WARNING "broken BIOS!!\n");
			polarity = 1;
			break;
		}
		case 3: 
		{
			polarity = 1;
			break;
		}
		default: 
		{
			printk(KERN_WARNING "broken BIOS!!\n");
			polarity = 1;
			break;
		}
	}
	return polarity;
}

static int MPBIOS_trigger(int idx)
{
	int bus = mp_irqs[idx].srcbus;
	int trigger;

	
	switch ((mp_irqs[idx].irqflag>>2) & 3)
	{
		case 0: 
			if (test_bit(bus, mp_bus_not_pci))
				trigger = default_ISA_trigger(idx);
			else
				trigger = default_PCI_trigger(idx);
#if defined(CONFIG_EISA) || defined(CONFIG_MCA)
			switch (mp_bus_id_to_type[bus]) {
				case MP_BUS_ISA: 
				{
					
					break;
				}
				case MP_BUS_EISA: 
				{
					trigger = default_EISA_trigger(idx);
					break;
				}
				case MP_BUS_PCI: 
				{
					
					break;
				}
				case MP_BUS_MCA: 
				{
					trigger = default_MCA_trigger(idx);
					break;
				}
				default:
				{
					printk(KERN_WARNING "broken BIOS!!\n");
					trigger = 1;
					break;
				}
			}
#endif
			break;
		case 1: 
		{
			trigger = 0;
			break;
		}
		case 2: 
		{
			printk(KERN_WARNING "broken BIOS!!\n");
			trigger = 1;
			break;
		}
		case 3: 
		{
			trigger = 1;
			break;
		}
		default: 
		{
			printk(KERN_WARNING "broken BIOS!!\n");
			trigger = 0;
			break;
		}
	}
	return trigger;
}

static inline int irq_polarity(int idx)
{
	return MPBIOS_polarity(idx);
}

static inline int irq_trigger(int idx)
{
	return MPBIOS_trigger(idx);
}

int (*ioapic_renumber_irq)(int ioapic, int irq);
static int pin_2_irq(int idx, int apic, int pin)
{
	int irq, i;
	int bus = mp_irqs[idx].srcbus;

	
	if (mp_irqs[idx].dstirq != pin)
		printk(KERN_ERR "broken BIOS or MPTABLE parser, ayiee!!\n");

	if (test_bit(bus, mp_bus_not_pci)) {
		irq = mp_irqs[idx].srcbusirq;
	} else {
		
		i = irq = 0;
		while (i < apic)
			irq += nr_ioapic_registers[i++];
		irq += pin;
		
		if (ioapic_renumber_irq)
			irq = ioapic_renumber_irq(apic, irq);
	}

#ifdef CONFIG_X86_32
	
	if ((pin >= 16) && (pin <= 23)) {
		if (pirq_entries[pin-16] != -1) {
			if (!pirq_entries[pin-16]) {
				apic_printk(APIC_VERBOSE, KERN_DEBUG
						"disabling PIRQ%d\n", pin-16);
			} else {
				irq = pirq_entries[pin-16];
				apic_printk(APIC_VERBOSE, KERN_DEBUG
						"using PIRQ%d -> IRQ %d\n",
						pin-16, irq);
			}
		}
	}
#endif

	return irq;
}


int IO_APIC_get_PCI_irq_vector(int bus, int slot, int pin,
				struct io_apic_irq_attr *irq_attr)
{
	int apic, i, best_guess = -1;

	apic_printk(APIC_DEBUG,
		    "querying PCI -> IRQ mapping bus:%d, slot:%d, pin:%d.\n",
		    bus, slot, pin);
	if (test_bit(bus, mp_bus_not_pci)) {
		apic_printk(APIC_VERBOSE,
			    "PCI BIOS passed nonexistent PCI bus %d!\n", bus);
		return -1;
	}
	for (i = 0; i < mp_irq_entries; i++) {
		int lbus = mp_irqs[i].srcbus;

		for (apic = 0; apic < nr_ioapics; apic++)
			if (mp_ioapics[apic].apicid == mp_irqs[i].dstapic ||
			    mp_irqs[i].dstapic == MP_APIC_ALL)
				break;

		if (!test_bit(lbus, mp_bus_not_pci) &&
		    !mp_irqs[i].irqtype &&
		    (bus == lbus) &&
		    (slot == ((mp_irqs[i].srcbusirq >> 2) & 0x1f))) {
			int irq = pin_2_irq(i, apic, mp_irqs[i].dstirq);

			if (!(apic || IO_APIC_IRQ(irq)))
				continue;

			if (pin == (mp_irqs[i].srcbusirq & 3)) {
				set_io_apic_irq_attr(irq_attr, apic,
						     mp_irqs[i].dstirq,
						     irq_trigger(i),
						     irq_polarity(i));
				return irq;
			}
			
			if (best_guess < 0) {
				set_io_apic_irq_attr(irq_attr, apic,
						     mp_irqs[i].dstirq,
						     irq_trigger(i),
						     irq_polarity(i));
				best_guess = irq;
			}
		}
	}
	return best_guess;
}
EXPORT_SYMBOL(IO_APIC_get_PCI_irq_vector);

void lock_vector_lock(void)
{
	
	spin_lock(&vector_lock);
}

void unlock_vector_lock(void)
{
	spin_unlock(&vector_lock);
}

static int
__assign_irq_vector(int irq, struct irq_cfg *cfg, const struct cpumask *mask)
{
	
	static int current_vector = FIRST_DEVICE_VECTOR, current_offset = 0;
	unsigned int old_vector;
	int cpu, err;
	cpumask_var_t tmp_mask;

	if ((cfg->move_in_progress) || cfg->move_cleanup_count)
		return -EBUSY;

	if (!alloc_cpumask_var(&tmp_mask, GFP_ATOMIC))
		return -ENOMEM;

	old_vector = cfg->vector;
	if (old_vector) {
		cpumask_and(tmp_mask, mask, cpu_online_mask);
		cpumask_and(tmp_mask, cfg->domain, tmp_mask);
		if (!cpumask_empty(tmp_mask)) {
			free_cpumask_var(tmp_mask);
			return 0;
		}
	}

	
	err = -ENOSPC;
	for_each_cpu_and(cpu, mask, cpu_online_mask) {
		int new_cpu;
		int vector, offset;

		apic->vector_allocation_domain(cpu, tmp_mask);

		vector = current_vector;
		offset = current_offset;
next:
		vector += 8;
		if (vector >= first_system_vector) {
			
			offset = (offset + 1) % 8;
			vector = FIRST_DEVICE_VECTOR + offset;
		}
		if (unlikely(current_vector == vector))
			continue;

		if (test_bit(vector, used_vectors))
			goto next;

		for_each_cpu_and(new_cpu, tmp_mask, cpu_online_mask)
			if (per_cpu(vector_irq, new_cpu)[vector] != -1)
				goto next;
		
		current_vector = vector;
		current_offset = offset;
		if (old_vector) {
			cfg->move_in_progress = 1;
			cpumask_copy(cfg->old_domain, cfg->domain);
		}
		for_each_cpu_and(new_cpu, tmp_mask, cpu_online_mask)
			per_cpu(vector_irq, new_cpu)[vector] = irq;
		cfg->vector = vector;
		cpumask_copy(cfg->domain, tmp_mask);
		err = 0;
		break;
	}
	free_cpumask_var(tmp_mask);
	return err;
}

static int
assign_irq_vector(int irq, struct irq_cfg *cfg, const struct cpumask *mask)
{
	int err;
	unsigned long flags;

	spin_lock_irqsave(&vector_lock, flags);
	err = __assign_irq_vector(irq, cfg, mask);
	spin_unlock_irqrestore(&vector_lock, flags);
	return err;
}

static void __clear_irq_vector(int irq, struct irq_cfg *cfg)
{
	int cpu, vector;

	BUG_ON(!cfg->vector);

	vector = cfg->vector;
	for_each_cpu_and(cpu, cfg->domain, cpu_online_mask)
		per_cpu(vector_irq, cpu)[vector] = -1;

	cfg->vector = 0;
	cpumask_clear(cfg->domain);

	if (likely(!cfg->move_in_progress))
		return;
	for_each_cpu_and(cpu, cfg->old_domain, cpu_online_mask) {
		for (vector = FIRST_EXTERNAL_VECTOR; vector < NR_VECTORS;
								vector++) {
			if (per_cpu(vector_irq, cpu)[vector] != irq)
				continue;
			per_cpu(vector_irq, cpu)[vector] = -1;
			break;
		}
	}
	cfg->move_in_progress = 0;
}

void __setup_vector_irq(int cpu)
{
	
	
	int irq, vector;
	struct irq_cfg *cfg;
	struct irq_desc *desc;

	
	for_each_irq_desc(irq, desc) {
		cfg = desc->chip_data;
		if (!cpumask_test_cpu(cpu, cfg->domain))
			continue;
		vector = cfg->vector;
		per_cpu(vector_irq, cpu)[vector] = irq;
	}
	
	for (vector = 0; vector < NR_VECTORS; ++vector) {
		irq = per_cpu(vector_irq, cpu)[vector];
		if (irq < 0)
			continue;

		cfg = irq_cfg(irq);
		if (!cpumask_test_cpu(cpu, cfg->domain))
			per_cpu(vector_irq, cpu)[vector] = -1;
	}
}

static struct irq_chip ioapic_chip;
static struct irq_chip ir_ioapic_chip;

#define IOAPIC_AUTO     -1
#define IOAPIC_EDGE     0
#define IOAPIC_LEVEL    1

#ifdef CONFIG_X86_32
static inline int IO_APIC_irq_trigger(int irq)
{
	int apic, idx, pin;

	for (apic = 0; apic < nr_ioapics; apic++) {
		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++) {
			idx = find_irq_entry(apic, pin, mp_INT);
			if ((idx != -1) && (irq == pin_2_irq(idx, apic, pin)))
				return irq_trigger(idx);
		}
	}
	
	return 0;
}
#else
static inline int IO_APIC_irq_trigger(int irq)
{
	return 1;
}
#endif

static void ioapic_register_intr(int irq, struct irq_desc *desc, unsigned long trigger)
{

	if ((trigger == IOAPIC_AUTO && IO_APIC_irq_trigger(irq)) ||
	    trigger == IOAPIC_LEVEL)
		desc->status |= IRQ_LEVEL;
	else
		desc->status &= ~IRQ_LEVEL;

	if (irq_remapped(irq)) {
		desc->status |= IRQ_MOVE_PCNTXT;
		if (trigger)
			set_irq_chip_and_handler_name(irq, &ir_ioapic_chip,
						      handle_fasteoi_irq,
						     "fasteoi");
		else
			set_irq_chip_and_handler_name(irq, &ir_ioapic_chip,
						      handle_edge_irq, "edge");
		return;
	}

	if ((trigger == IOAPIC_AUTO && IO_APIC_irq_trigger(irq)) ||
	    trigger == IOAPIC_LEVEL)
		set_irq_chip_and_handler_name(irq, &ioapic_chip,
					      handle_fasteoi_irq,
					      "fasteoi");
	else
		set_irq_chip_and_handler_name(irq, &ioapic_chip,
					      handle_edge_irq, "edge");
}

int setup_ioapic_entry(int apic_id, int irq,
		       struct IO_APIC_route_entry *entry,
		       unsigned int destination, int trigger,
		       int polarity, int vector, int pin)
{
	
	memset(entry,0,sizeof(*entry));

	if (intr_remapping_enabled) {
		struct intel_iommu *iommu = map_ioapic_to_ir(apic_id);
		struct irte irte;
		struct IR_IO_APIC_route_entry *ir_entry =
			(struct IR_IO_APIC_route_entry *) entry;
		int index;

		if (!iommu)
			panic("No mapping iommu for ioapic %d\n", apic_id);

		index = alloc_irte(iommu, irq, 1);
		if (index < 0)
			panic("Failed to allocate IRTE for ioapic %d\n", apic_id);

		memset(&irte, 0, sizeof(irte));

		irte.present = 1;
		irte.dst_mode = apic->irq_dest_mode;
		
		irte.trigger_mode = 0;
		irte.dlvry_mode = apic->irq_delivery_mode;
		irte.vector = vector;
		irte.dest_id = IRTE_DEST(destination);

		
		set_ioapic_sid(&irte, apic_id);

		modify_irte(irq, &irte);

		ir_entry->index2 = (index >> 15) & 0x1;
		ir_entry->zero = 0;
		ir_entry->format = 1;
		ir_entry->index = (index & 0x7fff);
		
		ir_entry->vector = pin;
	} else {
		entry->delivery_mode = apic->irq_delivery_mode;
		entry->dest_mode = apic->irq_dest_mode;
		entry->dest = destination;
		entry->vector = vector;
	}

	entry->mask = 0;				
	entry->trigger = trigger;
	entry->polarity = polarity;

	
	if (trigger)
		entry->mask = 1;
	return 0;
}

static void setup_IO_APIC_irq(int apic_id, int pin, unsigned int irq, struct irq_desc *desc,
			      int trigger, int polarity)
{
	struct irq_cfg *cfg;
	struct IO_APIC_route_entry entry;
	unsigned int dest;

	if (!IO_APIC_IRQ(irq))
		return;

	cfg = desc->chip_data;

	if (assign_irq_vector(irq, cfg, apic->target_cpus()))
		return;

	dest = apic->cpu_mask_to_apicid_and(cfg->domain, apic->target_cpus());

	apic_printk(APIC_VERBOSE,KERN_DEBUG
		    "IOAPIC[%d]: Set routing entry (%d-%d -> 0x%x -> "
		    "IRQ %d Mode:%i Active:%i)\n",
		    apic_id, mp_ioapics[apic_id].apicid, pin, cfg->vector,
		    irq, trigger, polarity);


	if (setup_ioapic_entry(mp_ioapics[apic_id].apicid, irq, &entry,
			       dest, trigger, polarity, cfg->vector, pin)) {
		printk("Failed to setup ioapic entry for ioapic  %d, pin %d\n",
		       mp_ioapics[apic_id].apicid, pin);
		__clear_irq_vector(irq, cfg);
		return;
	}

	ioapic_register_intr(irq, desc, trigger);
	if (irq < nr_legacy_irqs)
		disable_8259A_irq(irq);

	ioapic_write_entry(apic_id, pin, entry);
}

static struct {
	DECLARE_BITMAP(pin_programmed, MP_MAX_IOAPIC_PIN + 1);
} mp_ioapic_routing[MAX_IO_APICS];

static void __init setup_IO_APIC_irqs(void)
{
	int apic_id = 0, pin, idx, irq;
	int notcon = 0;
	struct irq_desc *desc;
	struct irq_cfg *cfg;
	int node = cpu_to_node(boot_cpu_id);

	apic_printk(APIC_VERBOSE, KERN_DEBUG "init IO_APIC IRQs\n");

#ifdef CONFIG_ACPI
	if (!acpi_disabled && acpi_ioapic) {
		apic_id = mp_find_ioapic(0);
		if (apic_id < 0)
			apic_id = 0;
	}
#endif

	for (pin = 0; pin < nr_ioapic_registers[apic_id]; pin++) {
		idx = find_irq_entry(apic_id, pin, mp_INT);
		if (idx == -1) {
			if (!notcon) {
				notcon = 1;
				apic_printk(APIC_VERBOSE,
					KERN_DEBUG " %d-%d",
					mp_ioapics[apic_id].apicid, pin);
			} else
				apic_printk(APIC_VERBOSE, " %d-%d",
					mp_ioapics[apic_id].apicid, pin);
			continue;
		}
		if (notcon) {
			apic_printk(APIC_VERBOSE,
				" (apicid-pin) not connected\n");
			notcon = 0;
		}

		irq = pin_2_irq(idx, apic_id, pin);

		
		if (apic->multi_timer_check &&
				apic->multi_timer_check(apic_id, irq))
			continue;

		desc = irq_to_desc_alloc_node(irq, node);
		if (!desc) {
			printk(KERN_INFO "can not get irq_desc for %d\n", irq);
			continue;
		}
		cfg = desc->chip_data;
		add_pin_to_irq_node(cfg, node, apic_id, pin);
		
		setup_IO_APIC_irq(apic_id, pin, irq, desc,
				irq_trigger(idx), irq_polarity(idx));
	}

	if (notcon)
		apic_printk(APIC_VERBOSE,
			" (apicid-pin) not connected\n");
}


static void __init setup_timer_IRQ0_pin(unsigned int apic_id, unsigned int pin,
					int vector)
{
	struct IO_APIC_route_entry entry;

	if (intr_remapping_enabled)
		return;

	memset(&entry, 0, sizeof(entry));

	
	entry.dest_mode = apic->irq_dest_mode;
	entry.mask = 0;			
	entry.dest = apic->cpu_mask_to_apicid(apic->target_cpus());
	entry.delivery_mode = apic->irq_delivery_mode;
	entry.polarity = 0;
	entry.trigger = 0;
	entry.vector = vector;

	
	set_irq_chip_and_handler_name(0, &ioapic_chip, handle_edge_irq, "edge");

	
	ioapic_write_entry(apic_id, pin, entry);
}


__apicdebuginit(void) print_IO_APIC(void)
{
	int apic, i;
	union IO_APIC_reg_00 reg_00;
	union IO_APIC_reg_01 reg_01;
	union IO_APIC_reg_02 reg_02;
	union IO_APIC_reg_03 reg_03;
	unsigned long flags;
	struct irq_cfg *cfg;
	struct irq_desc *desc;
	unsigned int irq;

	if (apic_verbosity == APIC_QUIET)
		return;

	printk(KERN_DEBUG "number of MP IRQ sources: %d.\n", mp_irq_entries);
	for (i = 0; i < nr_ioapics; i++)
		printk(KERN_DEBUG "number of IO-APIC #%d registers: %d.\n",
		       mp_ioapics[i].apicid, nr_ioapic_registers[i]);

	
	printk(KERN_INFO "testing the IO APIC.......................\n");

	for (apic = 0; apic < nr_ioapics; apic++) {

	spin_lock_irqsave(&ioapic_lock, flags);
	reg_00.raw = io_apic_read(apic, 0);
	reg_01.raw = io_apic_read(apic, 1);
	if (reg_01.bits.version >= 0x10)
		reg_02.raw = io_apic_read(apic, 2);
	if (reg_01.bits.version >= 0x20)
		reg_03.raw = io_apic_read(apic, 3);
	spin_unlock_irqrestore(&ioapic_lock, flags);

	printk("\n");
	printk(KERN_DEBUG "IO APIC #%d......\n", mp_ioapics[apic].apicid);
	printk(KERN_DEBUG ".... register #00: %08X\n", reg_00.raw);
	printk(KERN_DEBUG ".......    : physical APIC id: %02X\n", reg_00.bits.ID);
	printk(KERN_DEBUG ".......    : Delivery Type: %X\n", reg_00.bits.delivery_type);
	printk(KERN_DEBUG ".......    : LTS          : %X\n", reg_00.bits.LTS);

	printk(KERN_DEBUG ".... register #01: %08X\n", *(int *)&reg_01);
	printk(KERN_DEBUG ".......     : max redirection entries: %04X\n", reg_01.bits.entries);

	printk(KERN_DEBUG ".......     : PRQ implemented: %X\n", reg_01.bits.PRQ);
	printk(KERN_DEBUG ".......     : IO APIC version: %04X\n", reg_01.bits.version);

	
	if (reg_01.bits.version >= 0x10 && reg_02.raw != reg_01.raw) {
		printk(KERN_DEBUG ".... register #02: %08X\n", reg_02.raw);
		printk(KERN_DEBUG ".......     : arbitration: %02X\n", reg_02.bits.arbitration);
	}

	
	if (reg_01.bits.version >= 0x20 && reg_03.raw != reg_02.raw &&
	    reg_03.raw != reg_01.raw) {
		printk(KERN_DEBUG ".... register #03: %08X\n", reg_03.raw);
		printk(KERN_DEBUG ".......     : Boot DT    : %X\n", reg_03.bits.boot_DT);
	}

	printk(KERN_DEBUG ".... IRQ redirection table:\n");

	printk(KERN_DEBUG " NR Dst Mask Trig IRR Pol"
			  " Stat Dmod Deli Vect:   \n");

	for (i = 0; i <= reg_01.bits.entries; i++) {
		struct IO_APIC_route_entry entry;

		entry = ioapic_read_entry(apic, i);

		printk(KERN_DEBUG " %02x %03X ",
			i,
			entry.dest
		);

		printk("%1d    %1d    %1d   %1d   %1d    %1d    %1d    %02X\n",
			entry.mask,
			entry.trigger,
			entry.irr,
			entry.polarity,
			entry.delivery_status,
			entry.dest_mode,
			entry.delivery_mode,
			entry.vector
		);
	}
	}
	printk(KERN_DEBUG "IRQ to pin mappings:\n");
	for_each_irq_desc(irq, desc) {
		struct irq_pin_list *entry;

		cfg = desc->chip_data;
		entry = cfg->irq_2_pin;
		if (!entry)
			continue;
		printk(KERN_DEBUG "IRQ%d ", irq);
		for_each_irq_pin(entry, cfg->irq_2_pin)
			printk("-> %d:%d", entry->apic, entry->pin);
		printk("\n");
	}

	printk(KERN_INFO ".................................... done.\n");

	return;
}

__apicdebuginit(void) print_APIC_field(int base)
{
	int i;

	if (apic_verbosity == APIC_QUIET)
		return;

	printk(KERN_DEBUG);

	for (i = 0; i < 8; i++)
		printk(KERN_CONT "%08x", apic_read(base + i*0x10));

	printk(KERN_CONT "\n");
}

__apicdebuginit(void) print_local_APIC(void *dummy)
{
	unsigned int i, v, ver, maxlvt;
	u64 icr;

	if (apic_verbosity == APIC_QUIET)
		return;

	printk(KERN_DEBUG "printing local APIC contents on CPU#%d/%d:\n",
		smp_processor_id(), hard_smp_processor_id());
	v = apic_read(APIC_ID);
	printk(KERN_INFO "... APIC ID:      %08x (%01x)\n", v, read_apic_id());
	v = apic_read(APIC_LVR);
	printk(KERN_INFO "... APIC VERSION: %08x\n", v);
	ver = GET_APIC_VERSION(v);
	maxlvt = lapic_get_maxlvt();

	v = apic_read(APIC_TASKPRI);
	printk(KERN_DEBUG "... APIC TASKPRI: %08x (%02x)\n", v, v & APIC_TPRI_MASK);

	if (APIC_INTEGRATED(ver)) {                     
		if (!APIC_XAPIC(ver)) {
			v = apic_read(APIC_ARBPRI);
			printk(KERN_DEBUG "... APIC ARBPRI: %08x (%02x)\n", v,
			       v & APIC_ARBPRI_MASK);
		}
		v = apic_read(APIC_PROCPRI);
		printk(KERN_DEBUG "... APIC PROCPRI: %08x\n", v);
	}

	
	if (!APIC_INTEGRATED(ver) || maxlvt == 3) {
		v = apic_read(APIC_RRR);
		printk(KERN_DEBUG "... APIC RRR: %08x\n", v);
	}

	v = apic_read(APIC_LDR);
	printk(KERN_DEBUG "... APIC LDR: %08x\n", v);
	if (!x2apic_enabled()) {
		v = apic_read(APIC_DFR);
		printk(KERN_DEBUG "... APIC DFR: %08x\n", v);
	}
	v = apic_read(APIC_SPIV);
	printk(KERN_DEBUG "... APIC SPIV: %08x\n", v);

	printk(KERN_DEBUG "... APIC ISR field:\n");
	print_APIC_field(APIC_ISR);
	printk(KERN_DEBUG "... APIC TMR field:\n");
	print_APIC_field(APIC_TMR);
	printk(KERN_DEBUG "... APIC IRR field:\n");
	print_APIC_field(APIC_IRR);

	if (APIC_INTEGRATED(ver)) {             
		if (maxlvt > 3)         
			apic_write(APIC_ESR, 0);

		v = apic_read(APIC_ESR);
		printk(KERN_DEBUG "... APIC ESR: %08x\n", v);
	}

	icr = apic_icr_read();
	printk(KERN_DEBUG "... APIC ICR: %08x\n", (u32)icr);
	printk(KERN_DEBUG "... APIC ICR2: %08x\n", (u32)(icr >> 32));

	v = apic_read(APIC_LVTT);
	printk(KERN_DEBUG "... APIC LVTT: %08x\n", v);

	if (maxlvt > 3) {                       
		v = apic_read(APIC_LVTPC);
		printk(KERN_DEBUG "... APIC LVTPC: %08x\n", v);
	}
	v = apic_read(APIC_LVT0);
	printk(KERN_DEBUG "... APIC LVT0: %08x\n", v);
	v = apic_read(APIC_LVT1);
	printk(KERN_DEBUG "... APIC LVT1: %08x\n", v);

	if (maxlvt > 2) {			
		v = apic_read(APIC_LVTERR);
		printk(KERN_DEBUG "... APIC LVTERR: %08x\n", v);
	}

	v = apic_read(APIC_TMICT);
	printk(KERN_DEBUG "... APIC TMICT: %08x\n", v);
	v = apic_read(APIC_TMCCT);
	printk(KERN_DEBUG "... APIC TMCCT: %08x\n", v);
	v = apic_read(APIC_TDCR);
	printk(KERN_DEBUG "... APIC TDCR: %08x\n", v);

	if (boot_cpu_has(X86_FEATURE_EXTAPIC)) {
		v = apic_read(APIC_EFEAT);
		maxlvt = (v >> 16) & 0xff;
		printk(KERN_DEBUG "... APIC EFEAT: %08x\n", v);
		v = apic_read(APIC_ECTRL);
		printk(KERN_DEBUG "... APIC ECTRL: %08x\n", v);
		for (i = 0; i < maxlvt; i++) {
			v = apic_read(APIC_EILVTn(i));
			printk(KERN_DEBUG "... APIC EILVT%d: %08x\n", i, v);
		}
	}
	printk("\n");
}

__apicdebuginit(void) print_all_local_APICs(void)
{
	int cpu;

	preempt_disable();
	for_each_online_cpu(cpu)
		smp_call_function_single(cpu, print_local_APIC, NULL, 1);
	preempt_enable();
}

__apicdebuginit(void) print_PIC(void)
{
	unsigned int v;
	unsigned long flags;

	if (apic_verbosity == APIC_QUIET || !nr_legacy_irqs)
		return;

	printk(KERN_DEBUG "\nprinting PIC contents\n");

	spin_lock_irqsave(&i8259A_lock, flags);

	v = inb(0xa1) << 8 | inb(0x21);
	printk(KERN_DEBUG "... PIC  IMR: %04x\n", v);

	v = inb(0xa0) << 8 | inb(0x20);
	printk(KERN_DEBUG "... PIC  IRR: %04x\n", v);

	outb(0x0b,0xa0);
	outb(0x0b,0x20);
	v = inb(0xa0) << 8 | inb(0x20);
	outb(0x0a,0xa0);
	outb(0x0a,0x20);

	spin_unlock_irqrestore(&i8259A_lock, flags);

	printk(KERN_DEBUG "... PIC  ISR: %04x\n", v);

	v = inb(0x4d1) << 8 | inb(0x4d0);
	printk(KERN_DEBUG "... PIC ELCR: %04x\n", v);
}

__apicdebuginit(int) print_all_ICs(void)
{
	print_PIC();

	
	if (!cpu_has_apic && !apic_from_smp_config())
		return 0;

	print_all_local_APICs();
	print_IO_APIC();

	return 0;
}

fs_initcall(print_all_ICs);



static struct { int pin, apic; } ioapic_i8259 = { -1, -1 };

void __init enable_IO_APIC(void)
{
	union IO_APIC_reg_01 reg_01;
	int i8259_apic, i8259_pin;
	int apic;
	unsigned long flags;

	
	for (apic = 0; apic < nr_ioapics; apic++) {
		spin_lock_irqsave(&ioapic_lock, flags);
		reg_01.raw = io_apic_read(apic, 1);
		spin_unlock_irqrestore(&ioapic_lock, flags);
		nr_ioapic_registers[apic] = reg_01.bits.entries+1;
	}

	if (!nr_legacy_irqs)
		return;

	for(apic = 0; apic < nr_ioapics; apic++) {
		int pin;
		
		for (pin = 0; pin < nr_ioapic_registers[apic]; pin++) {
			struct IO_APIC_route_entry entry;
			entry = ioapic_read_entry(apic, pin);

			
			if ((entry.mask == 0) && (entry.delivery_mode == dest_ExtINT)) {
				ioapic_i8259.apic = apic;
				ioapic_i8259.pin  = pin;
				goto found_i8259;
			}
		}
	}
 found_i8259:
	
	
	i8259_pin  = find_isa_irq_pin(0, mp_ExtINT);
	i8259_apic = find_isa_irq_apic(0, mp_ExtINT);
	
	if ((ioapic_i8259.pin == -1) && (i8259_pin >= 0)) {
		printk(KERN_WARNING "ExtINT not setup in hardware but reported by MP table\n");
		ioapic_i8259.pin  = i8259_pin;
		ioapic_i8259.apic = i8259_apic;
	}
	
	if (((ioapic_i8259.apic != i8259_apic) || (ioapic_i8259.pin != i8259_pin)) &&
		(i8259_pin >= 0) && (ioapic_i8259.pin >= 0))
	{
		printk(KERN_WARNING "ExtINT in hardware and MP table differ\n");
	}

	
	clear_IO_APIC();
}


void disable_IO_APIC(void)
{
	
	clear_IO_APIC();

	if (!nr_legacy_irqs)
		return;

	
	if (ioapic_i8259.pin != -1 && !intr_remapping_enabled) {
		struct IO_APIC_route_entry entry;

		memset(&entry, 0, sizeof(entry));
		entry.mask            = 0; 
		entry.trigger         = 0; 
		entry.irr             = 0;
		entry.polarity        = 0; 
		entry.delivery_status = 0;
		entry.dest_mode       = 0; 
		entry.delivery_mode   = dest_ExtINT; 
		entry.vector          = 0;
		entry.dest            = read_apic_id();

		
		ioapic_write_entry(ioapic_i8259.apic, ioapic_i8259.pin, entry);
	}

	
	if (cpu_has_apic || apic_from_smp_config())
		disconnect_bsp_APIC(!intr_remapping_enabled &&
				ioapic_i8259.pin != -1);
}

#ifdef CONFIG_X86_32


void __init setup_ioapic_ids_from_mpc(void)
{
	union IO_APIC_reg_00 reg_00;
	physid_mask_t phys_id_present_map;
	int apic_id;
	int i;
	unsigned char old_id;
	unsigned long flags;

	if (acpi_ioapic)
		return;
	
	if (!(boot_cpu_data.x86_vendor == X86_VENDOR_INTEL)
		|| APIC_XAPIC(apic_version[boot_cpu_physical_apicid]))
		return;
	
	phys_id_present_map = apic->ioapic_phys_id_map(phys_cpu_present_map);

	
	for (apic_id = 0; apic_id < nr_ioapics; apic_id++) {

		
		spin_lock_irqsave(&ioapic_lock, flags);
		reg_00.raw = io_apic_read(apic_id, 0);
		spin_unlock_irqrestore(&ioapic_lock, flags);

		old_id = mp_ioapics[apic_id].apicid;

		if (mp_ioapics[apic_id].apicid >= get_physical_broadcast()) {
			printk(KERN_ERR "BIOS bug, IO-APIC#%d ID is %d in the MPC table!...\n",
				apic_id, mp_ioapics[apic_id].apicid);
			printk(KERN_ERR "... fixing up to %d. (tell your hw vendor)\n",
				reg_00.bits.ID);
			mp_ioapics[apic_id].apicid = reg_00.bits.ID;
		}

		
		if (apic->check_apicid_used(phys_id_present_map,
					mp_ioapics[apic_id].apicid)) {
			printk(KERN_ERR "BIOS bug, IO-APIC#%d ID %d is already used!...\n",
				apic_id, mp_ioapics[apic_id].apicid);
			for (i = 0; i < get_physical_broadcast(); i++)
				if (!physid_isset(i, phys_id_present_map))
					break;
			if (i >= get_physical_broadcast())
				panic("Max APIC ID exceeded!\n");
			printk(KERN_ERR "... fixing up to %d. (tell your hw vendor)\n",
				i);
			physid_set(i, phys_id_present_map);
			mp_ioapics[apic_id].apicid = i;
		} else {
			physid_mask_t tmp;
			tmp = apic->apicid_to_cpu_present(mp_ioapics[apic_id].apicid);
			apic_printk(APIC_VERBOSE, "Setting %d in the "
					"phys_id_present_map\n",
					mp_ioapics[apic_id].apicid);
			physids_or(phys_id_present_map, phys_id_present_map, tmp);
		}


		
		if (old_id != mp_ioapics[apic_id].apicid)
			for (i = 0; i < mp_irq_entries; i++)
				if (mp_irqs[i].dstapic == old_id)
					mp_irqs[i].dstapic
						= mp_ioapics[apic_id].apicid;

		
		apic_printk(APIC_VERBOSE, KERN_INFO
			"...changing IO-APIC physical APIC ID to %d ...",
			mp_ioapics[apic_id].apicid);

		reg_00.bits.ID = mp_ioapics[apic_id].apicid;
		spin_lock_irqsave(&ioapic_lock, flags);
		io_apic_write(apic_id, 0, reg_00.raw);
		spin_unlock_irqrestore(&ioapic_lock, flags);

		
		spin_lock_irqsave(&ioapic_lock, flags);
		reg_00.raw = io_apic_read(apic_id, 0);
		spin_unlock_irqrestore(&ioapic_lock, flags);
		if (reg_00.bits.ID != mp_ioapics[apic_id].apicid)
			printk("could not set ID!\n");
		else
			apic_printk(APIC_VERBOSE, " ok.\n");
	}
}
#endif

int no_timer_check __initdata;

static int __init notimercheck(char *s)
{
	no_timer_check = 1;
	return 1;
}
__setup("no_timer_check", notimercheck);


static int __init timer_irq_works(void)
{
	unsigned long t1 = jiffies;
	unsigned long flags;

	if (no_timer_check)
		return 1;

	local_save_flags(flags);
	local_irq_enable();
	
	mdelay((10 * 1000) / HZ);
	local_irq_restore(flags);

	

	
	if (time_after(jiffies, t1 + 4))
		return 1;
	return 0;
}






static unsigned int startup_ioapic_irq(unsigned int irq)
{
	int was_pending = 0;
	unsigned long flags;
	struct irq_cfg *cfg;

	spin_lock_irqsave(&ioapic_lock, flags);
	if (irq < nr_legacy_irqs) {
		disable_8259A_irq(irq);
		if (i8259A_irq_pending(irq))
			was_pending = 1;
	}
	cfg = irq_cfg(irq);
	__unmask_IO_APIC_irq(cfg);
	spin_unlock_irqrestore(&ioapic_lock, flags);

	return was_pending;
}

static int ioapic_retrigger_irq(unsigned int irq)
{

	struct irq_cfg *cfg = irq_cfg(irq);
	unsigned long flags;

	spin_lock_irqsave(&vector_lock, flags);
	apic->send_IPI_mask(cpumask_of(cpumask_first(cfg->domain)), cfg->vector);
	spin_unlock_irqrestore(&vector_lock, flags);

	return 1;
}



#ifdef CONFIG_SMP
static void send_cleanup_vector(struct irq_cfg *cfg)
{
	cpumask_var_t cleanup_mask;

	if (unlikely(!alloc_cpumask_var(&cleanup_mask, GFP_ATOMIC))) {
		unsigned int i;
		cfg->move_cleanup_count = 0;
		for_each_cpu_and(i, cfg->old_domain, cpu_online_mask)
			cfg->move_cleanup_count++;
		for_each_cpu_and(i, cfg->old_domain, cpu_online_mask)
			apic->send_IPI_mask(cpumask_of(i), IRQ_MOVE_CLEANUP_VECTOR);
	} else {
		cpumask_and(cleanup_mask, cfg->old_domain, cpu_online_mask);
		cfg->move_cleanup_count = cpumask_weight(cleanup_mask);
		apic->send_IPI_mask(cleanup_mask, IRQ_MOVE_CLEANUP_VECTOR);
		free_cpumask_var(cleanup_mask);
	}
	cfg->move_in_progress = 0;
}

static void __target_IO_APIC_irq(unsigned int irq, unsigned int dest, struct irq_cfg *cfg)
{
	int apic, pin;
	struct irq_pin_list *entry;
	u8 vector = cfg->vector;

	for_each_irq_pin(entry, cfg->irq_2_pin) {
		unsigned int reg;

		apic = entry->apic;
		pin = entry->pin;
		
		if (!irq_remapped(irq))
			io_apic_write(apic, 0x11 + pin*2, dest);
		reg = io_apic_read(apic, 0x10 + pin*2);
		reg &= ~IO_APIC_REDIR_VECTOR_MASK;
		reg |= vector;
		io_apic_modify(apic, 0x10 + pin*2, reg);
	}
}

static int
assign_irq_vector(int irq, struct irq_cfg *cfg, const struct cpumask *mask);


static unsigned int
set_desc_affinity(struct irq_desc *desc, const struct cpumask *mask)
{
	struct irq_cfg *cfg;
	unsigned int irq;

	if (!cpumask_intersects(mask, cpu_online_mask))
		return BAD_APICID;

	irq = desc->irq;
	cfg = desc->chip_data;
	if (assign_irq_vector(irq, cfg, mask))
		return BAD_APICID;

	cpumask_copy(desc->affinity, mask);

	return apic->cpu_mask_to_apicid_and(desc->affinity, cfg->domain);
}

static int
set_ioapic_affinity_irq_desc(struct irq_desc *desc, const struct cpumask *mask)
{
	struct irq_cfg *cfg;
	unsigned long flags;
	unsigned int dest;
	unsigned int irq;
	int ret = -1;

	irq = desc->irq;
	cfg = desc->chip_data;

	spin_lock_irqsave(&ioapic_lock, flags);
	dest = set_desc_affinity(desc, mask);
	if (dest != BAD_APICID) {
		
		dest = SET_APIC_LOGICAL_ID(dest);
		__target_IO_APIC_irq(irq, dest, cfg);
		ret = 0;
	}
	spin_unlock_irqrestore(&ioapic_lock, flags);

	return ret;
}

static int
set_ioapic_affinity_irq(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);

	return set_ioapic_affinity_irq_desc(desc, mask);
}

#ifdef CONFIG_INTR_REMAP


static int
migrate_ioapic_irq_desc(struct irq_desc *desc, const struct cpumask *mask)
{
	struct irq_cfg *cfg;
	struct irte irte;
	unsigned int dest;
	unsigned int irq;
	int ret = -1;

	if (!cpumask_intersects(mask, cpu_online_mask))
		return ret;

	irq = desc->irq;
	if (get_irte(irq, &irte))
		return ret;

	cfg = desc->chip_data;
	if (assign_irq_vector(irq, cfg, mask))
		return ret;

	dest = apic->cpu_mask_to_apicid_and(cfg->domain, mask);

	irte.vector = cfg->vector;
	irte.dest_id = IRTE_DEST(dest);

	
	modify_irte(irq, &irte);

	if (cfg->move_in_progress)
		send_cleanup_vector(cfg);

	cpumask_copy(desc->affinity, mask);

	return 0;
}


static int set_ir_ioapic_affinity_irq_desc(struct irq_desc *desc,
					    const struct cpumask *mask)
{
	return migrate_ioapic_irq_desc(desc, mask);
}
static int set_ir_ioapic_affinity_irq(unsigned int irq,
				       const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);

	return set_ir_ioapic_affinity_irq_desc(desc, mask);
}
#else
static inline int set_ir_ioapic_affinity_irq_desc(struct irq_desc *desc,
						   const struct cpumask *mask)
{
	return 0;
}
#endif

asmlinkage void smp_irq_move_cleanup_interrupt(void)
{
	unsigned vector, me;

	ack_APIC_irq();
	exit_idle();
	irq_enter();

	me = smp_processor_id();
	for (vector = FIRST_EXTERNAL_VECTOR; vector < NR_VECTORS; vector++) {
		unsigned int irq;
		unsigned int irr;
		struct irq_desc *desc;
		struct irq_cfg *cfg;
		irq = __get_cpu_var(vector_irq)[vector];

		if (irq == -1)
			continue;

		desc = irq_to_desc(irq);
		if (!desc)
			continue;

		cfg = irq_cfg(irq);
		spin_lock(&desc->lock);
		if (!cfg->move_cleanup_count)
			goto unlock;

		if (vector == cfg->vector && cpumask_test_cpu(me, cfg->domain))
			goto unlock;

		irr = apic_read(APIC_IRR + (vector / 32 * 0x10));
		
		if (irr  & (1 << (vector % 32))) {
			apic->send_IPI_self(IRQ_MOVE_CLEANUP_VECTOR);
			goto unlock;
		}
		__get_cpu_var(vector_irq)[vector] = -1;
		cfg->move_cleanup_count--;
unlock:
		spin_unlock(&desc->lock);
	}

	irq_exit();
}

static void irq_complete_move(struct irq_desc **descp)
{
	struct irq_desc *desc = *descp;
	struct irq_cfg *cfg = desc->chip_data;
	unsigned vector, me;

	if (likely(!cfg->move_in_progress))
		return;

	vector = ~get_irq_regs()->orig_ax;
	me = smp_processor_id();

	if (vector == cfg->vector && cpumask_test_cpu(me, cfg->domain))
		send_cleanup_vector(cfg);
}
#else
static inline void irq_complete_move(struct irq_desc **descp) {}
#endif

static void ack_apic_edge(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	irq_complete_move(&desc);
	move_native_irq(irq);
	ack_APIC_irq();
}

atomic_t irq_mis_count;

static void ack_apic_level(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long v;
	int i;
	struct irq_cfg *cfg;
	int do_unmask_irq = 0;

	irq_complete_move(&desc);
#ifdef CONFIG_GENERIC_PENDING_IRQ
	
	if (unlikely(desc->status & IRQ_MOVE_PENDING)) {
		do_unmask_irq = 1;
		mask_IO_APIC_irq_desc(desc);
	}
#endif

	
	cfg = desc->chip_data;
	i = cfg->vector;
	v = apic_read(APIC_TMR + ((i & ~0x1f) >> 1));

	
	ack_APIC_irq();

	
	if (unlikely(do_unmask_irq)) {
		
		cfg = desc->chip_data;
		if (!io_apic_level_ack_pending(cfg))
			move_masked_irq(irq);
		unmask_IO_APIC_irq_desc(desc);
	}

	
	if (!(v & (1 << (i & 0x1f)))) {
		atomic_inc(&irq_mis_count);
		spin_lock(&ioapic_lock);
		__mask_and_edge_IO_APIC_irq(cfg);
		__unmask_and_level_IO_APIC_irq(cfg);
		spin_unlock(&ioapic_lock);
	}
}

#ifdef CONFIG_INTR_REMAP
static void __eoi_ioapic_irq(unsigned int irq, struct irq_cfg *cfg)
{
	struct irq_pin_list *entry;

	for_each_irq_pin(entry, cfg->irq_2_pin)
		io_apic_eoi(entry->apic, entry->pin);
}

static void
eoi_ioapic_irq(struct irq_desc *desc)
{
	struct irq_cfg *cfg;
	unsigned long flags;
	unsigned int irq;

	irq = desc->irq;
	cfg = desc->chip_data;

	spin_lock_irqsave(&ioapic_lock, flags);
	__eoi_ioapic_irq(irq, cfg);
	spin_unlock_irqrestore(&ioapic_lock, flags);
}

static void ir_ack_apic_edge(unsigned int irq)
{
	ack_APIC_irq();
}

static void ir_ack_apic_level(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	ack_APIC_irq();
	eoi_ioapic_irq(desc);
}
#endif 

static struct irq_chip ioapic_chip __read_mostly = {
	.name		= "IO-APIC",
	.startup	= startup_ioapic_irq,
	.mask		= mask_IO_APIC_irq,
	.unmask		= unmask_IO_APIC_irq,
	.ack		= ack_apic_edge,
	.eoi		= ack_apic_level,
#ifdef CONFIG_SMP
	.set_affinity	= set_ioapic_affinity_irq,
#endif
	.retrigger	= ioapic_retrigger_irq,
};

static struct irq_chip ir_ioapic_chip __read_mostly = {
	.name		= "IR-IO-APIC",
	.startup	= startup_ioapic_irq,
	.mask		= mask_IO_APIC_irq,
	.unmask		= unmask_IO_APIC_irq,
#ifdef CONFIG_INTR_REMAP
	.ack		= ir_ack_apic_edge,
	.eoi		= ir_ack_apic_level,
#ifdef CONFIG_SMP
	.set_affinity	= set_ir_ioapic_affinity_irq,
#endif
#endif
	.retrigger	= ioapic_retrigger_irq,
};

static inline void init_IO_APIC_traps(void)
{
	int irq;
	struct irq_desc *desc;
	struct irq_cfg *cfg;

	
	for_each_irq_desc(irq, desc) {
		cfg = desc->chip_data;
		if (IO_APIC_IRQ(irq) && cfg && !cfg->vector) {
			
			if (irq < nr_legacy_irqs)
				make_8259A_irq(irq);
			else
				
				desc->chip = &no_irq_chip;
		}
	}
}



static void mask_lapic_irq(unsigned int irq)
{
	unsigned long v;

	v = apic_read(APIC_LVT0);
	apic_write(APIC_LVT0, v | APIC_LVT_MASKED);
}

static void unmask_lapic_irq(unsigned int irq)
{
	unsigned long v;

	v = apic_read(APIC_LVT0);
	apic_write(APIC_LVT0, v & ~APIC_LVT_MASKED);
}

static void ack_lapic_irq(unsigned int irq)
{
	ack_APIC_irq();
}

static struct irq_chip lapic_chip __read_mostly = {
	.name		= "local-APIC",
	.mask		= mask_lapic_irq,
	.unmask		= unmask_lapic_irq,
	.ack		= ack_lapic_irq,
};

static void lapic_register_intr(int irq, struct irq_desc *desc)
{
	desc->status &= ~IRQ_LEVEL;
	set_irq_chip_and_handler_name(irq, &lapic_chip, handle_edge_irq,
				      "edge");
}

static void __init setup_nmi(void)
{
	
	apic_printk(APIC_VERBOSE, KERN_INFO "activating NMI Watchdog ...");

	enable_NMI_through_LVT0();

	apic_printk(APIC_VERBOSE, " done.\n");
}


static inline void __init unlock_ExtINT_logic(void)
{
	int apic, pin, i;
	struct IO_APIC_route_entry entry0, entry1;
	unsigned char save_control, save_freq_select;

	pin  = find_isa_irq_pin(8, mp_INT);
	if (pin == -1) {
		WARN_ON_ONCE(1);
		return;
	}
	apic = find_isa_irq_apic(8, mp_INT);
	if (apic == -1) {
		WARN_ON_ONCE(1);
		return;
	}

	entry0 = ioapic_read_entry(apic, pin);
	clear_IO_APIC_pin(apic, pin);

	memset(&entry1, 0, sizeof(entry1));

	entry1.dest_mode = 0;			
	entry1.mask = 0;			
	entry1.dest = hard_smp_processor_id();
	entry1.delivery_mode = dest_ExtINT;
	entry1.polarity = entry0.polarity;
	entry1.trigger = 0;
	entry1.vector = 0;

	ioapic_write_entry(apic, pin, entry1);

	save_control = CMOS_READ(RTC_CONTROL);
	save_freq_select = CMOS_READ(RTC_FREQ_SELECT);
	CMOS_WRITE((save_freq_select & ~RTC_RATE_SELECT) | 0x6,
		   RTC_FREQ_SELECT);
	CMOS_WRITE(save_control | RTC_PIE, RTC_CONTROL);

	i = 100;
	while (i-- > 0) {
		mdelay(10);
		if ((CMOS_READ(RTC_INTR_FLAGS) & RTC_PF) == RTC_PF)
			i -= 10;
	}

	CMOS_WRITE(save_control, RTC_CONTROL);
	CMOS_WRITE(save_freq_select, RTC_FREQ_SELECT);
	clear_IO_APIC_pin(apic, pin);

	ioapic_write_entry(apic, pin, entry0);
}

static int disable_timer_pin_1 __initdata;

static int __init disable_timer_pin_setup(char *arg)
{
	disable_timer_pin_1 = 1;
	return 0;
}
early_param("disable_timer_pin_1", disable_timer_pin_setup);

int timer_through_8259 __initdata;


static inline void __init check_timer(void)
{
	struct irq_desc *desc = irq_to_desc(0);
	struct irq_cfg *cfg = desc->chip_data;
	int node = cpu_to_node(boot_cpu_id);
	int apic1, pin1, apic2, pin2;
	unsigned long flags;
	int no_pin1 = 0;

	local_irq_save(flags);

	
	disable_8259A_irq(0);
	assign_irq_vector(0, cfg, apic->target_cpus());

	
	apic_write(APIC_LVT0, APIC_LVT_MASKED | APIC_DM_EXTINT);
	init_8259A(1);
#ifdef CONFIG_X86_32
	{
		unsigned int ver;

		ver = apic_read(APIC_LVR);
		ver = GET_APIC_VERSION(ver);
		timer_ack = (nmi_watchdog == NMI_IO_APIC && !APIC_INTEGRATED(ver));
	}
#endif

	pin1  = find_isa_irq_pin(0, mp_INT);
	apic1 = find_isa_irq_apic(0, mp_INT);
	pin2  = ioapic_i8259.pin;
	apic2 = ioapic_i8259.apic;

	apic_printk(APIC_QUIET, KERN_INFO "..TIMER: vector=0x%02X "
		    "apic1=%d pin1=%d apic2=%d pin2=%d\n",
		    cfg->vector, apic1, pin1, apic2, pin2);

	
	if (pin1 == -1) {
		if (intr_remapping_enabled)
			panic("BIOS bug: timer not connected to IO-APIC");
		pin1 = pin2;
		apic1 = apic2;
		no_pin1 = 1;
	} else if (pin2 == -1) {
		pin2 = pin1;
		apic2 = apic1;
	}

	if (pin1 != -1) {
		
		if (no_pin1) {
			add_pin_to_irq_node(cfg, node, apic1, pin1);
			setup_timer_IRQ0_pin(apic1, pin1, cfg->vector);
		} else {
			
			int idx;
			idx = find_irq_entry(apic1, pin1, mp_INT);
			if (idx != -1 && irq_trigger(idx))
				unmask_IO_APIC_irq_desc(desc);
		}
		if (timer_irq_works()) {
			if (nmi_watchdog == NMI_IO_APIC) {
				setup_nmi();
				enable_8259A_irq(0);
			}
			if (disable_timer_pin_1 > 0)
				clear_IO_APIC_pin(0, pin1);
			goto out;
		}
		if (intr_remapping_enabled)
			panic("timer doesn't work through Interrupt-remapped IO-APIC");
		local_irq_disable();
		clear_IO_APIC_pin(apic1, pin1);
		if (!no_pin1)
			apic_printk(APIC_QUIET, KERN_ERR "..MP-BIOS bug: "
				    "8254 timer not connected to IO-APIC\n");

		apic_printk(APIC_QUIET, KERN_INFO "...trying to set up timer "
			    "(IRQ0) through the 8259A ...\n");
		apic_printk(APIC_QUIET, KERN_INFO
			    "..... (found apic %d pin %d) ...\n", apic2, pin2);
		
		replace_pin_at_irq_node(cfg, node, apic1, pin1, apic2, pin2);
		setup_timer_IRQ0_pin(apic2, pin2, cfg->vector);
		enable_8259A_irq(0);
		if (timer_irq_works()) {
			apic_printk(APIC_QUIET, KERN_INFO "....... works.\n");
			timer_through_8259 = 1;
			if (nmi_watchdog == NMI_IO_APIC) {
				disable_8259A_irq(0);
				setup_nmi();
				enable_8259A_irq(0);
			}
			goto out;
		}
		
		local_irq_disable();
		disable_8259A_irq(0);
		clear_IO_APIC_pin(apic2, pin2);
		apic_printk(APIC_QUIET, KERN_INFO "....... failed.\n");
	}

	if (nmi_watchdog == NMI_IO_APIC) {
		apic_printk(APIC_QUIET, KERN_WARNING "timer doesn't work "
			    "through the IO-APIC - disabling NMI Watchdog!\n");
		nmi_watchdog = NMI_NONE;
	}
#ifdef CONFIG_X86_32
	timer_ack = 0;
#endif

	apic_printk(APIC_QUIET, KERN_INFO
		    "...trying to set up timer as Virtual Wire IRQ...\n");

	lapic_register_intr(0, desc);
	apic_write(APIC_LVT0, APIC_DM_FIXED | cfg->vector);	
	enable_8259A_irq(0);

	if (timer_irq_works()) {
		apic_printk(APIC_QUIET, KERN_INFO "..... works.\n");
		goto out;
	}
	local_irq_disable();
	disable_8259A_irq(0);
	apic_write(APIC_LVT0, APIC_LVT_MASKED | APIC_DM_FIXED | cfg->vector);
	apic_printk(APIC_QUIET, KERN_INFO "..... failed.\n");

	apic_printk(APIC_QUIET, KERN_INFO
		    "...trying to set up timer as ExtINT IRQ...\n");

	init_8259A(0);
	make_8259A_irq(0);
	apic_write(APIC_LVT0, APIC_DM_EXTINT);

	unlock_ExtINT_logic();

	if (timer_irq_works()) {
		apic_printk(APIC_QUIET, KERN_INFO "..... works.\n");
		goto out;
	}
	local_irq_disable();
	apic_printk(APIC_QUIET, KERN_INFO "..... failed :(.\n");
	panic("IO-APIC + timer doesn't work!  Boot with apic=debug and send a "
		"report.  Then try booting with the 'noapic' option.\n");
out:
	local_irq_restore(flags);
}


#define PIC_IRQS	(1UL << PIC_CASCADE_IR)

void __init setup_IO_APIC(void)
{

	
	io_apic_irqs = nr_legacy_irqs ? ~PIC_IRQS : ~0UL;

	apic_printk(APIC_VERBOSE, "ENABLING IO-APIC IRQs\n");
	
	x86_init.mpparse.setup_ioapic_ids();

	sync_Arb_IDs();
	setup_IO_APIC_irqs();
	init_IO_APIC_traps();
	if (nr_legacy_irqs)
		check_timer();
}



static int __init io_apic_bug_finalize(void)
{
	if (sis_apic_bug == -1)
		sis_apic_bug = 0;
	return 0;
}

late_initcall(io_apic_bug_finalize);

struct sysfs_ioapic_data {
	struct sys_device dev;
	struct IO_APIC_route_entry entry[0];
};
static struct sysfs_ioapic_data * mp_ioapic_data[MAX_IO_APICS];

static int ioapic_suspend(struct sys_device *dev, pm_message_t state)
{
	struct IO_APIC_route_entry *entry;
	struct sysfs_ioapic_data *data;
	int i;

	data = container_of(dev, struct sysfs_ioapic_data, dev);
	entry = data->entry;
	for (i = 0; i < nr_ioapic_registers[dev->id]; i ++, entry ++ )
		*entry = ioapic_read_entry(dev->id, i);

	return 0;
}

static int ioapic_resume(struct sys_device *dev)
{
	struct IO_APIC_route_entry *entry;
	struct sysfs_ioapic_data *data;
	unsigned long flags;
	union IO_APIC_reg_00 reg_00;
	int i;

	data = container_of(dev, struct sysfs_ioapic_data, dev);
	entry = data->entry;

	spin_lock_irqsave(&ioapic_lock, flags);
	reg_00.raw = io_apic_read(dev->id, 0);
	if (reg_00.bits.ID != mp_ioapics[dev->id].apicid) {
		reg_00.bits.ID = mp_ioapics[dev->id].apicid;
		io_apic_write(dev->id, 0, reg_00.raw);
	}
	spin_unlock_irqrestore(&ioapic_lock, flags);
	for (i = 0; i < nr_ioapic_registers[dev->id]; i++)
		ioapic_write_entry(dev->id, i, entry[i]);

	return 0;
}

static struct sysdev_class ioapic_sysdev_class = {
	.name = "ioapic",
	.suspend = ioapic_suspend,
	.resume = ioapic_resume,
};

static int __init ioapic_init_sysfs(void)
{
	struct sys_device * dev;
	int i, size, error;

	error = sysdev_class_register(&ioapic_sysdev_class);
	if (error)
		return error;

	for (i = 0; i < nr_ioapics; i++ ) {
		size = sizeof(struct sys_device) + nr_ioapic_registers[i]
			* sizeof(struct IO_APIC_route_entry);
		mp_ioapic_data[i] = kzalloc(size, GFP_KERNEL);
		if (!mp_ioapic_data[i]) {
			printk(KERN_ERR "Can't suspend/resume IOAPIC %d\n", i);
			continue;
		}
		dev = &mp_ioapic_data[i]->dev;
		dev->id = i;
		dev->cls = &ioapic_sysdev_class;
		error = sysdev_register(dev);
		if (error) {
			kfree(mp_ioapic_data[i]);
			mp_ioapic_data[i] = NULL;
			printk(KERN_ERR "Can't suspend/resume IOAPIC %d\n", i);
			continue;
		}
	}

	return 0;
}

device_initcall(ioapic_init_sysfs);


unsigned int create_irq_nr(unsigned int irq_want, int node)
{
	
	unsigned int irq;
	unsigned int new;
	unsigned long flags;
	struct irq_cfg *cfg_new = NULL;
	struct irq_desc *desc_new = NULL;

	irq = 0;
	if (irq_want < nr_irqs_gsi)
		irq_want = nr_irqs_gsi;

	spin_lock_irqsave(&vector_lock, flags);
	for (new = irq_want; new < nr_irqs; new++) {
		desc_new = irq_to_desc_alloc_node(new, node);
		if (!desc_new) {
			printk(KERN_INFO "can not get irq_desc for %d\n", new);
			continue;
		}
		cfg_new = desc_new->chip_data;

		if (cfg_new->vector != 0)
			continue;

		desc_new = move_irq_desc(desc_new, node);
		cfg_new = desc_new->chip_data;

		if (__assign_irq_vector(new, cfg_new, apic->target_cpus()) == 0)
			irq = new;
		break;
	}
	spin_unlock_irqrestore(&vector_lock, flags);

	if (irq > 0) {
		dynamic_irq_init(irq);
		
		if (desc_new)
			desc_new->chip_data = cfg_new;
	}
	return irq;
}

int create_irq(void)
{
	int node = cpu_to_node(boot_cpu_id);
	unsigned int irq_want;
	int irq;

	irq_want = nr_irqs_gsi;
	irq = create_irq_nr(irq_want, node);

	if (irq == 0)
		irq = -1;

	return irq;
}

void destroy_irq(unsigned int irq)
{
	unsigned long flags;
	struct irq_cfg *cfg;
	struct irq_desc *desc;

	
	desc = irq_to_desc(irq);
	cfg = desc->chip_data;
	dynamic_irq_cleanup(irq);
	
	desc->chip_data = cfg;

	free_irte(irq);
	spin_lock_irqsave(&vector_lock, flags);
	__clear_irq_vector(irq, cfg);
	spin_unlock_irqrestore(&vector_lock, flags);
}


#ifdef CONFIG_PCI_MSI
static int msi_compose_msg(struct pci_dev *pdev, unsigned int irq, struct msi_msg *msg)
{
	struct irq_cfg *cfg;
	int err;
	unsigned dest;

	if (disable_apic)
		return -ENXIO;

	cfg = irq_cfg(irq);
	err = assign_irq_vector(irq, cfg, apic->target_cpus());
	if (err)
		return err;

	dest = apic->cpu_mask_to_apicid_and(cfg->domain, apic->target_cpus());

	if (irq_remapped(irq)) {
		struct irte irte;
		int ir_index;
		u16 sub_handle;

		ir_index = map_irq_to_irte_handle(irq, &sub_handle);
		BUG_ON(ir_index == -1);

		memset (&irte, 0, sizeof(irte));

		irte.present = 1;
		irte.dst_mode = apic->irq_dest_mode;
		irte.trigger_mode = 0; 
		irte.dlvry_mode = apic->irq_delivery_mode;
		irte.vector = cfg->vector;
		irte.dest_id = IRTE_DEST(dest);

		
		set_msi_sid(&irte, pdev);

		modify_irte(irq, &irte);

		msg->address_hi = MSI_ADDR_BASE_HI;
		msg->data = sub_handle;
		msg->address_lo = MSI_ADDR_BASE_LO | MSI_ADDR_IR_EXT_INT |
				  MSI_ADDR_IR_SHV |
				  MSI_ADDR_IR_INDEX1(ir_index) |
				  MSI_ADDR_IR_INDEX2(ir_index);
	} else {
		if (x2apic_enabled())
			msg->address_hi = MSI_ADDR_BASE_HI |
					  MSI_ADDR_EXT_DEST_ID(dest);
		else
			msg->address_hi = MSI_ADDR_BASE_HI;

		msg->address_lo =
			MSI_ADDR_BASE_LO |
			((apic->irq_dest_mode == 0) ?
				MSI_ADDR_DEST_MODE_PHYSICAL:
				MSI_ADDR_DEST_MODE_LOGICAL) |
			((apic->irq_delivery_mode != dest_LowestPrio) ?
				MSI_ADDR_REDIRECTION_CPU:
				MSI_ADDR_REDIRECTION_LOWPRI) |
			MSI_ADDR_DEST_ID(dest);

		msg->data =
			MSI_DATA_TRIGGER_EDGE |
			MSI_DATA_LEVEL_ASSERT |
			((apic->irq_delivery_mode != dest_LowestPrio) ?
				MSI_DATA_DELIVERY_FIXED:
				MSI_DATA_DELIVERY_LOWPRI) |
			MSI_DATA_VECTOR(cfg->vector);
	}
	return err;
}

#ifdef CONFIG_SMP
static int set_msi_irq_affinity(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_cfg *cfg;
	struct msi_msg msg;
	unsigned int dest;

	dest = set_desc_affinity(desc, mask);
	if (dest == BAD_APICID)
		return -1;

	cfg = desc->chip_data;

	read_msi_msg_desc(desc, &msg);

	msg.data &= ~MSI_DATA_VECTOR_MASK;
	msg.data |= MSI_DATA_VECTOR(cfg->vector);
	msg.address_lo &= ~MSI_ADDR_DEST_ID_MASK;
	msg.address_lo |= MSI_ADDR_DEST_ID(dest);

	write_msi_msg_desc(desc, &msg);

	return 0;
}
#ifdef CONFIG_INTR_REMAP

static int
ir_set_msi_irq_affinity(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_cfg *cfg = desc->chip_data;
	unsigned int dest;
	struct irte irte;

	if (get_irte(irq, &irte))
		return -1;

	dest = set_desc_affinity(desc, mask);
	if (dest == BAD_APICID)
		return -1;

	irte.vector = cfg->vector;
	irte.dest_id = IRTE_DEST(dest);

	
	modify_irte(irq, &irte);

	
	if (cfg->move_in_progress)
		send_cleanup_vector(cfg);

	return 0;
}

#endif
#endif 


static struct irq_chip msi_chip = {
	.name		= "PCI-MSI",
	.unmask		= unmask_msi_irq,
	.mask		= mask_msi_irq,
	.ack		= ack_apic_edge,
#ifdef CONFIG_SMP
	.set_affinity	= set_msi_irq_affinity,
#endif
	.retrigger	= ioapic_retrigger_irq,
};

static struct irq_chip msi_ir_chip = {
	.name		= "IR-PCI-MSI",
	.unmask		= unmask_msi_irq,
	.mask		= mask_msi_irq,
#ifdef CONFIG_INTR_REMAP
	.ack		= ir_ack_apic_edge,
#ifdef CONFIG_SMP
	.set_affinity	= ir_set_msi_irq_affinity,
#endif
#endif
	.retrigger	= ioapic_retrigger_irq,
};


static int msi_alloc_irte(struct pci_dev *dev, int irq, int nvec)
{
	struct intel_iommu *iommu;
	int index;

	iommu = map_dev_to_ir(dev);
	if (!iommu) {
		printk(KERN_ERR
		       "Unable to map PCI %s to iommu\n", pci_name(dev));
		return -ENOENT;
	}

	index = alloc_irte(iommu, irq, nvec);
	if (index < 0) {
		printk(KERN_ERR
		       "Unable to allocate %d IRTE for PCI %s\n", nvec,
		       pci_name(dev));
		return -ENOSPC;
	}
	return index;
}

static int setup_msi_irq(struct pci_dev *dev, struct msi_desc *msidesc, int irq)
{
	int ret;
	struct msi_msg msg;

	ret = msi_compose_msg(dev, irq, &msg);
	if (ret < 0)
		return ret;

	set_irq_msi(irq, msidesc);
	write_msi_msg(irq, &msg);

	if (irq_remapped(irq)) {
		struct irq_desc *desc = irq_to_desc(irq);
		
		desc->status |= IRQ_MOVE_PCNTXT;
		set_irq_chip_and_handler_name(irq, &msi_ir_chip, handle_edge_irq, "edge");
	} else
		set_irq_chip_and_handler_name(irq, &msi_chip, handle_edge_irq, "edge");

	dev_printk(KERN_DEBUG, &dev->dev, "irq %d for MSI/MSI-X\n", irq);

	return 0;
}

int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
	unsigned int irq;
	int ret, sub_handle;
	struct msi_desc *msidesc;
	unsigned int irq_want;
	struct intel_iommu *iommu = NULL;
	int index = 0;
	int node;

	
	if (type == PCI_CAP_ID_MSI && nvec > 1)
		return 1;

	node = dev_to_node(&dev->dev);
	irq_want = nr_irqs_gsi;
	sub_handle = 0;
	list_for_each_entry(msidesc, &dev->msi_list, list) {
		irq = create_irq_nr(irq_want, node);
		if (irq == 0)
			return -1;
		irq_want = irq + 1;
		if (!intr_remapping_enabled)
			goto no_ir;

		if (!sub_handle) {
			
			index = msi_alloc_irte(dev, irq, nvec);
			if (index < 0) {
				ret = index;
				goto error;
			}
		} else {
			iommu = map_dev_to_ir(dev);
			if (!iommu) {
				ret = -ENOENT;
				goto error;
			}
			
			set_irte_irq(irq, iommu, index, sub_handle);
		}
no_ir:
		ret = setup_msi_irq(dev, msidesc, irq);
		if (ret < 0)
			goto error;
		sub_handle++;
	}
	return 0;

error:
	destroy_irq(irq);
	return ret;
}

void arch_teardown_msi_irq(unsigned int irq)
{
	destroy_irq(irq);
}

#if defined (CONFIG_DMAR) || defined (CONFIG_INTR_REMAP)
#ifdef CONFIG_SMP
static int dmar_msi_set_affinity(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_cfg *cfg;
	struct msi_msg msg;
	unsigned int dest;

	dest = set_desc_affinity(desc, mask);
	if (dest == BAD_APICID)
		return -1;

	cfg = desc->chip_data;

	dmar_msi_read(irq, &msg);

	msg.data &= ~MSI_DATA_VECTOR_MASK;
	msg.data |= MSI_DATA_VECTOR(cfg->vector);
	msg.address_lo &= ~MSI_ADDR_DEST_ID_MASK;
	msg.address_lo |= MSI_ADDR_DEST_ID(dest);

	dmar_msi_write(irq, &msg);

	return 0;
}

#endif 

static struct irq_chip dmar_msi_type = {
	.name = "DMAR_MSI",
	.unmask = dmar_msi_unmask,
	.mask = dmar_msi_mask,
	.ack = ack_apic_edge,
#ifdef CONFIG_SMP
	.set_affinity = dmar_msi_set_affinity,
#endif
	.retrigger = ioapic_retrigger_irq,
};

int arch_setup_dmar_msi(unsigned int irq)
{
	int ret;
	struct msi_msg msg;

	ret = msi_compose_msg(NULL, irq, &msg);
	if (ret < 0)
		return ret;
	dmar_msi_write(irq, &msg);
	set_irq_chip_and_handler_name(irq, &dmar_msi_type, handle_edge_irq,
		"edge");
	return 0;
}
#endif

#ifdef CONFIG_HPET_TIMER

#ifdef CONFIG_SMP
static int hpet_msi_set_affinity(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_cfg *cfg;
	struct msi_msg msg;
	unsigned int dest;

	dest = set_desc_affinity(desc, mask);
	if (dest == BAD_APICID)
		return -1;

	cfg = desc->chip_data;

	hpet_msi_read(irq, &msg);

	msg.data &= ~MSI_DATA_VECTOR_MASK;
	msg.data |= MSI_DATA_VECTOR(cfg->vector);
	msg.address_lo &= ~MSI_ADDR_DEST_ID_MASK;
	msg.address_lo |= MSI_ADDR_DEST_ID(dest);

	hpet_msi_write(irq, &msg);

	return 0;
}

#endif 

static struct irq_chip hpet_msi_type = {
	.name = "HPET_MSI",
	.unmask = hpet_msi_unmask,
	.mask = hpet_msi_mask,
	.ack = ack_apic_edge,
#ifdef CONFIG_SMP
	.set_affinity = hpet_msi_set_affinity,
#endif
	.retrigger = ioapic_retrigger_irq,
};

int arch_setup_hpet_msi(unsigned int irq)
{
	int ret;
	struct msi_msg msg;
	struct irq_desc *desc = irq_to_desc(irq);

	ret = msi_compose_msg(NULL, irq, &msg);
	if (ret < 0)
		return ret;

	hpet_msi_write(irq, &msg);
	desc->status |= IRQ_MOVE_PCNTXT;
	set_irq_chip_and_handler_name(irq, &hpet_msi_type, handle_edge_irq,
		"edge");

	return 0;
}
#endif

#endif 

#ifdef CONFIG_HT_IRQ

#ifdef CONFIG_SMP

static void target_ht_irq(unsigned int irq, unsigned int dest, u8 vector)
{
	struct ht_irq_msg msg;
	fetch_ht_irq_msg(irq, &msg);

	msg.address_lo &= ~(HT_IRQ_LOW_VECTOR_MASK | HT_IRQ_LOW_DEST_ID_MASK);
	msg.address_hi &= ~(HT_IRQ_HIGH_DEST_ID_MASK);

	msg.address_lo |= HT_IRQ_LOW_VECTOR(vector) | HT_IRQ_LOW_DEST_ID(dest);
	msg.address_hi |= HT_IRQ_HIGH_DEST_ID(dest);

	write_ht_irq_msg(irq, &msg);
}

static int set_ht_irq_affinity(unsigned int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_cfg *cfg;
	unsigned int dest;

	dest = set_desc_affinity(desc, mask);
	if (dest == BAD_APICID)
		return -1;

	cfg = desc->chip_data;

	target_ht_irq(irq, dest, cfg->vector);

	return 0;
}

#endif

static struct irq_chip ht_irq_chip = {
	.name		= "PCI-HT",
	.mask		= mask_ht_irq,
	.unmask		= unmask_ht_irq,
	.ack		= ack_apic_edge,
#ifdef CONFIG_SMP
	.set_affinity	= set_ht_irq_affinity,
#endif
	.retrigger	= ioapic_retrigger_irq,
};

int arch_setup_ht_irq(unsigned int irq, struct pci_dev *dev)
{
	struct irq_cfg *cfg;
	int err;

	if (disable_apic)
		return -ENXIO;

	cfg = irq_cfg(irq);
	err = assign_irq_vector(irq, cfg, apic->target_cpus());
	if (!err) {
		struct ht_irq_msg msg;
		unsigned dest;

		dest = apic->cpu_mask_to_apicid_and(cfg->domain,
						    apic->target_cpus());

		msg.address_hi = HT_IRQ_HIGH_DEST_ID(dest);

		msg.address_lo =
			HT_IRQ_LOW_BASE |
			HT_IRQ_LOW_DEST_ID(dest) |
			HT_IRQ_LOW_VECTOR(cfg->vector) |
			((apic->irq_dest_mode == 0) ?
				HT_IRQ_LOW_DM_PHYSICAL :
				HT_IRQ_LOW_DM_LOGICAL) |
			HT_IRQ_LOW_RQEOI_EDGE |
			((apic->irq_delivery_mode != dest_LowestPrio) ?
				HT_IRQ_LOW_MT_FIXED :
				HT_IRQ_LOW_MT_ARBITRATED) |
			HT_IRQ_LOW_IRQ_MASKED;

		write_ht_irq_msg(irq, &msg);

		set_irq_chip_and_handler_name(irq, &ht_irq_chip,
					      handle_edge_irq, "edge");

		dev_printk(KERN_DEBUG, &dev->dev, "irq %d for HT\n", irq);
	}
	return err;
}
#endif 

#ifdef CONFIG_X86_UV

int arch_enable_uv_irq(char *irq_name, unsigned int irq, int cpu, int mmr_blade,
		       unsigned long mmr_offset)
{
	const struct cpumask *eligible_cpu = cpumask_of(cpu);
	struct irq_cfg *cfg;
	int mmr_pnode;
	unsigned long mmr_value;
	struct uv_IO_APIC_route_entry *entry;
	unsigned long flags;
	int err;

	BUILD_BUG_ON(sizeof(struct uv_IO_APIC_route_entry) != sizeof(unsigned long));

	cfg = irq_cfg(irq);

	err = assign_irq_vector(irq, cfg, eligible_cpu);
	if (err != 0)
		return err;

	spin_lock_irqsave(&vector_lock, flags);
	set_irq_chip_and_handler_name(irq, &uv_irq_chip, handle_percpu_irq,
				      irq_name);
	spin_unlock_irqrestore(&vector_lock, flags);

	mmr_value = 0;
	entry = (struct uv_IO_APIC_route_entry *)&mmr_value;
	entry->vector		= cfg->vector;
	entry->delivery_mode	= apic->irq_delivery_mode;
	entry->dest_mode	= apic->irq_dest_mode;
	entry->polarity		= 0;
	entry->trigger		= 0;
	entry->mask		= 0;
	entry->dest		= apic->cpu_mask_to_apicid(eligible_cpu);

	mmr_pnode = uv_blade_to_pnode(mmr_blade);
	uv_write_global_mmr64(mmr_pnode, mmr_offset, mmr_value);

	if (cfg->move_in_progress)
		send_cleanup_vector(cfg);

	return irq;
}


void arch_disable_uv_irq(int mmr_blade, unsigned long mmr_offset)
{
	unsigned long mmr_value;
	struct uv_IO_APIC_route_entry *entry;
	int mmr_pnode;

	BUILD_BUG_ON(sizeof(struct uv_IO_APIC_route_entry) != sizeof(unsigned long));

	mmr_value = 0;
	entry = (struct uv_IO_APIC_route_entry *)&mmr_value;
	entry->mask = 1;

	mmr_pnode = uv_blade_to_pnode(mmr_blade);
	uv_write_global_mmr64(mmr_pnode, mmr_offset, mmr_value);
}
#endif 

int __init io_apic_get_redir_entries (int ioapic)
{
	union IO_APIC_reg_01	reg_01;
	unsigned long flags;

	spin_lock_irqsave(&ioapic_lock, flags);
	reg_01.raw = io_apic_read(ioapic, 1);
	spin_unlock_irqrestore(&ioapic_lock, flags);

	return reg_01.bits.entries;
}

void __init probe_nr_irqs_gsi(void)
{
	int nr = 0;

	nr = acpi_probe_gsi();
	if (nr > nr_irqs_gsi) {
		nr_irqs_gsi = nr;
	} else {
		
		int idx;

		nr = 0;
		for (idx = 0; idx < nr_ioapics; idx++)
			nr += io_apic_get_redir_entries(idx) + 1;

		if (nr > nr_irqs_gsi)
			nr_irqs_gsi = nr;
	}

	printk(KERN_DEBUG "nr_irqs_gsi: %d\n", nr_irqs_gsi);
}

#ifdef CONFIG_SPARSE_IRQ
int __init arch_probe_nr_irqs(void)
{
	int nr;

	if (nr_irqs > (NR_VECTORS * nr_cpu_ids))
		nr_irqs = NR_VECTORS * nr_cpu_ids;

	nr = nr_irqs_gsi + 8 * nr_cpu_ids;
#if defined(CONFIG_PCI_MSI) || defined(CONFIG_HT_IRQ)
	
	nr += nr_irqs_gsi * 16;
#endif
	if (nr < nr_irqs)
		nr_irqs = nr;

	return 0;
}
#endif

static int __io_apic_set_pci_routing(struct device *dev, int irq,
				struct io_apic_irq_attr *irq_attr)
{
	struct irq_desc *desc;
	struct irq_cfg *cfg;
	int node;
	int ioapic, pin;
	int trigger, polarity;

	ioapic = irq_attr->ioapic;
	if (!IO_APIC_IRQ(irq)) {
		apic_printk(APIC_QUIET,KERN_ERR "IOAPIC[%d]: Invalid reference to IRQ 0\n",
			ioapic);
		return -EINVAL;
	}

	if (dev)
		node = dev_to_node(dev);
	else
		node = cpu_to_node(boot_cpu_id);

	desc = irq_to_desc_alloc_node(irq, node);
	if (!desc) {
		printk(KERN_INFO "can not get irq_desc %d\n", irq);
		return 0;
	}

	pin = irq_attr->ioapic_pin;
	trigger = irq_attr->trigger;
	polarity = irq_attr->polarity;

	
	if (irq >= nr_legacy_irqs) {
		cfg = desc->chip_data;
		if (add_pin_to_irq_node_nopanic(cfg, node, ioapic, pin)) {
			printk(KERN_INFO "can not add pin %d for irq %d\n",
				pin, irq);
			return 0;
		}
	}

	setup_IO_APIC_irq(ioapic, pin, irq, desc, trigger, polarity);

	return 0;
}

int io_apic_set_pci_routing(struct device *dev, int irq,
				struct io_apic_irq_attr *irq_attr)
{
	int ioapic, pin;
	
	ioapic = irq_attr->ioapic;
	pin = irq_attr->ioapic_pin;
	if (test_bit(pin, mp_ioapic_routing[ioapic].pin_programmed)) {
		pr_debug("Pin %d-%d already programmed\n",
			 mp_ioapics[ioapic].apicid, pin);
		return 0;
	}
	set_bit(pin, mp_ioapic_routing[ioapic].pin_programmed);

	return __io_apic_set_pci_routing(dev, irq, irq_attr);
}

u8 __init io_apic_unique_id(u8 id)
{
#ifdef CONFIG_X86_32
	if ((boot_cpu_data.x86_vendor == X86_VENDOR_INTEL) &&
	    !APIC_XAPIC(apic_version[boot_cpu_physical_apicid]))
		return io_apic_get_unique_id(nr_ioapics, id);
	else
		return id;
#else
	int i;
	DECLARE_BITMAP(used, 256);

	bitmap_zero(used, 256);
	for (i = 0; i < nr_ioapics; i++) {
		struct mpc_ioapic *ia = &mp_ioapics[i];
		__set_bit(ia->apicid, used);
	}
	if (!test_bit(id, used))
		return id;
	return find_first_zero_bit(used, 256);
#endif
}

#ifdef CONFIG_X86_32
int __init io_apic_get_unique_id(int ioapic, int apic_id)
{
	union IO_APIC_reg_00 reg_00;
	static physid_mask_t apic_id_map = PHYSID_MASK_NONE;
	physid_mask_t tmp;
	unsigned long flags;
	int i = 0;

	

	if (physids_empty(apic_id_map))
		apic_id_map = apic->ioapic_phys_id_map(phys_cpu_present_map);

	spin_lock_irqsave(&ioapic_lock, flags);
	reg_00.raw = io_apic_read(ioapic, 0);
	spin_unlock_irqrestore(&ioapic_lock, flags);

	if (apic_id >= get_physical_broadcast()) {
		printk(KERN_WARNING "IOAPIC[%d]: Invalid apic_id %d, trying "
			"%d\n", ioapic, apic_id, reg_00.bits.ID);
		apic_id = reg_00.bits.ID;
	}

	
	if (apic->check_apicid_used(apic_id_map, apic_id)) {

		for (i = 0; i < get_physical_broadcast(); i++) {
			if (!apic->check_apicid_used(apic_id_map, i))
				break;
		}

		if (i == get_physical_broadcast())
			panic("Max apic_id exceeded!\n");

		printk(KERN_WARNING "IOAPIC[%d]: apic_id %d already used, "
			"trying %d\n", ioapic, apic_id, i);

		apic_id = i;
	}

	tmp = apic->apicid_to_cpu_present(apic_id);
	physids_or(apic_id_map, apic_id_map, tmp);

	if (reg_00.bits.ID != apic_id) {
		reg_00.bits.ID = apic_id;

		spin_lock_irqsave(&ioapic_lock, flags);
		io_apic_write(ioapic, 0, reg_00.raw);
		reg_00.raw = io_apic_read(ioapic, 0);
		spin_unlock_irqrestore(&ioapic_lock, flags);

		
		if (reg_00.bits.ID != apic_id) {
			printk("IOAPIC[%d]: Unable to change apic_id!\n", ioapic);
			return -1;
		}
	}

	apic_printk(APIC_VERBOSE, KERN_INFO
			"IOAPIC[%d]: Assigned apic_id %d\n", ioapic, apic_id);

	return apic_id;
}
#endif

int __init io_apic_get_version(int ioapic)
{
	union IO_APIC_reg_01	reg_01;
	unsigned long flags;

	spin_lock_irqsave(&ioapic_lock, flags);
	reg_01.raw = io_apic_read(ioapic, 1);
	spin_unlock_irqrestore(&ioapic_lock, flags);

	return reg_01.bits.version;
}

int acpi_get_override_irq(int bus_irq, int *trigger, int *polarity)
{
	int i;

	if (skip_ioapic_setup)
		return -1;

	for (i = 0; i < mp_irq_entries; i++)
		if (mp_irqs[i].irqtype == mp_INT &&
		    mp_irqs[i].srcbusirq == bus_irq)
			break;
	if (i >= mp_irq_entries)
		return -1;

	*trigger = irq_trigger(i);
	*polarity = irq_polarity(i);
	return 0;
}


#ifdef CONFIG_SMP
void __init setup_ioapic_dest(void)
{
	int pin, ioapic = 0, irq, irq_entry;
	struct irq_desc *desc;
	const struct cpumask *mask;

	if (skip_ioapic_setup == 1)
		return;

#ifdef CONFIG_ACPI
	if (!acpi_disabled && acpi_ioapic) {
		ioapic = mp_find_ioapic(0);
		if (ioapic < 0)
			ioapic = 0;
	}
#endif

	for (pin = 0; pin < nr_ioapic_registers[ioapic]; pin++) {
		irq_entry = find_irq_entry(ioapic, pin, mp_INT);
		if (irq_entry == -1)
			continue;
		irq = pin_2_irq(irq_entry, ioapic, pin);

		desc = irq_to_desc(irq);

		
		if (desc->status &
		    (IRQ_NO_BALANCING | IRQ_AFFINITY_SET))
			mask = desc->affinity;
		else
			mask = apic->target_cpus();

		if (intr_remapping_enabled)
			set_ir_ioapic_affinity_irq_desc(desc, mask);
		else
			set_ioapic_affinity_irq_desc(desc, mask);
	}

}
#endif

#define IOAPIC_RESOURCE_NAME_SIZE 11

static struct resource *ioapic_resources;

static struct resource * __init ioapic_setup_resources(int nr_ioapics)
{
	unsigned long n;
	struct resource *res;
	char *mem;
	int i;

	if (nr_ioapics <= 0)
		return NULL;

	n = IOAPIC_RESOURCE_NAME_SIZE + sizeof(struct resource);
	n *= nr_ioapics;

	mem = alloc_bootmem(n);
	res = (void *)mem;

	mem += sizeof(struct resource) * nr_ioapics;

	for (i = 0; i < nr_ioapics; i++) {
		res[i].name = mem;
		res[i].flags = IORESOURCE_MEM | IORESOURCE_BUSY;
		sprintf(mem,  "IOAPIC %u", i);
		mem += IOAPIC_RESOURCE_NAME_SIZE;
	}

	ioapic_resources = res;

	return res;
}

void __init ioapic_init_mappings(void)
{
	unsigned long ioapic_phys, idx = FIX_IO_APIC_BASE_0;
	struct resource *ioapic_res;
	int i;

	ioapic_res = ioapic_setup_resources(nr_ioapics);
	for (i = 0; i < nr_ioapics; i++) {
		if (smp_found_config) {
			ioapic_phys = mp_ioapics[i].apicaddr;
#ifdef CONFIG_X86_32
			if (!ioapic_phys) {
				printk(KERN_ERR
				       "WARNING: bogus zero IO-APIC "
				       "address found in MPTABLE, "
				       "disabling IO/APIC support!\n");
				smp_found_config = 0;
				skip_ioapic_setup = 1;
				goto fake_ioapic_page;
			}
#endif
		} else {
#ifdef CONFIG_X86_32
fake_ioapic_page:
#endif
			ioapic_phys = (unsigned long)
				alloc_bootmem_pages(PAGE_SIZE);
			ioapic_phys = __pa(ioapic_phys);
		}
		set_fixmap_nocache(idx, ioapic_phys);
		apic_printk(APIC_VERBOSE,
			    "mapped IOAPIC to %08lx (%08lx)\n",
			    __fix_to_virt(idx), ioapic_phys);
		idx++;

		ioapic_res->start = ioapic_phys;
		ioapic_res->end = ioapic_phys + (4 * 1024) - 1;
		ioapic_res++;
	}
}

void __init ioapic_insert_resources(void)
{
	int i;
	struct resource *r = ioapic_resources;

	if (!r) {
		if (nr_ioapics > 0)
			printk(KERN_ERR
				"IO APIC resources couldn't be allocated.\n");
		return;
	}

	for (i = 0; i < nr_ioapics; i++) {
		insert_resource(&iomem_resource, r);
		r++;
	}
}

int mp_find_ioapic(int gsi)
{
	int i = 0;

	
	for (i = 0; i < nr_ioapics; i++) {
		if ((gsi >= mp_gsi_routing[i].gsi_base)
		    && (gsi <= mp_gsi_routing[i].gsi_end))
			return i;
	}

	printk(KERN_ERR "ERROR: Unable to locate IOAPIC for GSI %d\n", gsi);
	return -1;
}

int mp_find_ioapic_pin(int ioapic, int gsi)
{
	if (WARN_ON(ioapic == -1))
		return -1;
	if (WARN_ON(gsi > mp_gsi_routing[ioapic].gsi_end))
		return -1;

	return gsi - mp_gsi_routing[ioapic].gsi_base;
}

static int bad_ioapic(unsigned long address)
{
	if (nr_ioapics >= MAX_IO_APICS) {
		printk(KERN_WARNING "WARING: Max # of I/O APICs (%d) exceeded "
		       "(found %d), skipping\n", MAX_IO_APICS, nr_ioapics);
		return 1;
	}
	if (!address) {
		printk(KERN_WARNING "WARNING: Bogus (zero) I/O APIC address"
		       " found in table, skipping!\n");
		return 1;
	}
	return 0;
}

void __init mp_register_ioapic(int id, u32 address, u32 gsi_base)
{
	int idx = 0;

	if (bad_ioapic(address))
		return;

	idx = nr_ioapics;

	mp_ioapics[idx].type = MP_IOAPIC;
	mp_ioapics[idx].flags = MPC_APIC_USABLE;
	mp_ioapics[idx].apicaddr = address;

	set_fixmap_nocache(FIX_IO_APIC_BASE_0 + idx, address);
	mp_ioapics[idx].apicid = io_apic_unique_id(id);
	mp_ioapics[idx].apicver = io_apic_get_version(idx);

	
	mp_gsi_routing[idx].gsi_base = gsi_base;
	mp_gsi_routing[idx].gsi_end = gsi_base +
	    io_apic_get_redir_entries(idx);

	printk(KERN_INFO "IOAPIC[%d]: apic_id %d, version %d, address 0x%x, "
	       "GSI %d-%d\n", idx, mp_ioapics[idx].apicid,
	       mp_ioapics[idx].apicver, mp_ioapics[idx].apicaddr,
	       mp_gsi_routing[idx].gsi_base, mp_gsi_routing[idx].gsi_end);

	nr_ioapics++;
}
