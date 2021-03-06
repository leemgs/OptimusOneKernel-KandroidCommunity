

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/dmi.h>
#include <linux/io.h>
#include <linux/smp.h>
#include <asm/io_apic.h>
#include <linux/irq.h>
#include <linux/acpi.h>
#include <asm/pci_x86.h>

#define PIRQ_SIGNATURE	(('$' << 0) + ('P' << 8) + ('I' << 16) + ('R' << 24))
#define PIRQ_VERSION 0x0100

static int broken_hp_bios_irq9;
static int acer_tm360_irqrouting;

static struct irq_routing_table *pirq_table;

static int pirq_enable_irq(struct pci_dev *dev);


unsigned int pcibios_irq_mask = 0xfff8;

static int pirq_penalty[16] = {
	1000000, 1000000, 1000000, 1000, 1000, 0, 1000, 1000,
	0, 0, 0, 0, 1000, 100000, 100000, 100000
};

struct irq_router {
	char *name;
	u16 vendor, device;
	int (*get)(struct pci_dev *router, struct pci_dev *dev, int pirq);
	int (*set)(struct pci_dev *router, struct pci_dev *dev, int pirq,
		int new);
};

struct irq_router_handler {
	u16 vendor;
	int (*probe)(struct irq_router *r, struct pci_dev *router, u16 device);
};

int (*pcibios_enable_irq)(struct pci_dev *dev) = NULL;
void (*pcibios_disable_irq)(struct pci_dev *dev) = NULL;



static inline struct irq_routing_table *pirq_check_routing_table(u8 *addr)
{
	struct irq_routing_table *rt;
	int i;
	u8 sum;

	rt = (struct irq_routing_table *) addr;
	if (rt->signature != PIRQ_SIGNATURE ||
	    rt->version != PIRQ_VERSION ||
	    rt->size % 16 ||
	    rt->size < sizeof(struct irq_routing_table))
		return NULL;
	sum = 0;
	for (i = 0; i < rt->size; i++)
		sum += addr[i];
	if (!sum) {
		DBG(KERN_DEBUG "PCI: Interrupt Routing Table found at 0x%p\n",
			rt);
		return rt;
	}
	return NULL;
}





static struct irq_routing_table * __init pirq_find_routing_table(void)
{
	u8 *addr;
	struct irq_routing_table *rt;

	if (pirq_table_addr) {
		rt = pirq_check_routing_table((u8 *) __va(pirq_table_addr));
		if (rt)
			return rt;
		printk(KERN_WARNING "PCI: PIRQ table NOT found at pirqaddr\n");
	}
	for (addr = (u8 *) __va(0xf0000); addr < (u8 *) __va(0x100000); addr += 16) {
		rt = pirq_check_routing_table(addr);
		if (rt)
			return rt;
	}
	return NULL;
}



static void __init pirq_peer_trick(void)
{
	struct irq_routing_table *rt = pirq_table;
	u8 busmap[256];
	int i;
	struct irq_info *e;

	memset(busmap, 0, sizeof(busmap));
	for (i = 0; i < (rt->size - sizeof(struct irq_routing_table)) / sizeof(struct irq_info); i++) {
		e = &rt->slots[i];
#ifdef DEBUG
		{
			int j;
			DBG(KERN_DEBUG "%02x:%02x slot=%02x", e->bus, e->devfn/8, e->slot);
			for (j = 0; j < 4; j++)
				DBG(" %d:%02x/%04x", j, e->irq[j].link, e->irq[j].bitmap);
			DBG("\n");
		}
#endif
		busmap[e->bus] = 1;
	}
	for (i = 1; i < 256; i++) {
		int node;
		if (!busmap[i] || pci_find_bus(0, i))
			continue;
		node = get_mp_bus_to_node(i);
		if (pci_scan_bus_on_node(i, &pci_root_ops, node))
			printk(KERN_INFO "PCI: Discovered primary peer "
			       "bus %02x [IRQ]\n", i);
	}
	pcibios_last_bus = -1;
}



