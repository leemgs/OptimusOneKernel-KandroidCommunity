



#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/dma.h>
#include "../comedidev.h"

#include "8253.h"
#include "8255.h"
#include "comedi_fc.h"

#undef DEBUG


#ifdef DEBUG
#define DEBUG_PRINT(format, args...) printk("das16: " format, ## args)
#else
#define DEBUG_PRINT(format, args...)
#endif

#define DAS16_SIZE 20		
#define DAS16_DMA_SIZE 0xff00	








static const int sample_size = 2;	

#define DAS16_TRIG		0
#define DAS16_AI_LSB		0
#define DAS16_AI_MSB		1
#define DAS16_MUX		2
#define DAS16_DIO		3
#define DAS16_AO_LSB(x)	((x)?6:4)
#define DAS16_AO_MSB(x)	((x)?7:5)
#define DAS16_STATUS		8
#define   BUSY			(1<<7)
#define   UNIPOLAR			(1<<6)
#define   DAS16_MUXBIT			(1<<5)
#define   DAS16_INT			(1<<4)
#define DAS16_CONTROL		9
#define   DAS16_INTE			(1<<7)
#define   DAS16_IRQ(x)			(((x) & 0x7) << 4)
#define   DMA_ENABLE			(1<<2)
#define   PACING_MASK	0x3
#define   INT_PACER		0x03
#define   EXT_PACER			0x02
#define   DAS16_SOFT		0x00
#define DAS16_PACER		0x0A
#define   DAS16_CTR0			(1<<1)
#define   DAS16_TRIG0			(1<<0)
#define   BURST_LEN_BITS(x)			(((x) & 0xf) << 4)
#define DAS16_GAIN		0x0B
#define DAS16_CNTR0_DATA		0x0C
#define DAS16_CNTR1_DATA		0x0D
#define DAS16_CNTR2_DATA		0x0E
#define DAS16_CNTR_CONTROL	0x0F
#define   DAS16_TERM_CNT	0x00
#define   DAS16_ONE_SHOT	0x02
#define   DAS16_RATE_GEN	0x04
#define   DAS16_CNTR_LSB_MSB	0x30
#define   DAS16_CNTR0		0x00
#define   DAS16_CNTR1		0x40
#define   DAS16_CNTR2		0x80

#define DAS1600_CONV		0x404
#define   DAS1600_CONV_DISABLE		0x40
#define DAS1600_BURST		0x405
#define   DAS1600_BURST_VAL		0x40
#define DAS1600_ENABLE		0x406
#define   DAS1600_ENABLE_VAL		0x40
#define DAS1600_STATUS_B	0x407
#define   DAS1600_BME		0x40
#define   DAS1600_ME		0x20
#define   DAS1600_CD			0x10
#define   DAS1600_WS			0x02
#define   DAS1600_CLK_10MHZ		0x01

static const struct comedi_lrange range_das1x01_bip = { 4, {
							    BIP_RANGE(10),
							    BIP_RANGE(1),
							    BIP_RANGE(0.1),
							    BIP_RANGE(0.01),
							    }
};

static const struct comedi_lrange range_das1x01_unip = { 4, {
							     UNI_RANGE(10),
							     UNI_RANGE(1),
							     UNI_RANGE(0.1),
							     UNI_RANGE(0.01),
							     }
};

static const struct comedi_lrange range_das1x02_bip = { 4, {
							    BIP_RANGE(10),
							    BIP_RANGE(5),
							    BIP_RANGE(2.5),
							    BIP_RANGE(1.25),
							    }
};

static const struct comedi_lrange range_das1x02_unip = { 4, {
							     UNI_RANGE(10),
							     UNI_RANGE(5),
							     UNI_RANGE(2.5),
							     UNI_RANGE(1.25),
							     }
};

static const struct comedi_lrange range_das16jr = { 9, {
							
							BIP_RANGE(10),
							BIP_RANGE(5),
							BIP_RANGE(2.5),
							BIP_RANGE(1.25),
							BIP_RANGE(0.625),
							UNI_RANGE(10),
							UNI_RANGE(5),
							UNI_RANGE(2.5),
							UNI_RANGE(1.25),
							}
};

static const struct comedi_lrange range_das16jr_16 = { 8, {
							   BIP_RANGE(10),
							   BIP_RANGE(5),
							   BIP_RANGE(2.5),
							   BIP_RANGE(1.25),
							   UNI_RANGE(10),
							   UNI_RANGE(5),
							   UNI_RANGE(2.5),
							   UNI_RANGE(1.25),
							   }
};

static const int das16jr_gainlist[] = { 8, 0, 1, 2, 3, 4, 5, 6, 7 };
static const int das16jr_16_gainlist[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const int das1600_gainlist[] = { 0, 1, 2, 3 };

enum {
	das16_pg_none = 0,
	das16_pg_16jr,
	das16_pg_16jr_16,
	das16_pg_1601,
	das16_pg_1602,
};
static const int *const das16_gainlists[] = {
	NULL,
	das16jr_gainlist,
	das16jr_16_gainlist,
	das1600_gainlist,
	das1600_gainlist,
};

static const struct comedi_lrange *const das16_ai_uni_lranges[] = {
	&range_unknown,
	&range_das16jr,
	&range_das16jr_16,
	&range_das1x01_unip,
	&range_das1x02_unip,
};

static const struct comedi_lrange *const das16_ai_bip_lranges[] = {
	&range_unknown,
	&range_das16jr,
	&range_das16jr_16,
	&range_das1x01_bip,
	&range_das1x02_bip,
};

struct munge_info {
	uint8_t byte;
	unsigned have_byte:1;
};

static int das16_ao_winsn(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data);
static int das16_do_wbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data);
static int das16_di_rbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data);
static int das16_ai_rinsn(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data);

