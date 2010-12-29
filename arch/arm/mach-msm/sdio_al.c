



#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#include <mach/sdio_al.h>

#define MODULE_MAME "sdio_al"
#define DRV_VERSION "1.08"




#define SDIO_AL_MAX_CHANNELS 4


#define SDIO_AL_MAX_FUNCS    (SDIO_AL_MAX_CHANNELS+1)


#define SDIO_AL_MAX_PIPES    16


#define SDIO_AL_BLOCK_SIZE   128


#define HW_MAILBOX_ADDR			0x1000


#define SDIOC_SW_HEADER_ADDR		0x0400


#define SDIOC_SW_MAILBOX_ADDR			0x4000


#define PIPES_THRESHOLD_ADDR		0x01000

#define PIPES_0_7_IRQ_MASK_ADDR 	0x01048

#define PIPES_8_15_IRQ_MASK_ADDR	0x0104C

#define EOT_PIPES_ENABLE		0x00


#define MAX_DATA_AVAILABLE   		(16*1024)
#define INVALID_DATA_AVAILABLE  	(0x8000)


#define DEFAULT_WRITE_THRESHOLD 	(1024)


#define DEFAULT_READ_THRESHOLD  	(1024)


#define DEFAULT_MIN_WRITE_THRESHOLD 	(1024)

#define THRESHOLD_DISABLE_VAL  		(0xFFFFFFFF)

#define DEFAULT_POLL_DELAY_MSEC		10


#define DEFAULT_PEER_TX_BUF_SIZE	(128)

#define ROUND_UP(x, n) (((x + n - 1) / n) * n)


#define PIPE_RX_FIFO_ADDR   0x00
#define PIPE_TX_FIFO_ADDR   0x00


#define SDIO_AL_SIGNATURE 0xAABBCCDD


#define SD_IO_RW_EXTENDED_QCOM 54


enum sdio_priority {
	SDIO_PRIORITY_HIGH = 1,
	SDIO_PRIORITY_MED  = 5,
	SDIO_PRIORITY_LOW  = 9,
};


struct sdio_mailbox {
	u32 pipe_bytes_threshold[SDIO_AL_MAX_PIPES]; 

	
	u32 mask_irq_func_1:8; 
	u32 mask_irq_func_2:8;
	u32 mask_irq_func_3:8;
	u32 mask_irq_func_4:8;

	u32 mask_irq_func_5:8;
	u32 mask_irq_func_6:8;
	u32 mask_irq_func_7:8;
	u32 mask_mutex_irq:8;

	
	u32 mask_eot_pipe_0_7:8;
	u32 mask_thresh_above_limit_pipe_0_7:8;
	u32 mask_overflow_pipe_0_7:8;
	u32 mask_underflow_pipe_0_7:8;

	u32 mask_eot_pipe_8_15:8;
	u32 mask_thresh_above_limit_pipe_8_15:8;
	u32 mask_overflow_pipe_8_15:8;
	u32 mask_underflow_pipe_8_15:8;

	
	u32 user_irq_func_1:8;
	u32 user_irq_func_2:8;
	u32 user_irq_func_3:8;
	u32 user_irq_func_4:8;

	u32 user_irq_func_5:8;
	u32 user_irq_func_6:8;
	u32 user_irq_func_7:8;
	u32 user_mutex_irq:8;

	
	
	u32 eot_pipe_0_7:8;
	u32 thresh_above_limit_pipe_0_7:8;
	u32 overflow_pipe_0_7:8;
	u32 underflow_pipe_0_7:8;

	u32 eot_pipe_8_15:8;
	u32 thresh_above_limit_pipe_8_15:8;
	u32 overflow_pipe_8_15:8;
	u32 underflow_pipe_8_15:8;

	u16 pipe_bytes_avail[SDIO_AL_MAX_PIPES];
};


struct rx_packet_size {
	u32 size; 
	struct list_head	list;
};


struct peer_sdioc_channel_config {
	u32 is_ready;
	u32 max_rx_threshold; 
	u32 max_tx_threshold; 
	u32 tx_buf_size;
	u32 reserved[28];
};

#define PEER_SDIOC_SW_MAILBOX_SIGNATURE 0xFACECAFE


struct peer_sdioc_sw_header {
	u32 signature;
	u32 version;
	u32 max_channels;
	u32 reserved[29];
};


struct peer_sdioc_sw_mailbox {
	struct peer_sdioc_sw_header sw_header;
	struct peer_sdioc_channel_config ch_config[6];
};


struct sdio_channel {
	
	const char *name;
	int priority;
	int read_threshold;
	int write_threshold;
	int min_write_avail;
	int poll_delay_msec;
	int is_packet_mode;

	
	int num;

	void (*notify)(void *priv, unsigned channel_event);
	void *priv;

	int is_open;
	int is_suspend;

	struct sdio_func *func;

	int rx_pipe_index;
	int tx_pipe_index;

	struct mutex ch_lock;

	u32 read_avail;
	u32 write_avail;

	u32 peer_tx_buf_size;

	u16 rx_pending_bytes;

	struct list_head rx_size_list_head;

	struct platform_device pdev;

	u32 total_rx_bytes;
	u32 total_tx_bytes;

	u32 signature;
};


struct sdio_al {
	struct mmc_card *card;
	struct sdio_mailbox *mailbox;
	struct sdio_channel channel[SDIO_AL_MAX_CHANNELS];

	struct peer_sdioc_sw_header *sdioc_sw_header;

	struct workqueue_struct *workqueue;
	struct work_struct work;