void eisa_set_level_irq(unsigned int irq)
{
	unsigned char mask = 1 << (irq & 7);
	unsigned int port = 0x4d0 + (irq >> 3);
	unsigned char val;
	static u16 eisa_irq_mask;

	if (irq >= 16 || (1 << irq) & eisa_irq_mask)
		return;

	eisa_irq_mask |= (1 << irq);
	printk(KERN_DEBUG "PCI: setting IRQ %u as level-triggered\n", irq);
	val = inb(port);
	if (!(val & mask)) {
		DBG(KERN_DEBUG " -> edge");
		outb(val | mask, port);
	}
}


static unsigned int read_config_nybble(struct pci_dev *router, unsigned offset, unsigned nr)
{
	u8 x;
	unsigned reg = offset + (nr >> 1);

	pci_read_config_byte(router, reg, &x);
	return (nr & 1) ? (x >> 4) : (x & 0xf);
}

static void write_config_nybble(struct pci_dev *router, unsigned offset,
	unsigned nr, unsigned int val)
{
	u8 x;
	unsigned reg = offset + (nr >> 1);

	pci_read_config_byte(router, reg, &x);
	x = (nr & 1) ? ((x & 0x0f) | (val << 4)) : ((x & 0xf0) | val);
	pci_write_config_byte(router, reg, x);
}


static int pirq_ali_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	static const unsigned char irqmap[16] = { 0, 9, 3, 10, 4, 5, 7, 6, 1, 11, 0, 12, 0, 14, 0, 15 };

	WARN_ON_ONCE(pirq > 16);
	return irqmap[read_config_nybble(router, 0x48, pirq-1)];
}

static int pirq_ali_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	static const unsigned char irqmap[16] = { 0, 8, 0, 2, 4, 5, 7, 6, 0, 1, 3, 9, 11, 0, 13, 15 };
	unsigned int val = irqmap[irq];

	WARN_ON_ONCE(pirq > 16);
	if (val) {
		write_config_nybble(router, 0x48, pirq-1, val);
		return 1;
	}
	return 0;
}


static int pirq_piix_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	u8 x;

	pci_read_config_byte(router, pirq, &x);
	return (x < 16) ? x : 0;
}

static int pirq_piix_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	pci_write_config_byte(router, pirq, irq);
	return 1;
}


static int pirq_via_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	return read_config_nybble(router, 0x55, pirq == 4 ? 5 : pirq);
}

static int pirq_via_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	write_config_nybble(router, 0x55, pirq == 4 ? 5 : pirq, irq);
	return 1;
}


static int pirq_via586_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	static const unsigned int pirqmap[5] = { 3, 2, 5, 1, 1 };

	WARN_ON_ONCE(pirq > 5);
	return read_config_nybble(router, 0x55, pirqmap[pirq-1]);
}

static int pirq_via586_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	static const unsigned int pirqmap[5] = { 3, 2, 5, 1, 1 };

	WARN_ON_ONCE(pirq > 5);
	write_config_nybble(router, 0x55, pirqmap[pirq-1], irq);
	return 1;
}


static int pirq_ite_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	static const unsigned char pirqmap[4] = { 1, 0, 2, 3 };

	WARN_ON_ONCE(pirq > 4);
	return read_config_nybble(router, 0x43, pirqmap[pirq-1]);
}

static int pirq_ite_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	static const unsigned char pirqmap[4] = { 1, 0, 2, 3 };

	WARN_ON_ONCE(pirq > 4);
	write_config_nybble(router, 0x43, pirqmap[pirq-1], irq);
	return 1;
}


static int pirq_opti_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	return read_config_nybble(router, 0xb8, pirq >> 4);
}

static int pirq_opti_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	write_config_nybble(router, 0xb8, pirq >> 4, irq);
	return 1;
}


static int pirq_cyrix_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	return read_config_nybble(router, 0x5C, (pirq-1)^1);
}

static int pirq_cyrix_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	write_config_nybble(router, 0x5C, (pirq-1)^1, irq);
	return 1;
}



#define PIRQ_SIS_IRQ_MASK	0x0f
#define PIRQ_SIS_IRQ_DISABLE	0x80
#define PIRQ_SIS_USB_ENABLE	0x40

