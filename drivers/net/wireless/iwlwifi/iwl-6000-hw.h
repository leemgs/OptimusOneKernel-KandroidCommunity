


#ifndef __iwl_6000_hw_h__
#define __iwl_6000_hw_h__

#define IWL60_RTC_INST_LOWER_BOUND		(0x000000)
#define IWL60_RTC_INST_UPPER_BOUND		(0x040000)
#define IWL60_RTC_DATA_LOWER_BOUND		(0x800000)
#define IWL60_RTC_DATA_UPPER_BOUND		(0x814000)
#define IWL60_RTC_INST_SIZE \
	(IWL60_RTC_INST_UPPER_BOUND - IWL60_RTC_INST_LOWER_BOUND)
#define IWL60_RTC_DATA_SIZE \
	(IWL60_RTC_DATA_UPPER_BOUND - IWL60_RTC_DATA_LOWER_BOUND)

#endif 

