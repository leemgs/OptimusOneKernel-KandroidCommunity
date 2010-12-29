
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/scatterlist.h>
#include <linux/log2.h>
#include <linux/regulator/consumer.h>
#include <linux/wakelock.h>

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>

#include "core.h"
#include "bus.h"
#include "host.h"
#include "sdio_bus.h"

#include "mmc_ops.h"
#include "sd_ops.h"
#include "sdio_ops.h"

static struct workqueue_struct *workqueue;
static struct wake_lock mmc_delayed_work_wake_lock;


int use_spi_crc = 1;
module_param(use_spi_crc, bool, 0);


static int mmc_schedule_delayed_work(struct delayed_work *work,
				     unsigned long delay)
{
	wake_lock(&mmc_delayed_work_wake_lock);
	return queue_delayed_work(workqueue, work, delay);
}


static void mmc_flush_scheduled_work(void)
{
	flush_workqueue(workqueue);
}


void mmc_request_done(struct mmc_host *host, struct mmc_request *mrq)
{
	struct mmc_command *cmd = mrq->cmd;
	int err = cmd->error;
#ifdef CONFIG_MMC_PERF_PROFILING
	ktime_t diff;
#endif

	if (err && cmd->retries && mmc_host_is_spi(host)) {
		if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
			cmd->retries = 0;
	}

	if (err && cmd->retries) {
		pr_debug("%s: req failed (CMD%u): %d, retrying...\n",
			mmc_hostname(host), cmd->opcode, err);

		cmd->retries--;
		cmd->error = 0;
		host->ops->request(host, mrq);
	} else {
		led_trigger_event(host->led, LED_OFF);

		pr_debug("%s: req done (CMD%u): %d: %08x %08x %08x %08x\n",
			mmc_hostname(host), cmd->opcode, err,
			cmd->resp[0], cmd->resp[1],
			cmd->resp[2], cmd->resp[3]);

		if (mrq->data) {
#ifdef CONFIG_MMC_PERF_PROFILING
			diff = ktime_sub(ktime_get(), host->perf.start);
			if (mrq->data->flags == MMC_DATA_READ) {
				host->perf.rbytes_drv +=
						mrq->data->bytes_xfered;
				host->perf.rtime_drv =
					ktime_add(host->perf.rtime_drv, diff);
			} else {
				host->perf.wbytes_drv +=
						 mrq->data->bytes_xfered;
				host->perf.wtime_drv =
					ktime_add(host->perf.wtime_drv, diff);
			}
#endif
			pr_debug("%s:     %d bytes transferred: %d\n",
				mmc_hostname(host),
				mrq->data->bytes_xfered, mrq->data->error);
		}

		if (mrq->stop) {
			pr_debug("%s:     (CMD%u): %d: %08x %08x %08x %08x\n",
				mmc_hostname(host), mrq->stop->opcode,
				mrq->stop->error,
				mrq->stop->resp[0], mrq->stop->resp[1],
				mrq->stop->resp[2], mrq->stop->resp[3]);
		}

		if (mrq->done)
			mrq->done(mrq);
	}
}

EXPORT_SYMBOL(mmc_request_done);

