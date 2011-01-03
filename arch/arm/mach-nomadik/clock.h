

struct clk {
	unsigned long		rate;
};
extern int nmdk_clk_create(struct clk *clk, const char *dev_id);
