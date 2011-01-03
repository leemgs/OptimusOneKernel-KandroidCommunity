

#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#include "ttype.h"
#include "device.h"









BOOL
FIRMWAREbDownload(
    IN PSDevice pDevice
    );

BOOL
FIRMWAREbBrach2Sram(
    IN PSDevice pDevice
    );

BOOL
FIRMWAREbCheckVersion(
    IN PSDevice pDevice
    );


#endif 