static int das16_cmd_test(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_cmd *cmd);
static int das16_cmd_exec(struct comedi_device *dev,
			  struct comedi_subdevice *s);
static int das16_cancel(struct comedi_device *dev, struct comedi_subdevice *s);
static void das16_ai_munge(struct comedi_device *dev,
			   struct comedi_subdevice *s, void *array,
			   unsigned int num_bytes,
			   unsigned int start_chan_index);

static void das16_reset(struct comedi_device *dev);
static irqreturn_t das16_dma_interrupt(int irq, void *d);
static void das16_timer_interrupt(unsigned long arg);
static void das16_interrupt(struct comedi_device *dev);

static unsigned int das16_set_pacer(struct comedi_device *dev, unsigned int ns,
				    int flags);
static int das1600_mode_detect(struct comedi_device *dev);
static unsigned int das16_suggest_transfer_size(struct comedi_device *dev,
						struct comedi_cmd cmd);

static void reg_dump(struct comedi_device *dev);

struct das16_board {
	const char *name;
	void *ai;
	unsigned int ai_nbits;
	unsigned int ai_speed;	
	unsigned int ai_pg;
	void *ao;
	unsigned int ao_nbits;
	void *di;
	void *do_;

	unsigned int i8255_offset;
	unsigned int i8254_offset;

	unsigned int size;
	unsigned int id;
};

static const struct das16_board das16_boards[] = {
	{
	 .name = "das-16",
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 15000,
	 .ai_pg = das16_pg_none,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x10,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0x00,
	 },
	{
	 .name = "das-16g",
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 15000,
	 .ai_pg = das16_pg_none,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x10,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0x00,
	 },
	{
	 .name = "das-16f",
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 8500,
	 .ai_pg = das16_pg_none,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x10,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0x00,
	 },
	{
	 .name = "cio-das16",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 20000,
	 .ai_pg = das16_pg_none,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x10,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0x80,
	 },
	{
	 .name = "cio-das16/f",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_none,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x10,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0x80,
	 },
	{
	 .name = "cio-das16/jr",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 7692,
	 .ai_pg = das16_pg_16jr,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x10,
	 .id = 0x00,
	 },
	{
	 .name = "pc104-das16jr",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 3300,
	 .ai_pg = das16_pg_16jr,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x10,
	 .id = 0x00,
	 },
	{
	 .name = "cio-das16jr/16",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 16,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_16jr_16,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x10,
	 .id = 0x00,
	 },
	{
	 .name = "pc104-das16jr/16",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 16,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_16jr_16,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x10,
	 .id = 0x00,
	 },
	{
	 .name = "das-1201",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 20000,
	 .ai_pg = das16_pg_none,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0x20,
	 },
	{
	 .name = "das-1202",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_none,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0x20,
	 },
	{
	 .name = "das-1401",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1601,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x0,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0		
	 },
	{
	 .name = "das-1402",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1602,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x0,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0		
	 },
	{
	 .name = "das-1601",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1601,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "das-1602",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1602,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1401/12",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 6250,
	 .ai_pg = das16_pg_1601,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1402/12",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 6250,
	 .ai_pg = das16_pg_1602,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1402/16",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 16,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1602,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1601/12",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 6250,
	 .ai_pg = das16_pg_1601,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1602/12",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1602,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das1602/16",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 16,
	 .ai_speed = 10000,
	 .ai_pg = das16_pg_1602,
	 .ao = das16_ao_winsn,
	 .ao_nbits = 12,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0x400,
	 .i8254_offset = 0x0c,
	 .size = 0x408,
	 .id = 0xc0},
	{
	 .name = "cio-das16/330",	
	 .ai = das16_ai_rinsn,
	 .ai_nbits = 12,
	 .ai_speed = 3030,
	 .ai_pg = das16_pg_16jr,
	 .ao = NULL,
	 .di = das16_di_rbits,
	 .do_ = das16_do_wbits,
	 .i8255_offset = 0,
	 .i8254_offset = 0x0c,
	 .size = 0x14,
	 .id = 0xf0},
#if 0
	{
	 .name = "das16/330i",	
	 },
	{
	 .name = "das16/jr/ctr5",	
	 },
	{
	 .name = "cio-das16/m1/16",	
	 },
#endif
};

static int das16_attach(struct comedi_device *dev, struct comedi_devconfig *it);
static int das16_detach(struct comedi_device *dev);
static struct comedi_driver driver_das16 = {
	.driver_name = "das16",
	.module = THIS_MODULE,
	.attach = das16_attach,
	.detach = das16_detach,
	.board_name = &das16_boards[0].name,
	.num_names = ARRAY_SIZE(das16_boards),
	.offset = sizeof(das16_boards[0]),
};

#define DAS16_TIMEOUT 1000


static inline int timer_period(void)
{
	return HZ / 20;
}

