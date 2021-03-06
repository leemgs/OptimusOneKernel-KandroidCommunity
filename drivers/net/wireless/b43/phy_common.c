

#include "phy_common.h"
#include "phy_g.h"
#include "phy_a.h"
#include "phy_n.h"
#include "phy_lp.h"
#include "b43.h"
#include "main.h"


int b43_phy_allocate(struct b43_wldev *dev)
{
	struct b43_phy *phy = &(dev->phy);
	int err;

	phy->ops = NULL;

	switch (phy->type) {
	case B43_PHYTYPE_A:
		phy->ops = &b43_phyops_a;
		break;
	case B43_PHYTYPE_G:
		phy->ops = &b43_phyops_g;
		break;
	case B43_PHYTYPE_N:
#ifdef CONFIG_B43_NPHY
		phy->ops = &b43_phyops_n;
#endif
		break;
	case B43_PHYTYPE_LP:
#ifdef CONFIG_B43_PHY_LP
		phy->ops = &b43_phyops_lp;
#endif
		break;
	}
	if (B43_WARN_ON(!phy->ops))
		return -ENODEV;

	err = phy->ops->allocate(dev);
	if (err)
		phy->ops = NULL;

	return err;
}

void b43_phy_free(struct b43_wldev *dev)
{
	dev->phy.ops->free(dev);
	dev->phy.ops = NULL;
}

int b43_phy_init(struct b43_wldev *dev)
{
	struct b43_phy *phy = &dev->phy;
	const struct b43_phy_operations *ops = phy->ops;
	int err;

	phy->channel = ops->get_default_chan(dev);

	ops->software_rfkill(dev, false);
	err = ops->init(dev);
	if (err) {
		b43err(dev->wl, "PHY init failed\n");
		goto err_block_rf;
	}
	
	err = b43_switch_channel(dev, ops->get_default_chan(dev));
	if (err) {
		b43err(dev->wl, "PHY init: Channel switch to default failed\n");
		goto err_phy_exit;
	}

	return 0;

err_phy_exit:
	if (ops->exit)
		ops->exit(dev);
err_block_rf:
	ops->software_rfkill(dev, true);

	return err;
}

void b43_phy_exit(struct b43_wldev *dev)
{
	const struct b43_phy_operations *ops = dev->phy.ops;

	ops->software_rfkill(dev, true);
	if (ops->exit)
		ops->exit(dev);
}

bool b43_has_hardware_pctl(struct b43_wldev *dev)
{
	if (!dev->phy.hardware_power_control)
		return 0;
	if (!dev->phy.ops->supports_hwpctl)
		return 0;
	return dev->phy.ops->supports_hwpctl(dev);
}

void b43_radio_lock(struct b43_wldev *dev)
{
	u32 macctl;

#if B43_DEBUG
	B43_WARN_ON(dev->phy.radio_locked);
	dev->phy.radio_locked = 1;
#endif

	macctl = b43_read32(dev, B43_MMIO_MACCTL);
	macctl |= B43_MACCTL_RADIOLOCK;
	b43_write32(dev, B43_MMIO_MACCTL, macctl);
	
	b43_read32(dev, B43_MMIO_MACCTL);
	udelay(10);
}

void b43_radio_unlock(struct b43_wldev *dev)
{
	u32 macctl;

#if B43_DEBUG
	B43_WARN_ON(!dev->phy.radio_locked);
	dev->phy.radio_locked = 0;
#endif

	
	b43_read16(dev, B43_MMIO_PHY_VER);
	
	macctl = b43_read32(dev, B43_MMIO_MACCTL);
	macctl &= ~B43_MACCTL_RADIOLOCK;
	b43_write32(dev, B43_MMIO_MACCTL, macctl);
}

void b43_phy_lock(struct b43_wldev *dev)
{
#if B43_DEBUG
	B43_WARN_ON(dev->phy.phy_locked);
	dev->phy.phy_locked = 1;
#endif
	B43_WARN_ON(dev->dev->id.revision < 3);

	if (!b43_is_mode(dev->wl, NL80211_IFTYPE_AP))
		b43_power_saving_ctl_bits(dev, B43_PS_AWAKE);
}

