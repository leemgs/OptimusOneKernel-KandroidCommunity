

#ifndef ASM_ARM_ARCH_OMAP_POWERDOMAIN
#define ASM_ARM_ARCH_OMAP_POWERDOMAIN

#include <linux/types.h>
#include <linux/list.h>

#include <asm/atomic.h>

#include <mach/cpu.h>



#define PWRDM_POWER_OFF		0x0
#define PWRDM_POWER_RET		0x1
#define PWRDM_POWER_INACTIVE	0x2
#define PWRDM_POWER_ON		0x3


#define PWRSTS_OFF_ON		((1 << PWRDM_POWER_OFF) | \
				 (1 << PWRDM_POWER_ON))

#define PWRSTS_OFF_RET		((1 << PWRDM_POWER_OFF) | \
				 (1 << PWRDM_POWER_RET))

#define PWRSTS_OFF_RET_ON	(PWRSTS_OFF_RET | (1 << PWRDM_POWER_ON))



#define PWRDM_HAS_HDWR_SAR	(1 << 0) 



#define PWRDM_MAX_MEM_BANKS	4


#define PWRDM_MAX_CLKDMS	4


#define PWRDM_TRANSITION_BAILOUT 100000

struct clockdomain;
struct powerdomain;


struct pwrdm_dep {

	
	const char *pwrdm_name;

	
	struct powerdomain *pwrdm;

	
	const struct omap_chip_id omap_chip;

};

struct powerdomain {

	
	const char *name;

	
	const s16 prcm_offs;

	
	const struct omap_chip_id omap_chip;

	
	const u8 dep_bit;

	
	struct pwrdm_dep *wkdep_srcs;

	
	struct pwrdm_dep *sleepdep_srcs;

	
	const u8 pwrsts;

	
	const u8 pwrsts_logic_ret;

	
	const u8 flags;

	
	const u8 banks;

	
	const u8 pwrsts_mem_ret[PWRDM_MAX_MEM_BANKS];

	
	const u8 pwrsts_mem_on[PWRDM_MAX_MEM_BANKS];

	
	struct clockdomain *pwrdm_clkdms[PWRDM_MAX_CLKDMS];

	struct list_head node;

	int state;
	unsigned state_counter[4];

#ifdef CONFIG_PM_DEBUG
	s64 timer;
	s64 state_timer[4];
#endif
};


void pwrdm_init(struct powerdomain **pwrdm_list);

int pwrdm_register(struct powerdomain *pwrdm);
int pwrdm_unregister(struct powerdomain *pwrdm);
struct powerdomain *pwrdm_lookup(const char *name);

int pwrdm_for_each(int (*fn)(struct powerdomain *pwrdm, void *user),
			void *user);
int pwrdm_for_each_nolock(int (*fn)(struct powerdomain *pwrdm, void *user),
			void *user);

int pwrdm_add_clkdm(struct powerdomain *pwrdm, struct clockdomain *clkdm);
int pwrdm_del_clkdm(struct powerdomain *pwrdm, struct clockdomain *clkdm);
int pwrdm_for_each_clkdm(struct powerdomain *pwrdm,
			 int (*fn)(struct powerdomain *pwrdm,
				   struct clockdomain *clkdm));

int pwrdm_add_wkdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);
int pwrdm_del_wkdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);
int pwrdm_read_wkdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);
int pwrdm_add_sleepdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);
int pwrdm_del_sleepdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);
int pwrdm_read_sleepdep(struct powerdomain *pwrdm1, struct powerdomain *pwrdm2);

int pwrdm_get_mem_bank_count(struct powerdomain *pwrdm);

int pwrdm_set_next_pwrst(struct powerdomain *pwrdm, u8 pwrst);
int pwrdm_read_next_pwrst(struct powerdomain *pwrdm);
int pwrdm_read_pwrst(struct powerdomain *pwrdm);
int pwrdm_read_prev_pwrst(struct powerdomain *pwrdm);
int pwrdm_clear_all_prev_pwrst(struct powerdomain *pwrdm);

int pwrdm_set_logic_retst(struct powerdomain *pwrdm, u8 pwrst);
int pwrdm_set_mem_onst(struct powerdomain *pwrdm, u8 bank, u8 pwrst);
int pwrdm_set_mem_retst(struct powerdomain *pwrdm, u8 bank, u8 pwrst);

int pwrdm_read_logic_pwrst(struct powerdomain *pwrdm);
int pwrdm_read_prev_logic_pwrst(struct powerdomain *pwrdm);
int pwrdm_read_mem_pwrst(struct powerdomain *pwrdm, u8 bank);
int pwrdm_read_prev_mem_pwrst(struct powerdomain *pwrdm, u8 bank);

int pwrdm_enable_hdwr_sar(struct powerdomain *pwrdm);
int pwrdm_disable_hdwr_sar(struct powerdomain *pwrdm);
bool pwrdm_has_hdwr_sar(struct powerdomain *pwrdm);

int pwrdm_wait_transition(struct powerdomain *pwrdm);

int pwrdm_state_switch(struct powerdomain *pwrdm);
int pwrdm_clkdm_state_switch(struct clockdomain *clkdm);
int pwrdm_pre_transition(void);
int pwrdm_post_transition(void);

#endif