static void
mmc_start_request(struct mmc_host *host, struct mmc_request *mrq)
{
#ifdef CONFIG_MMC_DEBUG
	unsigned int i, sz;
	struct scatterlist *sg;
#endif

	pr_debug("%s: starting CMD%u arg %08x flags %08x\n",
		 mmc_hostname(host), mrq->cmd->opcode,
		 mrq->cmd->arg, mrq->cmd->flags);

	if (mrq->data) {
		pr_debug("%s:     blksz %d blocks %d flags %08x "
			"tsac %d ms nsac %d\n",
			mmc_hostname(host), mrq->data->blksz,
			mrq->data->blocks, mrq->data->flags,
			mrq->data->timeout_ns / 1000000,
			mrq->data->timeout_clks);
	}

	if (mrq->stop) {
		pr_debug("%s:     CMD%u arg %08x flags %08x\n",
			 mmc_hostname(host), mrq->stop->opcode,
			 mrq->stop->arg, mrq->stop->flags);
	}

	WARN_ON(!host->claimed);

	led_trigger_event(host->led, LED_FULL);

	mrq->cmd->error = 0;
	mrq->cmd->mrq = mrq;
	if (mrq->data) {
		BUG_ON(mrq->data->blksz > host->max_blk_size);
		BUG_ON(mrq->data->blocks > host->max_blk_count);
		BUG_ON(mrq->data->blocks * mrq->data->blksz >
			host->max_req_size);

#ifdef CONFIG_MMC_DEBUG
		sz = 0;
		for_each_sg(mrq->data->sg, sg, mrq->data->sg_len, i)
			sz += sg->length;
		BUG_ON(sz != mrq->data->blocks * mrq->data->blksz);
#endif

		mrq->cmd->data = mrq->data;
		mrq->data->error = 0;
		mrq->data->mrq = mrq;
		if (mrq->stop) {
			mrq->data->stop = mrq->stop;
			mrq->stop->error = 0;
			mrq->stop->mrq = mrq;
		}

#ifdef CONFIG_MMC_PERF_PROFILING
		host->perf.start = ktime_get();
#endif
	}
	host->ops->request(host, mrq);
}

static void mmc_wait_done(struct mmc_request *mrq)
{
	complete(mrq->done_data);
}


void mmc_wait_for_req(struct mmc_host *host, struct mmc_request *mrq)
{
	DECLARE_COMPLETION_ONSTACK(complete);

	mrq->done_data = &complete;
	mrq->done = mmc_wait_done;

	mmc_start_request(host, mrq);

	wait_for_completion(&complete);
}

EXPORT_SYMBOL(mmc_wait_for_req);


int mmc_wait_for_cmd(struct mmc_host *host, struct mmc_command *cmd, int retries)
{
	struct mmc_request mrq;

	WARN_ON(!host->claimed);

	memset(&mrq, 0, sizeof(struct mmc_request));

	memset(cmd->resp, 0, sizeof(cmd->resp));
	cmd->retries = retries;

	mrq.cmd = cmd;
	cmd->data = NULL;

	mmc_wait_for_req(host, &mrq);

	return cmd->error;
}

EXPORT_SYMBOL(mmc_wait_for_cmd);


void mmc_set_data_timeout(struct mmc_data *data, const struct mmc_card *card)
{
	unsigned int mult;

	
	if (mmc_card_sdio(card)) {
		data->timeout_ns = 1000000000;
		data->timeout_clks = 0;
		return;
	}

	
	mult = mmc_card_sd(card) ? 100 : 10;

	
	if (data->flags & MMC_DATA_WRITE)
		mult <<= card->csd.r2w_factor;

	data->timeout_ns = card->csd.tacc_ns * mult;
	data->timeout_clks = card->csd.tacc_clks * mult;

	
	if (mmc_card_sd(card)) {
		unsigned int timeout_us, limit_us;

		timeout_us = data->timeout_ns / 1000;
		timeout_us += data->timeout_clks * 1000 /
			(card->host->ios.clock / 1000);

		if (data->flags & MMC_DATA_WRITE)
			
			limit_us = 300000;
		else
			limit_us = 100000;

		
		if (timeout_us > limit_us || mmc_card_blockaddr(card)) {
			data->timeout_ns = limit_us * 1000;
			data->timeout_clks = 0;
		}
	}
	
	if (mmc_host_is_spi(card->host)) {
		if (data->flags & MMC_DATA_WRITE) {
			if (data->timeout_ns < 1000000000)
				data->timeout_ns = 1000000000;	
		} else {
			if (data->timeout_ns < 100000000)
				data->timeout_ns =  100000000;	
		}
	}
}
EXPORT_SYMBOL(mmc_set_data_timeout);


