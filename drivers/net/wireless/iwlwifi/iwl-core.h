

#ifndef __iwl_core_h__
#define __iwl_core_h__


struct iwl_host_cmd;
struct iwl_cmd;


#define IWLWIFI_VERSION "1.3.27k"
#define DRV_COPYRIGHT	"Copyright(c) 2003-2009 Intel Corporation"
#define DRV_AUTHOR     "<ilw@linux.intel.com>"

#define IWL_PCI_DEVICE(dev, subdev, cfg) \
	.vendor = PCI_VENDOR_ID_INTEL,  .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = (subdev), \
	.driver_data = (kernel_ulong_t)&(cfg)

#define IWL_SKU_G       0x1
#define IWL_SKU_A       0x2
#define IWL_SKU_N       0x8

#define IWL_CMD(x) case x: return #x

struct iwl_hcmd_ops {
	int (*rxon_assoc)(struct iwl_priv *priv);
	int (*commit_rxon)(struct iwl_priv *priv);
	void (*set_rxon_chain)(struct iwl_priv *priv);
};

struct iwl_hcmd_utils_ops {
	u16 (*get_hcmd_size)(u8 cmd_id, u16 len);
	u16 (*build_addsta_hcmd)(const struct iwl_addsta_cmd *cmd, u8 *data);
	void (*gain_computation)(struct iwl_priv *priv,
			u32 *average_noise,
			u16 min_average_noise_antennat_i,
			u32 min_average_noise);
	void (*chain_noise_reset)(struct iwl_priv *priv);
	void (*rts_tx_cmd_flag)(struct ieee80211_tx_info *info,
			__le32 *tx_flags);
	int  (*calc_rssi)(struct iwl_priv *priv,
			  struct iwl_rx_phy_res *rx_resp);
};

struct iwl_apm_ops {
	int (*init)(struct iwl_priv *priv);
	int (*reset)(struct iwl_priv *priv);
	void (*stop)(struct iwl_priv *priv);
	void (*config)(struct iwl_priv *priv);
	int (*set_pwr_src)(struct iwl_priv *priv, enum iwl_pwr_src src);
};

struct iwl_temp_ops {
	void (*temperature)(struct iwl_priv *priv);
	void (*set_ct_kill)(struct iwl_priv *priv);
};

struct iwl_ucode_ops {
	u32 (*get_header_size)(u32);
	u32 (*get_build)(const struct iwl_ucode_header *, u32);
	u32 (*get_inst_size)(const struct iwl_ucode_header *, u32);
	u32 (*get_data_size)(const struct iwl_ucode_header *, u32);
	u32 (*get_init_size)(const struct iwl_ucode_header *, u32);
	u32 (*get_init_data_size)(const struct iwl_ucode_header *, u32);
	u32 (*get_boot_size)(const struct iwl_ucode_header *, u32);
	u8 * (*get_data)(const struct iwl_ucode_header *, u32);
};

struct iwl_lib_ops {
	
	int (*set_hw_params)(struct iwl_priv *priv);
	
	void (*txq_update_byte_cnt_tbl)(struct iwl_priv *priv,
					struct iwl_tx_queue *txq,
					u16 byte_cnt);
	void (*txq_inval_byte_cnt_tbl)(struct iwl_priv *priv,
				       struct iwl_tx_queue *txq);
	void (*txq_set_sched)(struct iwl_priv *priv, u32 mask);
	int (*txq_attach_buf_to_tfd)(struct iwl_priv *priv,
				     struct iwl_tx_queue *txq,
				     dma_addr_t addr,
				     u16 len, u8 reset, u8 pad);
	void (*txq_free_tfd)(struct iwl_priv *priv,
			     struct iwl_tx_queue *txq);
	int (*txq_init)(struct iwl_priv *priv,
			struct iwl_tx_queue *txq);
	
	int (*txq_agg_enable)(struct iwl_priv *priv, int txq_id, int tx_fifo,
			      int sta_id, int tid, u16 ssn_idx);
	int (*txq_agg_disable)(struct iwl_priv *priv, u16 txq_id, u16 ssn_idx,
			       u8 tx_fifo);
	
	void (*rx_handler_setup)(struct iwl_priv *priv);
	
	void (*setup_deferred_work)(struct iwl_priv *priv);
	
	void (*cancel_deferred_work)(struct iwl_priv *priv);
	
	void (*init_alive_start)(struct iwl_priv *priv);
	
	int (*alive_notify)(struct iwl_priv *priv);
	
	int (*is_valid_rtc_data_addr)(u32 addr);
	
