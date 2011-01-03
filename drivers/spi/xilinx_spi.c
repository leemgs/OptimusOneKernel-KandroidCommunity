

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_spi.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/io.h>

#define XILINX_SPI_NAME "xilinx_spi"


#define XSPI_CR_OFFSET		0x62	

#define XSPI_CR_ENABLE		0x02
#define XSPI_CR_MASTER_MODE	0x04
#define XSPI_CR_CPOL		0x08
#define XSPI_CR_CPHA		0x10
#define XSPI_CR_MODE_MASK	(XSPI_CR_CPHA | XSPI_CR_CPOL)
#define XSPI_CR_TXFIFO_RESET	0x20
#define XSPI_CR_RXFIFO_RESET	0x40
#define XSPI_CR_MANUAL_SSELECT	0x80
#define XSPI_CR_TRANS_INHIBIT	0x100

#define XSPI_SR_OFFSET		0x67	

#define XSPI_SR_RX_EMPTY_MASK	0x01	
#define XSPI_SR_RX_FULL_MASK	0x02	
#define XSPI_SR_TX_EMPTY_MASK	0x04	
#define XSPI_SR_TX_FULL_MASK	0x08	
#define XSPI_SR_MODE_FAULT_MASK	0x10	

#define XSPI_TXD_OFFSET		0x6b	
#define XSPI_RXD_OFFSET		0x6f	

#define XSPI_SSR_OFFSET		0x70	


#define XIPIF_V123B_DGIER_OFFSET	0x1c	
#define XIPIF_V123B_GINTR_ENABLE	0x80000000

#define XIPIF_V123B_IISR_OFFSET		0x20	
#define XIPIF_V123B_IIER_OFFSET		0x28	

#define XSPI_INTR_MODE_FAULT		0x01	
#define XSPI_INTR_SLAVE_MODE_FAULT	0x02	
#define XSPI_INTR_TX_EMPTY		0x04	
#define XSPI_INTR_TX_UNDERRUN		0x08	
#define XSPI_INTR_RX_FULL		0x10	
#define XSPI_INTR_RX_OVERRUN		0x20	

#define XIPIF_V123B_RESETR_OFFSET	0x40	
#define XIPIF_V123B_RESET_MASK		0x0a	

struct xilinx_spi {
	
	struct spi_bitbang bitbang;
	struct completion done;

	void __iomem	*regs;	

	u32		irq;

	u32		speed_hz; 

	u8 *rx_ptr;		
	const u8 *tx_ptr;	
	int remaining_bytes;	
};

static void xspi_init_hw(void __iomem *regs_base)
{
	
	out_be32(regs_base + XIPIF_V123B_RESETR_OFFSET,
		 XIPIF_V123B_RESET_MASK);
	
	out_be32(regs_base + XIPIF_V123B_IIER_OFFSET, 0);
	
	out_be32(regs_base + XIPIF_V123B_DGIER_OFFSET,
		 XIPIF_V123B_GINTR_ENABLE);
	
	out_be32(regs_base + XSPI_SSR_OFFSET, 0xffff);
	
	out_be16(regs_base + XSPI_CR_OFFSET,
		 XSPI_CR_TRANS_INHIBIT | XSPI_CR_MANUAL_SSELECT
		 | XSPI_CR_MASTER_MODE | XSPI_CR_ENABLE);
}

static void xilinx_spi_chipselect(struct spi_device *spi, int is_on)
{
	struct xilinx_spi *xspi = spi_master_get_devdata(spi->master);

	if (is_on == BITBANG_CS_INACTIVE) {
		
		out_be32(xspi->regs + XSPI_SSR_OFFSET, 0xffff);
	} else if (is_on == BITBANG_CS_ACTIVE) {
		
		u16 cr = in_be16(xspi->regs + XSPI_CR_OFFSET)
			 & ~XSPI_CR_MODE_MASK;
		if (spi->mode & SPI_CPHA)
			cr |= XSPI_CR_CPHA;
		if (spi->mode & SPI_CPOL)
			cr |= XSPI_CR_CPOL;
		out_be16(xspi->regs + XSPI_CR_OFFSET, cr);

		

		
		out_be32(xspi->regs + XSPI_SSR_OFFSET,
			 ~(0x0001 << spi->chip_select));
	}
}


static int xilinx_spi_setup_transfer(struct spi_device *spi,
		struct spi_transfer *t)
{
	u8 bits_per_word;

	bits_per_word = (t) ? t->bits_per_word : spi->bits_per_word;
	if (bits_per_word != 8) {
		dev_err(&spi->dev, "%s, unsupported bits_per_word=%d\n",
			__func__, bits_per_word);
		return -EINVAL;
	}

