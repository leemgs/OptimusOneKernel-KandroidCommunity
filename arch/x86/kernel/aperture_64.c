
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/pci_ids.h>
#include <linux/pci.h>
#include <linux/bitops.h>
#include <linux/ioport.h>
#include <linux/suspend.h>
#include <linux/kmemleak.h>
#include <asm/e820.h>
#include <asm/io.h>
#include <asm/iommu.h>
#include <asm/gart.h>
#include <asm/pci-direct.h>
#include <asm/dma.h>
#include <asm/k8.h>

int gart_iommu_aperture;
int gart_iommu_aperture_disabled __initdata;
int gart_iommu_aperture_allowed __initdata;

int fallback_aper_order __initdata = 1; 
int fallback_aper_force __initdata;

int fix_aperture __initdata = 1;

struct bus_dev_range {
	int bus;
	int dev_base;
	int dev_limit;
};

static struct bus_dev_range bus_dev_ranges[] __initdata = {
	{ 0x00, 0x18, 0x20},
	{ 0xff, 0x00, 0x20},
	{ 0xfe, 0x00, 0x20}
};

static struct resource gart_resource = {
	.name	= "GART",
	.flags	= IORESOURCE_MEM,
};

static void __init insert_aperture_resource(u32 aper_base, u32 aper_size)
{
	gart_resource.start = aper_base;
	gart_resource.end = aper_base + aper_size - 1;
	insert_resource(&iomem_resource, &gart_resource);
}



static u32 __init allocate_aperture(void)
{
	u32 aper_size;
	void *p;

	
	if (fallback_aper_order > 5)
		fallback_aper_order = 5;
	aper_size = (32 * 1024 * 1024) << fallback_aper_order;

	
	
	p = __alloc_bootmem_nopanic(aper_size, aper_size, 512ULL<<20);
	
	kmemleak_ignore(p);
	if (!p || __pa(p)+aper_size > 0xffffffff) {
		printk(KERN_ERR
			"Cannot allocate aperture memory hole (%p,%uK)\n",
				p, aper_size>>10);
		if (p)
			free_bootmem(__pa(p), aper_size);
		return 0;
	}
	printk(KERN_INFO "Mapping aperture over %d KB of RAM @ %lx\n",
			aper_size >> 10, __pa(p));
	insert_aperture_resource((u32)__pa(p), aper_size);
	register_nosave_region((u32)__pa(p) >> PAGE_SHIFT,
				(u32)__pa(p+aper_size) >> PAGE_SHIFT);

	return (u32)__pa(p);
}



static u32 __init find_cap(int bus, int slot, int func, int cap)
{
	int bytes;
	u8 pos;

	if (!(read_pci_config_16(bus, slot, func, PCI_STATUS) &
						PCI_STATUS_CAP_LIST))
		return 0;

	pos = read_pci_config_byte(bus, slot, func, PCI_CAPABILITY_LIST);
	for (bytes = 0; bytes < 48 && pos >= 0x40; bytes++) {
		u8 id;

		pos &= ~3;
		id = read_pci_config_byte(bus, slot, func, pos+PCI_CAP_LIST_ID);
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos = read_pci_config_byte(bus, slot, func,
						pos+PCI_CAP_LIST_NEXT);
	}
	return 0;
}


static u32 __init read_agp(int bus, int slot, int func, int cap, u32 *order)
{
	u32 apsize;
	u32 apsizereg;
	int nbits;
	u32 aper_low, aper_hi;
	u64 aper;
	u32 old_order;

	printk(KERN_INFO "AGP bridge at %02x:%02x:%02x\n", bus, slot, func);
	apsizereg = read_pci_config_16(bus, slot, func, cap + 0x14);
	if (apsizereg == 0xffffffff) {
		printk(KERN_ERR "APSIZE in AGP bridge unreadable\n");
		return 0;
	}

	
	old_order = *order;

	apsize = apsizereg & 0xfff;
	
	if (apsize & 0xff)
		apsize |= 0xf00;
	nbits = hweight16(apsize);
	*order = 7 - nbits;
	if ((int)*order < 0) 
		*order = 0;

	aper_low = read_pci_config(bus, slot, func, 0x10);
	aper_hi = read_pci_config(bus, slot, func, 0x14);
	aper = (aper_low & ~((1<<22)-1)) | ((u64)aper_hi << 32);

	
	printk(KERN_INFO "Aperture from AGP @ %Lx old size %u MB\n",
			aper, 32 << old_order);
	if (aper + (32ULL<<(20 + *order)) > 0x100000000ULL) {
		printk(KERN_INFO "Aperture size %u MB (APSIZE %x) is not right, using settings from NB\n",
				32 << *order, apsizereg);
		*order = old_order;
	}

	printk(KERN_INFO "Aperture from AGP @ %Lx size %u MB (APSIZE %x)\n",
			aper, 32 << *order, apsizereg);

	if (!aperture_valid(aper, (32*1024*1024) << *order, 32<<20))
		return 0;
	return (u32)aper;
}


