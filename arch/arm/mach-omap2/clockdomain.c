
#ifdef CONFIG_OMAP_DEBUG_CLOCKDOMAIN
#  define DEBUG
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/limits.h>
#include <linux/err.h>

#include <linux/io.h>

#include <linux/bitops.h>

#include <mach/clock.h>

#include "prm.h"
#include "prm-regbits-24xx.h"
#include "cm.h"

#include <mach/powerdomain.h>
#include <mach/clockdomain.h>


static LIST_HEAD(clkdm_list);


static DEFINE_MUTEX(clkdm_mutex);


static struct clkdm_pwrdm_autodep *autodeps;





static void _autodep_lookup(struct clkdm_pwrdm_autodep *autodep)
{
	struct powerdomain *pwrdm;

	if (!autodep)
		return;

	if (!omap_chip_is(autodep->omap_chip))
		return;

	pwrdm = pwrdm_lookup(autodep->pwrdm.name);
	if (!pwrdm) {
		pr_err("clockdomain: autodeps: powerdomain %s does not exist\n",
			 autodep->pwrdm.name);
		pwrdm = ERR_PTR(-ENOENT);
	}
	autodep->pwrdm.ptr = pwrdm;
}


static void _clkdm_add_autodeps(struct clockdomain *clkdm)
{
	struct clkdm_pwrdm_autodep *autodep;

	for (autodep = autodeps; autodep->pwrdm.ptr; autodep++) {
		if (IS_ERR(autodep->pwrdm.ptr))
			continue;

		if (!omap_chip_is(autodep->omap_chip))
			continue;

		pr_debug("clockdomain: adding %s sleepdep/wkdep for "
			 "pwrdm %s\n", autodep->pwrdm.ptr->name,
			 clkdm->pwrdm.ptr->name);

		pwrdm_add_sleepdep(clkdm->pwrdm.ptr, autodep->pwrdm.ptr);
		pwrdm_add_wkdep(clkdm->pwrdm.ptr, autodep->pwrdm.ptr);
	}
}


static void _clkdm_del_autodeps(struct clockdomain *clkdm)
{
	struct clkdm_pwrdm_autodep *autodep;

	for (autodep = autodeps; autodep->pwrdm.ptr; autodep++) {
		if (IS_ERR(autodep->pwrdm.ptr))
			continue;

		if (!omap_chip_is(autodep->omap_chip))
			continue;

		pr_debug("clockdomain: removing %s sleepdep/wkdep for "
			 "pwrdm %s\n", autodep->pwrdm.ptr->name,
			 clkdm->pwrdm.ptr->name);

		pwrdm_del_sleepdep(clkdm->pwrdm.ptr, autodep->pwrdm.ptr);
		pwrdm_del_wkdep(clkdm->pwrdm.ptr, autodep->pwrdm.ptr);
	}
}


static void _omap2_clkdm_set_hwsup(struct clockdomain *clkdm, int enable)
{
	u32 v;

	if (cpu_is_omap24xx()) {
		if (enable)
			v = OMAP24XX_CLKSTCTRL_ENABLE_AUTO;
		else
			v = OMAP24XX_CLKSTCTRL_DISABLE_AUTO;
	} else if (cpu_is_omap34xx()) {
		if (enable)
			v = OMAP34XX_CLKSTCTRL_ENABLE_AUTO;
		else
			v = OMAP34XX_CLKSTCTRL_DISABLE_AUTO;
	} else {
		BUG();
	}

	cm_rmw_mod_reg_bits(clkdm->clktrctrl_mask,
			    v << __ffs(clkdm->clktrctrl_mask),
			    clkdm->pwrdm.ptr->prcm_offs, CM_CLKSTCTRL);
}

static struct clockdomain *_clkdm_lookup(const char *name)
{
	struct clockdomain *clkdm, *temp_clkdm;

	if (!name)
		return NULL;

	clkdm = NULL;

	list_for_each_entry(temp_clkdm, &clkdm_list, node) {
		if (!strcmp(name, temp_clkdm->name)) {
			clkdm = temp_clkdm;
			break;
		}
	}

	return clkdm;
}





void clkdm_init(struct clockdomain **clkdms,
		struct clkdm_pwrdm_autodep *init_autodeps)
{
	struct clockdomain **c = NULL;
	struct clkdm_pwrdm_autodep *autodep = NULL;

	if (clkdms)
		for (c = clkdms; *c; c++)
			clkdm_register(*c);

	autodeps = init_autodeps;
	if (autodeps)
		for (autodep = autodeps; autodep->pwrdm.ptr; autodep++)
			_autodep_lookup(autodep);
}


