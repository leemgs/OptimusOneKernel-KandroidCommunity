



#include <linux/kernel.h>
#include <linux/module.h>

#include "rt2x00.h"
#include "rt2x00lib.h"

void rt2x00ht_create_tx_descriptor(struct queue_entry *entry,
				   struct txentry_desc *txdesc,
				   const struct rt2x00_rate *hwrate)
{
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(entry->skb);
	struct ieee80211_tx_rate *txrate = &tx_info->control.rates[0];

	if (tx_info->control.sta)
		txdesc->mpdu_density =
		    tx_info->control.sta->ht_cap.ampdu_density;
	else
		txdesc->mpdu_density = 0;

	txdesc->ba_size = 7;	
	txdesc->stbc = 0;	

	txdesc->mcs = rt2x00_get_rate_mcs(hwrate->mcs);
	if (txrate->flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE)
		txdesc->mcs |= 0x08;

	
	if (tx_info->flags & IEEE80211_TX_CTL_AMPDU)
		__set_bit(ENTRY_TXD_HT_AMPDU, &txdesc->flags);

	
	if (txrate->flags & IEEE80211_TX_RC_MCS)
		txdesc->rate_mode = RATE_MODE_HT_MIX;
	if (txrate->flags & IEEE80211_TX_RC_GREEN_FIELD)
		txdesc->rate_mode = RATE_MODE_HT_GREENFIELD;
	if (txrate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
		__set_bit(ENTRY_TXD_HT_BW_40, &txdesc->flags);
	if (txrate->flags & IEEE80211_TX_RC_SHORT_GI)
		__set_bit(ENTRY_TXD_HT_SHORT_GI, &txdesc->flags);
}
