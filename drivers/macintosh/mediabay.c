
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/stddef.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <asm/prom.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/pmac_feature.h>
#include <asm/mediabay.h>
#include <asm/sections.h>
#include <asm/ohare.h>
#include <asm/heathrow.h>
#include <asm/keylargo.h>
#include <linux/adb.h>
#include <linux/pmu.h>


#define MB_DEBUG

#ifdef MB_DEBUG
#define MBDBG(fmt, arg...)	printk(KERN_INFO fmt , ## arg)
#else
#define MBDBG(fmt, arg...)	do { } while (0)
#endif

#define MB_FCR32(bay, r)	((bay)->base + ((r) >> 2))
#define MB_FCR8(bay, r)		(((volatile u8 __iomem *)((bay)->base)) + (r))

#define MB_IN32(bay,r)		(in_le32(MB_FCR32(bay,r)))
#define MB_OUT32(bay,r,v)	(out_le32(MB_FCR32(bay,r), (v)))
#define MB_BIS(bay,r,v)		(MB_OUT32((bay), (r), MB_IN32((bay), r) | (v)))
#define MB_BIC(bay,r,v)		(MB_OUT32((bay), (r), MB_IN32((bay), r) & ~(v)))
#define MB_IN8(bay,r)		(in_8(MB_FCR8(bay,r)))
#define MB_OUT8(bay,r,v)	(out_8(MB_FCR8(bay,r), (v)))

struct media_bay_info;

struct mb_ops {
	char*	name;
	void	(*init)(struct media_bay_info *bay);
	u8	(*content)(struct media_bay_info *bay);
	void	(*power)(struct media_bay_info *bay, int on_off);
	int	(*setup_bus)(struct media_bay_info *bay, u8 device_id);
	void	(*un_reset)(struct media_bay_info *bay);
	void	(*un_reset_ide)(struct media_bay_info *bay);
};

struct media_bay_info {
	u32 __iomem			*base;
	int				content_id;
	int				state;
	int				last_value;
	int				value_count;
	int				timer;
	struct macio_dev		*mdev;
	struct mb_ops*			ops;
	int				index;
	int				cached_gpio;
	int				sleeping;
	struct mutex			lock;
#ifdef CONFIG_BLK_DEV_IDE_PMAC
	ide_hwif_t			*cd_port;
	void __iomem			*cd_base;
	int				cd_irq;
	int				cd_retry;
#endif
#if defined(CONFIG_BLK_DEV_IDE_PMAC)
	int 				cd_index;
#endif
};

#define MAX_BAYS	2

static struct media_bay_info media_bays[MAX_BAYS];
int media_bay_count = 0;

#ifdef CONFIG_BLK_DEV_IDE_PMAC

#define MB_IDE_READY(i)	((readb(media_bays[i].cd_base + 0x70) & 0x80) == 0)
#endif


#define MB_POLL_DELAY	25


#define MB_STABLE_DELAY	100


#define MB_POWER_DELAY	200


#define MB_RESET_DELAY	50


#define MB_SETUP_DELAY	100


#define MB_IDE_WAIT	1000


#define MB_IDE_TIMEOUT	5000


#define MAX_CD_RETRIES	3


enum {
	mb_empty = 0,		
	mb_powering_up,		
	mb_enabling_bay,	
	mb_resetting,		
	mb_ide_resetting,	
	mb_ide_waiting,		
	mb_up,			
	mb_powering_down	
};

#define MB_POWER_SOUND		0x08
#define MB_POWER_FLOPPY		0x04
#define MB_POWER_ATA		0x02
#define MB_POWER_PCI		0x01
#define MB_POWER_OFF		0x00


 
static u8
ohare_mb_content(struct media_bay_info *bay)
{
	return (MB_IN32(bay, OHARE_MBCR) >> 12) & 7;
}

static u8
heathrow_mb_content(struct media_bay_info *bay)
{
	return (MB_IN32(bay, HEATHROW_MBCR) >> 12) & 7;
}

