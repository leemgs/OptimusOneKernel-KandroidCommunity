

















































#define FLOPPY_SANITY_CHECK
#undef  FLOPPY_SILENT_DCL_CLEAR

#define REALLY_SLOW_IO

#define DEBUGT 2
#define DCL_DEBUG	


static int print_unex = 1;
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#define FDPATCHES
#include <linux/fdreg.h>
#include <linux/fd.h>
#include <linux/hdreg.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/bio.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/mc146818rtc.h>	
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/buffer_head.h>	
#include <linux/mutex.h>


static int slow_floppy;

#include <asm/dma.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>

static int FLOPPY_IRQ = 6;
static int FLOPPY_DMA = 2;
static int can_use_virtual_dma = 2;


static int use_virtual_dma;


static DEFINE_SPINLOCK(floppy_lock);

static unsigned short virtual_dma_port = 0x3f0;
irqreturn_t floppy_interrupt(int irq, void *dev_id);
static int set_dor(int fdc, char mask, char data);

#define K_64	0x10000		



static int allowed_drive_mask = 0x33;

#include <asm/floppy.h>

static int irqdma_allocated;

#define DEVICE_NAME "floppy"

#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/cdrom.h>	
#include <linux/completion.h>

static struct request *current_req;
static struct request_queue *floppy_queue;
static void do_fd_request(struct request_queue * q);

#ifndef fd_get_dma_residue
#define fd_get_dma_residue() get_dma_residue(FLOPPY_DMA)
#endif



#ifndef fd_dma_mem_free
#define fd_dma_mem_free(addr, size) free_pages(addr, get_order(size))
#endif

#ifndef fd_dma_mem_alloc
#define fd_dma_mem_alloc(size) __get_dma_pages(GFP_KERNEL,get_order(size))
#endif

static inline void fallback_on_nodma_alloc(char **addr, size_t l)
{
#ifdef FLOPPY_CAN_FALLBACK_ON_NODMA
	if (*addr)
		return;		
	if (can_use_virtual_dma != 2)
		return;		
	printk("DMA memory shortage. Temporarily falling back on virtual DMA\n");
	*addr = (char *)nodma_mem_alloc(l);
#else
	return;
#endif
}



static unsigned long fake_change;
static int initialising = 1;

#define ITYPE(x) (((x)>>2) & 0x1f)
#define TOMINOR(x) ((x & 3) | ((x & 4) << 5))
#define UNIT(x) ((x) & 0x03)	
#define FDC(x) (((x) & 0x04) >> 2)	
	
#define REVDRIVE(fdc, unit) ((unit) + ((fdc) << 2))
#define DP (&drive_params[current_drive])
#define DRS (&drive_state[current_drive])
#define DRWE (&write_errors[current_drive])
#define FDCS (&fdc_state[fdc])
#define CLEARF(x) clear_bit(x##_BIT, &DRS->flags)
#define SETF(x) set_bit(x##_BIT, &DRS->flags)
#define TESTF(x) test_bit(x##_BIT, &DRS->flags)

#define UDP (&drive_params[drive])
#define UDRS (&drive_state[drive])
#define UDRWE (&write_errors[drive])
#define UFDCS (&fdc_state[FDC(drive)])
#define UCLEARF(x) clear_bit(x##_BIT, &UDRS->flags)
#define USETF(x) set_bit(x##_BIT, &UDRS->flags)
#define UTESTF(x) test_bit(x##_BIT, &UDRS->flags)

#define DPRINT(format, args...) printk(DEVICE_NAME "%d: " format, current_drive , ## args)

#define PH_HEAD(floppy,head) (((((floppy)->stretch & 2) >>1) ^ head) << 2)
#define STRETCH(floppy) ((floppy)->stretch & FD_STRETCH)

#define CLEARSTRUCT(x) memset((x), 0, sizeof(*(x)))


#define COMMAND raw_cmd->cmd[0]
#define DR_SELECT raw_cmd->cmd[1]
#define TRACK raw_cmd->cmd[2]
#define HEAD raw_cmd->cmd[3]
#define SECTOR raw_cmd->cmd[4]
#define SIZECODE raw_cmd->cmd[5]
#define SECT_PER_TRACK raw_cmd->cmd[6]
#define GAP raw_cmd->cmd[7]
#define SIZECODE2 raw_cmd->cmd[8]
#define NR_RW 9


#define F_SIZECODE raw_cmd->cmd[2]
#define F_SECT_PER_TRACK raw_cmd->cmd[3]
#define F_GAP raw_cmd->cmd[4]
#define F_FILL raw_cmd->cmd[5]
#define NR_F 6


#define MAX_DISK_SIZE 4		


#define MAX_REPLIES 16
static unsigned char reply_buffer[MAX_REPLIES];
static int inr;			
#define ST0 (reply_buffer[0])
#define ST1 (reply_buffer[1])
#define ST2 (reply_buffer[2])
#define ST3 (reply_buffer[0])	
#define R_TRACK (reply_buffer[3])
#define R_HEAD (reply_buffer[4])
#define R_SECTOR (reply_buffer[5])
#define R_SIZECODE (reply_buffer[6])
#define SEL_DLY (2*HZ/100)


static struct {
	struct floppy_drive_params params;
	const char *name;	
} default_drive_params[] = {

{{0,  500, 16, 16, 8000,    1*HZ, 3*HZ,  0, SEL_DLY, 5,  80, 3*HZ, 20, {3,1,2,0,2}, 0,
      0, { 7, 4, 8, 2, 1, 5, 3,10}, 3*HZ/2, 0 }, "unknown" },

{{1,  300, 16, 16, 8000,    1*HZ, 3*HZ,  0, SEL_DLY, 5,  40, 3*HZ, 17, {3,1,2,0,2}, 0,
      0, { 1, 0, 0, 0, 0, 0, 0, 0}, 3*HZ/2, 1 }, "360K PC" }, 

{{2,  500, 16, 16, 6000, 4*HZ/10, 3*HZ, 14, SEL_DLY, 6,  83, 3*HZ, 17, {3,1,2,0,2}, 0,
      0, { 2, 5, 6,23,10,20,12, 0}, 3*HZ/2, 2 }, "1.2M" }, 

{{3,  250, 16, 16, 3000,    1*HZ, 3*HZ,  0, SEL_DLY, 5,  83, 3*HZ, 20, {3,1,2,0,2}, 0,
      0, { 4,22,21,30, 3, 0, 0, 0}, 3*HZ/2, 4 }, "720k" }, 

{{4,  500, 16, 16, 4000, 4*HZ/10, 3*HZ, 10, SEL_DLY, 5,  83, 3*HZ, 20, {3,1,2,0,2}, 0,
      0, { 7, 4,25,22,31,21,29,11}, 3*HZ/2, 7 }, "1.44M" }, 

{{5, 1000, 15,  8, 3000, 4*HZ/10, 3*HZ, 10, SEL_DLY, 5,  83, 3*HZ, 40, {3,1,2,0,2}, 0,
      0, { 7, 8, 4,25,28,22,31,21}, 3*HZ/2, 8 }, "2.88M AMI BIOS" }, 

{{6, 1000, 15,  8, 3000, 4*HZ/10, 3*HZ, 10, SEL_DLY, 5,  83, 3*HZ, 40, {3,1,2,0,2}, 0,
      0, { 7, 8, 4,25,28,22,31,21}, 3*HZ/2, 8 }, "2.88M" } 

};

static struct floppy_drive_params drive_params[N_DRIVE];
static struct floppy_drive_struct drive_state[N_DRIVE];
static struct floppy_write_errors write_errors[N_DRIVE];
static struct timer_list motor_off_timer[N_DRIVE];
static struct gendisk *disks[N_DRIVE];
static struct block_device *opened_bdev[N_DRIVE];
static DEFINE_MUTEX(open_lock);
static struct floppy_raw_cmd *raw_cmd, default_raw_cmd;



static struct floppy_struct floppy_type[32] = {
	{    0, 0,0, 0,0,0x00,0x00,0x00,0x00,NULL    },	
	{  720, 9,2,40,0,0x2A,0x02,0xDF,0x50,"d360"  }, 
	{ 2400,15,2,80,0,0x1B,0x00,0xDF,0x54,"h1200" },	
	{  720, 9,1,80,0,0x2A,0x02,0xDF,0x50,"D360"  },	
	{ 1440, 9,2,80,0,0x2A,0x02,0xDF,0x50,"D720"  },	
	{  720, 9,2,40,1,0x23,0x01,0xDF,0x50,"h360"  },	
	{ 1440, 9,2,80,0,0x23,0x01,0xDF,0x50,"h720"  },	
	{ 2880,18,2,80,0,0x1B,0x00,0xCF,0x6C,"H1440" },	
	{ 5760,36,2,80,0,0x1B,0x43,0xAF,0x54,"E2880" },	
	{ 6240,39,2,80,0,0x1B,0x43,0xAF,0x28,"E3120" },	

	{ 2880,18,2,80,0,0x25,0x00,0xDF,0x02,"h1440" }, 
	{ 3360,21,2,80,0,0x1C,0x00,0xCF,0x0C,"H1680" }, 
	{  820,10,2,41,1,0x25,0x01,0xDF,0x2E,"h410"  },	
	{ 1640,10,2,82,0,0x25,0x02,0xDF,0x2E,"H820"  },	
	{ 2952,18,2,82,0,0x25,0x00,0xDF,0x02,"h1476" },	
	{ 3444,21,2,82,0,0x25,0x00,0xDF,0x0C,"H1722" },	
	{  840,10,2,42,1,0x25,0x01,0xDF,0x2E,"h420"  },	
	{ 1660,10,2,83,0,0x25,0x02,0xDF,0x2E,"H830"  },	
	{ 2988,18,2,83,0,0x25,0x00,0xDF,0x02,"h1494" },	
	{ 3486,21,2,83,0,0x25,0x00,0xDF,0x0C,"H1743" }, 

	{ 1760,11,2,80,0,0x1C,0x09,0xCF,0x00,"h880"  }, 
	{ 2080,13,2,80,0,0x1C,0x01,0xCF,0x00,"D1040" }, 
	{ 2240,14,2,80,0,0x1C,0x19,0xCF,0x00,"D1120" }, 
	{ 3200,20,2,80,0,0x1C,0x20,0xCF,0x2C,"h1600" }, 
	{ 3520,22,2,80,0,0x1C,0x08,0xCF,0x2e,"H1760" }, 
	{ 3840,24,2,80,0,0x1C,0x20,0xCF,0x00,"H1920" }, 
	{ 6400,40,2,80,0,0x25,0x5B,0xCF,0x00,"E3200" }, 
	{ 7040,44,2,80,0,0x25,0x5B,0xCF,0x00,"E3520" }, 
	{ 7680,48,2,80,0,0x25,0x63,0xCF,0x00,"E3840" }, 
	{ 3680,23,2,80,0,0x1C,0x10,0xCF,0x00,"H1840" }, 

	{ 1600,10,2,80,0,0x25,0x02,0xDF,0x2E,"D800"  },	
	{ 3200,20,2,80,0,0x1C,0x00,0xCF,0x2C,"H1600" }, 
};

#define SECTSIZE (_FD_SECTSIZE(*floppy))


static struct floppy_struct *current_type[N_DRIVE];


static struct floppy_struct user_params[N_DRIVE];

static sector_t floppy_sizes[256];

static char floppy_device_name[] = "floppy";


static int probing;


#define FD_COMMAND_NONE -1
#define FD_COMMAND_ERROR 2
#define FD_COMMAND_OKAY 3

static volatile int command_status = FD_COMMAND_NONE;
static unsigned long fdc_busy;
static DECLARE_WAIT_QUEUE_HEAD(fdc_wait);
static DECLARE_WAIT_QUEUE_HEAD(command_done);

#define NO_SIGNAL (!interruptible || !signal_pending(current))
#define CALL(x) if ((x) == -EINTR) return -EINTR
#define ECALL(x) if ((ret = (x))) return ret;
#define _WAIT(x,i) CALL(ret=wait_til_done((x),i))
#define WAIT(x) _WAIT((x),interruptible)
#define IWAIT(x) _WAIT((x),1)


static int format_errors;


static struct format_descr format_req;




static char *floppy_track_buffer;
static int max_buffer_sectors;

static int *errors;
typedef void (*done_f)(int);
static struct cont_t {
	void (*interrupt)(void);	
	void (*redo)(void);	
	void (*error)(void);	
	done_f done;		
} *cont;

static void floppy_ready(void);
static void floppy_start(void);
static void process_fd_request(void);
static void recalibrate_floppy(void);
static void floppy_shutdown(unsigned long);

static int floppy_request_regions(int);
static void floppy_release_regions(int);
static int floppy_grab_irq_and_dma(void);
static void floppy_release_irq_and_dma(void);


#define CHECK_RESET { if (FDCS->reset){ reset_fdc(); return; } }
static void reset_fdc(void);


#define NO_TRACK -1
#define NEED_1_RECAL -2
#define NEED_2_RECAL -3

static int usage_count;


static int buffer_track = -1;
static int buffer_drive = -1;
static int buffer_min = -1;
static int buffer_max = -1;


static struct floppy_fdc_state fdc_state[N_FDC];
static int fdc;			

static struct floppy_struct *_floppy = floppy_type;
static unsigned char current_drive;
static long current_count_sectors;
static unsigned char fsector_t;	
static unsigned char in_sector_offset;	

#ifndef fd_eject
static inline int fd_eject(int drive)
{
	return -EINVAL;
}
#endif


#ifdef DEBUGT
static long unsigned debugtimer;

static inline void set_debugt(void)
{
	debugtimer = jiffies;
}

static inline void debugt(const char *message)
{
	if (DP->flags & DEBUGT)
		printk("%s dtime=%lu\n", message, jiffies - debugtimer);
}
#else
static inline void set_debugt(void) { }
static inline void debugt(const char *message) { }
#endif 

typedef void (*timeout_fn) (unsigned long);
static DEFINE_TIMER(fd_timeout, floppy_shutdown, 0, 0);

static const char *timeout_message;

#ifdef FLOPPY_SANITY_CHECK
static void is_alive(const char *message)
{
	
	if (test_bit(0, &fdc_busy) && command_status < 2
	    && !timer_pending(&fd_timeout)) {
		DPRINT("timeout handler died: %s\n", message);
	}
}
#endif

static void (*do_floppy) (void) = NULL;

#ifdef FLOPPY_SANITY_CHECK

#define OLOGSIZE 20

static void (*lasthandler) (void);
static unsigned long interruptjiffies;
static unsigned long resultjiffies;
static int resultsize;
static unsigned long lastredo;

static struct output_log {
	unsigned char data;
	unsigned char status;
	unsigned long jiffies;
} output_log[OLOGSIZE];

static int output_log_pos;
#endif

#define current_reqD -1
#define MAXTIMEOUT -2

static void __reschedule_timeout(int drive, const char *message, int marg)
{
	if (drive == current_reqD)
		drive = current_drive;
	del_timer(&fd_timeout);
	if (drive < 0 || drive >= N_DRIVE) {
		fd_timeout.expires = jiffies + 20UL * HZ;
		drive = 0;
	} else
		fd_timeout.expires = jiffies + UDP->timeout;
	add_timer(&fd_timeout);
	if (UDP->flags & FD_DEBUG) {
		DPRINT("reschedule timeout ");
		printk(message, marg);
		printk("\n");
	}
	timeout_message = message;
}

static void reschedule_timeout(int drive, const char *message, int marg)
{
	unsigned long flags;

	spin_lock_irqsave(&floppy_lock, flags);
	__reschedule_timeout(drive, message, marg);
	spin_unlock_irqrestore(&floppy_lock, flags);
}

