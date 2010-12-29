

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/cpufreq.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>

#include "acpuclock.h"
#include "clock.h"


#define SHOT_SWITCH		4
#define HOP_SWITCH		5
#define SIMPLE_SLEW		6
#define COMPLEX_SLEW		7

#define L_VAL_384MHZ		0xA
#define L_VAL_1497MHZ		0x27
#define L_VAL_SCPLL_HW_MAX	L_VAL_1497MHZ


#define SCPLL_POWER_DOWN	0
#define SCPLL_BYPASS		1
#define SCPLL_STANDBY		2
#define SCPLL_FULL_CAL		4
#define SCPLL_HALF_CAL		5
#define SCPLL_STEP_CAL		6
#define SCPLL_NORMAL		7


#define SCPLL_CTL_ADDR         (MSM_SCPLL_BASE + 0x4)
#define SCPLL_CAL_ADDR         (MSM_SCPLL_BASE + 0x8)
#define SCPLL_STATUS_ADDR      (MSM_SCPLL_BASE + 0x10)
#define SCPLL_FSM_CTL_EXT_ADDR (MSM_SCPLL_BASE + 0x24)

#define SPSS_CLK_CTL_ADDR	(MSM_CSR_BASE + 0x100)
#define SPSS_CLK_SEL_ADDR	(MSM_CSR_BASE + 0x104)

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)

enum {
	ACPU_PLL_TCXO	= -1,
	ACPU_PLL_0	= 0,
	ACPU_PLL_1,
	ACPU_PLL_2,
	ACPU_PLL_3,
	ACPU_PLL_END,
};

struct clkctl_acpu_speed {
	unsigned int     use_for_scaling;
	unsigned int     acpuclk_khz;
	int              pll;
	unsigned int     acpuclk_src_sel;
	unsigned int     acpuclk_src_div;
	unsigned int     ahbclk_khz;
	unsigned int     ahbclk_div;
	unsigned int     ebi1clk_khz;
	unsigned int     core_src_sel;
	unsigned int     l_value;
	int              vdd;
	unsigned long    lpj; 
};

struct clkctl_acpu_speed acpu_freq_tbl[] = {
	{ 0,  19200, ACPU_PLL_TCXO, 0, 0, 0, 0, 14000, 0, 0, 1225 },
	
	{ 0,  192000, ACPU_PLL_1, 1, 5, 0, 0, 14000, 2, 0, 1225 },
	{ 1,  245760, ACPU_PLL_0, 4, 0, 0, 0, 29000, 0, 0, 1225 },
	{ 1,  384000, ACPU_PLL_3, 0, 0, 0, 0, 58000, 1, 0xA, 1225 },
	{ 0,  422400, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0xB, 1225 },
	{ 0,  460800, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0xC, 1225 },
	{ 0,  499200, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0xD, 1225 },
	{ 0,  537600, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0xE, 1225 },
	{ 1,  576000, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0xF, 1225 },
	{ 0,  614400, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0x10, 1225 },
	{ 0,  652800, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0x11, 1225 },
	{ 0,  691200, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0x12, 1225 },
	{ 0,  729600, ACPU_PLL_3, 0, 0, 0, 0, 117000, 1, 0x13, 1225 },
	{ 1,  768000, ACPU_PLL_3, 0, 0, 0, 0, 128000, 1, 0x14, 1225 },
	{ 0,  806400, ACPU_PLL_3, 0, 0, 0, 0, 128000, 1, 0x15, 1225 },
	{ 0,  844800, ACPU_PLL_3, 0, 0, 0, 0, 128000, 1, 0x16, 1225 },
	{ 0,  883200, ACPU_PLL_3, 0, 0, 0, 0, 160000, 1, 0x17, 1225 },
	{ 0,  921600, ACPU_PLL_3, 0, 0, 0, 0, 160000, 1, 0x18, 1225 },
	{ 0,  960000, ACPU_PLL_3, 0, 0, 0, 0, 192000, 1, 0x19, 1225 },
	{ 1,  998400, ACPU_PLL_3, 0, 0, 0, 0, 192000, 1, 0x1A, 1225 },
	{ 1, 1190400, ACPU_PLL_3, 0, 0, 0, 0, 259200, 1, 0x1F, 1225 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

struct clock_state {
	struct clkctl_acpu_speed	*current_speed;
	struct mutex			lock;
	uint32_t			acpu_switch_time_us;
	uint32_t			max_speed_delta_khz;
	unsigned int			max_vdd;
	int (*acpu_set_vdd) (int mvolts);
};

static struct clock_state drv_state = { 0 };

unsigned long clk_get_max_axi_khz(void)
{
	return 192000;
}
EXPORT_SYMBOL(clk_get_max_axi_khz);

unsigned long acpuclk_get_rate(int cpu)
{
	return drv_state.current_speed->acpuclk_khz;
}

uint32_t acpuclk_get_switch_time(void)
{
	return drv_state.acpu_switch_time_us;
}

#define POWER_COLLAPSE_KHZ 128000
unsigned long acpuclk_power_collapse(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), POWER_COLLAPSE_KHZ, SETRATE_PC);
	return ret;
}

