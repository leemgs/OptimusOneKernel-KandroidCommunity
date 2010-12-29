
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mfd/core.h>
#include <linux/mfd/tmio.h>

#include "tmio_mmc.h"

static void tmio_mmc_set_clock(struct tmio_mmc_host *host, int new_clock)
{
	u32 clk = 0, clock;

	if (new_clock) {
		for (clock = host->mmc->f_min, clk = 0x80000080;
			new_clock >= (clock<<1); clk >>= 1)
			clock <<= 1;
		clk |= 0x100;
	}

	sd_config_write8(host, CNF_SD_CLK_MODE, clk >> 22);
	sd_ctrl_write16(host, CTL_SD_CARD_CLK_CTL, clk & 0x1ff);
}

static void tmio_mmc_clk_stop(struct tmio_mmc_host *host)
{
	sd_ctrl_write16(host, CTL_CLK_AND_WAIT_CTL, 0x0000);
	msleep(10);
	sd_ctrl_write16(host, CTL_SD_CARD_CLK_CTL, ~0x0100 &
		sd_ctrl_read16(host, CTL_SD_CARD_CLK_CTL));
	msleep(10);
}

static void tmio_mmc_clk_start(struct tmio_mmc_host *host)
{
	sd_ctrl_write16(host, CTL_SD_CARD_CLK_CTL, 0x0100 |
		sd_ctrl_read16(host, CTL_SD_CARD_CLK_CTL));
	msleep(10);
	sd_ctrl_write16(host, CTL_CLK_AND_WAIT_CTL, 0x0100);
	msleep(10);
}

static void reset(struct tmio_mmc_host *host)
{
	
	sd_ctrl_write16(host, CTL_RESET_SD, 0x0000);
	sd_ctrl_write16(host, CTL_RESET_SDIO, 0x0000);
	msleep(10);
	sd_ctrl_write16(host, CTL_RESET_SD, 0x0001);
	sd_ctrl_write16(host, CTL_RESET_SDIO, 0x0001);
	msleep(10);
}

static void
tmio_mmc_finish_request(struct tmio_mmc_host *host)
{
	struct mmc_request *mrq = host->mrq;

	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;

	mmc_request_done(host->mmc, mrq);
}


#define APP_CMD        0x0040
#define RESP_NONE      0x0300
#define RESP_R1        0x0400
#define RESP_R1B       0x0500
#define RESP_R2        0x0600
#define RESP_R3        0x0700
#define DATA_PRESENT   0x0800
#define TRANSFER_READ  0x1000
#define TRANSFER_MULTI 0x2000
#define SECURITY_CMD   0x4000

static int
tmio_mmc_start_command(struct tmio_mmc_host *host, struct mmc_command *cmd)
{
	struct mmc_data *data = host->data;
	int c = cmd->opcode;

	
	if (cmd->opcode == 12 && !cmd->arg) {
		sd_ctrl_write16(host, CTL_STOP_INTERNAL_ACTION, 0x001);
		return 0;
	}

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_NONE: c |= RESP_NONE; break;
	case MMC_RSP_R1:   c |= RESP_R1;   break;
	case MMC_RSP_R1B:  c |= RESP_R1B;  break;
	case MMC_RSP_R2:   c |= RESP_R2;   break;
	case MMC_RSP_R3:   c |= RESP_R3;   break;
	default:
		pr_debug("Unknown response type %d\n", mmc_resp_type(cmd));
		return -EINVAL;
	}

	host->cmd = cmd;


	if (data) {
		c |= DATA_PRESENT;
		if (data->blocks > 1) {
			sd_ctrl_write16(host, CTL_STOP_INTERNAL_ACTION, 0x100);
			c |= TRANSFER_MULTI;
		}
		if (data->flags & MMC_DATA_READ)
			c |= TRANSFER_READ;
	}

	enable_mmc_irqs(host, TMIO_MASK_CMD);

	
	sd_ctrl_write32(host, CTL_ARG_REG, cmd->arg);
	sd_ctrl_write16(host, CTL_SD_CMD, c);

	return 0;
}


