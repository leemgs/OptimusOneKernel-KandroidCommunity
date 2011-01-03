
#ifndef __ASM_ARCH_REGSMEM_H
#define __ASM_ARCH_REGSMEM_H

#include <mach/hardware.h>




#define MEM_CTRL	__REG(0xa0700000)


#define MEM_STAT	__REG(0xa0700004)


#define MEM_CONF	__REG(0xa0700008)


#define MEM_DMCTRL	__REG(0xa0700020)


#define MEM_DMRT	__REG(0xa0700024)


#define MEM_DMRC	__REG(0xa0700028)


#define MEM_DMPCP	__REG(0xa0700030)


#define MEM_DMAPCP	__REG(0xa0700034)


#define MEM_DMSRET	__REG(0xa0700038)


#define MEM_DMLDOAT	__REG(0xa070003c)


#define MEM_DMDIACT	__REG(0xa0700040)


#define MEM_DMWRT	__REG(0xa0700044)


#define MEM_DMAACP	__REG(0xa0700048)


#define MEM_DMARP	__REG(0xa070004c)


#define MEM_DMESRAC	__REG(0xa0700050)


#define MEM_DMABAABT	__REG(0xa0700054)


#define MEM_DMLMACT	__REG(0xa0700058)


#define MEM_SMEW	__REG(0xa0700080)


#define MEM_DMCONF(x) 	__REG2(0xa0700100, (x) << 3)


#define MEM_DMRCD(x)	__REG2(0xa0700104, (x) << 3)


#define MEM_SMC(x)	__REG2(0xa0700200, (x) << 3)


#define MEM_SMC_PSMC		__REGBIT(20)
#define MEM_SMC_PSMC_OFF		__REGVAL(MEM_SMC_PSMC, 0)
#define MEM_SMC_PSMC_ON			__REGVAL(MEM_SMC_PSMC, 1)


#define MEM_SMC_BSMC		__REGBIT(19)
#define MEM_SMC_BSMC_OFF		__REGVAL(MEM_SMC_BSMC, 0)
#define MEM_SMC_BSMC_ON			__REGVAL(MEM_SMC_BSMC, 1)


#define MEM_SMC_EW		__REGBIT(8)
#define MEM_SMC_EW_OFF			__REGVAL(MEM_SMC_EW, 0)
#define MEM_SMC_EW_ON			__REGVAL(MEM_SMC_EW, 1)


#define MEM_SMC_PB		__REGBIT(7)
#define MEM_SMC_PB_0			__REGVAL(MEM_SMC_PB, 0)
#define MEM_SMC_PB_1			__REGVAL(MEM_SMC_PB, 1)


#define MEM_SMC_PC		__REGBIT(6)
#define MEM_SMC_PC_AL			__REGVAL(MEM_SMC_PC, 0)
#define MEM_SMC_PC_AH			__REGVAL(MEM_SMC_PC, 1)


#define MEM_SMC_PM		__REGBIT(3)
#define MEM_SMC_PM_DIS			__REGVAL(MEM_SMC_PM, 0)
#define MEM_SMC_PM_ASYNC		__REGVAL(MEM_SMC_PM, 1)


#define MEM_SMC_MW		__REGBITS(1, 0)
#define MEM_SMC_MW_8			__REGVAL(MEM_SMC_MW, 0)
#define MEM_SMC_MW_16			__REGVAL(MEM_SMC_MW, 1)
#define MEM_SMC_MW_32			__REGVAL(MEM_SMC_MW, 2)


#define MEM_SMWED(x)	__REG2(0xa0700204, (x) << 3)


#define MEM_SMOED(x)	__REG2(0xa0700208, (x) << 3)


#define MEM_SMRD(x)	__REG2(0xa070020c, (x) << 3)


#define MEM_SMPMRD(x)	__REG2(0xa0700210, (x) << 3)


#define MEM_SMWD(x)	__REG2(0xa0700214, (x) << 3)


#define MEM_SWT(x)	__REG2(0xa0700218, (x) << 3)

#endif 
