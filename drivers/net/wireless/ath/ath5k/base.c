

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hardirq.h>
#include <linux/if.h>
#include <linux/io.h>
#include <linux/netdevice.h>
#include <linux/cache.h>
#include <linux/pci.h>
#include <linux/ethtool.h>
#include <linux/uaccess.h>

#include <net/ieee80211_radiotap.h>

#include <asm/unaligned.h>

#include "base.h"
#include "reg.h"
#include "debug.h"

static u8 ath5k_calinterval = 10; 
static int modparam_nohwcrypt;
module_param_named(nohwcrypt, modparam_nohwcrypt, bool, S_IRUGO);
MODULE_PARM_DESC(nohwcrypt, "Disable hardware encryption.");

static int modparam_all_channels;
module_param_named(all_channels, modparam_all_channels, bool, S_IRUGO);
MODULE_PARM_DESC(all_channels, "Expose all channels the device can use.");





MODULE_AUTHOR("Jiri Slaby");
MODULE_AUTHOR("Nick Kossifidis");
MODULE_DESCRIPTION("Support for 5xxx series of Atheros 802.11 wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("Atheros 5xxx WLAN cards");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.6.0 (EXPERIMENTAL)");



static const struct pci_device_id ath5k_pci_id_table[] = {
	{ PCI_VDEVICE(ATHEROS, 0x0207) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0007) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0011) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0012) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0013) }, 
	{ PCI_VDEVICE(3COM_2,  0x0013) }, 
	{ PCI_VDEVICE(3COM,    0x0013) }, 
	{ PCI_VDEVICE(ATHEROS, 0x1014) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0014) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0015) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0016) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0017) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0018) }, 
	{ PCI_VDEVICE(ATHEROS, 0x0019) }, 
	{ PCI_VDEVICE(ATHEROS, 0x001a) }, 
	{ PCI_VDEVICE(ATHEROS, 0x001b) }, 
	{ PCI_VDEVICE(ATHEROS, 0x001c) }, 
	{ PCI_VDEVICE(ATHEROS, 0x001d) }, 
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, ath5k_pci_id_table);


static const struct ath5k_srev_name srev_names[] = {
	{ "5210",	AR5K_VERSION_MAC,	AR5K_SREV_AR5210 },
	{ "5311",	AR5K_VERSION_MAC,	AR5K_SREV_AR5311 },
	{ "5311A",	AR5K_VERSION_MAC,	AR5K_SREV_AR5311A },
	{ "5311B",	AR5K_VERSION_MAC,	AR5K_SREV_AR5311B },
	{ "5211",	AR5K_VERSION_MAC,	AR5K_SREV_AR5211 },
	{ "5212",	AR5K_VERSION_MAC,	AR5K_SREV_AR5212 },
	{ "5213",	AR5K_VERSION_MAC,	AR5K_SREV_AR5213 },
	{ "5213A",	AR5K_VERSION_MAC,	AR5K_SREV_AR5213A },
	{ "2413",	AR5K_VERSION_MAC,	AR5K_SREV_AR2413 },
	{ "2414",	AR5K_VERSION_MAC,	AR5K_SREV_AR2414 },
	{ "5424",	AR5K_VERSION_MAC,	AR5K_SREV_AR5424 },
	{ "5413",	AR5K_VERSION_MAC,	AR5K_SREV_AR5413 },
	{ "5414",	AR5K_VERSION_MAC,	AR5K_SREV_AR5414 },
	{ "2415",	AR5K_VERSION_MAC,	AR5K_SREV_AR2415 },
	{ "5416",	AR5K_VERSION_MAC,	AR5K_SREV_AR5416 },
	{ "5418",	AR5K_VERSION_MAC,	AR5K_SREV_AR5418 },
	{ "2425",	AR5K_VERSION_MAC,	AR5K_SREV_AR2425 },
	{ "2417",	AR5K_VERSION_MAC,	AR5K_SREV_AR2417 },
	{ "xxxxx",	AR5K_VERSION_MAC,	AR5K_SREV_UNKNOWN },
	{ "5110",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5110 },
	{ "5111",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5111 },
	{ "5111A",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5111A },
	{ "2111",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2111 },
	{ "5112",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5112 },
	{ "5112A",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5112A },
	{ "5112B",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5112B },
	{ "2112",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2112 },
	{ "2112A",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2112A },
	{ "2112B",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2112B },
	{ "2413",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2413 },
	{ "5413",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5413 },
	{ "2316",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2316 },
	{ "2317",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_2317 },
	{ "5424",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5424 },
	{ "5133",	AR5K_VERSION_RAD,	AR5K_SREV_RAD_5133 },
	{ "xxxxx",	AR5K_VERSION_RAD,	AR5K_SREV_UNKNOWN },
};

static const struct ieee80211_rate ath5k_rates[] = {
	{ .bitrate = 10,
	  .hw_value = ATH5K_RATE_CODE_1M, },
	{ .bitrate = 20,
	  .hw_value = ATH5K_RATE_CODE_2M,
	  .hw_value_short = ATH5K_RATE_CODE_2M | AR5K_SET_SHORT_PREAMBLE,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 55,
	  .hw_value = ATH5K_RATE_CODE_5_5M,
	  .hw_value_short = ATH5K_RATE_CODE_5_5M | AR5K_SET_SHORT_PREAMBLE,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 110,
	  .hw_value = ATH5K_RATE_CODE_11M,
	  .hw_value_short = ATH5K_RATE_CODE_11M | AR5K_SET_SHORT_PREAMBLE,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 60,
	  .hw_value = ATH5K_RATE_CODE_6M,
	  .flags = 0 },
	{ .bitrate = 90,
	  .hw_value = ATH5K_RATE_CODE_9M,
	  .flags = 0 },
	{ .bitrate = 120,
	  .hw_value = ATH5K_RATE_CODE_12M,
	  .flags = 0 },
	{ .bitrate = 180,
	  .hw_value = ATH5K_RATE_CODE_18M,
	  .flags = 0 },
	{ .bitrate = 240,
	  .hw_value = ATH5K_RATE_CODE_24M,
	  .flags = 0 },
	{ .bitrate = 360,
	  .hw_value = ATH5K_RATE_CODE_36M,
	  .flags = 0 },
	{ .bitrate = 480,
	  .hw_value = ATH5K_RATE_CODE_48M,
	  .flags = 0 },
	{ .bitrate = 540,
	  .hw_value = ATH5K_RATE_CODE_54M,
	  .flags = 0 },
	
};


static int __devinit	ath5k_pci_probe(struct pci_dev *pdev,
				const struct pci_device_id *id);
static void __devexit	ath5k_pci_remove(struct pci_dev *pdev);
#ifdef CONFIG_PM
static int		ath5k_pci_suspend(struct pci_dev *pdev,
					pm_message_t state);
static int		ath5k_pci_resume(struct pci_dev *pdev);
#else
#define ath5k_pci_suspend NULL
#define ath5k_pci_resume NULL
#endif 

static struct pci_driver ath5k_pci_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= ath5k_pci_id_table,
	.probe		= ath5k_pci_probe,
	.remove		= __devexit_p(ath5k_pci_remove),
	.suspend	= ath5k_pci_suspend,
	.resume		= ath5k_pci_resume,
};




static int ath5k_tx(struct ieee80211_hw *hw, struct sk_buff *skb);
static int ath5k_tx_queue(struct ieee80211_hw *hw, struct sk_buff *skb,
		struct ath5k_txq *txq);
static int ath5k_reset(struct ath5k_softc *sc, struct ieee80211_channel *chan);
static int ath5k_reset_wake(struct ath5k_softc *sc);
static int ath5k_start(struct ieee80211_hw *hw);
static void ath5k_stop(struct ieee80211_hw *hw);
static int ath5k_add_interface(struct ieee80211_hw *hw,
		struct ieee80211_if_init_conf *conf);
static void ath5k_remove_interface(struct ieee80211_hw *hw,
		struct ieee80211_if_init_conf *conf);
static int ath5k_config(struct ieee80211_hw *hw, u32 changed);
static u64 ath5k_prepare_multicast(struct ieee80211_hw *hw,
				   int mc_count, struct dev_addr_list *mc_list);
static void ath5k_configure_filter(struct ieee80211_hw *hw,
		unsigned int changed_flags,
		unsigned int *new_flags,
		u64 multicast);
static int ath5k_set_key(struct ieee80211_hw *hw,
		enum set_key_cmd cmd,
		struct ieee80211_vif *vif, struct ieee80211_sta *sta,
		struct ieee80211_key_conf *key);
static int ath5k_get_stats(struct ieee80211_hw *hw,
		struct ieee80211_low_level_stats *stats);
static int ath5k_get_tx_stats(struct ieee80211_hw *hw,
		struct ieee80211_tx_queue_stats *stats);
static u64 ath5k_get_tsf(struct ieee80211_hw *hw);
static void ath5k_set_tsf(struct ieee80211_hw *hw, u64 tsf);
static void ath5k_reset_tsf(struct ieee80211_hw *hw);
static int ath5k_beacon_update(struct ieee80211_hw *hw,
		struct ieee80211_vif *vif);
static void ath5k_bss_info_changed(struct ieee80211_hw *hw,
		struct ieee80211_vif *vif,
		struct ieee80211_bss_conf *bss_conf,
		u32 changes);
static void ath5k_sw_scan_start(struct ieee80211_hw *hw);
static void ath5k_sw_scan_complete(struct ieee80211_hw *hw);

static const struct ieee80211_ops ath5k_hw_ops = {
	.tx 		= ath5k_tx,
	.start 		= ath5k_start,
	.stop 		= ath5k_stop,
	.add_interface 	= ath5k_add_interface,
	.remove_interface = ath5k_remove_interface,
	.config 	= ath5k_config,
	.prepare_multicast = ath5k_prepare_multicast,
	.configure_filter = ath5k_configure_filter,
	.set_key 	= ath5k_set_key,
	.get_stats 	= ath5k_get_stats,
	.conf_tx 	= NULL,
	.get_tx_stats 	= ath5k_get_tx_stats,
	.get_tsf 	= ath5k_get_tsf,
	.set_tsf 	= ath5k_set_tsf,
	.reset_tsf 	= ath5k_reset_tsf,
	.bss_info_changed = ath5k_bss_info_changed,
	.sw_scan_start	= ath5k_sw_scan_start,
	.sw_scan_complete = ath5k_sw_scan_complete,
};



static int 	ath5k_attach(struct pci_dev *pdev,
			struct ieee80211_hw *hw);
static void 	ath5k_detach(struct pci_dev *pdev,
			struct ieee80211_hw *hw);

static inline short ath5k_ieee2mhz(short chan);
static unsigned int ath5k_copy_channels(struct ath5k_hw *ah,
				struct ieee80211_channel *channels,
				unsigned int mode,
				unsigned int max);
static int 	ath5k_setup_bands(struct ieee80211_hw *hw);
static int 	ath5k_chan_set(struct ath5k_softc *sc,
				struct ieee80211_channel *chan);
static void	ath5k_setcurmode(struct ath5k_softc *sc,
				unsigned int mode);
static void	ath5k_mode_setup(struct ath5k_softc *sc);