unsigned int mmc_align_data_size(struct mmc_card *card, unsigned int sz)
{
	
	sz = ((sz + 3) / 4) * 4;

	return sz;
}
EXPORT_SYMBOL(mmc_align_data_size);


int mmc_host_enable(struct mmc_host *host)
{
	if (!(host->caps & MMC_CAP_DISABLE))
		return 0;

	if (host->en_dis_recurs)
		return 0;

	if (host->nesting_cnt++)
		return 0;

	cancel_delayed_work_sync(&host->disable);

	if (host->enabled)
		return 0;

	if (host->ops->enable) {
		int err;

		host->en_dis_recurs = 1;
		err = host->ops->enable(host);
		host->en_dis_recurs = 0;

		if (err) {
			pr_debug("%s: enable error %d\n",
				 mmc_hostname(host), err);
			return err;
		}
	}
	host->enabled = 1;
	return 0;
}
EXPORT_SYMBOL(mmc_host_enable);

static int mmc_host_do_disable(struct mmc_host *host, int lazy)
{
	if (host->ops->disable) {
		int err;

		host->en_dis_recurs = 1;
		err = host->ops->disable(host, lazy);
		host->en_dis_recurs = 0;

		if (err < 0) {
			pr_debug("%s: disable error %d\n",
				 mmc_hostname(host), err);
			return err;
		}
		if (err > 0) {
			unsigned long delay = msecs_to_jiffies(err);

			mmc_schedule_delayed_work(&host->disable, delay);
		}
	}
	host->enabled = 0;
	return 0;
}


int mmc_host_disable(struct mmc_host *host)
{
	int err;

	if (!(host->caps & MMC_CAP_DISABLE))
		return 0;

	if (host->en_dis_recurs)
		return 0;

	if (--host->nesting_cnt)
		return 0;

	if (!host->enabled)
		return 0;

	err = mmc_host_do_disable(host, 0);
	return err;
}
EXPORT_SYMBOL(mmc_host_disable);


int __mmc_claim_host(struct mmc_host *host, atomic_t *abort)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long flags;
	int stop;

	might_sleep();

	add_wait_queue(&host->wq, &wait);
	spin_lock_irqsave(&host->lock, flags);
	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		stop = abort ? atomic_read(abort) : 0;
		if (stop || !host->claimed || host->claimer == current)
			break;
		spin_unlock_irqrestore(&host->lock, flags);
		schedule();
		spin_lock_irqsave(&host->lock, flags);
	}
	set_current_state(TASK_RUNNING);
	if (!stop) {
		host->claimed = 1;
		host->claimer = current;
		host->claim_cnt += 1;
	} else
		wake_up(&host->wq);
	spin_unlock_irqrestore(&host->lock, flags);
	remove_wait_queue(&host->wq, &wait);
	if (!stop)
		mmc_host_enable(host);
	return stop;
}

EXPORT_SYMBOL(__mmc_claim_host);


int mmc_try_claim_host(struct mmc_host *host)
{
	int claimed_host = 0;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	if (!host->claimed || host->claimer == current) {
		host->claimed = 1;
		host->claimer = current;
		host->claim_cnt += 1;
		claimed_host = 1;
	}
	spin_unlock_irqrestore(&host->lock, flags);
	return claimed_host;
}
EXPORT_SYMBOL(mmc_try_claim_host);

static void mmc_do_release_host(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	if (--host->claim_cnt) {
		
		spin_unlock_irqrestore(&host->lock, flags);
	} else {
		host->claimed = 0;
		host->claimer = NULL;
		spin_unlock_irqrestore(&host->lock, flags);
		wake_up(&host->wq);
	}
}

void mmc_host_deeper_disable(struct work_struct *work)
{
	struct mmc_host *host =
		container_of(work, struct mmc_host, disable.work);

	
	if (!mmc_try_claim_host(host))
		goto out;
	mmc_host_do_disable(host, 1);
	mmc_do_release_host(host);

out:
	wake_unlock(&mmc_delayed_work_wake_lock);
}


