
#ifndef LINUX_MMC_HOST_H
#define LINUX_MMC_HOST_H

#include <linux/leds.h>
#include <linux/sched.h>

#include <linux/mmc/core.h>

struct mmc_ios {
	unsigned int	clock;			
	unsigned short	vdd;



	unsigned char	bus_mode;		

#define MMC_BUSMODE_OPENDRAIN	1
#define MMC_BUSMODE_PUSHPULL	2

	unsigned char	chip_select;		

#define MMC_CS_DONTCARE		0
#define MMC_CS_HIGH		1
#define MMC_CS_LOW		2

	unsigned char	power_mode;		

#define MMC_POWER_OFF		0
#define MMC_POWER_UP		1
#define MMC_POWER_ON		2

	unsigned char	bus_width;		

#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3

	unsigned char	timing;			

#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2
};

struct mmc_host_ops {
	
	int (*enable)(struct mmc_host *host);
	int (*disable)(struct mmc_host *host, int lazy);
	void	(*request)(struct mmc_host *host, struct mmc_request *req);
	
	void	(*set_ios)(struct mmc_host *host, struct mmc_ios *ios);
	int	(*get_ro)(struct mmc_host *host);
	int	(*get_cd)(struct mmc_host *host);

	void	(*enable_sdio_irq)(struct mmc_host *host, int enable);

	int (*get_status)(struct mmc_host *host);
};

struct mmc_card;
struct device;

struct mmc_host {
	struct device		*parent;
	struct device		class_dev;
	int			index;
	const struct mmc_host_ops *ops;
	unsigned int		f_min;
	unsigned int		f_max;
	u32			ocr_avail;
	struct notifier_block	pm_notify;

#define MMC_VDD_165_195		0x00000080	
#define MMC_VDD_20_21		0x00000100	
#define MMC_VDD_21_22		0x00000200	
#define MMC_VDD_22_23		0x00000400	
#define MMC_VDD_23_24		0x00000800	
#define MMC_VDD_24_25		0x00001000	
#define MMC_VDD_25_26		0x00002000	
#define MMC_VDD_26_27		0x00004000	
#define MMC_VDD_27_28		0x00008000	
#define MMC_VDD_28_29		0x00010000	
#define MMC_VDD_29_30		0x00020000	
#define MMC_VDD_30_31		0x00040000	
#define MMC_VDD_31_32		0x00080000	
#define MMC_VDD_32_33		0x00100000	
#define MMC_VDD_33_34		0x00200000	
#define MMC_VDD_34_35		0x00400000	
#define MMC_VDD_35_36		0x00800000	

	unsigned long		caps;		

#define MMC_CAP_4_BIT_DATA	(1 << 0)	
#define MMC_CAP_MMC_HIGHSPEED	(1 << 1)	
#define MMC_CAP_SD_HIGHSPEED	(1 << 2)	
#define MMC_CAP_SDIO_IRQ	(1 << 3)	
#define MMC_CAP_SPI		(1 << 4)	
#define MMC_CAP_NEEDS_POLL	(1 << 5)	
#define MMC_CAP_8_BIT_DATA	(1 << 6)	
#define MMC_CAP_DISABLE		(1 << 7)	
#define MMC_CAP_NONREMOVABLE	(1 << 8)	
#define MMC_CAP_WAIT_WHILE_BUSY	(1 << 9)	

	
	unsigned int		max_seg_size;	
	unsigned short		max_hw_segs;	
	unsigned short		max_phys_segs;	
	unsigned short		unused;
	unsigned int		max_req_size;	
	unsigned int		max_blk_size;	
	unsigned int		max_blk_count;	

	
	spinlock_t		lock;		

	struct mmc_ios		ios;		
	u32			ocr;		

	
	unsigned int		use_spi_crc:1;
	unsigned int		claimed:1;	
	unsigned int		bus_dead:1;	
#ifdef CONFIG_MMC_DEBUG
	unsigned int		removed:1;	
#endif

	
	int			enabled;	
	int			nesting_cnt;	
	int			en_dis_recurs;	
	unsigned int		disable_delay;	
	struct delayed_work	disable;	

	struct mmc_card		*card;		

	wait_queue_head_t	wq;
	struct task_struct	*claimer;	
	int			claim_cnt;	

	struct delayed_work	detect;