static int	ath5k_desc_alloc(struct ath5k_softc *sc,
				struct pci_dev *pdev);
static void	ath5k_desc_free(struct ath5k_softc *sc,
				struct pci_dev *pdev);

static int 	ath5k_rxbuf_setup(struct ath5k_softc *sc,
				struct ath5k_buf *bf);
static int 	ath5k_txbuf_setup(struct ath5k_softc *sc,
				struct ath5k_buf *bf,
				struct ath5k_txq *txq);
static inline void ath5k_txbuf_free(struct ath5k_softc *sc,
				struct ath5k_buf *bf)
{
	BUG_ON(!bf);
	if (!bf->skb)
		return;
	pci_unmap_single(sc->pdev, bf->skbaddr, bf->skb->len,
			PCI_DMA_TODEVICE);
	dev_kfree_skb_any(bf->skb);
	bf->skb = NULL;
}

static inline void ath5k_rxbuf_free(struct ath5k_softc *sc,
				struct ath5k_buf *bf)
{
	BUG_ON(!bf);
	if (!bf->skb)
		return;
	pci_unmap_single(sc->pdev, bf->skbaddr, sc->rxbufsize,
			PCI_DMA_FROMDEVICE);
	dev_kfree_skb_any(bf->skb);
	bf->skb = NULL;
}



static struct 	ath5k_txq *ath5k_txq_setup(struct ath5k_softc *sc,
				int qtype, int subtype);
static int 	ath5k_beaconq_setup(struct ath5k_hw *ah);
static int 	ath5k_beaconq_config(struct ath5k_softc *sc);
static void 	ath5k_txq_drainq(struct ath5k_softc *sc,
				struct ath5k_txq *txq);
static void 	ath5k_txq_cleanup(struct ath5k_softc *sc);
static void 	ath5k_txq_release(struct ath5k_softc *sc);

static int 	ath5k_rx_start(struct ath5k_softc *sc);
static void 	ath5k_rx_stop(struct ath5k_softc *sc);
static unsigned int ath5k_rx_decrypted(struct ath5k_softc *sc,
					struct ath5k_desc *ds,
					struct sk_buff *skb,
					struct ath5k_rx_status *rs);
static void 	ath5k_tasklet_rx(unsigned long data);

static void 	ath5k_tx_processq(struct ath5k_softc *sc,
				struct ath5k_txq *txq);
static void 	ath5k_tasklet_tx(unsigned long data);

static int 	ath5k_beacon_setup(struct ath5k_softc *sc,
					struct ath5k_buf *bf);
static void 	ath5k_beacon_send(struct ath5k_softc *sc);
static void 	ath5k_beacon_config(struct ath5k_softc *sc);
static void	ath5k_beacon_update_timers(struct ath5k_softc *sc, u64 bc_tsf);
static void	ath5k_tasklet_beacon(unsigned long data);

static inline u64 ath5k_extend_tsf(struct ath5k_hw *ah, u32 rstamp)
{
	u64 tsf = ath5k_hw_get_tsf64(ah);

	if ((tsf & 0x7fff) < rstamp)
		tsf -= 0x8000;

	return (tsf & ~0x7fff) | rstamp;
}


static int 	ath5k_init(struct ath5k_softc *sc);
static int 	ath5k_stop_locked(struct ath5k_softc *sc);
static int 	ath5k_stop_hw(struct ath5k_softc *sc);
static irqreturn_t ath5k_intr(int irq, void *dev_id);
static void 	ath5k_tasklet_reset(unsigned long data);

static void 	ath5k_tasklet_calibrate(unsigned long data);


static int __init
init_ath5k_pci(void)
{
	int ret;

	ath5k_debug_init();

	ret = pci_register_driver(&ath5k_pci_driver);
	if (ret) {
		printk(KERN_ERR "ath5k_pci: can't register pci driver\n");
		return ret;
	}

	return 0;
}

static void __exit
exit_ath5k_pci(void)
{
	pci_unregister_driver(&ath5k_pci_driver);

	ath5k_debug_finish();
}

module_init(init_ath5k_pci);
module_exit(exit_ath5k_pci);




static const char *
ath5k_chip_name(enum ath5k_srev_type type, u_int16_t val)
{
	const char *name = "xxxxx";
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(srev_names); i++) {
		if (srev_names[i].sr_type != type)
			continue;

		if ((val & 0xf0) == srev_names[i].sr_val)
			name = srev_names[i].sr_name;

		if ((val & 0xff) == srev_names[i].sr_val) {
			name = srev_names[i].sr_name;
			break;
		}
	}

	return name;
}

static int __devinit
ath5k_pci_probe(struct pci_dev *pdev,
		const struct pci_device_id *id)
{
	void __iomem *mem;
	struct ath5k_softc *sc;
	struct ieee80211_hw *hw;
	int ret;
	u8 csz;

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "can't enable device\n");
		goto err;
	}

	
	ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (ret) {
		dev_err(&pdev->dev, "32-bit DMA not available\n");
		goto err_dis;
	}

	
	pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &csz);
	if (csz == 0) {
		
		csz = L1_CACHE_BYTES >> 2;
		pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE, csz);
	}
	
	pci_write_config_byte(pdev, PCI_LATENCY_TIMER, 0xa8);

	
	pci_set_master(pdev);

	
	pci_write_config_byte(pdev, 0x41, 0);

	ret = pci_request_region(pdev, 0, "ath5k");
	if (ret) {
		dev_err(&pdev->dev, "cannot reserve PCI memory region\n");
		goto err_dis;
	}

	mem = pci_iomap(pdev, 0, 0);
	if (!mem) {
		dev_err(&pdev->dev, "cannot remap PCI memory region\n") ;
		ret = -EIO;
		goto err_reg;
	}

	
	hw = ieee80211_alloc_hw(sizeof(*sc), &ath5k_hw_ops);
	if (hw == NULL) {
		dev_err(&pdev->dev, "cannot allocate ieee80211_hw\n");
		ret = -ENOMEM;
		goto err_map;
	}

	dev_info(&pdev->dev, "registered as '%s'\n", wiphy_name(hw->wiphy));

	
	SET_IEEE80211_DEV(hw, &pdev->dev);
	hw->flags = IEEE80211_HW_RX_INCLUDES_FCS |
		    IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING |
		    IEEE80211_HW_SIGNAL_DBM |
		    IEEE80211_HW_NOISE_DBM;

	hw->wiphy->interface_modes =
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_ADHOC) |
		BIT(NL80211_IFTYPE_MESH_POINT);

	hw->extra_tx_headroom = 2;
	hw->channel_change_time = 5000;
	sc = hw->priv;
	sc->hw = hw;
	sc->pdev = pdev;

	ath5k_debug_init_device(sc);

	
	__set_bit(ATH_STAT_INVALID, sc->status);

	sc->iobase = mem; 
	sc->common.cachelsz = csz << 2; 
	sc->opmode = NL80211_IFTYPE_STATION;
	sc->bintval = 1000;
	mutex_init(&sc->lock);
	spin_lock_init(&sc->rxbuflock);
	spin_lock_init(&sc->txbuflock);
	spin_lock_init(&sc->block);

	
	pci_set_drvdata(pdev, hw);

	
	ret = request_irq(pdev->irq, ath5k_intr, IRQF_SHARED, "ath", sc);
	if (ret) {
		ATH5K_ERR(sc, "request_irq failed\n");
		goto err_free;
	}

	
	sc->ah = ath5k_hw_attach(sc);
	if (IS_ERR(sc->ah)) {
		ret = PTR_ERR(sc->ah);
		goto err_irq;
	}

	
	if (sc->ah->ah_version == AR5K_AR5212) {
		hw->max_rates = 4;
		hw->max_rate_tries = 11;
	}

	
	ret = ath5k_attach(pdev, hw);
	if (ret)
		goto err_ah;

	ATH5K_INFO(sc, "Atheros AR%s chip found (MAC: 0x%x, PHY: 0x%x)\n",
			ath5k_chip_name(AR5K_VERSION_MAC, sc->ah->ah_mac_srev),
					sc->ah->ah_mac_srev,
					sc->ah->ah_phy_revision);

	if (!sc->ah->ah_single_chip) {
		
		if (sc->ah->ah_radio_5ghz_revision &&
			!sc->ah->ah_radio_2ghz_revision) {
			
			if (!test_bit(AR5K_MODE_11A,
				sc->ah->ah_capabilities.cap_mode)) {
				ATH5K_INFO(sc, "RF%s 2GHz radio found (0x%x)\n",
					ath5k_chip_name(AR5K_VERSION_RAD,
						sc->ah->ah_radio_5ghz_revision),
						sc->ah->ah_radio_5ghz_revision);
			
			} else if (!test_bit(AR5K_MODE_11B,
				sc->ah->ah_capabilities.cap_mode)) {
				ATH5K_INFO(sc, "RF%s 5GHz radio found (0x%x)\n",
					ath5k_chip_name(AR5K_VERSION_RAD,
						sc->ah->ah_radio_5ghz_revision),
						sc->ah->ah_radio_5ghz_revision);
			
			} else {
				ATH5K_INFO(sc, "RF%s multiband radio found"
					" (0x%x)\n",
					ath5k_chip_name(AR5K_VERSION_RAD,
						sc->ah->ah_radio_5ghz_revision),
						sc->ah->ah_radio_5ghz_revision);
			}
		}
		
		else if (sc->ah->ah_radio_5ghz_revision &&
				sc->ah->ah_radio_2ghz_revision){
			ATH5K_INFO(sc, "RF%s 5GHz radio found (0x%x)\n",
				ath5k_chip_name(AR5K_VERSION_RAD,
					sc->ah->ah_radio_5ghz_revision),
					sc->ah->ah_radio_5ghz_revision);
			ATH5K_INFO(sc, "RF%s 2GHz radio found (0x%x)\n",
				ath5k_chip_name(AR5K_VERSION_RAD,
					sc->ah->ah_radio_2ghz_revision),
					sc->ah->ah_radio_2ghz_revision);
		}
	}


	
	__clear_bit(ATH_STAT_INVALID, sc->status);

	return 0;
err_ah:
	ath5k_hw_detach(sc->ah);
err_irq:
	free_irq(pdev->irq, sc);
err_free:
	ieee80211_free_hw(hw);
err_map:
	pci_iounmap(pdev, mem);
err_reg:
	pci_release_region(pdev, 0);
err_dis:
	pci_disable_device(pdev);
err:
	return ret;
}

static void __devexit
ath5k_pci_remove(struct pci_dev *pdev)
{
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct ath5k_softc *sc = hw->priv;

	ath5k_debug_finish_device(sc);
	ath5k_detach(pdev, hw);
	ath5k_hw_detach(sc->ah);
	free_irq(pdev->irq, sc);
	pci_iounmap(pdev, sc->iobase);
	pci_release_region(pdev, 0);
	pci_disable_device(pdev);
	ieee80211_free_hw(hw);
}

#ifdef CONFIG_PM
static int
ath5k_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct ath5k_softc *sc = hw->priv;

	ath5k_led_off(sc);

	pci_save_state(pdev);
	pci_disable_device(pdev);
	pci_set_power_state(pdev, PCI_D3hot);

	return 0;
}

