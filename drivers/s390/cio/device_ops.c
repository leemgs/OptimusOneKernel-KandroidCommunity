
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/delay.h>

#include <asm/ccwdev.h>
#include <asm/idals.h>
#include <asm/chpid.h>
#include <asm/fcx.h>

#include "cio.h"
#include "cio_debug.h"
#include "css.h"
#include "chsc.h"
#include "device.h"
#include "chp.h"


int ccw_device_set_options_mask(struct ccw_device *cdev, unsigned long flags)
{
       
	if ((flags & CCWDEV_EARLY_NOTIFICATION) &&
	    (flags & CCWDEV_REPORT_ALL))
		return -EINVAL;
	cdev->private->options.fast = (flags & CCWDEV_EARLY_NOTIFICATION) != 0;
	cdev->private->options.repall = (flags & CCWDEV_REPORT_ALL) != 0;
	cdev->private->options.pgroup = (flags & CCWDEV_DO_PATHGROUP) != 0;
	cdev->private->options.force = (flags & CCWDEV_ALLOW_FORCE) != 0;
	return 0;
}


int ccw_device_set_options(struct ccw_device *cdev, unsigned long flags)
{
       
	if (((flags & CCWDEV_EARLY_NOTIFICATION) &&
	    (flags & CCWDEV_REPORT_ALL)) ||
	    ((flags & CCWDEV_EARLY_NOTIFICATION) &&
	     cdev->private->options.repall) ||
	    ((flags & CCWDEV_REPORT_ALL) &&
	     cdev->private->options.fast))
		return -EINVAL;
	cdev->private->options.fast |= (flags & CCWDEV_EARLY_NOTIFICATION) != 0;
	cdev->private->options.repall |= (flags & CCWDEV_REPORT_ALL) != 0;
	cdev->private->options.pgroup |= (flags & CCWDEV_DO_PATHGROUP) != 0;
	cdev->private->options.force |= (flags & CCWDEV_ALLOW_FORCE) != 0;
	return 0;
}


void ccw_device_clear_options(struct ccw_device *cdev, unsigned long flags)
{
	cdev->private->options.fast &= (flags & CCWDEV_EARLY_NOTIFICATION) == 0;
	cdev->private->options.repall &= (flags & CCWDEV_REPORT_ALL) == 0;
	cdev->private->options.pgroup &= (flags & CCWDEV_DO_PATHGROUP) == 0;
	cdev->private->options.force &= (flags & CCWDEV_ALLOW_FORCE) == 0;
}


int ccw_device_clear(struct ccw_device *cdev, unsigned long intparm)
{
	struct subchannel *sch;
	int ret;

	if (!cdev || !cdev->dev.parent)
		return -ENODEV;
	sch = to_subchannel(cdev->dev.parent);
	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state == DEV_STATE_NOT_OPER)
		return -ENODEV;
	if (cdev->private->state != DEV_STATE_ONLINE &&
	    cdev->private->state != DEV_STATE_W4SENSE)
		return -EINVAL;

	ret = cio_clear(sch);
	if (ret == 0)
		cdev->private->intparm = intparm;
	return ret;
}