#define INFBOUND(a,b) (a)=max_t(int, a, b)
#define SUPBOUND(a,b) (a)=min_t(int, a, b)





static int disk_change(int drive)
{
	int fdc = FDC(drive);

#ifdef FLOPPY_SANITY_CHECK
	if (time_before(jiffies, UDRS->select_date + UDP->select_delay))
		DPRINT("WARNING disk change called early\n");
	if (!(FDCS->dor & (0x10 << UNIT(drive))) ||
	    (FDCS->dor & 3) != UNIT(drive) || fdc != FDC(drive)) {
		DPRINT("probing disk change on unselected drive\n");
		DPRINT("drive=%d fdc=%d dor=%x\n", drive, FDC(drive),
		       (unsigned int)FDCS->dor);
	}
#endif

#ifdef DCL_DEBUG
	if (UDP->flags & FD_DEBUG) {
		DPRINT("checking disk change line for drive %d\n", drive);
		DPRINT("jiffies=%lu\n", jiffies);
		DPRINT("disk change line=%x\n", fd_inb(FD_DIR) & 0x80);
		DPRINT("flags=%lx\n", UDRS->flags);
	}
#endif
	if (UDP->flags & FD_BROKEN_DCL)
		return UTESTF(FD_DISK_CHANGED);
	if ((fd_inb(FD_DIR) ^ UDP->flags) & 0x80) {
		USETF(FD_VERIFY);	
		if (UDRS->maxblock) {
			
			USETF(FD_DISK_CHANGED);
		}

		
		if (UDRS->keep_data >= 0) {
			if ((UDP->flags & FTD_MSG) &&
			    current_type[drive] != NULL)
				DPRINT("Disk type is undefined after "
				       "disk change\n");
			current_type[drive] = NULL;
			floppy_sizes[TOMINOR(drive)] = MAX_DISK_SIZE << 1;
		}

		return 1;
	} else {
		UDRS->last_checked = jiffies;
		UCLEARF(FD_DISK_NEWCHANGE);
	}
	return 0;
}

static inline int is_selected(int dor, int unit)
{
	return ((dor & (0x10 << unit)) && (dor & 3) == unit);
}

static int set_dor(int fdc, char mask, char data)
{
	unsigned char unit;
	unsigned char drive;
	unsigned char newdor;
	unsigned char olddor;

	if (FDCS->address == -1)
		return -1;

	olddor = FDCS->dor;
	newdor = (olddor & mask) | data;
	if (newdor != olddor) {
		unit = olddor & 0x3;
		if (is_selected(olddor, unit) && !is_selected(newdor, unit)) {
			drive = REVDRIVE(fdc, unit);
#ifdef DCL_DEBUG
			if (UDP->flags & FD_DEBUG) {
				DPRINT("calling disk change from set_dor\n");
			}
#endif
			disk_change(drive);
		}
		FDCS->dor = newdor;
		fd_outb(newdor, FD_DOR);

		unit = newdor & 0x3;
		if (!is_selected(olddor, unit) && is_selected(newdor, unit)) {
			drive = REVDRIVE(fdc, unit);
			UDRS->select_date = jiffies;
		}
	}
	return olddor;
}

static void twaddle(void)
{
	if (DP->select_delay)
		return;
	fd_outb(FDCS->dor & ~(0x10 << UNIT(current_drive)), FD_DOR);
	fd_outb(FDCS->dor, FD_DOR);
	DRS->select_date = jiffies;
}


static void reset_fdc_info(int mode)
{
	int drive;

	FDCS->spec1 = FDCS->spec2 = -1;
	FDCS->need_configure = 1;
	FDCS->perp_mode = 1;
	FDCS->rawcmd = 0;
	for (drive = 0; drive < N_DRIVE; drive++)
		if (FDC(drive) == fdc && (mode || UDRS->track != NEED_1_RECAL))
			UDRS->track = NEED_2_RECAL;
}


static void set_fdc(int drive)
{
	if (drive >= 0 && drive < N_DRIVE) {
		fdc = FDC(drive);
		current_drive = drive;
	}
	if (fdc != 1 && fdc != 0) {
		printk("bad fdc value\n");
		return;
	}
	set_dor(fdc, ~0, 8);
#if N_FDC > 1
	set_dor(1 - fdc, ~8, 0);
#endif
	if (FDCS->rawcmd == 2)
		reset_fdc_info(1);
	if (fd_inb(FD_STATUS) != STATUS_READY)
		FDCS->reset = 1;
}


static int _lock_fdc(int drive, int interruptible, int line)
{
	if (!usage_count) {
		printk(KERN_ERR
		       "Trying to lock fdc while usage count=0 at line %d\n",
		       line);
		return -1;
	}

	if (test_and_set_bit(0, &fdc_busy)) {
		DECLARE_WAITQUEUE(wait, current);
		add_wait_queue(&fdc_wait, &wait);

		for (;;) {
			set_current_state(TASK_INTERRUPTIBLE);

			if (!test_and_set_bit(0, &fdc_busy))
				break;

			schedule();

			if (!NO_SIGNAL) {
				remove_wait_queue(&fdc_wait, &wait);
				return -EINTR;
			}
		}

		set_current_state(TASK_RUNNING);
		remove_wait_queue(&fdc_wait, &wait);
		flush_scheduled_work();
	}
	command_status = FD_COMMAND_NONE;

	__reschedule_timeout(drive, "lock fdc", 0);
	set_fdc(drive);
	return 0;
}

#define lock_fdc(drive,interruptible) _lock_fdc(drive,interruptible, __LINE__)

#define LOCK_FDC(drive,interruptible) \
if (lock_fdc(drive,interruptible)) return -EINTR;


static inline void unlock_fdc(void)
{
	unsigned long flags;

	raw_cmd = NULL;
	if (!test_bit(0, &fdc_busy))
		DPRINT("FDC access conflict!\n");

	if (do_floppy)
		DPRINT("device interrupt still active at FDC release: %p!\n",
		       do_floppy);
	command_status = FD_COMMAND_NONE;
	spin_lock_irqsave(&floppy_lock, flags);
	del_timer(&fd_timeout);
	cont = NULL;
	clear_bit(0, &fdc_busy);
	if (current_req || blk_peek_request(floppy_queue))
		do_fd_request(floppy_queue);
	spin_unlock_irqrestore(&floppy_lock, flags);
	wake_up(&fdc_wait);
}


static void motor_off_callback(unsigned long nr)
{
	unsigned char mask = ~(0x10 << UNIT(nr));

	set_dor(FDC(nr), mask, 0);
}


static void floppy_off(unsigned int drive)
{
	unsigned long volatile delta;
	int fdc = FDC(drive);

	if (!(FDCS->dor & (0x10 << UNIT(drive))))
		return;

	del_timer(motor_off_timer + drive);

	
	if (UDP->rps) {
		delta = jiffies - UDRS->first_read_date + HZ -
		    UDP->spindown_offset;
		delta = ((delta * UDP->rps) % HZ) / UDP->rps;
		motor_off_timer[drive].expires =
		    jiffies + UDP->spindown - delta;
	}
	add_timer(motor_off_timer + drive);
}


static void scandrives(void)
{
	int i;
	int drive;
	int saved_drive;

	if (DP->select_delay)
		return;

	saved_drive = current_drive;
	for (i = 0; i < N_DRIVE; i++) {
		drive = (saved_drive + i + 1) % N_DRIVE;
		if (UDRS->fd_ref == 0 || UDP->select_delay != 0)
			continue;	
		set_fdc(drive);
		if (!(set_dor(fdc, ~3, UNIT(drive) | (0x10 << UNIT(drive))) &
		      (0x10 << UNIT(drive))))
			
			set_dor(fdc, ~(0x10 << UNIT(drive)), 0);
	}
	set_fdc(saved_drive);
}

static void empty(void)
{
}

static DECLARE_WORK(floppy_work, NULL);

static void schedule_bh(void (*handler) (void))
{
	PREPARE_WORK(&floppy_work, (work_func_t)handler);
	schedule_work(&floppy_work);
}

static DEFINE_TIMER(fd_timer, NULL, 0, 0);

static void cancel_activity(void)
{
	unsigned long flags;

	spin_lock_irqsave(&floppy_lock, flags);
	do_floppy = NULL;
	PREPARE_WORK(&floppy_work, (work_func_t)empty);
	del_timer(&fd_timer);
	spin_unlock_irqrestore(&floppy_lock, flags);
}


static void fd_watchdog(void)
{
#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("calling disk change from watchdog\n");
	}
#endif

	if (disk_change(current_drive)) {
		DPRINT("disk removed during i/o\n");
		cancel_activity();
		cont->done(0);
		reset_fdc();
	} else {
		del_timer(&fd_timer);
		fd_timer.function = (timeout_fn) fd_watchdog;
		fd_timer.expires = jiffies + HZ / 10;
		add_timer(&fd_timer);
	}
}

static void main_command_interrupt(void)
{
	del_timer(&fd_timer);
	cont->interrupt();
}


static int fd_wait_for_completion(unsigned long delay, timeout_fn function)
{
	if (FDCS->reset) {
		reset_fdc();	
		return 1;
	}

	if (time_before(jiffies, delay)) {
		del_timer(&fd_timer);
		fd_timer.function = function;
		fd_timer.expires = delay;
		add_timer(&fd_timer);
		return 1;
	}
	return 0;
}

static DEFINE_SPINLOCK(floppy_hlt_lock);
static int hlt_disabled;
static void floppy_disable_hlt(void)
{
	unsigned long flags;

	spin_lock_irqsave(&floppy_hlt_lock, flags);
	if (!hlt_disabled) {
		hlt_disabled = 1;
#ifdef HAVE_DISABLE_HLT
		disable_hlt();
#endif
	}
	spin_unlock_irqrestore(&floppy_hlt_lock, flags);
}

static void floppy_enable_hlt(void)
{
	unsigned long flags;

	spin_lock_irqsave(&floppy_hlt_lock, flags);
	if (hlt_disabled) {
		hlt_disabled = 0;
#ifdef HAVE_DISABLE_HLT
		enable_hlt();
#endif
	}
	spin_unlock_irqrestore(&floppy_hlt_lock, flags);
}

static void setup_DMA(void)
{
	unsigned long f;

#ifdef FLOPPY_SANITY_CHECK
	if (raw_cmd->length == 0) {
		int i;

		printk("zero dma transfer size:");
		for (i = 0; i < raw_cmd->cmd_count; i++)
			printk("%x,", raw_cmd->cmd[i]);
		printk("\n");
		cont->done(0);
		FDCS->reset = 1;
		return;
	}
	if (((unsigned long)raw_cmd->kernel_data) % 512) {
		printk("non aligned address: %p\n", raw_cmd->kernel_data);
		cont->done(0);
		FDCS->reset = 1;
		return;
	}
#endif
	f = claim_dma_lock();
	fd_disable_dma();
#ifdef fd_dma_setup
	if (fd_dma_setup(raw_cmd->kernel_data, raw_cmd->length,
			 (raw_cmd->flags & FD_RAW_READ) ?
			 DMA_MODE_READ : DMA_MODE_WRITE, FDCS->address) < 0) {
		release_dma_lock(f);
		cont->done(0);
		FDCS->reset = 1;
		return;
	}
	release_dma_lock(f);
#else
	fd_clear_dma_ff();
	fd_cacheflush(raw_cmd->kernel_data, raw_cmd->length);
	fd_set_dma_mode((raw_cmd->flags & FD_RAW_READ) ?
			DMA_MODE_READ : DMA_MODE_WRITE);
	fd_set_dma_addr(raw_cmd->kernel_data);
	fd_set_dma_count(raw_cmd->length);
	virtual_dma_port = FDCS->address;
	fd_enable_dma();
	release_dma_lock(f);
#endif
	floppy_disable_hlt();
}

static void show_floppy(void);


static int wait_til_ready(void)
{
	int status;
	int counter;

	if (FDCS->reset)
		return -1;
	for (counter = 0; counter < 10000; counter++) {
		status = fd_inb(FD_STATUS);
		if (status & STATUS_READY)
			return status;
	}
	if (!initialising) {
		DPRINT("Getstatus times out (%x) on fdc %d\n", status, fdc);
		show_floppy();
	}
	FDCS->reset = 1;
	return -1;
}


static int output_byte(char byte)
{
	int status;

	if ((status = wait_til_ready()) < 0)
		return -1;
	if ((status & (STATUS_READY | STATUS_DIR | STATUS_DMA)) == STATUS_READY) {
		fd_outb(byte, FD_DATA);
#ifdef FLOPPY_SANITY_CHECK
		output_log[output_log_pos].data = byte;
		output_log[output_log_pos].status = status;
		output_log[output_log_pos].jiffies = jiffies;
		output_log_pos = (output_log_pos + 1) % OLOGSIZE;
#endif
		return 0;
	}
	FDCS->reset = 1;
	if (!initialising) {
		DPRINT("Unable to send byte %x to FDC. Fdc=%x Status=%x\n",
		       byte, fdc, status);
		show_floppy();
	}
	return -1;
}

#define LAST_OUT(x) if (output_byte(x)<0){ reset_fdc();return;}


static int result(void)
{
	int i;
	int status = 0;

	for (i = 0; i < MAX_REPLIES; i++) {
		if ((status = wait_til_ready()) < 0)
			break;
		status &= STATUS_DIR | STATUS_READY | STATUS_BUSY | STATUS_DMA;
		if ((status & ~STATUS_BUSY) == STATUS_READY) {
#ifdef FLOPPY_SANITY_CHECK
			resultjiffies = jiffies;
			resultsize = i;
#endif
			return i;
		}
		if (status == (STATUS_DIR | STATUS_READY | STATUS_BUSY))
			reply_buffer[i] = fd_inb(FD_DATA);
		else
			break;
	}
	if (!initialising) {
		DPRINT
		    ("get result error. Fdc=%d Last status=%x Read bytes=%d\n",
		     fdc, status, i);
		show_floppy();
	}
	FDCS->reset = 1;
	return -1;
}

#define MORE_OUTPUT -2

static int need_more_output(void)
{
	int status;

	if ((status = wait_til_ready()) < 0)
		return -1;
	if ((status & (STATUS_READY | STATUS_DIR | STATUS_DMA)) == STATUS_READY)
		return MORE_OUTPUT;
	return result();
}


static inline void perpendicular_mode(void)
{
	unsigned char perp_mode;

	if (raw_cmd->rate & 0x40) {
		switch (raw_cmd->rate & 3) {
		case 0:
			perp_mode = 2;
			break;
		case 3:
			perp_mode = 3;
			break;
		default:
			DPRINT("Invalid data rate for perpendicular mode!\n");
			cont->done(0);
			FDCS->reset = 1;	
			return;
		}
	} else
		perp_mode = 0;

	if (FDCS->perp_mode == perp_mode)
		return;
	if (FDCS->version >= FDC_82077_ORIG) {
		output_byte(FD_PERPENDICULAR);
		output_byte(perp_mode);
		FDCS->perp_mode = perp_mode;
	} else if (perp_mode) {
		DPRINT("perpendicular mode not supported by this FDC.\n");
	}
}				

static int fifo_depth = 0xa;
static int no_fifo;

static int fdc_configure(void)
{
	
	output_byte(FD_CONFIGURE);
	if (need_more_output() != MORE_OUTPUT)
		return 0;
	output_byte(0);
	output_byte(0x10 | (no_fifo & 0x20) | (fifo_depth & 0xf));
	output_byte(0);		
	return 1;
}

