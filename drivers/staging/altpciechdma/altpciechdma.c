

#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/pci.h>




#ifndef ALTPCIECHDMA_CDEV
#  define ALTPCIECHDMA_CDEV 0
#endif


#if ALTPCIECHDMA_CDEV
#  define MAX_CHDMA_SIZE (8 * 1024 * 1024)
#  include "mapper_user_to_sg.h"
#endif


#define DRV_NAME "altpciechdma"

#define APE_BAR_NUM (6)

#define APE_BAR_RCSLAVE (0)

#define APE_BAR_HEADER (2)


#define APE_CHDMA_TABLE_SIZE (4096)

#define APE_CHDMA_MAX_TRANSFER_LEN (253 * PAGE_SIZE)


static const unsigned long bar_min_len[APE_BAR_NUM] =
	{ 32768, 0, 256, 0, 32768, 0 };


struct ape_chdma_header {
	
	u32 w0;
	
	u32 bdt_addr_h;
	u32 bdt_addr_l;
	
	u32 w3;
} __attribute__ ((packed));


struct ape_chdma_desc {
	
	u32 w0;
	
	u32 ep_addr;
	
	u32 rc_addr_h;
	u32 rc_addr_l;
} __attribute__ ((packed));


struct ape_chdma_table {
	
	u32 reserved1[3];
	
	u32 w3;
	
	struct ape_chdma_desc desc[255];
} __attribute__ ((packed));


struct ape_dev {
	
	struct pci_dev *pci_dev;
	
	void * __iomem bar[APE_BAR_NUM];
	
	struct ape_chdma_table *table_virt;
	
	dma_addr_t table_bus;
	
	int in_use;
	
	int msi_enabled;
	
	int got_regions;
	
	int irq_line;
	
	u8 revision;
	
	int irq_count;
#if ALTPCIECHDMA_CDEV
	
	dev_t cdevno;
	struct cdev cdev;
	
	struct sg_mapping_t *sgm;
#endif
};