static u8
keylargo_mb_content(struct media_bay_info *bay)
{
	int new_gpio;

	new_gpio = MB_IN8(bay, KL_GPIO_MEDIABAY_IRQ) & KEYLARGO_GPIO_INPUT_DATA;
	if (new_gpio) {
		bay->cached_gpio = new_gpio;
		return MB_NO;
	} else if (bay->cached_gpio != new_gpio) {
		MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_ENABLE);
		(void)MB_IN32(bay, KEYLARGO_MBCR);
		udelay(5);
		MB_BIC(bay, KEYLARGO_MBCR, 0x0000000F);
		(void)MB_IN32(bay, KEYLARGO_MBCR);
		udelay(5);
		bay->cached_gpio = new_gpio;
	}
	return (MB_IN32(bay, KEYLARGO_MBCR) >> 4) & 7;
}



static void
ohare_mb_power(struct media_bay_info* bay, int on_off)
{
	if (on_off) {
		
		MB_BIC(bay, OHARE_FCR, OH_BAY_RESET_N);
		MB_BIC(bay, OHARE_FCR, OH_BAY_POWER_N);
	} else {
		
		MB_BIC(bay, OHARE_FCR, OH_BAY_DEV_MASK);
		MB_BIC(bay, OHARE_FCR, OH_FLOPPY_ENABLE);
		
		MB_BIS(bay, OHARE_FCR, OH_BAY_POWER_N);
		MB_BIS(bay, OHARE_FCR, OH_BAY_RESET_N);
		MB_BIS(bay, OHARE_FCR, OH_IDE1_RESET_N);
	}
	MB_BIC(bay, OHARE_MBCR, 0x00000F00);
}

static void
heathrow_mb_power(struct media_bay_info* bay, int on_off)
{
	if (on_off) {
		
		MB_BIC(bay, HEATHROW_FCR, HRW_BAY_RESET_N);
		MB_BIC(bay, HEATHROW_FCR, HRW_BAY_POWER_N);
	} else {
		
		MB_BIC(bay, HEATHROW_FCR, HRW_BAY_DEV_MASK);
		MB_BIC(bay, HEATHROW_FCR, HRW_SWIM_ENABLE);
		
		MB_BIS(bay, HEATHROW_FCR, HRW_BAY_POWER_N);
		MB_BIS(bay, HEATHROW_FCR, HRW_BAY_RESET_N);
		MB_BIS(bay, HEATHROW_FCR, HRW_IDE1_RESET_N);
	}
	MB_BIC(bay, HEATHROW_MBCR, 0x00000F00);
}

static void
keylargo_mb_power(struct media_bay_info* bay, int on_off)
{
	if (on_off) {
		
            	MB_BIC(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_RESET);
            	MB_BIC(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_POWER);
	} else {
		
		MB_BIC(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_MASK);
		MB_BIC(bay, KEYLARGO_FCR1, KL1_EIDE0_ENABLE);
		
		MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_POWER);
		MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_RESET);
		MB_BIS(bay, KEYLARGO_FCR1, KL1_EIDE0_RESET_N);
	}
	MB_BIC(bay, KEYLARGO_MBCR, 0x0000000F);
}



static int
ohare_mb_setup_bus(struct media_bay_info* bay, u8 device_id)
{
	switch(device_id) {
		case MB_FD:
		case MB_FD1:
			MB_BIS(bay, OHARE_FCR, OH_BAY_FLOPPY_ENABLE);
			MB_BIS(bay, OHARE_FCR, OH_FLOPPY_ENABLE);
			return 0;
		case MB_CD:
			MB_BIC(bay, OHARE_FCR, OH_IDE1_RESET_N);
			MB_BIS(bay, OHARE_FCR, OH_BAY_IDE_ENABLE);
			return 0;
		case MB_PCI:
			MB_BIS(bay, OHARE_FCR, OH_BAY_PCI_ENABLE);
			return 0;
	}
	return -ENODEV;
}

static int
heathrow_mb_setup_bus(struct media_bay_info* bay, u8 device_id)
{
	switch(device_id) {
		case MB_FD:
		case MB_FD1:
			MB_BIS(bay, HEATHROW_FCR, HRW_BAY_FLOPPY_ENABLE);
			MB_BIS(bay, HEATHROW_FCR, HRW_SWIM_ENABLE);
			return 0;
		case MB_CD:
			MB_BIC(bay, HEATHROW_FCR, HRW_IDE1_RESET_N);
			MB_BIS(bay, HEATHROW_FCR, HRW_BAY_IDE_ENABLE);
			return 0;
		case MB_PCI:
			MB_BIS(bay, HEATHROW_FCR, HRW_BAY_PCI_ENABLE);
			return 0;
	}
	return -ENODEV;
}

