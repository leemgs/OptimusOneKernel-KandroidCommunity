


#ifndef __QMATH_COMPLEX_H__
#define __QMATH_COMPLEX_H__

#include <typedefs.h>
#include <qmath.h>

ComplexShort qcm_conj(ComplexShort op1);
int32 qcm_sqmag16(ComplexShort op1);
ComplexShort qcm_add16(ComplexShort op1, ComplexShort op2);
ComplexShort qcm_sub16(ComplexShort op1, ComplexShort op2);
ComplexShort qcm_mul16(ComplexShort op1, ComplexShort op2);
ComplexInt qcm_muls321616(ComplexShort op1, ComplexShort op2);
ComplexShort qcm_div16(ComplexShort op1, ComplexShort op2, int16* qQuotient);
ComplexInt qcm_sub32(ComplexInt op1, ComplexInt op2);

#endif  