static inline void tmio_mmc_pio_irq(struct tmio_mmc_host *host)
{
	struct mmc_data *data = host->data;
	unsigned short *buf;
	unsigned int count;
	unsigned long flags;

	if (!data) {
		pr_debug("Spurious PIO IRQ\n");
		return;
	}

	buf = (unsigned short *)(tmio_mmc_kmap_atomic(host, &flags) +
	      host->sg_off);

	count = host->sg_ptr->length - host->sg_off;
	if (count > data->blksz)
		count = data->blksz;

	pr_debug("count: %08x offset: %08x flags %08x\n",
	    count, host->sg_off, data->flags);

	
	if (data->flags & MMC_DATA_READ)
		sd_ctrl_read16_rep(host, CTL_SD_DATA_PORT, buf, count >> 1);
	else
		sd_ctrl_write16_rep(host, CTL_SD_DATA_PORT, buf, count >> 1);

	host->sg_off += count;

	tmio_mmc_kunmap_atomic(host, &flags);

	if (host->sg_off == host->sg_ptr->length)
		tmio_mmc_next_sg(host);

	return;
}

static inline void tmio_mmc_data_irq(struct tmio_mmc_host *host)
{
	struct mmc_data *data = host->data;
	struct mmc_command *stop;

	host->data = NULL;

	if (!data) {
		pr_debug("Spurious data end IRQ\n");
		return;
	}
	stop = data->stop;

	
	if (!data->error)
		data->bytes_xfered = data->blocks * data->blksz;
	else
		data->bytes_xfered = 0;

	pr_debug("Completed data request\n");

	

	if (data->flags & MMC_DATA_READ)
		disable_mmc_irqs(host, TMIO_MASK_READOP);
	else
		disable_mmc_irqs(host, TMIO_MASK_WRITEOP);

	if (stop) {
		if (stop->opcode == 12 && !stop->arg)
			sd_ctrl_write16(host, CTL_STOP_INTERNAL_ACTION, 0x000);
		else
			BUG();
	}

	tmio_mmc_finish_request(host);
}

static inline void tmio_mmc_cmd_irq(struct tmio_mmc_host *host,
	unsigned int stat)
{
	struct mmc_command *cmd = host->cmd;
	int i, addr;

	if (!host->cmd) {
		pr_debug("Spurious CMD irq\n");
		return;
	}

	host->cmd = NULL;

	

	for (i = 3, addr = CTL_RESPONSE ; i >= 0 ; i--, addr += 4)
		cmd->resp[i] = sd_ctrl_read32(host, addr);

	if (cmd->flags &  MMC_RSP_136) {
		cmd->resp[0] = (cmd->resp[0] << 8) | (cmd->resp[1] >> 24);
		cmd->resp[1] = (cmd->resp[1] << 8) | (cmd->resp[2] >> 24);
		cmd->resp[2] = (cmd->resp[2] << 8) | (cmd->resp[3] >> 24);
		cmd->resp[3] <<= 8;
	} else if (cmd->flags & MMC_RSP_R3) {
		cmd->resp[0] = cmd->resp[3];
	}

	if (stat & TMIO_STAT_CMDTIMEOUT)
		cmd->error = -ETIMEDOUT;
	else if (stat & TMIO_STAT_CRCFAIL && cmd->flags & MMC_RSP_CRC)
		cmd->error = -EILSEQ;

	
	if (host->data && !cmd->error) {
		if (host->data->flags & MMC_DATA_READ)
			enable_mmc_irqs(host, TMIO_MASK_READOP);
		else
			enable_mmc_irqs(host, TMIO_MASK_WRITEOP);
	} else {
		tmio_mmc_finish_request(host);
	}

	return;
}


static irqreturn_t tmio_mmc_irq(int irq, void *devid)
{
	struct tmio_mmc_host *host = devid;
	unsigned int ireg, irq_mask, status;

	pr_debug("MMC IRQ begin\n");

	status = sd_ctrl_read32(host, CTL_STATUS);
	irq_mask = sd_ctrl_read32(host, CTL_IRQ_MASK);
	ireg = status & TMIO_MASK_IRQ & ~irq_mask;

	pr_debug_status(status);
	pr_debug_status(ireg);

	if (!ireg) {
		disable_mmc_irqs(host, status & ~irq_mask);

		pr_debug("tmio_mmc: Spurious irq, disabling! "
			"0x%08x 0x%08x 0x%08x\n", status, irq_mask, ireg);
		pr_debug_status(status);

		goto out;
	}

	while (ireg) {
		
		if (ireg & (TMIO_STAT_CARD_INSERT | TMIO_STAT_CARD_REMOVE)) {
			ack_mmc_irqs(host, TMIO_STAT_CARD_INSERT |
				TMIO_STAT_CARD_REMOVE);
			mmc_detect_change(host->mmc, 0);
		}

		


		
		if (ireg & TMIO_MASK_CMD) {
			ack_mmc_irqs(host, TMIO_MASK_CMD);
			tmio_mmc_cmd_irq(host, status);
		}

		
		if (ireg & (TMIO_STAT_RXRDY | TMIO_STAT_TXRQ)) {
			ack_mmc_irqs(host, TMIO_STAT_RXRDY | TMIO_STAT_TXRQ);
			tmio_mmc_pio_irq(host);
		}

		
		if (ireg & TMIO_STAT_DATAEND) {
			ack_mmc_irqs(host, TMIO_STAT_DATAEND);
			tmio_mmc_data_irq(host);
		}

		
		status = sd_ctrl_read32(host, CTL_STATUS);
		irq_mask = sd_ctrl_read32(host, CTL_IRQ_MASK);
		ireg = status & TMIO_MASK_IRQ & ~irq_mask;

		pr_debug("Status at end of loop: %08x\n", status);
		pr_debug_status(status);
	}
	pr_debug("MMC IRQ end\n");

out:
	return IRQ_HANDLED;
}

