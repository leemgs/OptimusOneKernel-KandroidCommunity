

#include <typedefs.h>
#include <pcicfg.h>
#include <bcmutils.h>
#include <sdio.h>	
#include <bcmsdbus.h>	
#include <sdiovar.h>	

#include <linux/sched.h>	

#include <bcmsdstd.h>

struct sdos_info {
	sdioh_info_t *sd;
	spinlock_t lock;
	wait_queue_head_t intr_wait_queue;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define BLOCKABLE()	(!in_atomic())
#else
#define BLOCKABLE()	(!in_interrupt())
#endif


static irqreturn_t
sdstd_isr(int irq, void *dev_id
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
, struct pt_regs *ptregs
#endif
)
{
	sdioh_info_t *sd;
	struct sdos_info *sdos;
	bool ours;

	sd = (sdioh_info_t *)dev_id;

	if (!sd->card_init_done) {
		sd_err(("%s: Hey Bogus intr...not even initted: irq %d\n", __FUNCTION__, irq));
		return IRQ_RETVAL(FALSE);
	} else {
		ours = check_client_intr(sd);

		
		if (ours && sd->got_hcint) {
			sd_trace(("INTR->WAKE\n"));
			sdos = (struct sdos_info *)sd->sdos_info;
			wake_up_interruptible(&sdos->intr_wait_queue);
		}
		return IRQ_RETVAL(ours);
	}
}


int
sdstd_register_irq(sdioh_info_t *sd, uint irq)
{
	sd_trace(("Entering %s: irq == %d\n", __FUNCTION__, irq));
	if (request_irq(irq, sdstd_isr, IRQF_SHARED, "bcmsdstd", sd) < 0) {
		sd_err(("%s: request_irq() failed\n", __FUNCTION__));
		return ERROR;
	}
	return SUCCESS;
}


void
sdstd_free_irq(uint irq, sdioh_info_t *sd)
{
	free_irq(irq, sd);
}



uint32 *
sdstd_reg_map(osl_t *osh, int32 addr, int size)
{
	return (uint32 *)REG_MAP(addr, size);
}

void
sdstd_reg_unmap(osl_t *osh, int32 addr, int size)
{
	REG_UNMAP((void*)(uintptr)addr);
}

int
sdstd_osinit(sdioh_info_t *sd)
{
	struct sdos_info *sdos;

	sdos = (struct sdos_info*)MALLOC(sd->osh, sizeof(struct sdos_info));
	sd->sdos_info = (void*)sdos;
	if (sdos == NULL)
		return BCME_NOMEM;

	sdos->sd = sd;
	spin_lock_init(&sdos->lock);
	init_waitqueue_head(&sdos->intr_wait_queue);
	return BCME_OK;
}

void
sdstd_osfree(sdioh_info_t *sd)
{
	struct sdos_info *sdos;
	ASSERT(sd && sd->sdos_info);

	sdos = (struct sdos_info *)sd->sdos_info;
	MFREE(sd->osh, sdos, sizeof(struct sdos_info));
}


SDIOH_API_RC
sdioh_interrupt_set(sdioh_info_t *sd, bool enable)
{
	ulong flags;
	struct sdos_info *sdos;

	sd_trace(("%s: %s\n", __FUNCTION__, enable ? "Enabling" : "Disabling"));

	sdos = (struct sdos_info *)sd->sdos_info;
	ASSERT(sdos);

	if (!(sd->host_init_done && sd->card_init_done)) {
		sd_err(("%s: Card & Host are not initted - bailing\n", __FUNCTION__));
		return SDIOH_API_RC_FAIL;
	}

	if (enable && !(sd->intr_handler && sd->intr_handler_arg)) {
		sd_err(("%s: no handler registered, will not enable\n", __FUNCTION__));
		return SDIOH_API_RC_FAIL;
	}

	
	spin_lock_irqsave(&sdos->lock, flags);

	sd->client_intr_enabled = enable;
	if (enable && !sd->lockcount)
		sdstd_devintr_on(sd);
	else
		sdstd_devintr_off(sd);

	spin_unlock_irqrestore(&sdos->lock, flags);

	return SDIOH_API_RC_SUCCESS;
}


void
sdstd_lock(sdioh_info_t *sd)
{
	ulong flags;
	struct sdos_info *sdos;

	sdos = (struct sdos_info *)sd->sdos_info;
	ASSERT(sdos);

	sd_trace(("%s: %d\n", __FUNCTION__, sd->lockcount));

	spin_lock_irqsave(&sdos->lock, flags);
	if (sd->lockcount) {
		sd_err(("%s: Already locked!\n", __FUNCTION__));
		ASSERT(sd->lockcount == 0);
	}
	sdstd_devintr_off(sd);
	sd->lockcount++;
	spin_unlock_irqrestore(&sdos->lock, flags);
}


void
sdstd_unlock(sdioh_info_t *sd)
{
	ulong flags;
	struct sdos_info *sdos;

	sd_trace(("%s: %d, %d\n", __FUNCTION__, sd->lockcount, sd->client_intr_enabled));
	ASSERT(sd->lockcount > 0);

	sdos = (struct sdos_info *)sd->sdos_info;
	ASSERT(sdos);

	spin_lock_irqsave(&sdos->lock, flags);
	if (--sd->lockcount == 0 && sd->client_intr_enabled) {
		sdstd_devintr_on(sd);
	}
	spin_unlock_irqrestore(&sdos->lock, flags);
}

uint16
sdstd_waitbits(sdioh_info_t *sd, uint16 norm, uint16 err, bool yield)
{
	struct sdos_info *sdos;

	sdos = (struct sdos_info *)sd->sdos_info;

#ifndef BCMSDYIELD
	ASSERT(!yield);
#endif
	sd_trace(("%s: int 0x%02x err 0x%02x yield %d canblock %d\n",
	          __FUNCTION__, norm, err, yield, BLOCKABLE()));

	
	sd->got_hcint = FALSE;
	sd->last_intrstatus = 0;

#ifdef BCMSDYIELD
	if (yield && BLOCKABLE()) {
		
		sdstd_intrs_on(sd, norm, err);
		wait_event_interruptible(sdos->intr_wait_queue, (sd->got_hcint));
		sdstd_intrs_off(sd, norm, err);
	} else
#endif 
	{
		sdstd_spinbits(sd, norm, err);
	}

	sd_trace(("%s: last_intrstatus 0x%04x\n", __FUNCTION__, sd->last_intrstatus));

	return sd->last_intrstatus;
}
