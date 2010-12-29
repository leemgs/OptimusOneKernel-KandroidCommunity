


#ifndef __iwl_3945_h__
#define __iwl_3945_h__

#include <linux/pci.h> 
#include <linux/kernel.h>
#include <net/ieee80211_radiotap.h>


extern struct pci_device_id iwl3945_hw_card_ids[];

#include "iwl-csr.h"
#include "iwl-prph.h"
#include "iwl-fh.h"
#include "iwl-3945-hw.h"
#include "iwl-debug.h"
#include "iwl-power.h"
#include "iwl-dev.h"
#include "iwl-3945-led.h"


#define IWL3945_UCODE_API_MAX 2


#define IWL3945_UCODE_API_MIN 1

#define IWL3945_FW_PRE	"iwlwifi-3945-"
#define _IWL3945_MODULE_FIRMWARE(api) IWL3945_FW_PRE #api ".ucode"
#define IWL3945_MODULE_FIRMWARE(api) _IWL3945_MODULE_FIRMWARE(api)


#define IWL_NOISE_MEAS_NOT_AVAILABLE (-127)


extern struct iwl_mod_params iwl3945_mod_params;

struct iwl3945_sta_priv {
	struct iwl3945_rs_sta *rs_sta;
};

enum iwl3945_antenna {
	IWL_ANTENNA_DIVERSITY,
	IWL_ANTENNA_MAIN,
	IWL_ANTENNA_AUX
};


#define DEFAULT_RTS_THRESHOLD     2347U
#define MIN_RTS_THRESHOLD         0U
#define MAX_RTS_THRESHOLD         2347U
#define MAX_MSDU_SIZE		  2304U
#define MAX_MPDU_SIZE		  2346U
#define DEFAULT_BEACON_INTERVAL   100U
#define	DEFAULT_SHORT_RETRY_LIMIT 7U
#define	DEFAULT_LONG_RETRY_LIMIT  4U

#include "iwl-agn-rs.h"

#define IWL_TX_FIFO_AC0	0
#define IWL_TX_FIFO_AC1	1
#define IWL_TX_FIFO_AC2	2
#define IWL_TX_FIFO_AC3	3
#define IWL_TX_FIFO_HCCA_1	5
#define IWL_TX_FIFO_HCCA_2	6
#define IWL_TX_FIFO_NONE	7

#define IEEE80211_DATA_LEN              2304
#define IEEE80211_4ADDR_LEN             30
#define IEEE80211_HLEN                  (IEEE80211_4ADDR_LEN)
#define IEEE80211_FRAME_LEN             (IEEE80211_DATA_LEN + IEEE80211_HLEN)

struct iwl3945_frame {
	union {
		struct ieee80211_hdr frame;
		struct iwl3945_tx_beacon_cmd beacon;
		u8 raw[IEEE80211_FRAME_LEN];
		u8 cmd[360];
	} u;
	struct list_head list;
};

#define SEQ_TO_SN(seq) (((seq) & IEEE80211_SCTL_SEQ) >> 4)
#define SN_TO_SEQ(ssn) (((ssn) << 4) & IEEE80211_SCTL_SEQ)
#define MAX_SN ((IEEE80211_SCTL_SEQ) >> 4)


#define RX_FREE_BUFFERS 64
#define RX_LOW_WATERMARK 8

#define SUP_RATE_11A_MAX_NUM_CHANNELS  8
#define SUP_RATE_11B_MAX_NUM_CHANNELS  4
#define SUP_RATE_11G_MAX_NUM_CHANNELS  12

#define IWL_SUPPORTED_RATES_IE_LEN         8

#define SCAN_INTERVAL 100

#define STATUS_HCMD_ACTIVE	0	
#define STATUS_HCMD_SYNC_ACTIVE	1	
#define STATUS_INT_ENABLED	2
#define STATUS_RF_KILL_HW	3
#define STATUS_INIT		5
#define STATUS_ALIVE		6
#define STATUS_READY		7
#define STATUS_TEMPERATURE	8
#define STATUS_GEO_CONFIGURED	9
#define STATUS_EXIT_PENDING	10
#define STATUS_STATISTICS	12
#define STATUS_SCANNING		13
#define STATUS_SCAN_ABORTING	14
#define STATUS_SCAN_HW		15
#define STATUS_POWER_PMI	16
#define STATUS_FW_ERROR		17
#define STATUS_CONF_PENDING	18

#define MAX_TID_COUNT        9

#define IWL_INVALID_RATE     0xFF
#define IWL_INVALID_VALUE    -1

#define STA_PS_STATUS_WAKE             0
#define STA_PS_STATUS_SLEEP            1

struct iwl3945_ibss_seq {
	u8 mac[ETH_ALEN];
	u16 seq_num;
	u16 frag_num;
	unsigned long packet_time;
	struct list_head list;
};

#define IWL_RX_HDR(x) ((struct iwl3945_rx_frame_hdr *)(\
		       x->u.rx_frame.stats.payload + \
		       x->u.rx_frame.stats.phy_count))