int mmc_host_lazy_disable(struct mmc_host *host)
{
	if (!(host->caps & MMC_CAP_DISABLE))
		return 0;

	if (host->en_dis_recurs)
		return 0;

	if (--host->nesting_cnt)
		return 0;

	if (!host->enabled)
		return 0;

	if (host->disable_delay) {
		mmc_schedule_delayed_work(&host->disable,
				msecs_to_jiffies(host->disable_delay));
		return 0;
	} else
		return mmc_host_do_disable(host, 1);
}
EXPORT_SYMBOL(mmc_host_lazy_disable);


void mmc_release_host(struct mmc_host *host)
{
	WARN_ON(!host->claimed);

	mmc_host_lazy_disable(host);

	mmc_do_release_host(host);
}

EXPORT_SYMBOL(mmc_release_host);


static inline void mmc_set_ios(struct mmc_host *host)
{
	struct mmc_ios *ios = &host->ios;

	pr_debug("%s: clock %uHz busmode %u powermode %u cs %u Vdd %u "
		"width %u timing %u\n",
		 mmc_hostname(host), ios->clock, ios->bus_mode,
		 ios->power_mode, ios->chip_select, ios->vdd,
		 ios->bus_width, ios->timing);

	host->ops->set_ios(host, ios);
}


void mmc_set_chip_select(struct mmc_host *host, int mode)
{
	host->ios.chip_select = mode;
	mmc_set_ios(host);
}


void mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
	WARN_ON(hz < host->f_min);

	if (hz > host->f_max)
		hz = host->f_max;

	host->ios.clock = hz;
	mmc_set_ios(host);
}


void mmc_set_bus_mode(struct mmc_host *host, unsigned int mode)
{
	host->ios.bus_mode = mode;
	mmc_set_ios(host);
}


void mmc_set_bus_width(struct mmc_host *host, unsigned int width)
{
	host->ios.bus_width = width;
	mmc_set_ios(host);
}


static int mmc_vdd_to_ocrbitnum(int vdd, bool low_bits)
{
	const int max_bit = ilog2(MMC_VDD_35_36);
	int bit;

	if (vdd < 1650 || vdd > 3600)
		return -EINVAL;

	if (vdd >= 1650 && vdd <= 1950)
		return ilog2(MMC_VDD_165_195);

	if (low_bits)
		vdd -= 1;

	
	bit = (vdd - 2000) / 100 + 8;
	if (bit > max_bit)
		return max_bit;
	return bit;
}


u32 mmc_vddrange_to_ocrmask(int vdd_min, int vdd_max)
{
	u32 mask = 0;

	if (vdd_max < vdd_min)
		return 0;

	
	vdd_max = mmc_vdd_to_ocrbitnum(vdd_max, false);
	if (vdd_max < 0)
		return 0;

	
	vdd_min = mmc_vdd_to_ocrbitnum(vdd_min, true);
	if (vdd_min < 0)
		return 0;

	
	while (vdd_max >= vdd_min)
		mask |= 1 << vdd_max--;

	return mask;
}
EXPORT_SYMBOL(mmc_vddrange_to_ocrmask);

#ifdef CONFIG_REGULATOR


int mmc_regulator_get_ocrmask(struct regulator *supply)
{
	int			result = 0;
	int			count;
	int			i;

	count = regulator_count_voltages(supply);
	if (count < 0)
		return count;

	for (i = 0; i < count; i++) {
		int		vdd_uV;
		int		vdd_mV;

		vdd_uV = regulator_list_voltage(supply, i);
		if (vdd_uV <= 0)
			continue;

		vdd_mV = vdd_uV / 1000;
		result |= mmc_vddrange_to_ocrmask(vdd_mV, vdd_mV);
	}

	return result;
}
EXPORT_SYMBOL(mmc_regulator_get_ocrmask);