static int
ath5k_pci_resume(struct pci_dev *pdev)
{
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct ath5k_softc *sc = hw->priv;
	int err;

	pci_restore_state(pdev);

	err = pci_enable_device(pdev);
	if (err)
		return err;

	
	pci_write_config_byte(pdev, 0x41, 0);

	ath5k_led_enable(sc);
	return 0;
}
#endif 




static int ath5k_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request)
{
	struct ieee80211_hw *hw = wiphy_to_ieee80211_hw(wiphy);
	struct ath5k_softc *sc = hw->priv;
	struct ath_regulatory *regulatory = &sc->common.regulatory;

	return ath_reg_notifier_apply(wiphy, request, regulatory);
}

static int
ath5k_attach(struct pci_dev *pdev, struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	struct ath_regulatory *regulatory = &sc->common.regulatory;
	u8 mac[ETH_ALEN] = {};
	int ret;

	ATH5K_DBG(sc, ATH5K_DEBUG_ANY, "devid 0x%x\n", pdev->device);

	
	ret = ah->ah_setup_mrr_tx_desc(ah, NULL, 0, 0, 0, 0, 0, 0);
	if (ret < 0)
		goto err;
	if (ret > 0)
		__set_bit(ATH_STAT_MRRETRY, sc->status);

	
	ret = ath5k_setup_bands(hw);
	if (ret) {
		ATH5K_ERR(sc, "can't get channels\n");
		goto err;
	}

	
	if (test_bit(AR5K_MODE_11A, ah->ah_modes))
		ath5k_setcurmode(sc, AR5K_MODE_11A);
	else
		ath5k_setcurmode(sc, AR5K_MODE_11B);

	
	ret = ath5k_desc_alloc(sc, pdev);
	if (ret) {
		ATH5K_ERR(sc, "can't allocate descriptors\n");
		goto err;
	}

	
	ret = ath5k_beaconq_setup(ah);
	if (ret < 0) {
		ATH5K_ERR(sc, "can't setup a beacon xmit queue\n");
		goto err_desc;
	}
	sc->bhalq = ret;
	sc->cabq = ath5k_txq_setup(sc, AR5K_TX_QUEUE_CAB, 0);
	if (IS_ERR(sc->cabq)) {
		ATH5K_ERR(sc, "can't setup cab queue\n");
		ret = PTR_ERR(sc->cabq);
		goto err_bhal;
	}

	sc->txq = ath5k_txq_setup(sc, AR5K_TX_QUEUE_DATA, AR5K_WME_AC_BK);
	if (IS_ERR(sc->txq)) {
		ATH5K_ERR(sc, "can't setup xmit queue\n");
		ret = PTR_ERR(sc->txq);
		goto err_queues;
	}

	tasklet_init(&sc->rxtq, ath5k_tasklet_rx, (unsigned long)sc);
	tasklet_init(&sc->txtq, ath5k_tasklet_tx, (unsigned long)sc);
	tasklet_init(&sc->restq, ath5k_tasklet_reset, (unsigned long)sc);
	tasklet_init(&sc->calib, ath5k_tasklet_calibrate, (unsigned long)sc);
	tasklet_init(&sc->beacontq, ath5k_tasklet_beacon, (unsigned long)sc);

	ret = ath5k_eeprom_read_mac(ah, mac);
	if (ret) {
		ATH5K_ERR(sc, "unable to read address from EEPROM: 0x%04x\n",
			sc->pdev->device);
		goto err_queues;
	}

	SET_IEEE80211_PERM_ADDR(hw, mac);
	
	memset(sc->bssidmask, 0xff, ETH_ALEN);
	ath5k_hw_set_bssid_mask(sc->ah, sc->bssidmask);

	regulatory->current_rd = ah->ah_capabilities.cap_eeprom.ee_regdomain;
	ret = ath_regd_init(regulatory, hw->wiphy, ath5k_reg_notifier);
	if (ret) {
		ATH5K_ERR(sc, "can't initialize regulatory system\n");
		goto err_queues;
	}

	ret = ieee80211_register_hw(hw);
	if (ret) {
		ATH5K_ERR(sc, "can't register ieee80211 hw\n");
		goto err_queues;
	}

	if (!ath_is_world_regd(regulatory))
		regulatory_hint(hw->wiphy, regulatory->alpha2);

	ath5k_init_leds(sc);

	return 0;
err_queues:
	ath5k_txq_release(sc);
err_bhal:
	ath5k_hw_release_tx_queue(ah, sc->bhalq);
err_desc:
	ath5k_desc_free(sc, pdev);
err:
	return ret;
}

static void
ath5k_detach(struct pci_dev *pdev, struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;

	
	ieee80211_unregister_hw(hw);
	ath5k_desc_free(sc, pdev);
	ath5k_txq_release(sc);
	ath5k_hw_release_tx_queue(sc->ah, sc->bhalq);
	ath5k_unregister_leds(sc);

	
}







static inline short
ath5k_ieee2mhz(short chan)
{
	if (chan <= 14 || chan >= 27)
		return ieee80211chan2mhz(chan);
	else
		return 2212 + chan * 20;
}


static bool ath5k_is_standard_channel(short chan)
{
	return ((chan <= 14) ||
		
		((chan & 3) == 0 && chan >= 36 && chan <= 64) ||
		
		((chan & 3) == 0 && chan >= 100 && chan <= 140) ||
		
		((chan & 3) == 1 && chan >= 149 && chan <= 165));
}

static unsigned int
ath5k_copy_channels(struct ath5k_hw *ah,
		struct ieee80211_channel *channels,
		unsigned int mode,
		unsigned int max)
{
	unsigned int i, count, size, chfreq, freq, ch;

	if (!test_bit(mode, ah->ah_modes))
		return 0;

	switch (mode) {
	case AR5K_MODE_11A:
	case AR5K_MODE_11A_TURBO:
		
		size = 220 ;
		chfreq = CHANNEL_5GHZ;
		break;
	case AR5K_MODE_11B:
	case AR5K_MODE_11G:
	case AR5K_MODE_11G_TURBO:
		size = 26;
		chfreq = CHANNEL_2GHZ;
		break;
	default:
		ATH5K_WARN(ah->ah_sc, "bad mode, not copying channels\n");
		return 0;
	}

	for (i = 0, count = 0; i < size && max > 0; i++) {
		ch = i + 1 ;
		freq = ath5k_ieee2mhz(ch);

		
		if (!ath5k_channel_ok(ah, freq, chfreq))
			continue;

		if (!modparam_all_channels && !ath5k_is_standard_channel(ch))
			continue;

		
		channels[count].center_freq = freq;
		channels[count].band = (chfreq == CHANNEL_2GHZ) ?
			IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
		switch (mode) {
		case AR5K_MODE_11A:
		case AR5K_MODE_11G:
			channels[count].hw_value = chfreq | CHANNEL_OFDM;
			break;
		case AR5K_MODE_11A_TURBO:
		case AR5K_MODE_11G_TURBO:
			channels[count].hw_value = chfreq |
				CHANNEL_OFDM | CHANNEL_TURBO;
			break;
		case AR5K_MODE_11B:
			channels[count].hw_value = CHANNEL_B;
		}

		count++;
		max--;
	}

	return count;
}

static void
ath5k_setup_rate_idx(struct ath5k_softc *sc, struct ieee80211_supported_band *b)
{
	u8 i;

	for (i = 0; i < AR5K_MAX_RATES; i++)
		sc->rate_idx[b->band][i] = -1;

	for (i = 0; i < b->n_bitrates; i++) {
		sc->rate_idx[b->band][b->bitrates[i].hw_value] = i;
		if (b->bitrates[i].hw_value_short)
			sc->rate_idx[b->band][b->bitrates[i].hw_value_short] = i;
	}
}

static int
ath5k_setup_bands(struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	struct ieee80211_supported_band *sband;
	int max_c, count_c = 0;
	int i;

	BUILD_BUG_ON(ARRAY_SIZE(sc->sbands) < IEEE80211_NUM_BANDS);
	max_c = ARRAY_SIZE(sc->channels);

	
	sband = &sc->sbands[IEEE80211_BAND_2GHZ];
	sband->band = IEEE80211_BAND_2GHZ;
	sband->bitrates = &sc->rates[IEEE80211_BAND_2GHZ][0];

	if (test_bit(AR5K_MODE_11G, sc->ah->ah_capabilities.cap_mode)) {
		
		memcpy(sband->bitrates, &ath5k_rates[0],
		       sizeof(struct ieee80211_rate) * 12);
		sband->n_bitrates = 12;

		sband->channels = sc->channels;
		sband->n_channels = ath5k_copy_channels(ah, sband->channels,
					AR5K_MODE_11G, max_c);

		hw->wiphy->bands[IEEE80211_BAND_2GHZ] = sband;
		count_c = sband->n_channels;
		max_c -= count_c;
	} else if (test_bit(AR5K_MODE_11B, sc->ah->ah_capabilities.cap_mode)) {
		
		memcpy(sband->bitrates, &ath5k_rates[0],
		       sizeof(struct ieee80211_rate) * 4);
		sband->n_bitrates = 4;

		
		if (ah->ah_version == AR5K_AR5211) {
			for (i = 0; i < 4; i++) {
				sband->bitrates[i].hw_value =
					sband->bitrates[i].hw_value & 0xF;
				sband->bitrates[i].hw_value_short =
					sband->bitrates[i].hw_value_short & 0xF;
			}
		}

		sband->channels = sc->channels;
		sband->n_channels = ath5k_copy_channels(ah, sband->channels,
					AR5K_MODE_11B, max_c);

		hw->wiphy->bands[IEEE80211_BAND_2GHZ] = sband;
		count_c = sband->n_channels;
		max_c -= count_c;
	}
	ath5k_setup_rate_idx(sc, sband);

	
	if (test_bit(AR5K_MODE_11A, sc->ah->ah_capabilities.cap_mode)) {
		sband = &sc->sbands[IEEE80211_BAND_5GHZ];
		sband->band = IEEE80211_BAND_5GHZ;
		sband->bitrates = &sc->rates[IEEE80211_BAND_5GHZ][0];

		memcpy(sband->bitrates, &ath5k_rates[4],
		       sizeof(struct ieee80211_rate) * 8);
		sband->n_bitrates = 8;

		sband->channels = &sc->channels[count_c];
		sband->n_channels = ath5k_copy_channels(ah, sband->channels,
					AR5K_MODE_11A, max_c);

		hw->wiphy->bands[IEEE80211_BAND_5GHZ] = sband;
	}
	ath5k_setup_rate_idx(sc, sband);

	ath5k_debug_dump_bands(sc);

	return 0;
}


static int
ath5k_chan_set(struct ath5k_softc *sc, struct ieee80211_channel *chan)
{
	ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "(%u MHz) -> (%u MHz)\n",
		sc->curchan->center_freq, chan->center_freq);

	
	return ath5k_reset(sc, chan);
}

