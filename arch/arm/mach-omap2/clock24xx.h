

#ifndef __ARCH_ARM_MACH_OMAP2_CLOCK24XX_H
#define __ARCH_ARM_MACH_OMAP2_CLOCK24XX_H

#include "clock.h"

#include "prm.h"
#include "cm.h"
#include "prm-regbits-24xx.h"
#include "cm-regbits-24xx.h"
#include "sdrc.h"


#ifdef CONFIG_ARCH_OMAP2420
#define OMAP_CM_REGADDR			OMAP2420_CM_REGADDR
#define OMAP24XX_PRCM_CLKOUT_CTRL	OMAP2420_PRCM_CLKOUT_CTRL
#define OMAP24XX_PRCM_CLKEMUL_CTRL	OMAP2420_PRCM_CLKEMUL_CTRL
#else
#define OMAP_CM_REGADDR			OMAP2430_CM_REGADDR
#define OMAP24XX_PRCM_CLKOUT_CTRL	OMAP2430_PRCM_CLKOUT_CTRL
#define OMAP24XX_PRCM_CLKEMUL_CTRL	OMAP2430_PRCM_CLKEMUL_CTRL
#endif

static unsigned long omap2_table_mpu_recalc(struct clk *clk);
static int omap2_select_table_rate(struct clk *clk, unsigned long rate);
static long omap2_round_to_table_rate(struct clk *clk, unsigned long rate);
static unsigned long omap2_sys_clk_recalc(struct clk *clk);
static unsigned long omap2_osc_clk_recalc(struct clk *clk);
static unsigned long omap2_sys_clk_recalc(struct clk *clk);
static unsigned long omap2_dpllcore_recalc(struct clk *clk);
static int omap2_reprogram_dpllcore(struct clk *clk, unsigned long rate);


struct prcm_config {
	unsigned long xtal_speed;	
	unsigned long dpll_speed;	
	unsigned long mpu_speed;	
	unsigned long cm_clksel_mpu;	
	unsigned long cm_clksel_dsp;	
	unsigned long cm_clksel_gfx;	
	unsigned long cm_clksel1_core;	
	unsigned long cm_clksel1_pll;	
	unsigned long cm_clksel2_pll;	
	unsigned long cm_clksel_mdm;	
	unsigned long base_sdrc_rfr;	
	unsigned char flags;
};




#define RX_CLKSEL_DSS1			(0x10 << 8)
#define RX_CLKSEL_DSS2			(0x0 << 13)
#define RX_CLKSEL_SSI			(0x5 << 20)




#define R1_CLKSEL_L3			(4 << 0)
#define R1_CLKSEL_L4			(2 << 5)
#define R1_CLKSEL_USB			(4 << 25)
#define R1_CM_CLKSEL1_CORE_VAL		R1_CLKSEL_USB | RX_CLKSEL_SSI | \
					RX_CLKSEL_DSS2 | RX_CLKSEL_DSS1 | \
					R1_CLKSEL_L4 | R1_CLKSEL_L3
#define R1_CLKSEL_MPU			(2 << 0)
#define R1_CM_CLKSEL_MPU_VAL		R1_CLKSEL_MPU
#define R1_CLKSEL_DSP			(2 << 0)
#define R1_CLKSEL_DSP_IF		(2 << 5)
#define R1_CM_CLKSEL_DSP_VAL		R1_CLKSEL_DSP | R1_CLKSEL_DSP_IF
#define R1_CLKSEL_GFX			(2 << 0)
#define R1_CM_CLKSEL_GFX_VAL		R1_CLKSEL_GFX
#define R1_CLKSEL_MDM			(4 << 0)
#define R1_CM_CLKSEL_MDM_VAL		R1_CLKSEL_MDM


#define R2_CLKSEL_L3			(6 << 0)
#define R2_CLKSEL_L4			(2 << 5)
#define R2_CLKSEL_USB			(2 << 25)
#define R2_CM_CLKSEL1_CORE_VAL		R2_CLKSEL_USB | RX_CLKSEL_SSI | \
					RX_CLKSEL_DSS2 | RX_CLKSEL_DSS1 | \
					R2_CLKSEL_L4 | R2_CLKSEL_L3
#define R2_CLKSEL_MPU			(2 << 0)
#define R2_CM_CLKSEL_MPU_VAL		R2_CLKSEL_MPU
#define R2_CLKSEL_DSP			(2 << 0)
#define R2_CLKSEL_DSP_IF		(3 << 5)
#define R2_CM_CLKSEL_DSP_VAL		R2_CLKSEL_DSP | R2_CLKSEL_DSP_IF
#define R2_CLKSEL_GFX			(2 << 0)
#define R2_CM_CLKSEL_GFX_VAL		R2_CLKSEL_GFX
#define R2_CLKSEL_MDM			(6 << 0)
#define R2_CM_CLKSEL_MDM_VAL		R2_CLKSEL_MDM


#define RB_CLKSEL_L3			(1 << 0)
#define RB_CLKSEL_L4			(1 << 5)
#define RB_CLKSEL_USB			(1 << 25)
#define RB_CM_CLKSEL1_CORE_VAL		RB_CLKSEL_USB | RX_CLKSEL_SSI | \
					RX_CLKSEL_DSS2 | RX_CLKSEL_DSS1 | \
					RB_CLKSEL_L4 | RB_CLKSEL_L3
#define RB_CLKSEL_MPU			(1 << 0)
#define RB_CM_CLKSEL_MPU_VAL		RB_CLKSEL_MPU
#define RB_CLKSEL_DSP			(1 << 0)
#define RB_CLKSEL_DSP_IF		(1 << 5)
#define RB_CM_CLKSEL_DSP_VAL		RB_CLKSEL_DSP | RB_CLKSEL_DSP_IF
#define RB_CLKSEL_GFX			(1 << 0)
#define RB_CM_CLKSEL_GFX_VAL		RB_CLKSEL_GFX
#define RB_CLKSEL_MDM			(1 << 0)
#define RB_CM_CLKSEL_MDM_VAL		RB_CLKSEL_MDM


#define RXX_CLKSEL_VLYNQ		(0x12 << 15)
#define RXX_CLKSEL_SSI			(0x8 << 20)


#define RIII_CLKSEL_L3			(4 << 0)	
#define RIII_CLKSEL_L4			(2 << 5)	
#define RIII_CLKSEL_USB			(4 << 25)	
#define RIII_CM_CLKSEL1_CORE_VAL	RIII_CLKSEL_USB | RXX_CLKSEL_SSI | \
					RXX_CLKSEL_VLYNQ | RX_CLKSEL_DSS2 | \
					RX_CLKSEL_DSS1 | RIII_CLKSEL_L4 | \
					RIII_CLKSEL_L3
#define RIII_CLKSEL_MPU			(2 << 0)	
#define RIII_CM_CLKSEL_MPU_VAL		RIII_CLKSEL_MPU
#define RIII_CLKSEL_DSP			(3 << 0)	
#define RIII_CLKSEL_DSP_IF		(2 << 5)	
#define RIII_SYNC_DSP			(1 << 7)	
#define RIII_CLKSEL_IVA			(6 << 8)	
#define RIII_SYNC_IVA			(1 << 13)	
#define RIII_CM_CLKSEL_DSP_VAL		RIII_SYNC_IVA | RIII_CLKSEL_IVA | \
					RIII_SYNC_DSP | RIII_CLKSEL_DSP_IF | \
					RIII_CLKSEL_DSP
