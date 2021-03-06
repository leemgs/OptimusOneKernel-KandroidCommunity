

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/blkdev.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <asm/dma.h>
#include <asm/irq.h>
#include <asm/sizes.h>
#include <mach/mmc.h>

#ifdef CONFIG_ARCH_MX2
#include <mach/dma-mx1-mx2.h>
#define HAS_DMA
#endif

#define DRIVER_NAME "mxc-mmc"

#define MMC_REG_STR_STP_CLK		0x00
#define MMC_REG_STATUS			0x04
#define MMC_REG_CLK_RATE		0x08
#define MMC_REG_CMD_DAT_CONT		0x0C
#define MMC_REG_RES_TO			0x10
#define MMC_REG_READ_TO			0x14
#define MMC_REG_BLK_LEN			0x18
#define MMC_REG_NOB			0x1C
#define MMC_REG_REV_NO			0x20
#define MMC_REG_INT_CNTR		0x24
#define MMC_REG_CMD			0x28
#define MMC_REG_ARG			0x2C
#define MMC_REG_RES_FIFO		0x34
#define MMC_REG_BUFFER_ACCESS		0x38

#define STR_STP_CLK_RESET               (1 << 3)
#define STR_STP_CLK_START_CLK           (1 << 1)
#define STR_STP_CLK_STOP_CLK            (1 << 0)

#define STATUS_CARD_INSERTION		(1 << 31)
#define STATUS_CARD_REMOVAL		(1 << 30)
#define STATUS_YBUF_EMPTY		(1 << 29)
#define STATUS_XBUF_EMPTY		(1 << 28)
#define STATUS_YBUF_FULL		(1 << 27)
#define STATUS_XBUF_FULL		(1 << 26)
#define STATUS_BUF_UND_RUN		(1 << 25)
#define STATUS_BUF_OVFL			(1 << 24)
#define STATUS_SDIO_INT_ACTIVE		(1 << 14)
#define STATUS_END_CMD_RESP		(1 << 13)
#define STATUS_WRITE_OP_DONE		(1 << 12)
#define STATUS_DATA_TRANS_DONE		(1 << 11)
#define STATUS_READ_OP_DONE		(1 << 11)
#define STATUS_WR_CRC_ERROR_CODE_MASK	(3 << 10)
#define STATUS_CARD_BUS_CLK_RUN		(1 << 8)
#define STATUS_BUF_READ_RDY		(1 << 7)
#define STATUS_BUF_WRITE_RDY		(1 << 6)
#define STATUS_RESP_CRC_ERR		(1 << 5)
#define STATUS_CRC_READ_ERR		(1 << 3)
#define STATUS_CRC_WRITE_ERR		(1 << 2)
#define STATUS_TIME_OUT_RESP		(1 << 1)
#define STATUS_TIME_OUT_READ		(1 << 0)
#define STATUS_ERR_MASK			0x2f

#define CMD_DAT_CONT_CMD_RESP_LONG_OFF	(1 << 12)
#define CMD_DAT_CONT_STOP_READWAIT	(1 << 11)
#define CMD_DAT_CONT_START_READWAIT	(1 << 10)
#define CMD_DAT_CONT_BUS_WIDTH_4	(2 << 8)
#define CMD_DAT_CONT_INIT		(1 << 7)
#define CMD_DAT_CONT_WRITE		(1 << 4)
#define CMD_DAT_CONT_DATA_ENABLE	(1 << 3)
#define CMD_DAT_CONT_RESPONSE_48BIT_CRC	(1 << 0)
#define CMD_DAT_CONT_RESPONSE_136BIT	(2 << 0)
#define CMD_DAT_CONT_RESPONSE_48BIT	(3 << 0)