static void
ath5k_setcurmode(struct ath5k_softc *sc, unsigned int mode)
{
	sc->curmode = mode;

	if (mode == AR5K_MODE_11A) {
		sc->curband = &sc->sbands[IEEE80211_BAND_5GHZ];
	} else {
		sc->curband = &sc->sbands[IEEE80211_BAND_2GHZ];
	}
}

static void
ath5k_mode_setup(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	u32 rfilt;

	ah->ah_op_mode = sc->opmode;

	
	rfilt = sc->filter_flags;
	ath5k_hw_set_rx_filter(ah, rfilt);

	if (ath5k_hw_hasbssidmask(ah))
		ath5k_hw_set_bssid_mask(ah, sc->bssidmask);

	
	ath5k_hw_set_opmode(ah);

	ATH5K_DBG(sc, ATH5K_DEBUG_MODE, "RX filter 0x%x\n", rfilt);
}

static inline int
ath5k_hw_to_driver_rix(struct ath5k_softc *sc, int hw_rix)
{
	int rix;

	
	if (WARN(hw_rix < 0 || hw_rix >= AR5K_MAX_RATES,
			"hw_rix out of bounds: %x\n", hw_rix))
		return 0;

	rix = sc->rate_idx[sc->curband->band][hw_rix];
	if (WARN(rix < 0, "invalid hw_rix: %x\n", hw_rix))
		rix = 0;

	return rix;
}



static
struct sk_buff *ath5k_rx_skb_alloc(struct ath5k_softc *sc, dma_addr_t *skb_addr)
{
	struct sk_buff *skb;

	
	skb = ath_rxbuf_alloc(&sc->common,
			      sc->rxbufsize + sc->common.cachelsz - 1,
			      GFP_ATOMIC);

	if (!skb) {
		ATH5K_ERR(sc, "can't alloc skbuff of size %u\n",
				sc->rxbufsize + sc->common.cachelsz - 1);
		return NULL;
	}

	*skb_addr = pci_map_single(sc->pdev,
		skb->data, sc->rxbufsize, PCI_DMA_FROMDEVICE);
	if (unlikely(pci_dma_mapping_error(sc->pdev, *skb_addr))) {
		ATH5K_ERR(sc, "%s: DMA mapping failed\n", __func__);
		dev_kfree_skb(skb);
		return NULL;
	}
	return skb;
}

static int
ath5k_rxbuf_setup(struct ath5k_softc *sc, struct ath5k_buf *bf)
{
	struct ath5k_hw *ah = sc->ah;
	struct sk_buff *skb = bf->skb;
	struct ath5k_desc *ds;

	if (!skb) {
		skb = ath5k_rx_skb_alloc(sc, &bf->skbaddr);
		if (!skb)
			return -ENOMEM;
		bf->skb = skb;
	}

	
	ds = bf->desc;
	ds->ds_link = bf->daddr;	
	ds->ds_data = bf->skbaddr;
	ah->ah_setup_rx_desc(ah, ds,
		skb_tailroom(skb),	
		0);

	if (sc->rxlink != NULL)
		*sc->rxlink = bf->daddr;
	sc->rxlink = &ds->ds_link;
	return 0;
}

static int
ath5k_txbuf_setup(struct ath5k_softc *sc, struct ath5k_buf *bf,
		  struct ath5k_txq *txq)
{
	struct ath5k_hw *ah = sc->ah;
	struct ath5k_desc *ds = bf->desc;
	struct sk_buff *skb = bf->skb;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	unsigned int pktlen, flags, keyidx = AR5K_TXKEYIX_INVALID;
	struct ieee80211_rate *rate;
	unsigned int mrr_rate[3], mrr_tries[3];
	int i, ret;
	u16 hw_rate;
	u16 cts_rate = 0;
	u16 duration = 0;
	u8 rc_flags;

	flags = AR5K_TXDESC_INTREQ | AR5K_TXDESC_CLRDMASK;

	
	bf->skbaddr = pci_map_single(sc->pdev, skb->data, skb->len,
			PCI_DMA_TODEVICE);

	rate = ieee80211_get_tx_rate(sc->hw, info);

	if (info->flags & IEEE80211_TX_CTL_NO_ACK)
		flags |= AR5K_TXDESC_NOACK;

	rc_flags = info->control.rates[0].flags;
	hw_rate = (rc_flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE) ?
		rate->hw_value_short : rate->hw_value;

	pktlen = skb->len;

	
	if (info->control.hw_key) {
		keyidx = info->control.hw_key->hw_key_idx;
		pktlen += info->control.hw_key->icv_len;
	}
	if (rc_flags & IEEE80211_TX_RC_USE_RTS_CTS) {
		flags |= AR5K_TXDESC_RTSENA;
		cts_rate = ieee80211_get_rts_cts_rate(sc->hw, info)->hw_value;
		duration = le16_to_cpu(ieee80211_rts_duration(sc->hw,
			sc->vif, pktlen, info));
	}
	if (rc_flags & IEEE80211_TX_RC_USE_CTS_PROTECT) {
		flags |= AR5K_TXDESC_CTSENA;
		cts_rate = ieee80211_get_rts_cts_rate(sc->hw, info)->hw_value;
		duration = le16_to_cpu(ieee80211_ctstoself_duration(sc->hw,
			sc->vif, pktlen, info));
	}
	ret = ah->ah_setup_tx_desc(ah, ds, pktlen,
		ieee80211_get_hdrlen_from_skb(skb), AR5K_PKT_TYPE_NORMAL,
		(sc->power_level * 2),
		hw_rate,
		info->control.rates[0].count, keyidx, ah->ah_tx_ant, flags,
		cts_rate, duration);
	if (ret)
		goto err_unmap;

	memset(mrr_rate, 0, sizeof(mrr_rate));
	memset(mrr_tries, 0, sizeof(mrr_tries));
	for (i = 0; i < 3; i++) {
		rate = ieee80211_get_alt_retry_rate(sc->hw, info, i);
		if (!rate)
			break;

		mrr_rate[i] = rate->hw_value;
		mrr_tries[i] = info->control.rates[i + 1].count;
	}

	ah->ah_setup_mrr_tx_desc(ah, ds,
		mrr_rate[0], mrr_tries[0],
		mrr_rate[1], mrr_tries[1],
		mrr_rate[2], mrr_tries[2]);

	ds->ds_link = 0;
	ds->ds_data = bf->skbaddr;

	spin_lock_bh(&txq->lock);
	list_add_tail(&bf->list, &txq->q);
	sc->tx_stats[txq->qnum].len++;
	if (txq->link == NULL) 
		ath5k_hw_set_txdp(ah, txq->qnum, bf->daddr);
	else 
		*txq->link = bf->daddr;

	txq->link = &ds->ds_link;
	ath5k_hw_start_tx_dma(ah, txq->qnum);
	mmiowb();
	spin_unlock_bh(&txq->lock);

	return 0;
err_unmap:
	pci_unmap_single(sc->pdev, bf->skbaddr, skb->len, PCI_DMA_TODEVICE);
	return ret;
}



static int
ath5k_desc_alloc(struct ath5k_softc *sc, struct pci_dev *pdev)
{
	struct ath5k_desc *ds;
	struct ath5k_buf *bf;
	dma_addr_t da;
	unsigned int i;
	int ret;

	
	sc->desc_len = sizeof(struct ath5k_desc) *
			(ATH_TXBUF + ATH_RXBUF + ATH_BCBUF + 1);
	sc->desc = pci_alloc_consistent(pdev, sc->desc_len, &sc->desc_daddr);
	if (sc->desc == NULL) {
		ATH5K_ERR(sc, "can't allocate descriptors\n");
		ret = -ENOMEM;
		goto err;
	}
	ds = sc->desc;
	da = sc->desc_daddr;
	ATH5K_DBG(sc, ATH5K_DEBUG_ANY, "DMA map: %p (%zu) -> %llx\n",
		ds, sc->desc_len, (unsigned long long)sc->desc_daddr);

	bf = kcalloc(1 + ATH_TXBUF + ATH_RXBUF + ATH_BCBUF,
			sizeof(struct ath5k_buf), GFP_KERNEL);
	if (bf == NULL) {
		ATH5K_ERR(sc, "can't allocate bufptr\n");
		ret = -ENOMEM;
		goto err_free;
	}
	sc->bufptr = bf;

	INIT_LIST_HEAD(&sc->rxbuf);
	for (i = 0; i < ATH_RXBUF; i++, bf++, ds++, da += sizeof(*ds)) {
		bf->desc = ds;
		bf->daddr = da;
		list_add_tail(&bf->list, &sc->rxbuf);
	}

	INIT_LIST_HEAD(&sc->txbuf);
	sc->txbuf_len = ATH_TXBUF;
	for (i = 0; i < ATH_TXBUF; i++, bf++, ds++,
			da += sizeof(*ds)) {
		bf->desc = ds;
		bf->daddr = da;
		list_add_tail(&bf->list, &sc->txbuf);
	}

	
	bf->desc = ds;
	bf->daddr = da;
	sc->bbuf = bf;

	return 0;
err_free:
	pci_free_consistent(pdev, sc->desc_len, sc->desc, sc->desc_daddr);
err:
	sc->desc = NULL;
	return ret;
}

static void
ath5k_desc_free(struct ath5k_softc *sc, struct pci_dev *pdev)
{
	struct ath5k_buf *bf;

	ath5k_txbuf_free(sc, sc->bbuf);
	list_for_each_entry(bf, &sc->txbuf, list)
		ath5k_txbuf_free(sc, bf);
	list_for_each_entry(bf, &sc->rxbuf, list)
		ath5k_rxbuf_free(sc, bf);

	
	pci_free_consistent(pdev, sc->desc_len, sc->desc, sc->desc_daddr);

	kfree(sc->bufptr);
	sc->bufptr = NULL;
}







static struct ath5k_txq *
ath5k_txq_setup(struct ath5k_softc *sc,
		int qtype, int subtype)
{
	struct ath5k_hw *ah = sc->ah;
	struct ath5k_txq *txq;
	struct ath5k_txq_info qi = {
		.tqi_subtype = subtype,
		.tqi_aifs = AR5K_TXQ_USEDEFAULT,
		.tqi_cw_min = AR5K_TXQ_USEDEFAULT,
		.tqi_cw_max = AR5K_TXQ_USEDEFAULT
	};
	int qnum;

	
	qi.tqi_flags = AR5K_TXQ_FLAG_TXEOLINT_ENABLE |
				AR5K_TXQ_FLAG_TXDESCINT_ENABLE;
	qnum = ath5k_hw_setup_tx_queue(ah, qtype, &qi);
	if (qnum < 0) {
		
		return ERR_PTR(qnum);
	}
	if (qnum >= ARRAY_SIZE(sc->txqs)) {
		ATH5K_ERR(sc, "hw qnum %u out of range, max %tu!\n",
			qnum, ARRAY_SIZE(sc->txqs));
		ath5k_hw_release_tx_queue(ah, qnum);
		return ERR_PTR(-EINVAL);
	}
	txq = &sc->txqs[qnum];
	if (!txq->setup) {
		txq->qnum = qnum;
		txq->link = NULL;
		INIT_LIST_HEAD(&txq->q);
		spin_lock_init(&txq->lock);
		txq->setup = true;
	}
	return &sc->txqs[qnum];
}

