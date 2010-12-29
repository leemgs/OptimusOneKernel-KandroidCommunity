

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/sort.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <asm/mach-types.h>

#include "smd_private.h"
#include "clock.h"
#include "clock-7x30.h"
#include "acpuclock.h"
#include "socinfo.h"
#include "spm.h"

#define SCSS_CLK_CTL_ADDR	(MSM_ACC_BASE + 0x04)
#define SCSS_CLK_SEL_ADDR	(MSM_ACC_BASE + 0x08)

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)

#define VREF_SEL     1	
#define V_STEP       (25 * (2 - VREF_SEL)) 
#define VREG_DATA    (VREG_CONFIG | (VREF_SEL << 5))
#define VREG_CONFIG  (BIT(7) | BIT(6)) 

#define MV(mv)      ((mv) / (!((mv) % V_STEP)))

#define VDD_RAW(mv) (((MV(mv) / V_STEP) - 30) | VREG_DATA)

#define MAX_AXI_KHZ 192000

struct clock_state {
	struct clkctl_acpu_speed	*current_speed;
	struct mutex			lock;
	uint32_t			acpu_switch_time_us;
	uint32_t			vdd_switch_time_us;
};

struct clkctl_acpu_speed {
	unsigned int	acpu_clk_khz;
	int		src;
	unsigned int	acpu_src_sel;
	unsigned int	acpu_src_div;
	unsigned int	axi_clk_khz;
	unsigned int	vdd_mv;
	unsigned int	vdd_raw;
	uint32_t	msmc1;
	unsigned long	lpj; 
};

static struct clock_state drv_state = { 0 };

static struct cpufreq_frequency_table freq_table[] = {
	{ 0, 122880 },
	{ 1, 245760 },
	{ 2, 368640 },
	{ 3, 768000 },
	
	{ 4, 806400 },
	{ 5, CPUFREQ_TABLE_END },
};


#define SRC_LPXO (-2)
#define SRC_AXI  (-1)

static struct clkctl_acpu_speed acpu_freq_tbl[] = {
	{ 24576,  SRC_LPXO, 0, 0,  30720,  900, VDD_RAW(900), LOW },
	{ 61440,  PLL_3,    5, 11, 61440,  900, VDD_RAW(900), LOW },
	{ 122880, PLL_3,    5, 5,  61440,  900, VDD_RAW(900), LOW },
	{ 184320, PLL_3,    5, 4,  61440,  900, VDD_RAW(900), LOW },
	{ MAX_AXI_KHZ, SRC_AXI, 1, 0, 61440, 900, VDD_RAW(900), LOW },
	{ 245760, PLL_3,    5, 2,  61440,  900, VDD_RAW(900), LOW },
	{ 368640, PLL_3,    5, 1,  122800, 900, VDD_RAW(900), LOW },
	
	{ 768000, PLL_1,    2, 0,  153600, 1050, VDD_RAW(1050), NOMINAL },
	
	{ 806400, PLL_2,    3, 0,  192000, 1100, VDD_RAW(1100), NOMINAL },
	{ 0 }
};

#define POWER_COLLAPSE_KHZ MAX_AXI_KHZ
unsigned long acpuclk_power_collapse(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), POWER_COLLAPSE_KHZ, SETRATE_PC);
	return ret;
}

#define WAIT_FOR_IRQ_KHZ MAX_AXI_KHZ
unsigned long acpuclk_wait_for_irq(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), WAIT_FOR_IRQ_KHZ, SETRATE_SWFI);
	return ret;
}

static int acpuclk_set_acpu_vdd(struct clkctl_acpu_speed *s)
{
	int ret = msm_spm_set_vdd(s->vdd_raw);
	if (ret)
		return ret;

	
	udelay(drv_state.vdd_switch_time_us);
	return 0;
}


static void acpuclk_set_src(const struct clkctl_acpu_speed *s)
{
	uint32_t reg_clksel, reg_clkctl, src_sel;

	reg_clksel = readl(SCSS_CLK_SEL_ADDR);

	
	src_sel = reg_clksel & 1;

	
	reg_clkctl = readl(SCSS_CLK_CTL_ADDR);
	reg_clkctl &= ~(0xFF << (8 * src_sel));
	reg_clkctl |= s->acpu_src_sel << (4 + 8 * src_sel);
	reg_clkctl |= s->acpu_src_div << (0 + 8 * src_sel);
	writel(reg_clkctl, SCSS_CLK_CTL_ADDR);

	
	reg_clksel ^= 1;

	
	writel(reg_clksel, SCSS_CLK_SEL_ADDR);
}