	int (*load_ucode)(struct iwl_priv *priv);
	void (*dump_nic_event_log)(struct iwl_priv *priv);
	void (*dump_nic_error_log)(struct iwl_priv *priv);
	
	struct iwl_apm_ops apm_ops;

	
	int (*send_tx_power) (struct iwl_priv *priv);
	void (*update_chain_flags)(struct iwl_priv *priv);
	void (*post_associate) (struct iwl_priv *priv);
	void (*config_ap) (struct iwl_priv *priv);
	irqreturn_t (*isr) (int irq, void *data);

	
	struct iwl_eeprom_ops eeprom_ops;

	
	struct iwl_temp_ops temp_ops;
};

struct iwl_ops {
	const struct iwl_ucode_ops *ucode;
	const struct iwl_lib_ops *lib;
	const struct iwl_hcmd_ops *hcmd;
	const struct iwl_hcmd_utils_ops *utils;
};

struct iwl_mod_params {
	int sw_crypto;		
	int disable_hw_scan;	
	int num_of_queues;	
	int num_of_ampdu_queues;
	int disable_11n;	
	int amsdu_size_8K;	
	int antenna;  		
	int restart_fw;		
};


struct iwl_cfg {
	const char *name;
	const char *fw_name_pre;
	const unsigned int ucode_api_max;
	const unsigned int ucode_api_min;
	unsigned int sku;
	int eeprom_size;
	u16  eeprom_ver;
	u16  eeprom_calib_ver;
	const struct iwl_ops *ops;
	const struct iwl_mod_params *mod_params;
	u8   valid_tx_ant;
	u8   valid_rx_ant;
	bool need_pll_cfg;
	bool use_isr_legacy;
	enum iwl_pa_type pa_type;
	const u16 max_ll_items;
	const bool shadow_ram_support;
	const bool ht_greenfield_support;
	const bool broken_powersave;
	bool use_rts_for_ht;
};



struct ieee80211_hw *iwl_alloc_all(struct iwl_cfg *cfg,
		struct ieee80211_ops *hw_ops);
void iwl_hw_detect(struct iwl_priv *priv);
void iwl_reset_qos(struct iwl_priv *priv);
void iwl_activate_qos(struct iwl_priv *priv, u8 force);
int iwl_mac_conf_tx(struct ieee80211_hw *hw, u16 queue,
		    const struct ieee80211_tx_queue_params *params);
void iwl_set_rxon_hwcrypto(struct iwl_priv *priv, int hw_decrypt);
int iwl_check_rxon_cmd(struct iwl_priv *priv);
int iwl_full_rxon_required(struct iwl_priv *priv);
void iwl_set_rxon_chain(struct iwl_priv *priv);
int iwl_set_rxon_channel(struct iwl_priv *priv, struct ieee80211_channel *ch);
void iwl_set_rxon_ht(struct iwl_priv *priv, struct iwl_ht_info *ht_info);
u8 iwl_is_ht40_tx_allowed(struct iwl_priv *priv,
			 struct ieee80211_sta_ht_cap *sta_ht_inf);
void iwl_set_flags_for_band(struct iwl_priv *priv, enum ieee80211_band band);
void iwl_connection_init_rx_config(struct iwl_priv *priv, int mode);
int iwl_set_decrypted_flag(struct iwl_priv *priv,
			   struct ieee80211_hdr *hdr,
			   u32 decrypt_res,
			   struct ieee80211_rx_status *stats);
void iwl_irq_handle_error(struct iwl_priv *priv);
void iwl_configure_filter(struct ieee80211_hw *hw,
			  unsigned int changed_flags,
			  unsigned int *total_flags, u64 multicast);
int iwl_hw_nic_init(struct iwl_priv *priv);
int iwl_setup_mac(struct iwl_priv *priv);
int iwl_set_hw_params(struct iwl_priv *priv);
int iwl_init_drv(struct iwl_priv *priv);
void iwl_uninit_drv(struct iwl_priv *priv);
bool iwl_is_monitor_mode(struct iwl_priv *priv);
void iwl_post_associate(struct iwl_priv *priv);
void iwl_bss_info_changed(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct ieee80211_bss_conf *bss_conf,
				     u32 changes);
int iwl_mac_beacon_update(struct ieee80211_hw *hw, struct sk_buff *skb);
int iwl_commit_rxon(struct iwl_priv *priv);
int iwl_set_mode(struct iwl_priv *priv, int mode);
int iwl_mac_add_interface(struct ieee80211_hw *hw,
				 struct ieee80211_if_init_conf *conf);