int mmc_regulator_set_ocr(struct regulator *supply, unsigned short vdd_bit)
{
	int			result = 0;
	int			min_uV, max_uV;
	int			enabled;

	enabled = regulator_is_enabled(supply);
	if (enabled < 0)
		return enabled;

	if (vdd_bit) {
		int		tmp;
		int		voltage;

		
		tmp = vdd_bit - ilog2(MMC_VDD_165_195);
		if (tmp == 0) {
			min_uV = 1650 * 1000;
			max_uV = 1950 * 1000;
		} else {
			min_uV = 1900 * 1000 + tmp * 100 * 1000;
			max_uV = min_uV + 100 * 1000;
		}

		
		voltage = regulator_get_voltage(supply);
		if (voltage < 0)
			result = voltage;
		else if (voltage < min_uV || voltage > max_uV)
			result = regulator_set_voltage(supply, min_uV, max_uV);
		else
			result = 0;

		if (result == 0 && !enabled)
			result = regulator_enable(supply);
	} else if (enabled) {
		result = regulator_disable(supply);
	}

	return result;
}
EXPORT_SYMBOL(mmc_regulator_set_ocr);

#endif


u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
	int bit;

	ocr &= host->ocr_avail;

	bit = ffs(ocr);
	if (bit) {
		bit -= 1;

		ocr &= 3 << bit;

		host->ios.vdd = bit;
		mmc_set_ios(host);
	} else {
		pr_warning("%s: host doesn't support card's voltages\n",
				mmc_hostname(host));
		ocr = 0;
	}

	return ocr;
}


void mmc_set_timing(struct mmc_host *host, unsigned int timing)
{
	host->ios.timing = timing;
	mmc_set_ios(host);
}


static void mmc_power_up(struct mmc_host *host)
{
	int bit;

	
	if (host->ocr)
		bit = ffs(host->ocr) - 1;
	else
		bit = fls(host->ocr_avail) - 1;

	host->ios.vdd = bit;
	if (mmc_host_is_spi(host)) {
		host->ios.chip_select = MMC_CS_HIGH;
		host->ios.bus_mode = MMC_BUSMODE_PUSHPULL;
	} else {
		host->ios.chip_select = MMC_CS_DONTCARE;
		host->ios.bus_mode = MMC_BUSMODE_OPENDRAIN;
	}
	host->ios.power_mode = MMC_POWER_UP;
	host->ios.bus_width = MMC_BUS_WIDTH_1;
	host->ios.timing = MMC_TIMING_LEGACY;
	mmc_set_ios(host);

	
	mmc_delay(10);

	host->ios.clock = host->f_min;

	host->ios.power_mode = MMC_POWER_ON;
	mmc_set_ios(host);

	
	mmc_delay(10);
}

static void mmc_power_off(struct mmc_host *host)
{
	host->ios.clock = 0;
	host->ios.vdd = 0;
	if (!mmc_host_is_spi(host)) {
		host->ios.bus_mode = MMC_BUSMODE_OPENDRAIN;
		host->ios.chip_select = MMC_CS_DONTCARE;
	}
	host->ios.power_mode = MMC_POWER_OFF;
	host->ios.bus_width = MMC_BUS_WIDTH_1;
	host->ios.timing = MMC_TIMING_LEGACY;
	mmc_set_ios(host);
}


static void __mmc_release_bus(struct mmc_host *host)
{
	BUG_ON(!host);
	BUG_ON(host->bus_refs);
	BUG_ON(!host->bus_dead);

	host->bus_ops = NULL;
}


static inline void mmc_bus_get(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->bus_refs++;
	spin_unlock_irqrestore(&host->lock, flags);
}


static inline void mmc_bus_put(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->bus_refs--;
	if ((host->bus_refs == 0) && host->bus_ops)
		__mmc_release_bus(host);
	spin_unlock_irqrestore(&host->lock, flags);
}