#define IWL_RX_END(x) ((struct iwl3945_rx_frame_end *)(\
		       IWL_RX_HDR(x)->payload + \
		       le16_to_cpu(IWL_RX_HDR(x)->len)))
#define IWL_RX_STATS(x) (&x->u.rx_frame.stats)
#define IWL_RX_DATA(x) (IWL_RX_HDR(x)->payload)



extern int iwl3945_power_init_handle(struct iwl_priv *priv);
extern int iwl3945_eeprom_init(struct iwl_priv *priv);
extern int iwl3945_calc_db_from_ratio(int sig_ratio);
extern int iwl3945_calc_sig_qual(int rssi_dbm, int noise_dbm);
extern int iwl3945_tx_queue_init(struct iwl_priv *priv,
			     struct iwl_tx_queue *txq, int count, u32 id);
extern void iwl3945_rx_replenish(void *data);
extern void iwl3945_rx_queue_reset(struct iwl_priv *priv, struct iwl_rx_queue *rxq);
extern void iwl3945_tx_queue_free(struct iwl_priv *priv, struct iwl_tx_queue *txq);
extern int iwl3945_send_cmd_pdu(struct iwl_priv *priv, u8 id, u16 len,
			    const void *data);
extern int __must_check iwl3945_send_cmd(struct iwl_priv *priv,
					 struct iwl_host_cmd *cmd);
extern unsigned int iwl3945_fill_beacon_frame(struct iwl_priv *priv,
					struct ieee80211_hdr *hdr,int left);
extern void iwl3945_dump_nic_event_log(struct iwl_priv *priv);
extern void iwl3945_dump_nic_error_log(struct iwl_priv *priv);


extern u8 iwl3945_sync_station(struct iwl_priv *priv, int sta_id,
			   u16 tx_rate, u8 flags);


extern void iwl3945_hw_rx_handler_setup(struct iwl_priv *priv);
extern void iwl3945_hw_setup_deferred_work(struct iwl_priv *priv);
extern void iwl3945_hw_cancel_deferred_work(struct iwl_priv *priv);
extern int iwl3945_hw_rxq_stop(struct iwl_priv *priv);
extern int iwl3945_hw_set_hw_params(struct iwl_priv *priv);
extern int iwl3945_hw_nic_init(struct iwl_priv *priv);
extern int iwl3945_hw_nic_stop_master(struct iwl_priv *priv);
extern void iwl3945_hw_txq_ctx_free(struct iwl_priv *priv);
extern void iwl3945_hw_txq_ctx_stop(struct iwl_priv *priv);
extern int iwl3945_hw_nic_reset(struct iwl_priv *priv);
extern int iwl3945_hw_txq_attach_buf_to_tfd(struct iwl_priv *priv,
					    struct iwl_tx_queue *txq,
					    dma_addr_t addr, u16 len,
					    u8 reset, u8 pad);
extern void iwl3945_hw_txq_free_tfd(struct iwl_priv *priv,
				    struct iwl_tx_queue *txq);
extern int iwl3945_hw_get_temperature(struct iwl_priv *priv);
extern int iwl3945_hw_tx_queue_init(struct iwl_priv *priv,
				struct iwl_tx_queue *txq);
extern unsigned int iwl3945_hw_get_beacon_cmd(struct iwl_priv *priv,
				 struct iwl3945_frame *frame, u8 rate);
void iwl3945_hw_build_tx_cmd_rate(struct iwl_priv *priv,
				  struct iwl_device_cmd *cmd,
				  struct ieee80211_tx_info *info,
				  struct ieee80211_hdr *hdr,
				  int sta_id, int tx_id);
extern int iwl3945_hw_reg_send_txpower(struct iwl_priv *priv);
extern int iwl3945_hw_reg_set_txpower(struct iwl_priv *priv, s8 power);
extern void iwl3945_hw_rx_statistics(struct iwl_priv *priv,
				 struct iwl_rx_mem_buffer *rxb);
extern void iwl3945_disable_events(struct iwl_priv *priv);
extern int iwl4965_get_temperature(const struct iwl_priv *priv);
extern void iwl3945_post_associate(struct iwl_priv *priv);
extern void iwl3945_config_ap(struct iwl_priv *priv);


extern u8 iwl3945_hw_find_station(struct iwl_priv *priv, const u8 *bssid);

extern int iwl3945_hw_channel_switch(struct iwl_priv *priv, u16 channel);


extern __le32 iwl3945_get_antenna_flags(const struct iwl_priv *priv);
extern int iwl3945_init_hw_rate_table(struct iwl_priv *priv);
extern void iwl3945_reg_txpower_periodic(struct iwl_priv *priv);
extern int iwl3945_txpower_set_from_eeprom(struct iwl_priv *priv);
extern u8 iwl3945_sync_sta(struct iwl_priv *priv, int sta_id,
		 u16 tx_rate, u8 flags);

extern const struct iwl_channel_info *iwl3945_get_channel_info(
	const struct iwl_priv *priv, enum ieee80211_band band, u16 channel);

extern int iwl3945_rs_next_rate(struct iwl_priv *priv, int rate);


#include "iwl-io.h"

#endif