int ccw_device_start_key(struct ccw_device *cdev, struct ccw1 *cpa,
			 unsigned long intparm, __u8 lpm, __u8 key,
			 unsigned long flags)
{
	struct subchannel *sch;
	int ret;

	if (!cdev || !cdev->dev.parent)
		return -ENODEV;
	sch = to_subchannel(cdev->dev.parent);
	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state == DEV_STATE_NOT_OPER)
		return -ENODEV;
	if (cdev->private->state == DEV_STATE_VERIFY ||
	    cdev->private->state == DEV_STATE_CLEAR_VERIFY) {
		
		if (!cdev->private->flags.fake_irb) {
			cdev->private->flags.fake_irb = 1;
			cdev->private->intparm = intparm;
			return 0;
		} else
			
			return -EBUSY;
	}
	if (cdev->private->state != DEV_STATE_ONLINE ||
	    ((sch->schib.scsw.cmd.stctl & SCSW_STCTL_PRIM_STATUS) &&
	     !(sch->schib.scsw.cmd.stctl & SCSW_STCTL_SEC_STATUS)) ||
	    cdev->private->flags.doverify)
		return -EBUSY;
	ret = cio_set_options (sch, flags);
	if (ret)
		return ret;
	
	if (lpm) {
		lpm &= sch->opm;
		if (lpm == 0)
			return -EACCES;
	}
	ret = cio_start_key (sch, cpa, lpm, key);
	switch (ret) {
	case 0:
		cdev->private->intparm = intparm;
		break;
	case -EACCES:
	case -ENODEV:
		dev_fsm_event(cdev, DEV_EVENT_VERIFY);
		break;
	}
	return ret;
}


int ccw_device_start_timeout_key(struct ccw_device *cdev, struct ccw1 *cpa,
				 unsigned long intparm, __u8 lpm, __u8 key,
				 unsigned long flags, int expires)
{
	int ret;

	if (!cdev)
		return -ENODEV;
	ccw_device_set_timeout(cdev, expires);
	ret = ccw_device_start_key(cdev, cpa, intparm, lpm, key, flags);
	if (ret != 0)
		ccw_device_set_timeout(cdev, 0);
	return ret;
}


int ccw_device_start(struct ccw_device *cdev, struct ccw1 *cpa,
		     unsigned long intparm, __u8 lpm, unsigned long flags)
{
	return ccw_device_start_key(cdev, cpa, intparm, lpm,
				    PAGE_DEFAULT_KEY, flags);
}


int ccw_device_start_timeout(struct ccw_device *cdev, struct ccw1 *cpa,
			     unsigned long intparm, __u8 lpm,
			     unsigned long flags, int expires)
{
	return ccw_device_start_timeout_key(cdev, cpa, intparm, lpm,
					    PAGE_DEFAULT_KEY, flags,
					    expires);
}



int ccw_device_halt(struct ccw_device *cdev, unsigned long intparm)
{
	struct subchannel *sch;
	int ret;

	if (!cdev || !cdev->dev.parent)
		return -ENODEV;
	sch = to_subchannel(cdev->dev.parent);
	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state == DEV_STATE_NOT_OPER)
		return -ENODEV;
	if (cdev->private->state != DEV_STATE_ONLINE &&
	    cdev->private->state != DEV_STATE_W4SENSE)
		return -EINVAL;

	ret = cio_halt(sch);
	if (ret == 0)
		cdev->private->intparm = intparm;
	return ret;
}


int ccw_device_resume(struct ccw_device *cdev)
{
	struct subchannel *sch;

	if (!cdev || !cdev->dev.parent)
		return -ENODEV;
	sch = to_subchannel(cdev->dev.parent);
	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state == DEV_STATE_NOT_OPER)
		return -ENODEV;
	if (cdev->private->state != DEV_STATE_ONLINE ||
	    !(sch->schib.scsw.cmd.actl & SCSW_ACTL_SUSPENDED))
		return -EINVAL;
	return cio_resume(sch);
}


int
ccw_device_call_handler(struct ccw_device *cdev)
{
	struct subchannel *sch;
	unsigned int stctl;
	int ending_status;

	sch = to_subchannel(cdev->dev.parent);

	
	stctl = scsw_stctl(&cdev->private->irb.scsw);
	ending_status = (stctl & SCSW_STCTL_SEC_STATUS) ||
		(stctl == (SCSW_STCTL_ALERT_STATUS | SCSW_STCTL_STATUS_PEND)) ||
		(stctl == SCSW_STCTL_STATUS_PEND);
	if (!ending_status &&
	    !cdev->private->options.repall &&
	    !(stctl & SCSW_STCTL_INTER_STATUS) &&
	    !(cdev->private->options.fast &&
	      (stctl & SCSW_STCTL_PRIM_STATUS)))
		return 0;

	
	if (ending_status)
		ccw_device_set_timeout(cdev, 0);
	
	if (cdev->handler)
		cdev->handler(cdev, cdev->private->intparm,
			      &cdev->private->irb);

	
	memset(&cdev->private->irb, 0, sizeof(struct irb));

	return 1;
}