int mmc_resume_bus(struct mmc_host *host)
{
	if (!mmc_bus_needs_resume(host))
		return -EINVAL;

	printk("%s: Starting deferred resume\n", mmc_hostname(host));
	host->bus_resume_flags &= ~MMC_BUSRESUME_NEEDS_RESUME;
	mmc_bus_get(host);
	if (host->bus_ops && !host->bus_dead) {
		mmc_power_up(host);
		BUG_ON(!host->bus_ops->resume);
		host->bus_ops->resume(host);
	}

	if (host->bus_ops->detect && !host->bus_dead)
		host->bus_ops->detect(host);

	mmc_bus_put(host);
	printk("%s: Deferred resume completed\n", mmc_hostname(host));
	return 0;
}

EXPORT_SYMBOL(mmc_resume_bus);


void mmc_attach_bus(struct mmc_host *host, const struct mmc_bus_ops *ops)
{
	unsigned long flags;

	BUG_ON(!host);
	BUG_ON(!ops);

	WARN_ON(!host->claimed);

	spin_lock_irqsave(&host->lock, flags);

	BUG_ON(host->bus_ops);
	BUG_ON(host->bus_refs);

	host->bus_ops = ops;
	host->bus_refs = 1;
	host->bus_dead = 0;

	spin_unlock_irqrestore(&host->lock, flags);
}


void mmc_detach_bus(struct mmc_host *host)
{
	unsigned long flags;

	BUG_ON(!host);

	WARN_ON(!host->claimed);
	WARN_ON(!host->bus_ops);

	spin_lock_irqsave(&host->lock, flags);

	host->bus_dead = 1;

	spin_unlock_irqrestore(&host->lock, flags);

	mmc_power_off(host);

	mmc_bus_put(host);
}


void mmc_detect_change(struct mmc_host *host, unsigned long delay)
{
#ifdef CONFIG_MMC_DEBUG
	unsigned long flags;
	spin_lock_irqsave(&host->lock, flags);
	WARN_ON(host->removed);
	spin_unlock_irqrestore(&host->lock, flags);
#endif

	mmc_schedule_delayed_work(&host->detect, delay);
}

EXPORT_SYMBOL(mmc_detect_change);


void mmc_rescan(struct work_struct *work)
{
	struct mmc_host *host =
		container_of(work, struct mmc_host, detect.work);
	u32 ocr;
	int err;
	int extend_wakelock = 0;
	int ret;

	ret = host->ops->get_status(host);
	if (host->ops->get_status && ret == 1){
		mmc_schedule_delayed_work(&host->detect, HZ / 5);
	}
	mmc_bus_get(host);

	
	if ((host->bus_ops != NULL) && host->bus_ops->detect && !host->bus_dead)
		host->bus_ops->detect(host);

	
	if (host->bus_dead)
		extend_wakelock = 1;

	mmc_bus_put(host);


	mmc_bus_get(host);

	
	if (host->bus_ops != NULL) {
		mmc_bus_put(host);
		goto out;
	}

	

	
	mmc_bus_put(host);

	if (host->ops->get_cd && host->ops->get_cd(host) == 0)
		goto out;

	mmc_claim_host(host);

	mmc_power_up(host);
	mmc_go_idle(host);

	mmc_send_if_cond(host, host->ocr_avail);

	
	err = mmc_send_io_op_cond(host, 0, &ocr);
	if (!err) {
		if (mmc_attach_sdio(host, ocr))
			mmc_power_off(host);
		extend_wakelock = 1;
		goto out;
	}

	
	err = mmc_send_app_op_cond(host, 0, &ocr);
	if (!err) {
		if (mmc_attach_sd(host, ocr))
			mmc_power_off(host);
		extend_wakelock = 1;
		goto out;
	}

	
	err = mmc_send_op_cond(host, 0, &ocr);
	if (!err) {
		if (mmc_attach_mmc(host, ocr))
			mmc_power_off(host);
		extend_wakelock = 1;
		goto out;
	}

	mmc_release_host(host);
	mmc_power_off(host);

out:
	if (extend_wakelock)
		wake_lock_timeout(&mmc_delayed_work_wake_lock, HZ / 2);
	else
		wake_unlock(&mmc_delayed_work_wake_lock);

	if (host->caps & MMC_CAP_NEEDS_POLL)
		mmc_schedule_delayed_work(&host->detect, HZ);
}