	return 0;
}

static int xilinx_spi_setup(struct spi_device *spi)
{
	struct spi_bitbang *bitbang;
	struct xilinx_spi *xspi;
	int retval;

	xspi = spi_master_get_devdata(spi->master);
	bitbang = &xspi->bitbang;

	retval = xilinx_spi_setup_transfer(spi, NULL);
	if (retval < 0)
		return retval;

	return 0;
}

static void xilinx_spi_fill_tx_fifo(struct xilinx_spi *xspi)
{
	u8 sr;

	
	sr = in_8(xspi->regs + XSPI_SR_OFFSET);
	while ((sr & XSPI_SR_TX_FULL_MASK) == 0 && xspi->remaining_bytes > 0) {
		if (xspi->tx_ptr) {
			out_8(xspi->regs + XSPI_TXD_OFFSET, *xspi->tx_ptr++);
		} else {
			out_8(xspi->regs + XSPI_TXD_OFFSET, 0);
		}
		xspi->remaining_bytes--;
		sr = in_8(xspi->regs + XSPI_SR_OFFSET);
	}
}

static int xilinx_spi_txrx_bufs(struct spi_device *spi, struct spi_transfer *t)
{
	struct xilinx_spi *xspi = spi_master_get_devdata(spi->master);
	u32 ipif_ier;
	u16 cr;

	

	xspi->tx_ptr = t->tx_buf;
	xspi->rx_ptr = t->rx_buf;
	xspi->remaining_bytes = t->len;
	INIT_COMPLETION(xspi->done);

	xilinx_spi_fill_tx_fifo(xspi);

	
	ipif_ier = in_be32(xspi->regs + XIPIF_V123B_IIER_OFFSET);
	out_be32(xspi->regs + XIPIF_V123B_IIER_OFFSET,
		 ipif_ier | XSPI_INTR_TX_EMPTY);

	
	cr = in_be16(xspi->regs + XSPI_CR_OFFSET) & ~XSPI_CR_TRANS_INHIBIT;
	out_be16(xspi->regs + XSPI_CR_OFFSET, cr);

	wait_for_completion(&xspi->done);

	
	out_be32(xspi->regs + XIPIF_V123B_IIER_OFFSET, ipif_ier);

	return t->len - xspi->remaining_bytes;
}



static irqreturn_t xilinx_spi_irq(int irq, void *dev_id)
{
	struct xilinx_spi *xspi = dev_id;
	u32 ipif_isr;

	
	ipif_isr = in_be32(xspi->regs + XIPIF_V123B_IISR_OFFSET);
	out_be32(xspi->regs + XIPIF_V123B_IISR_OFFSET, ipif_isr);

	if (ipif_isr & XSPI_INTR_TX_EMPTY) {	
		u16 cr;
		u8 sr;

		
		cr = in_be16(xspi->regs + XSPI_CR_OFFSET);
		out_be16(xspi->regs + XSPI_CR_OFFSET,
			 cr | XSPI_CR_TRANS_INHIBIT);

		
		sr = in_8(xspi->regs + XSPI_SR_OFFSET);
		while ((sr & XSPI_SR_RX_EMPTY_MASK) == 0) {
			u8 data;

			data = in_8(xspi->regs + XSPI_RXD_OFFSET);
			if (xspi->rx_ptr) {
				*xspi->rx_ptr++ = data;
			}
			sr = in_8(xspi->regs + XSPI_SR_OFFSET);
		}

		
		if (xspi->remaining_bytes > 0) {
			xilinx_spi_fill_tx_fifo(xspi);
			
			out_be16(xspi->regs + XSPI_CR_OFFSET, cr);
		} else {
			
			complete(&xspi->done);
		}
	}

	return IRQ_HANDLED;
}