#define WAIT_FOR_IRQ_KHZ 128000
unsigned long acpuclk_wait_for_irq(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), WAIT_FOR_IRQ_KHZ, SETRATE_SWFI);
	return ret;
}

static void select_core_source(unsigned int src)
{
	uint32_t regval;

	regval = readl(SPSS_CLK_SEL_ADDR);
	regval &= ~(0x3 << 1);
	regval |= (src << 1);
	writel(regval, SPSS_CLK_SEL_ADDR);
}

static void select_clk_source(struct clkctl_acpu_speed *s)
{
	uint32_t reg_clksel, reg_clkctl, src_sel;

	
	if (s->core_src_sel == 0) {

		reg_clksel = readl(SPSS_CLK_SEL_ADDR);

		
		src_sel = reg_clksel & 1;

		
		reg_clkctl = readl(SPSS_CLK_CTL_ADDR);
		reg_clkctl &= ~(0xFF << (8 * src_sel));
		reg_clkctl |= s->acpuclk_src_sel << (4 + 8 * src_sel);
		reg_clkctl |= s->acpuclk_src_div << (0 + 8 * src_sel);
		writel(reg_clkctl, SPSS_CLK_CTL_ADDR);

		
		reg_clksel ^= 1;

		
		writel(reg_clksel, SPSS_CLK_SEL_ADDR);
	}
}

static void scpll_enable(bool state, struct clkctl_acpu_speed *tgt_s)
{
	uint32_t regval;

	if (state)
		dprintk("Enabling PLL 3\n");
	else
		dprintk("Disabling PLL 3\n");

	if (state) {
		
		writel(SCPLL_STANDBY, SCPLL_CTL_ADDR);
		udelay(10);

		
		regval = (tgt_s->l_value << 3) | SHOT_SWITCH;
		writel(regval, SCPLL_FSM_CTL_EXT_ADDR);
		writel(SCPLL_NORMAL, SCPLL_CTL_ADDR);
		udelay(20);
	} else {
		
		writel(SCPLL_POWER_DOWN, SCPLL_CTL_ADDR);
	}

	if (state)
		dprintk("PLL 3 Enabled\n");
	else
		dprintk("PLL 3 Disabled\n");
}

static void scpll_change_freq(uint32_t lval)
{
	uint32_t regval;

	
	regval = (lval << 3) | COMPLEX_SLEW;
	writel(regval, SCPLL_FSM_CTL_EXT_ADDR);
	writel(SCPLL_NORMAL, SCPLL_CTL_ADDR);

	
	while (readl(SCPLL_STATUS_ADDR) & 0x1)
		;
}

static int acpuclk_set_vdd_level(int vdd)
{
	if (drv_state.acpu_set_vdd) {
		dprintk("Switching VDD to %d mV\n", vdd);
		return drv_state.acpu_set_vdd(vdd);
	} else {
		
		return 0;
	}
}

int acpuclk_set_rate(int cpu, unsigned long rate, enum setrate_reason reason)
{
	struct clkctl_acpu_speed *tgt_s, *strt_s;
	int res, rc = 0;
	int freq_index = 0;

	if (reason == SETRATE_CPUFREQ)
		mutex_lock(&drv_state.lock);

	strt_s = drv_state.current_speed;

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

	dprintk("Switching from ACPU rate %u KHz -> %u KHz\n",
		strt_s->acpuclk_khz, tgt_s->acpuclk_khz);

	if (strt_s->pll != ACPU_PLL_3 && tgt_s->pll != ACPU_PLL_3) {
		select_clk_source(tgt_s);
		
		select_core_source(tgt_s->core_src_sel);
	} else if (strt_s->pll != ACPU_PLL_3 && tgt_s->pll == ACPU_PLL_3) {
		scpll_enable(1, tgt_s);
		mb();
		select_core_source(tgt_s->core_src_sel);
	} else if (strt_s->pll == ACPU_PLL_3 && tgt_s->pll != ACPU_PLL_3) {
		select_clk_source(tgt_s);
		select_core_source(tgt_s->core_src_sel);
		mb();
		scpll_enable(0, NULL);
	} else {
		scpll_change_freq(tgt_s->l_value);
	}

	
	drv_state.current_speed = tgt_s;

	
	loops_per_jiffy = tgt_s->lpj;

	
	if (reason == SETRATE_SWFI)
		goto out;

	if (strt_s->ebi1clk_khz != tgt_s->ebi1clk_khz) {
		res = ebi1_clk_set_min_rate(CLKVOTE_ACPUCLK,
			tgt_s->ebi1clk_khz * 1000);
		if (res < 0)
			pr_warning("Setting EBI1/AXI min rate failed (%d)\n",
									res);
	}

	
	if (reason == SETRATE_PC)
		goto out;

	
	if (tgt_s->vdd < strt_s->vdd) {
		res = acpuclk_set_vdd_level(tgt_s->vdd);
		if (res)
			pr_warning("Unable to drop ACPU vdd (%d)\n", res);
	}

	dprintk("ACPU speed change complete\n");
out:
	if (reason == SETRATE_CPUFREQ)
		mutex_unlock(&drv_state.lock);
	return rc;
}