static const struct pci_device_id ids[] = {
	{ PCI_DEVICE(0x1172, 0xE001), },
	{ PCI_DEVICE(0x2071, 0x2071), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

#if ALTPCIECHDMA_CDEV

static int sg_init(struct ape_dev *ape);
static void sg_exit(struct ape_dev *ape);
#endif


static irqreturn_t altpciechdma_isr(int irq, void *dev_id)
{
	struct ape_dev *ape = (struct ape_dev *)dev_id;
	if (!ape)
		return IRQ_NONE;
	ape->irq_count++;
	return IRQ_HANDLED;
}

static int __devinit scan_bars(struct ape_dev *ape, struct pci_dev *dev)
{
	int i;
	for (i = 0; i < APE_BAR_NUM; i++) {
		unsigned long bar_start = pci_resource_start(dev, i);
		if (bar_start) {
			unsigned long bar_end = pci_resource_end(dev, i);
			unsigned long bar_flags = pci_resource_flags(dev, i);
			printk(KERN_DEBUG "BAR%d 0x%08lx-0x%08lx flags 0x%08lx\n",
			  i, bar_start, bar_end, bar_flags);
		}
	}
	return 0;
}


static void unmap_bars(struct ape_dev *ape, struct pci_dev *dev)
{
	int i;
	for (i = 0; i < APE_BAR_NUM; i++) {
	  
		if (ape->bar[i]) {
			
			pci_iounmap(dev, ape->bar[i]);
			ape->bar[i] = NULL;
		}
	}
}


static int __devinit map_bars(struct ape_dev *ape, struct pci_dev *dev)
{
	int rc;
	int i;
	
	for (i = 0; i < APE_BAR_NUM; i++) {
		unsigned long bar_start = pci_resource_start(dev, i);
		unsigned long bar_end = pci_resource_end(dev, i);
		unsigned long bar_length = bar_end - bar_start + 1;
		ape->bar[i] = NULL;
		
		if (!bar_min_len[i])
			continue;
		
		if (!bar_start || !bar_end) {
			printk(KERN_DEBUG "BAR #%d is not present?!\n", i);
			rc = -1;
			goto fail;
		}
		bar_length = bar_end - bar_start + 1;
		
		if (bar_length < bar_min_len[i]) {
			printk(KERN_DEBUG "BAR #%d length = %lu bytes but driver "
			"requires at least %lu bytes\n",
			i, bar_length, bar_min_len[i]);
			rc = -1;
			goto fail;
		}
		
		ape->bar[i] = pci_iomap(dev, i, bar_min_len[i]);
		if (!ape->bar[i]) {
			printk(KERN_DEBUG "Could not map BAR #%d.\n", i);
			rc = -1;
			goto fail;
		}
		printk(KERN_DEBUG "BAR[%d] mapped at 0x%p with length %lu(/%lu).\n", i,
		ape->bar[i], bar_min_len[i], bar_length);
	}
	
	rc = 0;
	goto success;
fail:
	
	unmap_bars(ape, dev);
success:
	return rc;
}

#if 0 
static void __devinit rcslave_test(struct ape_dev *ape, struct pci_dev *dev)
{
	u32 *rcslave_mem = (u32 *)ape->bar[APE_BAR_RCSLAVE];
	u32 result = 0;
	
	u32 seed = (u32)jiffies;
	u32 value = seed;
	int i;

	
	value = seed;
	for (i = 1024; i < 32768 / 4 ; i++) {
		printk(KERN_DEBUG "Writing 0x%08x to 0x%p.\n",
			(u32)value, (void *)rcslave_mem + i);
		iowrite32(value, rcslave_mem + i);
		value++;
	}
	
	value = seed;
	for (i = 1024; i < 32768 / 4; i++) {
		result = ioread32(rcslave_mem + i);
		if (result != value) {
			printk(KERN_DEBUG "Wrote 0x%08x to 0x%p, but read back 0x%08x.\n",
				(u32)value, (void *)rcslave_mem + i, (u32)result);
			break;
		}
		value++;
	}
}
#endif


#define pci_dma_h(addr) ((addr >> 16) >> 16)

#define pci_dma_l(addr) (addr & 0xffffffffUL)


static inline void ape_chdma_desc_set(struct ape_chdma_desc *desc, dma_addr_t addr, u32 ep_addr, int len)
{
  BUG_ON(len & 3);
	desc->w0 = cpu_to_le32(len / 4);
	desc->ep_addr = cpu_to_le32(ep_addr);
	desc->rc_addr_h = cpu_to_le32(pci_dma_h(addr));
	desc->rc_addr_l = cpu_to_le32(pci_dma_l(addr));
}

#if ALTPCIECHDMA_CDEV

static int ape_sg_to_chdma_table(struct scatterlist *sgl, int nents, int first, struct ape_chdma_desc *desc, u32 ep_addr)
{
	int i = first, j = 0;
	
	dma_addr_t addr = sg_dma_address(&sgl[i]);
	unsigned int len = sg_dma_len(&sgl[i]);
	
	dma_addr_t cont_addr = addr;
	unsigned int cont_len = len;
	
	for (; j < 25 && i < nents - 1; i++) {
		
		dma_addr_t next = sg_dma_address(&sgl[i + 1]);
		
		len = sg_dma_len(&sgl[i]);
		printk(KERN_DEBUG "%04d: addr=0x%Lx length=0x%08x\n", i,
			(unsigned long long)addr, len);
		
		if (next != addr + len) {
			
			printk(KERN_DEBUG "%4d: cont_addr=0x%Lx cont_len=0x%08x\n", j,
				(unsigned long long)cont_addr, cont_len);
			
			ape_chdma_desc_set(&desc[j], cont_addr, ep_addr, cont_len);
			
			ep_addr += cont_len;
			
			cont_addr = next;
			cont_len = 0;
			j++;
		}
		
		cont_len += len;
		
		addr = next;
	}
	
	printk(KERN_DEBUG "%04d: addr=0x%Lx length=0x%08x\n", i,
		(unsigned long long)addr, len);
	printk(KERN_DEBUG "%4d: cont_addr=0x%Lx length=0x%08x\n", j,
		(unsigned long long)cont_addr, cont_len);
	j++;
	return j;
}
#endif


static inline int compare(u32 *p, u32 *q, int len)
{
	int result = -1;
	int fail = 0;
	int i;
	for (i = 0; i < len / 4; i++) {
		if (*p == *q) {
			
			if ((i & 255) == 0)
				printk(KERN_DEBUG "[%p] = 0x%08x    [%p] = 0x%08x\n", p, *p, q, *q);
		} else {
			fail++;
			
			if (fail < 10)
				printk(KERN_DEBUG "[%p] = 0x%08x != [%p] = 0x%08x ?!\n", p, *p, q, *q);
				
			else if (fail == 10)
				printk(KERN_DEBUG "---more errors follow! not printed---\n");
			else
				
			break;
		}
		p++;
		q++;
	}
	if (!fail)
		result = 0;
	return result;
}


static int __devinit dma_test(struct ape_dev *ape, struct pci_dev *dev)
{
	
	int result = -1;
	
	struct ape_chdma_header *write_header = (struct ape_chdma_header *)ape->bar[APE_BAR_HEADER];
	
	struct ape_chdma_header *read_header = write_header + 1;
	
	u8 *buffer_virt = 0;
	
	dma_addr_t buffer_bus = 0;
	int i, n = 0, irq_count;

	
	u32 w;

	printk(KERN_DEBUG "bar_tests(), PAGE_SIZE = 0x%0x\n", (int)PAGE_SIZE);
	printk(KERN_DEBUG "write_header = 0x%p.\n", write_header);
	printk(KERN_DEBUG "read_header = 0x%p.\n", read_header);
	printk(KERN_DEBUG "&write_header->w3 = 0x%p\n", &write_header->w3);
	printk(KERN_DEBUG "&read_header->w3 = 0x%p\n", &read_header->w3);
	printk(KERN_DEBUG "ape->table_virt = 0x%p.\n", ape->table_virt);

	if (!write_header || !read_header || !ape->table_virt)
		goto fail;

	
	
	buffer_virt = (u8 *)pci_alloc_consistent(dev, PAGE_SIZE * 4, &buffer_bus);
	if (!buffer_virt) {
		printk(KERN_DEBUG "Could not allocate coherent DMA buffer.\n");
		goto fail;
	}
	printk(KERN_DEBUG "Allocated cache-coherent DMA buffer (virtual address = %p, bus address = 0x%016llx).\n",
	       buffer_virt, (u64)buffer_bus);

	
	for (i = 0; i < 4 * PAGE_SIZE; i += 4)
#if 0
		*(u32 *)(buffer_virt + i) = i / PAGE_SIZE + 1;
#else
		*(u32 *)(buffer_virt + i) = (u32)(unsigned long)(buffer_virt + i);
#endif
#if 0
  compare((u32 *)buffer_virt, (u32 *)(buffer_virt + 2 * PAGE_SIZE), 8192);
#endif

#if 0
	
	for (i = 2 * PAGE_SIZE; i < 4 * PAGE_SIZE; i += 4)
		*(u32 *)(buffer_virt + i) = 0;
#endif

	
	ape->table_virt->w3 = cpu_to_le32(0x0000FADE);

	
	n = 0;
	
	ape_chdma_desc_set(&ape->table_virt->desc[n], buffer_bus, 4096, 2 * PAGE_SIZE);
#if 1
	for (i = 0; i < 255; i++)
		ape_chdma_desc_set(&ape->table_virt->desc[i], buffer_bus, 4096, 2 * PAGE_SIZE);
	
	n = i - 1;
#endif
#if 0
	
	n++;
	
	ape_chdma_desc_set(&ape->table_virt->desc[n], buffer_bus + 1024, 4096 + 1024, 1024);
#endif

#if 1
	
	if (ape->msi_enabled)
		ape->table_virt->desc[n].w0 |= cpu_to_le32(1UL << 16);
#endif
#if 0
	
	printk(KERN_DEBUG "Descriptor Table (Read, in Root Complex Memory, # = %d)\n", n + 1);
	for (i = 0; i < 4 + (n + 1) * 4; i += 4) {
		u32 *p = (u32 *)ape->table_virt;
		p += i;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (LEN=0x%x)\n", (u32)p, (u32)p & 15, *p, 4 * le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (EPA=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (RCH=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (RCL=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
	}
#endif
	
	w = (u32)(n + 1);
	w |= (1UL << 18);
#if 0
	if (ape->msi_enabled)
		w |= (1UL << 17);
#endif
	printk(KERN_DEBUG "writing 0x%08x to 0x%p\n", w, (void *)&read_header->w0);
	iowrite32(w, &read_header->w0);

	
	printk(KERN_DEBUG "writing 0x%08x to 0x%p\n", (u32)((ape->table_bus >> 16) >> 16), (void *)&read_header->bdt_addr_h);
	iowrite32(pci_dma_h(ape->table_bus), &read_header->bdt_addr_h);

	
	printk(KERN_DEBUG "writing 0x%08x to 0x%p\n", (u32)(ape->table_bus & 0xffffffffUL), (void *)&read_header->bdt_addr_l);
	iowrite32(pci_dma_l(ape->table_bus), &read_header->bdt_addr_l);

	
	wmb();
	printk(KERN_DEBUG "Flush posted writes\n");
	
#if 0
	(void)ioread32();
#endif

	
	irq_count = ape->irq_count;
	
	printk(KERN_DEBUG "\nStart DMA read\n");
	printk(KERN_DEBUG "writing 0x%08x to 0x%p\n", (u32)n, (void *)&read_header->w3);
	iowrite32(n, &read_header->w3);
	printk(KERN_DEBUG "EPLAST = %lu\n", le32_to_cpu(*(u32 *)&ape->table_virt->w3) & 0xffffUL);

	
	wmb();
	
	
#if 0
	(void)ioread32();
#endif
	printk(KERN_DEBUG "POLL FOR READ:\n");
	
	for (i = 0; i < 100; i++) {
		volatile u32 *p = &ape->table_virt->w3;
		u32 eplast = le32_to_cpu(*p) & 0xffffUL;
		printk(KERN_DEBUG "EPLAST = %u, n = %d\n", eplast, n);
		if (eplast == n) {
			printk(KERN_DEBUG "DONE\n");
			
			printk(KERN_DEBUG "#IRQs during transfer: %d\n", ape->irq_count - irq_count);
			break;
		}
		udelay(100);
	}

	
	ape->table_virt->w3 = cpu_to_le32(0x0000FADE);

	
	n = 0;
	ape_chdma_desc_set(&ape->table_virt->desc[n], buffer_bus + 8192, 4096, 2 * PAGE_SIZE);
#if 1
	for (i = 0; i < 255; i++)
		ape_chdma_desc_set(&ape->table_virt->desc[i], buffer_bus + 8192, 4096, 2 * PAGE_SIZE);

	
	n = i - 1;
#endif
#if 1 
	if (ape->msi_enabled)
		ape->table_virt->desc[n].w0 |= cpu_to_le32(1UL << 16);
#endif
#if 0
	
	printk(KERN_DEBUG "Descriptor Table (Write, in Root Complex Memory, # = %d)\n", n + 1);
	for (i = 0; i < 4 + (n + 1) * 4; i += 4) {
		u32 *p = (u32 *)ape->table_virt;
		p += i;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (LEN=0x%x)\n", (u32)p, (u32)p & 15, *p, 4 * le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (EPA=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (RCH=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
		p++;
		printk(KERN_DEBUG "0x%08x/0x%02x: 0x%08x (RCL=0x%x)\n", (u32)p, (u32)p & 15, *p, le32_to_cpu(*p));
	}
#endif

	
	w = (u32)(n + 1);
	
	w |= (u32)(1UL << 18);
#if 0   
	
	if (ape->msi_enabled)
		w |= (1UL << 17);
#endif
	iowrite32(w, &write_header->w0);
	iowrite32(pci_dma_h(ape->table_bus), &write_header->bdt_addr_h);
	iowrite32(pci_dma_l(ape->table_bus), &write_header->bdt_addr_l);

	
	wmb();
	
	
#if 0
	(void)ioread32();
#endif
	irq_count = ape->irq_count;

	printk(KERN_DEBUG "\nStart DMA write\n");
	iowrite32(n, &write_header->w3);

	
	wmb();
	
	

	printk(KERN_DEBUG "POLL FOR WRITE:\n");
	
	for (i = 0; i < 100; i++) {
		volatile u32 *p = &ape->table_virt->w3;
		u32 eplast = le32_to_cpu(*p) & 0xffffUL;
		printk(KERN_DEBUG "EPLAST = %u, n = %d\n", eplast, n);
		if (eplast == n) {
			printk(KERN_DEBUG "DONE\n");
			
			printk(KERN_DEBUG "#IRQs during transfer: %d\n", ape->irq_count - irq_count);
			break;
		}
		udelay(100);
	}
	
	iowrite32(0x0000ffffUL, &write_header->w0);
	
	iowrite32(0x0000ffffUL, &read_header->w0);

	
	wmb();
	
	
#if 0
	(void)ioread32();
#endif
	
	result = compare((u32 *)buffer_virt, (u32 *)(buffer_virt + 2 * PAGE_SIZE), 8192);
	printk(KERN_DEBUG "DMA loop back test %s.\n", result ? "FAILED" : "PASSED");

	pci_free_consistent(dev, 4 * PAGE_SIZE, buffer_virt, buffer_bus);
fail:
	printk(KERN_DEBUG "bar_tests() end, result %d\n", result);
	return result;
}


static int __devinit probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int rc = 0;
	struct ape_dev *ape = NULL;
	u8 irq_pin, irq_line;
	printk(KERN_DEBUG "probe(dev = 0x%p, pciid = 0x%p)\n", dev, id);

	
	ape = kzalloc(sizeof(struct ape_dev), GFP_KERNEL);
	if (!ape) {
		printk(KERN_DEBUG "Could not kzalloc()ate memory.\n");
		goto err_ape;
	}
	ape->pci_dev = dev;
	dev_set_drvdata(&dev->dev, ape);
	printk(KERN_DEBUG "probe() ape = 0x%p\n", ape);

	printk(KERN_DEBUG "sizeof(struct ape_chdma_table) = %d.\n",
		(int)sizeof(struct ape_chdma_table));
	
	BUG_ON(sizeof(struct ape_chdma_table) > APE_CHDMA_TABLE_SIZE);

	
	
	ape->table_virt = (struct ape_chdma_table *)pci_alloc_consistent(dev,
		APE_CHDMA_TABLE_SIZE, &ape->table_bus);
	
	if (!ape->table_virt) {
		printk(KERN_DEBUG "Could not dma_alloc()ate_coherent memory.\n");
		goto err_table;
	}

	printk(KERN_DEBUG "table_virt = %p, table_bus = 0x%16llx.\n",
		ape->table_virt, (u64)ape->table_bus);

	
	rc = pci_enable_device(dev);
	if (rc) {
		printk(KERN_DEBUG "pci_enable_device() failed\n");
		goto err_enable;
	}

	
	pci_set_master(dev);
	
	rc = pci_enable_msi(dev);
	
	if (rc) {
		
		printk(KERN_DEBUG "Could not enable MSI interrupting.\n");
		ape->msi_enabled = 0;
	
	} else {
		printk(KERN_DEBUG "Enabled MSI interrupting.\n");
		ape->msi_enabled = 1;
	}

	pci_read_config_byte(dev, PCI_REVISION_ID, &ape->revision);
#if 0 
	
    if (ape->revision == 0x42) {
		printk(KERN_DEBUG "Revision 0x42 is not supported by this driver.\n");
		rc = -ENODEV;
		goto err_rev;
	}
#endif
	

	rc = pci_request_regions(dev, DRV_NAME);
	
	if (rc) {
		
		ape->in_use = 1;
		goto err_regions;
	}
	ape->got_regions = 1;

#if 1   
	
	
	if (!pci_set_dma_mask(dev, DMA_BIT_MASK(64))) {
		pci_set_consistent_dma_mask(dev, DMA_BIT_MASK(64));
		
		printk(KERN_DEBUG "Using a 64-bit DMA mask.\n");
	} else
#endif
	if (!pci_set_dma_mask(dev, DMA_BIT_MASK(32))) {
		printk(KERN_DEBUG "Could not set 64-bit DMA mask.\n");
		pci_set_consistent_dma_mask(dev, DMA_BIT_MASK(32));
		
		printk(KERN_DEBUG "Using a 32-bit DMA mask.\n");
	} else {
		printk(KERN_DEBUG "No suitable DMA possible.\n");
		
		rc = -1;
		goto err_mask;
	}

	rc = pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &irq_pin);
	
	if (rc)
		goto err_irq;
	printk(KERN_DEBUG "IRQ pin #%d (0=none, 1=INTA#...4=INTD#).\n", irq_pin);

	
	rc = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &irq_line);
	
	if (rc) {
		printk(KERN_DEBUG "Could not query PCI_INTERRUPT_LINE, error %d\n", rc);
		goto err_irq;
	}
	printk(KERN_DEBUG "IRQ line #%d.\n", irq_line);