static int
keylargo_mb_setup_bus(struct media_bay_info* bay, u8 device_id)
{
	switch(device_id) {
		case MB_CD:
			MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_IDE_ENABLE);
			MB_BIC(bay, KEYLARGO_FCR1, KL1_EIDE0_RESET_N);
			MB_BIS(bay, KEYLARGO_FCR1, KL1_EIDE0_ENABLE);
			return 0;
		case MB_PCI:
			MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_PCI_ENABLE);
			return 0;
		case MB_SOUND:
			MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_SOUND_ENABLE);
			return 0;
	}
	return -ENODEV;
}



static void
ohare_mb_un_reset(struct media_bay_info* bay)
{
	MB_BIS(bay, OHARE_FCR, OH_BAY_RESET_N);
}

static void keylargo_mb_init(struct media_bay_info *bay)
{
	MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_ENABLE);
}

static void heathrow_mb_un_reset(struct media_bay_info* bay)
{
	MB_BIS(bay, HEATHROW_FCR, HRW_BAY_RESET_N);
}

static void keylargo_mb_un_reset(struct media_bay_info* bay)
{
	MB_BIS(bay, KEYLARGO_MBCR, KL_MBCR_MB0_DEV_RESET);
}

static void ohare_mb_un_reset_ide(struct media_bay_info* bay)
{
	MB_BIS(bay, OHARE_FCR, OH_IDE1_RESET_N);
}

static void heathrow_mb_un_reset_ide(struct media_bay_info* bay)
{
	MB_BIS(bay, HEATHROW_FCR, HRW_IDE1_RESET_N);
}

static void keylargo_mb_un_reset_ide(struct media_bay_info* bay)
{
	MB_BIS(bay, KEYLARGO_FCR1, KL1_EIDE0_RESET_N);
}

static inline void set_mb_power(struct media_bay_info* bay, int onoff)
{
	
	if (onoff) {
		bay->ops->power(bay, 1);
		bay->state = mb_powering_up;
		MBDBG("mediabay%d: powering up\n", bay->index);
	} else { 
		
		bay->ops->power(bay, 0);
		bay->state = mb_powering_down;
		MBDBG("mediabay%d: powering down\n", bay->index);
	}
	bay->timer = msecs_to_jiffies(MB_POWER_DELAY);
}

static void poll_media_bay(struct media_bay_info* bay)
{
	int id = bay->ops->content(bay);

	if (id == bay->last_value) {
		if (id != bay->content_id) {
			bay->value_count += msecs_to_jiffies(MB_POLL_DELAY);
			if (bay->value_count >= msecs_to_jiffies(MB_STABLE_DELAY)) {
				
				if ((id != MB_NO) && (bay->content_id != MB_NO)) {
					id = MB_NO;
					MBDBG("mediabay%d: forcing MB_NO\n", bay->index);
				}
				MBDBG("mediabay%d: switching to %d\n", bay->index, id);
				set_mb_power(bay, id != MB_NO);
				bay->content_id = id;
				if (id == MB_NO) {
#ifdef CONFIG_BLK_DEV_IDE_PMAC
					bay->cd_retry = 0;
#endif
					printk(KERN_INFO "media bay %d is empty\n", bay->index);
				}
			}
		}
	} else {
		bay->last_value = id;
		bay->value_count = 0;
	}
}

#ifdef CONFIG_BLK_DEV_IDE_PMAC
int check_media_bay(struct device_node *which_bay, int what)
{
	int	i;

	for (i=0; i<media_bay_count; i++)
		if (media_bays[i].mdev && which_bay == media_bays[i].mdev->ofdev.node) {
			if ((what == media_bays[i].content_id) && media_bays[i].state == mb_up)
				return 0;
			media_bays[i].cd_index = -1;
			return -EINVAL;
		}
	return -ENODEV;
}
EXPORT_SYMBOL(check_media_bay);