static int
ath5k_beaconq_setup(struct ath5k_hw *ah)
{
	struct ath5k_txq_info qi = {
		.tqi_aifs = AR5K_TXQ_USEDEFAULT,
		.tqi_cw_min = AR5K_TXQ_USEDEFAULT,
		.tqi_cw_max = AR5K_TXQ_USEDEFAULT,
		
		.tqi_flags = AR5K_TXQ_FLAG_TXDESCINT_ENABLE
	};

	return ath5k_hw_setup_tx_queue(ah, AR5K_TX_QUEUE_BEACON, &qi);
}

static int
ath5k_beaconq_config(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	struct ath5k_txq_info qi;
	int ret;

	ret = ath5k_hw_get_tx_queueprops(ah, sc->bhalq, &qi);
	if (ret)
		return ret;
	if (sc->opmode == NL80211_IFTYPE_AP ||
		sc->opmode == NL80211_IFTYPE_MESH_POINT) {
		
		qi.tqi_aifs = 0;
		qi.tqi_cw_min = 0;
		qi.tqi_cw_max = 0;
	} else if (sc->opmode == NL80211_IFTYPE_ADHOC) {
		
		qi.tqi_aifs = 0;
		qi.tqi_cw_min = 0;
		qi.tqi_cw_max = 2 * ah->ah_cw_min;
	}

	ATH5K_DBG(sc, ATH5K_DEBUG_BEACON,
		"beacon queueprops tqi_aifs:%d tqi_cw_min:%d tqi_cw_max:%d\n",
		qi.tqi_aifs, qi.tqi_cw_min, qi.tqi_cw_max);

	ret = ath5k_hw_set_tx_queueprops(ah, sc->bhalq, &qi);
	if (ret) {
		ATH5K_ERR(sc, "%s: unable to update parameters for beacon "
			"hardware queue!\n", __func__);
		return ret;
	}

	return ath5k_hw_reset_tx_queue(ah, sc->bhalq); ;
}

static void
ath5k_txq_drainq(struct ath5k_softc *sc, struct ath5k_txq *txq)
{
	struct ath5k_buf *bf, *bf0;

	
	spin_lock_bh(&txq->lock);
	list_for_each_entry_safe(bf, bf0, &txq->q, list) {
		ath5k_debug_printtxbuf(sc, bf);

		ath5k_txbuf_free(sc, bf);

		spin_lock_bh(&sc->txbuflock);
		sc->tx_stats[txq->qnum].len--;
		list_move_tail(&bf->list, &sc->txbuf);
		sc->txbuf_len++;
		spin_unlock_bh(&sc->txbuflock);
	}
	txq->link = NULL;
	spin_unlock_bh(&txq->lock);
}


static void
ath5k_txq_cleanup(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	unsigned int i;

	
	if (likely(!test_bit(ATH_STAT_INVALID, sc->status))) {
		
		ath5k_hw_stop_tx_dma(ah, sc->bhalq);
		ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "beacon queue %x\n",
			ath5k_hw_get_txdp(ah, sc->bhalq));
		for (i = 0; i < ARRAY_SIZE(sc->txqs); i++)
			if (sc->txqs[i].setup) {
				ath5k_hw_stop_tx_dma(ah, sc->txqs[i].qnum);
				ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "txq [%u] %x, "
					"link %p\n",
					sc->txqs[i].qnum,
					ath5k_hw_get_txdp(ah,
							sc->txqs[i].qnum),
					sc->txqs[i].link);
			}
	}
	ieee80211_wake_queues(sc->hw); 

	for (i = 0; i < ARRAY_SIZE(sc->txqs); i++)
		if (sc->txqs[i].setup)
			ath5k_txq_drainq(sc, &sc->txqs[i]);
}

static void
ath5k_txq_release(struct ath5k_softc *sc)
{
	struct ath5k_txq *txq = sc->txqs;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(sc->txqs); i++, txq++)
		if (txq->setup) {
			ath5k_hw_release_tx_queue(sc->ah, txq->qnum);
			txq->setup = false;
		}
}







static int
ath5k_rx_start(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	struct ath5k_buf *bf;
	int ret;

	sc->rxbufsize = roundup(IEEE80211_MAX_LEN, sc->common.cachelsz);

	ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "cachelsz %u rxbufsize %u\n",
		sc->common.cachelsz, sc->rxbufsize);

	spin_lock_bh(&sc->rxbuflock);
	sc->rxlink = NULL;
	list_for_each_entry(bf, &sc->rxbuf, list) {
		ret = ath5k_rxbuf_setup(sc, bf);
		if (ret != 0) {
			spin_unlock_bh(&sc->rxbuflock);
			goto err;
		}
	}
	bf = list_first_entry(&sc->rxbuf, struct ath5k_buf, list);
	ath5k_hw_set_rxdp(ah, bf->daddr);
	spin_unlock_bh(&sc->rxbuflock);

	ath5k_hw_start_rx_dma(ah);	
	ath5k_mode_setup(sc);		
	ath5k_hw_start_rx_pcu(ah);	

	return 0;
err:
	return ret;
}


static void
ath5k_rx_stop(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;

	ath5k_hw_stop_rx_pcu(ah);	
	ath5k_hw_set_rx_filter(ah, 0);	
	ath5k_hw_stop_rx_dma(ah);	

	ath5k_debug_printrxbuffs(sc, ah);

	sc->rxlink = NULL;		
}

static unsigned int
ath5k_rx_decrypted(struct ath5k_softc *sc, struct ath5k_desc *ds,
		struct sk_buff *skb, struct ath5k_rx_status *rs)
{
	struct ieee80211_hdr *hdr = (void *)skb->data;
	unsigned int keyix, hlen;

	if (!(rs->rs_status & AR5K_RXERR_DECRYPT) &&
			rs->rs_keyix != AR5K_RXKEYIX_INVALID)
		return RX_FLAG_DECRYPTED;

	
	hlen = ieee80211_hdrlen(hdr->frame_control);
	if (ieee80211_has_protected(hdr->frame_control) &&
	    !(rs->rs_status & AR5K_RXERR_DECRYPT) &&
	    skb->len >= hlen + 4) {
		keyix = skb->data[hlen + 3] >> 6;

		if (test_bit(keyix, sc->keymap))
			return RX_FLAG_DECRYPTED;
	}

	return 0;
}


static void
ath5k_check_ibss_tsf(struct ath5k_softc *sc, struct sk_buff *skb,
		     struct ieee80211_rx_status *rxs)
{
	u64 tsf, bc_tstamp;
	u32 hw_tu;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;

	if (ieee80211_is_beacon(mgmt->frame_control) &&
	    le16_to_cpu(mgmt->u.beacon.capab_info) & WLAN_CAPABILITY_IBSS &&
	    memcmp(mgmt->bssid, sc->ah->ah_bssid, ETH_ALEN) == 0) {
		
		tsf = ath5k_hw_get_tsf64(sc->ah);
		bc_tstamp = le64_to_cpu(mgmt->u.beacon.timestamp);
		hw_tu = TSF_TO_TU(tsf);

		ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			"beacon %llx mactime %llx (diff %lld) tsf now %llx\n",
			(unsigned long long)bc_tstamp,
			(unsigned long long)rxs->mactime,
			(unsigned long long)(rxs->mactime - bc_tstamp),
			(unsigned long long)tsf);

		
		if (bc_tstamp > rxs->mactime) {
			ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
				"fixing mactime from %llx to %llx\n",
				(unsigned long long)rxs->mactime,
				(unsigned long long)tsf);
			rxs->mactime = tsf;
		}

		
		if (hw_tu >= sc->nexttbtt)
			ath5k_beacon_update_timers(sc, bc_tstamp);
	}
}

static void
ath5k_tasklet_rx(unsigned long data)
{
	struct ieee80211_rx_status *rxs;
	struct ath5k_rx_status rs = {};
	struct sk_buff *skb, *next_skb;
	dma_addr_t next_skb_addr;
	struct ath5k_softc *sc = (void *)data;
	struct ath5k_buf *bf;
	struct ath5k_desc *ds;
	int ret;
	int hdrlen;
	int padsize;
	int rx_flag;

	spin_lock(&sc->rxbuflock);
	if (list_empty(&sc->rxbuf)) {
		ATH5K_WARN(sc, "empty rx buf pool\n");
		goto unlock;
	}
	do {
		rx_flag = 0;

		bf = list_first_entry(&sc->rxbuf, struct ath5k_buf, list);
		BUG_ON(bf->skb == NULL);
		skb = bf->skb;
		ds = bf->desc;

		
		if (ath5k_hw_get_rxdp(sc->ah) == bf->daddr)
			break;

		ret = sc->ah->ah_proc_rx_desc(sc->ah, ds, &rs);
		if (unlikely(ret == -EINPROGRESS))
			break;
		else if (unlikely(ret)) {
			ATH5K_ERR(sc, "error in processing rx descriptor\n");
			spin_unlock(&sc->rxbuflock);
			return;
		}

		if (unlikely(rs.rs_more)) {
			ATH5K_WARN(sc, "unsupported jumbo\n");
			goto next;
		}

		if (unlikely(rs.rs_status)) {
			if (rs.rs_status & AR5K_RXERR_PHY)
				goto next;
			if (rs.rs_status & AR5K_RXERR_DECRYPT) {
				
				if (rs.rs_keyix == AR5K_RXKEYIX_INVALID &&
				    !(rs.rs_status & AR5K_RXERR_CRC))
					goto accept;
			}
			if (rs.rs_status & AR5K_RXERR_MIC) {
				rx_flag |= RX_FLAG_MMIC_ERROR;
				goto accept;
			}

			
			if ((rs.rs_status &
				~(AR5K_RXERR_DECRYPT|AR5K_RXERR_MIC)) ||
					sc->opmode != NL80211_IFTYPE_MONITOR)
				goto next;
		}
accept:
		next_skb = ath5k_rx_skb_alloc(sc, &next_skb_addr);

		
		if (!next_skb)
			goto next;

		pci_unmap_single(sc->pdev, bf->skbaddr, sc->rxbufsize,
				PCI_DMA_FROMDEVICE);
		skb_put(skb, rs.rs_datalen);

		
		hdrlen = ieee80211_get_hdrlen_from_skb(skb);
		padsize = ath5k_pad_size(hdrlen);
		if (padsize) {
			memmove(skb->data + padsize, skb->data, hdrlen);
			skb_pull(skb, padsize);
		}
		rxs = IEEE80211_SKB_RXCB(skb);

		
		rxs->mactime = ath5k_extend_tsf(sc->ah, rs.rs_tstamp);
		rxs->flag = rx_flag | RX_FLAG_TSFT;

		rxs->freq = sc->curchan->center_freq;
		rxs->band = sc->curband->band;

		rxs->noise = sc->ah->ah_noise_floor;
		rxs->signal = rxs->noise + rs.rs_rssi;

		
		rxs->qual = rs.rs_rssi * 100 / 35;

		
		if (rxs->qual > 100)
			rxs->qual = 100;

		rxs->antenna = rs.rs_antenna;
		rxs->rate_idx = ath5k_hw_to_driver_rix(sc, rs.rs_rate);
		rxs->flag |= ath5k_rx_decrypted(sc, ds, skb, &rs);

		if (rxs->rate_idx >= 0 && rs.rs_rate ==
		    sc->curband->bitrates[rxs->rate_idx].hw_value_short)
			rxs->flag |= RX_FLAG_SHORTPRE;

		ath5k_debug_dump_skb(sc, skb, "RX  ", 0);

		
		if (sc->opmode == NL80211_IFTYPE_ADHOC)
			ath5k_check_ibss_tsf(sc, skb, rxs);

		ieee80211_rx(sc->hw, skb);

		bf->skb = next_skb;
		bf->skbaddr = next_skb_addr;
next:
		list_move_tail(&bf->list, &sc->rxbuf);
	} while (ath5k_rxbuf_setup(sc, bf) == 0);
unlock:
	spin_unlock(&sc->rxbuflock);
}