void b43_phy_unlock(struct b43_wldev *dev)
{
#if B43_DEBUG
	B43_WARN_ON(!dev->phy.phy_locked);
	dev->phy.phy_locked = 0;
#endif
	B43_WARN_ON(dev->dev->id.revision < 3);

	if (!b43_is_mode(dev->wl, NL80211_IFTYPE_AP))
		b43_power_saving_ctl_bits(dev, 0);
}

static inline void assert_mac_suspended(struct b43_wldev *dev)
{
	if (!B43_DEBUG)
		return;
	if ((b43_status(dev) >= B43_STAT_INITIALIZED) &&
	    (dev->mac_suspended <= 0)) {
		b43dbg(dev->wl, "PHY/RADIO register access with "
		       "enabled MAC.\n");
		dump_stack();
	}
}

u16 b43_radio_read(struct b43_wldev *dev, u16 reg)
{
	assert_mac_suspended(dev);
	return dev->phy.ops->radio_read(dev, reg);
}

void b43_radio_write(struct b43_wldev *dev, u16 reg, u16 value)
{
	assert_mac_suspended(dev);
	dev->phy.ops->radio_write(dev, reg, value);
}

void b43_radio_mask(struct b43_wldev *dev, u16 offset, u16 mask)
{
	b43_radio_write16(dev, offset,
			  b43_radio_read16(dev, offset) & mask);
}

void b43_radio_set(struct b43_wldev *dev, u16 offset, u16 set)
{
	b43_radio_write16(dev, offset,
			  b43_radio_read16(dev, offset) | set);
}

void b43_radio_maskset(struct b43_wldev *dev, u16 offset, u16 mask, u16 set)
{
	b43_radio_write16(dev, offset,
			  (b43_radio_read16(dev, offset) & mask) | set);
}

u16 b43_phy_read(struct b43_wldev *dev, u16 reg)
{
	assert_mac_suspended(dev);
	return dev->phy.ops->phy_read(dev, reg);
}

void b43_phy_write(struct b43_wldev *dev, u16 reg, u16 value)
{
	assert_mac_suspended(dev);
	dev->phy.ops->phy_write(dev, reg, value);
}

void b43_phy_copy(struct b43_wldev *dev, u16 destreg, u16 srcreg)
{
	assert_mac_suspended(dev);
	dev->phy.ops->phy_write(dev, destreg,
		dev->phy.ops->phy_read(dev, srcreg));
}

void b43_phy_mask(struct b43_wldev *dev, u16 offset, u16 mask)
{
	if (dev->phy.ops->phy_maskset) {
		assert_mac_suspended(dev);
		dev->phy.ops->phy_maskset(dev, offset, mask, 0);
	} else {
		b43_phy_write(dev, offset,
			      b43_phy_read(dev, offset) & mask);
	}
}

void b43_phy_set(struct b43_wldev *dev, u16 offset, u16 set)
{
	if (dev->phy.ops->phy_maskset) {
		assert_mac_suspended(dev);
		dev->phy.ops->phy_maskset(dev, offset, 0xFFFF, set);
	} else {
		b43_phy_write(dev, offset,
			      b43_phy_read(dev, offset) | set);
	}
}

void b43_phy_maskset(struct b43_wldev *dev, u16 offset, u16 mask, u16 set)
{
	if (dev->phy.ops->phy_maskset) {
		assert_mac_suspended(dev);
		dev->phy.ops->phy_maskset(dev, offset, mask, set);
	} else {
		b43_phy_write(dev, offset,
			      (b43_phy_read(dev, offset) & mask) | set);
	}
}

int b43_switch_channel(struct b43_wldev *dev, unsigned int new_channel)
{
	struct b43_phy *phy = &(dev->phy);
	u16 channelcookie, savedcookie;
	int err;

	if (new_channel == B43_DEFAULT_CHANNEL)
		new_channel = phy->ops->get_default_chan(dev);

	
	channelcookie = new_channel;
	if (b43_current_band(dev->wl) == IEEE80211_BAND_5GHZ)
		channelcookie |= 0x100;
	
	savedcookie = b43_shm_read16(dev, B43_SHM_SHARED, B43_SHM_SH_CHAN);
	b43_shm_write16(dev, B43_SHM_SHARED, B43_SHM_SH_CHAN, channelcookie);

	
	err = phy->ops->switch_channel(dev, new_channel);
	if (err)
		goto err_restore_cookie;

	dev->phy.channel = new_channel;
	
	msleep(8);

	return 0;

err_restore_cookie:
	b43_shm_write16(dev, B43_SHM_SHARED,
			B43_SHM_SH_CHAN, savedcookie);

	return err;
}