	int is_ready;

	wait_queue_head_t   wait_mbox;
	int ask_mbox;

	struct timer_list timer;
	int poll_delay_msec;

	int use_irq;

	int is_err;

	u32 signature;
};


static struct sdio_al *sdio_al;


static int enable_eot_interrupt(int pipe_index, int enable);
static int enable_threshold_interrupt(int pipe_index, int enable);
static void sdio_func_irq(struct sdio_func *func);
static void timer_handler(unsigned long data);
static int get_min_poll_time_msec(void);
static u32 check_pending_rx_packet(struct sdio_channel *ch, u32 eot);
static u32 remove_handled_rx_packet(struct sdio_channel *ch);
static int set_pipe_threshold(int pipe_index, int threshold);


static int read_mailbox(int from_isr)
{
	int ret;
	struct sdio_func *func1 = sdio_al->card->sdio_func[0];
	struct sdio_mailbox *mailbox = sdio_al->mailbox;
	u32 new_write_avail = 0;
	u32 old_write_avail = 0;
	int i;
	u32 rx_notify_bitmask = 0;
	u32 tx_notify_bitmask = 0;
	u32 eot_pipe = 0;
	u32 thresh_pipe = 0;
	u32 overflow_pipe = 0;
	u32 underflow_pipe = 0;
	u32 thresh_intr_mask = 0;

	if (sdio_al->is_err) {
		pr_info(MODULE_MAME ":In Error state, ignore request\n");
		return 0;
	}

	pr_debug(MODULE_MAME ":start %s from_isr = %d.\n", __func__, from_isr);

	if (!from_isr)
		sdio_claim_host(sdio_al->card->sdio_func[0]);
	pr_debug(MODULE_MAME ":before sdio_memcpy_fromio.\n");
	ret = sdio_memcpy_fromio(func1, mailbox,
			HW_MAILBOX_ADDR, sizeof(*mailbox));
	pr_debug(MODULE_MAME ":after sdio_memcpy_fromio.\n");

	eot_pipe =	(mailbox->eot_pipe_0_7) |
			(mailbox->eot_pipe_8_15<<8);
	thresh_pipe = 	(mailbox->thresh_above_limit_pipe_0_7) |
			(mailbox->thresh_above_limit_pipe_8_15<<8);

	overflow_pipe = (mailbox->overflow_pipe_0_7) |
			(mailbox->overflow_pipe_8_15<<8);
	underflow_pipe = mailbox->underflow_pipe_0_7 |
			(mailbox->underflow_pipe_8_15<<8);
	thresh_intr_mask =
		(mailbox->mask_thresh_above_limit_pipe_0_7) |
		(mailbox->mask_thresh_above_limit_pipe_8_15<<8);


	if (ret) {
		pr_err(MODULE_MAME ":Fail to read Mailbox,"
				    " goto error state\n");
		sdio_al->is_err = true;
		
		sdio_al->poll_delay_msec = 0;
		goto exit_err;
	}

	if (overflow_pipe || underflow_pipe)
		pr_info(MODULE_MAME ":Mailbox ERROR "
				"overflow=0x%x, underflow=0x%x\n",
				overflow_pipe, underflow_pipe);


	pr_debug(MODULE_MAME ":eot=0x%x, thresh=0x%x\n",
			 eot_pipe, thresh_pipe);

	
	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		struct sdio_channel *ch = &sdio_al->channel[i];
		u32 old_read_avail;
		u32 read_avail;
		u32 new_packet_size = 0;

		if (!ch->is_open)
			continue;

		old_read_avail = ch->read_avail;
		read_avail = mailbox->pipe_bytes_avail[ch->rx_pipe_index];

		if (read_avail > INVALID_DATA_AVAILABLE) {
			pr_info(MODULE_MAME
				 ":Invalid read_avail 0x%x for pipe %d\n",
				 read_avail, ch->rx_pipe_index);
			continue;
		}

		if (ch->is_packet_mode)
			new_packet_size = check_pending_rx_packet(ch, eot_pipe);
		else
			ch->read_avail = read_avail;

		if ((ch->is_packet_mode) && (new_packet_size > 0))
			rx_notify_bitmask |= (1<<ch->num);

		if ((!ch->is_packet_mode) && (ch->read_avail > 0) &&
		    (old_read_avail == 0))
			rx_notify_bitmask |= (1<<ch->num);
	}

	
	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		struct sdio_channel *ch = &sdio_al->channel[i];

		if (!ch->is_open)
			continue;

		new_write_avail = mailbox->pipe_bytes_avail[ch->tx_pipe_index];

		if (new_write_avail > INVALID_DATA_AVAILABLE) {
			pr_info(MODULE_MAME
				 ":Invalid write_avail 0x%x for pipe %d\n",
				 new_write_avail, ch->tx_pipe_index);
			continue;
		}

		old_write_avail = ch->write_avail;
		ch->write_avail = new_write_avail;

		if ((old_write_avail <= ch->min_write_avail) &&
			(new_write_avail >= ch->min_write_avail))
			tx_notify_bitmask |= (1<<ch->num);
	}

	if ((rx_notify_bitmask == 0) && (tx_notify_bitmask == 0))
		pr_debug(MODULE_MAME ":Nothing to Notify\n");
	else
		pr_info(MODULE_MAME ":Notify bitmask rx=0x%x, tx=0x%x.\n",
			rx_notify_bitmask, tx_notify_bitmask);

	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		struct sdio_channel *ch = &sdio_al->channel[i];

		if ((!ch->is_open) || (ch->notify == NULL))
			continue;

		if (rx_notify_bitmask & (1<<ch->num))
			ch->notify(ch->priv,
					   SDIO_EVENT_DATA_READ_AVAIL);

		if (tx_notify_bitmask & (1<<ch->num))
			ch->notify(ch->priv,
					   SDIO_EVENT_DATA_WRITE_AVAIL);
	}

	
	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		struct sdio_channel *ch = &sdio_al->channel[i];
		u32 pipe_thresh_intr_disabled = 0;

		if (!ch->is_open)
			continue;


		pipe_thresh_intr_disabled = thresh_intr_mask &
			(1<<ch->tx_pipe_index);
	}

	pr_debug(MODULE_MAME ":end %s.\n", __func__);