static int pirq_sis_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	u8 x;
	int reg;

	reg = pirq;
	if (reg >= 0x01 && reg <= 0x04)
		reg += 0x40;
	pci_read_config_byte(router, reg, &x);
	return (x & PIRQ_SIS_IRQ_DISABLE) ? 0 : (x & PIRQ_SIS_IRQ_MASK);
}

static int pirq_sis_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	u8 x;
	int reg;

	reg = pirq;
	if (reg >= 0x01 && reg <= 0x04)
		reg += 0x40;
	pci_read_config_byte(router, reg, &x);
	x &= ~(PIRQ_SIS_IRQ_MASK | PIRQ_SIS_IRQ_DISABLE);
	x |= irq ? irq: PIRQ_SIS_IRQ_DISABLE;
	pci_write_config_byte(router, reg, x);
	return 1;
}




static int pirq_vlsi_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	WARN_ON_ONCE(pirq >= 9);
	if (pirq > 8) {
		dev_info(&dev->dev, "VLSI router PIRQ escape (%d)\n", pirq);
		return 0;
	}
	return read_config_nybble(router, 0x74, pirq-1);
}

static int pirq_vlsi_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	WARN_ON_ONCE(pirq >= 9);
	if (pirq > 8) {
		dev_info(&dev->dev, "VLSI router PIRQ escape (%d)\n", pirq);
		return 0;
	}
	write_config_nybble(router, 0x74, pirq-1, irq);
	return 1;
}


static int pirq_serverworks_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	outb(pirq, 0xc00);
	return inb(0xc01) & 0xf;
}

static int pirq_serverworks_set(struct pci_dev *router, struct pci_dev *dev,
	int pirq, int irq)
{
	outb(pirq, 0xc00);
	outb(irq, 0xc01);
	return 1;
}


static int pirq_amd756_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	u8 irq;
	irq = 0;
	if (pirq <= 4)
		irq = read_config_nybble(router, 0x56, pirq - 1);
	dev_info(&dev->dev,
		 "AMD756: dev [%04x:%04x], router PIRQ %d get IRQ %d\n",
		 dev->vendor, dev->device, pirq, irq);
	return irq;
}

static int pirq_amd756_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	dev_info(&dev->dev,
		 "AMD756: dev [%04x:%04x], router PIRQ %d set IRQ %d\n",
		 dev->vendor, dev->device, pirq, irq);
	if (pirq <= 4)
		write_config_nybble(router, 0x56, pirq - 1, irq);
	return 1;
}


static int pirq_pico_get(struct pci_dev *router, struct pci_dev *dev, int pirq)
{
	outb(0x10 + ((pirq - 1) >> 1), 0x24);
	return ((pirq - 1) & 1) ? (inb(0x26) >> 4) : (inb(0x26) & 0xf);
}

static int pirq_pico_set(struct pci_dev *router, struct pci_dev *dev, int pirq,
			int irq)
{
	unsigned int x;
	outb(0x10 + ((pirq - 1) >> 1), 0x24);
	x = inb(0x26);
	x = ((pirq - 1) & 1) ? ((x & 0x0f) | (irq << 4)) : ((x & 0xf0) | (irq));
	outb(x, 0x26);
	return 1;
}

#ifdef CONFIG_PCI_BIOS

static int pirq_bios_set(struct pci_dev *router, struct pci_dev *dev, int pirq, int irq)
{
	struct pci_dev *bridge;
	int pin = pci_get_interrupt_pin(dev, &bridge);
	return pcibios_set_irq_routing(bridge, pin - 1, irq);
}

#endif

