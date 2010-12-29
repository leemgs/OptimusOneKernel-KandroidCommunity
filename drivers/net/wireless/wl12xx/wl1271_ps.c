

#include "wl1271_reg.h"
#include "wl1271_ps.h"
#include "wl1271_spi.h"

#define WL1271_WAKEUP_TIMEOUT 500


void wl1271_ps_elp_sleep(struct wl1271 *wl)
{
	
	if (true || wl->elp || !wl->psm)
		return;

	
	if (!work_pending(&wl->irq_work) && !work_pending(&wl->tx_work)) {
		wl1271_debug(DEBUG_PSM, "chip to elp");
		wl1271_write32(wl, HW_ACCESS_ELP_CTRL_REG_ADDR, ELPCTRL_SLEEP);
		wl->elp = true;
	}
}

int wl1271_ps_elp_wakeup(struct wl1271 *wl, bool chip_awake)
{
	DECLARE_COMPLETION_ONSTACK(compl);
	unsigned long flags;
	int ret;
	u32 start_time = jiffies;
	bool pending = false;

	if (!wl->elp)
		return 0;

	wl1271_debug(DEBUG_PSM, "waking up chip from elp");

	
	spin_lock_irqsave(&wl->wl_lock, flags);
	if (work_pending(&wl->irq_work) || chip_awake)
		pending = true;
	else
		wl->elp_compl = &compl;
	spin_unlock_irqrestore(&wl->wl_lock, flags);

	wl1271_write32(wl, HW_ACCESS_ELP_CTRL_REG_ADDR, ELPCTRL_WAKE_UP);

	if (!pending) {
		ret = wait_for_completion_timeout(
			&compl, msecs_to_jiffies(WL1271_WAKEUP_TIMEOUT));
		if (ret == 0) {
			wl1271_error("ELP wakeup timeout!");
			ret = -ETIMEDOUT;
			goto err;
		} else if (ret < 0) {
			wl1271_error("ELP wakeup completion error.");
			goto err;
		}
	}

	wl->elp = false;

	wl1271_debug(DEBUG_PSM, "wakeup time: %u ms",
		     jiffies_to_msecs(jiffies - start_time));
	goto out;

err:
	spin_lock_irqsave(&wl->wl_lock, flags);
	wl->elp_compl = NULL;
	spin_unlock_irqrestore(&wl->wl_lock, flags);
	return ret;

out:
	return 0;
}

int wl1271_ps_set_mode(struct wl1271 *wl, enum wl1271_cmd_ps_mode mode)
{
	int ret;

	switch (mode) {
	case STATION_POWER_SAVE_MODE:
		wl1271_debug(DEBUG_PSM, "entering psm");
		ret = wl1271_cmd_ps_mode(wl, STATION_POWER_SAVE_MODE);
		if (ret < 0)
			return ret;

		wl1271_ps_elp_sleep(wl);
		if (ret < 0)
			return ret;

		wl->psm = 1;
		break;
	case STATION_ACTIVE_MODE:
	default:
		wl1271_debug(DEBUG_PSM, "leaving psm");
		ret = wl1271_ps_elp_wakeup(wl, false);
		if (ret < 0)
			return ret;

		ret = wl1271_cmd_ps_mode(wl, STATION_ACTIVE_MODE);
		if (ret < 0)
			return ret;

		wl->psm = 0;
		break;
	}

	return ret;
}