struct das16_private_struct {
	unsigned int ai_unipolar;	
	unsigned int ai_singleended;	
	unsigned int clockbase;	
	volatile unsigned int control_state;	
	volatile unsigned long adc_byte_count;	
	unsigned int divisor1;	
	unsigned int divisor2;	
	unsigned int dma_chan;	
	uint16_t *dma_buffer[2];
	dma_addr_t dma_buffer_addr[2];
	unsigned int current_buffer;
	volatile unsigned int dma_transfer_size;	
	
	struct comedi_lrange *user_ai_range_table;
	struct comedi_lrange *user_ao_range_table;

	struct timer_list timer;	
	volatile short timer_running;
	volatile short timer_mode;	
};
#define devpriv ((struct das16_private_struct *)(dev->private))
#define thisboard ((struct das16_board *)(dev->board_ptr))

static int das16_cmd_test(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_cmd *cmd)
{
	int err = 0, tmp;
	int gain, start_chan, i;
	int mask;

	
	tmp = cmd->start_src;
	cmd->start_src &= TRIG_NOW;
	if (!cmd->start_src || tmp != cmd->start_src)
		err++;

	tmp = cmd->scan_begin_src;
	mask = TRIG_FOLLOW;
	
	if (thisboard->size > 0x400)
		mask |= TRIG_TIMER | TRIG_EXT;
	cmd->scan_begin_src &= mask;
	if (!cmd->scan_begin_src || tmp != cmd->scan_begin_src)
		err++;

	tmp = cmd->convert_src;
	mask = TRIG_TIMER | TRIG_EXT;
	
	if (thisboard->size > 0x400)
		mask |= TRIG_NOW;
	cmd->convert_src &= mask;
	if (!cmd->convert_src || tmp != cmd->convert_src)
		err++;

	tmp = cmd->scan_end_src;
	cmd->scan_end_src &= TRIG_COUNT;
	if (!cmd->scan_end_src || tmp != cmd->scan_end_src)
		err++;

	tmp = cmd->stop_src;
	cmd->stop_src &= TRIG_COUNT | TRIG_NONE;
	if (!cmd->stop_src || tmp != cmd->stop_src)
		err++;

	if (err)
		return 1;

	
	if (cmd->scan_begin_src != TRIG_TIMER &&
	    cmd->scan_begin_src != TRIG_EXT &&
	    cmd->scan_begin_src != TRIG_FOLLOW)
		err++;
	if (cmd->convert_src != TRIG_TIMER &&
	    cmd->convert_src != TRIG_EXT && cmd->convert_src != TRIG_NOW)
		err++;
	if (cmd->stop_src != TRIG_NONE && cmd->stop_src != TRIG_COUNT)
		err++;

	
	if (cmd->scan_begin_src == TRIG_FOLLOW && cmd->convert_src == TRIG_NOW)
		err++;
	if (cmd->scan_begin_src != TRIG_FOLLOW && cmd->convert_src != TRIG_NOW)
		err++;

	if (err)
		return 2;

	
	if (cmd->start_arg != 0) {
		cmd->start_arg = 0;
		err++;
	}

	if (cmd->scan_begin_src == TRIG_FOLLOW) {
		
		if (cmd->scan_begin_arg != 0) {
			cmd->scan_begin_arg = 0;
			err++;
		}
	}

	if (cmd->scan_end_arg != cmd->chanlist_len) {
		cmd->scan_end_arg = cmd->chanlist_len;
		err++;
	}
	
	if (cmd->scan_begin_src == TRIG_TIMER) {
		if (cmd->scan_begin_arg <
		    thisboard->ai_speed * cmd->chanlist_len) {
			cmd->scan_begin_arg =
			    thisboard->ai_speed * cmd->chanlist_len;
			err++;
		}
	}
	if (cmd->convert_src == TRIG_TIMER) {
		if (cmd->convert_arg < thisboard->ai_speed) {
			cmd->convert_arg = thisboard->ai_speed;
			err++;
		}
	}

	if (cmd->stop_src == TRIG_NONE) {
		if (cmd->stop_arg != 0) {
			cmd->stop_arg = 0;
			err++;
		}
	}
	if (err)
		return 3;

	
	if (cmd->scan_begin_src == TRIG_TIMER) {
		unsigned int tmp = cmd->scan_begin_arg;
		
		i8253_cascade_ns_to_timer_2div(devpriv->clockbase,
					       &(devpriv->divisor1),
					       &(devpriv->divisor2),
					       &(cmd->scan_begin_arg),
					       cmd->flags & TRIG_ROUND_MASK);
		err += (tmp != cmd->scan_begin_arg);
	}
	if (cmd->convert_src == TRIG_TIMER) {
		unsigned int tmp = cmd->convert_arg;
		
		i8253_cascade_ns_to_timer_2div(devpriv->clockbase,
					       &(devpriv->divisor1),
					       &(devpriv->divisor2),
					       &(cmd->convert_arg),
					       cmd->flags & TRIG_ROUND_MASK);
		err += (tmp != cmd->convert_arg);
	}
	if (err)
		return 4;

	
	if (cmd->chanlist) {
		gain = CR_RANGE(cmd->chanlist[0]);
		start_chan = CR_CHAN(cmd->chanlist[0]);
		for (i = 1; i < cmd->chanlist_len; i++) {
			if (CR_CHAN(cmd->chanlist[i]) !=
			    (start_chan + i) % s->n_chan) {
				comedi_error(dev,
					     "entries in chanlist must be consecutive channels, counting upwards\n");
				err++;
			}
			if (CR_RANGE(cmd->chanlist[i]) != gain) {
				comedi_error(dev,
					     "entries in chanlist must all have the same gain\n");
				err++;
			}
		}
	}
	if (err)
		return 5;

	return 0;
}