exit_err:
	if (!from_isr)
		sdio_release_host(sdio_al->card->sdio_func[0]);


	return ret;
}


static u32 check_pending_rx_packet(struct sdio_channel *ch, u32 eot)
{
	u32 rx_pending;
	u32 rx_avail;
	u32 new_packet_size = 0;

	mutex_lock(&ch->ch_lock);

	rx_pending = ch->rx_pending_bytes;
	rx_avail = sdio_al->mailbox->pipe_bytes_avail[ch->rx_pipe_index];

	pr_debug(MODULE_MAME ":pipe %d rx_avail=0x%x , rx_pending=0x%x\n",
	   ch->rx_pipe_index, rx_avail, rx_pending);


	
	if (eot & (1<<ch->rx_pipe_index)) {
		struct rx_packet_size *p = NULL;
		new_packet_size = rx_avail - rx_pending;

		if ((rx_avail <= rx_pending)) {
			pr_info(MODULE_MAME ":Invalid new packet size."
					    " rx_avail=%d.\n", rx_avail);
			new_packet_size = 0;
			goto exit_err;
		}

		p = kzalloc(sizeof(*p), GFP_KERNEL);
		if (p == NULL)
			goto exit_err;
		p->size = new_packet_size;
		
		list_add_tail(&p->list, &ch->rx_size_list_head);
		ch->rx_pending_bytes += new_packet_size;

		if (ch->read_avail == 0)
			ch->read_avail = new_packet_size;
	}

exit_err:
	mutex_unlock(&ch->ch_lock);

	return new_packet_size;
}




static u32 remove_handled_rx_packet(struct sdio_channel *ch)
{
	struct rx_packet_size *p = NULL;

	mutex_lock(&ch->ch_lock);

	ch->rx_pending_bytes -= ch->read_avail;

	if (!list_empty(&ch->rx_size_list_head)) {
		p = list_first_entry(&ch->rx_size_list_head,
			struct rx_packet_size, list);
		list_del(&p->list);
		kfree(p);
	}

	if (list_empty(&ch->rx_size_list_head))	{
		ch->read_avail = 0;
	} else {
		p = list_first_entry(&ch->rx_size_list_head,
			struct rx_packet_size, list);
		ch->read_avail = p->size;
	}

	mutex_unlock(&ch->ch_lock);

	return ch->read_avail;
}


static void worker(struct work_struct *work)
{
	int ret = 0;

	pr_debug(MODULE_MAME ":Worker Started..\n");
	while ((sdio_al->is_ready) && (ret == 0)) {
		pr_debug(MODULE_MAME ":Wait for read mailbox request..\n");
		wait_event(sdio_al->wait_mbox, sdio_al->ask_mbox);
		ret = read_mailbox(false);
		sdio_al->ask_mbox = false;
	}
	pr_debug(MODULE_MAME ":Worker Exit!\n");
}


static int sdio_write_cmd54(struct mmc_card *card, unsigned fn,
	unsigned addr, const u8 *buf,
	unsigned blocks, unsigned blksz)
{
	struct mmc_request mrq;
	struct mmc_command cmd;
	struct mmc_data data;
	struct scatterlist sg;
	int incr_addr = 1; 
	int write = 1;

	BUG_ON(!card);
	BUG_ON(fn > 7);
	BUG_ON(blocks == 1 && blksz > 512);
	WARN_ON(blocks == 0);
	WARN_ON(blksz == 0);

	write = true;
	pr_debug(MODULE_MAME ":sdio_write_cmd54()"
		"fn=%d,buf=0x%x,blocks=%d,blksz=%d\n",
		fn, (u32) buf, blocks, blksz);

	memset(&mrq, 0, sizeof(struct mmc_request));
	memset(&cmd, 0, sizeof(struct mmc_command));
	memset(&data, 0, sizeof(struct mmc_data));

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = SD_IO_RW_EXTENDED_QCOM;

	cmd.arg = write ? 0x80000000 : 0x00000000;
	cmd.arg |= fn << 28;
	cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
	cmd.arg |= addr << 9;
	if (blocks == 1 && blksz <= 512)
		cmd.arg |= (blksz == 512) ? 0 : blksz;  
	else
		cmd.arg |= 0x08000000 | blocks; 	
	cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;

	data.blksz = blksz;
	data.blocks = blocks;
	data.flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, buf, blksz * blocks);

	mmc_set_data_timeout(&data, card);

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;

	if (mmc_host_is_spi(card->host)) {
		
	} else {
		if (cmd.resp[0] & R5_ERROR)
			return -EIO;
		if (cmd.resp[0] & R5_FUNCTION_NUMBER)
			return -EINVAL;
		if (cmd.resp[0] & R5_OUT_OF_RANGE)
			return -ERANGE;
	}

	return 0;
}