static u32 __init search_agp_bridge(u32 *order, int *valid_agp)
{
	int bus, slot, func;

	
	for (bus = 0; bus < 256; bus++) {
		for (slot = 0; slot < 32; slot++) {
			for (func = 0; func < 8; func++) {
				u32 class, cap;
				u8 type;
				class = read_pci_config(bus, slot, func,
							PCI_CLASS_REVISION);
				if (class == 0xffffffff)
					break;

				switch (class >> 16) {
				case PCI_CLASS_BRIDGE_HOST:
				case PCI_CLASS_BRIDGE_OTHER: 
					
					cap = find_cap(bus, slot, func,
							PCI_CAP_ID_AGP);
					if (!cap)
						break;
					*valid_agp = 1;
					return read_agp(bus, slot, func, cap,
							order);
				}

				
				type = read_pci_config_byte(bus, slot, func,
							       PCI_HEADER_TYPE);
				if (!(type & 0x80))
					break;
			}
		}
	}
	printk(KERN_INFO "No AGP bridge found\n");

	return 0;
}

static int gart_fix_e820 __initdata = 1;

static int __init parse_gart_mem(char *p)
{
	if (!p)
		return -EINVAL;

	if (!strncmp(p, "off", 3))
		gart_fix_e820 = 0;
	else if (!strncmp(p, "on", 2))
		gart_fix_e820 = 1;

	return 0;
}
early_param("gart_fix_e820", parse_gart_mem);

void __init early_gart_iommu_check(void)
{
	
	int i, fix, slot;
	u32 ctl;
	u32 aper_size = 0, aper_order = 0, last_aper_order = 0;
	u64 aper_base = 0, last_aper_base = 0;
	int aper_enabled = 0, last_aper_enabled = 0, last_valid = 0;

	if (!early_pci_allowed())
		return;

	
	fix = 0;
	for (i = 0; i < ARRAY_SIZE(bus_dev_ranges); i++) {
		int bus;
		int dev_base, dev_limit;

		bus = bus_dev_ranges[i].bus;
		dev_base = bus_dev_ranges[i].dev_base;
		dev_limit = bus_dev_ranges[i].dev_limit;

		for (slot = dev_base; slot < dev_limit; slot++) {
			if (!early_is_k8_nb(read_pci_config(bus, slot, 3, 0x00)))
				continue;

			ctl = read_pci_config(bus, slot, 3, AMD64_GARTAPERTURECTL);
			aper_enabled = ctl & AMD64_GARTEN;
			aper_order = (ctl >> 1) & 7;
			aper_size = (32 * 1024 * 1024) << aper_order;
			aper_base = read_pci_config(bus, slot, 3, AMD64_GARTAPERTUREBASE) & 0x7fff;
			aper_base <<= 25;

			if (last_valid) {
				if ((aper_order != last_aper_order) ||
				    (aper_base != last_aper_base) ||
				    (aper_enabled != last_aper_enabled)) {
					fix = 1;
					break;
				}
			}

			last_aper_order = aper_order;
			last_aper_base = aper_base;
			last_aper_enabled = aper_enabled;
			last_valid = 1;
		}
	}

	if (!fix && !aper_enabled)
		return;

	if (!aper_base || !aper_size || aper_base + aper_size > 0x100000000UL)
		fix = 1;

	if (gart_fix_e820 && !fix && aper_enabled) {
		if (e820_any_mapped(aper_base, aper_base + aper_size,
				    E820_RAM)) {
			
			printk(KERN_INFO "update e820 for GART\n");
			e820_add_region(aper_base, aper_size, E820_RESERVED);
			update_e820();
		}
	}

	if (!fix)
		return;

	
	for (i = 0; i < ARRAY_SIZE(bus_dev_ranges); i++) {
		int bus;
		int dev_base, dev_limit;

		bus = bus_dev_ranges[i].bus;
		dev_base = bus_dev_ranges[i].dev_base;
		dev_limit = bus_dev_ranges[i].dev_limit;

		for (slot = dev_base; slot < dev_limit; slot++) {
			if (!early_is_k8_nb(read_pci_config(bus, slot, 3, 0x00)))
				continue;

			ctl = read_pci_config(bus, slot, 3, AMD64_GARTAPERTURECTL);
			ctl &= ~AMD64_GARTEN;
			write_pci_config(bus, slot, 3, AMD64_GARTAPERTURECTL, ctl);
		}
	}

}