#define NOMINAL_DTR 500


static void fdc_specify(void)
{
	unsigned char spec1;
	unsigned char spec2;
	unsigned long srt;
	unsigned long hlt;
	unsigned long hut;
	unsigned long dtr = NOMINAL_DTR;
	unsigned long scale_dtr = NOMINAL_DTR;
	int hlt_max_code = 0x7f;
	int hut_max_code = 0xf;

	if (FDCS->need_configure && FDCS->version >= FDC_82072A) {
		fdc_configure();
		FDCS->need_configure = 0;
	}

	switch (raw_cmd->rate & 0x03) {
	case 3:
		dtr = 1000;
		break;
	case 1:
		dtr = 300;
		if (FDCS->version >= FDC_82078) {
			
			output_byte(FD_DRIVESPEC);
			if (need_more_output() == MORE_OUTPUT) {
				output_byte(UNIT(current_drive));
				output_byte(0xc0);
			}
		}
		break;
	case 2:
		dtr = 250;
		break;
	}

	if (FDCS->version >= FDC_82072) {
		scale_dtr = dtr;
		hlt_max_code = 0x00;	
		hut_max_code = 0x0;	
	}

	
	srt = 16 - DIV_ROUND_UP(DP->srt * scale_dtr / 1000, NOMINAL_DTR);
	if (slow_floppy) {
		srt = srt / 4;
	}
	SUPBOUND(srt, 0xf);
	INFBOUND(srt, 0);

	hlt = DIV_ROUND_UP(DP->hlt * scale_dtr / 2, NOMINAL_DTR);
	if (hlt < 0x01)
		hlt = 0x01;
	else if (hlt > 0x7f)
		hlt = hlt_max_code;

	hut = DIV_ROUND_UP(DP->hut * scale_dtr / 16, NOMINAL_DTR);
	if (hut < 0x1)
		hut = 0x1;
	else if (hut > 0xf)
		hut = hut_max_code;

	spec1 = (srt << 4) | hut;
	spec2 = (hlt << 1) | (use_virtual_dma & 1);

	
	if (FDCS->spec1 != spec1 || FDCS->spec2 != spec2) {
		
		output_byte(FD_SPECIFY);
		output_byte(FDCS->spec1 = spec1);
		output_byte(FDCS->spec2 = spec2);
	}
}				


static int fdc_dtr(void)
{
	
	if ((raw_cmd->rate & 3) == FDCS->dtr)
		return 0;

	
	fd_outb(raw_cmd->rate & 3, FD_DCR);

	
	FDCS->dtr = raw_cmd->rate & 3;
	return (fd_wait_for_completion(jiffies + 2UL * HZ / 100,
				       (timeout_fn) floppy_ready));
}				

static void tell_sector(void)
{
	printk(": track %d, head %d, sector %d, size %d",
	       R_TRACK, R_HEAD, R_SECTOR, R_SIZECODE);
}				


static int interpret_errors(void)
{
	char bad;

	if (inr != 7) {
		DPRINT("-- FDC reply error");
		FDCS->reset = 1;
		return 1;
	}

	
	switch (ST0 & ST0_INTR) {
	case 0x40:		
		if (ST1 & ST1_EOC)
			return 0;	
		bad = 1;
		if (ST1 & ST1_WP) {
			DPRINT("Drive is write protected\n");
			CLEARF(FD_DISK_WRITABLE);
			cont->done(0);
			bad = 2;
		} else if (ST1 & ST1_ND) {
			SETF(FD_NEED_TWADDLE);
		} else if (ST1 & ST1_OR) {
			if (DP->flags & FTD_MSG)
				DPRINT("Over/Underrun - retrying\n");
			bad = 0;
		} else if (*errors >= DP->max_errors.reporting) {
			DPRINT("");
			if (ST0 & ST0_ECE) {
				printk("Recalibrate failed!");
			} else if (ST2 & ST2_CRC) {
				printk("data CRC error");
				tell_sector();
			} else if (ST1 & ST1_CRC) {
				printk("CRC error");
				tell_sector();
			} else if ((ST1 & (ST1_MAM | ST1_ND))
				   || (ST2 & ST2_MAM)) {
				if (!probing) {
					printk("sector not found");
					tell_sector();
				} else
					printk("probe failed...");
			} else if (ST2 & ST2_WC) {	
				printk("wrong cylinder");
			} else if (ST2 & ST2_BC) {	
				printk("bad cylinder");
			} else {
				printk
				    ("unknown error. ST[0..2] are: 0x%x 0x%x 0x%x",
				     ST0, ST1, ST2);
				tell_sector();
			}
			printk("\n");
		}
		if (ST2 & ST2_WC || ST2 & ST2_BC)
			
			DRS->track = NEED_2_RECAL;
		return bad;
	case 0x80:		
		DPRINT("Invalid FDC command given!\n");
		cont->done(0);
		return 2;
	case 0xc0:
		DPRINT("Abnormal termination caused by polling\n");
		cont->error();
		return 2;
	default:		
		return 0;
	}
}


static void setup_rw_floppy(void)
{
	int i;
	int r;
	int flags;
	int dflags;
	unsigned long ready_date;
	timeout_fn function;

	flags = raw_cmd->flags;
	if (flags & (FD_RAW_READ | FD_RAW_WRITE))
		flags |= FD_RAW_INTR;

	if ((flags & FD_RAW_SPIN) && !(flags & FD_RAW_NO_MOTOR)) {
		ready_date = DRS->spinup_date + DP->spinup;
		
		if (time_after(ready_date, jiffies + DP->select_delay)) {
			ready_date -= DP->select_delay;
			function = (timeout_fn) floppy_start;
		} else
			function = (timeout_fn) setup_rw_floppy;

		
		if (fd_wait_for_completion(ready_date, function))
			return;
	}
	dflags = DRS->flags;

	if ((flags & FD_RAW_READ) || (flags & FD_RAW_WRITE))
		setup_DMA();

	if (flags & FD_RAW_INTR)
		do_floppy = main_command_interrupt;

	r = 0;
	for (i = 0; i < raw_cmd->cmd_count; i++)
		r |= output_byte(raw_cmd->cmd[i]);

	debugt("rw_command: ");

	if (r) {
		cont->error();
		reset_fdc();
		return;
	}

	if (!(flags & FD_RAW_INTR)) {
		inr = result();
		cont->interrupt();
	} else if (flags & FD_RAW_NEED_DISK)
		fd_watchdog();
}

static int blind_seek;


static void seek_interrupt(void)
{
	debugt("seek interrupt:");
	if (inr != 2 || (ST0 & 0xF8) != 0x20) {
		DPRINT("seek failed\n");
		DRS->track = NEED_2_RECAL;
		cont->error();
		cont->redo();
		return;
	}
	if (DRS->track >= 0 && DRS->track != ST1 && !blind_seek) {
#ifdef DCL_DEBUG
		if (DP->flags & FD_DEBUG) {
			DPRINT
			    ("clearing NEWCHANGE flag because of effective seek\n");
			DPRINT("jiffies=%lu\n", jiffies);
		}
#endif
		CLEARF(FD_DISK_NEWCHANGE);	
		DRS->select_date = jiffies;
	}
	DRS->track = ST1;
	floppy_ready();
}

static void check_wp(void)
{
	if (TESTF(FD_VERIFY)) {
		
		output_byte(FD_GETSTATUS);
		output_byte(UNIT(current_drive));
		if (result() != 1) {
			FDCS->reset = 1;
			return;
		}
		CLEARF(FD_VERIFY);
		CLEARF(FD_NEED_TWADDLE);
#ifdef DCL_DEBUG
		if (DP->flags & FD_DEBUG) {
			DPRINT("checking whether disk is write protected\n");
			DPRINT("wp=%x\n", ST3 & 0x40);
		}
#endif
		if (!(ST3 & 0x40))
			SETF(FD_DISK_WRITABLE);
		else
			CLEARF(FD_DISK_WRITABLE);
	}
}

static void seek_floppy(void)
{
	int track;

	blind_seek = 0;

#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("calling disk change from seek\n");
	}
#endif

	if (!TESTF(FD_DISK_NEWCHANGE) &&
	    disk_change(current_drive) && (raw_cmd->flags & FD_RAW_NEED_DISK)) {
		
		SETF(FD_DISK_CHANGED);
		cont->done(0);
		cont->redo();
		return;
	}
	if (DRS->track <= NEED_1_RECAL) {
		recalibrate_floppy();
		return;
	} else if (TESTF(FD_DISK_NEWCHANGE) &&
		   (raw_cmd->flags & FD_RAW_NEED_DISK) &&
		   (DRS->track <= NO_TRACK || DRS->track == raw_cmd->track)) {
		
		if (raw_cmd->track)
			track = raw_cmd->track - 1;
		else {
			if (DP->flags & FD_SILENT_DCL_CLEAR) {
				set_dor(fdc, ~(0x10 << UNIT(current_drive)), 0);
				blind_seek = 1;
				raw_cmd->flags |= FD_RAW_NEED_SEEK;
			}
			track = 1;
		}
	} else {
		check_wp();
		if (raw_cmd->track != DRS->track &&
		    (raw_cmd->flags & FD_RAW_NEED_SEEK))
			track = raw_cmd->track;
		else {
			setup_rw_floppy();
			return;
		}
	}

	do_floppy = seek_interrupt;
	output_byte(FD_SEEK);
	output_byte(UNIT(current_drive));
	LAST_OUT(track);
	debugt("seek command:");
}

static void recal_interrupt(void)
{
	debugt("recal interrupt:");
	if (inr != 2)
		FDCS->reset = 1;
	else if (ST0 & ST0_ECE) {
		switch (DRS->track) {
		case NEED_1_RECAL:
			debugt("recal interrupt need 1 recal:");
			
			cont->error();
			cont->redo();
			return;
		case NEED_2_RECAL:
			debugt("recal interrupt need 2 recal:");
			
#ifdef DCL_DEBUG
			if (DP->flags & FD_DEBUG) {
				DPRINT
				    ("clearing NEWCHANGE flag because of second recalibrate\n");
			}
#endif

			CLEARF(FD_DISK_NEWCHANGE);
			DRS->select_date = jiffies;
			
		default:
			debugt("recal interrupt default:");
			
			DRS->track = NEED_1_RECAL;
			break;
		}
	} else
		DRS->track = ST1;
	floppy_ready();
}

static void print_result(char *message, int inr)
{
	int i;

	DPRINT("%s ", message);
	if (inr >= 0)
		for (i = 0; i < inr; i++)
			printk("repl[%d]=%x ", i, reply_buffer[i]);
	printk("\n");
}


irqreturn_t floppy_interrupt(int irq, void *dev_id)
{
	int do_print;
	unsigned long f;
	void (*handler)(void) = do_floppy;

	lasthandler = handler;
	interruptjiffies = jiffies;

	f = claim_dma_lock();
	fd_disable_dma();
	release_dma_lock(f);

	floppy_enable_hlt();
	do_floppy = NULL;
	if (fdc >= N_FDC || FDCS->address == -1) {
		
		printk("DOR0=%x\n", fdc_state[0].dor);
		printk("floppy interrupt on bizarre fdc %d\n", fdc);
		printk("handler=%p\n", handler);
		is_alive("bizarre fdc");
		return IRQ_NONE;
	}

	FDCS->reset = 0;
	

	do_print = !handler && print_unex && !initialising;

	inr = result();
	if (do_print)
		print_result("unexpected interrupt", inr);
	if (inr == 0) {
		int max_sensei = 4;
		do {
			output_byte(FD_SENSEI);
			inr = result();
			if (do_print)
				print_result("sensei", inr);
			max_sensei--;
		} while ((ST0 & 0x83) != UNIT(current_drive) && inr == 2
			 && max_sensei);
	}
	if (!handler) {
		FDCS->reset = 1;
		return IRQ_NONE;
	}
	schedule_bh(handler);
	is_alive("normal interrupt end");

	
	return IRQ_HANDLED;
}

static void recalibrate_floppy(void)
{
	debugt("recalibrate floppy:");
	do_floppy = recal_interrupt;
	output_byte(FD_RECALIBRATE);
	LAST_OUT(UNIT(current_drive));
}


static void reset_interrupt(void)
{
	debugt("reset interrupt:");
	result();		
	if (FDCS->reset) {
		printk("reset set in interrupt, calling %p\n", cont->error);
		cont->error();	
	}
	cont->redo();
}


static void reset_fdc(void)
{
	unsigned long flags;

	do_floppy = reset_interrupt;
	FDCS->reset = 0;
	reset_fdc_info(0);

	
	

	flags = claim_dma_lock();
	fd_disable_dma();
	release_dma_lock(flags);

	if (FDCS->version >= FDC_82072A)
		fd_outb(0x80 | (FDCS->dtr & 3), FD_STATUS);
	else {
		fd_outb(FDCS->dor & ~0x04, FD_DOR);
		udelay(FD_RESET_DELAY);
		fd_outb(FDCS->dor, FD_DOR);
	}
}

static void show_floppy(void)
{
	int i;

	printk("\n");
	printk("floppy driver state\n");
	printk("-------------------\n");
	printk("now=%lu last interrupt=%lu diff=%lu last called handler=%p\n",
	       jiffies, interruptjiffies, jiffies - interruptjiffies,
	       lasthandler);

#ifdef FLOPPY_SANITY_CHECK
	printk("timeout_message=%s\n", timeout_message);
	printk("last output bytes:\n");
	for (i = 0; i < OLOGSIZE; i++)
		printk("%2x %2x %lu\n",
		       output_log[(i + output_log_pos) % OLOGSIZE].data,
		       output_log[(i + output_log_pos) % OLOGSIZE].status,
		       output_log[(i + output_log_pos) % OLOGSIZE].jiffies);
	printk("last result at %lu\n", resultjiffies);
	printk("last redo_fd_request at %lu\n", lastredo);
	for (i = 0; i < resultsize; i++) {
		printk("%2x ", reply_buffer[i]);
	}
	printk("\n");
#endif

	printk("status=%x\n", fd_inb(FD_STATUS));
	printk("fdc_busy=%lu\n", fdc_busy);
	if (do_floppy)
		printk("do_floppy=%p\n", do_floppy);
	if (work_pending(&floppy_work))
		printk("floppy_work.func=%p\n", floppy_work.func);
	if (timer_pending(&fd_timer))
		printk("fd_timer.function=%p\n", fd_timer.function);
	if (timer_pending(&fd_timeout)) {
		printk("timer_function=%p\n", fd_timeout.function);
		printk("expires=%lu\n", fd_timeout.expires - jiffies);
		printk("now=%lu\n", jiffies);
	}
	printk("cont=%p\n", cont);
	printk("current_req=%p\n", current_req);
	printk("command_status=%d\n", command_status);
	printk("\n");
}

static void floppy_shutdown(unsigned long data)
{
	unsigned long flags;

	if (!initialising)
		show_floppy();
	cancel_activity();

	floppy_enable_hlt();

	flags = claim_dma_lock();
	fd_disable_dma();
	release_dma_lock(flags);

	

	if (!initialising)
		DPRINT("floppy timeout called\n");
	FDCS->reset = 1;
	if (cont) {
		cont->done(0);
		cont->redo();	
	} else {
		printk("no cont in shutdown!\n");
		process_fd_request();
	}
	is_alive("floppy shutdown");
}