static void
ath5k_tx_processq(struct ath5k_softc *sc, struct ath5k_txq *txq)
{
	struct ath5k_tx_status ts = {};
	struct ath5k_buf *bf, *bf0;
	struct ath5k_desc *ds;
	struct sk_buff *skb;
	struct ieee80211_tx_info *info;
	int i, ret;

	spin_lock(&txq->lock);
	list_for_each_entry_safe(bf, bf0, &txq->q, list) {
		ds = bf->desc;

		ret = sc->ah->ah_proc_tx_desc(sc->ah, ds, &ts);
		if (unlikely(ret == -EINPROGRESS))
			break;
		else if (unlikely(ret)) {
			ATH5K_ERR(sc, "error %d while processing queue %u\n",
				ret, txq->qnum);
			break;
		}

		skb = bf->skb;
		info = IEEE80211_SKB_CB(skb);
		bf->skb = NULL;

		pci_unmap_single(sc->pdev, bf->skbaddr, skb->len,
				PCI_DMA_TODEVICE);

		ieee80211_tx_info_clear_status(info);
		for (i = 0; i < 4; i++) {
			struct ieee80211_tx_rate *r =
				&info->status.rates[i];

			if (ts.ts_rate[i]) {
				r->idx = ath5k_hw_to_driver_rix(sc, ts.ts_rate[i]);
				r->count = ts.ts_retry[i];
			} else {
				r->idx = -1;
				r->count = 0;
			}
		}

		
		info->status.rates[ts.ts_final_idx].count++;

		if (unlikely(ts.ts_status)) {
			sc->ll_stats.dot11ACKFailureCount++;
			if (ts.ts_status & AR5K_TXERR_FILT)
				info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
		} else {
			info->flags |= IEEE80211_TX_STAT_ACK;
			info->status.ack_signal = ts.ts_rssi;
		}

		ieee80211_tx_status(sc->hw, skb);
		sc->tx_stats[txq->qnum].count++;

		spin_lock(&sc->txbuflock);
		sc->tx_stats[txq->qnum].len--;
		list_move_tail(&bf->list, &sc->txbuf);
		sc->txbuf_len++;
		spin_unlock(&sc->txbuflock);
	}
	if (likely(list_empty(&txq->q)))
		txq->link = NULL;
	spin_unlock(&txq->lock);
	if (sc->txbuf_len > ATH_TXBUF / 5)
		ieee80211_wake_queues(sc->hw);
}

static void
ath5k_tasklet_tx(unsigned long data)
{
	int i;
	struct ath5k_softc *sc = (void *)data;

	for (i=0; i < AR5K_NUM_TX_QUEUES; i++)
		if (sc->txqs[i].setup && (sc->ah->ah_txq_isr & BIT(i)))
			ath5k_tx_processq(sc, &sc->txqs[i]);
}





static int
ath5k_beacon_setup(struct ath5k_softc *sc, struct ath5k_buf *bf)
{
	struct sk_buff *skb = bf->skb;
	struct	ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ath5k_hw *ah = sc->ah;
	struct ath5k_desc *ds;
	int ret = 0;
	u8 antenna;
	u32 flags;

	bf->skbaddr = pci_map_single(sc->pdev, skb->data, skb->len,
			PCI_DMA_TODEVICE);
	ATH5K_DBG(sc, ATH5K_DEBUG_BEACON, "skb %p [data %p len %u] "
			"skbaddr %llx\n", skb, skb->data, skb->len,
			(unsigned long long)bf->skbaddr);
	if (pci_dma_mapping_error(sc->pdev, bf->skbaddr)) {
		ATH5K_ERR(sc, "beacon DMA mapping failed\n");
		return -EIO;
	}

	ds = bf->desc;
	antenna = ah->ah_tx_ant;

	flags = AR5K_TXDESC_NOACK;
	if (sc->opmode == NL80211_IFTYPE_ADHOC && ath5k_hw_hasveol(ah)) {
		ds->ds_link = bf->daddr;	
		flags |= AR5K_TXDESC_VEOL;
	} else
		ds->ds_link = 0;

	
	if (ah->ah_ant_mode == AR5K_ANTMODE_SECTOR_AP)
		antenna = sc->bsent & 4 ? 2 : 1;


	
	ds->ds_data = bf->skbaddr;
	ret = ah->ah_setup_tx_desc(ah, ds, skb->len,
			ieee80211_get_hdrlen_from_skb(skb),
			AR5K_PKT_TYPE_BEACON, (sc->power_level * 2),
			ieee80211_get_tx_rate(sc->hw, info)->hw_value,
			1, AR5K_TXKEYIX_INVALID,
			antenna, flags, 0, 0);
	if (ret)
		goto err_unmap;

	return 0;
err_unmap:
	pci_unmap_single(sc->pdev, bf->skbaddr, skb->len, PCI_DMA_TODEVICE);
	return ret;
}


static void
ath5k_beacon_send(struct ath5k_softc *sc)
{
	struct ath5k_buf *bf = sc->bbuf;
	struct ath5k_hw *ah = sc->ah;
	struct sk_buff *skb;

	ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON, "in beacon_send\n");

	if (unlikely(bf->skb == NULL || sc->opmode == NL80211_IFTYPE_STATION ||
			sc->opmode == NL80211_IFTYPE_MONITOR)) {
		ATH5K_WARN(sc, "bf=%p bf_skb=%p\n", bf, bf ? bf->skb : NULL);
		return;
	}
	
	if (unlikely(ath5k_hw_num_tx_pending(ah, sc->bhalq) != 0)) {
		sc->bmisscount++;
		ATH5K_DBG(sc, ATH5K_DEBUG_BEACON,
			"missed %u consecutive beacons\n", sc->bmisscount);
		if (sc->bmisscount > 10) {	
			ATH5K_DBG(sc, ATH5K_DEBUG_BEACON,
				"stuck beacon time (%u missed)\n",
				sc->bmisscount);
			tasklet_schedule(&sc->restq);
		}
		return;
	}
	if (unlikely(sc->bmisscount != 0)) {
		ATH5K_DBG(sc, ATH5K_DEBUG_BEACON,
			"resume beacon xmit after %u misses\n",
			sc->bmisscount);
		sc->bmisscount = 0;
	}

	
	if (unlikely(ath5k_hw_stop_tx_dma(ah, sc->bhalq))) {
		ATH5K_WARN(sc, "beacon queue %u didn't start/stop ?\n", sc->bhalq);
		
	}

	
	if (sc->opmode == NL80211_IFTYPE_AP)
		ath5k_beacon_update(sc->hw, sc->vif);

	ath5k_hw_set_txdp(ah, sc->bhalq, bf->daddr);
	ath5k_hw_start_tx_dma(ah, sc->bhalq);
	ATH5K_DBG(sc, ATH5K_DEBUG_BEACON, "TXDP[%u] = %llx (%p)\n",
		sc->bhalq, (unsigned long long)bf->daddr, bf->desc);

	skb = ieee80211_get_buffered_bc(sc->hw, sc->vif);
	while (skb) {
		ath5k_tx_queue(sc->hw, skb, sc->cabq);
		skb = ieee80211_get_buffered_bc(sc->hw, sc->vif);
	}

	sc->bsent++;
}



static void
ath5k_beacon_update_timers(struct ath5k_softc *sc, u64 bc_tsf)
{
	struct ath5k_hw *ah = sc->ah;
	u32 nexttbtt, intval, hw_tu, bc_tu;
	u64 hw_tsf;

	intval = sc->bintval & AR5K_BEACON_PERIOD;
	if (WARN_ON(!intval))
		return;

	
	bc_tu = TSF_TO_TU(bc_tsf);

	
	hw_tsf = ath5k_hw_get_tsf64(ah);
	hw_tu = TSF_TO_TU(hw_tsf);

#define FUDGE 3
	
	if (bc_tsf == -1) {
		
		nexttbtt = roundup(hw_tu + FUDGE, intval);
	} else if (bc_tsf == 0) {
		
		nexttbtt = intval;
		intval |= AR5K_BEACON_RESET_TSF;
	} else if (bc_tsf > hw_tsf) {
		
		ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			"need to wait for HW TSF sync\n");
		return;
	} else {
		
		nexttbtt = bc_tu + roundup(hw_tu + FUDGE - bc_tu, intval);
	}
#undef FUDGE

	sc->nexttbtt = nexttbtt;

	intval |= AR5K_BEACON_ENA;
	ath5k_hw_init_beacon(ah, nexttbtt, intval);

	
	if (bc_tsf == -1)
		ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			"reconfigured timers based on HW TSF\n");
	else if (bc_tsf == 0)
		ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			"reset HW TSF and timers\n");
	else
		ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			"updated timers based on beacon TSF\n");

	ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON,
			  "bc_tsf %llx hw_tsf %llx bc_tu %u hw_tu %u nexttbtt %u\n",
			  (unsigned long long) bc_tsf,
			  (unsigned long long) hw_tsf, bc_tu, hw_tu, nexttbtt);
	ATH5K_DBG_UNLIMIT(sc, ATH5K_DEBUG_BEACON, "intval %u %s %s\n",
		intval & AR5K_BEACON_PERIOD,
		intval & AR5K_BEACON_ENA ? "AR5K_BEACON_ENA" : "",
		intval & AR5K_BEACON_RESET_TSF ? "AR5K_BEACON_RESET_TSF" : "");
}



static void
ath5k_beacon_config(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	unsigned long flags;

	spin_lock_irqsave(&sc->block, flags);
	sc->bmisscount = 0;
	sc->imask &= ~(AR5K_INT_BMISS | AR5K_INT_SWBA);

	if (sc->enable_beacon) {
		
		ath5k_beaconq_config(sc);

		sc->imask |= AR5K_INT_SWBA;

		if (sc->opmode == NL80211_IFTYPE_ADHOC) {
			if (ath5k_hw_hasveol(ah))
				ath5k_beacon_send(sc);
		} else
			ath5k_beacon_update_timers(sc, -1);
	} else {
		ath5k_hw_stop_tx_dma(sc->ah, sc->bhalq);
	}

	ath5k_hw_set_imr(ah, sc->imask);
	mmiowb();
	spin_unlock_irqrestore(&sc->block, flags);
}