static int sdio_ch_write(struct sdio_channel *ch, const u8 *buf, u32 len)
{
	int ret = 0;
	unsigned blksz = ch->func->cur_blksize;
	int blocks = len / blksz;
	int remain_bytes = len % blksz;
	struct mmc_card *card = NULL;
	u32 fn = ch->func->num;

	if (len == 0)
		return -EINVAL;

	card = ch->func->card;

	if (remain_bytes) {
		
		if (blocks)
			ret = sdio_memcpy_toio(ch->func, PIPE_TX_FIFO_ADDR,
					       (void *) buf, blocks*blksz);

		if (ret != 0)
			return ret;

		buf += (blocks*blksz);

		ret = sdio_write_cmd54(card, fn, PIPE_TX_FIFO_ADDR,
				buf, 1, remain_bytes);
	} else {
		ret = sdio_write_cmd54(card, fn, PIPE_TX_FIFO_ADDR,
				buf, blocks, blksz);
	}

	return ret;
}


static void set_default_channels_config(void)
{
   int i;

	
	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		struct sdio_channel *ch = &sdio_al->channel[i];

		ch->num = i;
		ch->read_threshold  = DEFAULT_READ_THRESHOLD;
		ch->write_threshold = DEFAULT_WRITE_THRESHOLD;
		ch->min_write_avail = DEFAULT_MIN_WRITE_THRESHOLD;
		if (sdio_al->use_irq)
			ch->poll_delay_msec = 0;
		else
			ch->poll_delay_msec = DEFAULT_POLL_DELAY_MSEC;
		ch->is_packet_mode = true;
		ch->peer_tx_buf_size = DEFAULT_PEER_TX_BUF_SIZE;
	}


	sdio_al->channel[0].name = "SDIO_RPC";
	sdio_al->channel[0].priority = SDIO_PRIORITY_HIGH;

	sdio_al->channel[1].name = "SDIO_RMNET_DATA";
	sdio_al->channel[1].priority = SDIO_PRIORITY_MED;
	sdio_al->channel[1].is_packet_mode = false;  
	sdio_al->channel[1].poll_delay_msec = 30;

	sdio_al->channel[1].read_threshold  = 14*1024;
	sdio_al->channel[1].write_threshold = 2*1024;
	sdio_al->channel[1].min_write_avail = 1600;

	sdio_al->channel[2].name = "SDIO_QMI";
	sdio_al->channel[2].priority = SDIO_PRIORITY_LOW;

	sdio_al->channel[3].name = "SDIO_DIAG";
	sdio_al->channel[3].priority = SDIO_PRIORITY_LOW;
}



static int read_sdioc_software_header(struct peer_sdioc_sw_header *header)
{
	int ret;
	struct sdio_func *func1 = sdio_al->card->sdio_func[0];

	pr_debug(MODULE_MAME ":reading sdioc sw header.\n");

	ret = sdio_memcpy_fromio(func1, header,
			SDIOC_SW_HEADER_ADDR, sizeof(*header));
	if (ret) {
		pr_info(MODULE_MAME ":fail to read sdioc sw header.\n");
		goto exit_err;
	}

	if (header->signature != (u32) PEER_SDIOC_SW_MAILBOX_SIGNATURE) {
		pr_info(MODULE_MAME ":sdioc sw invalid signature. 0x%x\n",
			header->signature);
		goto exit_err;
	}

	pr_info(MODULE_MAME ":SDIOC SW version 0x%x\n", header->version);

	return 0;

exit_err:
	memset(header, 0, sizeof(*header));

	return -1;
}


static int read_sdioc_channel_config(struct sdio_channel *ch)
{
	int ret;
	struct peer_sdioc_sw_mailbox *sw_mailbox = NULL;
	struct peer_sdioc_channel_config *ch_config = NULL;

	if (sdio_al->sdioc_sw_header->version == 0)
		return -1;

	pr_debug(MODULE_MAME ":reading sw mailbox %s channel.\n", ch->name);

	sw_mailbox = kzalloc(sizeof(*sw_mailbox), GFP_KERNEL);
	if (sw_mailbox == NULL)
		return -ENOMEM;

	ret = sdio_memcpy_fromio(ch->func, sw_mailbox,
			SDIOC_SW_MAILBOX_ADDR, sizeof(*sw_mailbox));
	if (ret) {
		pr_info(MODULE_MAME ":fail to read sw mailbox.\n");
		goto exit_err;
	}

	ch_config = &sw_mailbox->ch_config[ch->num];

	if (!ch_config->is_ready) {
		pr_info(MODULE_MAME ":sw mailbox channel not ready.\n");
		goto exit_err;
	}

	pr_info(MODULE_MAME ":ch %s max_rx_threshold=%d.\n",
		ch->name, ch_config->max_rx_threshold);
	pr_info(MODULE_MAME ":ch %s max_tx_threshold=%d.\n",
		ch->name, ch_config->max_tx_threshold);
	pr_info(MODULE_MAME ":ch %s tx_buf_size=%d.\n",
		ch->name, ch_config->tx_buf_size);

	
	ch_config->max_rx_threshold = (ch_config->max_rx_threshold * 9) / 10;
	
	ch_config->max_tx_threshold = (ch_config->max_tx_threshold * 5) / 10;

	ch->read_threshold = min(ch->read_threshold,
				 (int) ch_config->max_rx_threshold);

	ch->write_threshold = min(ch->write_threshold,
				 (int) ch_config->max_tx_threshold);

	if (ch->min_write_avail > ch->write_threshold)
		ch->min_write_avail = ch->write_threshold;

	ch->peer_tx_buf_size = ch_config->tx_buf_size;

	kfree(sw_mailbox);

	return 0;

exit_err:
	pr_info(MODULE_MAME ":Reading SW Mailbox error.\n");
	kfree(sw_mailbox);

	return -1;
}



