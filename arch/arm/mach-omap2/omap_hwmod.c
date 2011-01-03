
#undef DEBUG

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/bootmem.h>

#include <mach/cpu.h>
#include <mach/clockdomain.h>
#include <mach/powerdomain.h>
#include <mach/clock.h>
#include <mach/omap_hwmod.h>

#include "cm.h"


#define MAX_MODULE_RESET_WAIT		10000


#define MPU_INITIATOR_NAME		"mpu_hwmod"


static LIST_HEAD(omap_hwmod_list);

static DEFINE_MUTEX(omap_hwmod_mutex);


static struct omap_hwmod *mpu_oh;


static u8 inited;





static int _update_sysc_cache(struct omap_hwmod *oh)
{
	if (!oh->sysconfig) {
		WARN(!oh->sysconfig, "omap_hwmod: %s: cannot read "
		     "OCP_SYSCONFIG: not defined on hwmod\n", oh->name);
		return -EINVAL;
	}

	

	oh->_sysc_cache = omap_hwmod_readl(oh, oh->sysconfig->sysc_offs);

	oh->_int_flags |= _HWMOD_SYSCONFIG_LOADED;

	return 0;
}


static void _write_sysconfig(u32 v, struct omap_hwmod *oh)
{
	if (!oh->sysconfig) {
		WARN(!oh->sysconfig, "omap_hwmod: %s: cannot write "
		     "OCP_SYSCONFIG: not defined on hwmod\n", oh->name);
		return;
	}

	

	if (oh->_sysc_cache != v) {
		oh->_sysc_cache = v;
		omap_hwmod_writel(v, oh, oh->sysconfig->sysc_offs);
	}
}


static int _set_master_standbymode(struct omap_hwmod *oh, u8 standbymode,
				   u32 *v)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_MIDLEMODE))
		return -EINVAL;

	*v &= ~SYSC_MIDLEMODE_MASK;
	*v |= __ffs(standbymode) << SYSC_MIDLEMODE_SHIFT;

	return 0;
}


static int _set_slave_idlemode(struct omap_hwmod *oh, u8 idlemode, u32 *v)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_SIDLEMODE))
		return -EINVAL;

	*v &= ~SYSC_SIDLEMODE_MASK;
	*v |= __ffs(idlemode) << SYSC_SIDLEMODE_SHIFT;

	return 0;
}


static int _set_clockactivity(struct omap_hwmod *oh, u8 clockact, u32 *v)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_CLOCKACTIVITY))
		return -EINVAL;

	*v &= ~SYSC_CLOCKACTIVITY_MASK;
	*v |= clockact << SYSC_CLOCKACTIVITY_SHIFT;

	return 0;
}


static int _set_softreset(struct omap_hwmod *oh, u32 *v)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_SOFTRESET))
		return -EINVAL;

	*v |= SYSC_SOFTRESET_MASK;

	return 0;
}


static int _enable_wakeup(struct omap_hwmod *oh)
{
	u32 v;

	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_ENAWAKEUP))
		return -EINVAL;

	v = oh->_sysc_cache;
	v |= SYSC_ENAWAKEUP_MASK;
	_write_sysconfig(v, oh);

	

	oh->_int_flags |= _HWMOD_WAKEUP_ENABLED;

	return 0;
}


static int _disable_wakeup(struct omap_hwmod *oh)
{
	u32 v;

	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_ENAWAKEUP))
		return -EINVAL;

	v = oh->_sysc_cache;
	v &= ~SYSC_ENAWAKEUP_MASK;
	_write_sysconfig(v, oh);

	

	oh->_int_flags &= ~_HWMOD_WAKEUP_ENABLED;

	return 0;
}


static int _add_initiator_dep(struct omap_hwmod *oh, struct omap_hwmod *init_oh)
{
	if (!oh->_clk)
		return -EINVAL;

	return pwrdm_add_sleepdep(oh->_clk->clkdm->pwrdm.ptr,
				  init_oh->_clk->clkdm->pwrdm.ptr);
}


static int _del_initiator_dep(struct omap_hwmod *oh, struct omap_hwmod *init_oh)
{
	if (!oh->_clk)
		return -EINVAL;

	return pwrdm_del_sleepdep(oh->_clk->clkdm->pwrdm.ptr,
				  init_oh->_clk->clkdm->pwrdm.ptr);
}