static int das16_cmd_exec(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct comedi_async *async = s->async;
	struct comedi_cmd *cmd = &async->cmd;
	unsigned int byte;
	unsigned long flags;
	int range;

	if (devpriv->dma_chan == 0 || (dev->irq == 0
				       && devpriv->timer_mode == 0)) {
		comedi_error(dev,
			     "irq (or use of 'timer mode') dma required to execute comedi_cmd");
		return -1;
	}
	if (cmd->flags & TRIG_RT) {
		comedi_error(dev,
			     "isa dma transfers cannot be performed with TRIG_RT, aborting");
		return -1;
	}

	devpriv->adc_byte_count =
	    cmd->stop_arg * cmd->chanlist_len * sizeof(uint16_t);

	
	if (thisboard->size > 0x400) {
		outb(DAS1600_CONV_DISABLE, dev->iobase + DAS1600_CONV);
	}
	
	byte = CR_CHAN(cmd->chanlist[0]);
	byte |= CR_CHAN(cmd->chanlist[cmd->chanlist_len - 1]) << 4;
	outb(byte, dev->iobase + DAS16_MUX);

	
	if (thisboard->ai_pg != das16_pg_none) {
		range = CR_RANGE(cmd->chanlist[0]);
		outb((das16_gainlists[thisboard->ai_pg])[range],
		     dev->iobase + DAS16_GAIN);
	}

	
	cmd->convert_arg =
	    das16_set_pacer(dev, cmd->convert_arg,
			    cmd->flags & TRIG_ROUND_MASK);
	DEBUG_PRINT("pacer period: %d ns\n", cmd->convert_arg);

	
	byte = 0;
	
	if (thisboard->size > 0x400) {
		if (cmd->convert_src == TRIG_NOW) {
			outb(DAS1600_BURST_VAL, dev->iobase + DAS1600_BURST);
			
			byte |= BURST_LEN_BITS(cmd->chanlist_len - 1);
		} else {
			outb(0, dev->iobase + DAS1600_BURST);
		}
	}
	outb(byte, dev->iobase + DAS16_PACER);

	
	flags = claim_dma_lock();
	disable_dma(devpriv->dma_chan);
	
	clear_dma_ff(devpriv->dma_chan);
	devpriv->current_buffer = 0;
	set_dma_addr(devpriv->dma_chan,
		     devpriv->dma_buffer_addr[devpriv->current_buffer]);
	
	devpriv->dma_transfer_size = das16_suggest_transfer_size(dev, *cmd);
	set_dma_count(devpriv->dma_chan, devpriv->dma_transfer_size);
	enable_dma(devpriv->dma_chan);
	release_dma_lock(flags);

	
	if (devpriv->timer_mode) {
		devpriv->timer_running = 1;
		devpriv->timer.expires = jiffies + timer_period();
		add_timer(&devpriv->timer);
		devpriv->control_state &= ~DAS16_INTE;
	} else {
		
		outb(0x00, dev->iobase + DAS16_STATUS);
		
		devpriv->control_state |= DAS16_INTE;
	}
	devpriv->control_state |= DMA_ENABLE;
	devpriv->control_state &= ~PACING_MASK;
	if (cmd->convert_src == TRIG_EXT)
		devpriv->control_state |= EXT_PACER;
	else
		devpriv->control_state |= INT_PACER;
	outb(devpriv->control_state, dev->iobase + DAS16_CONTROL);

	
	if (thisboard->size > 0x400) {
		outb(0, dev->iobase + DAS1600_CONV);
	}

	return 0;
}

static int das16_cancel(struct comedi_device *dev, struct comedi_subdevice *s)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->spinlock, flags);
	
	devpriv->control_state &= ~DAS16_INTE & ~PACING_MASK & ~DMA_ENABLE;
	outb(devpriv->control_state, dev->iobase + DAS16_CONTROL);
	if (devpriv->dma_chan)
		disable_dma(devpriv->dma_chan);

	
	if (devpriv->timer_mode && devpriv->timer_running) {
		devpriv->timer_running = 0;
		del_timer(&devpriv->timer);
	}

	
	if (thisboard->size > 0x400) {
		outb(0, dev->iobase + DAS1600_BURST);
	}

	spin_unlock_irqrestore(&dev->spinlock, flags);

	return 0;
}

static void das16_reset(struct comedi_device *dev)
{
	outb(0, dev->iobase + DAS16_STATUS);
	outb(0, dev->iobase + DAS16_CONTROL);
	outb(0, dev->iobase + DAS16_PACER);
	outb(0, dev->iobase + DAS16_CNTR_CONTROL);
}

