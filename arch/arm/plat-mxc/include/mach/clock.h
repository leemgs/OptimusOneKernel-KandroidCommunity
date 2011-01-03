

#ifndef __ASM_ARCH_MXC_CLOCK_H__
#define __ASM_ARCH_MXC_CLOCK_H__

#ifndef __ASSEMBLY__
#include <linux/list.h>

struct module;

struct clk {
#ifndef CONFIG_COMMON_CLKDEV
	
	struct list_head node;
	struct module *owner;
	const char *name;
#endif
	int id;
	
	struct clk *parent;
	
	struct clk *secondary;
	
	__s8 usecount;
	
	u8 enable_shift;
	
	void __iomem *enable_reg;
	u32 flags;
	
	unsigned long (*get_rate) (struct clk *);
	
	int (*set_rate) (struct clk *, unsigned long);
	
	unsigned long (*round_rate) (struct clk *, unsigned long);
	
	int (*enable) (struct clk *);
	
	void (*disable) (struct clk *);
	
	int (*set_parent) (struct clk *, struct clk *);
};

int clk_register(struct clk *clk);
void clk_unregister(struct clk *clk);

unsigned long mxc_decode_pll(unsigned int pll, u32 f_ref);

#endif 
#endif 