int acpuclk_set_rate(int cpu, unsigned long rate, enum setrate_reason reason)
{
	struct clkctl_acpu_speed *tgt_s, *strt_s;
	int res, rc = 0;

	if (reason == SETRATE_CPUFREQ)
		mutex_lock(&drv_state.lock);

	strt_s = drv_state.current_speed;

	if (rate == strt_s->acpu_clk_khz)
		goto out;

	for (tgt_s = acpu_freq_tbl; tgt_s->acpu_clk_khz != 0; tgt_s++) {
		if (tgt_s->acpu_clk_khz == rate)
			break;
	}
	if (tgt_s->acpu_clk_khz == 0) {
		rc = -EINVAL;
		goto out;
	}

	if (reason == SETRATE_CPUFREQ) {
		
		if (tgt_s->msmc1 > strt_s->msmc1) {
			rc = vote_msmc1(tgt_s->msmc1);
			if (rc) {
				pr_err("Failed to vote for MSMC1\n");
				goto out;
			}
			unvote_msmc1(strt_s->msmc1);
		}
		if (tgt_s->vdd_mv > strt_s->vdd_mv) {

			rc = acpuclk_set_acpu_vdd(tgt_s);
			if (rc < 0) {
				pr_err("ACPU VDD increase to %d mV failed "
					"(%d)\n", tgt_s->vdd_mv, rc);
				goto out;
			}
		}
	}

	dprintk("Switching from ACPU rate %u KHz -> %u KHz\n",
	       strt_s->acpu_clk_khz, tgt_s->acpu_clk_khz);

	
	if (tgt_s->axi_clk_khz > strt_s->axi_clk_khz) {
		rc = ebi1_clk_set_min_rate(CLKVOTE_ACPUCLK,
						tgt_s->axi_clk_khz * 1000);
		if (rc < 0) {
			pr_err("Setting AXI min rate failed (%d)\n", rc);
			goto out;
		}
	}

	
	if (strt_s->src != tgt_s->src && tgt_s->src >= 0) {
		dprintk("Enabling PLL %d\n", tgt_s->src);
		pll_enable(tgt_s->src);
	}

	
	acpuclk_set_src(tgt_s);
	drv_state.current_speed = tgt_s;
	loops_per_jiffy = tgt_s->lpj;

	
	if (reason == SETRATE_SWFI)
		goto out;

	
	if (strt_s->src != tgt_s->src && strt_s->src >= 0) {
		dprintk("Disabling PLL %d\n", strt_s->src);
		pll_disable(strt_s->src);
	}

	
	if (tgt_s->axi_clk_khz < strt_s->axi_clk_khz) {
		res = ebi1_clk_set_min_rate(CLKVOTE_ACPUCLK,
						tgt_s->axi_clk_khz * 1000);
		if (res < 0)
			pr_warning("Setting AXI min rate failed (%d)\n", res);
	}

	
	if (reason == SETRATE_PC)
		goto out;

	
	if (tgt_s->vdd_mv < strt_s->vdd_mv) {
		res = acpuclk_set_acpu_vdd(tgt_s);
		if (res)
			pr_warning("ACPU VDD decrease to %d mV failed (%d)\n",
					tgt_s->vdd_mv, res);
	}
	if (tgt_s->msmc1 < strt_s->msmc1) {
		res = vote_msmc1(tgt_s->msmc1);
		if (res)
			pr_err("Failed to vote for MSMC1\n");
		else
			unvote_msmc1(strt_s->msmc1);
	}

	dprintk("ACPU speed change complete\n");
out:
	if (reason == SETRATE_CPUFREQ)
		mutex_unlock(&drv_state.lock);

	return rc;
}

unsigned long acpuclk_get_rate(int cpu)
{
	WARN_ONCE(drv_state.current_speed == NULL,
		  "acpuclk_get_rate: not initialized\n");
	if (drv_state.current_speed)
		return drv_state.current_speed->acpu_clk_khz;
	else
		return 0;
}

uint32_t acpuclk_get_switch_time(void)
{
	return drv_state.acpu_switch_time_us;
}

unsigned long clk_get_max_axi_khz(void)
{
	return MAX_AXI_KHZ;
}
EXPORT_SYMBOL(clk_get_max_axi_khz);