struct ciw *ccw_device_get_ciw(struct ccw_device *cdev, __u32 ct)
{
	int ciw_cnt;

	if (cdev->private->flags.esid == 0)
		return NULL;
	for (ciw_cnt = 0; ciw_cnt < MAX_CIWS; ciw_cnt++)
		if (cdev->private->senseid.ciw[ciw_cnt].ct == ct)
			return cdev->private->senseid.ciw + ciw_cnt;
	return NULL;
}


__u8 ccw_device_get_path_mask(struct ccw_device *cdev)
{
	struct subchannel *sch;

	if (!cdev->dev.parent)
		return 0;

	sch = to_subchannel(cdev->dev.parent);
	return sch->lpm;
}


int
ccw_device_stlck(struct ccw_device *cdev)
{
	void *buf, *buf2;
	unsigned long flags;
	struct subchannel *sch;
	int ret;

	if (!cdev)
		return -ENODEV;

	if (cdev->drv && !cdev->private->options.force)
		return -EINVAL;

	sch = to_subchannel(cdev->dev.parent);
	
	CIO_TRACE_EVENT(2, "stl lock");
	CIO_TRACE_EVENT(2, dev_name(&cdev->dev));

	buf = kmalloc(32*sizeof(char), GFP_DMA|GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	buf2 = kmalloc(32*sizeof(char), GFP_DMA|GFP_KERNEL);
	if (!buf2) {
		kfree(buf);
		return -ENOMEM;
	}
	spin_lock_irqsave(sch->lock, flags);
	ret = cio_enable_subchannel(sch, (u32)(addr_t)sch);
	if (ret)
		goto out_unlock;
	
	cdev->private->iccws[0].cmd_code = CCW_CMD_STLCK;
	cdev->private->iccws[0].cda = (__u32) __pa(buf);
	cdev->private->iccws[0].count = 32;
	cdev->private->iccws[0].flags = CCW_FLAG_CC;
	cdev->private->iccws[1].cmd_code = CCW_CMD_RELEASE;
	cdev->private->iccws[1].cda = (__u32) __pa(buf2);
	cdev->private->iccws[1].count = 32;
	cdev->private->iccws[1].flags = 0;
	ret = cio_start(sch, cdev->private->iccws, 0);
	if (ret) {
		cio_disable_subchannel(sch); 
		goto out_unlock;
	}
	cdev->private->irb.scsw.cmd.actl |= SCSW_ACTL_START_PEND;
	spin_unlock_irqrestore(sch->lock, flags);
	wait_event(cdev->private->wait_q,
		   cdev->private->irb.scsw.cmd.actl == 0);
	spin_lock_irqsave(sch->lock, flags);
	cio_disable_subchannel(sch); 
	if ((cdev->private->irb.scsw.cmd.dstat !=
	     (DEV_STAT_CHN_END|DEV_STAT_DEV_END)) ||
	    (cdev->private->irb.scsw.cmd.cstat != 0))
		ret = -EIO;
	
	memset(&cdev->private->irb, 0, sizeof(struct irb));
out_unlock:
	kfree(buf);
	kfree(buf2);
	spin_unlock_irqrestore(sch->lock, flags);
	return ret;
}

void *ccw_device_get_chp_desc(struct ccw_device *cdev, int chp_no)
{
	struct subchannel *sch;
	struct chp_id chpid;

	sch = to_subchannel(cdev->dev.parent);
	chp_id_init(&chpid);
	chpid.id = sch->schib.pmcw.chpid[chp_no];
	return chp_get_chp_desc(chpid);
}


void ccw_device_get_id(struct ccw_device *cdev, struct ccw_dev_id *dev_id)
{
	*dev_id = cdev->private->dev_id;
}
EXPORT_SYMBOL(ccw_device_get_id);


int ccw_device_tm_start_key(struct ccw_device *cdev, struct tcw *tcw,
			    unsigned long intparm, u8 lpm, u8 key)
{
	struct subchannel *sch;
	int rc;

	sch = to_subchannel(cdev->dev.parent);
	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state != DEV_STATE_ONLINE)
		return -EIO;
	
	if (lpm) {
		lpm &= sch->opm;
		if (lpm == 0)
			return -EACCES;
	}
	rc = cio_tm_start_key(sch, tcw, lpm, key);
	if (rc == 0)
		cdev->private->intparm = intparm;
	return rc;
}
EXPORT_SYMBOL(ccw_device_tm_start_key);