#define RIII_CLKSEL_GFX			(2 << 0)	
#define RIII_CM_CLKSEL_GFX_VAL		RIII_CLKSEL_GFX


#define RII_CLKSEL_L3			(6 << 0)	
#define RII_CLKSEL_L4			(2 << 5)	
#define RII_CLKSEL_USB			(2 << 25)	
#define RII_CM_CLKSEL1_CORE_VAL		RII_CLKSEL_USB | \
					RXX_CLKSEL_SSI | RXX_CLKSEL_VLYNQ | \
					RX_CLKSEL_DSS2 | RX_CLKSEL_DSS1 | \
					RII_CLKSEL_L4 | RII_CLKSEL_L3
#define RII_CLKSEL_MPU			(2 << 0)	
#define RII_CM_CLKSEL_MPU_VAL		RII_CLKSEL_MPU
#define RII_CLKSEL_DSP			(3 << 0)	
#define RII_CLKSEL_DSP_IF		(2 << 5)	
#define RII_SYNC_DSP			(0 << 7)	
#define RII_CLKSEL_IVA			(3 << 8)	
#define RII_SYNC_IVA			(0 << 13)	
#define RII_CM_CLKSEL_DSP_VAL		RII_SYNC_IVA | RII_CLKSEL_IVA | \
					RII_SYNC_DSP | RII_CLKSEL_DSP_IF | \
					RII_CLKSEL_DSP
#define RII_CLKSEL_GFX			(2 << 0)	
#define RII_CM_CLKSEL_GFX_VAL		RII_CLKSEL_GFX


#define RI_CLKSEL_L3			(4 << 0)	
#define RI_CLKSEL_L4			(2 << 5)	
#define RI_CLKSEL_USB			(4 << 25)	
#define RI_CM_CLKSEL1_CORE_VAL		RI_CLKSEL_USB | \
					RXX_CLKSEL_SSI | RXX_CLKSEL_VLYNQ | \
					RX_CLKSEL_DSS2 | RX_CLKSEL_DSS1 | \
					RI_CLKSEL_L4 | RI_CLKSEL_L3
#define RI_CLKSEL_MPU			(2 << 0)	
#define RI_CM_CLKSEL_MPU_VAL		RI_CLKSEL_MPU
#define RI_CLKSEL_DSP			(3 << 0)	
#define RI_CLKSEL_DSP_IF		(2 << 5)	
#define RI_SYNC_DSP			(1 << 7)	
#define RI_CLKSEL_IVA			(4 << 8)	
#define RI_SYNC_IVA			(0 << 13)	
#define RI_CM_CLKSEL_DSP_VAL		RI_SYNC_IVA | RI_CLKSEL_IVA | \
					RI_SYNC_DSP | RI_CLKSEL_DSP_IF | \
					RI_CLKSEL_DSP
#define RI_CLKSEL_GFX			(1 << 0)	
#define RI_CM_CLKSEL_GFX_VAL		RI_CLKSEL_GFX


#define RVII_CLKSEL_L3			(1 << 0)
#define RVII_CLKSEL_L4			(1 << 5)
#define RVII_CLKSEL_DSS1		(1 << 8)
#define RVII_CLKSEL_DSS2		(0 << 13)
#define RVII_CLKSEL_VLYNQ		(1 << 15)
#define RVII_CLKSEL_SSI			(1 << 20)
#define RVII_CLKSEL_USB			(1 << 25)

#define RVII_CM_CLKSEL1_CORE_VAL	RVII_CLKSEL_USB | RVII_CLKSEL_SSI | \
					RVII_CLKSEL_VLYNQ | RVII_CLKSEL_DSS2 | \
					RVII_CLKSEL_DSS1 | RVII_CLKSEL_L4 | RVII_CLKSEL_L3

#define RVII_CLKSEL_MPU			(1 << 0) 
#define RVII_CM_CLKSEL_MPU_VAL		RVII_CLKSEL_MPU

#define RVII_CLKSEL_DSP			(1 << 0)
#define RVII_CLKSEL_DSP_IF		(1 << 5)
#define RVII_SYNC_DSP			(0 << 7)
#define RVII_CLKSEL_IVA			(1 << 8)
#define RVII_SYNC_IVA			(0 << 13)
#define RVII_CM_CLKSEL_DSP_VAL		RVII_SYNC_IVA | RVII_CLKSEL_IVA | RVII_SYNC_DSP | \
					RVII_CLKSEL_DSP_IF | RVII_CLKSEL_DSP

#define RVII_CLKSEL_GFX			(1 << 0)
#define RVII_CM_CLKSEL_GFX_VAL		RVII_CLKSEL_GFX




#define MX_48M_SRC			(0 << 3)
#define MX_54M_SRC			(0 << 5)
#define MX_APLLS_CLIKIN_12		(3 << 23)
#define MX_APLLS_CLIKIN_13		(2 << 23)
#define MX_APLLS_CLIKIN_19_2		(0 << 23)


#define M5A_DPLL_MULT_12		(133 << 12)
#define M5A_DPLL_DIV_12			(5 << 8)
#define M5A_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5A_DPLL_DIV_12 | M5A_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12
#define M5A_DPLL_MULT_13		(61 << 12)
#define M5A_DPLL_DIV_13			(2 << 8)
#define M5A_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5A_DPLL_DIV_13 | M5A_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13
#define M5A_DPLL_MULT_19		(55 << 12)
#define M5A_DPLL_DIV_19			(3 << 8)
#define M5A_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5A_DPLL_DIV_19 | M5A_DPLL_MULT_19 | \
					MX_APLLS_CLIKIN_19_2

#define M5B_DPLL_MULT_12		(50 << 12)
#define M5B_DPLL_DIV_12			(2 << 8)
#define M5B_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5B_DPLL_DIV_12 | M5B_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12
#define M5B_DPLL_MULT_13		(200 << 12)
#define M5B_DPLL_DIV_13			(12 << 8)

#define M5B_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5B_DPLL_DIV_13 | M5B_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13
#define M5B_DPLL_MULT_19		(125 << 12)
#define M5B_DPLL_DIV_19			(31 << 8)
#define M5B_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | \
					M5B_DPLL_DIV_19 | M5B_DPLL_MULT_19 | \
					MX_APLLS_CLIKIN_19_2

#define M4_DPLL_MULT_12			(133 << 12)
#define M4_DPLL_DIV_12			(3 << 8)
#define M4_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					M4_DPLL_DIV_12 | M4_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12

#define M4_DPLL_MULT_13			(399 << 12)
#define M4_DPLL_DIV_13			(12 << 8)
#define M4_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					M4_DPLL_DIV_13 | M4_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13

#define M4_DPLL_MULT_19			(145 << 12)
#define M4_DPLL_DIV_19			(6 << 8)
#define M4_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | \
					M4_DPLL_DIV_19 | M4_DPLL_MULT_19 | \
					MX_APLLS_CLIKIN_19_2