static void __init acpuclk_init(void)
{
	struct clkctl_acpu_speed *s;
	uint32_t div, sel, src_num;
	uint32_t reg_clksel, reg_clkctl;
	int res;

	reg_clksel = readl(SCSS_CLK_SEL_ADDR);

	
	switch ((reg_clksel >> 1) & 0x3) {
	case 0:	
		reg_clkctl = readl(SCSS_CLK_CTL_ADDR);
		src_num = reg_clksel & 0x1;
		sel = (reg_clkctl >> (12 - (8 * src_num))) & 0x7;
		div = (reg_clkctl >> (8 -  (8 * src_num))) & 0xF;

		
		for (s = acpu_freq_tbl; s->acpu_clk_khz != 0; s++) {
			if (s->acpu_src_sel == sel && s->acpu_src_div == div)
				break;
		}
		if (s->acpu_clk_khz == 0) {
			pr_err("Error - ACPU clock reports invalid speed\n");
			return;
		}
		break;
	case 2:	
		
		for (s = acpu_freq_tbl; s->acpu_clk_khz != 0
			&& s->src != PLL_2 && s->acpu_src_div == 0; s++)
			;
		if (s->acpu_clk_khz != 0) {
			
			acpuclk_set_src(s);

			
			reg_clksel = readl(SCSS_CLK_SEL_ADDR);
			reg_clksel &= ~(0x3 << 1);
			writel(reg_clksel, SCSS_CLK_SEL_ADDR);
			break;
		}
		
	default:
		pr_err("Error - ACPU clock reports invalid source\n");
		return;
	}

	
	res = vote_msmc1(s->msmc1);
	if (res)
		pr_warning("Failed to vote for MSMC1\n");
	acpuclk_set_acpu_vdd(s);

	drv_state.current_speed = s;

	
	if (s->src >= 0)
		pll_enable(s->src);

	res = ebi1_clk_set_min_rate(CLKVOTE_ACPUCLK, s->axi_clk_khz * 1000);
	if (res < 0)
		pr_warning("Setting AXI min rate failed!\n");

	pr_info("ACPU running at %d KHz\n", s->acpu_clk_khz);

	return;
}


static void __init lpj_init(void)
{
	int i;
	const struct clkctl_acpu_speed *base_clk = drv_state.current_speed;

	for (i = 0; acpu_freq_tbl[i].acpu_clk_khz; i++) {
		acpu_freq_tbl[i].lpj = cpufreq_scale(loops_per_jiffy,
						base_clk->acpu_clk_khz,
						acpu_freq_tbl[i].acpu_clk_khz);
	}
}


void __init pll2_1024mhz_fixup(void)
{
	if (acpu_freq_tbl[ARRAY_SIZE(acpu_freq_tbl)-2].acpu_clk_khz != 806400
		  || freq_table[ARRAY_SIZE(freq_table)-2].frequency != 806400) {
		pr_err("Frequency table fixups for PLL2 rate failed.\n");
		BUG();
	}
	acpu_freq_tbl[ARRAY_SIZE(acpu_freq_tbl)-2].acpu_clk_khz = 1024000;
	acpu_freq_tbl[ARRAY_SIZE(acpu_freq_tbl)-2].vdd_mv = 1200;
	acpu_freq_tbl[ARRAY_SIZE(acpu_freq_tbl)-2].vdd_raw = VDD_RAW(1200);
	acpu_freq_tbl[ARRAY_SIZE(acpu_freq_tbl)-2].msmc1 = HIGH;
	freq_table[ARRAY_SIZE(freq_table)-2].frequency = 1024000;
}

#define RPM_BYPASS_MASK	(1 << 3)
#define PMIC_MODE_MASK	(1 << 4)

void __init msm_acpu_clock_init(struct msm_acpu_clock_platform_data *clkdata)
{
	pr_info("acpu_clock_init()\n");

	mutex_init(&drv_state.lock);
	drv_state.acpu_switch_time_us = clkdata->acpu_switch_time_us;
	drv_state.vdd_switch_time_us = clkdata->vdd_switch_time_us;
	
	if (cpu_is_msm8x55())
		pll2_1024mhz_fixup();
	acpuclk_init();
	lpj_init();
#ifdef CONFIG_CPU_FREQ_MSM
	cpufreq_frequency_table_get_attr(freq_table, smp_processor_id());
#endif
}