int clkdm_register(struct clockdomain *clkdm)
{
	int ret = -EINVAL;
	struct powerdomain *pwrdm;

	if (!clkdm || !clkdm->name)
		return -EINVAL;

	if (!omap_chip_is(clkdm->omap_chip))
		return -EINVAL;

	pwrdm = pwrdm_lookup(clkdm->pwrdm.name);
	if (!pwrdm) {
		pr_err("clockdomain: %s: powerdomain %s does not exist\n",
			clkdm->name, clkdm->pwrdm.name);
		return -EINVAL;
	}
	clkdm->pwrdm.ptr = pwrdm;

	mutex_lock(&clkdm_mutex);
	
	if (_clkdm_lookup(clkdm->name)) {
		ret = -EEXIST;
		goto cr_unlock;
	}

	list_add(&clkdm->node, &clkdm_list);

	pwrdm_add_clkdm(pwrdm, clkdm);

	pr_debug("clockdomain: registered %s\n", clkdm->name);
	ret = 0;

cr_unlock:
	mutex_unlock(&clkdm_mutex);

	return ret;
}


int clkdm_unregister(struct clockdomain *clkdm)
{
	if (!clkdm)
		return -EINVAL;

	pwrdm_del_clkdm(clkdm->pwrdm.ptr, clkdm);

	mutex_lock(&clkdm_mutex);
	list_del(&clkdm->node);
	mutex_unlock(&clkdm_mutex);

	pr_debug("clockdomain: unregistered %s\n", clkdm->name);

	return 0;
}


struct clockdomain *clkdm_lookup(const char *name)
{
	struct clockdomain *clkdm, *temp_clkdm;

	if (!name)
		return NULL;

	clkdm = NULL;

	mutex_lock(&clkdm_mutex);
	list_for_each_entry(temp_clkdm, &clkdm_list, node) {
		if (!strcmp(name, temp_clkdm->name)) {
			clkdm = temp_clkdm;
			break;
		}
	}
	mutex_unlock(&clkdm_mutex);

	return clkdm;
}


int clkdm_for_each(int (*fn)(struct clockdomain *clkdm, void *user),
			void *user)
{
	struct clockdomain *clkdm;
	int ret = 0;

	if (!fn)
		return -EINVAL;

	mutex_lock(&clkdm_mutex);
	list_for_each_entry(clkdm, &clkdm_list, node) {
		ret = (*fn)(clkdm, user);
		if (ret)
			break;
	}
	mutex_unlock(&clkdm_mutex);

	return ret;
}



struct powerdomain *clkdm_get_pwrdm(struct clockdomain *clkdm)
{
	if (!clkdm)
		return NULL;

	return clkdm->pwrdm.ptr;
}





static int omap2_clkdm_clktrctrl_read(struct clockdomain *clkdm)
{
	u32 v;

	if (!clkdm)
		return -EINVAL;

	v = cm_read_mod_reg(clkdm->pwrdm.ptr->prcm_offs, CM_CLKSTCTRL);
	v &= clkdm->clktrctrl_mask;
	v >>= __ffs(clkdm->clktrctrl_mask);

	return v;
}


int omap2_clkdm_sleep(struct clockdomain *clkdm)
{
	if (!clkdm)
		return -EINVAL;

	if (!(clkdm->flags & CLKDM_CAN_FORCE_SLEEP)) {
		pr_debug("clockdomain: %s does not support forcing "
			 "sleep via software\n", clkdm->name);
		return -EINVAL;
	}

	pr_debug("clockdomain: forcing sleep on %s\n", clkdm->name);

	if (cpu_is_omap24xx()) {

		cm_set_mod_reg_bits(OMAP24XX_FORCESTATE,
				    clkdm->pwrdm.ptr->prcm_offs, PM_PWSTCTRL);

	} else if (cpu_is_omap34xx()) {

		u32 v = (OMAP34XX_CLKSTCTRL_FORCE_SLEEP <<
			 __ffs(clkdm->clktrctrl_mask));

		cm_rmw_mod_reg_bits(clkdm->clktrctrl_mask, v,
				    clkdm->pwrdm.ptr->prcm_offs, CM_CLKSTCTRL);

	} else {
		BUG();
	};

	return 0;
}