void iwl_mac_remove_interface(struct ieee80211_hw *hw,
				 struct ieee80211_if_init_conf *conf);
int iwl_mac_config(struct ieee80211_hw *hw, u32 changed);
void iwl_config_ap(struct iwl_priv *priv);
int iwl_mac_get_tx_stats(struct ieee80211_hw *hw,
			 struct ieee80211_tx_queue_stats *stats);
void iwl_mac_reset_tsf(struct ieee80211_hw *hw);
#ifdef CONFIG_IWLWIFI_DEBUGFS
int iwl_alloc_traffic_mem(struct iwl_priv *priv);
void iwl_free_traffic_mem(struct iwl_priv *priv);
void iwl_reset_traffic_log(struct iwl_priv *priv);
void iwl_dbg_log_tx_data_frame(struct iwl_priv *priv,
				u16 length, struct ieee80211_hdr *header);
void iwl_dbg_log_rx_data_frame(struct iwl_priv *priv,
				u16 length, struct ieee80211_hdr *header);
const char *get_mgmt_string(int cmd);
const char *get_ctrl_string(int cmd);
void iwl_clear_tx_stats(struct iwl_priv *priv);
void iwl_clear_rx_stats(struct iwl_priv *priv);
void iwl_update_stats(struct iwl_priv *priv, bool is_tx, __le16 fc,
		      u16 len);
#else
static inline int iwl_alloc_traffic_mem(struct iwl_priv *priv)
{
	return 0;
}
static inline void iwl_free_traffic_mem(struct iwl_priv *priv)
{
}
static inline void iwl_reset_traffic_log(struct iwl_priv *priv)
{
}
static inline void iwl_dbg_log_tx_data_frame(struct iwl_priv *priv,
		      u16 length, struct ieee80211_hdr *header)
{
}
static inline void iwl_dbg_log_rx_data_frame(struct iwl_priv *priv,
		      u16 length, struct ieee80211_hdr *header)
{
}
static inline void iwl_update_stats(struct iwl_priv *priv, bool is_tx,
				    __le16 fc, u16 len)
{
	struct traffic_stats	*stats;

	if (is_tx)
		stats = &priv->tx_stats;
	else
		stats = &priv->rx_stats;

	if (ieee80211_is_data(fc)) {
		
		stats->data_bytes += len;
	}
}
#endif

void iwl_rx_pm_sleep_notif(struct iwl_priv *priv,
			   struct iwl_rx_mem_buffer *rxb);
void iwl_rx_pm_debug_statistics_notif(struct iwl_priv *priv,
				      struct iwl_rx_mem_buffer *rxb);
void iwl_rx_reply_error(struct iwl_priv *priv,
			struct iwl_rx_mem_buffer *rxb);


void iwl_rx_queue_free(struct iwl_priv *priv, struct iwl_rx_queue *rxq);
void iwl_cmd_queue_free(struct iwl_priv *priv);
int iwl_rx_queue_alloc(struct iwl_priv *priv);
void iwl_rx_handle(struct iwl_priv *priv);
int iwl_rx_queue_update_write_ptr(struct iwl_priv *priv,
				  struct iwl_rx_queue *q);
void iwl_rx_queue_reset(struct iwl_priv *priv, struct iwl_rx_queue *rxq);
void iwl_rx_replenish(struct iwl_priv *priv);
void iwl_rx_replenish_now(struct iwl_priv *priv);
int iwl_rx_init(struct iwl_priv *priv, struct iwl_rx_queue *rxq);
int iwl_rx_queue_restock(struct iwl_priv *priv);
int iwl_rx_queue_space(const struct iwl_rx_queue *q);
void iwl_rx_allocate(struct iwl_priv *priv, gfp_t priority);
void iwl_tx_cmd_complete(struct iwl_priv *priv, struct iwl_rx_mem_buffer *rxb);
int iwl_tx_queue_reclaim(struct iwl_priv *priv, int txq_id, int index);

void iwl_rx_missed_beacon_notif(struct iwl_priv *priv,
			       struct iwl_rx_mem_buffer *rxb);
void iwl_rx_statistics(struct iwl_priv *priv,
			      struct iwl_rx_mem_buffer *rxb);
void iwl_rx_csa(struct iwl_priv *priv, struct iwl_rx_mem_buffer *rxb);




int iwl_txq_ctx_reset(struct iwl_priv *priv);
void iwl_hw_txq_free_tfd(struct iwl_priv *priv, struct iwl_tx_queue *txq);
int iwl_hw_txq_attach_buf_to_tfd(struct iwl_priv *priv,
				 struct iwl_tx_queue *txq,
				 dma_addr_t addr, u16 len, u8 reset, u8 pad);