#define INT_SDIO_INT_WKP_EN		(1 << 18)
#define INT_CARD_INSERTION_WKP_EN	(1 << 17)
#define INT_CARD_REMOVAL_WKP_EN		(1 << 16)
#define INT_CARD_INSERTION_EN		(1 << 15)
#define INT_CARD_REMOVAL_EN		(1 << 14)
#define INT_SDIO_IRQ_EN			(1 << 13)
#define INT_DAT0_EN			(1 << 12)
#define INT_BUF_READ_EN			(1 << 4)
#define INT_BUF_WRITE_EN		(1 << 3)
#define INT_END_CMD_RES_EN		(1 << 2)
#define INT_WRITE_OP_DONE_EN		(1 << 1)
#define INT_READ_OP_EN			(1 << 0)

struct mxcmci_host {
	struct mmc_host		*mmc;
	struct resource		*res;
	void __iomem		*base;
	int			irq;
	int			detect_irq;
	int			dma;
	int			do_dma;
	unsigned int		power_mode;
	struct imxmmc_platform_data *pdata;

	struct mmc_request	*req;
	struct mmc_command	*cmd;
	struct mmc_data		*data;

	unsigned int		dma_nents;
	unsigned int		datasize;
	unsigned int		dma_dir;

	u16			rev_no;
	unsigned int		cmdat;

	struct clk		*clk;

	int			clock;

	struct work_struct	datawork;
};

static void mxcmci_set_clk_rate(struct mxcmci_host *host, unsigned int clk_ios);

static inline int mxcmci_use_dma(struct mxcmci_host *host)
{
	return host->do_dma;
}

static void mxcmci_softreset(struct mxcmci_host *host)
{
	int i;

	
	writew(STR_STP_CLK_RESET, host->base + MMC_REG_STR_STP_CLK);
	writew(STR_STP_CLK_RESET | STR_STP_CLK_START_CLK,
			host->base + MMC_REG_STR_STP_CLK);

	for (i = 0; i < 8; i++)
		writew(STR_STP_CLK_START_CLK, host->base + MMC_REG_STR_STP_CLK);

	writew(0xff, host->base + MMC_REG_RES_TO);
}

static int mxcmci_setup_data(struct mxcmci_host *host, struct mmc_data *data)
{
	unsigned int nob = data->blocks;
	unsigned int blksz = data->blksz;
	unsigned int datasize = nob * blksz;
#ifdef HAS_DMA
	struct scatterlist *sg;
	int i;
	int ret;
#endif
	if (data->flags & MMC_DATA_STREAM)
		nob = 0xffff;

	host->data = data;
	data->bytes_xfered = 0;

	writew(nob, host->base + MMC_REG_NOB);
	writew(blksz, host->base + MMC_REG_BLK_LEN);
	host->datasize = datasize;

#ifdef HAS_DMA
	for_each_sg(data->sg, sg, data->sg_len, i) {
		if (sg->offset & 3 || sg->length & 3) {
			host->do_dma = 0;
			return 0;
		}
	}

	if (data->flags & MMC_DATA_READ) {
		host->dma_dir = DMA_FROM_DEVICE;
		host->dma_nents = dma_map_sg(mmc_dev(host->mmc), data->sg,
					     data->sg_len,  host->dma_dir);

		ret = imx_dma_setup_sg(host->dma, data->sg, host->dma_nents,
				datasize,
				host->res->start + MMC_REG_BUFFER_ACCESS,
				DMA_MODE_READ);
	} else {
		host->dma_dir = DMA_TO_DEVICE;
		host->dma_nents = dma_map_sg(mmc_dev(host->mmc), data->sg,
					     data->sg_len,  host->dma_dir);

		ret = imx_dma_setup_sg(host->dma, data->sg, host->dma_nents,
				datasize,
				host->res->start + MMC_REG_BUFFER_ACCESS,
				DMA_MODE_WRITE);
	}

	if (ret) {
		dev_err(mmc_dev(host->mmc), "failed to setup DMA : %d\n", ret);
		return ret;
	}
	wmb();

	imx_dma_enable(host->dma);
#endif 
	return 0;
}

