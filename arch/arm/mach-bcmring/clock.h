
#include <mach/csp/chipcHw_def.h>

#define CLK_TYPE_PRIMARY         1	
#define CLK_TYPE_PLL1            2	
#define CLK_TYPE_PLL2            4	
#define CLK_TYPE_PROGRAMMABLE    8	
#define CLK_TYPE_BYPASSABLE      16	

#define CLK_MODE_XTAL            1	

struct clk {
	const char *name;	
	unsigned int type;	
	unsigned int mode;	
	volatile int use_bypass;	
	chipcHw_CLOCK_e csp_id;	
	unsigned long rate_hz;	
	unsigned int use_cnt;	
	struct clk *parent;	
};
