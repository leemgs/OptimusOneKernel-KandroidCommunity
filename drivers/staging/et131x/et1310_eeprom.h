

#ifndef __ET1310_EEPROM_H__
#define __ET1310_EEPROM_H__

#include "et1310_address_map.h"

#ifndef SUCCESS
#define SUCCESS		0
#define FAILURE		1
#endif


struct et131x_adapter;

int32_t EepromWriteByte(struct et131x_adapter *adapter, u32 unAddress,
			u8 bData);
int32_t EepromReadByte(struct et131x_adapter *adapter, u32 unAddress,
			u8 *pbData);

#endif 