static int mxcmci_start_cmd(struct mxcmci_host *host, struct mmc_command *cmd,
		unsigned int cmdat)
{
	WARN_ON(host->cmd != NULL);
	host->cmd = cmd;

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1: 
	case MMC_RSP_R1B:
		cmdat |= CMD_DAT_CONT_RESPONSE_48BIT_CRC;
		break;
	case MMC_RSP_R2: 
		cmdat |= CMD_DAT_CONT_RESPONSE_136BIT;
		break;
	case MMC_RSP_R3: 
		cmdat |= CMD_DAT_CONT_RESPONSE_48BIT;
		break;
	case MMC_RSP_NONE:
		break;
	default:
		dev_err(mmc_dev(host->mmc), "unhandled response type 0x%x\n",
				mmc_resp_type(cmd));
		cmd->error = -EINVAL;
		return -EINVAL;
	}

	if (mxcmci_use_dma(host))
		writel(INT_READ_OP_EN | INT_WRITE_OP_DONE_EN |
				INT_END_CMD_RES_EN,
				host->base + MMC_REG_INT_CNTR);
	else
		writel(INT_END_CMD_RES_EN, host->base + MMC_REG_INT_CNTR);

	writew(cmd->opcode, host->base + MMC_REG_CMD);
	writel(cmd->arg, host->base + MMC_REG_ARG);
	writew(cmdat, host->base + MMC_REG_CMD_DAT_CONT);

	return 0;
}

static void mxcmci_finish_request(struct mxcmci_host *host,
		struct mmc_request *req)
{
	writel(0, host->base + MMC_REG_INT_CNTR);

	host->req = NULL;
	host->cmd = NULL;
	host->data = NULL;

	mmc_request_done(host->mmc, req);
}

static int mxcmci_finish_data(struct mxcmci_host *host, unsigned int stat)
{
	struct mmc_data *data = host->data;
	int data_error;

#ifdef HAS_DMA
	if (mxcmci_use_dma(host)) {
		imx_dma_disable(host->dma);
		dma_unmap_sg(mmc_dev(host->mmc), data->sg, host->dma_nents,
				host->dma_dir);
	}
#endif

	if (stat & STATUS_ERR_MASK) {
		dev_dbg(mmc_dev(host->mmc), "request failed. status: 0x%08x\n",
				stat);
		if (stat & STATUS_CRC_READ_ERR) {
			data->error = -EILSEQ;
		} else if (stat & STATUS_CRC_WRITE_ERR) {
			u32 err_code = (stat >> 9) & 0x3;
			if (err_code == 2) 
				data->error = -ETIMEDOUT;
			else
				data->error = -EILSEQ;
		} else if (stat & STATUS_TIME_OUT_READ) {
			data->error = -ETIMEDOUT;
		} else {
			data->error = -EIO;
		}
	} else {
		data->bytes_xfered = host->datasize;
	}

	data_error = data->error;

	host->data = NULL;

	return data_error;
}

static void mxcmci_read_response(struct mxcmci_host *host, unsigned int stat)
{
	struct mmc_command *cmd = host->cmd;
	int i;
	u32 a, b, c;

	if (!cmd)
		return;

	if (stat & STATUS_TIME_OUT_RESP) {
		dev_dbg(mmc_dev(host->mmc), "CMD TIMEOUT\n");
		cmd->error = -ETIMEDOUT;
	} else if (stat & STATUS_RESP_CRC_ERR && cmd->flags & MMC_RSP_CRC) {
		dev_dbg(mmc_dev(host->mmc), "cmd crc error\n");
		cmd->error = -EILSEQ;
	}

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136) {
			for (i = 0; i < 4; i++) {
				a = readw(host->base + MMC_REG_RES_FIFO);
				b = readw(host->base + MMC_REG_RES_FIFO);
				cmd->resp[i] = a << 16 | b;
			}
		} else {
			a = readw(host->base + MMC_REG_RES_FIFO);
			b = readw(host->base + MMC_REG_RES_FIFO);
			c = readw(host->base + MMC_REG_RES_FIFO);
			cmd->resp[0] = a << 24 | b << 8 | c >> 8;
		}
	}
}

