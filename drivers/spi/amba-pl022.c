



#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/amba/bus.h>
#include <linux/amba/pl022.h>
#include <linux/io.h>


#define SSP_WRITE_BITS(reg, val, mask, sb) \
 ((reg) = (((reg) & ~(mask)) | (((val)<<(sb)) & (mask))))


#define GEN_MASK_BITS(val, mask, sb) \
 (((val)<<(sb)) & (mask))

#define DRIVE_TX		0
#define DO_NOT_DRIVE_TX		1

#define DO_NOT_QUEUE_DMA	0
#define QUEUE_DMA		1

#define RX_TRANSFER		1
#define TX_TRANSFER		2


#define SSP_CR0(r)	(r + 0x000)
#define SSP_CR1(r)	(r + 0x004)
#define SSP_DR(r)	(r + 0x008)
#define SSP_SR(r)	(r + 0x00C)
#define SSP_CPSR(r)	(r + 0x010)
#define SSP_IMSC(r)	(r + 0x014)
#define SSP_RIS(r)	(r + 0x018)
#define SSP_MIS(r)	(r + 0x01C)
#define SSP_ICR(r)	(r + 0x020)
#define SSP_DMACR(r)	(r + 0x024)
#define SSP_ITCR(r)	(r + 0x080)
#define SSP_ITIP(r)	(r + 0x084)
#define SSP_ITOP(r)	(r + 0x088)
#define SSP_TDR(r)	(r + 0x08C)

#define SSP_PID0(r)	(r + 0xFE0)
#define SSP_PID1(r)	(r + 0xFE4)
#define SSP_PID2(r)	(r + 0xFE8)
#define SSP_PID3(r)	(r + 0xFEC)

#define SSP_CID0(r)	(r + 0xFF0)
#define SSP_CID1(r)	(r + 0xFF4)
#define SSP_CID2(r)	(r + 0xFF8)
#define SSP_CID3(r)	(r + 0xFFC)


#define SSP_CR0_MASK_DSS	(0x1FUL << 0)
#define SSP_CR0_MASK_HALFDUP	(0x1UL << 5)
#define SSP_CR0_MASK_SPO	(0x1UL << 6)
#define SSP_CR0_MASK_SPH	(0x1UL << 7)
#define SSP_CR0_MASK_SCR	(0xFFUL << 8)
#define SSP_CR0_MASK_CSS	(0x1FUL << 16)
#define SSP_CR0_MASK_FRF	(0x3UL << 21)


#define SSP_CR1_MASK_LBM	(0x1UL << 0)
#define SSP_CR1_MASK_SSE	(0x1UL << 1)
#define SSP_CR1_MASK_MS		(0x1UL << 2)
#define SSP_CR1_MASK_SOD	(0x1UL << 3)
#define SSP_CR1_MASK_RENDN	(0x1UL << 4)
#define SSP_CR1_MASK_TENDN	(0x1UL << 5)
#define SSP_CR1_MASK_MWAIT	(0x1UL << 6)
#define SSP_CR1_MASK_RXIFLSEL	(0x7UL << 7)
#define SSP_CR1_MASK_TXIFLSEL	(0x7UL << 10)


#define SSP_DR_MASK_DATA	0xFFFFFFFF


#define SSP_SR_MASK_TFE		(0x1UL << 0) 
#define SSP_SR_MASK_TNF		(0x1UL << 1) 
#define SSP_SR_MASK_RNE		(0x1UL << 2) 
#define SSP_SR_MASK_RFF 	(0x1UL << 3) 
#define SSP_SR_MASK_BSY		(0x1UL << 4) 


#define SSP_CPSR_MASK_CPSDVSR	(0xFFUL << 0)


#define SSP_IMSC_MASK_RORIM (0x1UL << 0) 
#define SSP_IMSC_MASK_RTIM  (0x1UL << 1) 
#define SSP_IMSC_MASK_RXIM  (0x1UL << 2) 
#define SSP_IMSC_MASK_TXIM  (0x1UL << 3) 



#define SSP_RIS_MASK_RORRIS		(0x1UL << 0)

#define SSP_RIS_MASK_RTRIS		(0x1UL << 1)

#define SSP_RIS_MASK_RXRIS		(0x1UL << 2)

#define SSP_RIS_MASK_TXRIS		(0x1UL << 3)



#define SSP_MIS_MASK_RORMIS		(0x1UL << 0)

#define SSP_MIS_MASK_RTMIS		(0x1UL << 1)

#define SSP_MIS_MASK_RXMIS		(0x1UL << 2)

#define SSP_MIS_MASK_TXMIS		(0x1UL << 3)



#define SSP_ICR_MASK_RORIC		(0x1UL << 0)

#define SSP_ICR_MASK_RTIC		(0x1UL << 1)



#define SSP_DMACR_MASK_RXDMAE		(0x1UL << 0)

#define SSP_DMACR_MASK_TXDMAE		(0x1UL << 1)


#define SSP_ITCR_MASK_ITEN		(0x1UL << 0)
#define SSP_ITCR_MASK_TESTFIFO		(0x1UL << 1)


#define ITIP_MASK_SSPRXD		 (0x1UL << 0)
#define ITIP_MASK_SSPFSSIN		 (0x1UL << 1)
#define ITIP_MASK_SSPCLKIN		 (0x1UL << 2)
#define ITIP_MASK_RXDMAC		 (0x1UL << 3)
#define ITIP_MASK_TXDMAC		 (0x1UL << 4)
#define ITIP_MASK_SSPTXDIN		 (0x1UL << 5)


#define ITOP_MASK_SSPTXD		 (0x1UL << 0)
#define ITOP_MASK_SSPFSSOUT		 (0x1UL << 1)
#define ITOP_MASK_SSPCLKOUT		 (0x1UL << 2)
#define ITOP_MASK_SSPOEn		 (0x1UL << 3)
#define ITOP_MASK_SSPCTLOEn		 (0x1UL << 4)
#define ITOP_MASK_RORINTR		 (0x1UL << 5)
#define ITOP_MASK_RTINTR		 (0x1UL << 6)
#define ITOP_MASK_RXINTR		 (0x1UL << 7)
#define ITOP_MASK_TXINTR		 (0x1UL << 8)
#define ITOP_MASK_INTR			 (0x1UL << 9)
#define ITOP_MASK_RXDMABREQ		 (0x1UL << 10)
#define ITOP_MASK_RXDMASREQ		 (0x1UL << 11)
#define ITOP_MASK_TXDMABREQ		 (0x1UL << 12)
#define ITOP_MASK_TXDMASREQ		 (0x1UL << 13)