static int tmio_mmc_start_data(struct tmio_mmc_host *host,
	struct mmc_data *data)
{
	pr_debug("setup data transfer: blocksize %08x  nr_blocks %d\n",
	    data->blksz, data->blocks);

	
	if (data->blksz < 4 && host->mmc->ios.bus_width == MMC_BUS_WIDTH_4) {
		printk(KERN_ERR "%s: %d byte block unsupported in 4 bit mode\n",
			mmc_hostname(host->mmc), data->blksz);
		return -EINVAL;
	}

	tmio_mmc_init_sg(host, data);
	host->data = data;

	
	sd_ctrl_write16(host, CTL_SD_XFER_LEN, data->blksz);
	sd_ctrl_write16(host, CTL_XFER_BLK_COUNT, data->blocks);

	return 0;
}


static void tmio_mmc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct tmio_mmc_host *host = mmc_priv(mmc);
	int ret;

	if (host->mrq)
		pr_debug("request not null\n");

	host->mrq = mrq;

	if (mrq->data) {
		ret = tmio_mmc_start_data(host, mrq->data);
		if (ret)
			goto fail;
	}

	ret = tmio_mmc_start_command(host, mrq->cmd);

	if (!ret)
		return;

fail:
	mrq->cmd->error = ret;
	mmc_request_done(mmc, mrq);
}


static void tmio_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct tmio_mmc_host *host = mmc_priv(mmc);

	if (ios->clock)
		tmio_mmc_set_clock(host, ios->clock);

	
	switch (ios->power_mode) {
	case MMC_POWER_OFF: 
		sd_config_write8(host, CNF_PWR_CTL_2, 0x00);
		tmio_mmc_clk_stop(host);
		break;
	case MMC_POWER_ON: 

		sd_config_write8(host, CNF_PWR_CTL_2, 0x02);
		break;
	case MMC_POWER_UP: 
		tmio_mmc_clk_start(host);
		break;
	}

	switch (ios->bus_width) {
	case MMC_BUS_WIDTH_1:
		sd_ctrl_write16(host, CTL_SD_MEM_CARD_OPT, 0x80e0);
	break;
	case MMC_BUS_WIDTH_4:
		sd_ctrl_write16(host, CTL_SD_MEM_CARD_OPT, 0x00e0);
	break;
	}

	
	udelay(140);
}

static int tmio_mmc_get_ro(struct mmc_host *mmc)
{
	struct tmio_mmc_host *host = mmc_priv(mmc);

	return (sd_ctrl_read16(host, CTL_STATUS) & TMIO_STAT_WRPROTECT) ? 0 : 1;
}

static struct mmc_host_ops tmio_mmc_ops = {
	.request	= tmio_mmc_request,
	.set_ios	= tmio_mmc_set_ios,
	.get_ro         = tmio_mmc_get_ro,
};

#ifdef CONFIG_PM
static int tmio_mmc_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mfd_cell	*cell = (struct mfd_cell *)dev->dev.platform_data;
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret;

	ret = mmc_suspend_host(mmc, state);

	
	if (!ret && cell->disable)
		cell->disable(dev);

	return ret;
}

static int tmio_mmc_resume(struct platform_device *dev)
{
	struct mfd_cell	*cell = (struct mfd_cell *)dev->dev.platform_data;
	struct mmc_host *mmc = platform_get_drvdata(dev);
	struct tmio_mmc_host *host = mmc_priv(mmc);
	int ret = 0;

	
	if (cell->enable) {
		ret = cell->enable(dev);
		if (ret)
			goto out;
	}

	
	sd_config_write16(host, CNF_CMD, SDCREN);
	sd_config_write32(host, CNF_CTL_BASE,
		(dev->resource[0].start >> host->bus_shift) & 0xfffe);

	mmc_resume_host(mmc);

out:
	return ret;
}
#else
#define tmio_mmc_suspend NULL
#define tmio_mmc_resume NULL
#endif

