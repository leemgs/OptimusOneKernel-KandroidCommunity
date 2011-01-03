

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/bootmem.h>

#include <asm/pat.h>
#include <asm/e820.h>
#include <asm/pci_x86.h>
#include <asm/io_apic.h>


static int
skip_isa_ioresource_align(struct pci_dev *dev) {

	if ((pci_probe & PCI_CAN_SKIP_ISA_ALIGN) &&
	    !(dev->bus->bridge_ctl & PCI_BRIDGE_CTL_ISA))
		return 1;
	return 0;
}


void
pcibios_align_resource(void *data, struct resource *res,
			resource_size_t size, resource_size_t align)
{
	struct pci_dev *dev = data;

	if (res->flags & IORESOURCE_IO) {
		resource_size_t start = res->start;

		if (skip_isa_ioresource_align(dev))
			return;
		if (start & 0x300) {
			start = (start + 0x3ff) & ~0x3ff;
			res->start = start;
		}
	}
}
EXPORT_SYMBOL(pcibios_align_resource);



static void __init pcibios_allocate_bus_resources(struct list_head *bus_list)
{
	struct pci_bus *bus;
	struct pci_dev *dev;
	int idx;
	struct resource *r;

	
	list_for_each_entry(bus, bus_list, node) {
		if ((dev = bus->self)) {
			for (idx = PCI_BRIDGE_RESOURCES;
			    idx < PCI_NUM_RESOURCES; idx++) {
				r = &dev->resource[idx];
				if (!r->flags)
					continue;
				if (!r->start ||
				    pci_claim_resource(dev, idx) < 0) {
					dev_info(&dev->dev, "BAR %d: can't allocate resource\n", idx);
					
					r->flags = 0;
				}
			}
		}
		pcibios_allocate_bus_resources(&bus->children);
	}
}

static void __init pcibios_allocate_resources(int pass)
{
	struct pci_dev *dev = NULL;
	int idx, disabled;
	u16 command;
	struct resource *r;

	for_each_pci_dev(dev) {
		pci_read_config_word(dev, PCI_COMMAND, &command);
		for (idx = 0; idx < PCI_ROM_RESOURCE; idx++) {
			r = &dev->resource[idx];
			if (r->parent)		
				continue;
			if (!r->start)		
				continue;
			if (r->flags & IORESOURCE_IO)
				disabled = !(command & PCI_COMMAND_IO);
			else
				disabled = !(command & PCI_COMMAND_MEMORY);
			if (pass == disabled) {
				dev_dbg(&dev->dev, "resource %#08llx-%#08llx (f=%lx, d=%d, p=%d)\n",
					(unsigned long long) r->start,
					(unsigned long long) r->end,
					r->flags, disabled, pass);
				if (pci_claim_resource(dev, idx) < 0) {
					dev_info(&dev->dev, "BAR %d: can't allocate resource\n", idx);
					
					r->end -= r->start;
					r->start = 0;
				}
			}
		}
		if (!pass) {
			r = &dev->resource[PCI_ROM_RESOURCE];
			if (r->flags & IORESOURCE_ROM_ENABLE) {
				
				u32 reg;
				dev_dbg(&dev->dev, "disabling ROM\n");
				r->flags &= ~IORESOURCE_ROM_ENABLE;
				pci_read_config_dword(dev,
						dev->rom_base_reg, &reg);
				pci_write_config_dword(dev, dev->rom_base_reg,
						reg & ~PCI_ROM_ADDRESS_ENABLE);
			}
		}
	}
}

static int __init pcibios_assign_resources(void)
{
	struct pci_dev *dev = NULL;
	struct resource *r;

	if (!(pci_probe & PCI_ASSIGN_ROMS)) {
		
		for_each_pci_dev(dev) {
			r = &dev->resource[PCI_ROM_RESOURCE];
			if (!r->flags || !r->start)
				continue;
			if (pci_claim_resource(dev, PCI_ROM_RESOURCE) < 0) {
				r->end -= r->start;
				r->start = 0;
			}
		}
	}

	pci_assign_unassigned_resources();

	return 0;
}

void __init pcibios_resource_survey(void)
{
	DBG("PCI: Allocating resources\n");
	pcibios_allocate_bus_resources(&pci_root_buses);
	pcibios_allocate_resources(0);
	pcibios_allocate_resources(1);

	e820_reserve_resources_late();
	
	ioapic_insert_resources();
}


fs_initcall(pcibios_assign_resources);

void __weak x86_pci_root_bus_res_quirks(struct pci_bus *b)
{
}


unsigned int pcibios_max_latency = 255;

void pcibios_set_master(struct pci_dev *dev)
{
	u8 lat;
	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &lat);
	if (lat < 16)
		lat = (64 <= pcibios_max_latency) ? 64 : pcibios_max_latency;
	else if (lat > pcibios_max_latency)
		lat = pcibios_max_latency;
	else
		return;
	dev_printk(KERN_DEBUG, &dev->dev, "setting latency timer to %d\n", lat);
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, lat);
}

static const struct vm_operations_struct pci_mmap_ops = {
	.access = generic_access_phys,
};

int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
			enum pci_mmap_state mmap_state, int write_combine)
{
	unsigned long prot;

	
	if (mmap_state == pci_mmap_io)
		return -EINVAL;

	prot = pgprot_val(vma->vm_page_prot);

	
	if (!pat_enabled && write_combine)
		return -EINVAL;

	if (pat_enabled && write_combine)
		prot |= _PAGE_CACHE_WC;
	else if (pat_enabled || boot_cpu_data.x86 > 3)
		
		prot |= _PAGE_CACHE_UC_MINUS;

	vma->vm_page_prot = __pgprot(prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			       vma->vm_end - vma->vm_start,
			       vma->vm_page_prot))
		return -EAGAIN;

	vma->vm_ops = &pci_mmap_ops;

	return 0;
}