static __init int intel_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	static struct pci_device_id __initdata pirq_440gx[] = {
		{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82443GX_0) },
		{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82443GX_2) },
		{ },
	};

	
	if (pci_dev_present(pirq_440gx))
		return 0;

	switch (device) {
	case PCI_DEVICE_ID_INTEL_82371FB_0:
	case PCI_DEVICE_ID_INTEL_82371SB_0:
	case PCI_DEVICE_ID_INTEL_82371AB_0:
	case PCI_DEVICE_ID_INTEL_82371MX:
	case PCI_DEVICE_ID_INTEL_82443MX_0:
	case PCI_DEVICE_ID_INTEL_82801AA_0:
	case PCI_DEVICE_ID_INTEL_82801AB_0:
	case PCI_DEVICE_ID_INTEL_82801BA_0:
	case PCI_DEVICE_ID_INTEL_82801BA_10:
	case PCI_DEVICE_ID_INTEL_82801CA_0:
	case PCI_DEVICE_ID_INTEL_82801CA_12:
	case PCI_DEVICE_ID_INTEL_82801DB_0:
	case PCI_DEVICE_ID_INTEL_82801E_0:
	case PCI_DEVICE_ID_INTEL_82801EB_0:
	case PCI_DEVICE_ID_INTEL_ESB_1:
	case PCI_DEVICE_ID_INTEL_ICH6_0:
	case PCI_DEVICE_ID_INTEL_ICH6_1:
	case PCI_DEVICE_ID_INTEL_ICH7_0:
	case PCI_DEVICE_ID_INTEL_ICH7_1:
	case PCI_DEVICE_ID_INTEL_ICH7_30:
	case PCI_DEVICE_ID_INTEL_ICH7_31:
	case PCI_DEVICE_ID_INTEL_TGP_LPC:
	case PCI_DEVICE_ID_INTEL_ESB2_0:
	case PCI_DEVICE_ID_INTEL_ICH8_0:
	case PCI_DEVICE_ID_INTEL_ICH8_1:
	case PCI_DEVICE_ID_INTEL_ICH8_2:
	case PCI_DEVICE_ID_INTEL_ICH8_3:
	case PCI_DEVICE_ID_INTEL_ICH8_4:
	case PCI_DEVICE_ID_INTEL_ICH9_0:
	case PCI_DEVICE_ID_INTEL_ICH9_1:
	case PCI_DEVICE_ID_INTEL_ICH9_2:
	case PCI_DEVICE_ID_INTEL_ICH9_3:
	case PCI_DEVICE_ID_INTEL_ICH9_4:
	case PCI_DEVICE_ID_INTEL_ICH9_5:
	case PCI_DEVICE_ID_INTEL_TOLAPAI_0:
	case PCI_DEVICE_ID_INTEL_ICH10_0:
	case PCI_DEVICE_ID_INTEL_ICH10_1:
	case PCI_DEVICE_ID_INTEL_ICH10_2:
	case PCI_DEVICE_ID_INTEL_ICH10_3:
		r->name = "PIIX/ICH";
		r->get = pirq_piix_get;
		r->set = pirq_piix_set;
		return 1;
	}

	if ((device >= PCI_DEVICE_ID_INTEL_PCH_LPC_MIN) && 
		(device <= PCI_DEVICE_ID_INTEL_PCH_LPC_MAX)) {
		r->name = "PIIX/ICH";
		r->get = pirq_piix_get;
		r->set = pirq_piix_set;
		return 1;
	}

	return 0;
}

static __init int via_router_probe(struct irq_router *r,
				struct pci_dev *router, u16 device)
{
	

	
	if (device == PCI_DEVICE_ID_VIA_82C586_0) {
		switch (router->device) {
		case PCI_DEVICE_ID_VIA_82C686:
			
			device = PCI_DEVICE_ID_VIA_82C686;
			break;
		case PCI_DEVICE_ID_VIA_8235:
			
			device = PCI_DEVICE_ID_VIA_8235;
			break;
		case PCI_DEVICE_ID_VIA_8237:
			
			device = PCI_DEVICE_ID_VIA_8237;
			break;
		}
	}

	switch (device) {
	case PCI_DEVICE_ID_VIA_82C586_0:
		r->name = "VIA";
		r->get = pirq_via586_get;
		r->set = pirq_via586_set;
		return 1;
	case PCI_DEVICE_ID_VIA_82C596:
	case PCI_DEVICE_ID_VIA_82C686:
	case PCI_DEVICE_ID_VIA_8231:
	case PCI_DEVICE_ID_VIA_8233A:
	case PCI_DEVICE_ID_VIA_8235:
	case PCI_DEVICE_ID_VIA_8237:
		
		r->name = "VIA";
		r->get = pirq_via_get;
		r->set = pirq_via_set;
		return 1;
	}
	return 0;
}

