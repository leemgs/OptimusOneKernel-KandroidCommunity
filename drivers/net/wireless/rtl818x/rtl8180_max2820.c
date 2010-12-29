

#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <net/mac80211.h>

#include "rtl8180.h"
#include "rtl8180_max2820.h"

static const u32 max2820_chan[] = {
	12, 
	17,
	22,
	27,
	32,
	37,
	42,
	47,
	52,
	57,
	62,
	67,
	72,
	84, 
};

static void write_max2820(struct ieee80211_hw *dev, u8 addr, u32 data)
{
	struct rtl8180_priv *priv = dev->priv;
	u32 phy_config;

	phy_config = 0x90 + (data & 0xf);
	phy_config <<= 16;
	phy_config += addr;
	phy_config <<= 8;
	phy_config += (data >> 4) & 0xff;

	rtl818x_iowrite32(priv,
		(__le32 __iomem *) &priv->map->RFPinsOutput, phy_config);

	msleep(1);
}

static void max2820_write_phy_antenna(struct ieee80211_hw *dev, short chan)
{
	struct rtl8180_priv *priv = dev->priv;
	u8 ant;

	ant = MAXIM_ANTENNA;
	if (priv->rfparam & RF_PARAM_ANTBDEFAULT)
		ant |= BB_ANTENNA_B;
	if (chan == 14)
		ant |= BB_ANTATTEN_CHAN14;

	rtl8180_write_phy(dev, 0x10, ant);
}

static void max2820_rf_set_channel(struct ieee80211_hw *dev,
				   struct ieee80211_conf *conf)
{
	struct rtl8180_priv *priv = dev->priv;
	int channel = conf ?
		ieee80211_frequency_to_channel(conf->channel->center_freq) : 1;
	unsigned int chan_idx = channel - 1;
	u32 txpw = priv->channels[chan_idx].hw_value & 0xFF;
	u32 chan = max2820_chan[chan_idx];

	
	rtl8180_write_phy(dev, 3, txpw);

	max2820_write_phy_antenna(dev, channel);
	write_max2820(dev, 3, chan);
}

static void max2820_rf_stop(struct ieee80211_hw *dev)
{
	rtl8180_write_phy(dev, 3, 0x8);
	write_max2820(dev, 1, 0);
}


static void max2820_rf_init(struct ieee80211_hw *dev)
{
	struct rtl8180_priv *priv = dev->priv;

	
	write_max2820(dev, 0, 0x007); 
	write_max2820(dev, 1, 0x01e); 
	write_max2820(dev, 2, 0x001); 

	max2820_rf_set_channel(dev, NULL);

	write_max2820(dev, 4, 0x313); 

	
	write_max2820(dev, 5, 0x00f);

	
	rtl8180_write_phy(dev, 0, 0x88); 
	rtl8180_write_phy(dev, 3, 0x08); 
	rtl8180_write_phy(dev, 4, 0xf8); 
	rtl8180_write_phy(dev, 5, 0x90); 
	rtl8180_write_phy(dev, 6, 0x1a); 
	rtl8180_write_phy(dev, 7, 0x64); 

	max2820_write_phy_antenna(dev, 1);

	rtl8180_write_phy(dev, 0x11, 0x88); 

	if (rtl818x_ioread8(priv, &priv->map->CONFIG2) &
	    RTL818X_CONFIG2_ANTENNA_DIV)
		rtl8180_write_phy(dev, 0x12, 0xc7);
	else
		rtl8180_write_phy(dev, 0x12, 0x47);

	rtl8180_write_phy(dev, 0x13, 0x9b);

	rtl8180_write_phy(dev, 0x19, 0x0);  
	rtl8180_write_phy(dev, 0x1a, 0x9f); 

	max2820_rf_set_channel(dev, NULL);
}

const struct rtl818x_rf_ops max2820_rf_ops = {
	.name		= "Maxim",
	.init		= max2820_rf_init,
	.stop		= max2820_rf_stop,
	.set_chan	= max2820_rf_set_channel
};