static void ath5k_tasklet_beacon(unsigned long data)
{
	struct ath5k_softc *sc = (struct ath5k_softc *) data;

	
	if (sc->opmode == NL80211_IFTYPE_ADHOC) {
		
		u64 tsf = ath5k_hw_get_tsf64(sc->ah);
		sc->nexttbtt += sc->bintval;
		ATH5K_DBG(sc, ATH5K_DEBUG_BEACON,
				"SWBA nexttbtt: %x hw_tu: %x "
				"TSF: %llx\n",
				sc->nexttbtt,
				TSF_TO_TU(tsf),
				(unsigned long long) tsf);
	} else {
		spin_lock(&sc->block);
		ath5k_beacon_send(sc);
		spin_unlock(&sc->block);
	}
}




static int
ath5k_init(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;
	int ret, i;

	mutex_lock(&sc->lock);

	ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "mode %d\n", sc->opmode);

	
	ath5k_stop_locked(sc);

	
	ah->ah_cal_intval = ath5k_calinterval;

	
	sc->curchan = sc->hw->conf.channel;
	sc->curband = &sc->sbands[sc->curchan->band];
	sc->imask = AR5K_INT_RXOK | AR5K_INT_RXERR | AR5K_INT_RXEOL |
		AR5K_INT_RXORN | AR5K_INT_TXDESC | AR5K_INT_TXEOL |
		AR5K_INT_FATAL | AR5K_INT_GLOBAL | AR5K_INT_SWI;
	ret = ath5k_reset(sc, NULL);
	if (ret)
		goto done;

	ath5k_rfkill_hw_start(ah);

	
	for (i = 0; i < AR5K_KEYTABLE_SIZE; i++)
		ath5k_hw_reset_key(ah, i);

	
	ath5k_hw_set_ack_bitrate_high(ah, false);
	ret = 0;
done:
	mmiowb();
	mutex_unlock(&sc->lock);
	return ret;
}

static int
ath5k_stop_locked(struct ath5k_softc *sc)
{
	struct ath5k_hw *ah = sc->ah;

	ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "invalid %u\n",
			test_bit(ATH_STAT_INVALID, sc->status));

	
	ieee80211_stop_queues(sc->hw);

	if (!test_bit(ATH_STAT_INVALID, sc->status)) {
		ath5k_led_off(sc);
		ath5k_hw_set_imr(ah, 0);
		synchronize_irq(sc->pdev->irq);
	}
	ath5k_txq_cleanup(sc);
	if (!test_bit(ATH_STAT_INVALID, sc->status)) {
		ath5k_rx_stop(sc);
		ath5k_hw_phy_disable(ah);
	} else
		sc->rxlink = NULL;

	return 0;
}


static int
ath5k_stop_hw(struct ath5k_softc *sc)
{
	int ret;

	mutex_lock(&sc->lock);
	ret = ath5k_stop_locked(sc);
	if (ret == 0 && !test_bit(ATH_STAT_INVALID, sc->status)) {
		
		ret = ath5k_hw_on_hold(sc->ah);

		ATH5K_DBG(sc, ATH5K_DEBUG_RESET,
				"putting device to sleep\n");
	}
	ath5k_txbuf_free(sc, sc->bbuf);

	mmiowb();
	mutex_unlock(&sc->lock);

	tasklet_kill(&sc->rxtq);
	tasklet_kill(&sc->txtq);
	tasklet_kill(&sc->restq);
	tasklet_kill(&sc->calib);
	tasklet_kill(&sc->beacontq);

	ath5k_rfkill_hw_stop(sc->ah);

	return ret;
}

static irqreturn_t
ath5k_intr(int irq, void *dev_id)
{
	struct ath5k_softc *sc = dev_id;
	struct ath5k_hw *ah = sc->ah;
	enum ath5k_int status;
	unsigned int counter = 1000;

	if (unlikely(test_bit(ATH_STAT_INVALID, sc->status) ||
				!ath5k_hw_is_intr_pending(ah)))
		return IRQ_NONE;

	do {
		ath5k_hw_get_isr(ah, &status);		
		ATH5K_DBG(sc, ATH5K_DEBUG_INTR, "status 0x%x/0x%x\n",
				status, sc->imask);
		if (unlikely(status & AR5K_INT_FATAL)) {
			
			tasklet_schedule(&sc->restq);
		} else if (unlikely(status & AR5K_INT_RXORN)) {
			tasklet_schedule(&sc->restq);
		} else {
			if (status & AR5K_INT_SWBA) {
				tasklet_hi_schedule(&sc->beacontq);
			}
			if (status & AR5K_INT_RXEOL) {
				
				sc->rxlink = NULL;
			}
			if (status & AR5K_INT_TXURN) {
				
				ath5k_hw_update_tx_triglevel(ah, true);
			}
			if (status & (AR5K_INT_RXOK | AR5K_INT_RXERR))
				tasklet_schedule(&sc->rxtq);
			if (status & (AR5K_INT_TXOK | AR5K_INT_TXDESC
					| AR5K_INT_TXERR | AR5K_INT_TXEOL))
				tasklet_schedule(&sc->txtq);
			if (status & AR5K_INT_BMISS) {
				
			}
			if (status & AR5K_INT_SWI) {
				tasklet_schedule(&sc->calib);
			}
			if (status & AR5K_INT_MIB) {
				
				ath5k_hw_update_mib_counters(ah, &sc->ll_stats);
			}
			if (status & AR5K_INT_GPIO)
				tasklet_schedule(&sc->rf_kill.toggleq);

		}
	} while (ath5k_hw_is_intr_pending(ah) && --counter > 0);

	if (unlikely(!counter))
		ATH5K_WARN(sc, "too many interrupts, giving up for now\n");

	ath5k_hw_calibration_poll(ah);

	return IRQ_HANDLED;
}

static void
ath5k_tasklet_reset(unsigned long data)
{
	struct ath5k_softc *sc = (void *)data;

	ath5k_reset_wake(sc);
}


static void
ath5k_tasklet_calibrate(unsigned long data)
{
	struct ath5k_softc *sc = (void *)data;
	struct ath5k_hw *ah = sc->ah;

	
	if (ah->ah_swi_mask != AR5K_SWI_FULL_CALIBRATION)
		return;

	
	ieee80211_stop_queues(sc->hw);

	ATH5K_DBG(sc, ATH5K_DEBUG_CALIBRATE, "channel %u/%x\n",
		ieee80211_frequency_to_channel(sc->curchan->center_freq),
		sc->curchan->hw_value);

	if (ath5k_hw_gainf_calibrate(ah) == AR5K_RFGAIN_NEED_CHANGE) {
		
		ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "calibration, resetting\n");
		ath5k_reset_wake(sc);
	}
	if (ath5k_hw_phy_calibrate(ah, sc->curchan))
		ATH5K_ERR(sc, "calibration of channel %u failed\n",
			ieee80211_frequency_to_channel(
				sc->curchan->center_freq));

	ah->ah_swi_mask = 0;

	
	ieee80211_wake_queues(sc->hw);

}




static int
ath5k_tx(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	struct ath5k_softc *sc = hw->priv;

	return ath5k_tx_queue(hw, skb, sc->txq);
}

static int ath5k_tx_queue(struct ieee80211_hw *hw, struct sk_buff *skb,
			  struct ath5k_txq *txq)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_buf *bf;
	unsigned long flags;
	int hdrlen;
	int padsize;

	ath5k_debug_dump_skb(sc, skb, "TX  ", 1);

	if (sc->opmode == NL80211_IFTYPE_MONITOR)
		ATH5K_DBG(sc, ATH5K_DEBUG_XMIT, "tx in monitor (scan?)\n");

	
	hdrlen = ieee80211_get_hdrlen_from_skb(skb);
	padsize = ath5k_pad_size(hdrlen);
	if (padsize) {

		if (skb_headroom(skb) < padsize) {
			ATH5K_ERR(sc, "tx hdrlen not %%4: %d not enough"
				  " headroom to pad %d\n", hdrlen, padsize);
			goto drop_packet;
		}
		skb_push(skb, padsize);
		memmove(skb->data, skb->data+padsize, hdrlen);
	}

	spin_lock_irqsave(&sc->txbuflock, flags);
	if (list_empty(&sc->txbuf)) {
		ATH5K_ERR(sc, "no further txbuf available, dropping packet\n");
		spin_unlock_irqrestore(&sc->txbuflock, flags);
		ieee80211_stop_queue(hw, skb_get_queue_mapping(skb));
		goto drop_packet;
	}
	bf = list_first_entry(&sc->txbuf, struct ath5k_buf, list);
	list_del(&bf->list);
	sc->txbuf_len--;
	if (list_empty(&sc->txbuf))
		ieee80211_stop_queues(hw);
	spin_unlock_irqrestore(&sc->txbuflock, flags);

	bf->skb = skb;

	if (ath5k_txbuf_setup(sc, bf, txq)) {
		bf->skb = NULL;
		spin_lock_irqsave(&sc->txbuflock, flags);
		list_add_tail(&bf->list, &sc->txbuf);
		sc->txbuf_len++;
		spin_unlock_irqrestore(&sc->txbuflock, flags);
		goto drop_packet;
	}
	return NETDEV_TX_OK;

drop_packet:
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}


static int
ath5k_reset(struct ath5k_softc *sc, struct ieee80211_channel *chan)
{
	struct ath5k_hw *ah = sc->ah;
	int ret;

	ATH5K_DBG(sc, ATH5K_DEBUG_RESET, "resetting\n");

	if (chan) {
		ath5k_hw_set_imr(ah, 0);
		ath5k_txq_cleanup(sc);
		ath5k_rx_stop(sc);

		sc->curchan = chan;
		sc->curband = &sc->sbands[chan->band];
	}
	ret = ath5k_hw_reset(ah, sc->opmode, sc->curchan, chan != NULL);
	if (ret) {
		ATH5K_ERR(sc, "can't reset hardware (%d)\n", ret);
		goto err;
	}

	ret = ath5k_rx_start(sc);
	if (ret) {
		ATH5K_ERR(sc, "can't start recv logic\n");
		goto err;
	}

	


	ath5k_beacon_config(sc);
	

	return 0;
err:
	return ret;
}

static int
ath5k_reset_wake(struct ath5k_softc *sc)
{
	int ret;

	ret = ath5k_reset(sc, sc->curchan);
	if (!ret)
		ieee80211_wake_queues(sc->hw);

	return ret;
}

static int ath5k_start(struct ieee80211_hw *hw)
{
	return ath5k_init(hw->priv);
}

static void ath5k_stop(struct ieee80211_hw *hw)
{
	ath5k_stop_hw(hw->priv);
}

static int ath5k_add_interface(struct ieee80211_hw *hw,
		struct ieee80211_if_init_conf *conf)
{
	struct ath5k_softc *sc = hw->priv;
	int ret;

	mutex_lock(&sc->lock);
	if (sc->vif) {
		ret = 0;
		goto end;
	}

	sc->vif = conf->vif;

	switch (conf->type) {
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_MONITOR:
		sc->opmode = conf->type;
		break;
	default:
		ret = -EOPNOTSUPP;
		goto end;
	}

	ath5k_hw_set_lladdr(sc->ah, conf->mac_addr);
	ath5k_mode_setup(sc);

