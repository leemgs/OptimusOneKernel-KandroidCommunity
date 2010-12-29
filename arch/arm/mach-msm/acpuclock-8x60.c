

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>

#include <asm/cpu.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>

#include "acpuclock.h"

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)


#define SHOT_SWITCH		4
#define HOP_SWITCH		5
#define SIMPLE_SLEW		6
#define COMPLEX_SLEW		7


#define L_VAL_SCPLL_CAL_MIN	0x08 
#define L_VAL_SCPLL_CAL_MAX	0x1C 


#define SCPLL_POWER_DOWN	0
#define SCPLL_BYPASS		1
#define SCPLL_STANDBY		2
#define SCPLL_FULL_CAL		4
#define SCPLL_HALF_CAL		5
#define SCPLL_STEP_CAL		6
#define SCPLL_NORMAL		7

#define SCPLL_DEBUG_NONE	0
#define SCPLL_DEBUG_FULL	3


#define SCPLL_DEBUG_OFFSET		0x0
#define SCPLL_CTL_OFFSET		0x4
#define SCPLL_CAL_OFFSET		0x8
#define SCPLL_STATUS_OFFSET		0x10
#define SCPLL_CFG_OFFSET		0x1C
#define SCPLL_FSM_CTL_EXT_OFFSET	0x24
#define SCPLL_LUT_A_HW_MAX		(0x38 + ((L_VAL_SCPLL_CAL_MAX / 4) * 4))


#define SPSS0_CLK_CTL_ADDR		(MSM_ACC0_BASE + 0x04)
#define SPSS0_CLK_SEL_ADDR		(MSM_ACC0_BASE + 0x08)
#define SPSS1_CLK_CTL_ADDR		(MSM_ACC1_BASE + 0x04)
#define SPSS1_CLK_SEL_ADDR		(MSM_ACC1_BASE + 0x08)
#define SPSS_L2_CLK_SEL_ADDR		(MSM_GCC_BASE  + 0x38)
static const void * const clk_ctl_addr[] = {SPSS0_CLK_CTL_ADDR,
			SPSS1_CLK_CTL_ADDR};
static const void * const clk_sel_addr[] = {SPSS0_CLK_SEL_ADDR,
			SPSS1_CLK_SEL_ADDR, SPSS_L2_CLK_SEL_ADDR};

enum scplls {
	CPU0 = 0,
	CPU1,
	L2,
};

static const void * const sc_pll_base[] = {
	[CPU0]	= MSM_SCPLL_BASE + 0x200,
	[CPU1]	= MSM_SCPLL_BASE + 0x300,
	[L2]	= MSM_SCPLL_BASE + 0x400,
};

enum sc_src {
	ACPU_AFAB,
	ACPU_PLL_8,
	ACPU_SCPLL,
};

static struct clock_state {
	struct clkctl_acpu_speed	*current_speed[NR_CPUS];
	struct clkctl_acpu_speed	*current_l2_speed;
	struct mutex			lock;
	uint32_t			acpu_switch_time_us;
	uint32_t			max_speed_delta_khz;
	unsigned int			max_vdd;
} drv_state;

struct clkctl_acpu_speed {
	unsigned int     use_for_scaling[NR_CPUS];
	unsigned int     acpuclk_khz;
	int              pll;
	unsigned int     acpuclk_src_sel;
	unsigned int     acpuclk_src_div;
	unsigned int     core_src_sel;
	unsigned int     sc_l_val;
	int              vdd;
	unsigned int     l2_src_sel;
	unsigned int     l2_l_val;
	unsigned long    lpj; 
	struct clkctl_acpu_speed *up;
	struct clkctl_acpu_speed *down;
};

#define AFAB_IDX 1