int iwl_tx_skb(struct iwl_priv *priv, struct sk_buff *skb);
void iwl_hw_txq_ctx_free(struct iwl_priv *priv);
int iwl_hw_tx_queue_init(struct iwl_priv *priv,
			 struct iwl_tx_queue *txq);
int iwl_txq_update_write_ptr(struct iwl_priv *priv, struct iwl_tx_queue *txq);
int iwl_tx_queue_init(struct iwl_priv *priv, struct iwl_tx_queue *txq,
		      int slots_num, u32 txq_id);
void iwl_tx_queue_free(struct iwl_priv *priv, int txq_id);
int iwl_tx_agg_start(struct iwl_priv *priv, const u8 *ra, u16 tid, u16 *ssn);
int iwl_tx_agg_stop(struct iwl_priv *priv , const u8 *ra, u16 tid);
int iwl_txq_check_empty(struct iwl_priv *priv, int sta_id, u8 tid, int txq_id);

int iwl_set_tx_power(struct iwl_priv *priv, s8 tx_power, bool force);



void iwl_hwrate_to_tx_control(struct iwl_priv *priv, u32 rate_n_flags,
			      struct ieee80211_tx_info *info);
int iwl_hwrate_to_plcp_idx(u32 rate_n_flags);
int iwl_hwrate_to_mac80211_idx(u32 rate_n_flags, enum ieee80211_band band);

u8 iwl_rate_get_lowest_plcp(struct iwl_priv *priv);

u8 iwl_toggle_tx_ant(struct iwl_priv *priv, u8 ant_idx);

static inline u32 iwl_ant_idx_to_flags(u8 ant_idx)
{
	return BIT(ant_idx) << RATE_MCS_ANT_POS;
}

static inline u8 iwl_hw_get_rate(__le32 rate_n_flags)
{
	return le32_to_cpu(rate_n_flags) & 0xFF;
}
static inline u32 iwl_hw_get_rate_n_flags(__le32 rate_n_flags)
{
	return le32_to_cpu(rate_n_flags) & 0x1FFFF;
}
static inline __le32 iwl_hw_set_rate_n_flags(u8 rate, u32 flags)
{
	return cpu_to_le32(flags|(u32)rate);
}


void iwl_init_scan_params(struct iwl_priv *priv);
int iwl_scan_cancel(struct iwl_priv *priv);
int iwl_scan_cancel_timeout(struct iwl_priv *priv, unsigned long ms);
int iwl_mac_hw_scan(struct ieee80211_hw *hw, struct cfg80211_scan_request *req);
u16 iwl_fill_probe_req(struct iwl_priv *priv, struct ieee80211_mgmt *frame,
		       const u8 *ie, int ie_len, int left);
void iwl_setup_rx_scan_handlers(struct iwl_priv *priv);
u16 iwl_get_active_dwell_time(struct iwl_priv *priv,
			      enum ieee80211_band band,
			      u8 n_probes);
u16 iwl_get_passive_dwell_time(struct iwl_priv *priv,
			       enum ieee80211_band band);
void iwl_bg_scan_check(struct work_struct *data);
void iwl_bg_abort_scan(struct work_struct *work);
void iwl_bg_scan_completed(struct work_struct *work);
void iwl_setup_scan_deferred_work(struct iwl_priv *priv);


#define IWL_ACTIVE_QUIET_TIME       cpu_to_le16(10)  
#define IWL_PLCP_QUIET_THRESH       cpu_to_le16(1)  



int iwl_send_calib_results(struct iwl_priv *priv);
int iwl_calib_set(struct iwl_calib_result *res, const u8 *buf, int len);
void iwl_calib_free_results(struct iwl_priv *priv);


#ifdef CONFIG_IWLWIFI_SPECTRUM_MEASUREMENT
void iwl_setup_spectrum_handlers(struct iwl_priv *priv);
#else
static inline void iwl_setup_spectrum_handlers(struct iwl_priv *priv) {}
#endif


const char *get_cmd_string(u8 cmd);
int __must_check iwl_send_cmd_sync(struct iwl_priv *priv,
				   struct iwl_host_cmd *cmd);
int iwl_send_cmd(struct iwl_priv *priv, struct iwl_host_cmd *cmd);
int __must_check iwl_send_cmd_pdu(struct iwl_priv *priv, u8 id,
				  u16 len, const void *data);
int iwl_send_cmd_pdu_async(struct iwl_priv *priv, u8 id, u16 len,
			   const void *data,
			   void (*callback)(struct iwl_priv *priv,
					    struct iwl_device_cmd *cmd,
					    struct sk_buff *skb));