#if 1
	irq_line = dev->irq;
	
	rc = request_irq(irq_line, altpciechdma_isr, IRQF_SHARED, DRV_NAME, (void *)ape);
	if (rc) {
		printk(KERN_DEBUG "Could not request IRQ #%d, error %d\n", irq_line, rc);
		ape->irq_line = -1;
		goto err_irq;
	}
	
	ape->irq_line = (int)irq_line;
	printk(KERN_DEBUG "Succesfully requested IRQ #%d with dev_id 0x%p\n", irq_line, ape);
#endif
	
	scan_bars(ape, dev);
	
	rc = map_bars(ape, dev);
	if (rc)
		goto err_map;
#if ALTPCIECHDMA_CDEV
	
	rc = sg_init(ape);
	if (rc)
		goto err_cdev;
#endif
	
	rc = dma_test(ape, dev);
	(void)rc;
	
	rc = 0;
	printk(KERN_DEBUG "probe() successful.\n");
	goto end;
#if ALTPCIECHDMA_CDEV
err_cdev:
	
	unmap_bars(ape, dev);
#endif
err_map:
	
	if (ape->irq_line >= 0)
		free_irq(ape->irq_line, (void *)ape);
err_irq:
	if (ape->msi_enabled)
		pci_disable_msi(dev);
	
	if (!ape->in_use)
		pci_disable_device(dev);
	if (ape->got_regions)
		pci_release_regions(dev);
