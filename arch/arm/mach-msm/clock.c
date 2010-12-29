

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/pm_qos_params.h>

#include "clock.h"
#include "socinfo.h"

static DEFINE_MUTEX(clocks_mutex);
static DEFINE_SPINLOCK(clocks_lock);
static DEFINE_SPINLOCK(ebi1_vote_lock);
static LIST_HEAD(clocks);
struct clk *msm_clocks;
unsigned msm_num_clocks;


static DECLARE_BITMAP(clock_map_enabled, NR_CLKS);
static DEFINE_SPINLOCK(clock_map_lock);
static struct notifier_block axi_freq_notifier_block;


struct clk *clk_get(struct device *dev, const char *id)
{
	struct clk *clk;

	mutex_lock(&clocks_mutex);

	list_for_each_entry(clk, &clocks, list)
		if (!strcmp(id, clk->name) && clk->dev == dev)
			goto found_it;

	list_for_each_entry(clk, &clocks, list)
		if (!strcmp(id, clk->name) && clk->dev == NULL)
			goto found_it;

	clk = ERR_PTR(-ENOENT);
found_it:
	mutex_unlock(&clocks_mutex);
	return clk;
}
EXPORT_SYMBOL(clk_get);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);

int clk_enable(struct clk *clk)
{
	unsigned long flags;
	spin_lock_irqsave(&clocks_lock, flags);
	clk->count++;
	if (clk->count == 1) {
		clk->ops->enable(clk->id);
		spin_lock(&clock_map_lock);
		clock_map_enabled[BIT_WORD(clk->id)] |= BIT_MASK(clk->id);
		spin_unlock(&clock_map_lock);
	}
	spin_unlock_irqrestore(&clocks_lock, flags);
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	unsigned long flags;
	spin_lock_irqsave(&clocks_lock, flags);
	BUG_ON(clk->count == 0);
	clk->count--;
	if (clk->count == 0) {
		clk->ops->disable(clk->id);
		spin_lock(&clock_map_lock);
		clock_map_enabled[BIT_WORD(clk->id)] &= ~BIT_MASK(clk->id);
		spin_unlock(&clock_map_lock);
	}
	spin_unlock_irqrestore(&clocks_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

int clk_reset(struct clk *clk, enum clk_reset_action action)
{
	int ret = -EPERM;

	
	if (clk->ops->reset != NULL)
		ret = clk->ops->reset(clk->id, action);
	if (ret == -EPERM && clk_ops_remote.reset != NULL)
		ret = clk_ops_remote.reset(clk->remote_id, action);

	return ret;
}
EXPORT_SYMBOL(clk_reset);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->ops->get_rate(clk->id);
}
EXPORT_SYMBOL(clk_get_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	return clk->ops->set_rate(clk->id, rate);
}
EXPORT_SYMBOL(clk_set_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	return clk->ops->round_rate(clk->id, rate);
}
EXPORT_SYMBOL(clk_round_rate);

int clk_set_min_rate(struct clk *clk, unsigned long rate)
{
	return clk->ops->set_min_rate(clk->id, rate);
}
EXPORT_SYMBOL(clk_set_min_rate);

int clk_set_max_rate(struct clk *clk, unsigned long rate)
{
	return clk->ops->set_max_rate(clk->id, rate);
}
EXPORT_SYMBOL(clk_set_max_rate);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	return -ENOSYS;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk)
{
	return ERR_PTR(-ENOSYS);
}
EXPORT_SYMBOL(clk_get_parent);

int clk_set_flags(struct clk *clk, unsigned long flags)
{
	if (clk == NULL || IS_ERR(clk))
		return -EINVAL;
	return clk->ops->set_flags(clk->id, flags);
}
EXPORT_SYMBOL(clk_set_flags);


static unsigned long ebi1_min_rate[CLKVOTE_MAX];
static struct clk *ebi1_clk;
static struct clk *pbus_clk;