static int mxcmci_poll_status(struct mxcmci_host *host, u32 mask)
{
	u32 stat;
	unsigned long timeout = jiffies + HZ;

	do {
		stat = readl(host->base + MMC_REG_STATUS);
		if (stat & STATUS_ERR_MASK)
			return stat;
		if (time_after(jiffies, timeout)) {
			mxcmci_softreset(host);
			mxcmci_set_clk_rate(host, host->clock);
			return STATUS_TIME_OUT_READ;
		}
		if (stat & mask)
			return 0;
		cpu_relax();
	} while (1);
}

static int mxcmci_pull(struct mxcmci_host *host, void *_buf, int bytes)
{
	unsigned int stat;
	u32 *buf = _buf;

	while (bytes > 3) {
		stat = mxcmci_poll_status(host,
				STATUS_BUF_READ_RDY | STATUS_READ_OP_DONE);
		if (stat)
			return stat;
		*buf++ = readl(host->base + MMC_REG_BUFFER_ACCESS);
		bytes -= 4;
	}

	if (bytes) {
		u8 *b = (u8 *)buf;
		u32 tmp;

		stat = mxcmci_poll_status(host,
				STATUS_BUF_READ_RDY | STATUS_READ_OP_DONE);
		if (stat)
			return stat;
		tmp = readl(host->base + MMC_REG_BUFFER_ACCESS);
		memcpy(b, &tmp, bytes);
	}

	return 0;
}

static int mxcmci_push(struct mxcmci_host *host, void *_buf, int bytes)
{
	unsigned int stat;
	u32 *buf = _buf;

	while (bytes > 3) {
		stat = mxcmci_poll_status(host, STATUS_BUF_WRITE_RDY);
		if (stat)
			return stat;
		writel(*buf++, host->base + MMC_REG_BUFFER_ACCESS);
		bytes -= 4;
	}

	if (bytes) {
		u8 *b = (u8 *)buf;
		u32 tmp;

		stat = mxcmci_poll_status(host, STATUS_BUF_WRITE_RDY);
		if (stat)
			return stat;

		memcpy(&tmp, b, bytes);
		writel(tmp, host->base + MMC_REG_BUFFER_ACCESS);
	}

	stat = mxcmci_poll_status(host, STATUS_BUF_WRITE_RDY);
	if (stat)
		return stat;

	return 0;
}

static int mxcmci_transfer_data(struct mxcmci_host *host)
{
	struct mmc_data *data = host->req->data;
	struct scatterlist *sg;
	int stat, i;

	host->datasize = 0;

	host->data = data;
	host->datasize = 0;

	if (data->flags & MMC_DATA_READ) {
		for_each_sg(data->sg, sg, data->sg_len, i) {
			stat = mxcmci_pull(host, sg_virt(sg), sg->length);
			if (stat)
				return stat;
			host->datasize += sg->length;
		}
	} else {
		for_each_sg(data->sg, sg, data->sg_len, i) {
			stat = mxcmci_push(host, sg_virt(sg), sg->length);
			if (stat)
				return stat;
			host->datasize += sg->length;
		}
		stat = mxcmci_poll_status(host, STATUS_WRITE_OP_DONE);
		if (stat)
			return stat;
	}
	return 0;
}

static void mxcmci_datawork(struct work_struct *work)
{
	struct mxcmci_host *host = container_of(work, struct mxcmci_host,
						  datawork);
	int datastat = mxcmci_transfer_data(host);
	mxcmci_finish_data(host, datastat);

	if (host->req->stop) {
		if (mxcmci_start_cmd(host, host->req->stop, 0)) {
			mxcmci_finish_request(host, host->req);
			return;
		}
	} else {
		mxcmci_finish_request(host, host->req);
	}
}