static int das16_ai_rinsn(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	int i, n;
	int range;
	int chan;
	int msb, lsb;

	
	devpriv->control_state &= ~DAS16_INTE & ~DMA_ENABLE & ~PACING_MASK;
	outb(devpriv->control_state, dev->iobase + DAS16_CONTROL);

	
	chan = CR_CHAN(insn->chanspec);
	chan |= CR_CHAN(insn->chanspec) << 4;
	outb(chan, dev->iobase + DAS16_MUX);

	
	if (thisboard->ai_pg != das16_pg_none) {
		range = CR_RANGE(insn->chanspec);
		outb((das16_gainlists[thisboard->ai_pg])[range],
		     dev->iobase + DAS16_GAIN);
	}

	for (n = 0; n < insn->n; n++) {
		
		outb_p(0, dev->iobase + DAS16_TRIG);

		for (i = 0; i < DAS16_TIMEOUT; i++) {
			if (!(inb(dev->iobase + DAS16_STATUS) & BUSY))
				break;
		}
		if (i == DAS16_TIMEOUT) {
			printk("das16: timeout\n");
			return -ETIME;
		}
		msb = inb(dev->iobase + DAS16_AI_MSB);
		lsb = inb(dev->iobase + DAS16_AI_LSB);
		if (thisboard->ai_nbits == 12) {
			data[n] = ((lsb >> 4) & 0xf) | (msb << 4);
		} else {
			data[n] = lsb | (msb << 8);
		}
	}

	return n;
}

static int das16_di_rbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	unsigned int bits;

	bits = inb(dev->iobase + DAS16_DIO) & 0xf;
	data[1] = bits;
	data[0] = 0;

	return 2;
}

static int das16_do_wbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	unsigned int wbits;

	
	data[0] &= 0xf;
	wbits = s->state;
	
	wbits &= ~data[0];
	
	wbits |= data[0] & data[1];
	s->state = wbits;
	data[1] = wbits;

	outb(s->state, dev->iobase + DAS16_DIO);

	return 2;
}

static int das16_ao_winsn(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	int i;
	int lsb, msb;
	int chan;

	chan = CR_CHAN(insn->chanspec);

	for (i = 0; i < insn->n; i++) {
		if (thisboard->ao_nbits == 12) {
			lsb = (data[i] << 4) & 0xff;
			msb = (data[i] >> 4) & 0xff;
		} else {
			lsb = data[i] & 0xff;
			msb = (data[i] >> 8) & 0xff;
		}
		outb(lsb, dev->iobase + DAS16_AO_LSB(chan));
		outb(msb, dev->iobase + DAS16_AO_MSB(chan));
	}

	return i;
}

static irqreturn_t das16_dma_interrupt(int irq, void *d)
{
	int status;
	struct comedi_device *dev = d;

	status = inb(dev->iobase + DAS16_STATUS);

	if ((status & DAS16_INT) == 0) {
		DEBUG_PRINT("spurious interrupt\n");
		return IRQ_NONE;
	}

	
	outb(0x00, dev->iobase + DAS16_STATUS);
	das16_interrupt(dev);
	return IRQ_HANDLED;
}

static void das16_timer_interrupt(unsigned long arg)
{
	struct comedi_device *dev = (struct comedi_device *)arg;

	das16_interrupt(dev);

	if (devpriv->timer_running)
		mod_timer(&devpriv->timer, jiffies + timer_period());
}


static int disable_dma_on_even(struct comedi_device *dev)
{
	int residue;
	int i;
	static const int disable_limit = 100;
	static const int enable_timeout = 100;
	disable_dma(devpriv->dma_chan);
	residue = get_dma_residue(devpriv->dma_chan);
	for (i = 0; i < disable_limit && (residue % 2); ++i) {
		int j;
		enable_dma(devpriv->dma_chan);
		for (j = 0; j < enable_timeout; ++j) {
			int new_residue;
			udelay(2);
			new_residue = get_dma_residue(devpriv->dma_chan);
			if (new_residue != residue)
				break;
		}
		disable_dma(devpriv->dma_chan);
		residue = get_dma_residue(devpriv->dma_chan);
	}
	if (i == disable_limit) {
		comedi_error(dev,
			     "failed to get an even dma transfer, could be trouble.");
	}
	return residue;
}