void b43_software_rfkill(struct b43_wldev *dev, bool blocked)
{
	struct b43_phy *phy = &dev->phy;

	b43_mac_suspend(dev);
	phy->ops->software_rfkill(dev, blocked);
	phy->radio_on = !blocked;
	b43_mac_enable(dev);
}


void b43_phy_txpower_adjust_work(struct work_struct *work)
{
	struct b43_wl *wl = container_of(work, struct b43_wl,
					 txpower_adjust_work);
	struct b43_wldev *dev;

	mutex_lock(&wl->mutex);
	dev = wl->current_dev;

	if (likely(dev && (b43_status(dev) >= B43_STAT_STARTED)))
		dev->phy.ops->adjust_txpower(dev);

	mutex_unlock(&wl->mutex);
}

void b43_phy_txpower_check(struct b43_wldev *dev, unsigned int flags)
{
	struct b43_phy *phy = &dev->phy;
	unsigned long now = jiffies;
	enum b43_txpwr_result result;

	if (!(flags & B43_TXPWR_IGNORE_TIME)) {
		
		if (time_before(now, phy->next_txpwr_check_time))
			return; 
	}
	
	phy->next_txpwr_check_time = round_jiffies(now + (HZ * 2));

	if ((dev->dev->bus->boardinfo.vendor == SSB_BOARDVENDOR_BCM) &&
	    (dev->dev->bus->boardinfo.type == SSB_BOARD_BU4306))
		return; 

	result = phy->ops->recalc_txpower(dev, !!(flags & B43_TXPWR_IGNORE_TSSI));
	if (result == B43_TXPWR_RES_DONE)
		return; 
	B43_WARN_ON(result != B43_TXPWR_RES_NEED_ADJUST);
	B43_WARN_ON(phy->ops->adjust_txpower == NULL);

	
	ieee80211_queue_work(dev->wl->hw, &dev->wl->txpower_adjust_work);
}

int b43_phy_shm_tssi_read(struct b43_wldev *dev, u16 shm_offset)
{
	const bool is_ofdm = (shm_offset != B43_SHM_SH_TSSI_CCK);
	unsigned int a, b, c, d;
	unsigned int average;
	u32 tmp;

	tmp = b43_shm_read32(dev, B43_SHM_SHARED, shm_offset);
	a = tmp & 0xFF;
	b = (tmp >> 8) & 0xFF;
	c = (tmp >> 16) & 0xFF;
	d = (tmp >> 24) & 0xFF;
	if (a == 0 || a == B43_TSSI_MAX ||
	    b == 0 || b == B43_TSSI_MAX ||
	    c == 0 || c == B43_TSSI_MAX ||
	    d == 0 || d == B43_TSSI_MAX)
		return -ENOENT;
	
	tmp = B43_TSSI_MAX | (B43_TSSI_MAX << 8) |
	      (B43_TSSI_MAX << 16) | (B43_TSSI_MAX << 24);
	b43_shm_write32(dev, B43_SHM_SHARED, shm_offset, tmp);

	if (is_ofdm) {
		a = (a + 32) & 0x3F;
		b = (b + 32) & 0x3F;
		c = (c + 32) & 0x3F;
		d = (d + 32) & 0x3F;
	}

	
	average = (a + b + c + d + 2) / 4;
	if (is_ofdm) {
		
		if (b43_shm_read16(dev, B43_SHM_SHARED, B43_SHM_SH_HOSTFLO)
		    & B43_HF_CCKBOOST)
			average = (average >= 13) ? (average - 13) : 0;
	}

	return average;
}

void b43_phyop_switch_analog_generic(struct b43_wldev *dev, bool on)
{
	b43_write16(dev, B43_MMIO_PHY0, on ? 0 : 0xF4);
}
