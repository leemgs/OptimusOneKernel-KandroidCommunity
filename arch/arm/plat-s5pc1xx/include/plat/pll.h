

#define S5P_PLL_MDIV_MASK	((1 << (25-16+1)) - 1)
#define S5P_PLL_PDIV_MASK	((1 << (13-8+1)) - 1)
#define S5P_PLL_SDIV_MASK	((1 << (2-0+1)) - 1)
#define S5P_PLL_MDIV_SHIFT	(16)
#define S5P_PLL_PDIV_SHIFT	(8)
#define S5P_PLL_SDIV_SHIFT	(0)

#include <asm/div64.h>

static inline unsigned long s5pc1xx_get_pll(unsigned long baseclk,
					    u32 pllcon)
{
	u32 mdiv, pdiv, sdiv;
	u64 fvco = baseclk;

	mdiv = (pllcon >> S5P_PLL_MDIV_SHIFT) & S5P_PLL_MDIV_MASK;
	pdiv = (pllcon >> S5P_PLL_PDIV_SHIFT) & S5P_PLL_PDIV_MASK;
	sdiv = (pllcon >> S5P_PLL_SDIV_SHIFT) & S5P_PLL_SDIV_MASK;

	fvco *= mdiv;
	do_div(fvco, (pdiv << sdiv));

	return (unsigned long)fvco;
}