void mmc_start_host(struct mmc_host *host)
{
	mmc_power_off(host);
	mmc_detect_change(host, 0);
}

void mmc_stop_host(struct mmc_host *host)
{
#ifdef CONFIG_MMC_DEBUG
	unsigned long flags;
	spin_lock_irqsave(&host->lock, flags);
	host->removed = 1;
	spin_unlock_irqrestore(&host->lock, flags);
#endif

	if (host->caps & MMC_CAP_DISABLE)
		cancel_delayed_work(&host->disable);
	cancel_delayed_work(&host->detect);
	mmc_flush_scheduled_work();

	mmc_bus_get(host);
	if (host->bus_ops && !host->bus_dead) {
		if (host->bus_ops->remove)
			host->bus_ops->remove(host);

		mmc_claim_host(host);
		mmc_detach_bus(host);
		mmc_release_host(host);
		mmc_bus_put(host);
		return;
	}
	mmc_bus_put(host);

	BUG_ON(host->card);

	mmc_power_off(host);
}

void mmc_power_save_host(struct mmc_host *host)
{
	mmc_bus_get(host);

	if (!host->bus_ops || host->bus_dead || !host->bus_ops->power_restore) {
		mmc_bus_put(host);
		return;
	}

	if (host->bus_ops->power_save)
		host->bus_ops->power_save(host);

	mmc_bus_put(host);

	mmc_power_off(host);
}
EXPORT_SYMBOL(mmc_power_save_host);

void mmc_power_restore_host(struct mmc_host *host)
{
	mmc_bus_get(host);

	if (!host->bus_ops || host->bus_dead || !host->bus_ops->power_restore) {
		mmc_bus_put(host);
		return;
	}

	mmc_power_up(host);
	host->bus_ops->power_restore(host);

	mmc_bus_put(host);
}
EXPORT_SYMBOL(mmc_power_restore_host);

int mmc_card_awake(struct mmc_host *host)
{
	int err = -ENOSYS;

	mmc_bus_get(host);

	if (host->bus_ops && !host->bus_dead && host->bus_ops->awake)
		err = host->bus_ops->awake(host);

	mmc_bus_put(host);

	return err;
}
EXPORT_SYMBOL(mmc_card_awake);

int mmc_card_sleep(struct mmc_host *host)
{
	int err = -ENOSYS;

	mmc_bus_get(host);

	if (host->bus_ops && !host->bus_dead && host->bus_ops->awake)
		err = host->bus_ops->sleep(host);

	mmc_bus_put(host);

	return err;
}
EXPORT_SYMBOL(mmc_card_sleep);


int mmc_direct_power_off(struct mmc_host *host)
{
	mmc_power_off(host);
	printk("\n@@@@@@@@@@@@@@@@@@@@@@>>> mmc_direct_power_off    \n");
	return 0;
}

EXPORT_SYMBOL(mmc_direct_power_off);

int mmc_direct_power_up(struct mmc_host *host)
{
	mmc_power_up(host);
	printk("\n@@@@@@@@@@@@@@@@@@@@@@>>> mmc_direct_power_up    \n");
	return 0;
}

EXPORT_SYMBOL(mmc_direct_power_up);

int mmc_card_can_sleep(struct mmc_host *host)
{
	struct mmc_card *card = host->card;

	if (card && mmc_card_mmc(card) && card->ext_csd.rev >= 3)
		return 1;
	return 0;
}
EXPORT_SYMBOL(mmc_card_can_sleep);

