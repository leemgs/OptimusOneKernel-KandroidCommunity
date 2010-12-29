

#ifndef __iwl_debug_h__
#define __iwl_debug_h__

struct iwl_priv;
extern u32 iwl_debug_level;

#define IWL_ERR(p, f, a...) dev_err(&((p)->pci_dev->dev), f, ## a)
#define IWL_WARN(p, f, a...) dev_warn(&((p)->pci_dev->dev), f, ## a)
#define IWL_INFO(p, f, a...) dev_info(&((p)->pci_dev->dev), f, ## a)
#define IWL_CRIT(p, f, a...) dev_crit(&((p)->pci_dev->dev), f, ## a)

#define iwl_print_hex_error(priv, p, len) 				\
do {									\
	print_hex_dump(KERN_ERR, "iwl data: ",				\
		       DUMP_PREFIX_OFFSET, 16, 1, p, len, 1);		\
} while (0)

#ifdef CONFIG_IWLWIFI_DEBUG
#define IWL_DEBUG(__priv, level, fmt, args...)				\
do {									\
	if (iwl_get_debug_level(__priv) & (level))					\
		dev_printk(KERN_ERR, &(__priv->hw->wiphy->dev),		\
			 "%c %s " fmt, in_interrupt() ? 'I' : 'U',	\
			__func__ , ## args);				\
} while (0)

#define IWL_DEBUG_LIMIT(__priv, level, fmt, args...)			\
do {									\
	if ((iwl_get_debug_level(__priv) & (level)) && net_ratelimit())		\
		dev_printk(KERN_ERR, &(__priv->hw->wiphy->dev),		\
			"%c %s " fmt, in_interrupt() ? 'I' : 'U',	\
			 __func__ , ## args);				\
} while (0)

#define iwl_print_hex_dump(priv, level, p, len) 			\
do {                                            			\
	if (iwl_get_debug_level(priv) & level) 				\
		print_hex_dump(KERN_DEBUG, "iwl data: ",		\
			       DUMP_PREFIX_OFFSET, 16, 1, p, len, 1);	\
} while (0)

#ifdef CONFIG_IWLWIFI_DEBUGFS
struct iwl_debugfs {
	const char *name;
	struct dentry *dir_drv;
	struct dentry *dir_data;
	struct dentry *dir_debug;
	struct dentry *dir_rf;
	struct dir_data_files {
		struct dentry *file_sram;
		struct dentry *file_nvm;
		struct dentry *file_stations;
		struct dentry *file_log_event;
		struct dentry *file_channels;
		struct dentry *file_status;
		struct dentry *file_interrupt;
		struct dentry *file_qos;
		struct dentry *file_thermal_throttling;
#ifdef CONFIG_IWLWIFI_LEDS
		struct dentry *file_led;
#endif
		struct dentry *file_disable_ht40;
		struct dentry *file_sleep_level_override;
		struct dentry *file_current_sleep_command;
	} dbgfs_data_files;
	struct dir_rf_files {
		struct dentry *file_disable_sensitivity;
		struct dentry *file_disable_chain_noise;
		struct dentry *file_disable_tx_power;
	} dbgfs_rf_files;
	struct dir_debug_files {
		struct dentry *file_rx_statistics;
		struct dentry *file_tx_statistics;
		struct dentry *file_traffic_log;
		struct dentry *file_rx_queue;
		struct dentry *file_tx_queue;
		struct dentry *file_ucode_rx_stats;
		struct dentry *file_ucode_tx_stats;
		struct dentry *file_ucode_general_stats;
		struct dentry *file_sensitivity;
		struct dentry *file_chain_noise;
		struct dentry *file_tx_power;
	} dbgfs_debug_files;
	u32 sram_offset;
	u32 sram_len;
};

int iwl_dbgfs_register(struct iwl_priv *priv, const char *name);
void iwl_dbgfs_unregister(struct iwl_priv *priv);
#endif

#else
#define IWL_DEBUG(__priv, level, fmt, args...)
#define IWL_DEBUG_LIMIT(__priv, level, fmt, args...)
static inline void iwl_print_hex_dump(struct iwl_priv *priv, int level,
				      void *p, u32 len)
{}
#endif				



#ifndef CONFIG_IWLWIFI_DEBUGFS
static inline int iwl_dbgfs_register(struct iwl_priv *priv, const char *name)
{
	return 0;
}
static inline void iwl_dbgfs_unregister(struct iwl_priv *priv)
{
}
#endif				




#define IWL_DL_INFO		(1 << 0)
#define IWL_DL_MAC80211		(1 << 1)
#define IWL_DL_HCMD		(1 << 2)
#define IWL_DL_STATE		(1 << 3)

#define IWL_DL_MACDUMP		(1 << 4)
#define IWL_DL_HCMD_DUMP	(1 << 5)
#define IWL_DL_RADIO		(1 << 7)

#define IWL_DL_POWER		(1 << 8)
#define IWL_DL_TEMP		(1 << 9)
#define IWL_DL_NOTIF		(1 << 10)
#define IWL_DL_SCAN		(1 << 11)

#define IWL_DL_ASSOC		(1 << 12)
#define IWL_DL_DROP		(1 << 13)
#define IWL_DL_TXPOWER		(1 << 14)
#define IWL_DL_AP		(1 << 15)

#define IWL_DL_FW		(1 << 16)
#define IWL_DL_RF_KILL		(1 << 17)
#define IWL_DL_FW_ERRORS	(1 << 18)
#define IWL_DL_LED		(1 << 19)

#define IWL_DL_RATE		(1 << 20)
#define IWL_DL_CALIB		(1 << 21)
#define IWL_DL_WEP		(1 << 22)
#define IWL_DL_TX		(1 << 23)

#define IWL_DL_RX		(1 << 24)
#define IWL_DL_ISR		(1 << 25)
#define IWL_DL_HT		(1 << 26)
#define IWL_DL_IO		(1 << 27)

#define IWL_DL_11H		(1 << 28)
#define IWL_DL_STATS		(1 << 29)
#define IWL_DL_TX_REPLY		(1 << 30)
#define IWL_DL_QOS		(1 << 31)

#define IWL_DEBUG_INFO(p, f, a...)	IWL_DEBUG(p, IWL_DL_INFO, f, ## a)
#define IWL_DEBUG_MAC80211(p, f, a...)	IWL_DEBUG(p, IWL_DL_MAC80211, f, ## a)
#define IWL_DEBUG_MACDUMP(p, f, a...)	IWL_DEBUG(p, IWL_DL_MACDUMP, f, ## a)
#define IWL_DEBUG_TEMP(p, f, a...)	IWL_DEBUG(p, IWL_DL_TEMP, f, ## a)
#define IWL_DEBUG_SCAN(p, f, a...)	IWL_DEBUG(p, IWL_DL_SCAN, f, ## a)
#define IWL_DEBUG_RX(p, f, a...)	IWL_DEBUG(p, IWL_DL_RX, f, ## a)
#define IWL_DEBUG_TX(p, f, a...)	IWL_DEBUG(p, IWL_DL_TX, f, ## a)
#define IWL_DEBUG_ISR(p, f, a...)	IWL_DEBUG(p, IWL_DL_ISR, f, ## a)
#define IWL_DEBUG_LED(p, f, a...)	IWL_DEBUG(p, IWL_DL_LED, f, ## a)
#define IWL_DEBUG_WEP(p, f, a...)	IWL_DEBUG(p, IWL_DL_WEP, f, ## a)
#define IWL_DEBUG_HC(p, f, a...)	IWL_DEBUG(p, IWL_DL_HCMD, f, ## a)
#define IWL_DEBUG_HC_DUMP(p, f, a...)	IWL_DEBUG(p, IWL_DL_HCMD_DUMP, f, ## a)
#define IWL_DEBUG_CALIB(p, f, a...)	IWL_DEBUG(p, IWL_DL_CALIB, f, ## a)
#define IWL_DEBUG_FW(p, f, a...)	IWL_DEBUG(p, IWL_DL_FW, f, ## a)
#define IWL_DEBUG_RF_KILL(p, f, a...)	IWL_DEBUG(p, IWL_DL_RF_KILL, f, ## a)
#define IWL_DEBUG_DROP(p, f, a...)	IWL_DEBUG(p, IWL_DL_DROP, f, ## a)
#define IWL_DEBUG_DROP_LIMIT(p, f, a...)	\
		IWL_DEBUG_LIMIT(p, IWL_DL_DROP, f, ## a)
#define IWL_DEBUG_AP(p, f, a...)	IWL_DEBUG(p, IWL_DL_AP, f, ## a)
#define IWL_DEBUG_TXPOWER(p, f, a...)	IWL_DEBUG(p, IWL_DL_TXPOWER, f, ## a)
#define IWL_DEBUG_IO(p, f, a...)	IWL_DEBUG(p, IWL_DL_IO, f, ## a)
#define IWL_DEBUG_RATE(p, f, a...)	IWL_DEBUG(p, IWL_DL_RATE, f, ## a)
#define IWL_DEBUG_RATE_LIMIT(p, f, a...)	\
		IWL_DEBUG_LIMIT(p, IWL_DL_RATE, f, ## a)
#define IWL_DEBUG_NOTIF(p, f, a...)	IWL_DEBUG(p, IWL_DL_NOTIF, f, ## a)
#define IWL_DEBUG_ASSOC(p, f, a...)	\
		IWL_DEBUG(p, IWL_DL_ASSOC | IWL_DL_INFO, f, ## a)
#define IWL_DEBUG_ASSOC_LIMIT(p, f, a...)	\
		IWL_DEBUG_LIMIT(p, IWL_DL_ASSOC | IWL_DL_INFO, f, ## a)
#define IWL_DEBUG_HT(p, f, a...)	IWL_DEBUG(p, IWL_DL_HT, f, ## a)
#define IWL_DEBUG_STATS(p, f, a...)	IWL_DEBUG(p, IWL_DL_STATS, f, ## a)
#define IWL_DEBUG_STATS_LIMIT(p, f, a...)	\
		IWL_DEBUG_LIMIT(p, IWL_DL_STATS, f, ## a)
#define IWL_DEBUG_TX_REPLY(p, f, a...)	IWL_DEBUG(p, IWL_DL_TX_REPLY, f, ## a)
#define IWL_DEBUG_TX_REPLY_LIMIT(p, f, a...) \
		IWL_DEBUG_LIMIT(p, IWL_DL_TX_REPLY, f, ## a)
#define IWL_DEBUG_QOS(p, f, a...)	IWL_DEBUG(p, IWL_DL_QOS, f, ## a)
#define IWL_DEBUG_RADIO(p, f, a...)	IWL_DEBUG(p, IWL_DL_RADIO, f, ## a)
#define IWL_DEBUG_POWER(p, f, a...)	IWL_DEBUG(p, IWL_DL_POWER, f, ## a)
#define IWL_DEBUG_11H(p, f, a...)	IWL_DEBUG(p, IWL_DL_11H, f, ## a)

#endif