#define TDR_MASK_TESTDATA 		(0xFFFFFFFF)


#define STATE_START                     ((void *) 0)
#define STATE_RUNNING                   ((void *) 1)
#define STATE_DONE                      ((void *) 2)
#define STATE_ERROR                     ((void *) -1)


#define QUEUE_RUNNING                   (0)
#define QUEUE_STOPPED                   (1)

#define SSP_DISABLED 			(0)
#define SSP_ENABLED 			(1)


#define SSP_DMA_DISABLED 		(0)
#define SSP_DMA_ENABLED 		(1)


#define NMDK_SSP_DEFAULT_CLKRATE 0x2
#define NMDK_SSP_DEFAULT_PRESCALE 0x40


#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF


#define DEFAULT_SSP_REG_IMSC  0x0UL
#define DISABLE_ALL_INTERRUPTS DEFAULT_SSP_REG_IMSC
#define ENABLE_ALL_INTERRUPTS (~DEFAULT_SSP_REG_IMSC)

#define CLEAR_ALL_INTERRUPTS  0x3



enum ssp_reading {
	READING_NULL,
	READING_U8,
	READING_U16,
	READING_U32
};


enum ssp_writing {
	WRITING_NULL,
	WRITING_U8,
	WRITING_U16,
	WRITING_U32
};


struct vendor_data {
	int fifodepth;
	int max_bpw;
	bool unidir;
};


struct pl022 {
	struct amba_device		*adev;
	struct vendor_data		*vendor;
	resource_size_t			phybase;
	void __iomem			*virtbase;
	struct clk			*clk;
	struct spi_master		*master;
	struct pl022_ssp_controller	*master_info;
	
	struct workqueue_struct		*workqueue;
	struct work_struct		pump_messages;
	spinlock_t			queue_lock;
	struct list_head		queue;
	int				busy;
	int				run;
	
	struct tasklet_struct		pump_transfers;
	struct spi_message		*cur_msg;
	struct spi_transfer		*cur_transfer;
	struct chip_data		*cur_chip;
	void				*tx;
	void				*tx_end;
	void				*rx;
	void				*rx_end;
	enum ssp_reading		read;
	enum ssp_writing		write;
};


struct chip_data {
	u16 cr0;
	u16 cr1;
	u16 dmacr;
	u16 cpsr;
	u8 n_bytes;
	u8 enable_dma:1;
	enum ssp_reading read;
	enum ssp_writing write;
	void (*cs_control) (u32 command);
	int xfer_type;
};


static void null_cs_control(u32 command)
{
	pr_debug("pl022: dummy chip select control, CS=0x%x\n", command);
}


static void giveback(struct pl022 *pl022)
{
	struct spi_transfer *last_transfer;
	unsigned long flags;
	struct spi_message *msg;
	void (*curr_cs_control) (u32 command);

	
	curr_cs_control = pl022->cur_chip->cs_control;
	spin_lock_irqsave(&pl022->queue_lock, flags);
	msg = pl022->cur_msg;
	pl022->cur_msg = NULL;
	pl022->cur_transfer = NULL;
	pl022->cur_chip = NULL;
	queue_work(pl022->workqueue, &pl022->pump_messages);
	spin_unlock_irqrestore(&pl022->queue_lock, flags);

	last_transfer = list_entry(msg->transfers.prev,
					struct spi_transfer,
					transfer_list);

	
	if (last_transfer->delay_usecs)
		
		udelay(last_transfer->delay_usecs);

	
	if (!last_transfer->cs_change)
		curr_cs_control(SSP_CHIP_DESELECT);
	else {
		struct spi_message *next_msg;

		

		
		spin_lock_irqsave(&pl022->queue_lock, flags);
		if (list_empty(&pl022->queue))
			next_msg = NULL;
		else
			next_msg = list_entry(pl022->queue.next,
					struct spi_message, queue);
		spin_unlock_irqrestore(&pl022->queue_lock, flags);

		
		if (next_msg && next_msg->spi != msg->spi)
			next_msg = NULL;
		if (!next_msg || msg->state == STATE_ERROR)
			curr_cs_control(SSP_CHIP_DESELECT);
	}
	msg->state = NULL;
	if (msg->complete)
		msg->complete(msg->context);
	
	clk_disable(pl022->clk);
}


static int flush(struct pl022 *pl022)
{
	unsigned long limit = loops_per_jiffy << 1;

	dev_dbg(&pl022->adev->dev, "flush\n");
	do {
		while (readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_RNE)
			readw(SSP_DR(pl022->virtbase));
	} while ((readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_BSY) && limit--);
	return limit;
}


static void restore_state(struct pl022 *pl022)
{
	struct chip_data *chip = pl022->cur_chip;

	writew(chip->cr0, SSP_CR0(pl022->virtbase));
	writew(chip->cr1, SSP_CR1(pl022->virtbase));
	writew(chip->dmacr, SSP_DMACR(pl022->virtbase));
	writew(chip->cpsr, SSP_CPSR(pl022->virtbase));
	writew(DISABLE_ALL_INTERRUPTS, SSP_IMSC(pl022->virtbase));
	writew(CLEAR_ALL_INTERRUPTS, SSP_ICR(pl022->virtbase));
}




#define DEFAULT_SSP_REG_CR0 ( \
	GEN_MASK_BITS(SSP_DATA_BITS_12, SSP_CR0_MASK_DSS, 0)	| \
	GEN_MASK_BITS(SSP_MICROWIRE_CHANNEL_FULL_DUPLEX, SSP_CR0_MASK_HALFDUP, 5) | \
	GEN_MASK_BITS(SSP_CLK_POL_IDLE_LOW, SSP_CR0_MASK_SPO, 6) | \
	GEN_MASK_BITS(SSP_CLK_SECOND_EDGE, SSP_CR0_MASK_SPH, 7) | \
	GEN_MASK_BITS(NMDK_SSP_DEFAULT_CLKRATE, SSP_CR0_MASK_SCR, 8) | \
	GEN_MASK_BITS(SSP_BITS_8, SSP_CR0_MASK_CSS, 16)	| \
	GEN_MASK_BITS(SSP_INTERFACE_MOTOROLA_SPI, SSP_CR0_MASK_FRF, 21) \
)