#define M3_DPLL_MULT_12			(55 << 12)
#define M3_DPLL_DIV_12			(1 << 8)
#define M3_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					M3_DPLL_DIV_12 | M3_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12
#define M3_DPLL_MULT_13			(76 << 12)
#define M3_DPLL_DIV_13			(2 << 8)
#define M3_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					M3_DPLL_DIV_13 | M3_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13
#define M3_DPLL_MULT_19			(17 << 12)
#define M3_DPLL_DIV_19			(0 << 8)
#define M3_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | \
					M3_DPLL_DIV_19 | M3_DPLL_MULT_19 | \
					MX_APLLS_CLIKIN_19_2


#define M2_DPLL_MULT_12		        (55 << 12)
#define M2_DPLL_DIV_12		        (1 << 8)
#define M2_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					M2_DPLL_DIV_12 | M2_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12



#define M2_DPLL_MULT_13		        (76 << 12)
#define M2_DPLL_DIV_13		        (2 << 8)
#define M2_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					M2_DPLL_DIV_13 | M2_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13

#define M2_DPLL_MULT_19		        (17 << 12)
#define M2_DPLL_DIV_19		        (0 << 8)
#define M2_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | \
					M2_DPLL_DIV_19 | M2_DPLL_MULT_19 | \
					MX_APLLS_CLIKIN_19_2


#define MB_DPLL_MULT			(1 << 12)
#define MB_DPLL_DIV			(0 << 8)
#define MB_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV |\
					MB_DPLL_MULT | MX_APLLS_CLIKIN_12

#define MB_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV |\
					MB_DPLL_MULT | MX_APLLS_CLIKIN_13

#define MB_CM_CLKSEL1_PLL_19_VAL	MX_48M_SRC | MX_54M_SRC | MB_DPLL_DIV |\
					MB_DPLL_MULT | MX_APLLS_CLIKIN_19




#define MI_DPLL_MULT_12			(55 << 12)
#define MI_DPLL_DIV_12			(1 << 8)
#define MI_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					MI_DPLL_DIV_12 | MI_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12


#define MII_DPLL_MULT_12		(50 << 12)
#define MII_DPLL_DIV_12			(1 << 8)
#define MII_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					MII_DPLL_DIV_12 | MII_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12
#define MII_DPLL_MULT_13		(300 << 12)
#define MII_DPLL_DIV_13			(12 << 8)
#define MII_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					MII_DPLL_DIV_13 | MII_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13


#define MIII_DPLL_MULT_12		(133 << 12)
#define MIII_DPLL_DIV_12		(5 << 8)
#define MIII_CM_CLKSEL1_PLL_12_VAL	MX_48M_SRC | MX_54M_SRC | \
					MIII_DPLL_DIV_12 | MIII_DPLL_MULT_12 | \
					MX_APLLS_CLIKIN_12
#define MIII_DPLL_MULT_13		(266 << 12)
#define MIII_DPLL_DIV_13		(12 << 8)
#define MIII_CM_CLKSEL1_PLL_13_VAL	MX_48M_SRC | MX_54M_SRC | \
					MIII_DPLL_DIV_13 | MIII_DPLL_MULT_13 | \
					MX_APLLS_CLIKIN_13


#define MVII_CM_CLKSEL1_PLL_12_VAL	MB_CM_CLKSEL1_PLL_12_VAL
#define MVII_CM_CLKSEL1_PLL_13_VAL	MB_CM_CLKSEL1_PLL_13_VAL


#define MX_CLKSEL2_PLL_2x_VAL		(2 << 0)
#define MX_CLKSEL2_PLL_1x_VAL		(1 << 0)


#define S12M	12000000
#define S13M	13000000
#define S19M	19200000
#define S26M	26000000
#define S100M	100000000
#define S133M	133000000
#define S150M	150000000
#define S164M	164000000
#define S165M	165000000
#define S199M	199000000
#define S200M	200000000
#define S266M	266000000
#define S300M	300000000
#define S329M	329000000
#define S330M	330000000
#define S399M	399000000
#define S400M	400000000
#define S532M	532000000
#define S600M	600000000
#define S658M	658000000
#define S660M	660000000
#define S798M	798000000


static struct prcm_config rate_table[] = {
	
	{S12M, S660M, S330M, RI_CM_CLKSEL_MPU_VAL,		
		RI_CM_CLKSEL_DSP_VAL, RI_CM_CLKSEL_GFX_VAL,
		RI_CM_CLKSEL1_CORE_VAL, MI_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_165MHz,
		RATE_IN_242X},

	
	{S12M, S600M, S300M, RII_CM_CLKSEL_MPU_VAL,		
		RII_CM_CLKSEL_DSP_VAL, RII_CM_CLKSEL_GFX_VAL,
		RII_CM_CLKSEL1_CORE_VAL, MII_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_100MHz,
		RATE_IN_242X},

	{S13M, S600M, S300M, RII_CM_CLKSEL_MPU_VAL,		
		RII_CM_CLKSEL_DSP_VAL, RII_CM_CLKSEL_GFX_VAL,
		RII_CM_CLKSEL1_CORE_VAL, MII_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_100MHz,
		RATE_IN_242X},

	
	{S12M, S532M, S266M, RIII_CM_CLKSEL_MPU_VAL,		
		RIII_CM_CLKSEL_DSP_VAL, RIII_CM_CLKSEL_GFX_VAL,
		RIII_CM_CLKSEL1_CORE_VAL, MIII_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_133MHz,
		RATE_IN_242X},

	{S13M, S532M, S266M, RIII_CM_CLKSEL_MPU_VAL,		
		RIII_CM_CLKSEL_DSP_VAL, RIII_CM_CLKSEL_GFX_VAL,
		RIII_CM_CLKSEL1_CORE_VAL, MIII_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_133MHz,
		RATE_IN_242X},

	
	{S12M, S300M, S150M, RII_CM_CLKSEL_MPU_VAL,		
		RII_CM_CLKSEL_DSP_VAL, RII_CM_CLKSEL_GFX_VAL,
		RII_CM_CLKSEL1_CORE_VAL, MII_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_100MHz,
		RATE_IN_242X},

	{S13M, S300M, S150M, RII_CM_CLKSEL_MPU_VAL,		
		RII_CM_CLKSEL_DSP_VAL, RII_CM_CLKSEL_GFX_VAL,
		RII_CM_CLKSEL1_CORE_VAL, MII_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_100MHz,
		RATE_IN_242X},

	
	{S12M, S266M, S133M, RIII_CM_CLKSEL_MPU_VAL,		
		RIII_CM_CLKSEL_DSP_VAL, RIII_CM_CLKSEL_GFX_VAL,
		RIII_CM_CLKSEL1_CORE_VAL, MIII_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_133MHz,
		RATE_IN_242X},

