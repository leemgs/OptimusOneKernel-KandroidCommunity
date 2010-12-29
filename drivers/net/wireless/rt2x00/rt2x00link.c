



#include <linux/kernel.h>
#include <linux/module.h>

#include "rt2x00.h"
#include "rt2x00lib.h"


#define DEFAULT_RSSI		-128


#define DEFAULT_PERCENTAGE	50


#define PERCENTAGE(__value, __total) \
	( (__total) ? (((__value) * 100) / (__total)) : (DEFAULT_PERCENTAGE) )


#define AVG_SAMPLES	8
#define AVG_FACTOR	1000
#define MOVING_AVERAGE(__avg, __val) \
({ \
	struct avg_val __new; \
	__new.avg_weight = \
	    (__avg).avg_weight  ? \
		((((__avg).avg_weight * ((AVG_SAMPLES) - 1)) + \
		  ((__val) * (AVG_FACTOR))) / \
		 (AVG_SAMPLES) ) : \
		((__val) * (AVG_FACTOR)); \
	__new.avg = __new.avg_weight / (AVG_FACTOR); \
	__new; \
})


#define WEIGHT_RSSI	20
#define WEIGHT_RX	40
#define WEIGHT_TX	40

static int rt2x00link_antenna_get_link_rssi(struct rt2x00_dev *rt2x00dev)
{
	struct link_ant *ant = &rt2x00dev->link.ant;

	if (ant->rssi_ant.avg && rt2x00dev->link.qual.rx_success)
		return ant->rssi_ant.avg;
	return DEFAULT_RSSI;
}

static int rt2x00link_antenna_get_rssi_history(struct rt2x00_dev *rt2x00dev)
{
	struct link_ant *ant = &rt2x00dev->link.ant;

	if (ant->rssi_history)
		return ant->rssi_history;
	return DEFAULT_RSSI;
}

static void rt2x00link_antenna_update_rssi_history(struct rt2x00_dev *rt2x00dev,
						   int rssi)
{
	struct link_ant *ant = &rt2x00dev->link.ant;
	ant->rssi_history = rssi;
}

static void rt2x00link_antenna_reset(struct rt2x00_dev *rt2x00dev)
{
	rt2x00dev->link.ant.rssi_ant.avg = 0;
	rt2x00dev->link.ant.rssi_ant.avg_weight = 0;
}

static void rt2x00lib_antenna_diversity_sample(struct rt2x00_dev *rt2x00dev)
{
	struct link_ant *ant = &rt2x00dev->link.ant;
	struct antenna_setup new_ant;
	int other_antenna;

	int sample_current = rt2x00link_antenna_get_link_rssi(rt2x00dev);
	int sample_other = rt2x00link_antenna_get_rssi_history(rt2x00dev);

	memcpy(&new_ant, &ant->active, sizeof(new_ant));

	
	ant->flags &= ~ANTENNA_MODE_SAMPLE;

	
	if (sample_current >= sample_other) {
		rt2x00link_antenna_update_rssi_history(rt2x00dev,
			sample_current);
		return;
	}

	other_antenna = (ant->active.rx == ANTENNA_A) ? ANTENNA_B : ANTENNA_A;

	if (ant->flags & ANTENNA_RX_DIVERSITY)
		new_ant.rx = other_antenna;

	if (ant->flags & ANTENNA_TX_DIVERSITY)
		new_ant.tx = other_antenna;

	rt2x00lib_config_antenna(rt2x00dev, new_ant);
}

static void rt2x00lib_antenna_diversity_eval(struct rt2x00_dev *rt2x00dev)
{
	struct link_ant *ant = &rt2x00dev->link.ant;
	struct antenna_setup new_ant;
	int rssi_curr;
	int rssi_old;

	memcpy(&new_ant, &ant->active, sizeof(new_ant));

	
	rssi_curr = rt2x00link_antenna_get_link_rssi(rt2x00dev);
	rssi_old = rt2x00link_antenna_get_rssi_history(rt2x00dev);
	rt2x00link_antenna_update_rssi_history(rt2x00dev, rssi_curr);

	
	if (abs(rssi_curr - rssi_old) < 5)
		return;

	ant->flags |= ANTENNA_MODE_SAMPLE;

	if (ant->flags & ANTENNA_RX_DIVERSITY)
		new_ant.rx = (new_ant.rx == ANTENNA_A) ? ANTENNA_B : ANTENNA_A;

	if (ant->flags & ANTENNA_TX_DIVERSITY)
		new_ant.tx = (new_ant.tx == ANTENNA_A) ? ANTENNA_B : ANTENNA_A;

	rt2x00lib_config_antenna(rt2x00dev, new_ant);
}

static bool rt2x00lib_antenna_diversity(struct rt2x00_dev *rt2x00dev)
{
	struct link_ant *ant = &rt2x00dev->link.ant;
	unsigned int flags = ant->flags;

	
	flags &= ~ANTENNA_RX_DIVERSITY;
	flags &= ~ANTENNA_TX_DIVERSITY;

	if (rt2x00dev->default_ant.rx == ANTENNA_SW_DIVERSITY)
		flags |= ANTENNA_RX_DIVERSITY;
	if (rt2x00dev->default_ant.tx == ANTENNA_SW_DIVERSITY)
		flags |= ANTENNA_TX_DIVERSITY;

	if (!(ant->flags & ANTENNA_RX_DIVERSITY) &&
	    !(ant->flags & ANTENNA_TX_DIVERSITY)) {
		ant->flags = 0;
		return true;
	}

	
	ant->flags = flags;

	
	if (ant->flags & ANTENNA_MODE_SAMPLE) {
		rt2x00lib_antenna_diversity_sample(rt2x00dev);
		return true;
	} else if (rt2x00dev->link.count & 1) {
		rt2x00lib_antenna_diversity_eval(rt2x00dev);
		return true;
	}

	return false;
}