static __init int vlsi_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_VLSI_82C534:
		r->name = "VLSI 82C534";
		r->get = pirq_vlsi_get;
		r->set = pirq_vlsi_set;
		return 1;
	}
	return 0;
}


static __init int serverworks_router_probe(struct irq_router *r,
		struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_SERVERWORKS_OSB4:
	case PCI_DEVICE_ID_SERVERWORKS_CSB5:
		r->name = "ServerWorks";
		r->get = pirq_serverworks_get;
		r->set = pirq_serverworks_set;
		return 1;
	}
	return 0;
}

static __init int sis_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	if (device != PCI_DEVICE_ID_SI_503)
		return 0;

	r->name = "SIS";
	r->get = pirq_sis_get;
	r->set = pirq_sis_set;
	return 1;
}

static __init int cyrix_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_CYRIX_5520:
		r->name = "NatSemi";
		r->get = pirq_cyrix_get;
		r->set = pirq_cyrix_set;
		return 1;
	}
	return 0;
}

static __init int opti_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_OPTI_82C700:
		r->name = "OPTI";
		r->get = pirq_opti_get;
		r->set = pirq_opti_set;
		return 1;
	}
	return 0;
}

static __init int ite_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_ITE_IT8330G_0:
		r->name = "ITE";
		r->get = pirq_ite_get;
		r->set = pirq_ite_set;
		return 1;
	}
	return 0;
}

static __init int ali_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_AL_M1533:
	case PCI_DEVICE_ID_AL_M1563:
		r->name = "ALI";
		r->get = pirq_ali_get;
		r->set = pirq_ali_set;
		return 1;
	}
	return 0;
}

static __init int amd_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_AMD_VIPER_740B:
		r->name = "AMD756";
		break;
	case PCI_DEVICE_ID_AMD_VIPER_7413:
		r->name = "AMD766";
		break;
	case PCI_DEVICE_ID_AMD_VIPER_7443:
		r->name = "AMD768";
		break;
	default:
		return 0;
	}
	r->get = pirq_amd756_get;
	r->set = pirq_amd756_set;
	return 1;
}

static __init int pico_router_probe(struct irq_router *r, struct pci_dev *router, u16 device)
{
	switch (device) {
	case PCI_DEVICE_ID_PICOPOWER_PT86C523:
		r->name = "PicoPower PT86C523";
		r->get = pirq_pico_get;
		r->set = pirq_pico_set;
		return 1;

	case PCI_DEVICE_ID_PICOPOWER_PT86C523BBP:
		r->name = "PicoPower PT86C523 rev. BB+";
		r->get = pirq_pico_get;
		r->set = pirq_pico_set;
		return 1;
	}
	return 0;
}

static __initdata struct irq_router_handler pirq_routers[] = {
	{ PCI_VENDOR_ID_INTEL, intel_router_probe },
	{ PCI_VENDOR_ID_AL, ali_router_probe },
	{ PCI_VENDOR_ID_ITE, ite_router_probe },
	{ PCI_VENDOR_ID_VIA, via_router_probe },
	{ PCI_VENDOR_ID_OPTI, opti_router_probe },
	{ PCI_VENDOR_ID_SI, sis_router_probe },
	{ PCI_VENDOR_ID_CYRIX, cyrix_router_probe },
	{ PCI_VENDOR_ID_VLSI, vlsi_router_probe },
	{ PCI_VENDOR_ID_SERVERWORKS, serverworks_router_probe },
	{ PCI_VENDOR_ID_AMD, amd_router_probe },
	{ PCI_VENDOR_ID_PICOPOWER, pico_router_probe },
	
	{ 0, NULL }
};
static struct irq_router pirq_router;
static struct pci_dev *pirq_router_dev;