int check_media_bay_by_base(unsigned long base, int what)
{
	int	i;

	for (i=0; i<media_bay_count; i++)
		if (media_bays[i].mdev && base == (unsigned long) media_bays[i].cd_base) {
			if ((what == media_bays[i].content_id) && media_bays[i].state == mb_up)
				return 0;
			media_bays[i].cd_index = -1;
			return -EINVAL;
		} 

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(check_media_bay_by_base);

int media_bay_set_ide_infos(struct device_node* which_bay, unsigned long base,
			    int irq, ide_hwif_t *hwif)
{
	int	i;

	for (i=0; i<media_bay_count; i++) {
		struct media_bay_info* bay = &media_bays[i];

		if (bay->mdev && which_bay == bay->mdev->ofdev.node) {
			int timeout = 5000, index = hwif->index;
			
			mutex_lock(&bay->lock);

			bay->cd_port	= hwif;
 			bay->cd_base	= (void __iomem *) base;
			bay->cd_irq	= irq;

			if ((MB_CD != bay->content_id) || bay->state != mb_up) {
				mutex_unlock(&bay->lock);
				return 0;
			}
			printk(KERN_DEBUG "Registered ide%d for media bay %d\n", index, i);
			do {
				if (MB_IDE_READY(i)) {
					bay->cd_index	= index;
					mutex_unlock(&bay->lock);
					return 0;
				}
				mdelay(1);
			} while(--timeout);
			printk(KERN_DEBUG "Timeount waiting IDE in bay %d\n", i);
			mutex_unlock(&bay->lock);
			return -ENODEV;
		}
	}

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(media_bay_set_ide_infos);
#endif 

static void media_bay_step(int i)
{
	struct media_bay_info* bay = &media_bays[i];

	
	if (bay->state != mb_powering_down)
	    poll_media_bay(bay);

	
	if ((bay->state != mb_ide_waiting) && (bay->timer != 0)) {
		bay->timer -= msecs_to_jiffies(MB_POLL_DELAY);
		if (bay->timer > 0)
			return;
		bay->timer = 0;
	}

	switch(bay->state) {
	case mb_powering_up:
	    	if (bay->ops->setup_bus(bay, bay->last_value) < 0) {
			MBDBG("mediabay%d: device not supported (kind:%d)\n", i, bay->content_id);
	    		set_mb_power(bay, 0);
	    		break;
	    	}
	    	bay->timer = msecs_to_jiffies(MB_RESET_DELAY);
	    	bay->state = mb_enabling_bay;
		MBDBG("mediabay%d: enabling (kind:%d)\n", i, bay->content_id);
		break;
	case mb_enabling_bay:
		bay->ops->un_reset(bay);
	    	bay->timer = msecs_to_jiffies(MB_SETUP_DELAY);
	    	bay->state = mb_resetting;
		MBDBG("mediabay%d: waiting reset (kind:%d)\n", i, bay->content_id);
	    	break;
	case mb_resetting:
		if (bay->content_id != MB_CD) {
			MBDBG("mediabay%d: bay is up (kind:%d)\n", i, bay->content_id);
			bay->state = mb_up;
			break;
	    	}
#ifdef CONFIG_BLK_DEV_IDE_PMAC
		MBDBG("mediabay%d: waiting IDE reset (kind:%d)\n", i, bay->content_id);
		bay->ops->un_reset_ide(bay);
	    	bay->timer = msecs_to_jiffies(MB_IDE_WAIT);
	    	bay->state = mb_ide_resetting;
#else
		printk(KERN_DEBUG "media-bay %d is ide (not compiled in kernel)\n", i);
		set_mb_power(bay, 0);
#endif 
	    	break;
#ifdef CONFIG_BLK_DEV_IDE_PMAC
	case mb_ide_resetting:
	    	bay->timer = msecs_to_jiffies(MB_IDE_TIMEOUT);
	    	bay->state = mb_ide_waiting;
		MBDBG("mediabay%d: waiting IDE ready (kind:%d)\n", i, bay->content_id);
	    	break;
	case mb_ide_waiting:
		if (bay->cd_base == NULL) {
			bay->timer = 0;
			bay->state = mb_up;
			MBDBG("mediabay%d: up before IDE init\n", i);
			break;
		} else if (MB_IDE_READY(i)) {
			bay->timer = 0;
			bay->state = mb_up;
			if (bay->cd_index < 0) {
				printk("mediabay %d, registering IDE...\n", i);
				pmu_suspend();
				ide_port_scan(bay->cd_port);
				if (bay->cd_port->present)
					bay->cd_index = bay->cd_port->index;
				pmu_resume();
			}
			if (bay->cd_index == -1) {
				
				bay->cd_retry++;
				printk("IDE register error\n");
				set_mb_power(bay, 0);
			} else {
				printk(KERN_DEBUG "media-bay %d is ide%d\n", i, bay->cd_index);
				MBDBG("mediabay %d IDE ready\n", i);
			}
			break;
	    	} else if (bay->timer > 0)
			bay->timer -= msecs_to_jiffies(MB_POLL_DELAY);
	    	if (bay->timer <= 0) {
			printk("\nIDE Timeout in bay %d !, IDE state is: 0x%02x\n",
			       i, readb(bay->cd_base + 0x70));
			MBDBG("mediabay%d: nIDE Timeout !\n", i);
			set_mb_power(bay, 0);
			bay->timer = 0;
	    	}
		break;
#endif 
	case mb_powering_down:
	    	bay->state = mb_empty;
#ifdef CONFIG_BLK_DEV_IDE_PMAC
    	        if (bay->cd_index >= 0) {
			printk(KERN_DEBUG "Unregistering mb %d ide, index:%d\n", i,
			       bay->cd_index);
			ide_port_unregister_devices(bay->cd_port);
			bay->cd_index = -1;
		}
	    	if (bay->cd_retry) {
			if (bay->cd_retry > MAX_CD_RETRIES) {
				
				printk("\nmedia-bay %d, IDE device badly inserted or unrecognised\n", i);
			} else {
				
				bay->content_id = MB_NO;
			}
	    	}
#endif 
		MBDBG("mediabay%d: end of power down\n", i);
	    	break;
	}
}


static int media_bay_task(void *x)
{
	int	i;

	while (!kthread_should_stop()) {
		for (i = 0; i < media_bay_count; ++i) {
			mutex_lock(&media_bays[i].lock);
			if (!media_bays[i].sleeping)
				media_bay_step(i);
			mutex_unlock(&media_bays[i].lock);
		}

		msleep_interruptible(MB_POLL_DELAY);
	}
	return 0;
}

static int __devinit media_bay_attach(struct macio_dev *mdev, const struct of_device_id *match)
{
	struct media_bay_info* bay;
	u32 __iomem *regbase;
	struct device_node *ofnode;
	unsigned long base;
	int i;

	ofnode = mdev->ofdev.node;

	if (macio_resource_count(mdev) < 1)
		return -ENODEV;
	if (macio_request_resources(mdev, "media-bay"))
		return -EBUSY;
	
	base = macio_resource_start(mdev, 0) & 0xffff0000u;
	regbase = (u32 __iomem *)ioremap(base, 0x100);
	if (regbase == NULL) {
		macio_release_resources(mdev);
		return -ENOMEM;
	}
	
	i = media_bay_count++;
	bay = &media_bays[i];
	bay->mdev = mdev;
	bay->base = regbase;
	bay->index = i;
	bay->ops = match->data;
	bay->sleeping = 0;
	mutex_init(&bay->lock);

	
	if (bay->ops->init)
		bay->ops->init(bay);

	printk(KERN_INFO "mediabay%d: Registered %s media-bay\n", i, bay->ops->name);

	
	set_mb_power(bay, 0);
	msleep(MB_POWER_DELAY);
	bay->content_id = MB_NO;
	bay->last_value = bay->ops->content(bay);
	bay->value_count = msecs_to_jiffies(MB_STABLE_DELAY);
	bay->state = mb_empty;
	do {
		msleep(MB_POLL_DELAY);
		media_bay_step(i);
	} while((bay->state != mb_empty) &&
		(bay->state != mb_up));

	
	macio_set_drvdata(mdev, bay);

	
	if (i == 0)
		kthread_run(media_bay_task, NULL, "media-bay");

	return 0;

}

static int media_bay_suspend(struct macio_dev *mdev, pm_message_t state)
{
	struct media_bay_info	*bay = macio_get_drvdata(mdev);

	if (state.event != mdev->ofdev.dev.power.power_state.event
	    && (state.event & PM_EVENT_SLEEP)) {
		mutex_lock(&bay->lock);
		bay->sleeping = 1;
		set_mb_power(bay, 0);
		mutex_unlock(&bay->lock);
		msleep(MB_POLL_DELAY);
		mdev->ofdev.dev.power.power_state = state;
	}
	return 0;
}

static int media_bay_resume(struct macio_dev *mdev)
{
	struct media_bay_info	*bay = macio_get_drvdata(mdev);

	if (mdev->ofdev.dev.power.power_state.event != PM_EVENT_ON) {
		mdev->ofdev.dev.power.power_state = PMSG_ON;

	       	
	       	
		mutex_lock(&bay->lock);
	       	set_mb_power(bay, 0);
		msleep(MB_POWER_DELAY);
	       	if (bay->ops->content(bay) != bay->content_id) {
			printk("mediabay%d: content changed during sleep...\n", bay->index);
			mutex_unlock(&bay->lock);
	       		return 0;
		}
	       	set_mb_power(bay, 1);
	       	bay->last_value = bay->content_id;
	       	bay->value_count = msecs_to_jiffies(MB_STABLE_DELAY);
	       	bay->timer = msecs_to_jiffies(MB_POWER_DELAY);
#ifdef CONFIG_BLK_DEV_IDE_PMAC
	       	bay->cd_retry = 0;
#endif
	       	do {
			msleep(MB_POLL_DELAY);
	       		media_bay_step(bay->index);
	       	} while((bay->state != mb_empty) &&
	       		(bay->state != mb_up));
		bay->sleeping = 0;
		mutex_unlock(&bay->lock);
	}
	return 0;
}



static struct mb_ops ohare_mb_ops = {
	.name		= "Ohare",
	.content	= ohare_mb_content,
	.power		= ohare_mb_power,
	.setup_bus	= ohare_mb_setup_bus,
	.un_reset	= ohare_mb_un_reset,
	.un_reset_ide	= ohare_mb_un_reset_ide,
};

static struct mb_ops heathrow_mb_ops = {
	.name		= "Heathrow",
	.content	= heathrow_mb_content,
	.power		= heathrow_mb_power,
	.setup_bus	= heathrow_mb_setup_bus,
	.un_reset	= heathrow_mb_un_reset,
	.un_reset_ide	= heathrow_mb_un_reset_ide,
};

static struct mb_ops keylargo_mb_ops = {
	.name		= "KeyLargo",
	.init		= keylargo_mb_init,
	.content	= keylargo_mb_content,
	.power		= keylargo_mb_power,
	.setup_bus	= keylargo_mb_setup_bus,
	.un_reset	= keylargo_mb_un_reset,
	.un_reset_ide	= keylargo_mb_un_reset_ide,
};



static struct of_device_id media_bay_match[] =
{
	{
	.name		= "media-bay",
	.compatible	= "keylargo-media-bay",
	.data		= &keylargo_mb_ops,
	},
	{
	.name		= "media-bay",
	.compatible	= "heathrow-media-bay",
	.data		= &heathrow_mb_ops,
	},
	{
	.name		= "media-bay",
	.compatible	= "ohare-media-bay",
	.data		= &ohare_mb_ops,
	},
	{},
};

static struct macio_driver media_bay_driver =
{
	.name		= "media-bay",
	.match_table	= media_bay_match,
	.probe		= media_bay_attach,
	.suspend	= media_bay_suspend,
	.resume		= media_bay_resume
};

static int __init media_bay_init(void)
{
	int i;

	for (i=0; i<MAX_BAYS; i++) {
		memset((char *)&media_bays[i], 0, sizeof(struct media_bay_info));
		media_bays[i].content_id	= -1;
#ifdef CONFIG_BLK_DEV_IDE_PMAC
		media_bays[i].cd_index		= -1;
#endif
	}
	if (!machine_is(powermac))
		return 0;

	macio_register_driver(&media_bay_driver);	

	return 0;
}

device_initcall(media_bay_init);
