


#ifndef __iwl_5000_hw_h__
#define __iwl_5000_hw_h__

#define IWL50_RTC_INST_LOWER_BOUND		(0x000000)
#define IWL50_RTC_INST_UPPER_BOUND		(0x020000)

#define IWL50_RTC_DATA_LOWER_BOUND		(0x800000)
#define IWL50_RTC_DATA_UPPER_BOUND		(0x80C000)

#define IWL50_RTC_INST_SIZE (IWL50_RTC_INST_UPPER_BOUND - \
				IWL50_RTC_INST_LOWER_BOUND)
#define IWL50_RTC_DATA_SIZE (IWL50_RTC_DATA_UPPER_BOUND - \
				IWL50_RTC_DATA_LOWER_BOUND)


#define IWL_5000_EEPROM_IMG_SIZE			2048

#define IWL50_CMD_FIFO_NUM                 7
#define IWL50_NUM_QUEUES                  20
#define IWL50_NUM_AMPDU_QUEUES		  10
#define IWL50_FIRST_AMPDU_QUEUE		  10


#define IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF	(-5)

static inline s32 iwl_temp_calib_to_offset(struct iwl_priv *priv)
{
	u16 temperature, voltage;
	__le16 *temp_calib =
		(__le16 *)iwl_eeprom_query_addr(priv, EEPROM_5000_TEMPERATURE);

	temperature = le16_to_cpu(temp_calib[0]);
	voltage = le16_to_cpu(temp_calib[1]);

	
	return (s32)(temperature - voltage / IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF);
}




struct iwl5000_scd_bc_tbl {
	__le16 tfd_offset[TFD_QUEUE_BC_SIZE];
} __attribute__ ((packed));


#endif 