static void __init pirq_find_router(struct irq_router *r)
{
	struct irq_routing_table *rt = pirq_table;
	struct irq_router_handler *h;

#ifdef CONFIG_PCI_BIOS
	if (!rt->signature) {
		printk(KERN_INFO "PCI: Using BIOS for IRQ routing\n");
		r->set = pirq_bios_set;
		r->name = "BIOS";
		return;
	}
#endif

	
	r->name = "default";
	r->get = NULL;
	r->set = NULL;

	DBG(KERN_DEBUG "PCI: Attempting to find IRQ router for [%04x:%04x]\n",
	    rt->rtr_vendor, rt->rtr_device);

	pirq_router_dev = pci_get_bus_and_slot(rt->rtr_bus, rt->rtr_devfn);
	if (!pirq_router_dev) {
		DBG(KERN_DEBUG "PCI: Interrupt router not found at "
			"%02x:%02x\n", rt->rtr_bus, rt->rtr_devfn);
		return;
	}

	for (h = pirq_routers; h->vendor; h++) {
		
		if (rt->rtr_vendor == h->vendor &&
			h->probe(r, pirq_router_dev, rt->rtr_device))
			break;
		
		if (pirq_router_dev->vendor == h->vendor &&
			h->probe(r, pirq_router_dev, pirq_router_dev->device))
			break;
	}
	dev_info(&pirq_router_dev->dev, "%s IRQ router [%04x:%04x]\n",
		 pirq_router.name,
		 pirq_router_dev->vendor, pirq_router_dev->device);

	
}

static struct irq_info *pirq_get_info(struct pci_dev *dev)
{
	struct irq_routing_table *rt = pirq_table;
	int entries = (rt->size - sizeof(struct irq_routing_table)) /
		sizeof(struct irq_info);
	struct irq_info *info;

	for (info = rt->slots; entries--; info++)
		if (info->bus == dev->bus->number &&
			PCI_SLOT(info->devfn) == PCI_SLOT(dev->devfn))
			return info;
	return NULL;
}

static int pcibios_lookup_irq(struct pci_dev *dev, int assign)
{
	u8 pin;
	struct irq_info *info;
	int i, pirq, newirq;
	int irq = 0;
	u32 mask;
	struct irq_router *r = &pirq_router;
	struct pci_dev *dev2 = NULL;
	char *msg = NULL;

	
	pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
	if (!pin) {
		dev_dbg(&dev->dev, "no interrupt pin\n");
		return 0;
	}

	if (io_apic_assign_pci_irqs)
		return 0;

	

	if (!pirq_table)
		return 0;

	info = pirq_get_info(dev);
	if (!info) {
		dev_dbg(&dev->dev, "PCI INT %c not found in routing table\n",
			'A' + pin - 1);
		return 0;
	}
	pirq = info->irq[pin - 1].link;
	mask = info->irq[pin - 1].bitmap;
	if (!pirq) {
		dev_dbg(&dev->dev, "PCI INT %c not routed\n", 'A' + pin - 1);
		return 0;
	}
	dev_dbg(&dev->dev, "PCI INT %c -> PIRQ %02x, mask %04x, excl %04x",
		'A' + pin - 1, pirq, mask, pirq_table->exclusive_irqs);
	mask &= pcibios_irq_mask;

	

	if (broken_hp_bios_irq9 && pirq == 0x59 && dev->irq == 9) {
		dev->irq = 11;
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, 11);
		r->set(pirq_router_dev, dev, pirq, 11);
	}

	
	if (acer_tm360_irqrouting && dev->irq == 11 &&
		dev->vendor == PCI_VENDOR_ID_O2) {
		pirq = 0x68;
		mask = 0x400;
		dev->irq = r->get(pirq_router_dev, dev, pirq);
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
	}

	
	newirq = dev->irq;
	if (newirq && !((1 << newirq) & mask)) {
		if (pci_probe & PCI_USE_PIRQ_MASK)
			newirq = 0;
		else
			dev_warn(&dev->dev, "IRQ %d doesn't match PIRQ mask "
				 "%#x; try pci=usepirqmask\n", newirq, mask);
	}
	if (!newirq && assign) {
		for (i = 0; i < 16; i++) {
			if (!(mask & (1 << i)))
				continue;
			if (pirq_penalty[i] < pirq_penalty[newirq] &&
				can_request_irq(i, IRQF_SHARED))
				newirq = i;
		}
	}
	dev_dbg(&dev->dev, "PCI INT %c -> newirq %d", 'A' + pin - 1, newirq);

	
	if ((pirq & 0xf0) == 0xf0) {
		irq = pirq & 0xf;
		msg = "hardcoded";
	} else if (r->get && (irq = r->get(pirq_router_dev, dev, pirq)) && \
	((!(pci_probe & PCI_USE_PIRQ_MASK)) || ((1 << irq) & mask))) {
		msg = "found";
		eisa_set_level_irq(irq);
	} else if (newirq && r->set &&
		(dev->class >> 8) != PCI_CLASS_DISPLAY_VGA) {
		if (r->set(pirq_router_dev, dev, pirq, newirq)) {
			eisa_set_level_irq(newirq);
			msg = "assigned";
			irq = newirq;
		}
	}

	if (!irq) {
		if (newirq && mask == (1 << newirq)) {
			msg = "guessed";
			irq = newirq;
		} else {
			dev_dbg(&dev->dev, "can't route interrupt\n");
			return 0;
		}
	}
	dev_info(&dev->dev, "%s PCI INT %c -> IRQ %d\n", msg, 'A' + pin - 1, irq);

	
	while ((dev2 = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev2)) != NULL) {
		pci_read_config_byte(dev2, PCI_INTERRUPT_PIN, &pin);
		if (!pin)
			continue;

		info = pirq_get_info(dev2);
		if (!info)
			continue;
		if (info->irq[pin - 1].link == pirq) {
			
			if (dev2->irq && dev2->irq != irq && \
			(!(pci_probe & PCI_USE_PIRQ_MASK) || \
			((1 << dev2->irq) & mask))) {
#ifndef CONFIG_PCI_MSI
				dev_info(&dev2->dev, "IRQ routing conflict: "
					 "have IRQ %d, want IRQ %d\n",
					 dev2->irq, irq);
#endif
				continue;
			}
			dev2->irq = irq;
			pirq_penalty[irq]++;
			if (dev != dev2)
				dev_info(&dev->dev, "sharing IRQ %d with %s\n",
					 irq, pci_name(dev2));
		}
	}
	return 1;
}

