

#include <linux/kernel.h>
#include <linux/slab.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include "regd.h"
#include "regd_common.h"




#define ATH9K_2GHZ_CH01_11	REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)


#define ATH9K_2GHZ_CH12_13	REG_RULE(2467-10, 2472+10, 40, 0, 20,\
					NL80211_RRF_PASSIVE_SCAN)
#define ATH9K_2GHZ_CH14		REG_RULE(2484-10, 2484+10, 40, 0, 20,\
				NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_OFDM)


#define ATH9K_5GHZ_5150_5350	REG_RULE(5150-10, 5350+10, 40, 0, 30,\
				NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)
#define ATH9K_5GHZ_5470_5850	REG_RULE(5470-10, 5850+10, 40, 0, 30,\
				NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)
#define ATH9K_5GHZ_5725_5850	REG_RULE(5725-10, 5850+10, 40, 0, 30,\
				NL80211_RRF_PASSIVE_SCAN | NL80211_RRF_NO_IBSS)

#define ATH9K_2GHZ_ALL		ATH9K_2GHZ_CH01_11, \
				ATH9K_2GHZ_CH12_13, \
				ATH9K_2GHZ_CH14

#define ATH9K_5GHZ_ALL		ATH9K_5GHZ_5150_5350, \
				ATH9K_5GHZ_5470_5850

#define ATH9K_5GHZ_NO_MIDBAND	ATH9K_5GHZ_5150_5350, \
				ATH9K_5GHZ_5725_5850


static const struct ieee80211_regdomain ath_world_regdom_60_61_62 = {
	.n_reg_rules = 5,
	.alpha2 =  "99",
	.reg_rules = {
		ATH9K_2GHZ_ALL,
		ATH9K_5GHZ_ALL,
	}
};


static const struct ieee80211_regdomain ath_world_regdom_63_65 = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		ATH9K_2GHZ_CH01_11,
		ATH9K_2GHZ_CH12_13,
		ATH9K_5GHZ_NO_MIDBAND,
	}
};


static const struct ieee80211_regdomain ath_world_regdom_64 = {
	.n_reg_rules = 3,
	.alpha2 =  "99",
	.reg_rules = {
		ATH9K_2GHZ_CH01_11,
		ATH9K_5GHZ_NO_MIDBAND,
	}
};


static const struct ieee80211_regdomain ath_world_regdom_66_69 = {
	.n_reg_rules = 3,
	.alpha2 =  "99",
	.reg_rules = {
		ATH9K_2GHZ_CH01_11,
		ATH9K_5GHZ_ALL,
	}
};


static const struct ieee80211_regdomain ath_world_regdom_67_68_6A = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		ATH9K_2GHZ_CH01_11,
		ATH9K_2GHZ_CH12_13,
		ATH9K_5GHZ_ALL,
	}
};

static inline bool is_wwr_sku(u16 regd)
{
	return ((regd & WORLD_SKU_MASK) == WORLD_SKU_PREFIX) ||
		(regd == WORLD);
}

static u16 ath_regd_get_eepromRD(struct ath_regulatory *reg)
{
	return reg->current_rd & ~WORLDWIDE_ROAMING_FLAG;
}

bool ath_is_world_regd(struct ath_regulatory *reg)
{
	return is_wwr_sku(ath_regd_get_eepromRD(reg));
}
EXPORT_SYMBOL(ath_is_world_regd);

static const struct ieee80211_regdomain *ath_default_world_regdomain(void)
{
	
	return &ath_world_regdom_64;
}

static const struct
ieee80211_regdomain *ath_world_regdomain(struct ath_regulatory *reg)
{
	switch (reg->regpair->regDmnEnum) {
	case 0x60:
	case 0x61:
	case 0x62:
		return &ath_world_regdom_60_61_62;
	case 0x63:
	case 0x65:
		return &ath_world_regdom_63_65;
	case 0x64:
		return &ath_world_regdom_64;
	case 0x66:
	case 0x69:
		return &ath_world_regdom_66_69;
	case 0x67:
	case 0x68:
	case 0x6A:
		return &ath_world_regdom_67_68_6A;
	default:
		WARN_ON(1);
		return ath_default_world_regdomain();
	}
}


