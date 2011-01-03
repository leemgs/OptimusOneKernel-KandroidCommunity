

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "ttype.h"
#include "device.h"
#include "usbpipe.h"




#define CONTROLnsRequestOut( Device,Request,Value,Index,Length,Buffer) \
        PIPEnsControlOut( Device,Request,Value,Index,Length,Buffer)

#define CONTROLnsRequestOutAsyn( Device,Request,Value,Index,Length,Buffer) \
        PIPEnsControlOutAsyn( Device,Request,Value,Index,Length,Buffer)

#define CONTROLnsRequestIn( Device,Request,Value,Index,Length,Buffer) \
        PIPEnsControlIn( Device,Request,Value,Index,Length,Buffer)








void ControlvWriteByte(
    IN PSDevice pDevice,
    IN BYTE byRegType,
    IN BYTE byRegOfs,
    IN BYTE byData
    );


void ControlvReadByte(
    IN PSDevice pDevice,
    IN BYTE byRegType,
    IN BYTE byRegOfs,
    IN PBYTE pbyData
    );


void ControlvMaskByte(
    IN PSDevice pDevice,
    IN BYTE byRegType,
    IN BYTE byRegOfs,
    IN BYTE byMask,
    IN BYTE byData
    );

#endif 



