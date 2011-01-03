

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/ide.h>

#include <asm/macintosh.h>
#include <asm/macints.h>
#include <asm/mac_baboon.h>

#define IDE_BASE 0x50F1A000	



#define IDE_CONTROL	0x38	





#define IDE_IFR		0x101	

volatile unsigned char *ide_ifr = (unsigned char *) (IDE_BASE + IDE_IFR);

int macide_test_irq(ide_hwif_t *hwif)
{
	if (*ide_ifr & 0x20)
		return 1;
	return 0;
}

static void macide_clear_irq(ide_drive_t *drive)
{
	*ide_ifr &= ~0x20;
}

static void __init macide_setup_ports(struct ide_hw *hw, unsigned long base,
				      int irq)
{
	int i;

	memset(hw, 0, sizeof(*hw));

	for (i = 0; i < 8; i++)
		hw->io_ports_array[i] = base + i * 4;

	hw->io_ports.ctl_addr = base + IDE_CONTROL;

	hw->irq = irq;
}

static const struct ide_port_ops macide_port_ops = {
	.clear_irq		= macide_clear_irq,
	.test_irq		= macide_test_irq,
};

static const struct ide_port_info macide_port_info = {
	.port_ops		= &macide_port_ops,
	.host_flags		= IDE_HFLAG_MMIO | IDE_HFLAG_NO_DMA,
	.irq_flags		= IRQF_SHARED,
	.chipset		= ide_generic,
};

static const char *mac_ide_name[] =
	{ "Quadra", "Powerbook", "Powerbook Baboon" };



static int __init macide_init(void)
{
	unsigned long base;
	int irq;
	struct ide_hw hw, *hws[] = { &hw };
	struct ide_port_info d = macide_port_info;

	if (!MACH_IS_MAC)
		return -ENODEV;

	switch (macintosh_config->ide_type) {
	case MAC_IDE_QUADRA:
		base = IDE_BASE;
		irq = IRQ_NUBUS_F;
		break;
	case MAC_IDE_PB:
		base = IDE_BASE;
		irq = IRQ_NUBUS_C;
		break;
	case MAC_IDE_BABOON:
		base = BABOON_BASE;
		d.port_ops = NULL;
		irq = IRQ_BABOON_1;
		break;
	default:
		return -ENODEV;
	}

	printk(KERN_INFO "ide: Macintosh %s IDE controller\n",
			 mac_ide_name[macintosh_config->ide_type - 1]);

	macide_setup_ports(&hw, base, irq);

	return ide_host_add(&d, hws, 1, NULL);
}

module_init(macide_init);

MODULE_LICENSE("GPL");