int ccw_device_tm_start_timeout_key(struct ccw_device *cdev, struct tcw *tcw,
				    unsigned long intparm, u8 lpm, u8 key,
				    int expires)
{
	int ret;

	ccw_device_set_timeout(cdev, expires);
	ret = ccw_device_tm_start_key(cdev, tcw, intparm, lpm, key);
	if (ret != 0)
		ccw_device_set_timeout(cdev, 0);
	return ret;
}
EXPORT_SYMBOL(ccw_device_tm_start_timeout_key);


int ccw_device_tm_start(struct ccw_device *cdev, struct tcw *tcw,
			unsigned long intparm, u8 lpm)
{
	return ccw_device_tm_start_key(cdev, tcw, intparm, lpm,
				       PAGE_DEFAULT_KEY);
}
EXPORT_SYMBOL(ccw_device_tm_start);


int ccw_device_tm_start_timeout(struct ccw_device *cdev, struct tcw *tcw,
			       unsigned long intparm, u8 lpm, int expires)
{
	return ccw_device_tm_start_timeout_key(cdev, tcw, intparm, lpm,
					       PAGE_DEFAULT_KEY, expires);
}
EXPORT_SYMBOL(ccw_device_tm_start_timeout);


int ccw_device_tm_intrg(struct ccw_device *cdev)
{
	struct subchannel *sch = to_subchannel(cdev->dev.parent);

	if (!sch->schib.pmcw.ena)
		return -EINVAL;
	if (cdev->private->state != DEV_STATE_ONLINE)
		return -EIO;
	if (!scsw_is_tm(&sch->schib.scsw) ||
	    !(scsw_actl(&sch->schib.scsw) & SCSW_ACTL_START_PEND))
		return -EINVAL;
	return cio_tm_intrg(sch);
}
EXPORT_SYMBOL(ccw_device_tm_intrg);



int
_ccw_device_get_subchannel_number(struct ccw_device *cdev)
{
	return cdev->private->schid.sch_no;
}


MODULE_LICENSE("GPL");
EXPORT_SYMBOL(ccw_device_set_options_mask);
EXPORT_SYMBOL(ccw_device_set_options);
EXPORT_SYMBOL(ccw_device_clear_options);
EXPORT_SYMBOL(ccw_device_clear);
EXPORT_SYMBOL(ccw_device_halt);
EXPORT_SYMBOL(ccw_device_resume);
EXPORT_SYMBOL(ccw_device_start_timeout);
EXPORT_SYMBOL(ccw_device_start);
EXPORT_SYMBOL(ccw_device_start_timeout_key);
EXPORT_SYMBOL(ccw_device_start_key);
EXPORT_SYMBOL(ccw_device_get_ciw);
EXPORT_SYMBOL(ccw_device_get_path_mask);
EXPORT_SYMBOL(_ccw_device_get_subchannel_number);
EXPORT_SYMBOL_GPL(ccw_device_get_chp_desc);