static int _init_main_clk(struct omap_hwmod *oh)
{
	struct clk *c;
	int ret = 0;

	if (!oh->clkdev_con_id)
		return 0;

	c = clk_get_sys(oh->clkdev_dev_id, oh->clkdev_con_id);
	WARN(IS_ERR(c), "omap_hwmod: %s: cannot clk_get main_clk %s.%s\n",
	     oh->name, oh->clkdev_dev_id, oh->clkdev_con_id);
	if (IS_ERR(c))
		ret = -EINVAL;
	oh->_clk = c;

	return ret;
}


static int _init_interface_clks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct clk *c;
	int i;
	int ret = 0;

	if (oh->slaves_cnt == 0)
		return 0;

	for (i = 0, os = *oh->slaves; i < oh->slaves_cnt; i++, os++) {
		if (!os->clkdev_con_id)
			continue;

		c = clk_get_sys(os->clkdev_dev_id, os->clkdev_con_id);
		WARN(IS_ERR(c), "omap_hwmod: %s: cannot clk_get "
		     "interface_clk %s.%s\n", oh->name,
		     os->clkdev_dev_id, os->clkdev_con_id);
		if (IS_ERR(c))
			ret = -EINVAL;
		os->_clk = c;
	}

	return ret;
}


static int _init_opt_clks(struct omap_hwmod *oh)
{
	struct omap_hwmod_opt_clk *oc;
	struct clk *c;
	int i;
	int ret = 0;

	for (i = oh->opt_clks_cnt, oc = oh->opt_clks; i > 0; i--, oc++) {
		c = clk_get_sys(oc->clkdev_dev_id, oc->clkdev_con_id);
		WARN(IS_ERR(c), "omap_hwmod: %s: cannot clk_get opt_clk "
		     "%s.%s\n", oh->name, oc->clkdev_dev_id,
		     oc->clkdev_con_id);
		if (IS_ERR(c))
			ret = -EINVAL;
		oc->_clk = c;
	}

	return ret;
}


static int _enable_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	int i;

	pr_debug("omap_hwmod: %s: enabling clocks\n", oh->name);

	if (oh->_clk && !IS_ERR(oh->_clk))
		clk_enable(oh->_clk);

	if (oh->slaves_cnt > 0) {
		for (i = 0, os = *oh->slaves; i < oh->slaves_cnt; i++, os++) {
			struct clk *c = os->_clk;

			if (c && !IS_ERR(c) && (os->flags & OCPIF_SWSUP_IDLE))
				clk_enable(c);
		}
	}

	

	return 0;
}


static int _disable_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	int i;

	pr_debug("omap_hwmod: %s: disabling clocks\n", oh->name);

	if (oh->_clk && !IS_ERR(oh->_clk))
		clk_disable(oh->_clk);

	if (oh->slaves_cnt > 0) {
		for (i = 0, os = *oh->slaves; i < oh->slaves_cnt; i++, os++) {
			struct clk *c = os->_clk;

			if (c && !IS_ERR(c) && (os->flags & OCPIF_SWSUP_IDLE))
				clk_disable(c);
		}
	}

	

	return 0;
}


static int _find_mpu_port_index(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	int i;
	int found = 0;

	if (!oh || oh->slaves_cnt == 0)
		return -EINVAL;

	for (i = 0, os = *oh->slaves; i < oh->slaves_cnt; i++, os++) {
		if (os->user & OCP_USER_MPU) {
			found = 1;
			break;
		}
	}

	if (found)
		pr_debug("omap_hwmod: %s: MPU OCP slave port ID  %d\n",
			 oh->name, i);
	else
		pr_debug("omap_hwmod: %s: no MPU OCP slave port found\n",
			 oh->name);

	return (found) ? i : -EINVAL;
}


static void __iomem *_find_mpu_rt_base(struct omap_hwmod *oh, u8 index)
{
	struct omap_hwmod_ocp_if *os;
	struct omap_hwmod_addr_space *mem;
	int i;
	int found = 0;

	if (!oh || oh->slaves_cnt == 0)
		return NULL;

	os = *oh->slaves + index;

	for (i = 0, mem = os->addr; i < os->addr_cnt; i++, mem++) {
		if (mem->flags & ADDR_TYPE_RT) {
			found = 1;
			break;
		}
	}

	

	if (found)
		pr_debug("omap_hwmod: %s: MPU register target at va %p\n",
			 oh->name, OMAP2_IO_ADDRESS(mem->pa_start));
	else
		pr_debug("omap_hwmod: %s: no MPU register target found\n",
			 oh->name);

	return (found) ? OMAP2_IO_ADDRESS(mem->pa_start) : NULL;
}


