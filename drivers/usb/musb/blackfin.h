

#ifndef __MUSB_BLACKFIN_H__
#define __MUSB_BLACKFIN_H__



#undef DUMP_FIFO_DATA
#ifdef DUMP_FIFO_DATA
static void dump_fifo_data(u8 *buf, u16 len)
{
	u8 *tmp = buf;
	int i;

	for (i = 0; i < len; i++) {
		if (!(i % 16) && i)
			pr_debug("\n");
		pr_debug("%02x ", *tmp++);
	}
	pr_debug("\n");
}
#else
#define dump_fifo_data(buf, len)	do {} while (0)
#endif

#ifdef CONFIG_BF52x

#define USB_DMA_BASE		USB_DMA_INTERRUPT
#define USB_DMAx_CTRL		0x04
#define USB_DMAx_ADDR_LOW	0x08
#define USB_DMAx_ADDR_HIGH	0x0C
#define USB_DMAx_COUNT_LOW	0x10
#define USB_DMAx_COUNT_HIGH	0x14

#define USB_DMA_REG(ep, reg)	(USB_DMA_BASE + 0x20 * ep + reg)
#endif


#define TIMER_DELAY	(1 * HZ)

static struct timer_list musb_conn_timer;

#endif	
