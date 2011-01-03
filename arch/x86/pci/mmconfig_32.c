



#include <linux/pci.h>
#include <linux/init.h>
#include <asm/e820.h>
#include <asm/pci_x86.h>
#include <acpi/acpi.h>


#define mmcfg_virt_addr ((void __iomem *) fix_to_virt(FIX_PCIE_MCFG))


static u32 mmcfg_last_accessed_device;
static int mmcfg_last_accessed_cpu;


static u32 get_base_addr(unsigned int seg, int bus, unsigned devfn)
{
	struct acpi_mcfg_allocation *cfg;
	int cfg_num;

	for (cfg_num = 0; cfg_num < pci_mmcfg_config_num; cfg_num++) {
		cfg = &pci_mmcfg_config[cfg_num];
		if (cfg->pci_segment == seg &&
		    (cfg->start_bus_number <= bus) &&
		    (cfg->end_bus_number >= bus))
			return cfg->address;
	}

	
	return 0;
}


static void pci_exp_set_dev_base(unsigned int base, int bus, int devfn)
{
	u32 dev_base = base | (bus << 20) | (devfn << 12);
	int cpu = smp_processor_id();
	if (dev_base != mmcfg_last_accessed_device ||
	    cpu != mmcfg_last_accessed_cpu) {
		mmcfg_last_accessed_device = dev_base;
		mmcfg_last_accessed_cpu = cpu;
		set_fixmap_nocache(FIX_PCIE_MCFG, dev_base);
	}
}

static int pci_mmcfg_read(unsigned int seg, unsigned int bus,
			  unsigned int devfn, int reg, int len, u32 *value)
{
	unsigned long flags;
	u32 base;

	if ((bus > 255) || (devfn > 255) || (reg > 4095)) {
err:		*value = -1;
		return -EINVAL;
	}

	base = get_base_addr(seg, bus, devfn);
	if (!base)
		goto err;

	spin_lock_irqsave(&pci_config_lock, flags);

	pci_exp_set_dev_base(base, bus, devfn);

	switch (len) {
	case 1:
		*value = mmio_config_readb(mmcfg_virt_addr + reg);
		break;
	case 2:
		*value = mmio_config_readw(mmcfg_virt_addr + reg);
		break;
	case 4:
		*value = mmio_config_readl(mmcfg_virt_addr + reg);
		break;
	}
	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static int pci_mmcfg_write(unsigned int seg, unsigned int bus,
			   unsigned int devfn, int reg, int len, u32 value)
{
	unsigned long flags;
	u32 base;

	if ((bus > 255) || (devfn > 255) || (reg > 4095))
		return -EINVAL;

	base = get_base_addr(seg, bus, devfn);
	if (!base)
		return -EINVAL;

	spin_lock_irqsave(&pci_config_lock, flags);

	pci_exp_set_dev_base(base, bus, devfn);

	switch (len) {
	case 1:
		mmio_config_writeb(mmcfg_virt_addr + reg, value);
		break;
	case 2:
		mmio_config_writew(mmcfg_virt_addr + reg, value);
		break;
	case 4:
		mmio_config_writel(mmcfg_virt_addr + reg, value);
		break;
	}
	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static struct pci_raw_ops pci_mmcfg = {
	.read =		pci_mmcfg_read,
	.write =	pci_mmcfg_write,
};

int __init pci_mmcfg_arch_init(void)
{
	printk(KERN_INFO "PCI: Using MMCONFIG for extended config space\n");
	raw_pci_ext_ops = &pci_mmcfg;
	return 1;
}

void __init pci_mmcfg_arch_free(void)
{
}