static int enable_eot_interrupt(int pipe_index, int enable)
{
	int ret = 0;
	struct sdio_func *func1 = sdio_al->card->sdio_func[0];
	u32 mask;
	u32 pipe_mask;
	u32 addr;

	if (pipe_index < 8) {
		addr = PIPES_0_7_IRQ_MASK_ADDR;
		pipe_mask = (1<<pipe_index);
	} else {
		addr = PIPES_8_15_IRQ_MASK_ADDR;
		pipe_mask = (1<<(pipe_index-8));
	}

	mask = sdio_readl(func1, addr, &ret);
	if (ret) {
		pr_debug(MODULE_MAME ":enable_eot_interrupt fail\n");
		goto exit_err;
	}

	if (enable)
		mask &= (~pipe_mask); 
	else
		mask |= (pipe_mask);  

	sdio_writel(func1, mask, addr, &ret);

exit_err:
	return ret;
}


static int enable_threshold_interrupt(int pipe_index, int enable)
{
	int ret = 0;
	struct sdio_func *func1 = sdio_al->card->sdio_func[0];
	u32 mask;
	u32 pipe_mask;
	u32 addr;

	if (pipe_index < 8) {
		addr = PIPES_0_7_IRQ_MASK_ADDR;
		pipe_mask = (1<<pipe_index);
	} else {
		addr = PIPES_8_15_IRQ_MASK_ADDR;
		pipe_mask = (1<<(pipe_index-8));
	}

	mask = sdio_readl(func1, addr, &ret);
	if (ret) {
		pr_debug(MODULE_MAME ":enable_threshold_interrupt fail\n");
		goto exit_err;
	}

	pipe_mask = pipe_mask<<8; 
	if (enable)
		mask &= (~pipe_mask); 
	else
		mask |= (pipe_mask);  

	sdio_writel(func1, mask, addr, &ret);

exit_err:
	return ret;
}


static int set_pipe_threshold(int pipe_index, int threshold)
{
	int ret = 0;
	struct sdio_func *func1 = sdio_al->card->sdio_func[0];

	sdio_writel(func1, threshold,
			PIPES_THRESHOLD_ADDR+pipe_index*4, &ret);
	if (ret)
		pr_info(MODULE_MAME ":set_pipe_threshold err=%d\n", -ret);

	return ret;
}



static int open_channel(struct sdio_channel *ch)
{
	int ret = 0;
	int i;

	
	
	ch->func = sdio_al->card->sdio_func[ch->num+1];
	ch->rx_pipe_index = ch->num*2;
	ch->tx_pipe_index = ch->num*2+1;
	ch->signature = SDIO_AL_SIGNATURE;

	ch->total_rx_bytes = 0;
	ch->total_tx_bytes = 0;

	ch->write_avail = 0;
	ch->read_avail = 0;
	ch->rx_pending_bytes = 0;

	mutex_init(&ch->ch_lock);

	pr_debug(MODULE_MAME ":open_channel %s func#%d\n",
			 ch->name, ch->func->num);

	INIT_LIST_HEAD(&(ch->rx_size_list_head));

	
	for (i = 0; i < 10; i++) {
		ret = sdio_enable_func(ch->func);
		if (ret) {
			pr_info(MODULE_MAME ":retry enable ch %s func#%d\n",
					 ch->name, ch->func->num);
			msleep(1000);
		} else
			break;
	}
	if (ret) {
		pr_info(MODULE_MAME ":sdio_enable_func() err=%d\n", -ret);
		goto exit_err;
	}

	
	ret = sdio_set_block_size(ch->func, SDIO_AL_BLOCK_SIZE);
	if (ret) {
		pr_info(MODULE_MAME ":sdio_set_block_size() err=%d\n", -ret);
		goto exit_err;
	}

	ch->func->max_blksize = SDIO_AL_BLOCK_SIZE;

	sdio_set_drvdata(ch->func, ch);

	
	read_sdioc_channel_config(ch);

	
	ret = set_pipe_threshold(ch->rx_pipe_index, ch->read_threshold);
	if (ret)
		goto exit_err;
	ret = set_pipe_threshold(ch->tx_pipe_index, ch->write_threshold);
	if (ret)
		goto exit_err;

	
	if  ((ch->poll_delay_msec) && (sdio_al->poll_delay_msec == 0)) {
			sdio_al->poll_delay_msec = ch->poll_delay_msec;

			init_timer(&sdio_al->timer);
			sdio_al->timer.data = (unsigned long) sdio_al;
			sdio_al->timer.function = timer_handler;
			sdio_al->timer.expires = jiffies +
				msecs_to_jiffies(sdio_al->poll_delay_msec);
			add_timer(&sdio_al->timer);
	}

	
	ch->is_open = true;

	
	sdio_al->poll_delay_msec = get_min_poll_time_msec();

	
	enable_eot_interrupt(ch->rx_pipe_index, true);
	enable_eot_interrupt(ch->tx_pipe_index, true);

	enable_threshold_interrupt(ch->rx_pipe_index, true);
	enable_threshold_interrupt(ch->tx_pipe_index, true);

exit_err:

	return ret;
}