#define DEFAULT_SSP_REG_CR1 ( \
	GEN_MASK_BITS(LOOPBACK_DISABLED, SSP_CR1_MASK_LBM, 0) | \
	GEN_MASK_BITS(SSP_DISABLED, SSP_CR1_MASK_SSE, 1) | \
	GEN_MASK_BITS(SSP_MASTER, SSP_CR1_MASK_MS, 2) | \
	GEN_MASK_BITS(DO_NOT_DRIVE_TX, SSP_CR1_MASK_SOD, 3) | \
	GEN_MASK_BITS(SSP_RX_MSB, SSP_CR1_MASK_RENDN, 4) | \
	GEN_MASK_BITS(SSP_TX_MSB, SSP_CR1_MASK_TENDN, 5) | \
	GEN_MASK_BITS(SSP_MWIRE_WAIT_ZERO, SSP_CR1_MASK_MWAIT, 6) |\
	GEN_MASK_BITS(SSP_RX_1_OR_MORE_ELEM, SSP_CR1_MASK_RXIFLSEL, 7) | \
	GEN_MASK_BITS(SSP_TX_1_OR_MORE_EMPTY_LOC, SSP_CR1_MASK_TXIFLSEL, 10) \
)

#define DEFAULT_SSP_REG_CPSR ( \
	GEN_MASK_BITS(NMDK_SSP_DEFAULT_PRESCALE, SSP_CPSR_MASK_CPSDVSR, 0) \
)

#define DEFAULT_SSP_REG_DMACR (\
	GEN_MASK_BITS(SSP_DMA_DISABLED, SSP_DMACR_MASK_RXDMAE, 0) | \
	GEN_MASK_BITS(SSP_DMA_DISABLED, SSP_DMACR_MASK_TXDMAE, 1) \
)


static void load_ssp_default_config(struct pl022 *pl022)
{
	writew(DEFAULT_SSP_REG_CR0, SSP_CR0(pl022->virtbase));
	writew(DEFAULT_SSP_REG_CR1, SSP_CR1(pl022->virtbase));
	writew(DEFAULT_SSP_REG_DMACR, SSP_DMACR(pl022->virtbase));
	writew(DEFAULT_SSP_REG_CPSR, SSP_CPSR(pl022->virtbase));
	writew(DISABLE_ALL_INTERRUPTS, SSP_IMSC(pl022->virtbase));
	writew(CLEAR_ALL_INTERRUPTS, SSP_ICR(pl022->virtbase));
}


static void readwriter(struct pl022 *pl022)
{

	
	dev_dbg(&pl022->adev->dev,
		"%s, rx: %p, rxend: %p, tx: %p, txend: %p\n",
		__func__, pl022->rx, pl022->rx_end, pl022->tx, pl022->tx_end);

	
	while ((readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_RNE)
	       && (pl022->rx < pl022->rx_end)) {
		switch (pl022->read) {
		case READING_NULL:
			readw(SSP_DR(pl022->virtbase));
			break;
		case READING_U8:
			*(u8 *) (pl022->rx) =
				readw(SSP_DR(pl022->virtbase)) & 0xFFU;
			break;
		case READING_U16:
			*(u16 *) (pl022->rx) =
				(u16) readw(SSP_DR(pl022->virtbase));
			break;
		case READING_U32:
			*(u32 *) (pl022->rx) =
				readl(SSP_DR(pl022->virtbase));
			break;
		}
		pl022->rx += (pl022->cur_chip->n_bytes);
	}
	
	while ((readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_TNF)
	       && (pl022->tx < pl022->tx_end)) {
		switch (pl022->write) {
		case WRITING_NULL:
			writew(0x0, SSP_DR(pl022->virtbase));
			break;
		case WRITING_U8:
			writew(*(u8 *) (pl022->tx), SSP_DR(pl022->virtbase));
			break;
		case WRITING_U16:
			writew((*(u16 *) (pl022->tx)), SSP_DR(pl022->virtbase));
			break;
		case WRITING_U32:
			writel(*(u32 *) (pl022->tx), SSP_DR(pl022->virtbase));
			break;
		}
		pl022->tx += (pl022->cur_chip->n_bytes);
		
		while ((readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_RNE)
		       && (pl022->rx < pl022->rx_end)) {
			switch (pl022->read) {
			case READING_NULL:
				readw(SSP_DR(pl022->virtbase));
				break;
			case READING_U8:
				*(u8 *) (pl022->rx) =
					readw(SSP_DR(pl022->virtbase)) & 0xFFU;
				break;
			case READING_U16:
				*(u16 *) (pl022->rx) =
					(u16) readw(SSP_DR(pl022->virtbase));
				break;
			case READING_U32:
				*(u32 *) (pl022->rx) =
					readl(SSP_DR(pl022->virtbase));
				break;
			}
			pl022->rx += (pl022->cur_chip->n_bytes);
		}
	}
	
}



static void *next_transfer(struct pl022 *pl022)
{
	struct spi_message *msg = pl022->cur_msg;
	struct spi_transfer *trans = pl022->cur_transfer;

	
	if (trans->transfer_list.next != &msg->transfers) {
		pl022->cur_transfer =
		    list_entry(trans->transfer_list.next,
			       struct spi_transfer, transfer_list);
		return STATE_RUNNING;
	}
	return STATE_DONE;
}

static irqreturn_t pl022_interrupt_handler(int irq, void *dev_id)
{
	struct pl022 *pl022 = dev_id;
	struct spi_message *msg = pl022->cur_msg;
	u16 irq_status = 0;
	u16 flag = 0;

	if (unlikely(!msg)) {
		dev_err(&pl022->adev->dev,
			"bad message state in interrupt handler");
		
		return IRQ_HANDLED;
	}

	
	irq_status = readw(SSP_MIS(pl022->virtbase));

	if (unlikely(!irq_status))
		return IRQ_NONE;

	
	if (unlikely(irq_status & SSP_MIS_MASK_RORMIS)) {
		
		dev_err(&pl022->adev->dev,
			"FIFO overrun\n");
		if (readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_RFF)
			dev_err(&pl022->adev->dev,
				"RXFIFO is full\n");
		if (readw(SSP_SR(pl022->virtbase)) & SSP_SR_MASK_TNF)
			dev_err(&pl022->adev->dev,
				"TXFIFO is full\n");

		
		writew(DISABLE_ALL_INTERRUPTS,
		       SSP_IMSC(pl022->virtbase));
		writew(CLEAR_ALL_INTERRUPTS, SSP_ICR(pl022->virtbase));
		writew((readw(SSP_CR1(pl022->virtbase)) &
			(~SSP_CR1_MASK_SSE)), SSP_CR1(pl022->virtbase));
		msg->state = STATE_ERROR;

		
		tasklet_schedule(&pl022->pump_transfers);
		return IRQ_HANDLED;
	}

	readwriter(pl022);

	if ((pl022->tx == pl022->tx_end) && (flag == 0)) {
		flag = 1;
		
		writew(readw(SSP_IMSC(pl022->virtbase)) &
		       (~SSP_IMSC_MASK_TXIM),
		       SSP_IMSC(pl022->virtbase));
	}

	
	if (pl022->rx >= pl022->rx_end) {
		writew(DISABLE_ALL_INTERRUPTS,
		       SSP_IMSC(pl022->virtbase));
		writew(CLEAR_ALL_INTERRUPTS, SSP_ICR(pl022->virtbase));
		if (unlikely(pl022->rx > pl022->rx_end)) {
			dev_warn(&pl022->adev->dev, "read %u surplus "
				 "bytes (did you request an odd "
				 "number of bytes on a 16bit bus?)\n",
				 (u32) (pl022->rx - pl022->rx_end));
		}
		
		msg->actual_length += pl022->cur_transfer->len;
		if (pl022->cur_transfer->cs_change)
			pl022->cur_chip->
				cs_control(SSP_CHIP_DESELECT);
		
		msg->state = next_transfer(pl022);
		tasklet_schedule(&pl022->pump_transfers);
		return IRQ_HANDLED;
	}

	return IRQ_HANDLED;
}