static void __init pcibios_fixup_irqs(void)
{
	struct pci_dev *dev = NULL;
	u8 pin;

	DBG(KERN_DEBUG "PCI: IRQ fixup\n");
	while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != NULL) {
		
		if (dev->irq >= 16) {
			dev_dbg(&dev->dev, "ignoring bogus IRQ %d\n", dev->irq);
			dev->irq = 0;
		}
		
		if (pirq_penalty[dev->irq] >= 100 &&
				pirq_penalty[dev->irq] < 100000)
			pirq_penalty[dev->irq] = 0;
		pirq_penalty[dev->irq]++;
	}

	if (io_apic_assign_pci_irqs)
		return;

	dev = NULL;
	while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != NULL) {
		pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
		if (!pin)
			continue;

		
		if (!dev->irq)
			pcibios_lookup_irq(dev, 0);
	}
}


static int __init fix_broken_hp_bios_irq9(const struct dmi_system_id *d)
{
	if (!broken_hp_bios_irq9) {
		broken_hp_bios_irq9 = 1;
		printk(KERN_INFO "%s detected - fixing broken IRQ routing\n",
			d->ident);
	}
	return 0;
}


static int __init fix_acer_tm360_irqrouting(const struct dmi_system_id *d)
{
	if (!acer_tm360_irqrouting) {
		acer_tm360_irqrouting = 1;
		printk(KERN_INFO "%s detected - fixing broken IRQ routing\n",
			d->ident);
	}
	return 0;
}

static struct dmi_system_id __initdata pciirq_dmi_table[] = {
	{
		.callback = fix_broken_hp_bios_irq9,
		.ident = "HP Pavilion N5400 Series Laptop",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
			DMI_MATCH(DMI_BIOS_VERSION, "GE.M1.03"),
			DMI_MATCH(DMI_PRODUCT_VERSION,
				"HP Pavilion Notebook Model GE"),
			DMI_MATCH(DMI_BOARD_VERSION, "OmniBook N32N-736"),
		},
	},
	{
		.callback = fix_acer_tm360_irqrouting,
		.ident = "Acer TravelMate 36x Laptop",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
			DMI_MATCH(DMI_PRODUCT_NAME, "TravelMate 360"),
		},
	},
	{ }
};