static int close_channel(struct sdio_channel *ch)
{
	int ret;

	enable_eot_interrupt(ch->rx_pipe_index, false);
	enable_eot_interrupt(ch->tx_pipe_index, false);

	enable_threshold_interrupt(ch->rx_pipe_index, false);
	enable_threshold_interrupt(ch->tx_pipe_index, false);

	ret = sdio_disable_func(ch->func);

	return ret;
}



static void ask_reading_mailbox(void)
{
	if (!sdio_al->ask_mbox) {
		pr_debug(MODULE_MAME ":ask_reading_mailbox\n");
		sdio_al->ask_mbox = true;
		wake_up(&sdio_al->wait_mbox);
	}
}


static void sdio_func_irq(struct sdio_func *func)
{
	pr_debug(MODULE_MAME ":start %s.\n", __func__);

	
	if (sdio_al->poll_delay_msec) {
		ulong expires =	jiffies +
			msecs_to_jiffies(sdio_al->poll_delay_msec);
		mod_timer(&sdio_al->timer, expires);
	}

	read_mailbox(true);

	pr_debug(MODULE_MAME ":end %s.\n", __func__);
}


static void timer_handler(unsigned long data)
{
	struct sdio_al *sdio_al = (struct sdio_al *) data;

	pr_debug(MODULE_MAME " Timer Expired\n");

	ask_reading_mailbox();

	
	if (sdio_al->poll_delay_msec) {
		sdio_al->timer.expires = jiffies +
			msecs_to_jiffies(sdio_al->poll_delay_msec);
		add_timer(&sdio_al->timer);
	}
}


static int sdio_al_setup(void)
{
	int ret = 0;
	struct mmc_card *card = sdio_al->card;
	struct sdio_func *func1;
	int i = 0;
	int fn = 0;

	pr_info(MODULE_MAME ":sdio_al_setup\n");

	if (card == NULL) {
		pr_info(MODULE_MAME ":No Card detected\n");
		return -ENODEV;
	}

	func1 = card->sdio_func[0];

	sdio_claim_host(sdio_al->card->sdio_func[0]);

	sdio_al->mailbox = kzalloc(sizeof(struct sdio_mailbox), GFP_KERNEL);
	if (sdio_al->mailbox == NULL)
		return -ENOMEM;

	sdio_al->sdioc_sw_header
		= kzalloc(sizeof(*sdio_al->sdioc_sw_header), GFP_KERNEL);
	if (sdio_al->sdioc_sw_header == NULL)
		return -ENOMEM;

	
	ret = sdio_enable_func(func1);
	if (ret) {
		pr_info(MODULE_MAME ":Fail to enable Func#%d\n", func1->num);
		goto exit_err;
	}

	
	ret = sdio_set_block_size(func1, SDIO_AL_BLOCK_SIZE);
	func1->max_blksize = SDIO_AL_BLOCK_SIZE;

	sdio_al->workqueue = create_singlethread_workqueue("sdio_al_wq");
	INIT_WORK(&sdio_al->work, worker);

	init_waitqueue_head(&sdio_al->wait_mbox);

	
	for (i = 0 ; i < SDIO_AL_MAX_PIPES; i++) {
		enable_eot_interrupt(i, false);
		enable_threshold_interrupt(i, false);
	}

	
	for (fn = 1 ; fn <= card->sdio_funcs; fn++)
		sdio_disable_func(card->sdio_func[fn-1]);

	if (sdio_al->use_irq) {
		sdio_set_drvdata(func1, sdio_al);

		ret = sdio_claim_irq(func1, sdio_func_irq);
		if (ret) {
			pr_info(MODULE_MAME ":Fail to claim IRQ\n");
			goto exit_err;
		}
	} else {
		pr_debug(MODULE_MAME ":Not using IRQ\n");
	}

	read_sdioc_software_header(sdio_al->sdioc_sw_header);
	pr_info(MODULE_MAME ":SDIO-AL SW version %s.\n", DRV_VERSION);

	sdio_release_host(sdio_al->card->sdio_func[0]);
	sdio_al->is_ready = true;

	
	queue_work(sdio_al->workqueue, &sdio_al->work);

	pr_debug(MODULE_MAME ":Ready.\n");

	return 0;

exit_err:
	sdio_release_host(sdio_al->card->sdio_func[0]);
	pr_err(MODULE_MAME ":Setup Failure.\n");

	return ret;
}


static void sdio_al_tear_down(void)
{
	if (sdio_al->is_ready) {
		struct sdio_func *func1;

		func1 = sdio_al->card->sdio_func[0];

		sdio_al->is_ready = false; 
		ask_reading_mailbox(); 
		msleep(100); 

		flush_workqueue(sdio_al->workqueue);
		destroy_workqueue(sdio_al->workqueue);

		sdio_claim_host(func1);
		sdio_release_irq(func1);
		sdio_disable_func(func1);
		sdio_release_host(func1);
	}
}


static struct sdio_channel *find_channel_by_name(const char *name)
{
	struct sdio_channel *ch = NULL;
	int i;

	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++)
		if (strcmp(sdio_al->channel[i].name, name) == 0) {
			ch = &sdio_al->channel[i];
			break;
		}

	WARN_ON(ch == NULL);

	return ch;
}


static int get_min_poll_time_msec(void)
{
	int i;
	int poll_delay_msec = 0x0FFFFFFF;

	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++)
		if ((sdio_al->channel[i].is_open) &&
		    (sdio_al->channel[i].poll_delay_msec > 0) &&
		    (sdio_al->channel[i].poll_delay_msec < poll_delay_msec))
			poll_delay_msec = sdio_al->channel[i].poll_delay_msec;

	if (poll_delay_msec == 0x0FFFFFFF)
		poll_delay_msec = 0;

	pr_debug(MODULE_MAME ":poll delay time is %d msec\n", poll_delay_msec);

	return poll_delay_msec;
}