#ifdef HAS_DMA
static void mxcmci_data_done(struct mxcmci_host *host, unsigned int stat)
{
	struct mmc_data *data = host->data;
	int data_error;

	if (!data)
		return;

	data_error = mxcmci_finish_data(host, stat);

	mxcmci_read_response(host, stat);
	host->cmd = NULL;

	if (host->req->stop) {
		if (mxcmci_start_cmd(host, host->req->stop, 0)) {
			mxcmci_finish_request(host, host->req);
			return;
		}
	} else {
		mxcmci_finish_request(host, host->req);
	}
}
#endif 

static void mxcmci_cmd_done(struct mxcmci_host *host, unsigned int stat)
{
	mxcmci_read_response(host, stat);
	host->cmd = NULL;

	if (!host->data && host->req) {
		mxcmci_finish_request(host, host->req);
		return;
	}

	
	if (!mxcmci_use_dma(host) && host->data)
		schedule_work(&host->datawork);

}

static irqreturn_t mxcmci_irq(int irq, void *devid)
{
	struct mxcmci_host *host = devid;
	u32 stat;

	stat = readl(host->base + MMC_REG_STATUS);
	writel(stat, host->base + MMC_REG_STATUS);

	dev_dbg(mmc_dev(host->mmc), "%s: 0x%08x\n", __func__, stat);

	if (stat & STATUS_END_CMD_RESP)
		mxcmci_cmd_done(host, stat);
#ifdef HAS_DMA
	if (mxcmci_use_dma(host) &&
		  (stat & (STATUS_DATA_TRANS_DONE | STATUS_WRITE_OP_DONE)))
		mxcmci_data_done(host, stat);
#endif
	return IRQ_HANDLED;
}

static void mxcmci_request(struct mmc_host *mmc, struct mmc_request *req)
{
	struct mxcmci_host *host = mmc_priv(mmc);
	unsigned int cmdat = host->cmdat;
	int error;

	WARN_ON(host->req != NULL);

	host->req = req;
	host->cmdat &= ~CMD_DAT_CONT_INIT;
#ifdef HAS_DMA
	host->do_dma = 1;
#endif
	if (req->data) {
		error = mxcmci_setup_data(host, req->data);
		if (error) {
			req->cmd->error = error;
			goto out;
		}


		cmdat |= CMD_DAT_CONT_DATA_ENABLE;

		if (req->data->flags & MMC_DATA_WRITE)
			cmdat |= CMD_DAT_CONT_WRITE;
	}

	error = mxcmci_start_cmd(host, req->cmd, cmdat);
out:
	if (error)
		mxcmci_finish_request(host, req);
}

static void mxcmci_set_clk_rate(struct mxcmci_host *host, unsigned int clk_ios)
{
	unsigned int divider;
	int prescaler = 0;
	unsigned int clk_in = clk_get_rate(host->clk);

	while (prescaler <= 0x800) {
		for (divider = 1; divider <= 0xF; divider++) {
			int x;

			x = (clk_in / (divider + 1));

			if (prescaler)
				x /= (prescaler * 2);

			if (x <= clk_ios)
				break;
		}
		if (divider < 0x10)
			break;

		if (prescaler == 0)
			prescaler = 1;
		else
			prescaler <<= 1;
	}

	writew((prescaler << 4) | divider, host->base + MMC_REG_CLK_RATE);

	dev_dbg(mmc_dev(host->mmc), "scaler: %d divider: %d in: %d out: %d\n",
			prescaler, divider, clk_in, clk_ios);
}