int omap2_clkdm_wakeup(struct clockdomain *clkdm)
{
	if (!clkdm)
		return -EINVAL;

	if (!(clkdm->flags & CLKDM_CAN_FORCE_WAKEUP)) {
		pr_debug("clockdomain: %s does not support forcing "
			 "wakeup via software\n", clkdm->name);
		return -EINVAL;
	}

	pr_debug("clockdomain: forcing wakeup on %s\n", clkdm->name);

	if (cpu_is_omap24xx()) {

		cm_clear_mod_reg_bits(OMAP24XX_FORCESTATE,
				      clkdm->pwrdm.ptr->prcm_offs, PM_PWSTCTRL);

	} else if (cpu_is_omap34xx()) {

		u32 v = (OMAP34XX_CLKSTCTRL_FORCE_WAKEUP <<
			 __ffs(clkdm->clktrctrl_mask));

		cm_rmw_mod_reg_bits(clkdm->clktrctrl_mask, v,
				    clkdm->pwrdm.ptr->prcm_offs, CM_CLKSTCTRL);

	} else {
		BUG();
	};

	return 0;
}


void omap2_clkdm_allow_idle(struct clockdomain *clkdm)
{
	if (!clkdm)
		return;

	if (!(clkdm->flags & CLKDM_CAN_ENABLE_AUTO)) {
		pr_debug("clock: automatic idle transitions cannot be enabled "
			 "on clockdomain %s\n", clkdm->name);
		return;
	}

	pr_debug("clockdomain: enabling automatic idle transitions for %s\n",
		 clkdm->name);

	if (atomic_read(&clkdm->usecount) > 0)
		_clkdm_add_autodeps(clkdm);

	_omap2_clkdm_set_hwsup(clkdm, 1);

	pwrdm_clkdm_state_switch(clkdm);
}


void omap2_clkdm_deny_idle(struct clockdomain *clkdm)
{
	if (!clkdm)
		return;

	if (!(clkdm->flags & CLKDM_CAN_DISABLE_AUTO)) {
		pr_debug("clockdomain: automatic idle transitions cannot be "
			 "disabled on %s\n", clkdm->name);
		return;
	}

	pr_debug("clockdomain: disabling automatic idle transitions for %s\n",
		 clkdm->name);

	_omap2_clkdm_set_hwsup(clkdm, 0);

	if (atomic_read(&clkdm->usecount) > 0)
		_clkdm_del_autodeps(clkdm);
}





int omap2_clkdm_clk_enable(struct clockdomain *clkdm, struct clk *clk)
{
	int v;

	

	if (!clkdm || !clk)
		return -EINVAL;

	if (atomic_inc_return(&clkdm->usecount) > 1)
		return 0;

	

	pr_debug("clockdomain: clkdm %s: clk %s now enabled\n", clkdm->name,
		 clk->name);

	v = omap2_clkdm_clktrctrl_read(clkdm);

	if ((cpu_is_omap34xx() && v == OMAP34XX_CLKSTCTRL_ENABLE_AUTO) ||
	    (cpu_is_omap24xx() && v == OMAP24XX_CLKSTCTRL_ENABLE_AUTO)) {
		
		_omap2_clkdm_set_hwsup(clkdm, 0);
		_clkdm_add_autodeps(clkdm);
		_omap2_clkdm_set_hwsup(clkdm, 1);
	} else {
		omap2_clkdm_wakeup(clkdm);
	}

	pwrdm_wait_transition(clkdm->pwrdm.ptr);
	pwrdm_clkdm_state_switch(clkdm);

	return 0;
}


int omap2_clkdm_clk_disable(struct clockdomain *clkdm, struct clk *clk)
{
	int v;

	

	if (!clkdm || !clk)
		return -EINVAL;

#ifdef DEBUG
	if (atomic_read(&clkdm->usecount) == 0) {
		WARN_ON(1); 
		return -ERANGE;
	}
#endif

	if (atomic_dec_return(&clkdm->usecount) > 0)
		return 0;

	

	pr_debug("clockdomain: clkdm %s: clk %s now disabled\n", clkdm->name,
		 clk->name);

	v = omap2_clkdm_clktrctrl_read(clkdm);

	if ((cpu_is_omap34xx() && v == OMAP34XX_CLKSTCTRL_ENABLE_AUTO) ||
	    (cpu_is_omap24xx() && v == OMAP24XX_CLKSTCTRL_ENABLE_AUTO)) {
		
		_omap2_clkdm_set_hwsup(clkdm, 0);
		_clkdm_del_autodeps(clkdm);
		_omap2_clkdm_set_hwsup(clkdm, 1);
	} else {
		omap2_clkdm_sleep(clkdm);
	}

	pwrdm_clkdm_state_switch(clkdm);

	return 0;
}