static int set_up_next_transfer(struct pl022 *pl022,
				struct spi_transfer *transfer)
{
	int residue;

	
	residue = pl022->cur_transfer->len % pl022->cur_chip->n_bytes;
	if (unlikely(residue != 0)) {
		dev_err(&pl022->adev->dev,
			"message of %u bytes to transmit but the current "
			"chip bus has a data width of %u bytes!\n",
			pl022->cur_transfer->len,
			pl022->cur_chip->n_bytes);
		dev_err(&pl022->adev->dev, "skipping this message\n");
		return -EIO;
	}
	pl022->tx = (void *)transfer->tx_buf;
	pl022->tx_end = pl022->tx + pl022->cur_transfer->len;
	pl022->rx = (void *)transfer->rx_buf;
	pl022->rx_end = pl022->rx + pl022->cur_transfer->len;
	pl022->write =
	    pl022->tx ? pl022->cur_chip->write : WRITING_NULL;
	pl022->read = pl022->rx ? pl022->cur_chip->read : READING_NULL;
	return 0;
}


static void pump_transfers(unsigned long data)
{
	struct pl022 *pl022 = (struct pl022 *) data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;

	
	message = pl022->cur_msg;
	transfer = pl022->cur_transfer;

	
	if (message->state == STATE_ERROR) {
		message->status = -EIO;
		giveback(pl022);
		return;
	}

	
	if (message->state == STATE_DONE) {
		message->status = 0;
		giveback(pl022);
		return;
	}

	
	if (message->state == STATE_RUNNING) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);
		if (previous->delay_usecs)
			
			udelay(previous->delay_usecs);

		
		if (previous->cs_change)
			pl022->cur_chip->cs_control(SSP_CHIP_SELECT);
	} else {
		
		message->state = STATE_RUNNING;
	}

	if (set_up_next_transfer(pl022, transfer)) {
		message->state = STATE_ERROR;
		message->status = -EIO;
		giveback(pl022);
		return;
	}
	
	flush(pl022);
	writew(ENABLE_ALL_INTERRUPTS, SSP_IMSC(pl022->virtbase));
}


static int configure_dma(void *data)
{
	struct pl022 *pl022 = data;
	dev_dbg(&pl022->adev->dev, "configure DMA\n");
	return -ENOTSUPP;
}


static void do_dma_transfer(void *data)
{
	struct pl022 *pl022 = data;

	if (configure_dma(data)) {
		dev_dbg(&pl022->adev->dev, "configuration of DMA Failed!\n");
		goto err_config_dma;
	}

	

	
	pl022->cur_chip->cs_control(SSP_CHIP_SELECT);
	if (set_up_next_transfer(pl022, pl022->cur_transfer)) {
		
		pl022->cur_msg->state = STATE_ERROR;
		pl022->cur_msg->status = -EIO;
		giveback(pl022);
		return;
	}
	
	writew((readw(SSP_CR1(pl022->virtbase)) | SSP_CR1_MASK_SSE),
	       SSP_CR1(pl022->virtbase));

	
	return;

 err_config_dma:
	pl022->cur_msg->state = STATE_ERROR;
	pl022->cur_msg->status = -EIO;
	giveback(pl022);
	return;
}

static void do_interrupt_transfer(void *data)
{
	struct pl022 *pl022 = data;

	
	pl022->cur_chip->cs_control(SSP_CHIP_SELECT);
	if (set_up_next_transfer(pl022, pl022->cur_transfer)) {
		
		pl022->cur_msg->state = STATE_ERROR;
		pl022->cur_msg->status = -EIO;
		giveback(pl022);
		return;
	}
	
	writew((readw(SSP_CR1(pl022->virtbase)) | SSP_CR1_MASK_SSE),
	       SSP_CR1(pl022->virtbase));
	writew(ENABLE_ALL_INTERRUPTS, SSP_IMSC(pl022->virtbase));
}

static void do_polling_transfer(void *data)
{
	struct pl022 *pl022 = data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct chip_data *chip;

	chip = pl022->cur_chip;
	message = pl022->cur_msg;

	while (message->state != STATE_DONE) {
		
		if (message->state == STATE_ERROR)
			break;
		transfer = pl022->cur_transfer;

		
		if (message->state == STATE_RUNNING) {
			previous =
			    list_entry(transfer->transfer_list.prev,
				       struct spi_transfer, transfer_list);
			if (previous->delay_usecs)
				udelay(previous->delay_usecs);
			if (previous->cs_change)
				pl022->cur_chip->cs_control(SSP_CHIP_SELECT);
		} else {
			
			message->state = STATE_RUNNING;
			pl022->cur_chip->cs_control(SSP_CHIP_SELECT);
		}

		
		if (set_up_next_transfer(pl022, transfer)) {
			
			message->state = STATE_ERROR;
			break;
		}
		
		flush(pl022);
		writew((readw(SSP_CR1(pl022->virtbase)) | SSP_CR1_MASK_SSE),
		       SSP_CR1(pl022->virtbase));

		dev_dbg(&pl022->adev->dev, "POLLING TRANSFER ONGOING ... \n");
		
		while (pl022->tx < pl022->tx_end || pl022->rx < pl022->rx_end)
			readwriter(pl022);

		
		message->actual_length += pl022->cur_transfer->len;
		if (pl022->cur_transfer->cs_change)
			pl022->cur_chip->cs_control(SSP_CHIP_DESELECT);
		
		message->state = next_transfer(pl022);
	}

	
	if (message->state == STATE_DONE)
		message->status = 0;
	else
		message->status = -EIO;

	giveback(pl022);
	return;
}