	ret = 0;
end:
	mutex_unlock(&sc->lock);
	return ret;
}

static void
ath5k_remove_interface(struct ieee80211_hw *hw,
			struct ieee80211_if_init_conf *conf)
{
	struct ath5k_softc *sc = hw->priv;
	u8 mac[ETH_ALEN] = {};

	mutex_lock(&sc->lock);
	if (sc->vif != conf->vif)
		goto end;

	ath5k_hw_set_lladdr(sc->ah, mac);
	sc->vif = NULL;
end:
	mutex_unlock(&sc->lock);
}


static int
ath5k_config(struct ieee80211_hw *hw, u32 changed)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	struct ieee80211_conf *conf = &hw->conf;
	int ret = 0;

	mutex_lock(&sc->lock);

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		ret = ath5k_chan_set(sc, conf->channel);
		if (ret < 0)
			goto unlock;
	}

	if ((changed & IEEE80211_CONF_CHANGE_POWER) &&
	(sc->power_level != conf->power_level)) {
		sc->power_level = conf->power_level;

		
		ath5k_hw_set_txpower_limit(ah, (conf->power_level * 2));
	}

	
	ath5k_hw_set_antenna_mode(ah, AR5K_ANTMODE_DEFAULT);

unlock:
	mutex_unlock(&sc->lock);
	return ret;
}

static u64 ath5k_prepare_multicast(struct ieee80211_hw *hw,
				   int mc_count, struct dev_addr_list *mclist)
{
	u32 mfilt[2], val;
	int i;
	u8 pos;

	mfilt[0] = 0;
	mfilt[1] = 1;

	for (i = 0; i < mc_count; i++) {
		if (!mclist)
			break;
		
		val = get_unaligned_le32(mclist->dmi_addr + 0);
		pos = (val >> 18) ^ (val >> 12) ^ (val >> 6) ^ val;
		val = get_unaligned_le32(mclist->dmi_addr + 3);
		pos ^= (val >> 18) ^ (val >> 12) ^ (val >> 6) ^ val;
		pos &= 0x3f;
		mfilt[pos / 32] |= (1 << (pos % 32));
		
		
		mclist = mclist->next;
	}

	return ((u64)(mfilt[1]) << 32) | mfilt[0];
}

#define SUPPORTED_FIF_FLAGS \
	FIF_PROMISC_IN_BSS |  FIF_ALLMULTI | FIF_FCSFAIL | \
	FIF_PLCPFAIL | FIF_CONTROL | FIF_OTHER_BSS | \
	FIF_BCN_PRBRESP_PROMISC

static void ath5k_configure_filter(struct ieee80211_hw *hw,
		unsigned int changed_flags,
		unsigned int *new_flags,
		u64 multicast)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	u32 mfilt[2], rfilt;

	mutex_lock(&sc->lock);

	mfilt[0] = multicast;
	mfilt[1] = multicast >> 32;

	
	changed_flags &= SUPPORTED_FIF_FLAGS;
	*new_flags &= SUPPORTED_FIF_FLAGS;

	
	rfilt = (ath5k_hw_get_rx_filter(ah) & (AR5K_RX_FILTER_PHYERR)) |
		(AR5K_RX_FILTER_UCAST | AR5K_RX_FILTER_BCAST |
		AR5K_RX_FILTER_MCAST);

	if (changed_flags & (FIF_PROMISC_IN_BSS | FIF_OTHER_BSS)) {
		if (*new_flags & FIF_PROMISC_IN_BSS) {
			rfilt |= AR5K_RX_FILTER_PROM;
			__set_bit(ATH_STAT_PROMISC, sc->status);
		} else {
			__clear_bit(ATH_STAT_PROMISC, sc->status);
		}
	}

	
	if (*new_flags & FIF_ALLMULTI) {
		mfilt[0] =  ~0;
		mfilt[1] =  ~0;
	}

	
	if (*new_flags & (FIF_FCSFAIL | FIF_PLCPFAIL))
		rfilt |= AR5K_RX_FILTER_PHYERR;

	
	if (*new_flags & FIF_BCN_PRBRESP_PROMISC)
		rfilt |= AR5K_RX_FILTER_BEACON | AR5K_RX_FILTER_PROBEREQ;

	
	if (*new_flags & FIF_CONTROL)
		rfilt |= AR5K_RX_FILTER_CONTROL;

	

	

	switch (sc->opmode) {
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_MONITOR:
		rfilt |= AR5K_RX_FILTER_CONTROL |
			 AR5K_RX_FILTER_BEACON |
			 AR5K_RX_FILTER_PROBEREQ |
			 AR5K_RX_FILTER_PROM;
		break;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_ADHOC:
		rfilt |= AR5K_RX_FILTER_PROBEREQ |
			 AR5K_RX_FILTER_BEACON;
		break;
	case NL80211_IFTYPE_STATION:
		if (sc->assoc)
			rfilt |= AR5K_RX_FILTER_BEACON;
	default:
		break;
	}

	
	ath5k_hw_set_rx_filter(ah, rfilt);

	
	ath5k_hw_set_mcast_filter(ah, mfilt[0], mfilt[1]);
	
	sc->filter_flags = rfilt;

	mutex_unlock(&sc->lock);
}

static int
ath5k_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
	      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
	      struct ieee80211_key_conf *key)
{
	struct ath5k_softc *sc = hw->priv;
	int ret = 0;

	if (modparam_nohwcrypt)
		return -EOPNOTSUPP;

	if (sc->opmode == NL80211_IFTYPE_AP)
		return -EOPNOTSUPP;

	switch (key->alg) {
	case ALG_WEP:
	case ALG_TKIP:
		break;
	case ALG_CCMP:
		if (sc->ah->ah_aes_support)
			break;

		return -EOPNOTSUPP;
	default:
		WARN_ON(1);
		return -EINVAL;
	}

	mutex_lock(&sc->lock);

	switch (cmd) {
	case SET_KEY:
		ret = ath5k_hw_set_key(sc->ah, key->keyidx, key,
				       sta ? sta->addr : NULL);
		if (ret) {
			ATH5K_ERR(sc, "can't set the key\n");
			goto unlock;
		}
		__set_bit(key->keyidx, sc->keymap);
		key->hw_key_idx = key->keyidx;
		key->flags |= (IEEE80211_KEY_FLAG_GENERATE_IV |
			       IEEE80211_KEY_FLAG_GENERATE_MMIC);
		break;
	case DISABLE_KEY:
		ath5k_hw_reset_key(sc->ah, key->keyidx);
		__clear_bit(key->keyidx, sc->keymap);
		break;
	default:
		ret = -EINVAL;
		goto unlock;
	}

unlock:
	mmiowb();
	mutex_unlock(&sc->lock);
	return ret;
}

static int
ath5k_get_stats(struct ieee80211_hw *hw,
		struct ieee80211_low_level_stats *stats)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;

	
	ath5k_hw_update_mib_counters(ah, &sc->ll_stats);

	memcpy(stats, &sc->ll_stats, sizeof(sc->ll_stats));

	return 0;
}

static int
ath5k_get_tx_stats(struct ieee80211_hw *hw,
		struct ieee80211_tx_queue_stats *stats)
{
	struct ath5k_softc *sc = hw->priv;

	memcpy(stats, &sc->tx_stats, sizeof(sc->tx_stats));

	return 0;
}

static u64
ath5k_get_tsf(struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;

	return ath5k_hw_get_tsf64(sc->ah);
}

static void
ath5k_set_tsf(struct ieee80211_hw *hw, u64 tsf)
{
	struct ath5k_softc *sc = hw->priv;

	ath5k_hw_set_tsf64(sc->ah, tsf);
}

static void
ath5k_reset_tsf(struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;

	
	if (sc->opmode == NL80211_IFTYPE_ADHOC)
		ath5k_beacon_update_timers(sc, 0);
	else
		ath5k_hw_reset_tsf(sc->ah);
}


static int
ath5k_beacon_update(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	int ret;
	struct ath5k_softc *sc = hw->priv;
	struct sk_buff *skb;

	if (WARN_ON(!vif)) {
		ret = -EINVAL;
		goto out;
	}

	skb = ieee80211_beacon_get(hw, vif);

	if (!skb) {
		ret = -ENOMEM;
		goto out;
	}

	ath5k_debug_dump_skb(sc, skb, "BC  ", 1);

	ath5k_txbuf_free(sc, sc->bbuf);
	sc->bbuf->skb = skb;
	ret = ath5k_beacon_setup(sc, sc->bbuf);
	if (ret)
		sc->bbuf->skb = NULL;
out:
	return ret;
}

static void
set_beacon_filter(struct ieee80211_hw *hw, bool enable)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	u32 rfilt;
	rfilt = ath5k_hw_get_rx_filter(ah);
	if (enable)
		rfilt |= AR5K_RX_FILTER_BEACON;
	else
		rfilt &= ~AR5K_RX_FILTER_BEACON;
	ath5k_hw_set_rx_filter(ah, rfilt);
	sc->filter_flags = rfilt;
}

static void ath5k_bss_info_changed(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    struct ieee80211_bss_conf *bss_conf,
				    u32 changes)
{
	struct ath5k_softc *sc = hw->priv;
	struct ath5k_hw *ah = sc->ah;
	unsigned long flags;

	mutex_lock(&sc->lock);
	if (WARN_ON(sc->vif != vif))
		goto unlock;

	if (changes & BSS_CHANGED_BSSID) {
		
		memcpy(ah->ah_bssid, bss_conf->bssid, ETH_ALEN);
		
		ath5k_hw_set_associd(ah, ah->ah_bssid, 0);
		mmiowb();
	}

	if (changes & BSS_CHANGED_BEACON_INT)
		sc->bintval = bss_conf->beacon_int;

	if (changes & BSS_CHANGED_ASSOC) {
		sc->assoc = bss_conf->assoc;
		if (sc->opmode == NL80211_IFTYPE_STATION)
			set_beacon_filter(hw, sc->assoc);
		ath5k_hw_set_ledstate(sc->ah, sc->assoc ?
			AR5K_LED_ASSOC : AR5K_LED_INIT);
	}

	if (changes & BSS_CHANGED_BEACON) {
		spin_lock_irqsave(&sc->block, flags);
		ath5k_beacon_update(hw, vif);
		spin_unlock_irqrestore(&sc->block, flags);
	}

	if (changes & BSS_CHANGED_BEACON_ENABLED)
		sc->enable_beacon = bss_conf->enable_beacon;

	if (changes & (BSS_CHANGED_BEACON | BSS_CHANGED_BEACON_ENABLED |
		       BSS_CHANGED_BEACON_INT))
		ath5k_beacon_config(sc);

 unlock:
	mutex_unlock(&sc->lock);
}

static void ath5k_sw_scan_start(struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;
	if (!sc->assoc)
		ath5k_hw_set_ledstate(sc->ah, AR5K_LED_SCAN);
}

static void ath5k_sw_scan_complete(struct ieee80211_hw *hw)
{
	struct ath5k_softc *sc = hw->priv;
	ath5k_hw_set_ledstate(sc->ah, sc->assoc ?
		AR5K_LED_ASSOC : AR5K_LED_INIT);
}
