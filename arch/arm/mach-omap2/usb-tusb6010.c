

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/usb/musb.h>

#include <mach/gpmc.h>
#include <mach/mux.h>


static u8		async_cs, sync_cs;
static unsigned		refclk_psec;



static unsigned
next_clk(unsigned t1_NS, unsigned t2_ps, unsigned fclk_ps)
{
	unsigned	t1_ps = t1_NS * 1000;
	unsigned	t1_f, t2_f;

	if ((t1_ps + fclk_ps) < t2_ps)
		return t2_ps / 1000;

	t1_f = (t1_ps + fclk_ps - 1) / fclk_ps;
	t2_f = (t2_ps + fclk_ps - 1) / fclk_ps;

	if (t1_f >= t2_f)
		t2_f = t1_f + 1;

	return (t2_f * fclk_ps) / 1000;
}



static int tusb_set_async_mode(unsigned sysclk_ps, unsigned fclk_ps)
{
	struct gpmc_timings	t;
	unsigned		t_acsnh_advnh = sysclk_ps + 3000;
	unsigned		tmp;

	memset(&t, 0, sizeof(t));

	
	t.cs_on = 8;
	
	t.adv_on = next_clk(t.cs_on, t_acsnh_advnh - 7000, fclk_ps);

	

	
	t.adv_rd_off = next_clk(t.adv_on, t_acsnh_advnh, fclk_ps);

	
	t.oe_on = next_clk(t.adv_on, t_acsnh_advnh + 1000, fclk_ps);

	
	tmp = t.oe_on * 1000 + 300;
	t.access = next_clk(t.oe_on, tmp, fclk_ps);

	
	tmp = t.access * 1000;
	t.oe_off = next_clk(t.access, tmp, fclk_ps);

	t.cs_rd_off = t.oe_off;

	tmp = t.cs_rd_off * 1000 + 7000 ;
	t.rd_cycle = next_clk(t.cs_rd_off, tmp, fclk_ps);

	

	
	t.adv_wr_off = t.adv_rd_off;

	
	t.we_on = next_clk(t.adv_wr_off, t_acsnh_advnh + 1000, fclk_ps);

	
	tmp = t.we_on * 1000 + 300;
	t.we_off = next_clk(t.we_on, tmp, fclk_ps);

	t.cs_wr_off = t.we_off;

	tmp = t.cs_wr_off * 1000 + 7000 ;
	t.wr_cycle = next_clk(t.cs_wr_off, tmp, fclk_ps);

	return gpmc_cs_set_timings(async_cs, &t);
}

static int tusb_set_sync_mode(unsigned sysclk_ps, unsigned fclk_ps)
{
	struct gpmc_timings	t;
	unsigned		t_scsnh_advnh = sysclk_ps + 3000;
	unsigned		tmp;

	memset(&t, 0, sizeof(t));
	t.cs_on = 8;

	
	t.adv_on = next_clk(t.cs_on, t_scsnh_advnh - 7000, fclk_ps);

	
	t.sync_clk = 12 ;
	tmp = (t.sync_clk * 1000 + fclk_ps - 1) / fclk_ps;
	if (tmp > 4)
		return -ERANGE;
	if (tmp <= 0)
		tmp = 1;
	t.page_burst_access = (fclk_ps * tmp) / 1000;

	

	
	t.adv_rd_off = next_clk(t.adv_on, t_scsnh_advnh, fclk_ps);

	
	tmp = (t.adv_rd_off * 1000) + (3 * fclk_ps);
	t.oe_on = next_clk(t.adv_on, tmp, fclk_ps);

	
	tmp = (t.oe_on * 1000) + (5 * fclk_ps);
	t.access = next_clk(t.oe_on, tmp, fclk_ps);

	
	tmp = (t.access * 1000) + (1 * fclk_ps);
	t.oe_off = next_clk(t.access, tmp, fclk_ps);

	t.cs_rd_off = t.oe_off;

	tmp = t.cs_rd_off * 1000 + 7000 ;
	t.rd_cycle = next_clk(t.cs_rd_off, tmp, fclk_ps);

	

	
	t.adv_wr_off = t.adv_rd_off;

	
	tmp = (t.adv_wr_off * 1000) + (3 * fclk_ps);
	t.we_on = next_clk(t.adv_wr_off, tmp, fclk_ps);

	
	tmp = (t.we_on * 1000) + (6 * fclk_ps);
	t.we_off = next_clk(t.we_on, tmp, fclk_ps);

	t.cs_wr_off = t.we_off;

	tmp = t.cs_wr_off * 1000 + 7000 ;
	t.wr_cycle = next_clk(t.cs_wr_off, tmp, fclk_ps);

	return gpmc_cs_set_timings(sync_cs, &t);
}

extern unsigned long gpmc_get_fclk_period(void);


