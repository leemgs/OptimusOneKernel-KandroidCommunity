

#define CLK_TYPE_PRIMARY	0x1
#define CLK_TYPE_PLL		0x2
#define CLK_TYPE_PROGRAMMABLE	0x4
#define CLK_TYPE_PERIPHERAL	0x8
#define CLK_TYPE_SYSTEM		0x10


struct clk {
	struct list_head node;
	const char	*name;		
	const char	*function;	
	struct device	*dev;		
	unsigned long	rate_hz;
	struct clk	*parent;
	u32		pmc_mask;
	void		(*mode)(struct clk *, int);
	unsigned	id:2;		
	unsigned	type;		
	u16		users;
};


extern int __init clk_register(struct clk *clk);