err_mask:
err_regions:


err_enable:
	if (ape->table_virt)
		pci_free_consistent(dev, APE_CHDMA_TABLE_SIZE, ape->table_virt, ape->table_bus);

err_table:
	if (ape)
		kfree(ape);
err_ape:
end:
	return rc;
}

static void __devexit remove(struct pci_dev *dev)
{
	struct ape_dev *ape = dev_get_drvdata(&dev->dev);

	printk(KERN_DEBUG "remove(0x%p)\n", dev);
	printk(KERN_DEBUG "remove(dev = 0x%p) where ape = 0x%p\n", dev, ape);

	
#if ALTPCIECHDMA_CDEV
	sg_exit(ape);
#endif

	if (ape->table_virt)
		pci_free_consistent(dev, APE_CHDMA_TABLE_SIZE, ape->table_virt, ape->table_bus);

	
	if (ape->irq_line >= 0) {
		printk(KERN_DEBUG "Freeing IRQ #%d for dev_id 0x%08lx.\n",
		ape->irq_line, (unsigned long)ape);
		free_irq(ape->irq_line, (void *)ape);
	}
	
	if (ape->msi_enabled) {
		
		pci_disable_msi(dev);
		ape->msi_enabled = 0;
	}
	
	unmap_bars(ape, dev);
	if (!ape->in_use)
		pci_disable_device(dev);
	if (ape->got_regions)
		
		pci_release_regions(dev);
}