static int start_motor(void (*function)(void))
{
	int mask;
	int data;

	mask = 0xfc;
	data = UNIT(current_drive);
	if (!(raw_cmd->flags & FD_RAW_NO_MOTOR)) {
		if (!(FDCS->dor & (0x10 << UNIT(current_drive)))) {
			set_debugt();
			
			DRS->first_read_date = 0;
			
			DRS->spinup_date = jiffies;
			data |= (0x10 << UNIT(current_drive));
		}
	} else if (FDCS->dor & (0x10 << UNIT(current_drive)))
		mask &= ~(0x10 << UNIT(current_drive));

	
	del_timer(motor_off_timer + current_drive);
	set_dor(fdc, mask, data);

	
	return (fd_wait_for_completion(DRS->select_date + DP->select_delay,
				       (timeout_fn) function));
}

static void floppy_ready(void)
{
	CHECK_RESET;
	if (start_motor(floppy_ready))
		return;
	if (fdc_dtr())
		return;

#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("calling disk change from floppy_ready\n");
	}
#endif
	if (!(raw_cmd->flags & FD_RAW_NO_MOTOR) &&
	    disk_change(current_drive) && !DP->select_delay)
		twaddle();	

#ifdef fd_chose_dma_mode
	if ((raw_cmd->flags & FD_RAW_READ) || (raw_cmd->flags & FD_RAW_WRITE)) {
		unsigned long flags = claim_dma_lock();
		fd_chose_dma_mode(raw_cmd->kernel_data, raw_cmd->length);
		release_dma_lock(flags);
	}
#endif

	if (raw_cmd->flags & (FD_RAW_NEED_SEEK | FD_RAW_NEED_DISK)) {
		perpendicular_mode();
		fdc_specify();	
		seek_floppy();
	} else {
		if ((raw_cmd->flags & FD_RAW_READ) ||
		    (raw_cmd->flags & FD_RAW_WRITE))
			fdc_specify();
		setup_rw_floppy();
	}
}

static void floppy_start(void)
{
	reschedule_timeout(current_reqD, "floppy start", 0);

	scandrives();
#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("setting NEWCHANGE in floppy_start\n");
	}
#endif
	SETF(FD_DISK_NEWCHANGE);
	floppy_ready();
}




static void do_wakeup(void)
{
	reschedule_timeout(MAXTIMEOUT, "do wakeup", 0);
	cont = NULL;
	command_status += 2;
	wake_up(&command_done);
}

static struct cont_t wakeup_cont = {
	.interrupt	= empty,
	.redo		= do_wakeup,
	.error		= empty,
	.done		= (done_f)empty
};

static struct cont_t intr_cont = {
	.interrupt	= empty,
	.redo		= process_fd_request,
	.error		= empty,
	.done		= (done_f)empty
};

static int wait_til_done(void (*handler)(void), int interruptible)
{
	int ret;

	schedule_bh(handler);

	if (command_status < 2 && NO_SIGNAL) {
		DECLARE_WAITQUEUE(wait, current);

		add_wait_queue(&command_done, &wait);
		for (;;) {
			set_current_state(interruptible ?
					  TASK_INTERRUPTIBLE :
					  TASK_UNINTERRUPTIBLE);

			if (command_status >= 2 || !NO_SIGNAL)
				break;

			is_alive("wait_til_done");
			schedule();
		}

		set_current_state(TASK_RUNNING);
		remove_wait_queue(&command_done, &wait);
	}

	if (command_status < 2) {
		cancel_activity();
		cont = &intr_cont;
		reset_fdc();
		return -EINTR;
	}

	if (FDCS->reset)
		command_status = FD_COMMAND_ERROR;
	if (command_status == FD_COMMAND_OKAY)
		ret = 0;
	else
		ret = -EIO;
	command_status = FD_COMMAND_NONE;
	return ret;
}

static void generic_done(int result)
{
	command_status = result;
	cont = &wakeup_cont;
}

static void generic_success(void)
{
	cont->done(1);
}

static void generic_failure(void)
{
	cont->done(0);
}

static void success_and_wakeup(void)
{
	generic_success();
	cont->redo();
}



static int next_valid_format(void)
{
	int probed_format;

	probed_format = DRS->probed_format;
	while (1) {
		if (probed_format >= 8 || !DP->autodetect[probed_format]) {
			DRS->probed_format = 0;
			return 1;
		}
		if (floppy_type[DP->autodetect[probed_format]].sect) {
			DRS->probed_format = probed_format;
			return 0;
		}
		probed_format++;
	}
}

static void bad_flp_intr(void)
{
	int err_count;

	if (probing) {
		DRS->probed_format++;
		if (!next_valid_format())
			return;
	}
	err_count = ++(*errors);
	INFBOUND(DRWE->badness, err_count);
	if (err_count > DP->max_errors.abort)
		cont->done(0);
	if (err_count > DP->max_errors.reset)
		FDCS->reset = 1;
	else if (err_count > DP->max_errors.recal)
		DRS->track = NEED_2_RECAL;
}

static void set_floppy(int drive)
{
	int type = ITYPE(UDRS->fd_device);

	if (type)
		_floppy = floppy_type + type;
	else
		_floppy = current_type[drive];
}


static void format_interrupt(void)
{
	switch (interpret_errors()) {
	case 1:
		cont->error();
	case 2:
		break;
	case 0:
		cont->done(1);
	}
	cont->redo();
}

#define CODE2SIZE (ssize = ((1 << SIZECODE) + 3) >> 2)
#define FM_MODE(x,y) ((y) & ~(((x)->rate & 0x80) >>1))
#define CT(x) ((x) | 0xc0)
static void setup_format_params(int track)
{
	int n;
	int il;
	int count;
	int head_shift;
	int track_shift;
	struct fparm {
		unsigned char track, head, sect, size;
	} *here = (struct fparm *)floppy_track_buffer;

	raw_cmd = &default_raw_cmd;
	raw_cmd->track = track;

	raw_cmd->flags = FD_RAW_WRITE | FD_RAW_INTR | FD_RAW_SPIN |
	    FD_RAW_NEED_DISK | FD_RAW_NEED_SEEK;
	raw_cmd->rate = _floppy->rate & 0x43;
	raw_cmd->cmd_count = NR_F;
	COMMAND = FM_MODE(_floppy, FD_FORMAT);
	DR_SELECT = UNIT(current_drive) + PH_HEAD(_floppy, format_req.head);
	F_SIZECODE = FD_SIZECODE(_floppy);
	F_SECT_PER_TRACK = _floppy->sect << 2 >> F_SIZECODE;
	F_GAP = _floppy->fmt_gap;
	F_FILL = FD_FILL_BYTE;

	raw_cmd->kernel_data = floppy_track_buffer;
	raw_cmd->length = 4 * F_SECT_PER_TRACK;

	
	head_shift = (F_SECT_PER_TRACK + 5) / 6;

	
	track_shift = 2 * head_shift + 3;

	
	n = (track_shift * format_req.track + head_shift * format_req.head)
	    % F_SECT_PER_TRACK;

	
	il = 1;
	if (_floppy->fmt_gap < 0x22)
		il++;

	
	for (count = 0; count < F_SECT_PER_TRACK; ++count) {
		here[count].track = format_req.track;
		here[count].head = format_req.head;
		here[count].sect = 0;
		here[count].size = F_SIZECODE;
	}
	
	for (count = 1; count <= F_SECT_PER_TRACK; ++count) {
		here[n].sect = count;
		n = (n + il) % F_SECT_PER_TRACK;
		if (here[n].sect) {	
			++n;
			if (n >= F_SECT_PER_TRACK) {
				n -= F_SECT_PER_TRACK;
				while (here[n].sect)
					++n;
			}
		}
	}
	if (_floppy->stretch & FD_SECTBASEMASK) {
		for (count = 0; count < F_SECT_PER_TRACK; count++)
			here[count].sect += FD_SECTBASE(_floppy) - 1;
	}
}

static void redo_format(void)
{
	buffer_track = -1;
	setup_format_params(format_req.track << STRETCH(_floppy));
	floppy_start();
	debugt("queue format request");
}

static struct cont_t format_cont = {
	.interrupt	= format_interrupt,
	.redo		= redo_format,
	.error		= bad_flp_intr,
	.done		= generic_done
};

static int do_format(int drive, struct format_descr *tmp_format_req)
{
	int ret;

	LOCK_FDC(drive, 1);
	set_floppy(drive);
	if (!_floppy ||
	    _floppy->track > DP->tracks ||
	    tmp_format_req->track >= _floppy->track ||
	    tmp_format_req->head >= _floppy->head ||
	    (_floppy->sect << 2) % (1 << FD_SIZECODE(_floppy)) ||
	    !_floppy->fmt_gap) {
		process_fd_request();
		return -EINVAL;
	}
	format_req = *tmp_format_req;
	format_errors = 0;
	cont = &format_cont;
	errors = &format_errors;
	IWAIT(redo_format);
	process_fd_request();
	return ret;
}



static void floppy_end_request(struct request *req, int error)
{
	unsigned int nr_sectors = current_count_sectors;
	unsigned int drive = (unsigned long)req->rq_disk->private_data;

	
	if (error)
		nr_sectors = blk_rq_cur_sectors(req);
	if (__blk_end_request(req, error, nr_sectors << 9))
		return;

	
	floppy_off(drive);
	current_req = NULL;
}


static void request_done(int uptodate)
{
	struct request_queue *q = floppy_queue;
	struct request *req = current_req;
	unsigned long flags;
	int block;

	probing = 0;
	reschedule_timeout(MAXTIMEOUT, "request done %d", uptodate);

	if (!req) {
		printk("floppy.c: no request in request_done\n");
		return;
	}

	if (uptodate) {
		
		block = current_count_sectors + blk_rq_pos(req);
		INFBOUND(DRS->maxblock, block);
		if (block > _floppy->sect)
			DRS->maxtrack = 1;

		
		spin_lock_irqsave(q->queue_lock, flags);
		floppy_end_request(req, 0);
		spin_unlock_irqrestore(q->queue_lock, flags);
	} else {
		if (rq_data_dir(req) == WRITE) {
			
			DRWE->write_errors++;
			if (DRWE->write_errors == 1) {
				DRWE->first_error_sector = blk_rq_pos(req);
				DRWE->first_error_generation = DRS->generation;
			}
			DRWE->last_error_sector = blk_rq_pos(req);
			DRWE->last_error_generation = DRS->generation;
		}
		spin_lock_irqsave(q->queue_lock, flags);
		floppy_end_request(req, -EIO);
		spin_unlock_irqrestore(q->queue_lock, flags);
	}
}


static void rw_interrupt(void)
{
	int eoc;
	int ssize;
	int heads;
	int nr_sectors;

	if (R_HEAD >= 2) {
		
		return;
	}

	if (!DRS->first_read_date)
		DRS->first_read_date = jiffies;

	nr_sectors = 0;
	CODE2SIZE;

	if (ST1 & ST1_EOC)
		eoc = 1;
	else
		eoc = 0;

	if (COMMAND & 0x80)
		heads = 2;
	else
		heads = 1;

	nr_sectors = (((R_TRACK - TRACK) * heads +
		       R_HEAD - HEAD) * SECT_PER_TRACK +
		      R_SECTOR - SECTOR + eoc) << SIZECODE >> 2;

#ifdef FLOPPY_SANITY_CHECK
	if (nr_sectors / ssize >
	    DIV_ROUND_UP(in_sector_offset + current_count_sectors, ssize)) {
		DPRINT("long rw: %x instead of %lx\n",
		       nr_sectors, current_count_sectors);
		printk("rs=%d s=%d\n", R_SECTOR, SECTOR);
		printk("rh=%d h=%d\n", R_HEAD, HEAD);
		printk("rt=%d t=%d\n", R_TRACK, TRACK);
		printk("heads=%d eoc=%d\n", heads, eoc);
		printk("spt=%d st=%d ss=%d\n", SECT_PER_TRACK,
		       fsector_t, ssize);
		printk("in_sector_offset=%d\n", in_sector_offset);
	}
#endif

	nr_sectors -= in_sector_offset;
	INFBOUND(nr_sectors, 0);
	SUPBOUND(current_count_sectors, nr_sectors);

	switch (interpret_errors()) {
	case 2:
		cont->redo();
		return;
	case 1:
		if (!current_count_sectors) {
			cont->error();
			cont->redo();
			return;
		}
		break;
	case 0:
		if (!current_count_sectors) {
			cont->redo();
			return;
		}
		current_type[current_drive] = _floppy;
		floppy_sizes[TOMINOR(current_drive)] = _floppy->size;
		break;
	}

	if (probing) {
		if (DP->flags & FTD_MSG)
			DPRINT("Auto-detected floppy type %s in fd%d\n",
			       _floppy->name, current_drive);
		current_type[current_drive] = _floppy;
		floppy_sizes[TOMINOR(current_drive)] = _floppy->size;
		probing = 0;
	}

	if (CT(COMMAND) != FD_READ ||
	    raw_cmd->kernel_data == current_req->buffer) {
		
		cont->done(1);
	} else if (CT(COMMAND) == FD_READ) {
		buffer_track = raw_cmd->track;
		buffer_drive = current_drive;
		INFBOUND(buffer_max, nr_sectors + fsector_t);
	}
	cont->redo();
}


static int buffer_chain_size(void)
{
	struct bio_vec *bv;
	int size;
	struct req_iterator iter;
	char *base;

	base = bio_data(current_req->bio);
	size = 0;

	rq_for_each_segment(bv, current_req, iter) {
		if (page_address(bv->bv_page) + bv->bv_offset != base + size)
			break;

		size += bv->bv_len;
	}

	return size >> 9;
}


static int transfer_size(int ssize, int max_sector, int max_size)
{
	SUPBOUND(max_sector, fsector_t + max_size);

	
	max_sector -= (max_sector % _floppy->sect) % ssize;

	
	current_count_sectors = max_sector - fsector_t;

	return max_sector;
}


static void copy_buffer(int ssize, int max_sector, int max_sector_2)
{
	int remaining;		
	struct bio_vec *bv;
	char *buffer;
	char *dma_buffer;
	int size;
	struct req_iterator iter;

	max_sector = transfer_size(ssize,
				   min(max_sector, max_sector_2),
				   blk_rq_sectors(current_req));

	if (current_count_sectors <= 0 && CT(COMMAND) == FD_WRITE &&
	    buffer_max > fsector_t + blk_rq_sectors(current_req))
		current_count_sectors = min_t(int, buffer_max - fsector_t,
					      blk_rq_sectors(current_req));

	remaining = current_count_sectors << 9;
#ifdef FLOPPY_SANITY_CHECK
	if (remaining > blk_rq_bytes(current_req) && CT(COMMAND) == FD_WRITE) {
		DPRINT("in copy buffer\n");
		printk("current_count_sectors=%ld\n", current_count_sectors);
		printk("remaining=%d\n", remaining >> 9);
		printk("current_req->nr_sectors=%u\n",
		       blk_rq_sectors(current_req));
		printk("current_req->current_nr_sectors=%u\n",
		       blk_rq_cur_sectors(current_req));
		printk("max_sector=%d\n", max_sector);
		printk("ssize=%d\n", ssize);
	}
#endif

	buffer_max = max(max_sector, buffer_max);

	dma_buffer = floppy_track_buffer + ((fsector_t - buffer_min) << 9);

	size = blk_rq_cur_bytes(current_req);

	rq_for_each_segment(bv, current_req, iter) {
		if (!remaining)
			break;

		size = bv->bv_len;
		SUPBOUND(size, remaining);

		buffer = page_address(bv->bv_page) + bv->bv_offset;
#ifdef FLOPPY_SANITY_CHECK
		if (dma_buffer + size >
		    floppy_track_buffer + (max_buffer_sectors << 10) ||
		    dma_buffer < floppy_track_buffer) {
			DPRINT("buffer overrun in copy buffer %d\n",
			       (int)((floppy_track_buffer -
				      dma_buffer) >> 9));
			printk("fsector_t=%d buffer_min=%d\n",
			       fsector_t, buffer_min);
			printk("current_count_sectors=%ld\n",
			       current_count_sectors);
			if (CT(COMMAND) == FD_READ)
				printk("read\n");
			if (CT(COMMAND) == FD_WRITE)
				printk("write\n");
			break;
		}
		if (((unsigned long)buffer) % 512)
			DPRINT("%p buffer not aligned\n", buffer);
#endif
		if (CT(COMMAND) == FD_READ)
			memcpy(buffer, dma_buffer, size);
		else
			memcpy(dma_buffer, buffer, size);

		remaining -= size;
		dma_buffer += size;
	}
#ifdef FLOPPY_SANITY_CHECK
	if (remaining) {
		if (remaining > 0)
			max_sector -= remaining >> 9;
		DPRINT("weirdness: remaining %d\n", remaining >> 9);
	}
#endif
}


