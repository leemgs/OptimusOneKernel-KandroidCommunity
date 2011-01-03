

#ifndef _ET1310_PM_H_
#define _ET1310_PM_H_

#include "et1310_address_map.h"

typedef struct _MP_POWER_MGMT {
	
	u8 TransPhyComaModeOnBoot;

	
	u16 PowerDownSpeed;
	u8 PowerDownDuplex;
} MP_POWER_MGMT, *PMP_POWER_MGMT;


struct et131x_adapter;

void EnablePhyComa(struct et131x_adapter *adapter);
void DisablePhyComa(struct et131x_adapter *adapter);

#endif 