#if ALTPCIECHDMA_CDEV


static int sg_open(struct inode *inode, struct file *file)
{
	struct ape_dev *ape;
	printk(KERN_DEBUG DRV_NAME "_open()\n");
	
	ape = container_of(inode->i_cdev, struct ape_dev, cdev);
	
	file->private_data = ape;
	
	ape->sgm = sg_create_mapper(MAX_CHDMA_SIZE);
	return 0;
}


static int sg_close(struct inode *inode, struct file *file)
{
	
	struct ape_dev *ape = (struct ape_dev *)file->private_data;
	printk(KERN_DEBUG DRV_NAME "_close()\n");
	
	sg_destroy_mapper(ape->sgm);
	return 0;
}

static ssize_t sg_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	
	struct ape_dev *ape = (struct ape_dev *)file->private_data;
	(void)ape;
	printk(KERN_DEBUG DRV_NAME "_read(buf=0x%p, count=%lld, pos=%llu)\n", buf, (s64)count, (u64)*pos);
	return count;
}


static ssize_t sg_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	int hwnents, tents;
	size_t transfer_len, remaining = count, done = 0;
	u64 transfer_addr = (u64)buf;
	
	struct ape_dev *ape = (struct ape_dev *)file->private_data;
	printk(KERN_DEBUG DRV_NAME "_write(buf=0x%p, count=%lld, pos=%llu)\n",
		buf, (s64)count, (u64)*pos);
	
	while (remaining > 0) {
		
		transfer_len = (remaining < APE_CHDMA_MAX_TRANSFER_LEN) ? remaining :
			APE_CHDMA_MAX_TRANSFER_LEN;
		
		sgm_map_user_pages(ape->sgm, transfer_addr, transfer_len, 0);
		printk(KERN_DEBUG DRV_NAME "mapped_pages=%d\n", ape->sgm->mapped_pages);
		
		hwnents = pci_map_sg(ape->pci_dev, ape->sgm->sgl, ape->sgm->mapped_pages, DMA_TO_DEVICE);
		printk(KERN_DEBUG DRV_NAME "hwnents=%d\n", hwnents);
		
		tents = ape_sg_to_chdma_table(ape->sgm->sgl, hwnents, 0, &ape->table_virt->desc[0], 4096);
		printk(KERN_DEBUG DRV_NAME "tents=%d\n", hwnents);
#if 0
		while (tables) {
			
			
			
		}
		put ourselves on wait queue
#endif

		dma_unmap_sg(NULL, ape->sgm->sgl, ape->sgm->mapped_pages, DMA_TO_DEVICE);
		
		sgm_unmap_user_pages(ape->sgm, 1);
		
		transfer_addr += transfer_len;
		remaining -= transfer_len;
		done += transfer_len;
	}
	return done;
}