static void pump_messages(struct work_struct *work)
{
	struct pl022 *pl022 =
		container_of(work, struct pl022, pump_messages);
	unsigned long flags;

	
	spin_lock_irqsave(&pl022->queue_lock, flags);
	if (list_empty(&pl022->queue) || pl022->run == QUEUE_STOPPED) {
		pl022->busy = 0;
		spin_unlock_irqrestore(&pl022->queue_lock, flags);
		return;
	}
	
	if (pl022->cur_msg) {
		spin_unlock_irqrestore(&pl022->queue_lock, flags);
		return;
	}
	
	pl022->cur_msg =
	    list_entry(pl022->queue.next, struct spi_message, queue);

	list_del_init(&pl022->cur_msg->queue);
	pl022->busy = 1;
	spin_unlock_irqrestore(&pl022->queue_lock, flags);

	
	pl022->cur_msg->state = STATE_START;
	pl022->cur_transfer = list_entry(pl022->cur_msg->transfers.next,
					    struct spi_transfer,
					    transfer_list);

	
	pl022->cur_chip = spi_get_ctldata(pl022->cur_msg->spi);
	
	clk_enable(pl022->clk);
	restore_state(pl022);
	flush(pl022);

	if (pl022->cur_chip->xfer_type == POLLING_TRANSFER)
		do_polling_transfer(pl022);
	else if (pl022->cur_chip->xfer_type == INTERRUPT_TRANSFER)
		do_interrupt_transfer(pl022);
	else
		do_dma_transfer(pl022);
}


static int __init init_queue(struct pl022 *pl022)
{
	INIT_LIST_HEAD(&pl022->queue);
	spin_lock_init(&pl022->queue_lock);

	pl022->run = QUEUE_STOPPED;
	pl022->busy = 0;

	tasklet_init(&pl022->pump_transfers,
			pump_transfers,	(unsigned long)pl022);

	INIT_WORK(&pl022->pump_messages, pump_messages);
	pl022->workqueue = create_singlethread_workqueue(
					dev_name(pl022->master->dev.parent));
	if (pl022->workqueue == NULL)
		return -EBUSY;

	return 0;
}


static int start_queue(struct pl022 *pl022)
{
	unsigned long flags;

	spin_lock_irqsave(&pl022->queue_lock, flags);

	if (pl022->run == QUEUE_RUNNING || pl022->busy) {
		spin_unlock_irqrestore(&pl022->queue_lock, flags);
		return -EBUSY;
	}

	pl022->run = QUEUE_RUNNING;
	pl022->cur_msg = NULL;
	pl022->cur_transfer = NULL;
	pl022->cur_chip = NULL;
	spin_unlock_irqrestore(&pl022->queue_lock, flags);

	queue_work(pl022->workqueue, &pl022->pump_messages);

	return 0;
}


static int stop_queue(struct pl022 *pl022)
{
	unsigned long flags;
	unsigned limit = 500;
	int status = 0;

	spin_lock_irqsave(&pl022->queue_lock, flags);

	
	pl022->run = QUEUE_STOPPED;
	while (!list_empty(&pl022->queue) && pl022->busy && limit--) {
		spin_unlock_irqrestore(&pl022->queue_lock, flags);
		msleep(10);
		spin_lock_irqsave(&pl022->queue_lock, flags);
	}

	if (!list_empty(&pl022->queue) || pl022->busy)
		status = -EBUSY;

	spin_unlock_irqrestore(&pl022->queue_lock, flags);

	return status;
}

static int destroy_queue(struct pl022 *pl022)
{
	int status;

	status = stop_queue(pl022);
	
	if (status != 0)
		return status;

	destroy_workqueue(pl022->workqueue);

	return 0;
}

