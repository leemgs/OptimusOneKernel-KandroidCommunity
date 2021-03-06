

#include <linux/nl80211.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include "ath9k.h"


static void ath_ahb_read_cachesize(struct ath_softc *sc, int *csz)
{
	*csz = L1_CACHE_BYTES >> 2;
}

static void ath_ahb_cleanup(struct ath_softc *sc)
{
	iounmap(sc->mem);
}

static bool ath_ahb_eeprom_read(struct ath_hw *ah, u32 off, u16 *data)
{
	struct ath_softc *sc = ah->ah_sc;
	struct platform_device *pdev = to_platform_device(sc->dev);
	struct ath9k_platform_data *pdata;

	pdata = (struct ath9k_platform_data *) pdev->dev.platform_data;
	if (off >= (ARRAY_SIZE(pdata->eeprom_data))) {
		DPRINTF(ah->ah_sc, ATH_DBG_FATAL,
			"%s: flash read failed, offset %08x is out of range\n",
				__func__, off);
		return false;
	}

	*data = pdata->eeprom_data[off];
	return true;
}

static struct ath_bus_ops ath_ahb_bus_ops  = {
	.read_cachesize = ath_ahb_read_cachesize,
	.cleanup = ath_ahb_cleanup,

	.eeprom_read = ath_ahb_eeprom_read,
};

static int ath_ahb_probe(struct platform_device *pdev)
{
	void __iomem *mem;
	struct ath_wiphy *aphy;
	struct ath_softc *sc;
	struct ieee80211_hw *hw;
	struct resource *res;
	int irq;
	int ret = 0;
	struct ath_hw *ah;

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "no platform data specified\n");
		ret = -EINVAL;
		goto err_out;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource found\n");
		ret = -ENXIO;
		goto err_out;
	}

	mem = ioremap_nocache(res->start, res->end - res->start + 1);
	if (mem == NULL) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENOMEM;
		goto err_out;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		ret = -ENXIO;
		goto err_iounmap;
	}

	irq = res->start;

	hw = ieee80211_alloc_hw(sizeof(struct ath_wiphy) +
				sizeof(struct ath_softc), &ath9k_ops);
	if (hw == NULL) {
		dev_err(&pdev->dev, "no memory for ieee80211_hw\n");
		ret = -ENOMEM;
		goto err_iounmap;
	}

	SET_IEEE80211_DEV(hw, &pdev->dev);
	platform_set_drvdata(pdev, hw);

	aphy = hw->priv;
	sc = (struct ath_softc *) (aphy + 1);
	aphy->sc = sc;
	aphy->hw = hw;
	sc->pri_wiphy = aphy;
	sc->hw = hw;
	sc->dev = &pdev->dev;
	sc->mem = mem;
	sc->bus_ops = &ath_ahb_bus_ops;
	sc->irq = irq;

	ret = ath_init_device(AR5416_AR9100_DEVID, sc, 0x0);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize device\n");
		goto err_free_hw;
	}

	ret = request_irq(irq, ath_isr, IRQF_SHARED, "ath9k", sc);
	if (ret) {
		dev_err(&pdev->dev, "request_irq failed\n");
		goto err_detach;
	}

	ah = sc->sc_ah;
	printk(KERN_INFO
	       "%s: Atheros AR%s MAC/BB Rev:%x, "
	       "AR%s RF Rev:%x, mem=0x%lx, irq=%d\n",
	       wiphy_name(hw->wiphy),
	       ath_mac_bb_name(ah->hw_version.macVersion),
	       ah->hw_version.macRev,
	       ath_rf_name((ah->hw_version.analog5GhzRev & AR_RADIO_SREV_MAJOR)),
	       ah->hw_version.phyRev,
	       (unsigned long)mem, irq);

	return 0;

 err_detach:
	ath_detach(sc);
 err_free_hw:
	ieee80211_free_hw(hw);
	platform_set_drvdata(pdev, NULL);
 err_iounmap:
	iounmap(mem);
 err_out:
	return ret;
}

static int ath_ahb_remove(struct platform_device *pdev)
{
	struct ieee80211_hw *hw = platform_get_drvdata(pdev);

	if (hw) {
		struct ath_wiphy *aphy = hw->priv;
		struct ath_softc *sc = aphy->sc;

		ath_cleanup(sc);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static struct platform_driver ath_ahb_driver = {
	.probe      = ath_ahb_probe,
	.remove     = ath_ahb_remove,
	.driver		= {
		.name	= "ath9k",
		.owner	= THIS_MODULE,
	},
};

int ath_ahb_init(void)
{
	return platform_driver_register(&ath_ahb_driver);
}

void ath_ahb_exit(void)
{
	platform_driver_unregister(&ath_ahb_driver);
}