static void das16_interrupt(struct comedi_device *dev)
{
	unsigned long dma_flags, spin_flags;
	struct comedi_subdevice *s = dev->read_subdev;
	struct comedi_async *async;
	struct comedi_cmd *cmd;
	int num_bytes, residue;
	int buffer_index;

	if (dev->attached == 0) {
		comedi_error(dev, "premature interrupt");
		return;
	}
	
	async = s->async;
	cmd = &async->cmd;

	if (devpriv->dma_chan == 0) {
		comedi_error(dev, "interrupt with no dma channel?");
		return;
	}

	spin_lock_irqsave(&dev->spinlock, spin_flags);
	if ((devpriv->control_state & DMA_ENABLE) == 0) {
		spin_unlock_irqrestore(&dev->spinlock, spin_flags);
		DEBUG_PRINT("interrupt while dma disabled?\n");
		return;
	}

	dma_flags = claim_dma_lock();
	clear_dma_ff(devpriv->dma_chan);
	residue = disable_dma_on_even(dev);

	
	if (residue > devpriv->dma_transfer_size) {
		comedi_error(dev, "residue > transfer size!\n");
		async->events |= COMEDI_CB_ERROR | COMEDI_CB_EOA;
		num_bytes = 0;
	} else
		num_bytes = devpriv->dma_transfer_size - residue;

	if (cmd->stop_src == TRIG_COUNT && num_bytes >= devpriv->adc_byte_count) {
		num_bytes = devpriv->adc_byte_count;
		async->events |= COMEDI_CB_EOA;
	}

	buffer_index = devpriv->current_buffer;
	devpriv->current_buffer = (devpriv->current_buffer + 1) % 2;
	devpriv->adc_byte_count -= num_bytes;

	
	if (cmd->stop_src == TRIG_COUNT && devpriv->timer_mode == 0 &&
	    devpriv->dma_transfer_size > devpriv->adc_byte_count)
		devpriv->dma_transfer_size = devpriv->adc_byte_count;

	
	if ((async->events & COMEDI_CB_EOA) == 0) {
		set_dma_addr(devpriv->dma_chan,
			     devpriv->dma_buffer_addr[devpriv->current_buffer]);
		set_dma_count(devpriv->dma_chan, devpriv->dma_transfer_size);
		enable_dma(devpriv->dma_chan);
		
		if (thisboard->size > 0x400 && devpriv->timer_mode == 0) {
			outb(0x00, dev->iobase + DAS1600_CONV);
		}
	}
	release_dma_lock(dma_flags);

	spin_unlock_irqrestore(&dev->spinlock, spin_flags);

	cfc_write_array_to_buffer(s,
				  devpriv->dma_buffer[buffer_index], num_bytes);

	cfc_handle_events(dev, s);
}

static unsigned int das16_set_pacer(struct comedi_device *dev, unsigned int ns,
				    int rounding_flags)
{
	i8253_cascade_ns_to_timer_2div(devpriv->clockbase, &(devpriv->divisor1),
				       &(devpriv->divisor2), &ns,
				       rounding_flags & TRIG_ROUND_MASK);

	
	i8254_load(dev->iobase + DAS16_CNTR0_DATA, 0, 1, devpriv->divisor1, 2);
	i8254_load(dev->iobase + DAS16_CNTR0_DATA, 0, 2, devpriv->divisor2, 2);

	return ns;
}

static void reg_dump(struct comedi_device *dev)
{
	DEBUG_PRINT("********DAS1600 REGISTER DUMP********\n");
	DEBUG_PRINT("DAS16_MUX: %x\n", inb(dev->iobase + DAS16_MUX));
	DEBUG_PRINT("DAS16_DIO: %x\n", inb(dev->iobase + DAS16_DIO));
	DEBUG_PRINT("DAS16_STATUS: %x\n", inb(dev->iobase + DAS16_STATUS));
	DEBUG_PRINT("DAS16_CONTROL: %x\n", inb(dev->iobase + DAS16_CONTROL));
	DEBUG_PRINT("DAS16_PACER: %x\n", inb(dev->iobase + DAS16_PACER));
	DEBUG_PRINT("DAS16_GAIN: %x\n", inb(dev->iobase + DAS16_GAIN));
	DEBUG_PRINT("DAS16_CNTR_CONTROL: %x\n",
		    inb(dev->iobase + DAS16_CNTR_CONTROL));
	DEBUG_PRINT("DAS1600_CONV: %x\n", inb(dev->iobase + DAS1600_CONV));
	DEBUG_PRINT("DAS1600_BURST: %x\n", inb(dev->iobase + DAS1600_BURST));
	DEBUG_PRINT("DAS1600_ENABLE: %x\n", inb(dev->iobase + DAS1600_ENABLE));
	DEBUG_PRINT("DAS1600_STATUS_B: %x\n",
		    inb(dev->iobase + DAS1600_STATUS_B));
}

static int das16_probe(struct comedi_device *dev, struct comedi_devconfig *it)
{
	int status;
	int diobits;

	

	status = inb(dev->iobase + DAS16_STATUS);

	if ((status & UNIPOLAR)) {
		devpriv->ai_unipolar = 1;
	} else {
		devpriv->ai_unipolar = 0;
	}

	if ((status & DAS16_MUXBIT)) {
		devpriv->ai_singleended = 1;
	} else {
		devpriv->ai_singleended = 0;
	}

	

	diobits = inb(dev->iobase + DAS16_DIO) & 0xf0;

	printk(" id bits are 0x%02x\n", diobits);
	if (thisboard->id != diobits) {
		printk(" requested board's id bits are 0x%x (ignore)\n",
		       thisboard->id);
	}

	return 0;
}

static int das1600_mode_detect(struct comedi_device *dev)
{
	int status = 0;

	status = inb(dev->iobase + DAS1600_STATUS_B);

	if (status & DAS1600_CLK_10MHZ) {
		devpriv->clockbase = 100;
		printk(" 10MHz pacer clock\n");
	} else {
		devpriv->clockbase = 1000;
		printk(" 1MHz pacer clock\n");
	}

	reg_dump(dev);

	return 0;
}



static int das16_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	int ret;
	unsigned int irq;
	unsigned long iobase;
	unsigned int dma_chan;
	int timer_mode;
	unsigned long flags;
	struct comedi_krange *user_ai_range, *user_ao_range;

	iobase = it->options[0];
#if 0
	irq = it->options[1];
	timer_mode = it->options[8];