static void mxcmci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct mxcmci_host *host = mmc_priv(mmc);
#ifdef HAS_DMA
	unsigned int blen;
	
	if (ios->bus_width == MMC_BUS_WIDTH_4)
		blen = 0;
	else
		blen = 16;

	imx_dma_config_burstlen(host->dma, blen);
#endif
	if (ios->bus_width == MMC_BUS_WIDTH_4)
		host->cmdat |= CMD_DAT_CONT_BUS_WIDTH_4;
	else
		host->cmdat &= ~CMD_DAT_CONT_BUS_WIDTH_4;

	if (host->power_mode != ios->power_mode) {
		if (host->pdata && host->pdata->setpower)
			host->pdata->setpower(mmc_dev(mmc), ios->vdd);
		host->power_mode = ios->power_mode;
		if (ios->power_mode == MMC_POWER_ON)
			host->cmdat |= CMD_DAT_CONT_INIT;
	}

	if (ios->clock) {
		mxcmci_set_clk_rate(host, ios->clock);
		writew(STR_STP_CLK_START_CLK, host->base + MMC_REG_STR_STP_CLK);
	} else {
		writew(STR_STP_CLK_STOP_CLK, host->base + MMC_REG_STR_STP_CLK);
	}

	host->clock = ios->clock;
}

static irqreturn_t mxcmci_detect_irq(int irq, void *data)
{
	struct mmc_host *mmc = data;

	dev_dbg(mmc_dev(mmc), "%s\n", __func__);

	mmc_detect_change(mmc, msecs_to_jiffies(250));
	return IRQ_HANDLED;
}

static int mxcmci_get_ro(struct mmc_host *mmc)
{
	struct mxcmci_host *host = mmc_priv(mmc);

	if (host->pdata && host->pdata->get_ro)
		return !!host->pdata->get_ro(mmc_dev(mmc));
	
	return -ENOSYS;
}


static const struct mmc_host_ops mxcmci_ops = {
	.request	= mxcmci_request,
	.set_ios	= mxcmci_set_ios,
	.get_ro		= mxcmci_get_ro,
};

static int mxcmci_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct mxcmci_host *host = NULL;
	struct resource *r;
	int ret = 0, irq;

	printk(KERN_INFO "i.MX SDHC driver\n");

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!r || irq < 0)
		return -EINVAL;

	r = request_mem_region(r->start, resource_size(r), pdev->name);
	if (!r)
		return -EBUSY;

	mmc = mmc_alloc_host(sizeof(struct mxcmci_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		goto out_release_mem;
	}

	mmc->ops = &mxcmci_ops;
	mmc->caps = MMC_CAP_4_BIT_DATA;

	
	mmc->max_hw_segs = 64;
	mmc->max_phys_segs = 64;
	mmc->max_blk_size = 2048;
	mmc->max_blk_count = 65535;
	mmc->max_req_size = mmc->max_blk_size * mmc->max_blk_count;
	mmc->max_seg_size = mmc->max_seg_size;

	host = mmc_priv(mmc);
	host->base = ioremap(r->start, resource_size(r));
	if (!host->base) {
		ret = -ENOMEM;
		goto out_free;
	}

	host->mmc = mmc;
	host->pdata = pdev->dev.platform_data;

	if (host->pdata && host->pdata->ocr_avail)
		mmc->ocr_avail = host->pdata->ocr_avail;
	else
		mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;

	host->res = r;
	host->irq = irq;

	host->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(host->clk)) {
		ret = PTR_ERR(host->clk);
		goto out_iounmap;
	}
	clk_enable(host->clk);

	mxcmci_softreset(host);

	host->rev_no = readw(host->base + MMC_REG_REV_NO);
	if (host->rev_no != 0x400) {
		ret = -ENODEV;
		dev_err(mmc_dev(host->mmc), "wrong rev.no. 0x%08x. aborting.\n",
			host->rev_no);
		goto out_clk_put;
	}

	mmc->f_min = clk_get_rate(host->clk) >> 16;
	mmc->f_max = clk_get_rate(host->clk) >> 1;

	
	writew(0x2db4, host->base + MMC_REG_READ_TO);

	writel(0, host->base + MMC_REG_INT_CNTR);

