#ifndef __SEP_DEV_H__
#define __SEP_DEV_H__



struct sep_device {
	
	struct pci_dev *pdev;

	unsigned long in_use;

	
	void *shared_addr;
	
	dma_addr_t shared_bus;

	
	dma_addr_t rar_bus;
	void *rar_addr;
	
	unsigned long cache_size;

	
	dma_addr_t resident_bus;
	unsigned long resident_size;
	void *resident_addr;

	
	void __iomem *reg_addr;
	
	unsigned long send_ct;
	
	unsigned long reply_ct;
	
	unsigned long data_pool_bytes_allocated;

	
	struct page **in_page_array;

	
	struct page **out_page_array;

	
	unsigned long in_num_pages;

	
	unsigned long out_num_pages;

	
	struct sep_flow_context_t flows[SEP_DRIVER_NUM_FLOWS];

	
	struct workqueue_struct *flow_wq;

};

static struct sep_device *sep_dev;

static inline void sep_write_reg(struct sep_device *dev, int reg, u32 value)
{
	void __iomem *addr = dev->reg_addr + reg;
	writel(value, addr);
}

static inline u32 sep_read_reg(struct sep_device *dev, int reg)
{
	void __iomem *addr = dev->reg_addr + reg;
	return readl(addr);
}


static inline void sep_wait_sram_write(struct sep_device *dev)
{
	u32 reg_val;
	do
		reg_val = sep_read_reg(dev, HW_SRAM_DATA_READY_REG_ADDR);
	while (!(reg_val & 1));
}


#endif