static struct clkctl_acpu_speed acpu_freq_tbl[] = {
	{ {0, 0},  192000, ACPU_PLL_8, 3, 1, 0, 0,    1100, 0, 0    },
	{ {1, 1},  262000, ACPU_AFAB,  1, 0, 0, 0,    1100, 0, 0    },
	{ {1, 1},  384000, ACPU_PLL_8, 3, 0, 0, 0,    1100, 0, 0    },
	{ {1, 1},  432000, ACPU_SCPLL, 0, 0, 1, 0x08, 1100, 1, 0x08 },
	{ {1, 1},  648000, ACPU_SCPLL, 0, 0, 1, 0x0C, 1100, 1, 0x0C },
	{ {1, 1},  810000, ACPU_SCPLL, 0, 0, 1, 0x0F, 1100, 1, 0x0F },
	{ {0, 0},  972000, ACPU_SCPLL, 0, 0, 1, 0x12, 1100, 1, 0x10 },
	{ {1, 1}, 1080000, ACPU_SCPLL, 0, 0, 1, 0x14, 1100, 1, 0x10 },
	{ {0, 0}, 1188000, ACPU_SCPLL, 0, 0, 1, 0x16, 1100, 1, 0x10 },
	{ {0, 0}, 0 },
};

unsigned long acpuclk_get_rate(int cpu)
{
	return drv_state.current_speed[cpu]->acpuclk_khz;
}

uint32_t acpuclk_get_switch_time(void)
{
	return drv_state.acpu_switch_time_us;
}

#define POWER_COLLAPSE_KHZ 262000
unsigned long acpuclk_power_collapse(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), POWER_COLLAPSE_KHZ, SETRATE_PC);
	return ret;
}

#define WAIT_FOR_IRQ_KHZ 262000
unsigned long acpuclk_wait_for_irq(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), WAIT_FOR_IRQ_KHZ, SETRATE_SWFI);
	return ret;
}

static void select_core_source(unsigned int id, unsigned int src)
{
	uint32_t regval;
	int shift;

	shift = (id == L2) ? 0 : 1;
	regval = readl(clk_sel_addr[id]);
	regval &= ~(0x3 << shift);
	regval |= (src << shift);
	writel(regval, clk_sel_addr[id]);
}

static void select_clk_source_div(unsigned int id, struct clkctl_acpu_speed *s)
{
	uint32_t reg_clksel, reg_clkctl, src_sel;

	
	if (s->core_src_sel == 0) {

		reg_clksel = readl(clk_sel_addr[id]);

		
		src_sel = reg_clksel & 1;

		
		reg_clkctl = readl(clk_ctl_addr[id]);
		reg_clkctl &= ~(0xFF << (8 * src_sel));
		reg_clkctl |= s->acpuclk_src_sel << (4 + 8 * src_sel);
		reg_clkctl |= s->acpuclk_src_div << (0 + 8 * src_sel);
		writel(reg_clkctl, clk_ctl_addr[id]);

		
		reg_clksel ^= 1;

		
		writel(reg_clksel, clk_sel_addr[id]);
	}
}

