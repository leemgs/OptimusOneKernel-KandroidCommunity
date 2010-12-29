



#include "ath5k.h"
#include "reg.h"
#include "debug.h"
#include "base.h"


int ath5k_hw_set_capabilities(struct ath5k_hw *ah)
{
	u16 ee_header;

	ATH5K_TRACE(ah->ah_sc);
	
	ee_header = ah->ah_capabilities.cap_eeprom.ee_header;

	if (ah->ah_version == AR5K_AR5210) {
		
		ah->ah_capabilities.cap_range.range_5ghz_min = 5120;
		ah->ah_capabilities.cap_range.range_5ghz_max = 5430;
		ah->ah_capabilities.cap_range.range_2ghz_min = 0;
		ah->ah_capabilities.cap_range.range_2ghz_max = 0;

		
		__set_bit(AR5K_MODE_11A, ah->ah_capabilities.cap_mode);
		__set_bit(AR5K_MODE_11A_TURBO, ah->ah_capabilities.cap_mode);
	} else {
		

		

		if (AR5K_EEPROM_HDR_11A(ee_header)) {
			
			ah->ah_capabilities.cap_range.range_5ghz_min = 5005;
			ah->ah_capabilities.cap_range.range_5ghz_max = 6100;

			
			__set_bit(AR5K_MODE_11A,
					ah->ah_capabilities.cap_mode);
			__set_bit(AR5K_MODE_11A_TURBO,
					ah->ah_capabilities.cap_mode);
			if (ah->ah_version == AR5K_AR5212)
				__set_bit(AR5K_MODE_11G_TURBO,
						ah->ah_capabilities.cap_mode);
		}

		
		if (AR5K_EEPROM_HDR_11B(ee_header) ||
		    (AR5K_EEPROM_HDR_11G(ee_header) &&
		     ah->ah_version != AR5K_AR5211)) {
			
			ah->ah_capabilities.cap_range.range_2ghz_min = 2412;
			ah->ah_capabilities.cap_range.range_2ghz_max = 2732;

			if (AR5K_EEPROM_HDR_11B(ee_header))
				__set_bit(AR5K_MODE_11B,
						ah->ah_capabilities.cap_mode);

			if (AR5K_EEPROM_HDR_11G(ee_header) &&
			    ah->ah_version != AR5K_AR5211)
				__set_bit(AR5K_MODE_11G,
						ah->ah_capabilities.cap_mode);
		}
	}

	
	ah->ah_gpio_npins = AR5K_NUM_GPIO;

	
	if (ah->ah_version == AR5K_AR5210)
		ah->ah_capabilities.cap_queues.q_tx_num =
			AR5K_NUM_TX_QUEUES_NOQCU;
	else
		ah->ah_capabilities.cap_queues.q_tx_num = AR5K_NUM_TX_QUEUES;

	return 0;
}


int ath5k_hw_get_capability(struct ath5k_hw *ah,
		enum ath5k_capability_type cap_type,
		u32 capability, u32 *result)
{
	ATH5K_TRACE(ah->ah_sc);

	switch (cap_type) {
	case AR5K_CAP_NUM_TXQUEUES:
		if (result) {
			if (ah->ah_version == AR5K_AR5210)
				*result = AR5K_NUM_TX_QUEUES_NOQCU;
			else
				*result = AR5K_NUM_TX_QUEUES;
			goto yes;
		}
	case AR5K_CAP_VEOL:
		goto yes;
	case AR5K_CAP_COMPRESSION:
		if (ah->ah_version == AR5K_AR5212)
			goto yes;
		else
			goto no;
	case AR5K_CAP_BURST:
		goto yes;
	case AR5K_CAP_TPC:
		goto yes;
	case AR5K_CAP_BSSIDMASK:
		if (ah->ah_version == AR5K_AR5212)
			goto yes;
		else
			goto no;
	case AR5K_CAP_XR:
		if (ah->ah_version == AR5K_AR5212)
			goto yes;
		else
			goto no;
	default:
		goto no;
	}

no:
	return -EINVAL;
yes:
	return 0;
}



int ath5k_hw_enable_pspoll(struct ath5k_hw *ah, u8 *bssid,
		u16 assoc_id)
{
	ATH5K_TRACE(ah->ah_sc);

	if (ah->ah_version == AR5K_AR5210) {
		AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1,
			AR5K_STA_ID1_NO_PSPOLL | AR5K_STA_ID1_DEFAULT_ANTENNA);
		return 0;
	}

	return -EIO;
}

int ath5k_hw_disable_pspoll(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);

	if (ah->ah_version == AR5K_AR5210) {
		AR5K_REG_ENABLE_BITS(ah, AR5K_STA_ID1,
			AR5K_STA_ID1_NO_PSPOLL | AR5K_STA_ID1_DEFAULT_ANTENNA);
		return 0;
	}

	return -EIO;
}