static void virtualdmabug_workaround(void)
{
	int hard_sectors;
	int end_sector;

	if (CT(COMMAND) == FD_WRITE) {
		COMMAND &= ~0x80;	

		hard_sectors = raw_cmd->length >> (7 + SIZECODE);
		end_sector = SECTOR + hard_sectors - 1;
#ifdef FLOPPY_SANITY_CHECK
		if (end_sector > SECT_PER_TRACK) {
			printk("too many sectors %d > %d\n",
			       end_sector, SECT_PER_TRACK);
			return;
		}
#endif
		SECT_PER_TRACK = end_sector;	
	}
}



static int make_raw_rw_request(void)
{
	int aligned_sector_t;
	int max_sector;
	int max_size;
	int tracksize;
	int ssize;

	if (max_buffer_sectors == 0) {
		printk("VFS: Block I/O scheduled on unopened device\n");
		return 0;
	}

	set_fdc((long)current_req->rq_disk->private_data);

	raw_cmd = &default_raw_cmd;
	raw_cmd->flags = FD_RAW_SPIN | FD_RAW_NEED_DISK | FD_RAW_NEED_DISK |
	    FD_RAW_NEED_SEEK;
	raw_cmd->cmd_count = NR_RW;
	if (rq_data_dir(current_req) == READ) {
		raw_cmd->flags |= FD_RAW_READ;
		COMMAND = FM_MODE(_floppy, FD_READ);
	} else if (rq_data_dir(current_req) == WRITE) {
		raw_cmd->flags |= FD_RAW_WRITE;
		COMMAND = FM_MODE(_floppy, FD_WRITE);
	} else {
		DPRINT("make_raw_rw_request: unknown command\n");
		return 0;
	}

	max_sector = _floppy->sect * _floppy->head;

	TRACK = (int)blk_rq_pos(current_req) / max_sector;
	fsector_t = (int)blk_rq_pos(current_req) % max_sector;
	if (_floppy->track && TRACK >= _floppy->track) {
		if (blk_rq_cur_sectors(current_req) & 1) {
			current_count_sectors = 1;
			return 1;
		} else
			return 0;
	}
	HEAD = fsector_t / _floppy->sect;

	if (((_floppy->stretch & (FD_SWAPSIDES | FD_SECTBASEMASK)) ||
	     TESTF(FD_NEED_TWADDLE)) && fsector_t < _floppy->sect)
		max_sector = _floppy->sect;

	
	if ((_floppy->rate & FD_2M) && (!TRACK) && (!HEAD)) {
		max_sector = 2 * _floppy->sect / 3;
		if (fsector_t >= max_sector) {
			current_count_sectors =
			    min_t(int, _floppy->sect - fsector_t,
				  blk_rq_sectors(current_req));
			return 1;
		}
		SIZECODE = 2;
	} else
		SIZECODE = FD_SIZECODE(_floppy);
	raw_cmd->rate = _floppy->rate & 0x43;
	if ((_floppy->rate & FD_2M) && (TRACK || HEAD) && raw_cmd->rate == 2)
		raw_cmd->rate = 1;

	if (SIZECODE)
		SIZECODE2 = 0xff;
	else
		SIZECODE2 = 0x80;
	raw_cmd->track = TRACK << STRETCH(_floppy);
	DR_SELECT = UNIT(current_drive) + PH_HEAD(_floppy, HEAD);
	GAP = _floppy->gap;
	CODE2SIZE;
	SECT_PER_TRACK = _floppy->sect << 2 >> SIZECODE;
	SECTOR = ((fsector_t % _floppy->sect) << 2 >> SIZECODE) +
	    FD_SECTBASE(_floppy);

	
	tracksize = _floppy->sect - _floppy->sect % ssize;
	if (tracksize < _floppy->sect) {
		SECT_PER_TRACK++;
		if (tracksize <= fsector_t % _floppy->sect)
			SECTOR--;

		
		while (tracksize <= fsector_t % _floppy->sect) {
			while (tracksize + ssize > _floppy->sect) {
				SIZECODE--;
				ssize >>= 1;
			}
			SECTOR++;
			SECT_PER_TRACK++;
			tracksize += ssize;
		}
		max_sector = HEAD * _floppy->sect + tracksize;
	} else if (!TRACK && !HEAD && !(_floppy->rate & FD_2M) && probing) {
		max_sector = _floppy->sect;
	} else if (!HEAD && CT(COMMAND) == FD_WRITE) {
		
		max_sector = _floppy->sect;
	}

	in_sector_offset = (fsector_t % _floppy->sect) % ssize;
	aligned_sector_t = fsector_t - in_sector_offset;
	max_size = blk_rq_sectors(current_req);
	if ((raw_cmd->track == buffer_track) &&
	    (current_drive == buffer_drive) &&
	    (fsector_t >= buffer_min) && (fsector_t < buffer_max)) {
		
		if (CT(COMMAND) == FD_READ) {
			copy_buffer(1, max_sector, buffer_max);
			return 1;
		}
	} else if (in_sector_offset || blk_rq_sectors(current_req) < ssize) {
		if (CT(COMMAND) == FD_WRITE) {
			if (fsector_t + blk_rq_sectors(current_req) > ssize &&
			    fsector_t + blk_rq_sectors(current_req) < ssize + ssize)
				max_size = ssize + ssize;
			else
				max_size = ssize;
		}
		raw_cmd->flags &= ~FD_RAW_WRITE;
		raw_cmd->flags |= FD_RAW_READ;
		COMMAND = FM_MODE(_floppy, FD_READ);
	} else if ((unsigned long)current_req->buffer < MAX_DMA_ADDRESS) {
		unsigned long dma_limit;
		int direct, indirect;

		indirect =
		    transfer_size(ssize, max_sector,
				  max_buffer_sectors * 2) - fsector_t;

		
		max_size = buffer_chain_size();
		dma_limit =
		    (MAX_DMA_ADDRESS -
		     ((unsigned long)current_req->buffer)) >> 9;
		if ((unsigned long)max_size > dma_limit) {
			max_size = dma_limit;
		}
		
		if (CROSS_64KB(current_req->buffer, max_size << 9))
			max_size = (K_64 -
				    ((unsigned long)current_req->buffer) %
				    K_64) >> 9;
		direct = transfer_size(ssize, max_sector, max_size) - fsector_t;
		
		if (!direct ||
		    (indirect * 2 > direct * 3 &&
		     *errors < DP->max_errors.read_track && ((!probing
		       || (DP->read_track & (1 << DRS->probed_format)))))) {
			max_size = blk_rq_sectors(current_req);
		} else {
			raw_cmd->kernel_data = current_req->buffer;
			raw_cmd->length = current_count_sectors << 9;
			if (raw_cmd->length == 0) {
				DPRINT
				    ("zero dma transfer attempted from make_raw_request\n");
				DPRINT("indirect=%d direct=%d fsector_t=%d",
				       indirect, direct, fsector_t);
				return 0;
			}
			virtualdmabug_workaround();
			return 2;
		}
	}

	if (CT(COMMAND) == FD_READ)
		max_size = max_sector;	

	
	if (buffer_track != raw_cmd->track ||	
	    buffer_drive != current_drive ||	
	    fsector_t > buffer_max ||
	    fsector_t < buffer_min ||
	    ((CT(COMMAND) == FD_READ ||
	      (!in_sector_offset && blk_rq_sectors(current_req) >= ssize)) &&
	     max_sector > 2 * max_buffer_sectors + buffer_min &&
	     max_size + fsector_t > 2 * max_buffer_sectors + buffer_min)
	    
	    ) {
		buffer_track = -1;
		buffer_drive = current_drive;
		buffer_max = buffer_min = aligned_sector_t;
	}
	raw_cmd->kernel_data = floppy_track_buffer +
	    ((aligned_sector_t - buffer_min) << 9);

	if (CT(COMMAND) == FD_WRITE) {
		
#ifdef FLOPPY_SANITY_CHECK
		if (in_sector_offset && buffer_track == -1)
			DPRINT("internal error offset !=0 on write\n");
#endif
		buffer_track = raw_cmd->track;
		buffer_drive = current_drive;
		copy_buffer(ssize, max_sector,
			    2 * max_buffer_sectors + buffer_min);
	} else
		transfer_size(ssize, max_sector,
			      2 * max_buffer_sectors + buffer_min -
			      aligned_sector_t);

	
	raw_cmd->length = in_sector_offset + current_count_sectors;
	raw_cmd->length = ((raw_cmd->length - 1) | (ssize - 1)) + 1;
	raw_cmd->length <<= 9;
#ifdef FLOPPY_SANITY_CHECK
	if ((raw_cmd->length < current_count_sectors << 9) ||
	    (raw_cmd->kernel_data != current_req->buffer &&
	     CT(COMMAND) == FD_WRITE &&
	     (aligned_sector_t + (raw_cmd->length >> 9) > buffer_max ||
	      aligned_sector_t < buffer_min)) ||
	    raw_cmd->length % (128 << SIZECODE) ||
	    raw_cmd->length <= 0 || current_count_sectors <= 0) {
		DPRINT("fractionary current count b=%lx s=%lx\n",
		       raw_cmd->length, current_count_sectors);
		if (raw_cmd->kernel_data != current_req->buffer)
			printk("addr=%d, length=%ld\n",
			       (int)((raw_cmd->kernel_data -
				      floppy_track_buffer) >> 9),
			       current_count_sectors);
		printk("st=%d ast=%d mse=%d msi=%d\n",
		       fsector_t, aligned_sector_t, max_sector, max_size);
		printk("ssize=%x SIZECODE=%d\n", ssize, SIZECODE);
		printk("command=%x SECTOR=%d HEAD=%d, TRACK=%d\n",
		       COMMAND, SECTOR, HEAD, TRACK);
		printk("buffer drive=%d\n", buffer_drive);
		printk("buffer track=%d\n", buffer_track);
		printk("buffer_min=%d\n", buffer_min);
		printk("buffer_max=%d\n", buffer_max);
		return 0;
	}

	if (raw_cmd->kernel_data != current_req->buffer) {
		if (raw_cmd->kernel_data < floppy_track_buffer ||
		    current_count_sectors < 0 ||
		    raw_cmd->length < 0 ||
		    raw_cmd->kernel_data + raw_cmd->length >
		    floppy_track_buffer + (max_buffer_sectors << 10)) {
			DPRINT("buffer overrun in schedule dma\n");
			printk("fsector_t=%d buffer_min=%d current_count=%ld\n",
			       fsector_t, buffer_min, raw_cmd->length >> 9);
			printk("current_count_sectors=%ld\n",
			       current_count_sectors);
			if (CT(COMMAND) == FD_READ)
				printk("read\n");
			if (CT(COMMAND) == FD_WRITE)
				printk("write\n");
			return 0;
		}
	} else if (raw_cmd->length > blk_rq_bytes(current_req) ||
		   current_count_sectors > blk_rq_sectors(current_req)) {
		DPRINT("buffer overrun in direct transfer\n");
		return 0;
	} else if (raw_cmd->length < current_count_sectors << 9) {
		DPRINT("more sectors than bytes\n");
		printk("bytes=%ld\n", raw_cmd->length >> 9);
		printk("sectors=%ld\n", current_count_sectors);
	}
	if (raw_cmd->length == 0) {
		DPRINT("zero dma transfer attempted from make_raw_request\n");
		return 0;
	}
#endif

	virtualdmabug_workaround();
	return 2;
}

static void redo_fd_request(void)
{
#define REPEAT {request_done(0); continue; }
	int drive;
	int tmp;

	lastredo = jiffies;
	if (current_drive < N_DRIVE)
		floppy_off(current_drive);

	for (;;) {
		if (!current_req) {
			struct request *req;

			spin_lock_irq(floppy_queue->queue_lock);
			req = blk_fetch_request(floppy_queue);
			spin_unlock_irq(floppy_queue->queue_lock);
			if (!req) {
				do_floppy = NULL;
				unlock_fdc();
				return;
			}
			current_req = req;
		}
		drive = (long)current_req->rq_disk->private_data;
		set_fdc(drive);
		reschedule_timeout(current_reqD, "redo fd request", 0);

		set_floppy(drive);
		raw_cmd = &default_raw_cmd;
		raw_cmd->flags = 0;
		if (start_motor(redo_fd_request))
			return;
		disk_change(current_drive);
		if (test_bit(current_drive, &fake_change) ||
		    TESTF(FD_DISK_CHANGED)) {
			DPRINT("disk absent or changed during operation\n");
			REPEAT;
		}
		if (!_floppy) {	
			if (!probing) {
				DRS->probed_format = 0;
				if (next_valid_format()) {
					DPRINT("no autodetectable formats\n");
					_floppy = NULL;
					REPEAT;
				}
			}
			probing = 1;
			_floppy =
			    floppy_type + DP->autodetect[DRS->probed_format];
		} else
			probing = 0;
		errors = &(current_req->errors);
		tmp = make_raw_rw_request();
		if (tmp < 2) {
			request_done(tmp);
			continue;
		}

		if (TESTF(FD_NEED_TWADDLE))
			twaddle();
		schedule_bh(floppy_start);
		debugt("queue fd request");
		return;
	}
#undef REPEAT
}

static struct cont_t rw_cont = {
	.interrupt	= rw_interrupt,
	.redo		= redo_fd_request,
	.error		= bad_flp_intr,
	.done		= request_done
};

static void process_fd_request(void)
{
	cont = &rw_cont;
	schedule_bh(redo_fd_request);
}

static void do_fd_request(struct request_queue * q)
{
	if (max_buffer_sectors == 0) {
		printk("VFS: do_fd_request called on non-open device\n");
		return;
	}

	if (usage_count == 0) {
		printk("warning: usage count=0, current_req=%p exiting\n",
		       current_req);
		printk("sect=%ld type=%x flags=%x\n",
		       (long)blk_rq_pos(current_req), current_req->cmd_type,
		       current_req->cmd_flags);
		return;
	}
	if (test_bit(0, &fdc_busy)) {
		
		is_alive("do fd request, old request running");
		return;
	}
	lock_fdc(MAXTIMEOUT, 0);
	process_fd_request();
	is_alive("do fd request");
}

static struct cont_t poll_cont = {
	.interrupt	= success_and_wakeup,
	.redo		= floppy_ready,
	.error		= generic_failure,
	.done		= generic_done
};

static int poll_drive(int interruptible, int flag)
{
	int ret;

	
	raw_cmd = &default_raw_cmd;
	raw_cmd->flags = flag;
	raw_cmd->track = 0;
	raw_cmd->cmd_count = 0;
	cont = &poll_cont;
#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("setting NEWCHANGE in poll_drive\n");
	}