static int __devinit tmio_mmc_probe(struct platform_device *dev)
{
	struct mfd_cell	*cell = (struct mfd_cell *)dev->dev.platform_data;
	struct tmio_mmc_data *pdata;
	struct resource *res_ctl, *res_cnf;
	struct tmio_mmc_host *host;
	struct mmc_host *mmc;
	int ret = -EINVAL;

	if (dev->num_resources != 3)
		goto out;

	res_ctl = platform_get_resource(dev, IORESOURCE_MEM, 0);
	res_cnf = platform_get_resource(dev, IORESOURCE_MEM, 1);
	if (!res_ctl || !res_cnf)
		goto out;

	pdata = cell->driver_data;
	if (!pdata || !pdata->hclk)
		goto out;

	ret = -ENOMEM;

	mmc = mmc_alloc_host(sizeof(struct tmio_mmc_host), &dev->dev);
	if (!mmc)
		goto out;

	host = mmc_priv(mmc);
	host->mmc = mmc;
	platform_set_drvdata(dev, mmc);

	
	host->bus_shift = resource_size(res_ctl) >> 10;

	host->ctl = ioremap(res_ctl->start, resource_size(res_ctl));
	if (!host->ctl)
		goto host_free;

	host->cnf = ioremap(res_cnf->start, resource_size(res_cnf));
	if (!host->cnf)
		goto unmap_ctl;

	mmc->ops = &tmio_mmc_ops;
	mmc->caps = MMC_CAP_4_BIT_DATA;
	mmc->f_max = pdata->hclk;
	mmc->f_min = mmc->f_max / 512;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;

	
	if (cell->enable) {
		ret = cell->enable(dev);
		if (ret)
			goto unmap_cnf;
	}

	
	sd_config_write16(host, CNF_CMD, SDCREN);
	sd_config_write32(host, CNF_CTL_BASE,
		(dev->resource[0].start >> host->bus_shift) & 0xfffe);

	
	sd_config_write8(host, CNF_PWR_CTL_3, 0x01);

	
	sd_config_write8(host, CNF_STOP_CLK_CTL, 0x1f);

	
	sd_config_write8(host, CNF_PWR_CTL_2, 0x00);

	tmio_mmc_clk_stop(host);
	reset(host);

	ret = platform_get_irq(dev, 0);
	if (ret >= 0)
		host->irq = ret;
	else
		goto unmap_cnf;

	disable_mmc_irqs(host, TMIO_MASK_ALL);

	ret = request_irq(host->irq, tmio_mmc_irq, IRQF_DISABLED |
		IRQF_TRIGGER_FALLING, "tmio-mmc", host);
	if (ret)
		goto unmap_cnf;

	mmc_add_host(mmc);

	printk(KERN_INFO "%s at 0x%08lx irq %d\n", mmc_hostname(host->mmc),
	       (unsigned long)host->ctl, host->irq);

	
	enable_mmc_irqs(host, TMIO_MASK_IRQ);

	return 0;

unmap_cnf:
	iounmap(host->cnf);
unmap_ctl:
	iounmap(host->ctl);
host_free:
	mmc_free_host(mmc);
out:
	return ret;
}

static int __devexit tmio_mmc_remove(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	if (mmc) {
		struct tmio_mmc_host *host = mmc_priv(mmc);
		mmc_remove_host(mmc);
		free_irq(host->irq, host);
		iounmap(host->ctl);
		iounmap(host->cnf);
		mmc_free_host(mmc);
	}

	return 0;
}



static struct platform_driver tmio_mmc_driver = {
	.driver = {
		.name = "tmio-mmc",
		.owner = THIS_MODULE,
	},
	.probe = tmio_mmc_probe,
	.remove = __devexit_p(tmio_mmc_remove),
	.suspend = tmio_mmc_suspend,
	.resume = tmio_mmc_resume,
};


static int __init tmio_mmc_init(void)
{
	return platform_driver_register(&tmio_mmc_driver);
}

static void __exit tmio_mmc_exit(void)
{
	platform_driver_unregister(&tmio_mmc_driver);
}

module_init(tmio_mmc_init);
module_exit(tmio_mmc_exit);

MODULE_DESCRIPTION("Toshiba TMIO SD/MMC driver");
MODULE_AUTHOR("Ian Molton <spyro@f2s.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:tmio-mmc");