static void _sysc_enable(struct omap_hwmod *oh)
{
	u8 idlemode;
	u32 v;

	if (!oh->sysconfig)
		return;

	v = oh->_sysc_cache;

	if (oh->sysconfig->sysc_flags & SYSC_HAS_SIDLEMODE) {
		idlemode = (oh->flags & HWMOD_SWSUP_SIDLE) ?
			HWMOD_IDLEMODE_NO : HWMOD_IDLEMODE_SMART;
		_set_slave_idlemode(oh, idlemode, &v);
	}

	if (oh->sysconfig->sysc_flags & SYSC_HAS_MIDLEMODE) {
		idlemode = (oh->flags & HWMOD_SWSUP_MSTANDBY) ?
			HWMOD_IDLEMODE_NO : HWMOD_IDLEMODE_SMART;
		_set_master_standbymode(oh, idlemode, &v);
	}

	

	if (oh->flags & HWMOD_SET_DEFAULT_CLOCKACT &&
	    oh->sysconfig->sysc_flags & SYSC_HAS_CLOCKACTIVITY)
		_set_clockactivity(oh, oh->sysconfig->clockact, &v);

	_write_sysconfig(v, oh);
}


static void _sysc_idle(struct omap_hwmod *oh)
{
	u8 idlemode;
	u32 v;

	if (!oh->sysconfig)
		return;

	v = oh->_sysc_cache;

	if (oh->sysconfig->sysc_flags & SYSC_HAS_SIDLEMODE) {
		idlemode = (oh->flags & HWMOD_SWSUP_SIDLE) ?
			HWMOD_IDLEMODE_FORCE : HWMOD_IDLEMODE_SMART;
		_set_slave_idlemode(oh, idlemode, &v);
	}

	if (oh->sysconfig->sysc_flags & SYSC_HAS_MIDLEMODE) {
		idlemode = (oh->flags & HWMOD_SWSUP_MSTANDBY) ?
			HWMOD_IDLEMODE_FORCE : HWMOD_IDLEMODE_SMART;
		_set_master_standbymode(oh, idlemode, &v);
	}

	_write_sysconfig(v, oh);
}


static void _sysc_shutdown(struct omap_hwmod *oh)
{
	u32 v;

	if (!oh->sysconfig)
		return;

	v = oh->_sysc_cache;

	if (oh->sysconfig->sysc_flags & SYSC_HAS_SIDLEMODE)
		_set_slave_idlemode(oh, HWMOD_IDLEMODE_FORCE, &v);

	if (oh->sysconfig->sysc_flags & SYSC_HAS_MIDLEMODE)
		_set_master_standbymode(oh, HWMOD_IDLEMODE_FORCE, &v);

	

	_write_sysconfig(v, oh);
}


static struct omap_hwmod *_lookup(const char *name)
{
	struct omap_hwmod *oh, *temp_oh;

	oh = NULL;

	list_for_each_entry(temp_oh, &omap_hwmod_list, node) {
		if (!strcmp(name, temp_oh->name)) {
			oh = temp_oh;
			break;
		}
	}

	return oh;
}


static int _init_clocks(struct omap_hwmod *oh)
{
	int ret = 0;

	if (!oh || (oh->_state != _HWMOD_STATE_REGISTERED))
		return -EINVAL;

	pr_debug("omap_hwmod: %s: looking up clocks\n", oh->name);

	ret |= _init_main_clk(oh);
	ret |= _init_interface_clks(oh);
	ret |= _init_opt_clks(oh);

	oh->_state = _HWMOD_STATE_CLKS_INITED;

	return ret;
}