static const struct file_operations sg_fops = {
	.owner = THIS_MODULE,
	.open = sg_open,
	.release = sg_close,
	.read = sg_read,
	.write = sg_write,
};


static int sg_init(struct ape_dev *ape)
{
	int rc;
	printk(KERN_DEBUG DRV_NAME " sg_init()\n");
	
	rc = alloc_chrdev_region(&ape->cdevno, 0, 1, DRV_NAME);
	
	if (rc < 0) {
		printk("alloc_chrdev_region() = %d\n", rc);
		goto fail_alloc;
	}
	
	cdev_init(&ape->cdev, &sg_fops);
	ape->cdev.owner = THIS_MODULE;
	
	rc = cdev_add(&ape->cdev, ape->cdevno, 1);
	if (rc < 0) {
		printk("cdev_add() = %d\n", rc);
		goto fail_add;
	}
	printk(KERN_DEBUG "altpciechdma = %d:%d\n", MAJOR(ape->cdevno), MINOR(ape->cdevno));
	return 0;
fail_add:
	
    unregister_chrdev_region(ape->cdevno, 1);
fail_alloc:
	return -1;
}



static void sg_exit(struct ape_dev *ape)
{
	printk(KERN_DEBUG DRV_NAME " sg_exit()\n");
	
	cdev_del(&ape->cdev);
	
	unregister_chrdev_region(ape->cdevno, 1);
}

#endif 


static struct pci_driver pci_driver = {
	.name = DRV_NAME,
	.id_table = ids,
	.probe = probe,
	.remove = __devexit_p(remove),
	
};


static int __init alterapciechdma_init(void)
{
	int rc = 0;
	printk(KERN_DEBUG DRV_NAME " init(), built at " __DATE__ " " __TIME__ "\n");
	
	rc = pci_register_driver(&pci_driver);
	if (rc < 0)
		return rc;
	return 0;
}


static void __exit alterapciechdma_exit(void)
{
	printk(KERN_DEBUG DRV_NAME " exit(), built at " __DATE__ " " __TIME__ "\n");
	
	pci_unregister_driver(&pci_driver);
}

MODULE_LICENSE("GPL");

module_init(alterapciechdma_init);
module_exit(alterapciechdma_exit);