	{S13M, S266M, S133M, RIII_CM_CLKSEL_MPU_VAL,		
		RIII_CM_CLKSEL_DSP_VAL, RIII_CM_CLKSEL_GFX_VAL,
		RIII_CM_CLKSEL1_CORE_VAL, MIII_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_133MHz,
		RATE_IN_242X},

	
	{S12M, S12M, S12M, RVII_CM_CLKSEL_MPU_VAL,		
		RVII_CM_CLKSEL_DSP_VAL, RVII_CM_CLKSEL_GFX_VAL,
		RVII_CM_CLKSEL1_CORE_VAL, MVII_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_BYPASS,
		RATE_IN_242X},

	
	{S13M, S13M, S13M, RVII_CM_CLKSEL_MPU_VAL,		
		RVII_CM_CLKSEL_DSP_VAL, RVII_CM_CLKSEL_GFX_VAL,
		RVII_CM_CLKSEL1_CORE_VAL, MVII_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, 0, SDRC_RFR_CTRL_BYPASS,
		RATE_IN_242X},

	
	{S13M, S798M, S399M, R2_CM_CLKSEL_MPU_VAL,		
		R2_CM_CLKSEL_DSP_VAL, R2_CM_CLKSEL_GFX_VAL,
		R2_CM_CLKSEL1_CORE_VAL, M4_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, R2_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_133MHz,
		RATE_IN_243X},

	
	{S13M, S658M, S329M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M2_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_165MHz,
		RATE_IN_243X},

	
	{S13M, S532M, S266M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M5A_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_133MHz,
		RATE_IN_243X},

	
	{S13M, S400M, S200M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M5B_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_100MHz,
		RATE_IN_243X},

	
	{S13M, S399M, S199M, R2_CM_CLKSEL_MPU_VAL,		
		R2_CM_CLKSEL_DSP_VAL, R2_CM_CLKSEL_GFX_VAL,
		R2_CM_CLKSEL1_CORE_VAL, M4_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_1x_VAL, R2_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_133MHz,
		RATE_IN_243X},

	
	{S13M, S329M, S164M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M2_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_1x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_165MHz,
		RATE_IN_243X},

	
	{S13M, S266M, S133M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M5A_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_1x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_133MHz,
		RATE_IN_243X},

	
	{S13M, S200M, S100M, R1_CM_CLKSEL_MPU_VAL,		
		R1_CM_CLKSEL_DSP_VAL, R1_CM_CLKSEL_GFX_VAL,
		R1_CM_CLKSEL1_CORE_VAL, M5B_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_1x_VAL, R1_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_100MHz,
		RATE_IN_243X},

	
	{S13M, S13M, S13M, RB_CM_CLKSEL_MPU_VAL,		
		RB_CM_CLKSEL_DSP_VAL, RB_CM_CLKSEL_GFX_VAL,
		RB_CM_CLKSEL1_CORE_VAL, MB_CM_CLKSEL1_PLL_13_VAL,
		MX_CLKSEL2_PLL_2x_VAL, RB_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_BYPASS,
		RATE_IN_243X},

	
	{S12M, S12M, S12M, RB_CM_CLKSEL_MPU_VAL,		
		RB_CM_CLKSEL_DSP_VAL, RB_CM_CLKSEL_GFX_VAL,
		RB_CM_CLKSEL1_CORE_VAL, MB_CM_CLKSEL1_PLL_12_VAL,
		MX_CLKSEL2_PLL_2x_VAL, RB_CM_CLKSEL_MDM_VAL,
		SDRC_RFR_CTRL_BYPASS,
		RATE_IN_243X},

	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};




static struct clk func_32k_ck = {
	.name		= "func_32k_ck",
	.ops		= &clkops_null,
	.rate		= 32000,
	.flags		= RATE_FIXED,
	.clkdm_name	= "wkup_clkdm",
};

static struct clk secure_32k_ck = {
	.name		= "secure_32k_ck",
	.ops		= &clkops_null,
	.rate		= 32768,
	.flags		= RATE_FIXED,
	.clkdm_name	= "wkup_clkdm",
};


static struct clk osc_ck = {		
	.name		= "osc_ck",
	.ops		= &clkops_oscck,
	.clkdm_name	= "wkup_clkdm",
	.recalc		= &omap2_osc_clk_recalc,
};


static struct clk sys_ck = {		
	.name		= "sys_ck",		
	.ops		= &clkops_null,
	.parent		= &osc_ck,
	.clkdm_name	= "wkup_clkdm",
	.recalc		= &omap2_sys_clk_recalc,
};

static struct clk alt_ck = {		
	.name		= "alt_ck",
	.ops		= &clkops_null,
	.rate		= 54000000,
	.flags		= RATE_FIXED,
	.clkdm_name	= "wkup_clkdm",
};






static struct dpll_data dpll_dd = {
	.mult_div1_reg		= OMAP_CM_REGADDR(PLL_MOD, CM_CLKSEL1),
	.mult_mask		= OMAP24XX_DPLL_MULT_MASK,
	.div1_mask		= OMAP24XX_DPLL_DIV_MASK,
	.clk_bypass		= &sys_ck,
	.clk_ref		= &sys_ck,
	.control_reg		= OMAP_CM_REGADDR(PLL_MOD, CM_CLKEN),
	.enable_mask		= OMAP24XX_EN_DPLL_MASK,
	.max_multiplier		= 1024,
	.min_divider		= 1,
	.max_divider		= 16,
	.rate_tolerance		= DEFAULT_DPLL_RATE_TOLERANCE
};


static struct clk dpll_ck = {
	.name		= "dpll_ck",
	.ops		= &clkops_null,
	.parent		= &sys_ck,		
	.dpll_data	= &dpll_dd,
	.clkdm_name	= "wkup_clkdm",
	.recalc		= &omap2_dpllcore_recalc,
	.set_rate	= &omap2_reprogram_dpllcore,
};

static struct clk apll96_ck = {
	.name		= "apll96_ck",
	.ops		= &clkops_fixed,
	.parent		= &sys_ck,
	.rate		= 96000000,
	.flags		= RATE_FIXED | ENABLE_ON_INIT,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(PLL_MOD, CM_CLKEN),
	.enable_bit	= OMAP24XX_EN_96M_PLL_SHIFT,
};

static struct clk apll54_ck = {
	.name		= "apll54_ck",
	.ops		= &clkops_fixed,
	.parent		= &sys_ck,
	.rate		= 54000000,
	.flags		= RATE_FIXED | ENABLE_ON_INIT,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(PLL_MOD, CM_CLKEN),
	.enable_bit	= OMAP24XX_EN_54M_PLL_SHIFT,
};





static const struct clksel_rate func_54m_apll54_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel_rate func_54m_alt_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel func_54m_clksel[] = {
	{ .parent = &apll54_ck, .rates = func_54m_apll54_rates, },
	{ .parent = &alt_ck,	.rates = func_54m_alt_rates, },
	{ .parent = NULL },
};