#ifdef CONFIG_PM


int mmc_suspend_host(struct mmc_host *host, pm_message_t state)
{
	int err = 0;

	if (mmc_bus_needs_resume(host))
		return 0;

	if (host->caps & MMC_CAP_DISABLE)
		cancel_delayed_work(&host->disable);

	mmc_bus_get(host);
	if (host->bus_ops && !host->bus_dead) {
		if (host->bus_ops->suspend)
			err = host->bus_ops->suspend(host);
	}
	mmc_bus_put(host);

	if (!err)
		mmc_power_off(host);

	return err;
}

EXPORT_SYMBOL(mmc_suspend_host);


int mmc_resume_host(struct mmc_host *host)
{
	int err = 0;

	mmc_bus_get(host);
	if (host->bus_resume_flags & MMC_BUSRESUME_MANUAL_RESUME) {
		host->bus_resume_flags |= MMC_BUSRESUME_NEEDS_RESUME;
		mmc_bus_put(host);
		return 0;
	}

	if (host->bus_ops && !host->bus_dead) {
		mmc_power_up(host);
		mmc_select_voltage(host, host->ocr);
		BUG_ON(!host->bus_ops->resume);
		err = host->bus_ops->resume(host);
		if (err) {
			printk(KERN_WARNING "%s: error %d during resume "
					    "(card was removed?)\n",
					    mmc_hostname(host), err);
			err = 0;
		}
	}
	mmc_bus_put(host);

	
	mmc_detect_change(host, 1);

	return err;
}
EXPORT_SYMBOL(mmc_resume_host);


int mmc_pm_notify(struct notifier_block *notify_block,
					unsigned long mode, void *unused)
{
	struct mmc_host *host = container_of(
		notify_block, struct mmc_host, pm_notify);


	switch (mode) {
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:

		if (!host->bus_ops || host->bus_ops->suspend)
			break;

		if (host->bus_ops->remove)
			host->bus_ops->remove(host);
		mmc_claim_host(host);
		mmc_detach_bus(host);
		mmc_release_host(host);
		break;

	}

	return 0;
}

#endif

#ifdef CONFIG_MMC_EMBEDDED_SDIO
void mmc_set_embedded_sdio_data(struct mmc_host *host,
				struct sdio_cis *cis,
				struct sdio_cccr *cccr,
				struct sdio_embedded_func *funcs,
				int num_funcs)
{
	host->embedded_sdio_data.cis = cis;
	host->embedded_sdio_data.cccr = cccr;
	host->embedded_sdio_data.funcs = funcs;
	host->embedded_sdio_data.num_funcs = num_funcs;
}

EXPORT_SYMBOL(mmc_set_embedded_sdio_data);
#endif

static int __init mmc_init(void)
{
	int ret;

	wake_lock_init(&mmc_delayed_work_wake_lock, WAKE_LOCK_SUSPEND, "mmc_delayed_work");

	workqueue = create_freezeable_workqueue("kmmcd");
	if (!workqueue)
		return -ENOMEM;

	ret = mmc_register_bus();
	if (ret)
		goto destroy_workqueue;

	ret = mmc_register_host_class();
	if (ret)
		goto unregister_bus;

	ret = sdio_register_bus();
	if (ret)
		goto unregister_host_class;

	return 0;

unregister_host_class:
	mmc_unregister_host_class();
unregister_bus:
	mmc_unregister_bus();
destroy_workqueue:
	destroy_workqueue(workqueue);

	return ret;
}

static void __exit mmc_exit(void)
{
	sdio_unregister_bus();
	mmc_unregister_host_class();
	mmc_unregister_bus();
	destroy_workqueue(workqueue);
	wake_lock_destroy(&mmc_delayed_work_wake_lock);
}

subsys_initcall(mmc_init);
module_exit(mmc_exit);

MODULE_LICENSE("GPL");