static int verify_controller_parameters(struct pl022 *pl022,
					struct pl022_config_chip *chip_info)
{
	if ((chip_info->lbm != LOOPBACK_ENABLED)
	    && (chip_info->lbm != LOOPBACK_DISABLED)) {
		dev_err(chip_info->dev,
			"loopback Mode is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->iface < SSP_INTERFACE_MOTOROLA_SPI)
	    || (chip_info->iface > SSP_INTERFACE_UNIDIRECTIONAL)) {
		dev_err(chip_info->dev,
			"interface is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->iface == SSP_INTERFACE_UNIDIRECTIONAL) &&
	    (!pl022->vendor->unidir)) {
		dev_err(chip_info->dev,
			"unidirectional mode not supported in this "
			"hardware version\n");
		return -EINVAL;
	}
	if ((chip_info->hierarchy != SSP_MASTER)
	    && (chip_info->hierarchy != SSP_SLAVE)) {
		dev_err(chip_info->dev,
			"hierarchy is configured incorrectly\n");
		return -EINVAL;
	}
	if (((chip_info->clk_freq).cpsdvsr < CPSDVR_MIN)
	    || ((chip_info->clk_freq).cpsdvsr > CPSDVR_MAX)) {
		dev_err(chip_info->dev,
			"cpsdvsr is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->endian_rx != SSP_RX_MSB)
	    && (chip_info->endian_rx != SSP_RX_LSB)) {
		dev_err(chip_info->dev,
			"RX FIFO endianess is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->endian_tx != SSP_TX_MSB)
	    && (chip_info->endian_tx != SSP_TX_LSB)) {
		dev_err(chip_info->dev,
			"TX FIFO endianess is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->data_size < SSP_DATA_BITS_4)
	    || (chip_info->data_size > SSP_DATA_BITS_32)) {
		dev_err(chip_info->dev,
			"DATA Size is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->com_mode != INTERRUPT_TRANSFER)
	    && (chip_info->com_mode != DMA_TRANSFER)
	    && (chip_info->com_mode != POLLING_TRANSFER)) {
		dev_err(chip_info->dev,
			"Communication mode is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->rx_lev_trig < SSP_RX_1_OR_MORE_ELEM)
	    || (chip_info->rx_lev_trig > SSP_RX_32_OR_MORE_ELEM)) {
		dev_err(chip_info->dev,
			"RX FIFO Trigger Level is configured incorrectly\n");
		return -EINVAL;
	}
	if ((chip_info->tx_lev_trig < SSP_TX_1_OR_MORE_EMPTY_LOC)
	    || (chip_info->tx_lev_trig > SSP_TX_32_OR_MORE_EMPTY_LOC)) {
		dev_err(chip_info->dev,
			"TX FIFO Trigger Level is configured incorrectly\n");
		return -EINVAL;
	}
	if (chip_info->iface == SSP_INTERFACE_MOTOROLA_SPI) {
		if ((chip_info->clk_phase != SSP_CLK_FIRST_EDGE)
		    && (chip_info->clk_phase != SSP_CLK_SECOND_EDGE)) {
			dev_err(chip_info->dev,
				"Clock Phase is configured incorrectly\n");
			return -EINVAL;
		}
		if ((chip_info->clk_pol != SSP_CLK_POL_IDLE_LOW)
		    && (chip_info->clk_pol != SSP_CLK_POL_IDLE_HIGH)) {
			dev_err(chip_info->dev,
				"Clock Polarity is configured incorrectly\n");
			return -EINVAL;
		}
	}
	if (chip_info->iface == SSP_INTERFACE_NATIONAL_MICROWIRE) {
		if ((chip_info->ctrl_len < SSP_BITS_4)
		    || (chip_info->ctrl_len > SSP_BITS_32)) {
			dev_err(chip_info->dev,
				"CTRL LEN is configured incorrectly\n");
			return -EINVAL;
		}
		if ((chip_info->wait_state != SSP_MWIRE_WAIT_ZERO)
		    && (chip_info->wait_state != SSP_MWIRE_WAIT_ONE)) {
			dev_err(chip_info->dev,
				"Wait State is configured incorrectly\n");
			return -EINVAL;
		}
		if ((chip_info->duplex != SSP_MICROWIRE_CHANNEL_FULL_DUPLEX)
		    && (chip_info->duplex !=
			SSP_MICROWIRE_CHANNEL_HALF_DUPLEX)) {
			dev_err(chip_info->dev,
				"DUPLEX is configured incorrectly\n");
			return -EINVAL;
		}
	}
	if (chip_info->cs_control == NULL) {
		dev_warn(chip_info->dev,
			"Chip Select Function is NULL for this chip\n");
		chip_info->cs_control = null_cs_control;
	}
	return 0;
}


static int pl022_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct pl022 *pl022 = spi_master_get_devdata(spi->master);
	unsigned long flags;

	spin_lock_irqsave(&pl022->queue_lock, flags);

	if (pl022->run == QUEUE_STOPPED) {
		spin_unlock_irqrestore(&pl022->queue_lock, flags);
		return -ESHUTDOWN;
	}
	msg->actual_length = 0;
	msg->status = -EINPROGRESS;
	msg->state = STATE_START;

	list_add_tail(&msg->queue, &pl022->queue);
	if (pl022->run == QUEUE_RUNNING && !pl022->busy)
		queue_work(pl022->workqueue, &pl022->pump_messages);

	spin_unlock_irqrestore(&pl022->queue_lock, flags);
	return 0;
}

static int calculate_effective_freq(struct pl022 *pl022,
				    int freq,
				    struct ssp_clock_params *clk_freq)
{
	
	u16 cpsdvsr = 2;
	u16 scr = 0;
	bool freq_found = false;
	u32 rate;
	u32 max_tclk;
	u32 min_tclk;

	rate = clk_get_rate(pl022->clk);
	
	max_tclk = (rate / (CPSDVR_MIN * (1 + SCR_MIN)));
	
	min_tclk = (rate / (CPSDVR_MAX * (1 + SCR_MAX)));

	if ((freq <= max_tclk) && (freq >= min_tclk)) {
		while (cpsdvsr <= CPSDVR_MAX && !freq_found) {
			while (scr <= SCR_MAX && !freq_found) {
				if ((rate /
				     (cpsdvsr * (1 + scr))) > freq)
					scr += 1;
				else {
					
					freq_found = true;
					if ((rate /
					     (cpsdvsr * (1 + scr))) != freq) {
						if (scr == SCR_MIN) {
							cpsdvsr -= 2;
							scr = SCR_MAX;
						} else
							scr -= 1;
					}
				}
			}
			if (!freq_found) {
				cpsdvsr += 2;
				scr = SCR_MIN;
			}
		}
		if (cpsdvsr != 0) {
			dev_dbg(&pl022->adev->dev,
				"SSP Effective Frequency is %u\n",
				(rate / (cpsdvsr * (1 + scr))));
			clk_freq->cpsdvsr = (u8) (cpsdvsr & 0xFF);
			clk_freq->scr = (u8) (scr & 0xFF);
			dev_dbg(&pl022->adev->dev,
				"SSP cpsdvsr = %d, scr = %d\n",
				clk_freq->cpsdvsr, clk_freq->scr);
		}
	} else {
		dev_err(&pl022->adev->dev,
			"controller data is incorrect: out of range frequency");
		return -EINVAL;
	}
	return 0;
}


static int process_dma_info(struct pl022_config_chip *chip_info,
			    struct chip_data *chip)
{
	dev_err(chip_info->dev,
		"cannot process DMA info, DMA not implemented!\n");
	return -ENOTSUPP;
}




#define MODEBITS	(SPI_CPOL | SPI_CPHA | SPI_CS_HIGH \
			| SPI_LSB_FIRST | SPI_LOOP)

static int pl022_setup(struct spi_device *spi)
{
	struct pl022_config_chip *chip_info;
	struct chip_data *chip;
	int status = 0;
	struct pl022 *pl022 = spi_master_get_devdata(spi->master);

	if (spi->mode & ~MODEBITS) {
		dev_dbg(&spi->dev, "unsupported mode bits %x\n",
			spi->mode & ~MODEBITS);
		return -EINVAL;
	}

	if (!spi->max_speed_hz)
		return -EINVAL;

	
	chip = spi_get_ctldata(spi);

	if (chip == NULL) {
		chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
		if (!chip) {
			dev_err(&spi->dev,
				"cannot allocate controller state\n");
			return -ENOMEM;
		}
		dev_dbg(&spi->dev,
			"allocated memory for controller's runtime state\n");
	}

	
	chip_info = spi->controller_data;

	if (chip_info == NULL) {
		
		dev_dbg(&spi->dev,
			"using default controller_data settings\n");

		chip_info =
			kzalloc(sizeof(struct pl022_config_chip), GFP_KERNEL);

		if (!chip_info) {
			dev_err(&spi->dev,
				"cannot allocate controller data\n");
			status = -ENOMEM;
			goto err_first_setup;
		}

		dev_dbg(&spi->dev, "allocated memory for controller data\n");

		
		chip_info->dev = &spi->dev;
		
		chip_info->lbm = LOOPBACK_DISABLED;
		chip_info->com_mode = POLLING_TRANSFER;
		chip_info->iface = SSP_INTERFACE_MOTOROLA_SPI;
		chip_info->hierarchy = SSP_SLAVE;
		chip_info->slave_tx_disable = DO_NOT_DRIVE_TX;
		chip_info->endian_tx = SSP_TX_LSB;
		chip_info->endian_rx = SSP_RX_LSB;
		chip_info->data_size = SSP_DATA_BITS_12;
		chip_info->rx_lev_trig = SSP_RX_1_OR_MORE_ELEM;
		chip_info->tx_lev_trig = SSP_TX_1_OR_MORE_EMPTY_LOC;
		chip_info->clk_phase = SSP_CLK_SECOND_EDGE;
		chip_info->clk_pol = SSP_CLK_POL_IDLE_LOW;
		chip_info->ctrl_len = SSP_BITS_8;
		chip_info->wait_state = SSP_MWIRE_WAIT_ZERO;
		chip_info->duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX;
		chip_info->cs_control = null_cs_control;
	} else {
		dev_dbg(&spi->dev,
			"using user supplied controller_data settings\n");
	}

	
	if ((0 == chip_info->clk_freq.cpsdvsr)
	    && (0 == chip_info->clk_freq.scr)) {
		status = calculate_effective_freq(pl022,
						  spi->max_speed_hz,
						  &chip_info->clk_freq);
		if (status < 0)
			goto err_config_params;
	} else {
		if ((chip_info->clk_freq.cpsdvsr % 2) != 0)
			chip_info->clk_freq.cpsdvsr =
				chip_info->clk_freq.cpsdvsr - 1;
	}
	status = verify_controller_parameters(pl022, chip_info);
	if (status) {
		dev_err(&spi->dev, "controller data is incorrect");
		goto err_config_params;
	}
	
	chip->xfer_type = chip_info->com_mode;
	chip->cs_control = chip_info->cs_control;

	if (chip_info->data_size <= 8) {
		dev_dbg(&spi->dev, "1 <= n <=8 bits per word\n");
		chip->n_bytes = 1;
		chip->read = READING_U8;
		chip->write = WRITING_U8;
	} else if (chip_info->data_size <= 16) {
		dev_dbg(&spi->dev, "9 <= n <= 16 bits per word\n");
		chip->n_bytes = 2;
		chip->read = READING_U16;
		chip->write = WRITING_U16;
	} else {
		if (pl022->vendor->max_bpw >= 32) {
			dev_dbg(&spi->dev, "17 <= n <= 32 bits per word\n");
			chip->n_bytes = 4;
			chip->read = READING_U32;
			chip->write = WRITING_U32;
		} else {
			dev_err(&spi->dev,
				"illegal data size for this controller!\n");
			dev_err(&spi->dev,
				"a standard pl022 can only handle "
				"1 <= n <= 16 bit words\n");
			goto err_config_params;
		}
	}

	
	chip->cr0 = 0;
	chip->cr1 = 0;
	chip->dmacr = 0;
	chip->cpsr = 0;
	if ((chip_info->com_mode == DMA_TRANSFER)
	    && ((pl022->master_info)->enable_dma)) {
		chip->enable_dma = 1;
		dev_dbg(&spi->dev, "DMA mode set in controller state\n");
		status = process_dma_info(chip_info, chip);
		if (status < 0)
			goto err_config_params;
		SSP_WRITE_BITS(chip->dmacr, SSP_DMA_ENABLED,
			       SSP_DMACR_MASK_RXDMAE, 0);
		SSP_WRITE_BITS(chip->dmacr, SSP_DMA_ENABLED,
			       SSP_DMACR_MASK_TXDMAE, 1);
	} else {
		chip->enable_dma = 0;
		dev_dbg(&spi->dev, "DMA mode NOT set in controller state\n");
		SSP_WRITE_BITS(chip->dmacr, SSP_DMA_DISABLED,
			       SSP_DMACR_MASK_RXDMAE, 0);
		SSP_WRITE_BITS(chip->dmacr, SSP_DMA_DISABLED,
			       SSP_DMACR_MASK_TXDMAE, 1);
	}

	chip->cpsr = chip_info->clk_freq.cpsdvsr;

	SSP_WRITE_BITS(chip->cr0, chip_info->data_size, SSP_CR0_MASK_DSS, 0);
	SSP_WRITE_BITS(chip->cr0, chip_info->duplex, SSP_CR0_MASK_HALFDUP, 5);
	SSP_WRITE_BITS(chip->cr0, chip_info->clk_pol, SSP_CR0_MASK_SPO, 6);
	SSP_WRITE_BITS(chip->cr0, chip_info->clk_phase, SSP_CR0_MASK_SPH, 7);
	SSP_WRITE_BITS(chip->cr0, chip_info->clk_freq.scr, SSP_CR0_MASK_SCR, 8);
	SSP_WRITE_BITS(chip->cr0, chip_info->ctrl_len, SSP_CR0_MASK_CSS, 16);
	SSP_WRITE_BITS(chip->cr0, chip_info->iface, SSP_CR0_MASK_FRF, 21);
	SSP_WRITE_BITS(chip->cr1, chip_info->lbm, SSP_CR1_MASK_LBM, 0);
	SSP_WRITE_BITS(chip->cr1, SSP_DISABLED, SSP_CR1_MASK_SSE, 1);
	SSP_WRITE_BITS(chip->cr1, chip_info->hierarchy, SSP_CR1_MASK_MS, 2);
	SSP_WRITE_BITS(chip->cr1, chip_info->slave_tx_disable, SSP_CR1_MASK_SOD, 3);
	SSP_WRITE_BITS(chip->cr1, chip_info->endian_rx, SSP_CR1_MASK_RENDN, 4);
	SSP_WRITE_BITS(chip->cr1, chip_info->endian_tx, SSP_CR1_MASK_TENDN, 5);
	SSP_WRITE_BITS(chip->cr1, chip_info->wait_state, SSP_CR1_MASK_MWAIT, 6);
	SSP_WRITE_BITS(chip->cr1, chip_info->rx_lev_trig, SSP_CR1_MASK_RXIFLSEL, 7);
	SSP_WRITE_BITS(chip->cr1, chip_info->tx_lev_trig, SSP_CR1_MASK_TXIFLSEL, 10);

	
	spi_set_ctldata(spi, chip);
	return status;
 err_config_params:
 err_first_setup:
	kfree(chip);
	return status;
}


static void pl022_cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);

	spi_set_ctldata(spi, NULL);
	kfree(chip);
}


static int __init
pl022_probe(struct amba_device *adev, struct amba_id *id)
{
	struct device *dev = &adev->dev;
	struct pl022_ssp_controller *platform_info = adev->dev.platform_data;
	struct spi_master *master;
	struct pl022 *pl022 = NULL;	
	int status = 0;

	dev_info(&adev->dev,
		 "ARM PL022 driver, device ID: 0x%08x\n", adev->periphid);
	if (platform_info == NULL) {
		dev_err(&adev->dev, "probe - no platform data supplied\n");
		status = -ENODEV;
		goto err_no_pdata;
	}

	
	master = spi_alloc_master(dev, sizeof(struct pl022));
	if (master == NULL) {
		dev_err(&adev->dev, "probe - cannot alloc SPI master\n");
		status = -ENOMEM;
		goto err_no_master;
	}

	pl022 = spi_master_get_devdata(master);
	pl022->master = master;
	pl022->master_info = platform_info;
	pl022->adev = adev;
	pl022->vendor = id->data;

	
	master->bus_num = platform_info->bus_id;
	master->num_chipselect = platform_info->num_chipselect;
	master->cleanup = pl022_cleanup;
	master->setup = pl022_setup;
	master->transfer = pl022_transfer;

	dev_dbg(&adev->dev, "BUSNO: %d\n", master->bus_num);

	status = amba_request_regions(adev, NULL);
	if (status)
		goto err_no_ioregion;

	pl022->virtbase = ioremap(adev->res.start, resource_size(&adev->res));
	if (pl022->virtbase == NULL) {
		status = -ENOMEM;
		goto err_no_ioremap;
	}
	printk(KERN_INFO "pl022: mapped registers from 0x%08x to %p\n",
	       adev->res.start, pl022->virtbase);

	pl022->clk = clk_get(&adev->dev, NULL);
	if (IS_ERR(pl022->clk)) {
		status = PTR_ERR(pl022->clk);
		dev_err(&adev->dev, "could not retrieve SSP/SPI bus clock\n");
		goto err_no_clk;
	}

	
	clk_enable(pl022->clk);
	writew((readw(SSP_CR1(pl022->virtbase)) & (~SSP_CR1_MASK_SSE)),
	       SSP_CR1(pl022->virtbase));
	load_ssp_default_config(pl022);
	clk_disable(pl022->clk);

	status = request_irq(adev->irq[0], pl022_interrupt_handler, 0, "pl022",
			     pl022);
	if (status < 0) {
		dev_err(&adev->dev, "probe - cannot get IRQ (%d)\n", status);
		goto err_no_irq;
	}
	
	status = init_queue(pl022);
	if (status != 0) {
		dev_err(&adev->dev, "probe - problem initializing queue\n");
		goto err_init_queue;
	}
	status = start_queue(pl022);
	if (status != 0) {
		dev_err(&adev->dev, "probe - problem starting queue\n");
		goto err_start_queue;
	}
	
	amba_set_drvdata(adev, pl022);
	status = spi_register_master(master);
	if (status != 0) {
		dev_err(&adev->dev,
			"probe - problem registering spi master\n");
		goto err_spi_register;
	}
	dev_dbg(dev, "probe succeded\n");
	return 0;

 err_spi_register:
 err_start_queue:
 err_init_queue:
	destroy_queue(pl022);
	free_irq(adev->irq[0], pl022);
 err_no_irq:
	clk_put(pl022->clk);
 err_no_clk:
	iounmap(pl022->virtbase);
 err_no_ioremap:
	amba_release_regions(adev);
 err_no_ioregion:
	spi_master_put(master);
 err_no_master:
 err_no_pdata:
	return status;
}

static int __exit
pl022_remove(struct amba_device *adev)
{
	struct pl022 *pl022 = amba_get_drvdata(adev);
	int status = 0;
	if (!pl022)
		return 0;

	
	status = destroy_queue(pl022);
	if (status != 0) {
		dev_err(&adev->dev,
			"queue remove failed (%d)\n", status);
		return status;
	}
	load_ssp_default_config(pl022);
	free_irq(adev->irq[0], pl022);
	clk_disable(pl022->clk);
	clk_put(pl022->clk);
	iounmap(pl022->virtbase);
	amba_release_regions(adev);
	tasklet_disable(&pl022->pump_transfers);
	spi_unregister_master(pl022->master);
	spi_master_put(pl022->master);
	amba_set_drvdata(adev, NULL);
	dev_dbg(&adev->dev, "remove succeded\n");
	return 0;
}

#ifdef CONFIG_PM
static int pl022_suspend(struct amba_device *adev, pm_message_t state)
{
	struct pl022 *pl022 = amba_get_drvdata(adev);
	int status = 0;

	status = stop_queue(pl022);
	if (status) {
		dev_warn(&adev->dev, "suspend cannot stop queue\n");
		return status;
	}

	clk_enable(pl022->clk);
	load_ssp_default_config(pl022);
	clk_disable(pl022->clk);
	dev_dbg(&adev->dev, "suspended\n");
	return 0;
}

static int pl022_resume(struct amba_device *adev)
{
	struct pl022 *pl022 = amba_get_drvdata(adev);
	int status = 0;

	
	status = start_queue(pl022);
	if (status)
		dev_err(&adev->dev, "problem starting queue (%d)\n", status);
	else
		dev_dbg(&adev->dev, "resumed\n");

	return status;
}
#else
#define pl022_suspend NULL
#define pl022_resume NULL
#endif	

static struct vendor_data vendor_arm = {
	.fifodepth = 8,
	.max_bpw = 16,
	.unidir = false,
};


static struct vendor_data vendor_st = {
	.fifodepth = 32,
	.max_bpw = 32,
	.unidir = false,
};

static struct amba_id pl022_ids[] = {
	{
		
		.id	= 0x00041022,
		.mask	= 0x000fffff,
		.data	= &vendor_arm,
	},
	{
		
		.id	= 0x01080022,
		.mask	= 0xffffffff,
		.data	= &vendor_st,
	},
	{ 0, 0 },
};

static struct amba_driver pl022_driver = {
	.drv = {
		.name	= "ssp-pl022",
	},
	.id_table	= pl022_ids,
	.probe		= pl022_probe,
	.remove		= __exit_p(pl022_remove),
	.suspend        = pl022_suspend,
	.resume         = pl022_resume,
};


static int __init pl022_init(void)
{
	return amba_driver_register(&pl022_driver);
}

module_init(pl022_init);

static void __exit pl022_exit(void)
{
	amba_driver_unregister(&pl022_driver);
}

module_exit(pl022_exit);

MODULE_AUTHOR("Linus Walleij <linus.walleij@stericsson.com>");
MODULE_DESCRIPTION("PL022 SSP Controller Driver");
MODULE_LICENSE("GPL");