static struct clk func_54m_ck = {
	.name		= "func_54m_ck",
	.ops		= &clkops_null,
	.parent		= &apll54_ck,	
	.clkdm_name	= "wkup_clkdm",
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(PLL_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_54M_SOURCE,
	.clksel		= func_54m_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk core_ck = {
	.name		= "core_ck",
	.ops		= &clkops_null,
	.parent		= &dpll_ck,		
	.clkdm_name	= "wkup_clkdm",
	.recalc		= &followparent_recalc,
};


static const struct clksel_rate func_96m_apll96_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel_rate func_96m_alt_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_243X | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel func_96m_clksel[] = {
	{ .parent = &apll96_ck,	.rates = func_96m_apll96_rates },
	{ .parent = &alt_ck,	.rates = func_96m_alt_rates },
	{ .parent = NULL }
};


static struct clk func_96m_ck = {
	.name		= "func_96m_ck",
	.ops		= &clkops_null,
	.parent		= &apll96_ck,
	.clkdm_name	= "wkup_clkdm",
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(PLL_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP2430_96M_SOURCE,
	.clksel		= func_96m_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};



static const struct clksel_rate func_48m_apll96_rates[] = {
	{ .div = 2, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel_rate func_48m_alt_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 },
};

static const struct clksel func_48m_clksel[] = {
	{ .parent = &apll96_ck,	.rates = func_48m_apll96_rates },
	{ .parent = &alt_ck, .rates = func_48m_alt_rates },
	{ .parent = NULL }
};

static struct clk func_48m_ck = {
	.name		= "func_48m_ck",
	.ops		= &clkops_null,
	.parent		= &apll96_ck,	 
	.clkdm_name	= "wkup_clkdm",
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(PLL_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_48M_SOURCE,
	.clksel		= func_48m_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk func_12m_ck = {
	.name		= "func_12m_ck",
	.ops		= &clkops_null,
	.parent		= &func_48m_ck,
	.fixed_div	= 4,
	.clkdm_name	= "wkup_clkdm",
	.recalc		= &omap2_fixed_divisor_recalc,
};


static struct clk wdt1_osc_ck = {
	.name		= "ck_wdt1_osc",
	.ops		= &clkops_null, 
	.parent		= &osc_ck,
	.recalc		= &followparent_recalc,
};


static const struct clksel_rate common_clkout_src_core_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate common_clkout_src_sys_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate common_clkout_src_96m_rates[] = {
	{ .div = 1, .val = 2, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate common_clkout_src_54m_rates[] = {
	{ .div = 1, .val = 3, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel common_clkout_src_clksel[] = {
	{ .parent = &core_ck,	  .rates = common_clkout_src_core_rates },
	{ .parent = &sys_ck,	  .rates = common_clkout_src_sys_rates },
	{ .parent = &func_96m_ck, .rates = common_clkout_src_96m_rates },
	{ .parent = &func_54m_ck, .rates = common_clkout_src_54m_rates },
	{ .parent = NULL }
};

static struct clk sys_clkout_src = {
	.name		= "sys_clkout_src",
	.ops		= &clkops_omap2_dflt,
	.parent		= &func_54m_ck,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.enable_bit	= OMAP24XX_CLKOUT_EN_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.clksel_mask	= OMAP24XX_CLKOUT_SOURCE_MASK,
	.clksel		= common_clkout_src_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static const struct clksel_rate common_clkout_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 2, .val = 1, .flags = RATE_IN_24XX },
	{ .div = 4, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 8, .val = 3, .flags = RATE_IN_24XX },
	{ .div = 16, .val = 4, .flags = RATE_IN_24XX },
	{ .div = 0 },
};

static const struct clksel sys_clkout_clksel[] = {
	{ .parent = &sys_clkout_src, .rates = common_clkout_rates },
	{ .parent = NULL }
};

static struct clk sys_clkout = {
	.name		= "sys_clkout",
	.ops		= &clkops_null,
	.parent		= &sys_clkout_src,
	.clkdm_name	= "wkup_clkdm",
	.clksel_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.clksel_mask	= OMAP24XX_CLKOUT_DIV_MASK,
	.clksel		= sys_clkout_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static struct clk sys_clkout2_src = {
	.name		= "sys_clkout2_src",
	.ops		= &clkops_omap2_dflt,
	.parent		= &func_54m_ck,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.enable_bit	= OMAP2420_CLKOUT2_EN_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.clksel_mask	= OMAP2420_CLKOUT2_SOURCE_MASK,
	.clksel		= common_clkout_src_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static const struct clksel sys_clkout2_clksel[] = {
	{ .parent = &sys_clkout2_src, .rates = common_clkout_rates },
	{ .parent = NULL }
};


static struct clk sys_clkout2 = {
	.name		= "sys_clkout2",
	.ops		= &clkops_null,
	.parent		= &sys_clkout2_src,
	.clkdm_name	= "wkup_clkdm",
	.clksel_reg	= OMAP24XX_PRCM_CLKOUT_CTRL,
	.clksel_mask	= OMAP2420_CLKOUT2_DIV_MASK,
	.clksel		= sys_clkout2_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk emul_ck = {
	.name		= "emul_ck",
	.ops		= &clkops_omap2_dflt,
	.parent		= &func_54m_ck,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP24XX_PRCM_CLKEMUL_CTRL,
	.enable_bit	= OMAP24XX_EMULATION_EN_SHIFT,
	.recalc		= &followparent_recalc,

};


static const struct clksel_rate mpu_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 4, .val = 4, .flags = RATE_IN_242X },
	{ .div = 6, .val = 6, .flags = RATE_IN_242X },
	{ .div = 8, .val = 8, .flags = RATE_IN_242X },
	{ .div = 0 },
};

static const struct clksel mpu_clksel[] = {
	{ .parent = &core_ck, .rates = mpu_core_rates },
	{ .parent = NULL }
};

static struct clk mpu_ck = {	
	.name		= "mpu_ck",
	.ops		= &clkops_null,
	.parent		= &core_ck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clkdm_name	= "mpu_clkdm",
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(MPU_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP24XX_CLKSEL_MPU_MASK,
	.clksel		= mpu_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static const struct clksel_rate dsp_fck_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 3, .val = 3, .flags = RATE_IN_24XX },
	{ .div = 4, .val = 4, .flags = RATE_IN_24XX },
	{ .div = 6, .val = 6, .flags = RATE_IN_242X },
	{ .div = 8, .val = 8, .flags = RATE_IN_242X },
	{ .div = 12, .val = 12, .flags = RATE_IN_242X },
	{ .div = 0 },
};

static const struct clksel dsp_fck_clksel[] = {
	{ .parent = &core_ck, .rates = dsp_fck_core_rates },
	{ .parent = NULL }
};

static struct clk dsp_fck = {
	.name		= "dsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_ck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clkdm_name	= "dsp_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_CM_FCLKEN_DSP_EN_DSP_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP24XX_CLKSEL_DSP_MASK,
	.clksel		= dsp_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static const struct clksel_rate dsp_irate_ick_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 3, .val = 3, .flags = RATE_IN_243X },
	{ .div = 0 },
};

static const struct clksel dsp_irate_ick_clksel[] = {
	{ .parent = &dsp_fck, .rates = dsp_irate_ick_rates },
	{ .parent = NULL }
};


static struct clk dsp_irate_ick = {
	.name		= "dsp_irate_ick",
	.ops		= &clkops_null,
	.parent		= &dsp_fck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clksel_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP24XX_CLKSEL_DSP_IF_MASK,
	.clksel		= dsp_irate_ick_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	      = &omap2_clksel_set_rate
};


static struct clk dsp_ick = {
	.name		= "dsp_ick",	 
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &dsp_irate_ick,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.enable_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP2420_EN_DSP_IPI_SHIFT,	      
};


static struct clk iva2_1_ick = {
	.name		= "iva2_1_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &dsp_irate_ick,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.enable_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_CM_FCLKEN_DSP_EN_DSP_SHIFT,
};


static struct clk iva1_ifck = {
	.name		= "iva1_ifck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_ck,
	.flags		= CONFIG_PARTICIPANT | DELAYED_APP,
	.clkdm_name	= "iva1_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP2420_EN_IVA_COP_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP2420_CLKSEL_IVA_MASK,
	.clksel		= dsp_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static struct clk iva1_mpu_int_ifck = {
	.name		= "iva1_mpu_int_ifck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &iva1_ifck,
	.clkdm_name	= "iva1_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(OMAP24XX_DSP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP2420_EN_IVA_MPU_SHIFT,
	.fixed_div	= 2,
	.recalc		= &omap2_fixed_divisor_recalc,
};


static const struct clksel_rate core_l3_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX },
	{ .div = 2, .val = 2, .flags = RATE_IN_242X },
	{ .div = 4, .val = 4, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 6, .val = 6, .flags = RATE_IN_24XX },
	{ .div = 8, .val = 8, .flags = RATE_IN_242X },
	{ .div = 12, .val = 12, .flags = RATE_IN_242X },
	{ .div = 16, .val = 16, .flags = RATE_IN_242X },
	{ .div = 0 }
};

static const struct clksel core_l3_clksel[] = {
	{ .parent = &core_ck, .rates = core_l3_core_rates },
	{ .parent = NULL }
};

static struct clk core_l3_ck = {	
	.name		= "core_l3_ck",
	.ops		= &clkops_null,
	.parent		= &core_ck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clkdm_name	= "core_l3_clkdm",
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_L3_MASK,
	.clksel		= core_l3_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static const struct clksel_rate usb_l4_ick_core_l3_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 4, .val = 4, .flags = RATE_IN_24XX },
	{ .div = 0 }
};

static const struct clksel usb_l4_ick_clksel[] = {
	{ .parent = &core_l3_ck, .rates = usb_l4_ick_core_l3_rates },
	{ .parent = NULL },
};


static struct clk usb_l4_ick = {	
	.name		= "usb_l4_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP24XX_EN_USB_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_USB_MASK,
	.clksel		= usb_l4_ick_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static const struct clksel_rate l4_core_l3_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 0 }
};

static const struct clksel l4_clksel[] = {
	{ .parent = &core_l3_ck, .rates = l4_core_l3_rates },
	{ .parent = NULL }
};

static struct clk l4_ck = {		
	.name		= "l4_ck",
	.ops		= &clkops_null,
	.parent		= &core_l3_ck,
	.flags		= DELAYED_APP,
	.clkdm_name	= "core_l4_clkdm",
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_L4_MASK,
	.clksel		= l4_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static const struct clksel_rate ssi_ssr_sst_fck_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 3, .val = 3, .flags = RATE_IN_24XX },
	{ .div = 4, .val = 4, .flags = RATE_IN_24XX },
	{ .div = 5, .val = 5, .flags = RATE_IN_243X },
	{ .div = 6, .val = 6, .flags = RATE_IN_242X },
	{ .div = 8, .val = 8, .flags = RATE_IN_242X },
	{ .div = 0 }
};

static const struct clksel ssi_ssr_sst_fck_clksel[] = {
	{ .parent = &core_ck, .rates = ssi_ssr_sst_fck_core_rates },
	{ .parent = NULL }
};

static struct clk ssi_ssr_sst_fck = {
	.name		= "ssi_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_ck,
	.flags		= DELAYED_APP,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP24XX_EN_SSI_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_SSI_MASK,
	.clksel		= ssi_ssr_sst_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};


static struct clk ssi_l4_ick = {
	.name		= "ssi_l4_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP24XX_EN_SSI_SHIFT,
	.recalc		= &followparent_recalc,
};






static const struct clksel gfx_fck_clksel[] = {
	{ .parent = &core_l3_ck, .rates = gfx_l3_rates },
	{ .parent = NULL },
};

static struct clk gfx_3d_fck = {
	.name		= "gfx_3d_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.clkdm_name	= "gfx_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(GFX_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_EN_3D_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(GFX_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP_CLKSEL_GFX_MASK,
	.clksel		= gfx_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk gfx_2d_fck = {
	.name		= "gfx_2d_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.clkdm_name	= "gfx_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(GFX_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_EN_2D_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(GFX_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP_CLKSEL_GFX_MASK,
	.clksel		= gfx_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk gfx_ick = {
	.name		= "gfx_ick",		
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.clkdm_name	= "gfx_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(GFX_MOD, CM_ICLKEN),
	.enable_bit	= OMAP_EN_GFX_SHIFT,
	.recalc		= &followparent_recalc,
};


static const struct clksel_rate mdm_ick_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_243X },
	{ .div = 4, .val = 4, .flags = RATE_IN_243X | DEFAULT_RATE },
	{ .div = 6, .val = 6, .flags = RATE_IN_243X },
	{ .div = 9, .val = 9, .flags = RATE_IN_243X },
	{ .div = 0 }
};

static const struct clksel mdm_ick_clksel[] = {
	{ .parent = &core_ck, .rates = mdm_ick_core_rates },
	{ .parent = NULL }
};

static struct clk mdm_ick = {		
	.name		= "mdm_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_ck,
	.flags		= DELAYED_APP | CONFIG_PARTICIPANT,
	.clkdm_name	= "mdm_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(OMAP2430_MDM_MOD, CM_ICLKEN),
	.enable_bit	= OMAP2430_CM_ICLKEN_MDM_EN_MDM_SHIFT,
	.clksel_reg	= OMAP_CM_REGADDR(OMAP2430_MDM_MOD, CM_CLKSEL),
	.clksel_mask	= OMAP2430_CLKSEL_MDM_MASK,
	.clksel		= mdm_ick_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk mdm_osc_ck = {
	.name		= "mdm_osc_ck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &osc_ck,
	.clkdm_name	= "mdm_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(OMAP2430_MDM_MOD, CM_FCLKEN),
	.enable_bit	= OMAP2430_EN_OSC_SHIFT,
	.recalc		= &followparent_recalc,
};




static const struct clksel_rate dss1_fck_sys_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate dss1_fck_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX },
	{ .div = 2, .val = 2, .flags = RATE_IN_24XX },
	{ .div = 3, .val = 3, .flags = RATE_IN_24XX },
	{ .div = 4, .val = 4, .flags = RATE_IN_24XX },
	{ .div = 5, .val = 5, .flags = RATE_IN_24XX },
	{ .div = 6, .val = 6, .flags = RATE_IN_24XX },
	{ .div = 8, .val = 8, .flags = RATE_IN_24XX },
	{ .div = 9, .val = 9, .flags = RATE_IN_24XX },
	{ .div = 12, .val = 12, .flags = RATE_IN_24XX },
	{ .div = 16, .val = 16, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel dss1_fck_clksel[] = {
	{ .parent = &sys_ck,  .rates = dss1_fck_sys_rates },
	{ .parent = &core_ck, .rates = dss1_fck_core_rates },
	{ .parent = NULL },
};

static struct clk dss_ick = {		
	.name		= "dss_ick",
	.ops		= &clkops_omap2_dflt,
	.parent		= &l4_ck,	
	.clkdm_name	= "dss_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_DSS1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk dss1_fck = {
	.name		= "dss1_fck",
	.ops		= &clkops_omap2_dflt,
	.parent		= &core_ck,		
	.flags		= DELAYED_APP,
	.clkdm_name	= "dss_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_DSS1_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_DSS1_MASK,
	.clksel		= dss1_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static const struct clksel_rate dss2_fck_sys_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate dss2_fck_48m_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel dss2_fck_clksel[] = {
	{ .parent = &sys_ck,	  .rates = dss2_fck_sys_rates },
	{ .parent = &func_48m_ck, .rates = dss2_fck_48m_rates },
	{ .parent = NULL }
};

static struct clk dss2_fck = {		
	.name		= "dss2_fck",
	.ops		= &clkops_omap2_dflt,
	.parent		= &sys_ck,		
	.flags		= DELAYED_APP,
	.clkdm_name	= "dss_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_DSS2_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_DSS2_MASK,
	.clksel		= dss2_fck_clksel,
	.recalc		= &followparent_recalc,
};

static struct clk dss_54m_fck = {	
	.name		= "dss_54m_fck",	
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_54m_ck,
	.clkdm_name	= "dss_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_TV_SHIFT,
	.recalc		= &followparent_recalc,
};


static const struct clksel_rate gpt_alt_rates[] = {
	{ .div = 1, .val = 2, .flags = RATE_IN_24XX | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel omap24xx_gpt_clksel[] = {
	{ .parent = &func_32k_ck, .rates = gpt_32k_rates },
	{ .parent = &sys_ck,	  .rates = gpt_sys_rates },
	{ .parent = &alt_ck,	  .rates = gpt_alt_rates },
	{ .parent = NULL },
};

static struct clk gpt1_ick = {
	.name		= "gpt1_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_GPT1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt1_fck = {
	.name		= "gpt1_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_EN_GPT1_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT1_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk gpt2_ick = {
	.name		= "gpt2_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt2_fck = {
	.name		= "gpt2_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT2_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT2_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt3_ick = {
	.name		= "gpt3_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt3_fck = {
	.name		= "gpt3_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT3_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT3_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt4_ick = {
	.name		= "gpt4_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT4_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt4_fck = {
	.name		= "gpt4_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT4_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT4_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt5_ick = {
	.name		= "gpt5_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT5_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt5_fck = {
	.name		= "gpt5_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT5_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT5_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt6_ick = {
	.name		= "gpt6_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT6_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt6_fck = {
	.name		= "gpt6_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT6_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT6_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt7_ick = {
	.name		= "gpt7_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT7_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt7_fck = {
	.name		= "gpt7_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT7_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT7_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt8_ick = {
	.name		= "gpt8_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT8_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt8_fck = {
	.name		= "gpt8_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT8_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT8_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt9_ick = {
	.name		= "gpt9_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT9_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt9_fck = {
	.name		= "gpt9_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT9_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT9_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt10_ick = {
	.name		= "gpt10_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT10_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt10_fck = {
	.name		= "gpt10_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT10_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT10_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt11_ick = {
	.name		= "gpt11_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT11_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt11_fck = {
	.name		= "gpt11_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT11_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT11_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk gpt12_ick = {
	.name		= "gpt12_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT12_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpt12_fck = {
	.name		= "gpt12_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &secure_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_GPT12_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL2),
	.clksel_mask	= OMAP24XX_CLKSEL_GPT12_MASK,
	.clksel		= omap24xx_gpt_clksel,
	.recalc		= &omap2_clksel_recalc,
};

static struct clk mcbsp1_ick = {
	.name		= "mcbsp_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MCBSP1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp1_fck = {
	.name		= "mcbsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_MCBSP1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp2_ick = {
	.name		= "mcbsp_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MCBSP2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp2_fck = {
	.name		= "mcbsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_MCBSP2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp3_ick = {
	.name		= "mcbsp_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 3,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp3_fck = {
	.name		= "mcbsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 3,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp4_ick = {
	.name		= "mcbsp_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 4,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP4_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp4_fck = {
	.name		= "mcbsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 4,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP4_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp5_ick = {
	.name		= "mcbsp_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 5,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP5_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcbsp5_fck = {
	.name		= "mcbsp_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 5,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MCBSP5_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi1_ick = {
	.name		= "mcspi_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MCSPI1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi1_fck = {
	.name		= "mcspi_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_MCSPI1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi2_ick = {
	.name		= "mcspi_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MCSPI2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi2_fck = {
	.name		= "mcspi_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_MCSPI2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi3_ick = {
	.name		= "mcspi_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 3,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MCSPI3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mcspi3_fck = {
	.name		= "mcspi_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 3,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MCSPI3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart1_ick = {
	.name		= "uart1_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_UART1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart1_fck = {
	.name		= "uart1_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_UART1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart2_ick = {
	.name		= "uart2_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_UART2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart2_fck = {
	.name		= "uart2_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_UART2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart3_ick = {
	.name		= "uart3_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP24XX_EN_UART3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk uart3_fck = {
	.name		= "uart3_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP24XX_EN_UART3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpios_ick = {
	.name		= "gpios_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_GPIOS_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpios_fck = {
	.name		= "gpios_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_EN_GPIOS_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mpu_wdt_ick = {
	.name		= "mpu_wdt_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_MPU_WDT_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mpu_wdt_fck = {
	.name		= "mpu_wdt_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "wkup_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_FCLKEN),
	.enable_bit	= OMAP24XX_EN_MPU_WDT_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk sync_32k_ick = {
	.name		= "sync_32k_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.flags		= ENABLE_ON_INIT,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_32KSYNC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk wdt1_ick = {
	.name		= "wdt1_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_WDT1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk omapctrl_ick = {
	.name		= "omapctrl_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.flags		= ENABLE_ON_INIT,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP24XX_EN_OMAPCTRL_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk icr_ick = {
	.name		= "icr_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(WKUP_MOD, CM_ICLKEN),
	.enable_bit	= OMAP2430_EN_ICR_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk cam_ick = {
	.name		= "cam_ick",
	.ops		= &clkops_omap2_dflt,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_CAM_SHIFT,
	.recalc		= &followparent_recalc,
};


static struct clk cam_fck = {
	.name		= "cam_fck",
	.ops		= &clkops_omap2_dflt,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_CAM_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mailboxes_ick = {
	.name		= "mailboxes_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MAILBOXES_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk wdt4_ick = {
	.name		= "wdt4_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_WDT4_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk wdt4_fck = {
	.name		= "wdt4_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_WDT4_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk wdt3_ick = {
	.name		= "wdt3_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_WDT3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk wdt3_fck = {
	.name		= "wdt3_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_WDT3_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mspro_ick = {
	.name		= "mspro_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_MSPRO_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mspro_fck = {
	.name		= "mspro_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_MSPRO_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmc_ick = {
	.name		= "mmc_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_MMC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmc_fck = {
	.name		= "mmc_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_MMC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk fac_ick = {
	.name		= "fac_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_FAC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk fac_fck = {
	.name		= "fac_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_12m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_FAC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk eac_ick = {
	.name		= "eac_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_EAC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk eac_fck = {
	.name		= "eac_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_EAC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk hdq_ick = {
	.name		= "hdq_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP24XX_EN_HDQ_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk hdq_fck = {
	.name		= "hdq_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_12m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP24XX_EN_HDQ_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2c2_ick = {
	.name		= "i2c_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_I2C2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2c2_fck = {
	.name		= "i2c_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 2,
	.parent		= &func_12m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_I2C2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2chs2_fck = {
	.name		= "i2c_fck",
	.ops		= &clkops_omap2430_i2chs_wait,
	.id		= 2,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_I2CHS2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2c1_ick = {
	.name		= "i2c_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_I2C1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2c1_fck = {
	.name		= "i2c_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &func_12m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_I2C1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk i2chs1_fck = {
	.name		= "i2c_fck",
	.ops		= &clkops_omap2430_i2chs_wait,
	.id		= 1,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_I2CHS1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpmc_fck = {
	.name		= "gpmc_fck",
	.ops		= &clkops_null, 
	.parent		= &core_l3_ck,
	.flags		= ENABLE_ON_INIT,
	.clkdm_name	= "core_l3_clkdm",
	.recalc		= &followparent_recalc,
};

static struct clk sdma_fck = {
	.name		= "sdma_fck",
	.ops		= &clkops_null, 
	.parent		= &core_l3_ck,
	.clkdm_name	= "core_l3_clkdm",
	.recalc		= &followparent_recalc,
};

static struct clk sdma_ick = {
	.name		= "sdma_ick",
	.ops		= &clkops_null, 
	.parent		= &l4_ck,
	.clkdm_name	= "core_l3_clkdm",
	.recalc		= &followparent_recalc,
};

static struct clk vlynq_ick = {
	.name		= "vlynq_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN1),
	.enable_bit	= OMAP2420_EN_VLYNQ_SHIFT,
	.recalc		= &followparent_recalc,
};

static const struct clksel_rate vlynq_fck_96m_rates[] = {
	{ .div = 1, .val = 0, .flags = RATE_IN_242X | DEFAULT_RATE },
	{ .div = 0 }
};

static const struct clksel_rate vlynq_fck_core_rates[] = {
	{ .div = 1, .val = 1, .flags = RATE_IN_242X },
	{ .div = 2, .val = 2, .flags = RATE_IN_242X },
	{ .div = 3, .val = 3, .flags = RATE_IN_242X },
	{ .div = 4, .val = 4, .flags = RATE_IN_242X },
	{ .div = 6, .val = 6, .flags = RATE_IN_242X },
	{ .div = 8, .val = 8, .flags = RATE_IN_242X },
	{ .div = 9, .val = 9, .flags = RATE_IN_242X },
	{ .div = 12, .val = 12, .flags = RATE_IN_242X },
	{ .div = 16, .val = 16, .flags = RATE_IN_242X | DEFAULT_RATE },
	{ .div = 18, .val = 18, .flags = RATE_IN_242X },
	{ .div = 0 }
};

static const struct clksel vlynq_fck_clksel[] = {
	{ .parent = &func_96m_ck, .rates = vlynq_fck_96m_rates },
	{ .parent = &core_ck,	  .rates = vlynq_fck_core_rates },
	{ .parent = NULL }
};

static struct clk vlynq_fck = {
	.name		= "vlynq_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_96m_ck,
	.flags		= DELAYED_APP,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_FCLKEN1),
	.enable_bit	= OMAP2420_EN_VLYNQ_SHIFT,
	.init		= &omap2_init_clksel_parent,
	.clksel_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_CLKSEL1),
	.clksel_mask	= OMAP2420_CLKSEL_VLYNQ_MASK,
	.clksel		= vlynq_fck_clksel,
	.recalc		= &omap2_clksel_recalc,
	.round_rate	= &omap2_clksel_round_rate,
	.set_rate	= &omap2_clksel_set_rate
};

static struct clk sdrc_ick = {
	.name		= "sdrc_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.flags		= ENABLE_ON_INIT,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN3),
	.enable_bit	= OMAP2430_EN_SDRC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk des_ick = {
	.name		= "des_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_ICLKEN4),
	.enable_bit	= OMAP24XX_EN_DES_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk sha_ick = {
	.name		= "sha_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_ICLKEN4),
	.enable_bit	= OMAP24XX_EN_SHA_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk rng_ick = {
	.name		= "rng_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_ICLKEN4),
	.enable_bit	= OMAP24XX_EN_RNG_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk aes_ick = {
	.name		= "aes_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_ICLKEN4),
	.enable_bit	= OMAP24XX_EN_AES_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk pka_ick = {
	.name		= "pka_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_ICLKEN4),
	.enable_bit	= OMAP24XX_EN_PKA_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk usb_fck = {
	.name		= "usb_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_48m_ck,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP24XX_EN_USB_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk usbhs_ick = {
	.name		= "usbhs_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &core_l3_ck,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_USBHS_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchs1_ick = {
	.name		= "mmchs_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHS1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchs1_fck = {
	.name		= "mmchs_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_96m_ck,
	.clkdm_name	= "core_l3_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHS1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchs2_ick = {
	.name		= "mmchs_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHS2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchs2_fck = {
	.name		= "mmchs_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &func_96m_ck,
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHS2_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpio5_ick = {
	.name		= "gpio5_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_GPIO5_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk gpio5_fck = {
	.name		= "gpio5_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_GPIO5_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mdm_intc_ick = {
	.name		= "mdm_intc_ick",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &l4_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, CM_ICLKEN2),
	.enable_bit	= OMAP2430_EN_MDM_INTC_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchsdb1_fck = {
	.name		= "mmchsdb_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHSDB1_SHIFT,
	.recalc		= &followparent_recalc,
};

static struct clk mmchsdb2_fck = {
	.name		= "mmchsdb_fck",
	.ops		= &clkops_omap2_dflt_wait,
	.id		= 1,
	.parent		= &func_32k_ck,
	.clkdm_name	= "core_l4_clkdm",
	.enable_reg	= OMAP_CM_REGADDR(CORE_MOD, OMAP24XX_CM_FCLKEN2),
	.enable_bit	= OMAP2430_EN_MMCHSDB2_SHIFT,
	.recalc		= &followparent_recalc,
};


static struct clk virt_prcm_set = {
	.name		= "virt_prcm_set",
	.ops		= &clkops_null,
	.flags		= DELAYED_APP,
	.parent		= &mpu_ck,	
	.recalc		= &omap2_table_mpu_recalc,	
	.set_rate	= &omap2_select_table_rate,
	.round_rate	= &omap2_round_to_table_rate,
};

#endif