int ebi1_clk_set_min_rate(enum clkvote_client client, unsigned long rate)
{
	static unsigned long last_set_val = -1;
	unsigned long new_val;
	unsigned long flags;
	int ret = 0, i;

	spin_lock_irqsave(&ebi1_vote_lock, flags);

	ebi1_min_rate[client] = (rate == MSM_AXI_MAX_FREQ) ?
				(clk_get_max_axi_khz() * 1000) : rate;

	new_val = ebi1_min_rate[0];
	for (i = 1; i < CLKVOTE_MAX; i++)
		if (ebi1_min_rate[i] > new_val)
			new_val = ebi1_min_rate[i];

	
	if (last_set_val != new_val) {
		ret = clk_set_min_rate(ebi1_clk, new_val);
		if (ret < 0) {
			pr_err("Setting EBI1 min rate to %lu Hz failed!\n",
				new_val);
			pr_err("Last successful value was %lu Hz.\n",
				last_set_val);
		} else {
			last_set_val = new_val;
		}
	}

	spin_unlock_irqrestore(&ebi1_vote_lock, flags);

	return ret;
}

static int axi_freq_notifier_handler(struct notifier_block *block,
				unsigned long min_freq, void *v)
{
	
	if (min_freq != MSM_AXI_MAX_FREQ)
		min_freq *= 1000;

	
	if (cpu_is_msm7x30() || cpu_is_msm8x55())
		return clk_set_min_rate(pbus_clk, min_freq/2);
	else
		return ebi1_clk_set_min_rate(CLKVOTE_PMQOS, min_freq);
}


int msm_clock_require_tcxo(unsigned long *reason, int nbits)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&clock_map_lock, flags);
	ret = !bitmap_empty(clock_map_enabled, NR_CLKS);
	if (nbits > 0)
		bitmap_copy(reason, clock_map_enabled, min(nbits, NR_CLKS));
	spin_unlock_irqrestore(&clock_map_lock, flags);

	return ret;
}


int msm_clock_get_name(uint32_t id, char *name, uint32_t size)
{
	struct clk *c_clk;
	int ret = -ENODEV;

	mutex_lock(&clocks_mutex);
	list_for_each_entry(c_clk, &clocks, list) {
		if (id == c_clk->id) {
			strlcpy(name, c_clk->name, size);
			ret = 0;
			break;
		}
	}
	mutex_unlock(&clocks_mutex);

	return ret;
}

void __init msm_clock_init(struct clk *clock_tbl, unsigned num_clocks)
{
	unsigned n;

	
	msm_clk_soc_init();

	spin_lock_init(&clocks_lock);
	mutex_lock(&clocks_mutex);
	msm_clocks = clock_tbl;
	msm_num_clocks = num_clocks;
	for (n = 0; n < msm_num_clocks; n++) {
		msm_clk_soc_set_ops(&msm_clocks[n]);
		list_add_tail(&msm_clocks[n].list, &clocks);
	}
	mutex_unlock(&clocks_mutex);

	ebi1_clk = clk_get(NULL, "ebi1_clk");
	BUG_ON(IS_ERR(ebi1_clk));
	if (cpu_is_msm7x30() || cpu_is_msm8x55()) {
		pbus_clk = clk_get(NULL, "pbus_clk");
		BUG_ON(IS_ERR(pbus_clk));
	}

	axi_freq_notifier_block.notifier_call = axi_freq_notifier_handler;
	pm_qos_add_notifier(PM_QOS_SYSTEM_BUS_FREQ, &axi_freq_notifier_block);
}


static int __init clock_late_init(void)
{
	unsigned long flags;
	struct clk *clk;
	unsigned count = 0;

	mutex_lock(&clocks_mutex);
	list_for_each_entry(clk, &clocks, list) {
		if (clk->flags & CLKFLAG_AUTO_OFF) {
			spin_lock_irqsave(&clocks_lock, flags);
			if (!clk->count) {
				count++;
				clk->ops->auto_off(clk->id);
			}
			spin_unlock_irqrestore(&clocks_lock, flags);
		}
	}
	mutex_unlock(&clocks_mutex);
	pr_info("clock_late_init() disabled %d unused clocks\n", count);
	return 0;
}

late_initcall(clock_late_init);
