

#include "wl1271.h"
#include "wl1271_acx.h"
#include "wl1271_reg.h"
#include "wl1271_rx.h"
#include "wl1271_spi.h"

static u8 wl1271_rx_get_mem_block(struct wl1271_fw_status *status,
				  u32 drv_rx_counter)
{
	return status->rx_pkt_descs[drv_rx_counter] & RX_MEM_BLOCK_MASK;
}

static u32 wl1271_rx_get_buf_size(struct wl1271_fw_status *status,
				 u32 drv_rx_counter)
{
	return (status->rx_pkt_descs[drv_rx_counter] & RX_BUF_SIZE_MASK) >>
		RX_BUF_SIZE_SHIFT_DIV;
}


static u8 wl1271_rx_rate_to_idx[] = {
	
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 
	WL1271_RX_RATE_UNSUPPORTED, 

	11,                         
	10,                         
	9,                          
	8,                          

	
	WL1271_RX_RATE_UNSUPPORTED, 

	7,                          
	6,                          
	3,                          
	5,                          
	4,                          
	2,                          
	1,                          
	0                           
};

static void wl1271_rx_status(struct wl1271 *wl,
			     struct wl1271_rx_descriptor *desc,
			     struct ieee80211_rx_status *status,
			     u8 beacon)
{
	memset(status, 0, sizeof(struct ieee80211_rx_status));

	if ((desc->flags & WL1271_RX_DESC_BAND_MASK) == WL1271_RX_DESC_BAND_BG)
		status->band = IEEE80211_BAND_2GHZ;
	else
		wl1271_warning("unsupported band 0x%x",
			       desc->flags & WL1271_RX_DESC_BAND_MASK);

	
	status->signal = desc->rssi;

	
	status->qual = (desc->rssi - WL1271_RX_MIN_RSSI) * 100 /
		(WL1271_RX_MAX_RSSI - WL1271_RX_MIN_RSSI);
	status->qual = min(status->qual, 100);
	status->qual = max(status->qual, 0);

	
	status->noise = desc->rssi - (desc->snr >> 1);

	status->freq = ieee80211_channel_to_frequency(desc->channel);

	if (desc->flags & WL1271_RX_DESC_ENCRYPT_MASK) {
		status->flag |= RX_FLAG_IV_STRIPPED | RX_FLAG_MMIC_STRIPPED;

		if (likely(!(desc->flags & WL1271_RX_DESC_DECRYPT_FAIL)))
			status->flag |= RX_FLAG_DECRYPTED;

		if (unlikely(desc->flags & WL1271_RX_DESC_MIC_FAIL))
			status->flag |= RX_FLAG_MMIC_ERROR;
	}

	status->rate_idx = wl1271_rx_rate_to_idx[desc->rate];

	if (status->rate_idx == WL1271_RX_RATE_UNSUPPORTED)
		wl1271_warning("unsupported rate");
}

static void wl1271_rx_handle_data(struct wl1271 *wl, u32 length)
{
	struct ieee80211_rx_status rx_status;
	struct wl1271_rx_descriptor *desc;
	struct sk_buff *skb;
	u16 *fc;
	u8 *buf;
	u8 beacon = 0;

	skb = dev_alloc_skb(length);
	if (!skb) {
		wl1271_error("Couldn't allocate RX frame");
		return;
	}

	buf = skb_put(skb, length);
	wl1271_spi_reg_read(wl, WL1271_SLV_MEM_DATA, buf, length, true);

	
	desc = (struct wl1271_rx_descriptor *) buf;

	
	skb_pull(skb, sizeof(*desc));

	fc = (u16 *)skb->data;
	if ((*fc & IEEE80211_FCTL_STYPE) == IEEE80211_STYPE_BEACON)
		beacon = 1;

	wl1271_rx_status(wl, desc, &rx_status, beacon);

	wl1271_debug(DEBUG_RX, "rx skb 0x%p: %d B %s", skb, skb->len,
		     beacon ? "beacon" : "");

	memcpy(IEEE80211_SKB_RXCB(skb), &rx_status, sizeof(rx_status));
	ieee80211_rx(wl->hw, skb);
}

void wl1271_rx(struct wl1271 *wl, struct wl1271_fw_status *status)
{
	struct wl1271_acx_mem_map *wl_mem_map = wl->target_mem_map;
	u32 buf_size;
	u32 fw_rx_counter  = status->fw_rx_counter & NUM_RX_PKT_DESC_MOD_MASK;
	u32 drv_rx_counter = wl->rx_counter & NUM_RX_PKT_DESC_MOD_MASK;
	u32 mem_block;

	while (drv_rx_counter != fw_rx_counter) {
		mem_block = wl1271_rx_get_mem_block(status, drv_rx_counter);
		buf_size = wl1271_rx_get_buf_size(status, drv_rx_counter);

		if (buf_size == 0) {
			wl1271_warning("received empty data");
			break;
		}

		wl->rx_mem_pool_addr.addr =
			(mem_block << 8) + wl_mem_map->packet_memory_pool_start;
		wl->rx_mem_pool_addr.addr_extra =
			wl->rx_mem_pool_addr.addr + 4;

		
		wl1271_spi_reg_write(wl, WL1271_SLV_REG_DATA,
				     &wl->rx_mem_pool_addr,
				     sizeof(wl->rx_mem_pool_addr), false);

		wl1271_rx_handle_data(wl, buf_size);

		wl->rx_counter++;
		drv_rx_counter = wl->rx_counter & NUM_RX_PKT_DESC_MOD_MASK;
	}

	wl1271_reg_write32(wl, RX_DRIVER_COUNTER_ADDRESS, wl->rx_counter);

	
	wl1271_reg_write32(wl, RX_DRIVER_DUMMY_WRITE_ADDRESS, 0x1);

}