void rt2x00link_update_stats(struct rt2x00_dev *rt2x00dev,
			     struct sk_buff *skb,
			     struct rxdone_entry_desc *rxdesc)
{
	struct link *link = &rt2x00dev->link;
	struct link_qual *qual = &rt2x00dev->link.qual;
	struct link_ant *ant = &rt2x00dev->link.ant;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	
	qual->rx_success++;

	
	if (!ieee80211_is_beacon(hdr->frame_control) ||
	    !(rxdesc->dev_flags & RXDONE_MY_BSS))
		return;

	
	link->avg_rssi = MOVING_AVERAGE(link->avg_rssi, rxdesc->rssi);

	
	ant->rssi_ant = MOVING_AVERAGE(ant->rssi_ant, rxdesc->rssi);
}

static void rt2x00link_precalculate_signal(struct rt2x00_dev *rt2x00dev)
{
	struct link *link = &rt2x00dev->link;
	struct link_qual *qual = &rt2x00dev->link.qual;

	link->rx_percentage =
	    PERCENTAGE(qual->rx_success, qual->rx_failed + qual->rx_success);
	link->tx_percentage =
	    PERCENTAGE(qual->tx_success, qual->tx_failed + qual->tx_success);
}

int rt2x00link_calculate_signal(struct rt2x00_dev *rt2x00dev, int rssi)
{
	struct link *link = &rt2x00dev->link;
	int rssi_percentage = 0;
	int signal;

	
	if (rssi < 0)
		rssi += rt2x00dev->rssi_offset;

	
	rssi_percentage = PERCENTAGE(rssi, rt2x00dev->rssi_offset);

	
	signal = ((WEIGHT_RSSI * rssi_percentage) +
		  (WEIGHT_TX * link->tx_percentage) +
		  (WEIGHT_RX * link->rx_percentage)) / 100;

	return max_t(int, signal, 100);
}

void rt2x00link_start_tuner(struct rt2x00_dev *rt2x00dev)
{
	struct link *link = &rt2x00dev->link;

	
	if (!rt2x00dev->intf_ap_count && !rt2x00dev->intf_sta_count)
		return;

	link->rx_percentage = DEFAULT_PERCENTAGE;
	link->tx_percentage = DEFAULT_PERCENTAGE;

	rt2x00link_reset_tuner(rt2x00dev, false);

	if (test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		ieee80211_queue_delayed_work(rt2x00dev->hw,
					     &link->work, LINK_TUNE_INTERVAL);
}

void rt2x00link_stop_tuner(struct rt2x00_dev *rt2x00dev)
{
	cancel_delayed_work_sync(&rt2x00dev->link.work);
}

void rt2x00link_reset_tuner(struct rt2x00_dev *rt2x00dev, bool antenna)
{
	struct link_qual *qual = &rt2x00dev->link.qual;

	if (!test_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return;

	
	rt2x00dev->link.count = 0;
	memset(qual, 0, sizeof(*qual));

	
	rt2x00dev->ops->lib->reset_tuner(rt2x00dev, qual);

	if (antenna)
		rt2x00link_antenna_reset(rt2x00dev);
}

static void rt2x00link_reset_qual(struct rt2x00_dev *rt2x00dev)
{
	struct link_qual *qual = &rt2x00dev->link.qual;

	qual->rx_success = 0;
	qual->rx_failed = 0;
	qual->tx_success = 0;
	qual->tx_failed = 0;
}

static void rt2x00link_tuner(struct work_struct *work)
{
	struct rt2x00_dev *rt2x00dev =
	    container_of(work, struct rt2x00_dev, link.work.work);
	struct link *link = &rt2x00dev->link;
	struct link_qual *qual = &rt2x00dev->link.qual;

	
	if (!test_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return;

	
	rt2x00dev->ops->lib->link_stats(rt2x00dev, qual);
	rt2x00dev->low_level_stats.dot11FCSErrorCount += qual->rx_failed;

	
	if (!link->avg_rssi.avg || !qual->rx_success)
		qual->rssi = DEFAULT_RSSI;
	else
		qual->rssi = link->avg_rssi.avg;

	
	if (!test_bit(CONFIG_DISABLE_LINK_TUNING, &rt2x00dev->flags))
		rt2x00dev->ops->lib->link_tuner(rt2x00dev, qual, link->count);

	
	rt2x00link_precalculate_signal(rt2x00dev);

	
	rt2x00leds_led_quality(rt2x00dev, qual->rssi);

	
	if (rt2x00lib_antenna_diversity(rt2x00dev))
		rt2x00link_reset_qual(rt2x00dev);

	
	link->count++;

	if (test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		ieee80211_queue_delayed_work(rt2x00dev->hw,
					     &link->work, LINK_TUNE_INTERVAL);
}

void rt2x00link_register(struct rt2x00_dev *rt2x00dev)
{
	INIT_DELAYED_WORK(&rt2x00dev->link.work, rt2x00link_tuner);
}