	const struct mmc_bus_ops *bus_ops;	
	unsigned int		bus_refs;	

	unsigned int		bus_resume_flags;
#define MMC_BUSRESUME_MANUAL_RESUME	(1 << 0)
#define MMC_BUSRESUME_NEEDS_RESUME	(1 << 1)

	unsigned int		sdio_irqs;
	struct task_struct	*sdio_irq_thread;
	atomic_t		sdio_irq_thread_abort;

#ifdef CONFIG_LEDS_TRIGGERS
	struct led_trigger	*led;		
#endif

	struct dentry		*debugfs_root;

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	struct {
		struct sdio_cis			*cis;
		struct sdio_cccr		*cccr;
		struct sdio_embedded_func	*funcs;
		int				num_funcs;
	} embedded_sdio_data;
#endif

#ifdef CONFIG_MMC_PERF_PROFILING
	struct {

		unsigned long rbytes_mmcq; 
		unsigned long wbytes_mmcq; 
		unsigned long rbytes_drv;  
		unsigned long wbytes_drv;  
		ktime_t rtime_mmcq;	   
		ktime_t wtime_mmcq;	   
		ktime_t rtime_drv;	   
		ktime_t wtime_drv;	   
		ktime_t start;
	} perf;
#endif
	unsigned long		private[0] ____cacheline_aligned;
};

extern struct mmc_host *mmc_alloc_host(int extra, struct device *);
extern int mmc_add_host(struct mmc_host *);
extern void mmc_remove_host(struct mmc_host *);
extern void mmc_free_host(struct mmc_host *);

#ifdef CONFIG_MMC_EMBEDDED_SDIO
extern void mmc_set_embedded_sdio_data(struct mmc_host *host,
				       struct sdio_cis *cis,
				       struct sdio_cccr *cccr,
				       struct sdio_embedded_func *funcs,
				       int num_funcs);
#endif

static inline void *mmc_priv(struct mmc_host *host)
{
	return (void *)host->private;
}

#define mmc_host_is_spi(host)	((host)->caps & MMC_CAP_SPI)

#define mmc_dev(x)	((x)->parent)
#define mmc_classdev(x)	(&(x)->class_dev)
#define mmc_hostname(x)	(dev_name(&(x)->class_dev))
#define mmc_bus_needs_resume(host) ((host)->bus_resume_flags & MMC_BUSRESUME_NEEDS_RESUME)

static inline void mmc_set_bus_resume_policy(struct mmc_host *host, int manual)
{
	if (manual)
		host->bus_resume_flags |= MMC_BUSRESUME_MANUAL_RESUME;
	else
		host->bus_resume_flags &= ~MMC_BUSRESUME_MANUAL_RESUME;
}

extern int mmc_resume_bus(struct mmc_host *host);

extern int mmc_suspend_host(struct mmc_host *, pm_message_t);
extern int mmc_resume_host(struct mmc_host *);

extern void mmc_power_save_host(struct mmc_host *host);
extern void mmc_power_restore_host(struct mmc_host *host);

extern void mmc_detect_change(struct mmc_host *, unsigned long delay);
extern void mmc_request_done(struct mmc_host *, struct mmc_request *);

static inline void mmc_signal_sdio_irq(struct mmc_host *host)
{
	host->ops->enable_sdio_irq(host, 0);
	wake_up_process(host->sdio_irq_thread);
}

struct regulator;

int mmc_regulator_get_ocrmask(struct regulator *supply);
int mmc_regulator_set_ocr(struct regulator *supply, unsigned short vdd_bit);

int mmc_card_awake(struct mmc_host *host);
int mmc_card_sleep(struct mmc_host *host);
int mmc_card_can_sleep(struct mmc_host *host);


extern int mmc_direct_power_off(struct mmc_host *);
extern int mmc_direct_power_up(struct mmc_host *);

int mmc_host_enable(struct mmc_host *host);
int mmc_host_disable(struct mmc_host *host);
int mmc_host_lazy_disable(struct mmc_host *host);
#ifdef CONFIG_PM
int mmc_pm_notify(struct notifier_block *notify_block, unsigned long mode,
		  void *unused);
#else
#define mmc_pm_notify NULL
#endif

static inline void mmc_set_disable_delay(struct mmc_host *host,
					 unsigned int disable_delay)
{
	host->disable_delay = disable_delay;
}

#endif