#endif
	SETF(FD_DISK_NEWCHANGE);
	WAIT(floppy_ready);
	return ret;
}



static void reset_intr(void)
{
	printk("weird, reset interrupt called\n");
}

static struct cont_t reset_cont = {
	.interrupt	= reset_intr,
	.redo		= success_and_wakeup,
	.error		= generic_failure,
	.done		= generic_done
};

static int user_reset_fdc(int drive, int arg, int interruptible)
{
	int ret;

	ret = 0;
	LOCK_FDC(drive, interruptible);
	if (arg == FD_RESET_ALWAYS)
		FDCS->reset = 1;
	if (FDCS->reset) {
		cont = &reset_cont;
		WAIT(reset_fdc);
	}
	process_fd_request();
	return ret;
}


static inline int fd_copyout(void __user *param, const void *address,
			     unsigned long size)
{
	return copy_to_user(param, address, size) ? -EFAULT : 0;
}

static inline int fd_copyin(void __user *param, void *address, unsigned long size)
{
	return copy_from_user(address, param, size) ? -EFAULT : 0;
}

#define _COPYOUT(x) (copy_to_user((void __user *)param, &(x), sizeof(x)) ? -EFAULT : 0)
#define _COPYIN(x) (copy_from_user(&(x), (void __user *)param, sizeof(x)) ? -EFAULT : 0)

#define COPYOUT(x) ECALL(_COPYOUT(x))
#define COPYIN(x) ECALL(_COPYIN(x))

static inline const char *drive_name(int type, int drive)
{
	struct floppy_struct *floppy;

	if (type)
		floppy = floppy_type + type;
	else {
		if (UDP->native_format)
			floppy = floppy_type + UDP->native_format;
		else
			return "(null)";
	}
	if (floppy->name)
		return floppy->name;
	else
		return "(null)";
}


static void raw_cmd_done(int flag)
{
	int i;

	if (!flag) {
		raw_cmd->flags |= FD_RAW_FAILURE;
		raw_cmd->flags |= FD_RAW_HARDFAILURE;
	} else {
		raw_cmd->reply_count = inr;
		if (raw_cmd->reply_count > MAX_REPLIES)
			raw_cmd->reply_count = 0;
		for (i = 0; i < raw_cmd->reply_count; i++)
			raw_cmd->reply[i] = reply_buffer[i];

		if (raw_cmd->flags & (FD_RAW_READ | FD_RAW_WRITE)) {
			unsigned long flags;
			flags = claim_dma_lock();
			raw_cmd->length = fd_get_dma_residue();
			release_dma_lock(flags);
		}

		if ((raw_cmd->flags & FD_RAW_SOFTFAILURE) &&
		    (!raw_cmd->reply_count || (raw_cmd->reply[0] & 0xc0)))
			raw_cmd->flags |= FD_RAW_FAILURE;

		if (disk_change(current_drive))
			raw_cmd->flags |= FD_RAW_DISK_CHANGE;
		else
			raw_cmd->flags &= ~FD_RAW_DISK_CHANGE;
		if (raw_cmd->flags & FD_RAW_NO_MOTOR_AFTER)
			motor_off_callback(current_drive);

		if (raw_cmd->next &&
		    (!(raw_cmd->flags & FD_RAW_FAILURE) ||
		     !(raw_cmd->flags & FD_RAW_STOP_IF_FAILURE)) &&
		    ((raw_cmd->flags & FD_RAW_FAILURE) ||
		     !(raw_cmd->flags & FD_RAW_STOP_IF_SUCCESS))) {
			raw_cmd = raw_cmd->next;
			return;
		}
	}
	generic_done(flag);
}

static struct cont_t raw_cmd_cont = {
	.interrupt	= success_and_wakeup,
	.redo		= floppy_start,
	.error		= generic_failure,
	.done		= raw_cmd_done
};

static inline int raw_cmd_copyout(int cmd, char __user *param,
				  struct floppy_raw_cmd *ptr)
{
	int ret;

	while (ptr) {
		COPYOUT(*ptr);
		param += sizeof(struct floppy_raw_cmd);
		if ((ptr->flags & FD_RAW_READ) && ptr->buffer_length) {
			if (ptr->length >= 0
			    && ptr->length <= ptr->buffer_length)
				ECALL(fd_copyout
				      (ptr->data, ptr->kernel_data,
				       ptr->buffer_length - ptr->length));
		}
		ptr = ptr->next;
	}
	return 0;
}

static void raw_cmd_free(struct floppy_raw_cmd **ptr)
{
	struct floppy_raw_cmd *next;
	struct floppy_raw_cmd *this;

	this = *ptr;
	*ptr = NULL;
	while (this) {
		if (this->buffer_length) {
			fd_dma_mem_free((unsigned long)this->kernel_data,
					this->buffer_length);
			this->buffer_length = 0;
		}
		next = this->next;
		kfree(this);
		this = next;
	}
}

static inline int raw_cmd_copyin(int cmd, char __user *param,
				 struct floppy_raw_cmd **rcmd)
{
	struct floppy_raw_cmd *ptr;
	int ret;
	int i;

	*rcmd = NULL;
	while (1) {
		ptr = (struct floppy_raw_cmd *)
		    kmalloc(sizeof(struct floppy_raw_cmd), GFP_USER);
		if (!ptr)
			return -ENOMEM;
		*rcmd = ptr;
		COPYIN(*ptr);
		ptr->next = NULL;
		ptr->buffer_length = 0;
		param += sizeof(struct floppy_raw_cmd);
		if (ptr->cmd_count > 33)
			
			return -EINVAL;

		for (i = 0; i < 16; i++)
			ptr->reply[i] = 0;
		ptr->resultcode = 0;
		ptr->kernel_data = NULL;

		if (ptr->flags & (FD_RAW_READ | FD_RAW_WRITE)) {
			if (ptr->length <= 0)
				return -EINVAL;
			ptr->kernel_data =
			    (char *)fd_dma_mem_alloc(ptr->length);
			fallback_on_nodma_alloc(&ptr->kernel_data, ptr->length);
			if (!ptr->kernel_data)
				return -ENOMEM;
			ptr->buffer_length = ptr->length;
		}
		if (ptr->flags & FD_RAW_WRITE)
			ECALL(fd_copyin(ptr->data, ptr->kernel_data,
					ptr->length));
		rcmd = &(ptr->next);
		if (!(ptr->flags & FD_RAW_MORE))
			return 0;
		ptr->rate &= 0x43;
	}
}

static int raw_cmd_ioctl(int cmd, void __user *param)
{
	struct floppy_raw_cmd *my_raw_cmd;
	int drive;
	int ret2;
	int ret;

	if (FDCS->rawcmd <= 1)
		FDCS->rawcmd = 1;
	for (drive = 0; drive < N_DRIVE; drive++) {
		if (FDC(drive) != fdc)
			continue;
		if (drive == current_drive) {
			if (UDRS->fd_ref > 1) {
				FDCS->rawcmd = 2;
				break;
			}
		} else if (UDRS->fd_ref) {
			FDCS->rawcmd = 2;
			break;
		}
	}

	if (FDCS->reset)
		return -EIO;

	ret = raw_cmd_copyin(cmd, param, &my_raw_cmd);
	if (ret) {
		raw_cmd_free(&my_raw_cmd);
		return ret;
	}

	raw_cmd = my_raw_cmd;
	cont = &raw_cmd_cont;
	ret = wait_til_done(floppy_start, 1);
#ifdef DCL_DEBUG
	if (DP->flags & FD_DEBUG) {
		DPRINT("calling disk change from raw_cmd ioctl\n");
	}
#endif

	if (ret != -EINTR && FDCS->reset)
		ret = -EIO;

	DRS->track = NO_TRACK;

	ret2 = raw_cmd_copyout(cmd, param, my_raw_cmd);
	if (!ret)
		ret = ret2;
	raw_cmd_free(&my_raw_cmd);
	return ret;
}

static int invalidate_drive(struct block_device *bdev)
{
	
	set_bit((long)bdev->bd_disk->private_data, &fake_change);
	process_fd_request();
	check_disk_change(bdev);
	return 0;
}

static inline int set_geometry(unsigned int cmd, struct floppy_struct *g,
			       int drive, int type, struct block_device *bdev)
{
	int cnt;

	
	if (g->sect <= 0 ||
	    g->head <= 0 ||
	    g->track <= 0 || g->track > UDP->tracks >> STRETCH(g) ||
	    
	    (g->stretch & ~(FD_STRETCH | FD_SWAPSIDES | FD_SECTBASEMASK)) != 0)
		return -EINVAL;
	if (type) {
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		mutex_lock(&open_lock);
		if (lock_fdc(drive, 1)) {
			mutex_unlock(&open_lock);
			return -EINTR;
		}
		floppy_type[type] = *g;
		floppy_type[type].name = "user format";
		for (cnt = type << 2; cnt < (type << 2) + 4; cnt++)
			floppy_sizes[cnt] = floppy_sizes[cnt + 0x80] =
			    floppy_type[type].size + 1;
		process_fd_request();
		for (cnt = 0; cnt < N_DRIVE; cnt++) {
			struct block_device *bdev = opened_bdev[cnt];
			if (!bdev || ITYPE(drive_state[cnt].fd_device) != type)
				continue;
			__invalidate_device(bdev);
		}
		mutex_unlock(&open_lock);
	} else {
		int oldStretch;
		LOCK_FDC(drive, 1);
		if (cmd != FDDEFPRM)
			
			CALL(poll_drive(1, FD_RAW_NEED_DISK));
		oldStretch = g->stretch;
		user_params[drive] = *g;
		if (buffer_drive == drive)
			SUPBOUND(buffer_max, user_params[drive].sect);
		current_type[drive] = &user_params[drive];
		floppy_sizes[drive] = user_params[drive].size;
		if (cmd == FDDEFPRM)
			DRS->keep_data = -1;
		else
			DRS->keep_data = 1;
		
		if (DRS->maxblock > user_params[drive].sect ||
		    DRS->maxtrack ||
		    ((user_params[drive].sect ^ oldStretch) &
		     (FD_SWAPSIDES | FD_SECTBASEMASK)))
			invalidate_drive(bdev);
		else
			process_fd_request();
	}
	return 0;
}


static int ioctl_table[] = {
	FDCLRPRM,
	FDSETPRM,
	FDDEFPRM,
	FDGETPRM,
	FDMSGON,
	FDMSGOFF,
	FDFMTBEG,
	FDFMTTRK,
	FDFMTEND,
	FDSETEMSGTRESH,
	FDFLUSH,
	FDSETMAXERRS,
	FDGETMAXERRS,
	FDGETDRVTYP,
	FDSETDRVPRM,
	FDGETDRVPRM,
	FDGETDRVSTAT,
	FDPOLLDRVSTAT,
	FDRESET,
	FDGETFDCSTAT,
	FDWERRORCLR,
	FDWERRORGET,
	FDRAWCMD,
	FDEJECT,
	FDTWADDLE
};

static inline int normalize_ioctl(int *cmd, int *size)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ioctl_table); i++) {
		if ((*cmd & 0xffff) == (ioctl_table[i] & 0xffff)) {
			*size = _IOC_SIZE(*cmd);
			*cmd = ioctl_table[i];
			if (*size > _IOC_SIZE(*cmd)) {
				printk("ioctl not yet supported\n");
				return -EFAULT;
			}
			return 0;
		}
	}
	return -EINVAL;
}

static int get_floppy_geometry(int drive, int type, struct floppy_struct **g)
{
	if (type)
		*g = &floppy_type[type];
	else {
		LOCK_FDC(drive, 0);
		CALL(poll_drive(0, 0));
		process_fd_request();
		*g = current_type[drive];
	}
	if (!*g)
		return -ENODEV;
	return 0;
}

static int fd_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	int drive = (long)bdev->bd_disk->private_data;
	int type = ITYPE(drive_state[drive].fd_device);
	struct floppy_struct *g;
	int ret;

	ret = get_floppy_geometry(drive, type, &g);
	if (ret)
		return ret;

	geo->heads = g->head;
	geo->sectors = g->sect;
	geo->cylinders = g->track;
	return 0;
}

static int fd_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd,
		    unsigned long param)
{
#define FD_IOCTL_ALLOWED (mode & (FMODE_WRITE|FMODE_WRITE_IOCTL))
#define OUT(c,x) case c: outparam = (const char *) (x); break
#define IN(c,x,tag) case c: *(x) = inparam. tag ; return 0

	int drive = (long)bdev->bd_disk->private_data;
	int type = ITYPE(UDRS->fd_device);
	int i;
	int ret;
	int size;
	union inparam {
		struct floppy_struct g;	
		struct format_descr f;
		struct floppy_max_errors max_errors;
		struct floppy_drive_params dp;
	} inparam;		
	const char *outparam;	

	
	if (cmd == CDROMEJECT ||	
	    cmd == 0x6470  ) {
		DPRINT("obsolete eject ioctl\n");
		DPRINT("please use floppycontrol --eject\n");
		cmd = FDEJECT;
	}

	
	if ((cmd & 0xff00) == 0x0200) {
		ECALL(normalize_ioctl(&cmd, &size));
	} else
		return -EINVAL;

	
	if (((cmd & 0x40) && !FD_IOCTL_ALLOWED) ||
	    ((cmd & 0x80) && !capable(CAP_SYS_ADMIN)))
		return -EPERM;

	
	CLEARSTRUCT(&inparam);
	if (_IOC_DIR(cmd) & _IOC_WRITE)
	    ECALL(fd_copyin((void __user *)param, &inparam, size))

		switch (cmd) {
		case FDEJECT:
			if (UDRS->fd_ref != 1)
				
				return -EBUSY;
			LOCK_FDC(drive, 1);

			
			ret = fd_eject(UNIT(drive));

			USETF(FD_DISK_CHANGED);
			USETF(FD_VERIFY);
			process_fd_request();
			return ret;
		case FDCLRPRM:
			LOCK_FDC(drive, 1);
			current_type[drive] = NULL;
			floppy_sizes[drive] = MAX_DISK_SIZE << 1;
			UDRS->keep_data = 0;
			return invalidate_drive(bdev);
		case FDSETPRM:
		case FDDEFPRM:
			return set_geometry(cmd, &inparam.g,
					    drive, type, bdev);
		case FDGETPRM:
			ECALL(get_floppy_geometry(drive, type,
						  (struct floppy_struct **)
						  &outparam));
			break;

		case FDMSGON:
			UDP->flags |= FTD_MSG;
			return 0;
		case FDMSGOFF:
			UDP->flags &= ~FTD_MSG;
			return 0;

		case FDFMTBEG:
			LOCK_FDC(drive, 1);
			CALL(poll_drive(1, FD_RAW_NEED_DISK));
			ret = UDRS->flags;
			process_fd_request();
			if (ret & FD_VERIFY)
				return -ENODEV;
			if (!(ret & FD_DISK_WRITABLE))
				return -EROFS;
			return 0;
		case FDFMTTRK:
			if (UDRS->fd_ref != 1)
				return -EBUSY;
			return do_format(drive, &inparam.f);
		case FDFMTEND:
		case FDFLUSH:
			LOCK_FDC(drive, 1);
			return invalidate_drive(bdev);

		case FDSETEMSGTRESH:
			UDP->max_errors.reporting =
			    (unsigned short)(param & 0x0f);
			return 0;
			OUT(FDGETMAXERRS, &UDP->max_errors);
			IN(FDSETMAXERRS, &UDP->max_errors, max_errors);

		case FDGETDRVTYP:
			outparam = drive_name(type, drive);
			SUPBOUND(size, strlen(outparam) + 1);
			break;

			IN(FDSETDRVPRM, UDP, dp);
			OUT(FDGETDRVPRM, UDP);

		case FDPOLLDRVSTAT:
			LOCK_FDC(drive, 1);
			CALL(poll_drive(1, FD_RAW_NEED_DISK));
			process_fd_request();
			
			OUT(FDGETDRVSTAT, UDRS);

		case FDRESET:
			return user_reset_fdc(drive, (int)param, 1);

			OUT(FDGETFDCSTAT, UFDCS);

		case FDWERRORCLR:
			CLEARSTRUCT(UDRWE);
			return 0;
			OUT(FDWERRORGET, UDRWE);

		case FDRAWCMD:
			if (type)
				return -EINVAL;
			LOCK_FDC(drive, 1);
			set_floppy(drive);
			CALL(i = raw_cmd_ioctl(cmd, (void __user *)param));
			process_fd_request();
			return i;

		case FDTWADDLE:
			LOCK_FDC(drive, 1);
			twaddle();
			process_fd_request();
			return 0;

		default:
			return -EINVAL;
		}

	if (_IOC_DIR(cmd) & _IOC_READ)
		return fd_copyout((void __user *)param, outparam, size);
	else
		return 0;
#undef OUT
#undef IN
}