int iwl_enqueue_hcmd(struct iwl_priv *priv, struct iwl_host_cmd *cmd);

int iwl_send_card_state(struct iwl_priv *priv, u32 flags,
			u8 meta_flag);


irqreturn_t iwl_isr_legacy(int irq, void *data);
int iwl_reset_ict(struct iwl_priv *priv);
void iwl_disable_ict(struct iwl_priv *priv);
int iwl_alloc_isr_ict(struct iwl_priv *priv);
void iwl_free_isr_ict(struct iwl_priv *priv);
irqreturn_t iwl_isr_ict(int irq, void *data);

static inline u16 iwl_pcie_link_ctl(struct iwl_priv *priv)
{
	int pos;
	u16 pci_lnk_ctl;
	pos = pci_find_capability(priv->pci_dev, PCI_CAP_ID_EXP);
	pci_read_config_word(priv->pci_dev, pos + PCI_EXP_LNKCTL, &pci_lnk_ctl);
	return pci_lnk_ctl;
}
#ifdef CONFIG_PM
int iwl_pci_suspend(struct pci_dev *pdev, pm_message_t state);
int iwl_pci_resume(struct pci_dev *pdev);
#endif 


#ifdef CONFIG_IWLWIFI_DEBUG
void iwl_dump_nic_event_log(struct iwl_priv *priv);
void iwl_dump_nic_error_log(struct iwl_priv *priv);
#else
static inline void iwl_dump_nic_event_log(struct iwl_priv *priv)
{
}

static inline void iwl_dump_nic_error_log(struct iwl_priv *priv)
{
}
#endif

void iwl_clear_isr_stats(struct iwl_priv *priv);


int iwlcore_init_geos(struct iwl_priv *priv);
void iwlcore_free_geos(struct iwl_priv *priv);



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
#define STATUS_MODE_PENDING	18


static inline int iwl_is_ready(struct iwl_priv *priv)
{
	
	return test_bit(STATUS_READY, &priv->status) &&
	       test_bit(STATUS_GEO_CONFIGURED, &priv->status) &&
	       !test_bit(STATUS_EXIT_PENDING, &priv->status);
}

static inline int iwl_is_alive(struct iwl_priv *priv)
{
	return test_bit(STATUS_ALIVE, &priv->status);
}

static inline int iwl_is_init(struct iwl_priv *priv)
{
	return test_bit(STATUS_INIT, &priv->status);
}

static inline int iwl_is_rfkill_hw(struct iwl_priv *priv)
{
	return test_bit(STATUS_RF_KILL_HW, &priv->status);
}

static inline int iwl_is_rfkill(struct iwl_priv *priv)
{
	return iwl_is_rfkill_hw(priv);
}

static inline int iwl_is_ready_rf(struct iwl_priv *priv)
{

	if (iwl_is_rfkill(priv))
		return 0;

	return iwl_is_ready(priv);
}

extern void iwl_rf_kill_ct_config(struct iwl_priv *priv);
extern int iwl_send_bt_config(struct iwl_priv *priv);
extern int iwl_send_statistics_request(struct iwl_priv *priv, u8 flags);
extern int iwl_verify_ucode(struct iwl_priv *priv);
extern int iwl_send_lq_cmd(struct iwl_priv *priv,
		struct iwl_link_quality_cmd *lq, u8 flags);
extern void iwl_rx_reply_rx(struct iwl_priv *priv,
		struct iwl_rx_mem_buffer *rxb);
extern void iwl_rx_reply_rx_phy(struct iwl_priv *priv,
				    struct iwl_rx_mem_buffer *rxb);
void iwl_rx_reply_compressed_ba(struct iwl_priv *priv,
					   struct iwl_rx_mem_buffer *rxb);

void iwl_setup_rxon_timing(struct iwl_priv *priv);
static inline int iwl_send_rxon_assoc(struct iwl_priv *priv)
{
	return priv->cfg->ops->hcmd->rxon_assoc(priv);
}
static inline int iwlcore_commit_rxon(struct iwl_priv *priv)
{
	return priv->cfg->ops->hcmd->commit_rxon(priv);
}
static inline void iwlcore_config_ap(struct iwl_priv *priv)
{
	priv->cfg->ops->lib->config_ap(priv);
}
static inline const struct ieee80211_supported_band *iwl_get_hw_mode(
			struct iwl_priv *priv, enum ieee80211_band band)
{
	return priv->hw->wiphy->bands[band];
}

#endif 