#endif
	
	irq = 0;
	timer_mode = 1;
	if (timer_mode)
		irq = 0;

	printk("comedi%d: das16:", dev->minor);

	
	if (it->options[3]) {
		if (it->options[3] != 0 &&
		    it->options[3] != 1 && it->options[3] != 10) {
			printk
			    ("\n Invalid option.  Master clock must be set to 1 or 10 (MHz)\n");
			return -EINVAL;
		}
	}

	ret = alloc_private(dev, sizeof(struct das16_private_struct));
	if (ret < 0)
		return ret;

	if (thisboard->size < 0x400) {
		printk(" 0x%04lx-0x%04lx\n", iobase, iobase + thisboard->size);
		if (!request_region(iobase, thisboard->size, "das16")) {
			printk(" I/O port conflict\n");
			return -EIO;
		}
	} else {
		printk(" 0x%04lx-0x%04lx 0x%04lx-0x%04lx\n",
		       iobase, iobase + 0x0f,
		       iobase + 0x400,
		       iobase + 0x400 + (thisboard->size & 0x3ff));
		if (!request_region(iobase, 0x10, "das16")) {
			printk(" I/O port conflict:  0x%04lx-0x%04lx\n",
			       iobase, iobase + 0x0f);
			return -EIO;
		}
		if (!request_region(iobase + 0x400, thisboard->size & 0x3ff,
				    "das16")) {
			release_region(iobase, 0x10);
			printk(" I/O port conflict:  0x%04lx-0x%04lx\n",
			       iobase + 0x400,
			       iobase + 0x400 + (thisboard->size & 0x3ff));
			return -EIO;
		}
	}

	dev->iobase = iobase;

	
	if (das16_probe(dev, it)) {
		printk(" id bits do not match selected board, aborting\n");
		return -EINVAL;
	}
	dev->board_name = thisboard->name;

	
	if (thisboard->size < 0x400) {
		if (it->options[3])
			devpriv->clockbase = 1000 / it->options[3];
		else
			devpriv->clockbase = 1000;	
	} else {
		das1600_mode_detect(dev);
	}

	
	if (irq > 1 && irq < 8) {
		ret = request_irq(irq, das16_dma_interrupt, 0, "das16", dev);

		if (ret < 0)
			return ret;
		dev->irq = irq;
		printk(" ( irq = %u )", irq);
	} else if (irq == 0) {
		printk(" ( no irq )");
	} else {
		printk(" invalid irq\n");
		return -EINVAL;
	}

	
	dma_chan = it->options[2];
	if (dma_chan == 1 || dma_chan == 3) {
		
		int i;
		for (i = 0; i < 2; i++) {
			devpriv->dma_buffer[i] = pci_alloc_consistent(NULL,
								      DAS16_DMA_SIZE,
								      &devpriv->
								      dma_buffer_addr
								      [i]);
			if (devpriv->dma_buffer[i] == NULL)
				return -ENOMEM;
		}
		if (request_dma(dma_chan, "das16")) {
			printk(" failed to allocate dma channel %i\n",
			       dma_chan);
			return -EINVAL;
		}
		devpriv->dma_chan = dma_chan;
		flags = claim_dma_lock();
		disable_dma(devpriv->dma_chan);
		set_dma_mode(devpriv->dma_chan, DMA_MODE_READ);
		release_dma_lock(flags);
		printk(" ( dma = %u)\n", dma_chan);
	} else if (dma_chan == 0) {
		printk(" ( no dma )\n");
	} else {
		printk(" invalid dma channel\n");
		return -EINVAL;
	}

	
	if (thisboard->ai_pg == das16_pg_none &&
	    (it->options[4] || it->options[5])) {
		
		devpriv->user_ai_range_table =
		    kmalloc(sizeof(struct comedi_lrange) +
			    sizeof(struct comedi_krange), GFP_KERNEL);
		
		devpriv->user_ai_range_table->length = 1;
		user_ai_range = devpriv->user_ai_range_table->range;
		user_ai_range->min = it->options[4];
		user_ai_range->max = it->options[5];
		user_ai_range->flags = UNIT_volt;
	}
	
	if (it->options[6] || it->options[7]) {
		
		devpriv->user_ao_range_table =
		    kmalloc(sizeof(struct comedi_lrange) +
			    sizeof(struct comedi_krange), GFP_KERNEL);
		
		devpriv->user_ao_range_table->length = 1;
		user_ao_range = devpriv->user_ao_range_table->range;
		user_ao_range->min = it->options[6];
		user_ao_range->max = it->options[7];
		user_ao_range->flags = UNIT_volt;
	}

	if (timer_mode) {
		init_timer(&(devpriv->timer));
		devpriv->timer.function = das16_timer_interrupt;
		devpriv->timer.data = (unsigned long)dev;
	}
	devpriv->timer_mode = timer_mode ? 1 : 0;

	ret = alloc_subdevices(dev, 5);
	if (ret < 0)
		return ret;

	s = dev->subdevices + 0;
	dev->read_subdev = s;
	
	if (thisboard->ai) {
		s->type = COMEDI_SUBD_AI;
		s->subdev_flags = SDF_READABLE | SDF_CMD_READ;
		if (devpriv->ai_singleended) {
			s->n_chan = 16;
			s->len_chanlist = 16;
			s->subdev_flags |= SDF_GROUND;
		} else {
			s->n_chan = 8;
			s->len_chanlist = 8;
			s->subdev_flags |= SDF_DIFF;
		}
		s->maxdata = (1 << thisboard->ai_nbits) - 1;
		if (devpriv->user_ai_range_table) {	
			s->range_table = devpriv->user_ai_range_table;
		} else if (devpriv->ai_unipolar) {
			s->range_table = das16_ai_uni_lranges[thisboard->ai_pg];
		} else {
			s->range_table = das16_ai_bip_lranges[thisboard->ai_pg];
		}
		s->insn_read = thisboard->ai;
		s->do_cmdtest = das16_cmd_test;
		s->do_cmd = das16_cmd_exec;
		s->cancel = das16_cancel;
		s->munge = das16_ai_munge;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = dev->subdevices + 1;
	
	if (thisboard->ao) {
		s->type = COMEDI_SUBD_AO;
		s->subdev_flags = SDF_WRITABLE;
		s->n_chan = 2;
		s->maxdata = (1 << thisboard->ao_nbits) - 1;
		if (devpriv->user_ao_range_table) {	
			s->range_table = devpriv->user_ao_range_table;
		} else {
			s->range_table = &range_unknown;
		}
		s->insn_write = thisboard->ao;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = dev->subdevices + 2;
	
	if (thisboard->di) {
		s->type = COMEDI_SUBD_DI;
		s->subdev_flags = SDF_READABLE;
		s->n_chan = 4;
		s->maxdata = 1;
		s->range_table = &range_digital;
		s->insn_bits = thisboard->di;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = dev->subdevices + 3;
	
	if (thisboard->do_) {
		s->type = COMEDI_SUBD_DO;
		s->subdev_flags = SDF_WRITABLE | SDF_READABLE;
		s->n_chan = 4;
		s->maxdata = 1;
		s->range_table = &range_digital;
		s->insn_bits = thisboard->do_;
		
		outb(s->state, dev->iobase + DAS16_DIO);
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = dev->subdevices + 4;
	
	if (thisboard->i8255_offset != 0) {
		subdev_8255_init(dev, s, NULL, (dev->iobase +
						thisboard->i8255_offset));
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	das16_reset(dev);
	
	devpriv->control_state = DAS16_IRQ(dev->irq);
	outb(devpriv->control_state, dev->iobase + DAS16_CONTROL);

	
	if (thisboard->size > 0x400) {
		outb(DAS1600_ENABLE_VAL, dev->iobase + DAS1600_ENABLE);
		outb(0, dev->iobase + DAS1600_CONV);
		outb(0, dev->iobase + DAS1600_BURST);
	}

	return 0;
}

static int das16_detach(struct comedi_device *dev)
{
	printk("comedi%d: das16: remove\n", dev->minor);

	das16_reset(dev);

	if (dev->subdevices)
		subdev_8255_cleanup(dev, dev->subdevices + 4);

	if (devpriv) {
		int i;
		for (i = 0; i < 2; i++) {
			if (devpriv->dma_buffer[i])
				pci_free_consistent(NULL, DAS16_DMA_SIZE,
						    devpriv->dma_buffer[i],
						    devpriv->
						    dma_buffer_addr[i]);
		}
		if (devpriv->dma_chan)
			free_dma(devpriv->dma_chan);
		if (devpriv->user_ai_range_table)
			kfree(devpriv->user_ai_range_table);
		if (devpriv->user_ao_range_table)
			kfree(devpriv->user_ao_range_table);
	}

	if (dev->irq)
		free_irq(dev->irq, dev);

	if (dev->iobase) {
		if (thisboard->size < 0x400) {
			release_region(dev->iobase, thisboard->size);
		} else {
			release_region(dev->iobase, 0x10);
			release_region(dev->iobase + 0x400,
				       thisboard->size & 0x3ff);
		}
	}

	return 0;
}

COMEDI_INITCLEANUP(driver_das16);


static unsigned int das16_suggest_transfer_size(struct comedi_device *dev,
						struct comedi_cmd cmd)
{
	unsigned int size;
	unsigned int freq;

	
	if (devpriv->timer_mode)
		return DAS16_DMA_SIZE;

	
	if (cmd.convert_src == TRIG_TIMER)
		freq = 1000000000 / cmd.convert_arg;
	else if (cmd.scan_begin_src == TRIG_TIMER)
		freq = (1000000000 / cmd.scan_begin_arg) * cmd.chanlist_len;
	
	else
		freq = 0xffffffff;

	if (cmd.flags & TRIG_WAKE_EOS) {
		size = sample_size * cmd.chanlist_len;
	} else {
		
		size = (freq / 3) * sample_size;
	}

	
	if (size > DAS16_DMA_SIZE)
		size = DAS16_DMA_SIZE - DAS16_DMA_SIZE % sample_size;
	else if (size < sample_size)
		size = sample_size;

	if (cmd.stop_src == TRIG_COUNT && size > devpriv->adc_byte_count)
		size = devpriv->adc_byte_count;

	return size;
}

static void das16_ai_munge(struct comedi_device *dev,
			   struct comedi_subdevice *s, void *array,
			   unsigned int num_bytes,
			   unsigned int start_chan_index)
{
	unsigned int i, num_samples = num_bytes / sizeof(short);
	short *data = array;

	for (i = 0; i < num_samples; i++) {
		data[i] = le16_to_cpu(data[i]);
		if (thisboard->ai_nbits == 12) {
			data[i] = (data[i] >> 4) & 0xfff;
		}
	}
}