#ifdef HAS_DMA
	host->dma = imx_dma_request_by_prio(DRIVER_NAME, DMA_PRIO_LOW);
	if (host->dma < 0) {
		dev_err(mmc_dev(host->mmc), "imx_dma_request_by_prio failed\n");
		ret = -EBUSY;
		goto out_clk_put;
	}

	r = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (!r) {
		ret = -EINVAL;
		goto out_free_dma;
	}

	ret = imx_dma_config_channel(host->dma,
				     IMX_DMA_MEMSIZE_32 | IMX_DMA_TYPE_FIFO,
				     IMX_DMA_MEMSIZE_32 | IMX_DMA_TYPE_LINEAR,
				     r->start, 0);
	if (ret) {
		dev_err(mmc_dev(host->mmc), "failed to config DMA channel\n");
		goto out_free_dma;
	}
#endif
	INIT_WORK(&host->datawork, mxcmci_datawork);

	ret = request_irq(host->irq, mxcmci_irq, 0, DRIVER_NAME, host);
	if (ret)
		goto out_free_dma;

	platform_set_drvdata(pdev, mmc);

	if (host->pdata && host->pdata->init) {
		ret = host->pdata->init(&pdev->dev, mxcmci_detect_irq,
				host->mmc);
		if (ret)
			goto out_free_irq;
	}

	mmc_add_host(mmc);

	return 0;

out_free_irq:
	free_irq(host->irq, host);
out_free_dma:
#ifdef HAS_DMA
	imx_dma_free(host->dma);
#endif
out_clk_put:
	clk_disable(host->clk);
	clk_put(host->clk);
out_iounmap:
	iounmap(host->base);
out_free:
	mmc_free_host(mmc);
out_release_mem:
	release_mem_region(host->res->start, resource_size(host->res));
	return ret;
}

static int mxcmci_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct mxcmci_host *host = mmc_priv(mmc);

	platform_set_drvdata(pdev, NULL);

	mmc_remove_host(mmc);

	if (host->pdata && host->pdata->exit)
		host->pdata->exit(&pdev->dev, mmc);

	free_irq(host->irq, host);
	iounmap(host->base);
#ifdef HAS_DMA
	imx_dma_free(host->dma);
#endif
	clk_disable(host->clk);
	clk_put(host->clk);

	release_mem_region(host->res->start, resource_size(host->res));
	release_resource(host->res);

	mmc_free_host(mmc);

	return 0;
}

#ifdef CONFIG_PM
static int mxcmci_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	if (mmc)
		ret = mmc_suspend_host(mmc, state);

	return ret;
}

static int mxcmci_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	struct mxcmci_host *host;
	int ret = 0;

	if (mmc) {
		host = mmc_priv(mmc);
		ret = mmc_resume_host(mmc);
	}

	return ret;
}
#else
#define mxcmci_suspend  NULL
#define mxcmci_resume   NULL
#endif 

static struct platform_driver mxcmci_driver = {
	.probe		= mxcmci_probe,
	.remove		= mxcmci_remove,
	.suspend	= mxcmci_suspend,
	.resume		= mxcmci_resume,
	.driver		= {
		.name		= DRIVER_NAME,
		.owner		= THIS_MODULE,
	}
};

static int __init mxcmci_init(void)
{
	return platform_driver_register(&mxcmci_driver);
}

static void __exit mxcmci_exit(void)
{
	platform_driver_unregister(&mxcmci_driver);
}

module_init(mxcmci_init);
module_exit(mxcmci_exit);

MODULE_DESCRIPTION("i.MX Multimedia Card Interface Driver");
MODULE_AUTHOR("Sascha Hauer, Pengutronix");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imx-mmc");