int sdio_open(const char *name, struct sdio_channel **ret_ch, void *priv,
		 void (*notify)(void *priv, unsigned ch_event))
{
	int ret = 0;
	struct sdio_channel *ch = NULL;

	*ret_ch = NULL; 

	if (sdio_al == NULL) {
		pr_info(MODULE_MAME
			":Try to open ch %s before Module Init\n", name);
		return -ENODEV;
	}

	if (!sdio_al->is_ready) {
		ret = sdio_al_setup();
		if (ret)
			return -ENODEV;
	}

	ch = find_channel_by_name(name);
	if (ch == NULL) {
		pr_info(MODULE_MAME ":Can't find channel name %s\n", name);
		return -EINVAL;
	}

	if (ch->is_open) {
		pr_info(MODULE_MAME ":Channel already opened %s\n", name);
		return -EPERM;
	}

	ch->name = name;
	ch->notify = notify;
	ch->priv = priv;

	
	*ret_ch = ch;

	if (ch->is_suspend) {
		pr_info(MODULE_MAME ":Resume channel %s.\n", name);
		ch->is_suspend = false;
		ch->is_open = true;
		ask_reading_mailbox();
		return 0;
	}


	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = open_channel(ch);
	sdio_release_host(sdio_al->card->sdio_func[0]);

	if (ret)
		pr_info(MODULE_MAME ":sdio_open %s err=%d\n", name, -ret);
	else
		pr_info(MODULE_MAME ":sdio_open %s completed OK\n", name);

	
	if ((!ret) && (!sdio_al->use_irq))
		ask_reading_mailbox();

	return ret;
}
EXPORT_SYMBOL(sdio_open);


int sdio_close(struct sdio_channel *ch)
{
	int ret;

	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	if (!ch->is_open)
		return -EINVAL;

	pr_info(MODULE_MAME ":sdio_close %s\n", ch->name);

	
	ch->is_open = false;
	ch->is_suspend = true;

	ch->notify = NULL;

	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = close_channel(ch);
	sdio_release_host(sdio_al->card->sdio_func[0]);

	do
		ret = remove_handled_rx_packet(ch);
	while (ret > 0);

	if  (ch->poll_delay_msec > 0)
		sdio_al->poll_delay_msec = get_min_poll_time_msec();

	return ret;
}
EXPORT_SYMBOL(sdio_close);


int sdio_write_avail(struct sdio_channel *ch)
{
	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	pr_debug(MODULE_MAME ":sdio_write_avail %s 0x%x\n",
			 ch->name, ch->write_avail);

	return ch->write_avail;
}
EXPORT_SYMBOL(sdio_write_avail);


int sdio_read_avail(struct sdio_channel *ch)
{
	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	pr_debug(MODULE_MAME ":sdio_read_avail %s 0x%x\n",
			 ch->name, ch->read_avail);

	return ch->read_avail;

}
EXPORT_SYMBOL(sdio_read_avail);


int sdio_read(struct sdio_channel *ch, void *data, int len)
{
	int ret = 0;

	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	if (sdio_al->is_err) {
		pr_info(MODULE_MAME ":In Error state, ignore sdio_read\n");
		return -ENODEV;
	}

	if (!ch->is_open) {
		pr_info(MODULE_MAME ":reading from closed channel %s\n",
				 ch->name);
		return -EINVAL;
	}

	pr_info(MODULE_MAME ":start ch %s read %d avail %d.\n",
		ch->name, len, ch->read_avail);

	if ((ch->is_packet_mode) && (len != ch->read_avail)) {
		pr_info(MODULE_MAME ":sdio_read ch %s len != read_avail\n",
				 ch->name);
		return -EINVAL;
	}

	if (len > ch->read_avail) {
		pr_info(MODULE_MAME ":ERR ch %s read %d avail %d.\n",
				ch->name, len, ch->read_avail);
		return -ENOMEM;
	}

	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = sdio_memcpy_fromio(ch->func, data, PIPE_RX_FIFO_ADDR, len);

	if (ret)
		pr_info(MODULE_MAME ":sdio_read err=%d\n", -ret);

	
	if (ch->is_packet_mode)
		remove_handled_rx_packet(ch);
	else
		ch->read_avail -= len;

	ch->total_rx_bytes += len;
	pr_info(MODULE_MAME ":end ch %s read %d avail %d total %d.\n",
		ch->name, len, ch->read_avail, ch->total_rx_bytes);

	sdio_release_host(sdio_al->card->sdio_func[0]);

	if ((ch->read_avail == 0) &&
	    !((ch->is_packet_mode) && (sdio_al->use_irq)))
		ask_reading_mailbox();

	return ret;
}
EXPORT_SYMBOL(sdio_read);