static bool ath_is_radar_freq(u16 center_freq)
{
	return (center_freq >= 5260 && center_freq <= 5700);
}


static void
ath_reg_apply_beaconing_flags(struct wiphy *wiphy,
			      enum nl80211_reg_initiator initiator)
{
	enum ieee80211_band band;
	struct ieee80211_supported_band *sband;
	const struct ieee80211_reg_rule *reg_rule;
	struct ieee80211_channel *ch;
	unsigned int i;
	u32 bandwidth = 0;
	int r;

	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {

		if (!wiphy->bands[band])
			continue;

		sband = wiphy->bands[band];

		for (i = 0; i < sband->n_channels; i++) {

			ch = &sband->channels[i];

			if (ath_is_radar_freq(ch->center_freq) ||
			    (ch->flags & IEEE80211_CHAN_RADAR))
				continue;

			if (initiator == NL80211_REGDOM_SET_BY_COUNTRY_IE) {
				r = freq_reg_info(wiphy,
						  ch->center_freq,
						  bandwidth,
						  &reg_rule);
				if (r)
					continue;
				
				if (!(reg_rule->flags &
				    NL80211_RRF_NO_IBSS))
					ch->flags &=
					  ~IEEE80211_CHAN_NO_IBSS;
				if (!(reg_rule->flags &
				    NL80211_RRF_PASSIVE_SCAN))
					ch->flags &=
					  ~IEEE80211_CHAN_PASSIVE_SCAN;
			} else {
				if (ch->beacon_found)
					ch->flags &= ~(IEEE80211_CHAN_NO_IBSS |
					  IEEE80211_CHAN_PASSIVE_SCAN);
			}
		}
	}

}


static void
ath_reg_apply_active_scan_flags(struct wiphy *wiphy,
				enum nl80211_reg_initiator initiator)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	const struct ieee80211_reg_rule *reg_rule;
	u32 bandwidth = 0;
	int r;

	sband = wiphy->bands[IEEE80211_BAND_2GHZ];

	
	if (initiator != NL80211_REGDOM_SET_BY_COUNTRY_IE) {
		ch = &sband->channels[11]; 
		if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN)
			ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
		ch = &sband->channels[12]; 
		if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN)
			ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
		return;
	}

	

	ch = &sband->channels[11]; 
	r = freq_reg_info(wiphy, ch->center_freq, bandwidth, &reg_rule);
	if (!r) {
		if (!(reg_rule->flags & NL80211_RRF_PASSIVE_SCAN))
			if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN)
				ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
	}

	ch = &sband->channels[12]; 
	r = freq_reg_info(wiphy, ch->center_freq, bandwidth, &reg_rule);
	if (!r) {
		if (!(reg_rule->flags & NL80211_RRF_PASSIVE_SCAN))
			if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN)
				ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
	}
}


static void ath_reg_apply_radar_flags(struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	unsigned int i;

	if (!wiphy->bands[IEEE80211_BAND_5GHZ])
		return;

	sband = wiphy->bands[IEEE80211_BAND_5GHZ];

	for (i = 0; i < sband->n_channels; i++) {
		ch = &sband->channels[i];
		if (!ath_is_radar_freq(ch->center_freq))
			continue;
		
		if (!(ch->flags & IEEE80211_CHAN_DISABLED))
			ch->flags |= IEEE80211_CHAN_RADAR |
				     IEEE80211_CHAN_NO_IBSS |
				     IEEE80211_CHAN_PASSIVE_SCAN;
	}
}