static void __init config_types(void)
{
	int first = 1;
	int drive;

	
	drive = 0;
	if (!UDP->cmos)
		UDP->cmos = FLOPPY0_TYPE;
	drive = 1;
	if (!UDP->cmos && FLOPPY1_TYPE)
		UDP->cmos = FLOPPY1_TYPE;

	

	for (drive = 0; drive < N_DRIVE; drive++) {
		unsigned int type = UDP->cmos;
		struct floppy_drive_params *params;
		const char *name = NULL;
		static char temparea[32];

		if (type < ARRAY_SIZE(default_drive_params)) {
			params = &default_drive_params[type].params;
			if (type) {
				name = default_drive_params[type].name;
				allowed_drive_mask |= 1 << drive;
			} else
				allowed_drive_mask &= ~(1 << drive);
		} else {
			params = &default_drive_params[0].params;
			sprintf(temparea, "unknown type %d (usb?)", type);
			name = temparea;
		}
		if (name) {
			const char *prepend = ",";
			if (first) {
				prepend = KERN_INFO "Floppy drive(s):";
				first = 0;
			}
			printk("%s fd%d is %s", prepend, drive, name);
		}
		*UDP = *params;
	}
	if (!first)
		printk("\n");
}

static int floppy_release(struct gendisk *disk, fmode_t mode)
{
	int drive = (long)disk->private_data;

	mutex_lock(&open_lock);
	if (UDRS->fd_ref < 0)
		UDRS->fd_ref = 0;
	else if (!UDRS->fd_ref--) {
		DPRINT("floppy_release with fd_ref == 0");
		UDRS->fd_ref = 0;
	}
	if (!UDRS->fd_ref)
		opened_bdev[drive] = NULL;
	mutex_unlock(&open_lock);

	return 0;
}


static int floppy_open(struct block_device *bdev, fmode_t mode)
{
	int drive = (long)bdev->bd_disk->private_data;
	int old_dev, new_dev;
	int try;
	int res = -EBUSY;
	char *tmp;

	mutex_lock(&open_lock);
	old_dev = UDRS->fd_device;
	if (opened_bdev[drive] && opened_bdev[drive] != bdev)
		goto out2;

	if (!UDRS->fd_ref && (UDP->flags & FD_BROKEN_DCL)) {
		USETF(FD_DISK_CHANGED);
		USETF(FD_VERIFY);
	}

	if (UDRS->fd_ref == -1 || (UDRS->fd_ref && (mode & FMODE_EXCL)))
		goto out2;

	if (mode & FMODE_EXCL)
		UDRS->fd_ref = -1;
	else
		UDRS->fd_ref++;

	opened_bdev[drive] = bdev;

	res = -ENXIO;

	if (!floppy_track_buffer) {
		
		if ((UDP->cmos == 6) || (UDP->cmos == 5))
			try = 64;	
		else
			try = 32;	

		tmp = (char *)fd_dma_mem_alloc(1024 * try);
		if (!tmp && !floppy_track_buffer) {
			try >>= 1;	
			INFBOUND(try, 16);
			tmp = (char *)fd_dma_mem_alloc(1024 * try);
		}
		if (!tmp && !floppy_track_buffer) {
			fallback_on_nodma_alloc(&tmp, 2048 * try);
		}
		if (!tmp && !floppy_track_buffer) {
			DPRINT("Unable to allocate DMA memory\n");
			goto out;
		}
		if (floppy_track_buffer) {
			if (tmp)
				fd_dma_mem_free((unsigned long)tmp, try * 1024);
		} else {
			buffer_min = buffer_max = -1;
			floppy_track_buffer = tmp;
			max_buffer_sectors = try;
		}
	}

	new_dev = MINOR(bdev->bd_dev);
	UDRS->fd_device = new_dev;
	set_capacity(disks[drive], floppy_sizes[new_dev]);
	if (old_dev != -1 && old_dev != new_dev) {
		if (buffer_drive == drive)
			buffer_track = -1;
	}

	if (UFDCS->rawcmd == 1)
		UFDCS->rawcmd = 2;

	if (!(mode & FMODE_NDELAY)) {
		if (mode & (FMODE_READ|FMODE_WRITE)) {
			UDRS->last_checked = 0;
			check_disk_change(bdev);
			if (UTESTF(FD_DISK_CHANGED))
				goto out;
		}
		res = -EROFS;
		if ((mode & FMODE_WRITE) && !(UTESTF(FD_DISK_WRITABLE)))
			goto out;
	}
	mutex_unlock(&open_lock);
	return 0;
out:
	if (UDRS->fd_ref < 0)
		UDRS->fd_ref = 0;
	else
		UDRS->fd_ref--;
	if (!UDRS->fd_ref)
		opened_bdev[drive] = NULL;
out2:
	mutex_unlock(&open_lock);
	return res;
}


static int check_floppy_change(struct gendisk *disk)
{
	int drive = (long)disk->private_data;

	if (UTESTF(FD_DISK_CHANGED) || UTESTF(FD_VERIFY))
		return 1;

	if (time_after(jiffies, UDRS->last_checked + UDP->checkfreq)) {
		lock_fdc(drive, 0);
		poll_drive(0, 0);
		process_fd_request();
	}

	if (UTESTF(FD_DISK_CHANGED) ||
	    UTESTF(FD_VERIFY) ||
	    test_bit(drive, &fake_change) ||
	    (!ITYPE(UDRS->fd_device) && !current_type[drive]))
		return 1;
	return 0;
}



static void floppy_rb0_complete(struct bio *bio,
			       int err)
{
	complete((struct completion *)bio->bi_private);
}

static int __floppy_read_block_0(struct block_device *bdev)
{
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;
	size_t size;

	page = alloc_page(GFP_NOIO);
	if (!page) {
		process_fd_request();
		return -ENOMEM;
	}

	size = bdev->bd_block_size;
	if (!size)
		size = 1024;

	bio_init(&bio);
	bio.bi_io_vec = &bio_vec;
	bio_vec.bv_page = page;
	bio_vec.bv_len = size;
	bio_vec.bv_offset = 0;
	bio.bi_vcnt = 1;
	bio.bi_idx = 0;
	bio.bi_size = size;
	bio.bi_bdev = bdev;
	bio.bi_sector = 0;
	init_completion(&complete);
	bio.bi_private = &complete;
	bio.bi_end_io = floppy_rb0_complete;

	submit_bio(READ, &bio);
	generic_unplug_device(bdev_get_queue(bdev));
	process_fd_request();
	wait_for_completion(&complete);

	__free_page(page);

	return 0;
}


static int floppy_revalidate(struct gendisk *disk)
{
	int drive = (long)disk->private_data;
#define NO_GEOM (!current_type[drive] && !ITYPE(UDRS->fd_device))
	int cf;
	int res = 0;

	if (UTESTF(FD_DISK_CHANGED) ||
	    UTESTF(FD_VERIFY) || test_bit(drive, &fake_change) || NO_GEOM) {
		if (usage_count == 0) {
			printk("VFS: revalidate called on non-open device.\n");
			return -EFAULT;
		}
		lock_fdc(drive, 0);
		cf = UTESTF(FD_DISK_CHANGED) || UTESTF(FD_VERIFY);
		if (!(cf || test_bit(drive, &fake_change) || NO_GEOM)) {
			process_fd_request();	
			return 0;
		}
		UDRS->maxblock = 0;
		UDRS->maxtrack = 0;
		if (buffer_drive == drive)
			buffer_track = -1;
		clear_bit(drive, &fake_change);
		UCLEARF(FD_DISK_CHANGED);
		if (cf)
			UDRS->generation++;
		if (NO_GEOM) {
			
			res = __floppy_read_block_0(opened_bdev[drive]);
		} else {
			if (cf)
				poll_drive(0, FD_RAW_NEED_DISK);
			process_fd_request();
		}
	}
	set_capacity(disk, floppy_sizes[UDRS->fd_device]);
	return res;
}

static const struct block_device_operations floppy_fops = {
	.owner			= THIS_MODULE,
	.open			= floppy_open,
	.release		= floppy_release,
	.locked_ioctl		= fd_ioctl,
	.getgeo			= fd_getgeo,
	.media_changed		= check_floppy_change,
	.revalidate_disk	= floppy_revalidate,
};





static char __init get_fdc_version(void)
{
	int r;

	output_byte(FD_DUMPREGS);	
	if (FDCS->reset)
		return FDC_NONE;
	if ((r = result()) <= 0x00)
		return FDC_NONE;	
	if ((r == 1) && (reply_buffer[0] == 0x80)) {
		printk(KERN_INFO "FDC %d is an 8272A\n", fdc);
		return FDC_8272A;	
	}
	if (r != 10) {
		printk
		    ("FDC %d init: DUMPREGS: unexpected return of %d bytes.\n",
		     fdc, r);
		return FDC_UNKNOWN;
	}

	if (!fdc_configure()) {
		printk(KERN_INFO "FDC %d is an 82072\n", fdc);
		return FDC_82072;	
	}

	output_byte(FD_PERPENDICULAR);
	if (need_more_output() == MORE_OUTPUT) {
		output_byte(0);
	} else {
		printk(KERN_INFO "FDC %d is an 82072A\n", fdc);
		return FDC_82072A;	
	}

	output_byte(FD_UNLOCK);
	r = result();
	if ((r == 1) && (reply_buffer[0] == 0x80)) {
		printk(KERN_INFO "FDC %d is a pre-1991 82077\n", fdc);
		return FDC_82077_ORIG;	
	}
	if ((r != 1) || (reply_buffer[0] != 0x00)) {
		printk("FDC %d init: UNLOCK: unexpected return of %d bytes.\n",
		       fdc, r);
		return FDC_UNKNOWN;
	}
	output_byte(FD_PARTID);
	r = result();
	if (r != 1) {
		printk("FDC %d init: PARTID: unexpected return of %d bytes.\n",
		       fdc, r);
		return FDC_UNKNOWN;
	}
	if (reply_buffer[0] == 0x80) {
		printk(KERN_INFO "FDC %d is a post-1991 82077\n", fdc);
		return FDC_82077;	
	}
	switch (reply_buffer[0] >> 5) {
	case 0x0:
		
		printk(KERN_INFO "FDC %d is an 82078.\n", fdc);
		return FDC_82078;
	case 0x1:
		printk(KERN_INFO "FDC %d is a 44pin 82078\n", fdc);
		return FDC_82078;
	case 0x2:
		printk(KERN_INFO "FDC %d is a S82078B\n", fdc);
		return FDC_S82078B;
	case 0x3:
		printk(KERN_INFO "FDC %d is a National Semiconductor PC87306\n",
		       fdc);
		return FDC_87306;
	default:
		printk(KERN_INFO
		       "FDC %d init: 82078 variant with unknown PARTID=%d.\n",
		       fdc, reply_buffer[0] >> 5);
		return FDC_82078_UNKN;
	}
}				



static void __init floppy_set_flags(int *ints, int param, int param2)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(default_drive_params); i++) {
		if (param)
			default_drive_params[i].params.flags |= param2;
		else
			default_drive_params[i].params.flags &= ~param2;
	}
	DPRINT("%s flag 0x%x\n", param2 ? "Setting" : "Clearing", param);
}

static void __init daring(int *ints, int param, int param2)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(default_drive_params); i++) {
		if (param) {
			default_drive_params[i].params.select_delay = 0;
			default_drive_params[i].params.flags |=
			    FD_SILENT_DCL_CLEAR;
		} else {
			default_drive_params[i].params.select_delay =
			    2 * HZ / 100;
			default_drive_params[i].params.flags &=
			    ~FD_SILENT_DCL_CLEAR;
		}
	}
	DPRINT("Assuming %s floppy hardware\n", param ? "standard" : "broken");
}

static void __init set_cmos(int *ints, int dummy, int dummy2)
{
	int current_drive = 0;

	if (ints[0] != 2) {
		DPRINT("wrong number of parameters for CMOS\n");
		return;
	}
	current_drive = ints[1];
	if (current_drive < 0 || current_drive >= 8) {
		DPRINT("bad drive for set_cmos\n");
		return;
	}
#if N_FDC > 1
	if (current_drive >= 4 && !FDC2)
		FDC2 = 0x370;
#endif
	DP->cmos = ints[2];
	DPRINT("setting CMOS code to %d\n", ints[2]);
}

static struct param_table {
	const char *name;
	void (*fn) (int *ints, int param, int param2);
	int *var;
	int def_param;
	int param2;
} config_params[] __initdata = {
	{"allowed_drive_mask", NULL, &allowed_drive_mask, 0xff, 0}, 
	{"all_drives", NULL, &allowed_drive_mask, 0xff, 0},	
	{"asus_pci", NULL, &allowed_drive_mask, 0x33, 0},
	{"irq", NULL, &FLOPPY_IRQ, 6, 0},
	{"dma", NULL, &FLOPPY_DMA, 2, 0},
	{"daring", daring, NULL, 1, 0},
#if N_FDC > 1
	{"two_fdc", NULL, &FDC2, 0x370, 0},
	{"one_fdc", NULL, &FDC2, 0, 0},
#endif
	{"thinkpad", floppy_set_flags, NULL, 1, FD_INVERTED_DCL},
	{"broken_dcl", floppy_set_flags, NULL, 1, FD_BROKEN_DCL},
	{"messages", floppy_set_flags, NULL, 1, FTD_MSG},
	{"silent_dcl_clear", floppy_set_flags, NULL, 1, FD_SILENT_DCL_CLEAR},
	{"debug", floppy_set_flags, NULL, 1, FD_DEBUG},
	{"nodma", NULL, &can_use_virtual_dma, 1, 0},
	{"omnibook", NULL, &can_use_virtual_dma, 1, 0},
	{"yesdma", NULL, &can_use_virtual_dma, 0, 0},
	{"fifo_depth", NULL, &fifo_depth, 0xa, 0},
	{"nofifo", NULL, &no_fifo, 0x20, 0},
	{"usefifo", NULL, &no_fifo, 0, 0},
	{"cmos", set_cmos, NULL, 0, 0},
	{"slow", NULL, &slow_floppy, 1, 0},
	{"unexpected_interrupts", NULL, &print_unex, 1, 0},
	{"no_unexpected_interrupts", NULL, &print_unex, 0, 0},
	{"L40SX", NULL, &print_unex, 0, 0}

	EXTRA_FLOPPY_PARAMS
};