int tusb6010_platform_retime(unsigned is_refclk)
{
	static const char	error[] =
		KERN_ERR "tusb6010 %s retime error %d\n";

	unsigned	fclk_ps = gpmc_get_fclk_period();
	unsigned	sysclk_ps;
	int		status;

	if (!refclk_psec || fclk_ps == 0)
		return -ENODEV;

	sysclk_ps = is_refclk ? refclk_psec : TUSB6010_OSCCLK_60;

	status = tusb_set_async_mode(sysclk_ps, fclk_ps);
	if (status < 0) {
		printk(error, "async", status);
		goto done;
	}
	status = tusb_set_sync_mode(sysclk_ps, fclk_ps);
	if (status < 0)
		printk(error, "sync", status);
done:
	return status;
}
EXPORT_SYMBOL_GPL(tusb6010_platform_retime);

static struct resource tusb_resources[] = {
	
	{ 
		.flags	= IORESOURCE_MEM,
	},
	{ 
		.flags	= IORESOURCE_MEM,
	},
	{ 
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 tusb_dmamask = ~(u32)0;

static struct platform_device tusb_device = {
	.name		= "musb_hdrc",
	.id		= -1,
	.dev = {
		.dma_mask		= &tusb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(tusb_resources),
	.resource	= tusb_resources,
};



int __init
tusb6010_setup_interface(struct musb_hdrc_platform_data *data,
		unsigned ps_refclk, unsigned waitpin,
		unsigned async, unsigned sync,
		unsigned irq, unsigned dmachan)
{
	int		status;
	static char	error[] __initdata =
		KERN_ERR "tusb6010 init error %d, %d\n";

	
	status = gpmc_cs_request(async, SZ_16M, (unsigned long *)
				&tusb_resources[0].start);
	if (status < 0) {
		printk(error, 1, status);
		return status;
	}
	tusb_resources[0].end = tusb_resources[0].start + 0x9ff;
	async_cs = async;
	gpmc_cs_write_reg(async, GPMC_CS_CONFIG1,
			  GPMC_CONFIG1_PAGE_LEN(2)
			| GPMC_CONFIG1_WAIT_READ_MON
			| GPMC_CONFIG1_WAIT_WRITE_MON
			| GPMC_CONFIG1_WAIT_PIN_SEL(waitpin)
			| GPMC_CONFIG1_READTYPE_ASYNC
			| GPMC_CONFIG1_WRITETYPE_ASYNC
			| GPMC_CONFIG1_DEVICESIZE_16
			| GPMC_CONFIG1_DEVICETYPE_NOR
			| GPMC_CONFIG1_MUXADDDATA);


	
	status = gpmc_cs_request(sync, SZ_16M, (unsigned long *)
				&tusb_resources[1].start);
	if (status < 0) {
		printk(error, 2, status);
		return status;
	}
	tusb_resources[1].end = tusb_resources[1].start + 0x9ff;
	sync_cs = sync;
	gpmc_cs_write_reg(sync, GPMC_CS_CONFIG1,
			  GPMC_CONFIG1_READMULTIPLE_SUPP
			| GPMC_CONFIG1_READTYPE_SYNC
			| GPMC_CONFIG1_WRITEMULTIPLE_SUPP
			| GPMC_CONFIG1_WRITETYPE_SYNC
			| GPMC_CONFIG1_CLKACTIVATIONTIME(1)
			| GPMC_CONFIG1_PAGE_LEN(2)
			| GPMC_CONFIG1_WAIT_READ_MON
			| GPMC_CONFIG1_WAIT_WRITE_MON
			| GPMC_CONFIG1_WAIT_PIN_SEL(waitpin)
			| GPMC_CONFIG1_DEVICESIZE_16
			| GPMC_CONFIG1_DEVICETYPE_NOR
			| GPMC_CONFIG1_MUXADDDATA
			
			);

	
	status = gpio_request(irq, "TUSB6010 irq");
	if (status < 0) {
		printk(error, 3, status);
		return status;
	}
	gpio_direction_input(irq);
	tusb_resources[2].start = irq + IH_GPIO_BASE;

	
	if (!ps_refclk) {
		printk(error, 4, status);
		return -ENODEV;
	}
	refclk_psec = ps_refclk;
	status = tusb6010_platform_retime(1);
	if (status < 0) {
		printk(error, 5, status);
		return status;
	}

	
	if (!data) {
		printk(error, 6, status);
		return -ENODEV;
	}
	tusb_device.dev.platform_data = data;

	
	if (!dmachan)
		tusb_device.dev.dma_mask = NULL;
	else {
		
		if (dmachan & (1 << 0))
			omap_cfg_reg(AA10_242X_DMAREQ0);
		if (dmachan & (1 << 1))
			omap_cfg_reg(AA6_242X_DMAREQ1);
		if (dmachan & (1 << 2))
			omap_cfg_reg(E4_242X_DMAREQ2);
		if (dmachan & (1 << 3))
			omap_cfg_reg(G4_242X_DMAREQ3);
		if (dmachan & (1 << 4))
			omap_cfg_reg(D3_242X_DMAREQ4);
		if (dmachan & (1 << 5))
			omap_cfg_reg(E3_242X_DMAREQ5);
	}

	
	status = platform_device_register(&tusb_device);
	if (status < 0) {
		printk(error, 7, status);
		return status;
	}
	return 0;
}