static int __init xilinx_spi_of_probe(struct of_device *ofdev,
					const struct of_device_id *match)
{
	struct spi_master *master;
	struct xilinx_spi *xspi;
	struct resource r_irq_struct;
	struct resource r_mem_struct;

	struct resource *r_irq = &r_irq_struct;
	struct resource *r_mem = &r_mem_struct;
	int rc = 0;
	const u32 *prop;
	int len;

	
	master = spi_alloc_master(&ofdev->dev, sizeof(struct xilinx_spi));

	if (master == NULL) {
		return -ENOMEM;
	}

	dev_set_drvdata(&ofdev->dev, master);

	rc = of_address_to_resource(ofdev->node, 0, r_mem);
	if (rc) {
		dev_warn(&ofdev->dev, "invalid address\n");
		goto put_master;
	}

	rc = of_irq_to_resource(ofdev->node, 0, r_irq);
	if (rc == NO_IRQ) {
		dev_warn(&ofdev->dev, "no IRQ found\n");
		goto put_master;
	}

	
	master->mode_bits = SPI_CPOL | SPI_CPHA;

	xspi = spi_master_get_devdata(master);
	xspi->bitbang.master = spi_master_get(master);
	xspi->bitbang.chipselect = xilinx_spi_chipselect;
	xspi->bitbang.setup_transfer = xilinx_spi_setup_transfer;
	xspi->bitbang.txrx_bufs = xilinx_spi_txrx_bufs;
	xspi->bitbang.master->setup = xilinx_spi_setup;
	init_completion(&xspi->done);

	xspi->irq = r_irq->start;

	if (!request_mem_region(r_mem->start,
			r_mem->end - r_mem->start + 1, XILINX_SPI_NAME)) {
		rc = -ENXIO;
		dev_warn(&ofdev->dev, "memory request failure\n");
		goto put_master;
	}

	xspi->regs = ioremap(r_mem->start, r_mem->end - r_mem->start + 1);
	if (xspi->regs == NULL) {
		rc = -ENOMEM;
		dev_warn(&ofdev->dev, "ioremap failure\n");
		goto release_mem;
	}
	xspi->irq = r_irq->start;

	
	master->bus_num = -1;

	
	prop = of_get_property(ofdev->node, "xlnx,num-ss-bits", &len);
	if (!prop || len < sizeof(*prop)) {
		dev_warn(&ofdev->dev, "no 'xlnx,num-ss-bits' property\n");
		goto unmap_io;
	}
	master->num_chipselect = *prop;

	
	xspi_init_hw(xspi->regs);

	
	rc = request_irq(xspi->irq, xilinx_spi_irq, 0, XILINX_SPI_NAME, xspi);
	if (rc != 0) {
		dev_warn(&ofdev->dev, "irq request failure: %d\n", xspi->irq);
		goto unmap_io;
	}

	rc = spi_bitbang_start(&xspi->bitbang);
	if (rc != 0) {
		dev_err(&ofdev->dev, "spi_bitbang_start FAILED\n");
		goto free_irq;
	}

	dev_info(&ofdev->dev, "at 0x%08X mapped to 0x%08X, irq=%d\n",
			(unsigned int)r_mem->start, (u32)xspi->regs, xspi->irq);

	
	of_register_spi_devices(master, ofdev->node);

	return rc;

free_irq:
	free_irq(xspi->irq, xspi);
unmap_io:
	iounmap(xspi->regs);
release_mem:
	release_mem_region(r_mem->start, resource_size(r_mem));
put_master:
	spi_master_put(master);
	return rc;
}

static int __devexit xilinx_spi_remove(struct of_device *ofdev)
{
	struct xilinx_spi *xspi;
	struct spi_master *master;
	struct resource r_mem;

	master = platform_get_drvdata(ofdev);
	xspi = spi_master_get_devdata(master);

	spi_bitbang_stop(&xspi->bitbang);
	free_irq(xspi->irq, xspi);
	iounmap(xspi->regs);
	if (!of_address_to_resource(ofdev->node, 0, &r_mem))
		release_mem_region(r_mem.start, resource_size(&r_mem));
	dev_set_drvdata(&ofdev->dev, 0);
	spi_master_put(xspi->bitbang.master);

	return 0;
}


MODULE_ALIAS("platform:" XILINX_SPI_NAME);

static int __exit xilinx_spi_of_remove(struct of_device *op)
{
	return xilinx_spi_remove(op);
}

static struct of_device_id xilinx_spi_of_match[] = {
	{ .compatible = "xlnx,xps-spi-2.00.a", },
	{ .compatible = "xlnx,xps-spi-2.00.b", },
	{}
};

MODULE_DEVICE_TABLE(of, xilinx_spi_of_match);

static struct of_platform_driver xilinx_spi_of_driver = {
	.owner = THIS_MODULE,
	.name = "xilinx-xps-spi",
	.match_table = xilinx_spi_of_match,
	.probe = xilinx_spi_of_probe,
	.remove = __exit_p(xilinx_spi_of_remove),
	.driver = {
		.name = "xilinx-xps-spi",
		.owner = THIS_MODULE,
	},
};

static int __init xilinx_spi_init(void)
{
	return of_register_platform_driver(&xilinx_spi_of_driver);
}
module_init(xilinx_spi_init);

static void __exit xilinx_spi_exit(void)
{
	of_unregister_platform_driver(&xilinx_spi_of_driver);
}
module_exit(xilinx_spi_exit);
MODULE_AUTHOR("MontaVista Software, Inc. <source@mvista.com>");
MODULE_DESCRIPTION("Xilinx SPI driver");
MODULE_LICENSE("GPL");