static int _wait_target_ready(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	int ret;

	if (!oh)
		return -EINVAL;

	if (oh->_int_flags & _HWMOD_NO_MPU_PORT)
		return 0;

	os = *oh->slaves + oh->_mpu_port_index;

	if (!(os->flags & OCPIF_HAS_IDLEST))
		return 0;

	

	

	if (cpu_is_omap24xx() || cpu_is_omap34xx()) {
		ret = omap2_cm_wait_module_ready(oh->prcm.omap2.module_offs,
						 oh->prcm.omap2.idlest_reg_id,
						 oh->prcm.omap2.idlest_idle_bit);
#if 0
	} else if (cpu_is_omap44xx()) {
		ret = omap4_cm_wait_module_ready(oh->prcm.omap4.module_offs,
						 oh->prcm.omap4.device_offs);
#endif
	} else {
		BUG();
	};

	return ret;
}


static int _reset(struct omap_hwmod *oh)
{
	u32 r, v;
	int c;

	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_SOFTRESET) ||
	    (oh->sysconfig->sysc_flags & SYSS_MISSING))
		return -EINVAL;

	
	if (oh->_state != _HWMOD_STATE_ENABLED) {
		WARN(1, "omap_hwmod: %s: reset can only be entered from "
		     "enabled state\n", oh->name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: resetting\n", oh->name);

	v = oh->_sysc_cache;
	r = _set_softreset(oh, &v);
	if (r)
		return r;
	_write_sysconfig(v, oh);

	c = 0;
	while (c < MAX_MODULE_RESET_WAIT &&
	       !(omap_hwmod_readl(oh, oh->sysconfig->syss_offs) &
		 SYSS_RESETDONE_MASK)) {
		udelay(1);
		c++;
	}

	if (c == MAX_MODULE_RESET_WAIT)
		WARN(1, "omap_hwmod: %s: failed to reset in %d usec\n",
		     oh->name, MAX_MODULE_RESET_WAIT);
	else
		pr_debug("omap_hwmod: %s: reset in %d usec\n", oh->name, c);

	

	return (c == MAX_MODULE_RESET_WAIT) ? -ETIMEDOUT : 0;
}


static int _enable(struct omap_hwmod *oh)
{
	int r;

	if (oh->_state != _HWMOD_STATE_INITIALIZED &&
	    oh->_state != _HWMOD_STATE_IDLE &&
	    oh->_state != _HWMOD_STATE_DISABLED) {
		WARN(1, "omap_hwmod: %s: enabled state can only be entered "
		     "from initialized, idle, or disabled state\n", oh->name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: enabling\n", oh->name);

	

	_add_initiator_dep(oh, mpu_oh);
	_enable_clocks(oh);

	if (oh->sysconfig) {
		if (!(oh->_int_flags & _HWMOD_SYSCONFIG_LOADED))
			_update_sysc_cache(oh);
		_sysc_enable(oh);
	}

	r = _wait_target_ready(oh);
	if (!r)
		oh->_state = _HWMOD_STATE_ENABLED;

	return r;
}


static int _idle(struct omap_hwmod *oh)
{
	if (oh->_state != _HWMOD_STATE_ENABLED) {
		WARN(1, "omap_hwmod: %s: idle state can only be entered from "
		     "enabled state\n", oh->name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: idling\n", oh->name);

	if (oh->sysconfig)
		_sysc_idle(oh);
	_del_initiator_dep(oh, mpu_oh);
	_disable_clocks(oh);

	oh->_state = _HWMOD_STATE_IDLE;

	return 0;
}


static int _shutdown(struct omap_hwmod *oh)
{
	if (oh->_state != _HWMOD_STATE_IDLE &&
	    oh->_state != _HWMOD_STATE_ENABLED) {
		WARN(1, "omap_hwmod: %s: disabled state can only be entered "
		     "from idle, or enabled state\n", oh->name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: disabling\n", oh->name);

	if (oh->sysconfig)
		_sysc_shutdown(oh);
	_del_initiator_dep(oh, mpu_oh);
	
	_disable_clocks(oh);
	

	

	oh->_state = _HWMOD_STATE_DISABLED;

	return 0;
}


static int _write_clockact_lock(struct omap_hwmod *oh, u8 clockact)
{
	u32 v;

	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_CLOCKACTIVITY))
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	v = oh->_sysc_cache;
	_set_clockactivity(oh, clockact, &v);
	_write_sysconfig(v, oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}



static int _setup(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	int i;

	if (!oh)
		return -EINVAL;

	
	if (oh->slaves_cnt > 0) {
		for (i = 0, os = *oh->slaves; i < oh->slaves_cnt; i++, os++) {
			struct clk *c = os->_clk;

			if (!c || IS_ERR(c))
				continue;

			if (os->flags & OCPIF_SWSUP_IDLE) {
				
			} else {
				
				clk_enable(c);
			}
		}
	}

	oh->_state = _HWMOD_STATE_INITIALIZED;

	_enable(oh);

	if (!(oh->flags & HWMOD_INIT_NO_RESET))
		_reset(oh);

	
	

	if (!(oh->flags & HWMOD_INIT_NO_IDLE))
		_idle(oh);

	return 0;
}





u32 omap_hwmod_readl(struct omap_hwmod *oh, u16 reg_offs)
{
	return __raw_readl(oh->_rt_va + reg_offs);
}

void omap_hwmod_writel(u32 v, struct omap_hwmod *oh, u16 reg_offs)
{
	__raw_writel(v, oh->_rt_va + reg_offs);
}


int omap_hwmod_register(struct omap_hwmod *oh)
{
	int ret, ms_id;

	if (!oh || (oh->_state != _HWMOD_STATE_UNKNOWN))
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);

	pr_debug("omap_hwmod: %s: registering\n", oh->name);

	if (_lookup(oh->name)) {
		ret = -EEXIST;
		goto ohr_unlock;
	}

	ms_id = _find_mpu_port_index(oh);
	if (!IS_ERR_VALUE(ms_id)) {
		oh->_mpu_port_index = ms_id;
		oh->_rt_va = _find_mpu_rt_base(oh, oh->_mpu_port_index);
	} else {
		oh->_int_flags |= _HWMOD_NO_MPU_PORT;
	}

	list_add_tail(&oh->node, &omap_hwmod_list);

	oh->_state = _HWMOD_STATE_REGISTERED;

	ret = 0;

ohr_unlock:
	mutex_unlock(&omap_hwmod_mutex);
	return ret;
}


struct omap_hwmod *omap_hwmod_lookup(const char *name)
{
	struct omap_hwmod *oh;

	if (!name)
		return NULL;

	mutex_lock(&omap_hwmod_mutex);
	oh = _lookup(name);
	mutex_unlock(&omap_hwmod_mutex);

	return oh;
}


int omap_hwmod_for_each(int (*fn)(struct omap_hwmod *oh))
{
	struct omap_hwmod *temp_oh;
	int ret;

	if (!fn)
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	list_for_each_entry(temp_oh, &omap_hwmod_list, node) {
		ret = (*fn)(temp_oh);
		if (ret)
			break;
	}
	mutex_unlock(&omap_hwmod_mutex);

	return ret;
}



int omap_hwmod_init(struct omap_hwmod **ohs)
{
	struct omap_hwmod *oh;
	int r;

	if (inited)
		return -EINVAL;

	inited = 1;

	if (!ohs)
		return 0;

	oh = *ohs;
	while (oh) {
		if (omap_chip_is(oh->omap_chip)) {
			r = omap_hwmod_register(oh);
			WARN(r, "omap_hwmod: %s: omap_hwmod_register returned "
			     "%d\n", oh->name, r);
		}
		oh = *++ohs;
	}

	return 0;
}


int omap_hwmod_late_init(void)
{
	int r;

	
	r = omap_hwmod_for_each(_init_clocks);
	WARN(r, "omap_hwmod: omap_hwmod_late_init(): _init_clocks failed\n");

	mpu_oh = omap_hwmod_lookup(MPU_INITIATOR_NAME);
	WARN(!mpu_oh, "omap_hwmod: could not find MPU initiator hwmod %s\n",
	     MPU_INITIATOR_NAME);

	omap_hwmod_for_each(_setup);

	return 0;
}


int omap_hwmod_unregister(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	pr_debug("omap_hwmod: %s: unregistering\n", oh->name);

	mutex_lock(&omap_hwmod_mutex);
	list_del(&oh->node);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


int omap_hwmod_enable(struct omap_hwmod *oh)
{
	int r;

	if (!oh)
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	r = _enable(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return r;
}


int omap_hwmod_idle(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	_idle(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


int omap_hwmod_shutdown(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	_shutdown(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


int omap_hwmod_enable_clocks(struct omap_hwmod *oh)
{
	mutex_lock(&omap_hwmod_mutex);
	_enable_clocks(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


int omap_hwmod_disable_clocks(struct omap_hwmod *oh)
{
	mutex_lock(&omap_hwmod_mutex);
	_disable_clocks(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


void omap_hwmod_ocp_barrier(struct omap_hwmod *oh)
{
	BUG_ON(!oh);

	if (!oh->sysconfig || !oh->sysconfig->sysc_flags) {
		WARN(1, "omap_device: %s: OCP barrier impossible due to "
		      "device configuration\n", oh->name);
		return;
	}

	
	omap_hwmod_readl(oh, oh->sysconfig->sysc_offs);
}


int omap_hwmod_reset(struct omap_hwmod *oh)
{
	int r;

	if (!oh || !(oh->_state & _HWMOD_STATE_ENABLED))
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	r = _reset(oh);
	if (!r)
		r = _enable(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return r;
}


int omap_hwmod_count_resources(struct omap_hwmod *oh)
{
	int ret, i;

	ret = oh->mpu_irqs_cnt + oh->sdma_chs_cnt;

	for (i = 0; i < oh->slaves_cnt; i++)
		ret += (*oh->slaves + i)->addr_cnt;

	return ret;
}


int omap_hwmod_fill_resources(struct omap_hwmod *oh, struct resource *res)
{
	int i, j;
	int r = 0;

	

	for (i = 0; i < oh->mpu_irqs_cnt; i++) {
		(res + r)->start = *(oh->mpu_irqs + i);
		(res + r)->end = *(oh->mpu_irqs + i);
		(res + r)->flags = IORESOURCE_IRQ;
		r++;
	}

	for (i = 0; i < oh->sdma_chs_cnt; i++) {
		(res + r)->name = (oh->sdma_chs + i)->name;
		(res + r)->start = (oh->sdma_chs + i)->dma_ch;
		(res + r)->end = (oh->sdma_chs + i)->dma_ch;
		(res + r)->flags = IORESOURCE_DMA;
		r++;
	}

	for (i = 0; i < oh->slaves_cnt; i++) {
		struct omap_hwmod_ocp_if *os;

		os = *oh->slaves + i;

		for (j = 0; j < os->addr_cnt; j++) {
			(res + r)->start = (os->addr + j)->pa_start;
			(res + r)->end = (os->addr + j)->pa_end;
			(res + r)->flags = IORESOURCE_MEM;
			r++;
		}
	}

	return r;
}


struct powerdomain *omap_hwmod_get_pwrdm(struct omap_hwmod *oh)
{
	struct clk *c;

	if (!oh)
		return NULL;

	if (oh->_clk) {
		c = oh->_clk;
	} else {
		if (oh->_int_flags & _HWMOD_NO_MPU_PORT)
			return NULL;
		c = oh->slaves[oh->_mpu_port_index]->_clk;
	}

	return c->clkdm->pwrdm.ptr;

}


int omap_hwmod_add_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh)
{
	return _add_initiator_dep(oh, init_oh);
}




int omap_hwmod_del_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh)
{
	return _del_initiator_dep(oh, init_oh);
}


int omap_hwmod_set_clockact_both(struct omap_hwmod *oh)
{
	return _write_clockact_lock(oh, CLOCKACT_TEST_BOTH);
}


int omap_hwmod_set_clockact_main(struct omap_hwmod *oh)
{
	return _write_clockact_lock(oh, CLOCKACT_TEST_MAIN);
}


int omap_hwmod_set_clockact_iclk(struct omap_hwmod *oh)
{
	return _write_clockact_lock(oh, CLOCKACT_TEST_ICLK);
}


int omap_hwmod_set_clockact_none(struct omap_hwmod *oh)
{
	return _write_clockact_lock(oh, CLOCKACT_TEST_NONE);
}


int omap_hwmod_enable_wakeup(struct omap_hwmod *oh)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_ENAWAKEUP))
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	_enable_wakeup(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}


int omap_hwmod_disable_wakeup(struct omap_hwmod *oh)
{
	if (!oh->sysconfig ||
	    !(oh->sysconfig->sysc_flags & SYSC_HAS_ENAWAKEUP))
		return -EINVAL;

	mutex_lock(&omap_hwmod_mutex);
	_disable_wakeup(oh);
	mutex_unlock(&omap_hwmod_mutex);

	return 0;
}