static void ath_reg_apply_world_flags(struct wiphy *wiphy,
				      enum nl80211_reg_initiator initiator,
				      struct ath_regulatory *reg)
{
	switch (reg->regpair->regDmnEnum) {
	case 0x60:
	case 0x63:
	case 0x66:
	case 0x67:
		ath_reg_apply_beaconing_flags(wiphy, initiator);
		break;
	case 0x68:
		ath_reg_apply_beaconing_flags(wiphy, initiator);
		ath_reg_apply_active_scan_flags(wiphy, initiator);
		break;
	}
	return;
}

int ath_reg_notifier_apply(struct wiphy *wiphy,
			   struct regulatory_request *request,
			   struct ath_regulatory *reg)
{
	
	ath_reg_apply_radar_flags(wiphy);

	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
	case NL80211_REGDOM_SET_BY_CORE:
	case NL80211_REGDOM_SET_BY_USER:
		break;
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		if (ath_is_world_regd(reg))
			ath_reg_apply_world_flags(wiphy, request->initiator,
						  reg);
		break;
	}

	return 0;
}
EXPORT_SYMBOL(ath_reg_notifier_apply);

static bool ath_regd_is_eeprom_valid(struct ath_regulatory *reg)
{
	 u16 rd = ath_regd_get_eepromRD(reg);
	int i;

	if (rd & COUNTRY_ERD_FLAG) {
		
		u16 cc = rd & ~COUNTRY_ERD_FLAG;
		printk(KERN_DEBUG
		       "ath: EEPROM indicates we should expect "
			"a country code\n");
		for (i = 0; i < ARRAY_SIZE(allCountries); i++)
			if (allCountries[i].countryCode == cc)
				return true;
	} else {
		
		if (rd != CTRY_DEFAULT)
			printk(KERN_DEBUG "ath: EEPROM indicates we "
			       "should expect a direct regpair map\n");
		for (i = 0; i < ARRAY_SIZE(regDomainPairs); i++)
			if (regDomainPairs[i].regDmnEnum == rd)
				return true;
	}
	printk(KERN_DEBUG
		 "ath: invalid regulatory domain/country code 0x%x\n", rd);
	return false;
}


static struct country_code_to_enum_rd*
ath_regd_find_country(u16 countryCode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(allCountries); i++) {
		if (allCountries[i].countryCode == countryCode)
			return &allCountries[i];
	}
	return NULL;
}


static struct country_code_to_enum_rd*
ath_regd_find_country_by_rd(int regdmn)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(allCountries); i++) {
		if (allCountries[i].regDmnEnum == regdmn)
			return &allCountries[i];
	}
	return NULL;
}


static u16 ath_regd_get_default_country(u16 rd)
{
	if (rd & COUNTRY_ERD_FLAG) {
		struct country_code_to_enum_rd *country = NULL;
		u16 cc = rd & ~COUNTRY_ERD_FLAG;

		country = ath_regd_find_country(cc);
		if (country != NULL)
			return cc;
	}

	return CTRY_DEFAULT;
}

static struct reg_dmn_pair_mapping*
ath_get_regpair(int regdmn)
{
	int i;

	if (regdmn == NO_ENUMRD)
		return NULL;
	for (i = 0; i < ARRAY_SIZE(regDomainPairs); i++) {
		if (regDomainPairs[i].regDmnEnum == regdmn)
			return &regDomainPairs[i];
	}
	return NULL;
}

static int
ath_regd_init_wiphy(struct ath_regulatory *reg,
		    struct wiphy *wiphy,
		    int (*reg_notifier)(struct wiphy *wiphy,
					struct regulatory_request *request))
{
	const struct ieee80211_regdomain *regd;

	wiphy->reg_notifier = reg_notifier;
	wiphy->strict_regulatory = true;