int __init pcibios_irq_init(void)
{
	DBG(KERN_DEBUG "PCI: IRQ init\n");

	if (pcibios_enable_irq || raw_pci_ops == NULL)
		return 0;

	dmi_check_system(pciirq_dmi_table);

	pirq_table = pirq_find_routing_table();

#ifdef CONFIG_PCI_BIOS
	if (!pirq_table && (pci_probe & PCI_BIOS_IRQ_SCAN))
		pirq_table = pcibios_get_irq_routing_table();
#endif
	if (pirq_table) {
		pirq_peer_trick();
		pirq_find_router(&pirq_router);
		if (pirq_table->exclusive_irqs) {
			int i;
			for (i = 0; i < 16; i++)
				if (!(pirq_table->exclusive_irqs & (1 << i)))
					pirq_penalty[i] += 100;
		}
		
		if (io_apic_assign_pci_irqs)
			pirq_table = NULL;
	}

	pcibios_enable_irq = pirq_enable_irq;

	pcibios_fixup_irqs();

	if (io_apic_assign_pci_irqs && pci_routeirq) {
		struct pci_dev *dev = NULL;
		
		printk(KERN_INFO "PCI: Routing PCI interrupts for all devices because \"pci=routeirq\" specified\n");
		for_each_pci_dev(dev)
			pirq_enable_irq(dev);
	}

	return 0;
}

static void pirq_penalize_isa_irq(int irq, int active)
{
	
	if (irq < 16) {
		if (active)
			pirq_penalty[irq] += 1000;
		else
			pirq_penalty[irq] += 100;
	}
}

void pcibios_penalize_isa_irq(int irq, int active)
{
#ifdef CONFIG_ACPI
	if (!acpi_noirq)
		acpi_penalize_isa_irq(irq, active);
	else
#endif
		pirq_penalize_isa_irq(irq, active);
}

static int pirq_enable_irq(struct pci_dev *dev)
{
	u8 pin;

	pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
	if (pin && !pcibios_lookup_irq(dev, 1)) {
		char *msg = "";

		if (!io_apic_assign_pci_irqs && dev->irq)
			return 0;

		if (io_apic_assign_pci_irqs) {
#ifdef CONFIG_X86_IO_APIC
			struct pci_dev *temp_dev;
			int irq;
			struct io_apic_irq_attr irq_attr;

			irq = IO_APIC_get_PCI_irq_vector(dev->bus->number,
						PCI_SLOT(dev->devfn),
						pin - 1, &irq_attr);
			
			temp_dev = dev;
			while (irq < 0 && dev->bus->parent) { 
				struct pci_dev *bridge = dev->bus->self;

				pin = pci_swizzle_interrupt_pin(dev, pin);
				irq = IO_APIC_get_PCI_irq_vector(bridge->bus->number,
						PCI_SLOT(bridge->devfn),
						pin - 1, &irq_attr);
				if (irq >= 0)
					dev_warn(&dev->dev, "using bridge %s "
						 "INT %c to get IRQ %d\n",
						 pci_name(bridge), 'A' + pin - 1,
						 irq);
				dev = bridge;
			}
			dev = temp_dev;
			if (irq >= 0) {
				io_apic_set_pci_routing(&dev->dev, irq,
							 &irq_attr);
				dev->irq = irq;
				dev_info(&dev->dev, "PCI->APIC IRQ transform: "
					 "INT %c -> IRQ %d\n", 'A' + pin - 1, irq);
				return 0;
			} else
				msg = "; probably buggy MP table";
#endif
		} else if (pci_probe & PCI_BIOS_IRQ_SCAN)
			msg = "";
		else
			msg = "; please try using pci=biosirq";

		
		if (dev->class >> 8 == PCI_CLASS_STORAGE_IDE &&
				!(dev->class & 0x5))
			return 0;

		dev_warn(&dev->dev, "can't find IRQ for PCI INT %c%s\n",
			 'A' + pin - 1, msg);
	}
	return 0;
}