static void scpll_enable(int sc_pll, uint32_t l_val)
{
	uint32_t regval;

	
	writel(SCPLL_STANDBY, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(10);

	
	regval = (l_val << 3) | SHOT_SWITCH;
	writel(regval, sc_pll_base[sc_pll] + SCPLL_FSM_CTL_EXT_OFFSET);
	writel(SCPLL_NORMAL, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(20);
}

static void scpll_disable(int sc_pll)
{
	
	writel(SCPLL_POWER_DOWN, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
}

static void scpll_change_freq(int sc_pll, uint32_t l_val)
{
	uint32_t regval;
	const void *base_addr = sc_pll_base[sc_pll];

	
	regval = (l_val << 3) | COMPLEX_SLEW;
	writel(regval, base_addr + SCPLL_FSM_CTL_EXT_OFFSET);
	writel(SCPLL_NORMAL, base_addr + SCPLL_CTL_OFFSET);

	
	while (readl(base_addr + SCPLL_STATUS_OFFSET) & 0x1)
		cpu_relax();
}

static int acpuclk_set_vdd_level(int vdd)
{
	
	return 0;
}

static void l2_set_speed(struct clkctl_acpu_speed *tgt_s)
{
	if (drv_state.current_l2_speed->l2_src_sel == 1
				&& tgt_s->l2_src_sel == 1)
		scpll_change_freq(L2, tgt_s->l2_l_val);
	else {
		if (tgt_s->l2_src_sel == 1) {
			scpll_enable(L2, tgt_s->l2_l_val);
			mb();
			select_core_source(L2, tgt_s->l2_src_sel);
		} else {
			select_core_source(L2, tgt_s->l2_src_sel);
			mb();
			scpll_disable(L2);
		}
	}
	drv_state.current_l2_speed = tgt_s;
}


static void update_l2_speed(unsigned int voting_cpu,
			    struct clkctl_acpu_speed *speed)
{
	int cpu;
	struct clkctl_acpu_speed *max;
	static struct clkctl_acpu_speed *l2_vote[NR_CPUS] = {
		acpu_freq_tbl,
		acpu_freq_tbl,
	};

	
	l2_vote[voting_cpu] = speed;

	
	max = acpu_freq_tbl;
	for_each_online_cpu(cpu)
		if (l2_vote[cpu]->l2_l_val > max->l2_l_val)
			max = l2_vote[cpu];

	
	if (max->l2_l_val != drv_state.current_l2_speed->l2_l_val)
		l2_set_speed(max);
}

static void switch_sc_speed(int cpu, struct clkctl_acpu_speed *tgt_s)
{
	struct clkctl_acpu_speed *strt_s = drv_state.current_speed[cpu];

	if (strt_s->pll != ACPU_SCPLL && tgt_s->pll != ACPU_SCPLL) {
		select_clk_source_div(cpu, tgt_s);
		
		select_core_source(cpu, tgt_s->core_src_sel);
	} else if (strt_s->pll != ACPU_SCPLL && tgt_s->pll == ACPU_SCPLL) {
		scpll_enable(cpu, tgt_s->sc_l_val);
		mb();
		select_core_source(cpu, tgt_s->core_src_sel);
	} else if (strt_s->pll == ACPU_SCPLL && tgt_s->pll != ACPU_SCPLL) {
		select_clk_source_div(cpu, tgt_s);
		select_core_source(cpu, tgt_s->core_src_sel);
		mb();
		scpll_disable(cpu);
	} else
		scpll_change_freq(cpu, tgt_s->sc_l_val);

	
	drv_state.current_speed[cpu] = tgt_s;

	
	per_cpu(cpu_data, cpu).loops_per_jiffy = tgt_s->lpj;

	
	loops_per_jiffy = tgt_s->lpj;
	for_each_online_cpu(cpu)
		if (per_cpu(cpu_data, cpu).loops_per_jiffy > loops_per_jiffy)
			loops_per_jiffy =
				per_cpu(cpu_data, cpu).loops_per_jiffy;
}

int acpuclk_set_rate(int cpu, unsigned long rate, enum setrate_reason reason)
{
	struct clkctl_acpu_speed *cur_s, *tgt_s, *strt_s;
	int res, rc = 0;
	int freq_index = 0;

	if (cpu > num_possible_cpus()) {
		rc = -EINVAL;
		goto out;
	}

	if (reason == SETRATE_CPUFREQ)
		mutex_lock(&drv_state.lock);

	strt_s = drv_state.current_speed[cpu];

	
	if (rate == strt_s->acpuclk_khz)
		goto out;

	
	for (tgt_s = acpu_freq_tbl; tgt_s->acpuclk_khz != 0; tgt_s++) {
		if (tgt_s->acpuclk_khz == rate)
			break;
		freq_index++;
	}
	if (tgt_s->acpuclk_khz == 0) {
		rc = -EINVAL;
		goto out;
	}

	if (reason == SETRATE_CPUFREQ) {
		
		if (tgt_s->vdd > strt_s->vdd) {
			rc = acpuclk_set_vdd_level(tgt_s->vdd);
			if (rc) {
				pr_err("Unable to increase ACPU vdd (%d)\n",
					rc);
				goto out;
			}
		}
	}

	dprintk("Switching from ACPU%d rate %u KHz -> %u KHz\n",
		cpu, strt_s->acpuclk_khz, tgt_s->acpuclk_khz);

	
	cur_s = strt_s;
	while (cur_s != tgt_s) {
		int d = abs((int)(cur_s->acpuclk_khz - tgt_s->acpuclk_khz));
		if (d > drv_state.max_speed_delta_khz) {
			if (tgt_s->acpuclk_khz > cur_s->acpuclk_khz)
				cur_s = cur_s->up;
			else
				cur_s = cur_s->down;
		} else
			cur_s = tgt_s;
		switch_sc_speed(cpu, cur_s);
	}

	
	update_l2_speed(cpu, tgt_s);

	
	if (reason == SETRATE_SWFI)
		goto out;

	
	if (reason == SETRATE_PC)
		goto out;

	
	if (tgt_s->vdd < strt_s->vdd) {
		res = acpuclk_set_vdd_level(tgt_s->vdd);
		if (res)
			pr_warning("Unable to drop ACPU vdd (%d)\n", res);
	}

	dprintk("ACPU%d speed change complete\n", cpu);
out:
	if (reason == SETRATE_CPUFREQ)
		mutex_unlock(&drv_state.lock);
	return rc;
}

static void __init scpll_init(int sc_pll)
{
	uint32_t regval;

	dprintk("Initializing SCPLL%d\n", sc_pll);

	
	writel(SCPLL_DEBUG_FULL, sc_pll_base[sc_pll] + SCPLL_DEBUG_OFFSET);
	writel(0x0, sc_pll_base[sc_pll] + SCPLL_LUT_A_HW_MAX);
	writel(SCPLL_DEBUG_NONE, sc_pll_base[sc_pll] + SCPLL_DEBUG_OFFSET);

	
	writel(SCPLL_STANDBY, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(10);

	
	regval = (L_VAL_SCPLL_CAL_MAX << 24) | (L_VAL_SCPLL_CAL_MIN << 16);
	writel(regval, sc_pll_base[sc_pll] + SCPLL_CAL_OFFSET);

	
	writel(SCPLL_FULL_CAL, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);

	
	while (readl(sc_pll_base[sc_pll] + SCPLL_LUT_A_HW_MAX) == 0)
		cpu_relax();

	
	while (readl(sc_pll_base[sc_pll] + SCPLL_STATUS_OFFSET) & 0x2)
		cpu_relax();

	
	scpll_disable(sc_pll);
}

static void __init precompute_stepping(void)
{
	int i, step_idx;

#define cur_freq acpu_freq_tbl[i].acpuclk_khz
#define step_freq acpu_freq_tbl[step_idx].acpuclk_khz
#define cur_pll acpu_freq_tbl[i].pll

	for (i = 0; acpu_freq_tbl[i].acpuclk_khz; i++) {

		
		step_idx = i + 1;
		while (step_freq && (step_freq - cur_freq)
					<= drv_state.max_speed_delta_khz) {
			acpu_freq_tbl[i].up = &acpu_freq_tbl[step_idx];
			step_idx++;
		}
		if (step_idx == (i + 1) && step_freq) {
			pr_crit("Delta between freqs %u KHz and %u KHz is"
				" too high!\n", cur_freq, step_freq);
			BUG();
		}

		
		step_idx = i - 1;
		while (step_idx >= 0 && (cur_freq - step_freq)
					<= drv_state.max_speed_delta_khz) {
			acpu_freq_tbl[i].down =	&acpu_freq_tbl[step_idx];
			step_idx--;
		}
		if (step_idx == (i - 1) && i > 0) {
			pr_crit("Delta between freqs %u KHz and %u KHz is"
				" too high!\n", cur_freq, step_freq);
			BUG();
		}
	}
}


static void __init force_all_to_afab(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		select_clk_source_div(cpu, &acpu_freq_tbl[AFAB_IDX]);
		select_core_source(cpu, 0);
		drv_state.current_speed[cpu] = &acpu_freq_tbl[AFAB_IDX];
	}

	select_core_source(L2, 0);
	drv_state.current_l2_speed = &acpu_freq_tbl[AFAB_IDX];

	
	calibrate_delay();
	for_each_possible_cpu(cpu)
		per_cpu(cpu_data, cpu).loops_per_jiffy = loops_per_jiffy;
}


static void __init scpll_set_refs_mxo(void)
{
	int cpu;
	uint32_t regval;

	
	for_each_possible_cpu(cpu) {
		regval = readl(sc_pll_base[cpu] + SCPLL_CFG_OFFSET);
		regval |= BIT(4);
		writel(regval, sc_pll_base[cpu] + SCPLL_CFG_OFFSET);
	}
	regval = readl(sc_pll_base[L2] + SCPLL_CFG_OFFSET);
	regval |= BIT(4);
	writel(regval, sc_pll_base[L2] + SCPLL_CFG_OFFSET);
}


static void __init lpj_init(void)
{
	int i;
	const struct clkctl_acpu_speed *base_freq;

	base_freq = drv_state.current_speed[smp_processor_id()];
	for (i = 0; acpu_freq_tbl[i].acpuclk_khz; i++) {
		acpu_freq_tbl[i].lpj =
			cpufreq_scale(
				per_cpu(cpu_data,
					smp_processor_id()).loops_per_jiffy,
				base_freq->acpuclk_khz,
				acpu_freq_tbl[i].acpuclk_khz);
	}
}

#ifdef CONFIG_CPU_FREQ_MSM
static struct cpufreq_frequency_table freq_table[NR_CPUS][20];

static void __init cpufreq_table_init(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		int i, freq_cnt = 0;
		
		for (i = 0; acpu_freq_tbl[i].acpuclk_khz != 0
				&& freq_cnt < ARRAY_SIZE(*freq_table); i++) {
			if (acpu_freq_tbl[i].use_for_scaling[cpu]) {
				freq_table[cpu][freq_cnt].index = freq_cnt;
				freq_table[cpu][freq_cnt].frequency
					= acpu_freq_tbl[i].acpuclk_khz;
				freq_cnt++;
			}
		}
		
		BUG_ON(acpu_freq_tbl[i].acpuclk_khz != 0);

		freq_table[cpu][freq_cnt].index = freq_cnt;
		freq_table[cpu][freq_cnt].frequency = CPUFREQ_TABLE_END;

		pr_info("CPU%d: %d scaling frequencies supported.\n",
			cpu, freq_cnt);

		
		cpufreq_frequency_table_get_attr(freq_table[cpu], cpu);
	}
}
#else
static void __init cpufreq_table_init(void) {}
#endif

void __init msm_acpu_clock_init(struct msm_acpu_clock_platform_data *clkdata)
{
	int cpu;

	mutex_init(&drv_state.lock);
	drv_state.acpu_switch_time_us = clkdata->acpu_switch_time_us;
	drv_state.max_speed_delta_khz = clkdata->max_speed_delta_khz;
	drv_state.max_vdd = clkdata->max_vdd;

	
	force_all_to_afab();
	scpll_set_refs_mxo();
	for_each_possible_cpu(cpu)
		scpll_init(cpu);
	scpll_init(L2);

	lpj_init();
	precompute_stepping();

	
	for_each_online_cpu(cpu)
		acpuclk_set_rate(cpu, 1080000, SETRATE_CPUFREQ);

	cpufreq_table_init();
}