static int __init floppy_setup(char *str)
{
	int i;
	int param;
	int ints[11];

	str = get_options(str, ARRAY_SIZE(ints), ints);
	if (str) {
		for (i = 0; i < ARRAY_SIZE(config_params); i++) {
			if (strcmp(str, config_params[i].name) == 0) {
				if (ints[0])
					param = ints[1];
				else
					param = config_params[i].def_param;
				if (config_params[i].fn)
					config_params[i].
					    fn(ints, param,
					       config_params[i].param2);
				if (config_params[i].var) {
					DPRINT("%s=%d\n", str, param);
					*config_params[i].var = param;
				}
				return 1;
			}
		}
	}
	if (str) {
		DPRINT("unknown floppy option [%s]\n", str);

		DPRINT("allowed options are:");
		for (i = 0; i < ARRAY_SIZE(config_params); i++)
			printk(" %s", config_params[i].name);
		printk("\n");
	} else
		DPRINT("botched floppy option\n");
	DPRINT("Read Documentation/blockdev/floppy.txt\n");
	return 0;
}

static int have_no_fdc = -ENODEV;

static ssize_t floppy_cmos_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct platform_device *p = to_platform_device(dev);
	int drive;

	drive = p->id;
	return sprintf(buf, "%X\n", UDP->cmos);
}
DEVICE_ATTR(cmos,S_IRUGO,floppy_cmos_show,NULL);

static void floppy_device_release(struct device *dev)
{
}

static int floppy_resume(struct device *dev)
{
	int fdc;

	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			user_reset_fdc(-1, FD_RESET_ALWAYS, 0);

	return 0;
}

static struct dev_pm_ops floppy_pm_ops = {
	.resume = floppy_resume,
	.restore = floppy_resume,
};

static struct platform_driver floppy_driver = {
	.driver = {
		.name = "floppy",
		.pm = &floppy_pm_ops,
	},
};

static struct platform_device floppy_device[N_DRIVE];

static struct kobject *floppy_find(dev_t dev, int *part, void *data)
{
	int drive = (*part & 3) | ((*part & 0x80) >> 5);
	if (drive >= N_DRIVE ||
	    !(allowed_drive_mask & (1 << drive)) ||
	    fdc_state[FDC(drive)].version == FDC_NONE)
		return NULL;
	if (((*part >> 2) & 0x1f) >= ARRAY_SIZE(floppy_type))
		return NULL;
	*part = 0;
	return get_disk(disks[drive]);
}

static int __init floppy_init(void)
{
	int i, unit, drive;
	int err, dr;

#if defined(CONFIG_PPC)
	if (check_legacy_ioport(FDC1))
		return -ENODEV;
#endif

	raw_cmd = NULL;

	for (dr = 0; dr < N_DRIVE; dr++) {
		disks[dr] = alloc_disk(1);
		if (!disks[dr]) {
			err = -ENOMEM;
			goto out_put_disk;
		}

		disks[dr]->major = FLOPPY_MAJOR;
		disks[dr]->first_minor = TOMINOR(dr);
		disks[dr]->fops = &floppy_fops;
		sprintf(disks[dr]->disk_name, "fd%d", dr);

		init_timer(&motor_off_timer[dr]);
		motor_off_timer[dr].data = dr;
		motor_off_timer[dr].function = motor_off_callback;
	}

	err = register_blkdev(FLOPPY_MAJOR, "fd");
	if (err)
		goto out_put_disk;

	err = platform_driver_register(&floppy_driver);
	if (err)
		goto out_unreg_blkdev;

	floppy_queue = blk_init_queue(do_fd_request, &floppy_lock);
	if (!floppy_queue) {
		err = -ENOMEM;
		goto out_unreg_driver;
	}
	blk_queue_max_sectors(floppy_queue, 64);

	blk_register_region(MKDEV(FLOPPY_MAJOR, 0), 256, THIS_MODULE,
			    floppy_find, NULL, NULL);

	for (i = 0; i < 256; i++)
		if (ITYPE(i))
			floppy_sizes[i] = floppy_type[ITYPE(i)].size;
		else
			floppy_sizes[i] = MAX_DISK_SIZE << 1;

	reschedule_timeout(MAXTIMEOUT, "floppy init", MAXTIMEOUT);
	config_types();

	for (i = 0; i < N_FDC; i++) {
		fdc = i;
		CLEARSTRUCT(FDCS);
		FDCS->dtr = -1;
		FDCS->dor = 0x4;
#if defined(__sparc__) || defined(__mc68000__)
		
#ifdef __mc68000__
		if (MACH_IS_SUN3X)
#endif
			FDCS->version = FDC_82072A;
#endif
	}

	use_virtual_dma = can_use_virtual_dma & 1;
	fdc_state[0].address = FDC1;
	if (fdc_state[0].address == -1) {
		del_timer(&fd_timeout);
		err = -ENODEV;
		goto out_unreg_region;
	}
#if N_FDC > 1
	fdc_state[1].address = FDC2;
#endif

	fdc = 0;		
	err = floppy_grab_irq_and_dma();
	if (err) {
		del_timer(&fd_timeout);
		err = -EBUSY;
		goto out_unreg_region;
	}

	
	for (drive = 0; drive < N_DRIVE; drive++) {
		CLEARSTRUCT(UDRS);
		CLEARSTRUCT(UDRWE);
		USETF(FD_DISK_NEWCHANGE);
		USETF(FD_DISK_CHANGED);
		USETF(FD_VERIFY);
		UDRS->fd_device = -1;
		floppy_track_buffer = NULL;
		max_buffer_sectors = 0;
	}
	
	msleep(10);

	for (i = 0; i < N_FDC; i++) {
		fdc = i;
		FDCS->driver_version = FD_DRIVER_VERSION;
		for (unit = 0; unit < 4; unit++)
			FDCS->track[unit] = 0;
		if (FDCS->address == -1)
			continue;
		FDCS->rawcmd = 2;
		if (user_reset_fdc(-1, FD_RESET_ALWAYS, 0)) {
			
			floppy_release_regions(fdc);
			FDCS->address = -1;
			FDCS->version = FDC_NONE;
			continue;
		}
		
		FDCS->version = get_fdc_version();
		if (FDCS->version == FDC_NONE) {
			
			floppy_release_regions(fdc);
			FDCS->address = -1;
			continue;
		}
		if (can_use_virtual_dma == 2 && FDCS->version < FDC_82072A)
			can_use_virtual_dma = 0;

		have_no_fdc = 0;
		
		user_reset_fdc(-1, FD_RESET_ALWAYS, 0);
	}
	fdc = 0;
	del_timer(&fd_timeout);
	current_drive = 0;
	initialising = 0;
	if (have_no_fdc) {
		DPRINT("no floppy controllers found\n");
		err = have_no_fdc;
		goto out_flush_work;
	}

	for (drive = 0; drive < N_DRIVE; drive++) {
		if (!(allowed_drive_mask & (1 << drive)))
			continue;
		if (fdc_state[FDC(drive)].version == FDC_NONE)
			continue;

		floppy_device[drive].name = floppy_device_name;
		floppy_device[drive].id = drive;
		floppy_device[drive].dev.release = floppy_device_release;

		err = platform_device_register(&floppy_device[drive]);
		if (err)
			goto out_flush_work;

		err = device_create_file(&floppy_device[drive].dev,&dev_attr_cmos);
		if (err)
			goto out_unreg_platform_dev;

		
		disks[drive]->private_data = (void *)(long)drive;
		disks[drive]->queue = floppy_queue;
		disks[drive]->flags |= GENHD_FL_REMOVABLE;
		disks[drive]->driverfs_dev = &floppy_device[drive].dev;
		add_disk(disks[drive]);
	}

	return 0;

out_unreg_platform_dev:
	platform_device_unregister(&floppy_device[drive]);
out_flush_work:
	flush_scheduled_work();
	if (usage_count)
		floppy_release_irq_and_dma();
out_unreg_region:
	blk_unregister_region(MKDEV(FLOPPY_MAJOR, 0), 256);
	blk_cleanup_queue(floppy_queue);
out_unreg_driver:
	platform_driver_unregister(&floppy_driver);
out_unreg_blkdev:
	unregister_blkdev(FLOPPY_MAJOR, "fd");
out_put_disk:
	while (dr--) {
		del_timer(&motor_off_timer[dr]);
		put_disk(disks[dr]);
	}
	return err;
}

static DEFINE_SPINLOCK(floppy_usage_lock);

static const struct io_region {
	int offset;
	int size;
} io_regions[] = {
	{ 2, 1 },
	
	{ 4, 2 },
	
	{ 7, 1 },
};

static void floppy_release_allocated_regions(int fdc, const struct io_region *p)
{
	while (p != io_regions) {
		p--;
		release_region(FDCS->address + p->offset, p->size);
	}
}

#define ARRAY_END(X) (&((X)[ARRAY_SIZE(X)]))

static int floppy_request_regions(int fdc)
{
	const struct io_region *p;

	for (p = io_regions; p < ARRAY_END(io_regions); p++) {
		if (!request_region(FDCS->address + p->offset, p->size, "floppy")) {
			DPRINT("Floppy io-port 0x%04lx in use\n", FDCS->address + p->offset);
			floppy_release_allocated_regions(fdc, p);
			return -EBUSY;
		}
	}
	return 0;
}

static void floppy_release_regions(int fdc)
{
	floppy_release_allocated_regions(fdc, ARRAY_END(io_regions));
}

static int floppy_grab_irq_and_dma(void)
{
	unsigned long flags;

	spin_lock_irqsave(&floppy_usage_lock, flags);
	if (usage_count++) {
		spin_unlock_irqrestore(&floppy_usage_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&floppy_usage_lock, flags);

	
	flush_scheduled_work();

	if (fd_request_irq()) {
		DPRINT("Unable to grab IRQ%d for the floppy driver\n",
		       FLOPPY_IRQ);
		spin_lock_irqsave(&floppy_usage_lock, flags);
		usage_count--;
		spin_unlock_irqrestore(&floppy_usage_lock, flags);
		return -1;
	}
	if (fd_request_dma()) {
		DPRINT("Unable to grab DMA%d for the floppy driver\n",
		       FLOPPY_DMA);
		if (can_use_virtual_dma & 2)
			use_virtual_dma = can_use_virtual_dma = 1;
		if (!(can_use_virtual_dma & 1)) {
			fd_free_irq();
			spin_lock_irqsave(&floppy_usage_lock, flags);
			usage_count--;
			spin_unlock_irqrestore(&floppy_usage_lock, flags);
			return -1;
		}
	}

	for (fdc = 0; fdc < N_FDC; fdc++) {
		if (FDCS->address != -1) {
			if (floppy_request_regions(fdc))
				goto cleanup;
		}
	}
	for (fdc = 0; fdc < N_FDC; fdc++) {
		if (FDCS->address != -1) {
			reset_fdc_info(1);
			fd_outb(FDCS->dor, FD_DOR);
		}
	}
	fdc = 0;
	set_dor(0, ~0, 8);	

	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			fd_outb(FDCS->dor, FD_DOR);
	
	fdc = 0;
	irqdma_allocated = 1;
	return 0;
cleanup:
	fd_free_irq();
	fd_free_dma();
	while (--fdc >= 0)
		floppy_release_regions(fdc);
	spin_lock_irqsave(&floppy_usage_lock, flags);
	usage_count--;
	spin_unlock_irqrestore(&floppy_usage_lock, flags);
	return -1;
}

static void floppy_release_irq_and_dma(void)
{
	int old_fdc;
#ifdef FLOPPY_SANITY_CHECK
#ifndef __sparc__
	int drive;
#endif
#endif
	long tmpsize;
	unsigned long tmpaddr;
	unsigned long flags;

	spin_lock_irqsave(&floppy_usage_lock, flags);
	if (--usage_count) {
		spin_unlock_irqrestore(&floppy_usage_lock, flags);
		return;
	}
	spin_unlock_irqrestore(&floppy_usage_lock, flags);
	if (irqdma_allocated) {
		fd_disable_dma();
		fd_free_dma();
		fd_free_irq();
		irqdma_allocated = 0;
	}
	set_dor(0, ~0, 8);
#if N_FDC > 1
	set_dor(1, ~8, 0);
#endif
	floppy_enable_hlt();

	if (floppy_track_buffer && max_buffer_sectors) {
		tmpsize = max_buffer_sectors * 1024;
		tmpaddr = (unsigned long)floppy_track_buffer;
		floppy_track_buffer = NULL;
		max_buffer_sectors = 0;
		buffer_min = buffer_max = -1;
		fd_dma_mem_free(tmpaddr, tmpsize);
	}
#ifdef FLOPPY_SANITY_CHECK
#ifndef __sparc__
	for (drive = 0; drive < N_FDC * 4; drive++)
		if (timer_pending(motor_off_timer + drive))
			printk("motor off timer %d still active\n", drive);
#endif

	if (timer_pending(&fd_timeout))
		printk("floppy timer still active:%s\n", timeout_message);
	if (timer_pending(&fd_timer))
		printk("auxiliary floppy timer still active\n");
	if (work_pending(&floppy_work))
		printk("work still pending\n");
#endif
	old_fdc = fdc;
	for (fdc = 0; fdc < N_FDC; fdc++)
		if (FDCS->address != -1)
			floppy_release_regions(fdc);
	fdc = old_fdc;
}

#ifdef MODULE

static char *floppy;

static void __init parse_floppy_cfg_string(char *cfg)
{
	char *ptr;

	while (*cfg) {
		for (ptr = cfg; *cfg && *cfg != ' ' && *cfg != '\t'; cfg++) ;
		if (*cfg) {
			*cfg = '\0';
			cfg++;
		}
		if (*ptr)
			floppy_setup(ptr);
	}
}

static int __init floppy_module_init(void)
{
	if (floppy)
		parse_floppy_cfg_string(floppy);
	return floppy_init();
}
module_init(floppy_module_init);

static void __exit floppy_module_exit(void)
{
	int drive;

	blk_unregister_region(MKDEV(FLOPPY_MAJOR, 0), 256);
	unregister_blkdev(FLOPPY_MAJOR, "fd");
	platform_driver_unregister(&floppy_driver);

	for (drive = 0; drive < N_DRIVE; drive++) {
		del_timer_sync(&motor_off_timer[drive]);

		if ((allowed_drive_mask & (1 << drive)) &&
		    fdc_state[FDC(drive)].version != FDC_NONE) {
			del_gendisk(disks[drive]);
			device_remove_file(&floppy_device[drive].dev, &dev_attr_cmos);
			platform_device_unregister(&floppy_device[drive]);
		}
		put_disk(disks[drive]);
	}

	del_timer_sync(&fd_timeout);
	del_timer_sync(&fd_timer);
	blk_cleanup_queue(floppy_queue);

	if (usage_count)
		floppy_release_irq_and_dma();

	
	fd_eject(0);
}
module_exit(floppy_module_exit);

module_param(floppy, charp, 0);
module_param(FLOPPY_IRQ, int, 0);
module_param(FLOPPY_DMA, int, 0);
MODULE_AUTHOR("Alain L. Knaff");
MODULE_SUPPORTED_DEVICE("fd");
MODULE_LICENSE("GPL");


static const struct pnp_device_id floppy_pnpids[] = {
	{ "PNP0700", 0 },
	{ }
};
MODULE_DEVICE_TABLE(pnp, floppy_pnpids);

#else

__setup("floppy=", floppy_setup);
module_init(floppy_init)
#endif

MODULE_ALIAS_BLOCKDEV_MAJOR(FLOPPY_MAJOR);