	if (ath_is_world_regd(reg)) {
		
		regd = ath_world_regdomain(reg);
		wiphy->custom_regulatory = true;
		wiphy->strict_regulatory = false;
	} else {
		
		regd = ath_default_world_regdomain();
	}
	wiphy_apply_custom_regulatory(wiphy, regd);
	ath_reg_apply_radar_flags(wiphy);
	ath_reg_apply_world_flags(wiphy, NL80211_REGDOM_SET_BY_DRIVER, reg);
	return 0;
}


static void ath_regd_sanitize(struct ath_regulatory *reg)
{
	if (reg->current_rd != COUNTRY_ERD_FLAG)
		return;
	printk(KERN_DEBUG "ath: EEPROM regdomain sanitized\n");
	reg->current_rd = 0x64;
}

int
ath_regd_init(struct ath_regulatory *reg,
	      struct wiphy *wiphy,
	      int (*reg_notifier)(struct wiphy *wiphy,
				  struct regulatory_request *request))
{
	struct country_code_to_enum_rd *country = NULL;
	u16 regdmn;

	if (!reg)
		return -EINVAL;

	ath_regd_sanitize(reg);

	printk(KERN_DEBUG "ath: EEPROM regdomain: 0x%0x\n", reg->current_rd);

	if (!ath_regd_is_eeprom_valid(reg)) {
		printk(KERN_ERR "ath: Invalid EEPROM contents\n");
		return -EINVAL;
	}

	regdmn = ath_regd_get_eepromRD(reg);
	reg->country_code = ath_regd_get_default_country(regdmn);

	if (reg->country_code == CTRY_DEFAULT &&
	    regdmn == CTRY_DEFAULT) {
		printk(KERN_DEBUG "ath: EEPROM indicates default "
		       "country code should be used\n");
		reg->country_code = CTRY_UNITED_STATES;
	}

	if (reg->country_code == CTRY_DEFAULT) {
		country = NULL;
	} else {
		printk(KERN_DEBUG "ath: doing EEPROM country->regdmn "
		       "map search\n");
		country = ath_regd_find_country(reg->country_code);
		if (country == NULL) {
			printk(KERN_DEBUG
				"ath: no valid country maps found for "
				"country code: 0x%0x\n",
				reg->country_code);
			return -EINVAL;
		} else {
			regdmn = country->regDmnEnum;
			printk(KERN_DEBUG "ath: country maps to "
			       "regdmn code: 0x%0x\n",
			       regdmn);
		}
	}

	reg->regpair = ath_get_regpair(regdmn);

	if (!reg->regpair) {
		printk(KERN_DEBUG "ath: "
			"No regulatory domain pair found, cannot continue\n");
		return -EINVAL;
	}

	if (!country)
		country = ath_regd_find_country_by_rd(regdmn);

	if (country) {
		reg->alpha2[0] = country->isoName[0];
		reg->alpha2[1] = country->isoName[1];
	} else {
		reg->alpha2[0] = '0';
		reg->alpha2[1] = '0';
	}

	printk(KERN_DEBUG "ath: Country alpha2 being used: %c%c\n",
		reg->alpha2[0], reg->alpha2[1]);
	printk(KERN_DEBUG "ath: Regpair used: 0x%0x\n",
		reg->regpair->regDmnEnum);

	ath_regd_init_wiphy(reg, wiphy, reg_notifier);
	return 0;
}
EXPORT_SYMBOL(ath_regd_init);

u32 ath_regd_get_band_ctl(struct ath_regulatory *reg,
			  enum ieee80211_band band)
{
	if (!reg->regpair ||
	    (reg->country_code == CTRY_DEFAULT &&
	     is_wwr_sku(ath_regd_get_eepromRD(reg)))) {
		return SD_NO_CTL;
	}

	switch (band) {
	case IEEE80211_BAND_2GHZ:
		return reg->regpair->reg_2ghz_ctl;
	case IEEE80211_BAND_5GHZ:
		return reg->regpair->reg_5ghz_ctl;
	default:
		return NO_CTL;
	}
}
EXPORT_SYMBOL(ath_regd_get_band_ctl);