int sdio_write(struct sdio_channel *ch, const void *data, int len)
{
	int ret = 0;

	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);
	WARN_ON(len > ch->write_avail);

	if (sdio_al->is_err) {
		pr_info(MODULE_MAME ":In Error state, ignore sdio_write\n");
		return -ENODEV;
	}

	if (!ch->is_open) {
		pr_info(MODULE_MAME ":writing to closed channel %s\n",
				 ch->name);
		return -EINVAL;
	}

	pr_info(MODULE_MAME ":start ch %s write %d avail %d.\n",
		ch->name, len, ch->write_avail);

	if (len > ch->write_avail) {
		pr_info(MODULE_MAME ":ERR ch %s write %d avail %d.\n",
				ch->name, len, ch->write_avail);
		return -ENOMEM;
	}

	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = sdio_ch_write(ch, data, len);

	ch->total_tx_bytes += len;
	pr_info(MODULE_MAME ":end ch %s write %d avail %d total %d.\n",
		ch->name, len, ch->write_avail, ch->total_tx_bytes);

	if (ret) {
		pr_info(MODULE_MAME ":sdio_write err=%d\n", -ret);
	} else {
		
		len = ROUND_UP(len, ch->peer_tx_buf_size);
		
		len = min(len, (int) ch->write_avail);
		ch->write_avail -= len;
	}

	sdio_release_host(sdio_al->card->sdio_func[0]);

	if (ch->write_avail < ch->min_write_avail)
		ask_reading_mailbox();

	return ret;
}
EXPORT_SYMBOL(sdio_write);


int sdio_set_write_threshold(struct sdio_channel *ch, int threshold)
{
	int ret;

	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	ch->write_threshold = threshold;

	pr_debug(MODULE_MAME ":sdio_set_write_threshold %s 0x%x\n",
			 ch->name, ch->write_threshold);

	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = set_pipe_threshold(ch->tx_pipe_index, ch->write_threshold);
	sdio_release_host(sdio_al->card->sdio_func[0]);

	return ret;
}
EXPORT_SYMBOL(sdio_set_write_threshold);


int sdio_set_read_threshold(struct sdio_channel *ch, int threshold)
{
	int ret;

	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	ch->read_threshold = threshold;

	pr_debug(MODULE_MAME ":sdio_set_write_threshold %s 0x%x\n",
			 ch->name, ch->read_threshold);

	sdio_claim_host(sdio_al->card->sdio_func[0]);
	ret = set_pipe_threshold(ch->rx_pipe_index, ch->read_threshold);
	sdio_release_host(sdio_al->card->sdio_func[0]);

	return ret;
}
EXPORT_SYMBOL(sdio_set_read_threshold);



int sdio_set_poll_time(struct sdio_channel *ch, int poll_delay_msec)
{
	BUG_ON(ch->signature != SDIO_AL_SIGNATURE);

	ch->poll_delay_msec = poll_delay_msec;

	
	sdio_al->poll_delay_msec = get_min_poll_time_msec();

	return sdio_al->poll_delay_msec;
}
EXPORT_SYMBOL(sdio_set_poll_time);


static void default_sdio_al_release(struct device *dev)
{
	pr_info(MODULE_MAME ":platform device released.\n");
}


static int mmc_probe(struct mmc_card *card)
{
    int ret = 0;
    int i;

	dev_info(&card->dev, "Probing..\n");

	if (!mmc_card_sdio(card))
		return -ENODEV;

	if (card->sdio_funcs < SDIO_AL_MAX_FUNCS) {
		dev_info(&card->dev,
			 "SDIO-functions# %d less than expected.\n",
			 card->sdio_funcs);
		return -ENODEV;
	}

	dev_info(&card->dev, "vendor_id = 0x%x, device_id = 0x%x\n",
			 card->cis.vendor, card->cis.device);

	dev_info(&card->dev, "SDIO Card claimed.\n");

	sdio_al->card = card;

	#ifdef DEBUG_SDIO_AL_UNIT_TEST
	pr_info(MODULE_MAME ":==== SDIO-AL UNIT-TEST ====\n");
	#else
	
	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++) {
		sdio_al->channel[i].pdev.name = sdio_al->channel[i].name;
		sdio_al->channel[i].pdev.dev.release = default_sdio_al_release;
		platform_device_register(&sdio_al->channel[i].pdev);
	}
	#endif

	return ret;
}


static void mmc_remove(struct mmc_card *card)
{
	#ifndef DEBUG_SDIO_AL_UNIT_TEST
	int i;

	for (i = 0; i < SDIO_AL_MAX_CHANNELS; i++)
		platform_device_unregister(&sdio_al->channel[i].pdev);
	#endif

	pr_info(MODULE_MAME ":sdio card removed.\n");
}

static struct mmc_driver mmc_driver = {
	.drv		= {
		.name   = "sdio_al",
	},
	.probe  	= mmc_probe,
	.remove 	= mmc_remove,
};


static int __init sdio_al_init(void)
{
	int ret = 0;

	pr_debug(MODULE_MAME ":sdio_al_init\n");

	sdio_al = kzalloc(sizeof(struct sdio_al), GFP_KERNEL);
	if (sdio_al == NULL)
		return -ENOMEM;

	sdio_al->is_ready = false;

	sdio_al->use_irq = true;

	sdio_al->signature = SDIO_AL_SIGNATURE;

	set_default_channels_config();

	ret = mmc_register_driver(&mmc_driver);

	return ret;
}


static void __exit sdio_al_exit(void)
{
	if (sdio_al == NULL)
		return;

	pr_debug(MODULE_MAME ":sdio_al_exit\n");

	sdio_al_tear_down();

	kfree(sdio_al->sdioc_sw_header);
	kfree(sdio_al->mailbox);
	kfree(sdio_al);

	mmc_unregister_driver(&mmc_driver);

	pr_debug(MODULE_MAME ":sdio_al_exit complete\n");
}

module_init(sdio_al_init);
module_exit(sdio_al_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SDIO Abstraction Layer");
MODULE_AUTHOR("Amir Samuelov <amirs@qualcomm.com>");
MODULE_VERSION(DRV_VERSION);