static void __init scpll_init(void)
{
	uint32_t regval;

	dprintk("Initializing PLL 3\n");

	
	writel(SCPLL_STANDBY, SCPLL_CTL_ADDR);
	udelay(10);

	
	regval = (L_VAL_SCPLL_HW_MAX << 24) | (L_VAL_384MHZ << 16);
	writel(regval, SCPLL_CAL_ADDR);
	writel(SCPLL_FULL_CAL, SCPLL_CTL_ADDR);

	
	while (readl(SCPLL_STATUS_ADDR) & 0x2)
		;

	
	scpll_enable(0, NULL);
}

static void __init acpuclk_init(void)
{
	struct clkctl_acpu_speed *speed;
	uint32_t div, sel, regval;
	int res;

	
	regval = readl(SPSS_CLK_SEL_ADDR);
	switch ((regval & 0x6) >> 1) {
	case 0: 
	case 3: 
		if (regval & 0x1) {
			sel = ((readl(SPSS_CLK_CTL_ADDR) >> 4) & 0x7);
			div = ((readl(SPSS_CLK_CTL_ADDR) >> 0) & 0xf);
		} else {
			sel = ((readl(SPSS_CLK_CTL_ADDR) >> 12) & 0x7);
			div = ((readl(SPSS_CLK_CTL_ADDR) >> 8) & 0xf);
		}

		
		for (speed = acpu_freq_tbl; speed->acpuclk_khz != 0; speed++) {
			if (speed->acpuclk_src_sel == sel &&
			    speed->acpuclk_src_div == div)
				break;
		}
		break;

	case 1: 
		sel = ((readl(SCPLL_FSM_CTL_EXT_ADDR) >> 3) & 0x3f);

		
		for (speed = acpu_freq_tbl; speed->acpuclk_khz != 0; speed++) {
			if (speed->l_value == sel &&
			    speed->core_src_sel == 1)
				break;
		}
		break;

	case 2: 
		speed = &acpu_freq_tbl[1];
		break;
	default:
		BUG();
	}

	
	if (speed->pll != ACPU_PLL_3)
		scpll_init();

	if (speed->acpuclk_khz == 0) {
		pr_err("Error - ACPU clock reports invalid speed\n");
		return;
	}

	
	acpuclk_set_vdd_level(speed->vdd);

	drv_state.current_speed = speed;
	res = ebi1_clk_set_min_rate(CLKVOTE_ACPUCLK, speed->ebi1clk_khz * 1000);
	if (res < 0)
		pr_warning("Setting EBI1/AXI min rate failed (%d)\n", res);

	pr_info("ACPU running at %d KHz\n", speed->acpuclk_khz);
}


static void __init lpj_init(void)
{
	int i;
	const struct clkctl_acpu_speed *base_clk = drv_state.current_speed;
	for (i = 0; acpu_freq_tbl[i].acpuclk_khz; i++) {
		acpu_freq_tbl[i].lpj = cpufreq_scale(loops_per_jiffy,
						base_clk->acpuclk_khz,
						acpu_freq_tbl[i].acpuclk_khz);
	}
}

#ifdef CONFIG_CPU_FREQ_MSM
static struct cpufreq_frequency_table freq_table[20];

static void __init cpufreq_table_init(void)
{
	unsigned int i;
	unsigned int freq_cnt = 0;

	
	for (i = 0; acpu_freq_tbl[i].acpuclk_khz != 0
			&& freq_cnt < ARRAY_SIZE(freq_table)-1; i++) {
		if (acpu_freq_tbl[i].use_for_scaling) {
			freq_table[freq_cnt].index = freq_cnt;
			freq_table[freq_cnt].frequency
				= acpu_freq_tbl[i].acpuclk_khz;
			freq_cnt++;
		}
	}

	
	BUG_ON(acpu_freq_tbl[i].acpuclk_khz != 0);

	freq_table[freq_cnt].index = freq_cnt;
	freq_table[freq_cnt].frequency = CPUFREQ_TABLE_END;

	pr_info("%d scaling frequencies supported.\n", freq_cnt);
}
#endif

void __init msm_acpu_clock_init(struct msm_acpu_clock_platform_data *clkdata)
{
	mutex_init(&drv_state.lock);
	drv_state.acpu_switch_time_us = clkdata->acpu_switch_time_us;
	drv_state.max_speed_delta_khz = clkdata->max_speed_delta_khz;
	drv_state.max_vdd = clkdata->max_vdd;
	drv_state.acpu_set_vdd = clkdata->acpu_set_vdd;

	acpuclk_init();
	lpj_init();
#ifdef CONFIG_CPU_FREQ_MSM
	cpufreq_table_init();
	cpufreq_frequency_table_get_attr(freq_table, smp_processor_id());
#endif
}