static int __initdata printed_gart_size_msg;

void __init gart_iommu_hole_init(void)
{
	u32 agp_aper_base = 0, agp_aper_order = 0;
	u32 aper_size, aper_alloc = 0, aper_order = 0, last_aper_order = 0;
	u64 aper_base, last_aper_base = 0;
	int fix, slot, valid_agp = 0;
	int i, node;

	if (gart_iommu_aperture_disabled || !fix_aperture ||
	    !early_pci_allowed())
		return;

	printk(KERN_INFO  "Checking aperture...\n");

	if (!fallback_aper_force)
		agp_aper_base = search_agp_bridge(&agp_aper_order, &valid_agp);

	fix = 0;
	node = 0;
	for (i = 0; i < ARRAY_SIZE(bus_dev_ranges); i++) {
		int bus;
		int dev_base, dev_limit;

		bus = bus_dev_ranges[i].bus;
		dev_base = bus_dev_ranges[i].dev_base;
		dev_limit = bus_dev_ranges[i].dev_limit;

		for (slot = dev_base; slot < dev_limit; slot++) {
			if (!early_is_k8_nb(read_pci_config(bus, slot, 3, 0x00)))
				continue;

			iommu_detected = 1;
			gart_iommu_aperture = 1;

			aper_order = (read_pci_config(bus, slot, 3, AMD64_GARTAPERTURECTL) >> 1) & 7;
			aper_size = (32 * 1024 * 1024) << aper_order;
			aper_base = read_pci_config(bus, slot, 3, AMD64_GARTAPERTUREBASE) & 0x7fff;
			aper_base <<= 25;

			printk(KERN_INFO "Node %d: aperture @ %Lx size %u MB\n",
					node, aper_base, aper_size >> 20);
			node++;

			if (!aperture_valid(aper_base, aper_size, 64<<20)) {
				if (valid_agp && agp_aper_base &&
				    agp_aper_base == aper_base &&
				    agp_aper_order == aper_order) {
					
					if (!no_iommu &&
					    max_pfn > MAX_DMA32_PFN &&
					    !printed_gart_size_msg) {
						printk(KERN_ERR "you are using iommu with agp, but GART size is less than 64M\n");
						printk(KERN_ERR "please increase GART size in your BIOS setup\n");
						printk(KERN_ERR "if BIOS doesn't have that option, contact your HW vendor!\n");
						printed_gart_size_msg = 1;
					}
				} else {
					fix = 1;
					goto out;
				}
			}

			if ((last_aper_order && aper_order != last_aper_order) ||
			    (last_aper_base && aper_base != last_aper_base)) {
				fix = 1;
				goto out;
			}
			last_aper_order = aper_order;
			last_aper_base = aper_base;
		}
	}

out:
	if (!fix && !fallback_aper_force) {
		if (last_aper_base) {
			unsigned long n = (32 * 1024 * 1024) << last_aper_order;

			insert_aperture_resource((u32)last_aper_base, n);
		}
		return;
	}

	if (!fallback_aper_force) {
		aper_alloc = agp_aper_base;
		aper_order = agp_aper_order;
	}

	if (aper_alloc) {
		
	} else if (swiotlb && !valid_agp) {
		
	} else if ((!no_iommu && max_pfn > MAX_DMA32_PFN) ||
		   force_iommu ||
		   valid_agp ||
		   fallback_aper_force) {
		printk(KERN_INFO
			"Your BIOS doesn't leave a aperture memory hole\n");
		printk(KERN_INFO
			"Please enable the IOMMU option in the BIOS setup\n");
		printk(KERN_INFO
			"This costs you %d MB of RAM\n",
				32 << fallback_aper_order);

		aper_order = fallback_aper_order;
		aper_alloc = allocate_aperture();
		if (!aper_alloc) {
			
			panic("Not enough memory for aperture");
		}
	} else {
		return;
	}

	
	for (i = 0; i < ARRAY_SIZE(bus_dev_ranges); i++) {
		int bus;
		int dev_base, dev_limit;

		bus = bus_dev_ranges[i].bus;
		dev_base = bus_dev_ranges[i].dev_base;
		dev_limit = bus_dev_ranges[i].dev_limit;
		for (slot = dev_base; slot < dev_limit; slot++) {
			if (!early_is_k8_nb(read_pci_config(bus, slot, 3, 0x00)))
				continue;

			
			write_pci_config(bus, slot, 3, AMD64_GARTAPERTURECTL, aper_order << 1);
			write_pci_config(bus, slot, 3, AMD64_GARTAPERTUREBASE, aper_alloc >> 25);
		}
	}

	set_up_gart_resume(aper_order, aper_alloc);
}
