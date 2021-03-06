
#include <linux/module.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/dma.h>
#include <mach/mux.h>
#include <mach/cpu.h>
#include <mach/mcbsp.h>

static void omap2_mcbsp2_mux_setup(void)
{
	omap_cfg_reg(Y15_24XX_MCBSP2_CLKX);
	omap_cfg_reg(R14_24XX_MCBSP2_FSX);
	omap_cfg_reg(W15_24XX_MCBSP2_DR);
	omap_cfg_reg(V15_24XX_MCBSP2_DX);
	omap_cfg_reg(V14_24XX_GPIO117);
	
}

static void omap2_mcbsp_request(unsigned int id)
{
	if (cpu_is_omap2420() && (id == OMAP_MCBSP2))
		omap2_mcbsp2_mux_setup();
}

static struct omap_mcbsp_ops omap2_mcbsp_ops = {
	.request	= omap2_mcbsp_request,
};

#ifdef CONFIG_ARCH_OMAP2420
static struct omap_mcbsp_platform_data omap2420_mcbsp_pdata[] = {
	{
		.phys_base	= OMAP24XX_MCBSP1_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP1_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP1_TX,
		.rx_irq		= INT_24XX_MCBSP1_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP1_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
	{
		.phys_base	= OMAP24XX_MCBSP2_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP2_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP2_TX,
		.rx_irq		= INT_24XX_MCBSP2_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP2_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
};
#define OMAP2420_MCBSP_PDATA_SZ		ARRAY_SIZE(omap2420_mcbsp_pdata)
#else
#define omap2420_mcbsp_pdata		NULL
#define OMAP2420_MCBSP_PDATA_SZ		0
#endif

#ifdef CONFIG_ARCH_OMAP2430
static struct omap_mcbsp_platform_data omap2430_mcbsp_pdata[] = {
	{
		.phys_base	= OMAP24XX_MCBSP1_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP1_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP1_TX,
		.rx_irq		= INT_24XX_MCBSP1_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP1_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
	{
		.phys_base	= OMAP24XX_MCBSP2_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP2_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP2_TX,
		.rx_irq		= INT_24XX_MCBSP2_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP2_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
	{
		.phys_base	= OMAP2430_MCBSP3_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP3_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP3_TX,
		.rx_irq		= INT_24XX_MCBSP3_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP3_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
	{
		.phys_base	= OMAP2430_MCBSP4_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP4_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP4_TX,
		.rx_irq		= INT_24XX_MCBSP4_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP4_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
	{
		.phys_base	= OMAP2430_MCBSP5_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP5_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP5_TX,
		.rx_irq		= INT_24XX_MCBSP5_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP5_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
	},
};
#define OMAP2430_MCBSP_PDATA_SZ		ARRAY_SIZE(omap2430_mcbsp_pdata)
#else
#define omap2430_mcbsp_pdata		NULL
#define OMAP2430_MCBSP_PDATA_SZ		0
#endif

#ifdef CONFIG_ARCH_OMAP34XX
static struct omap_mcbsp_platform_data omap34xx_mcbsp_pdata[] = {
	{
		.phys_base	= OMAP34XX_MCBSP1_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP1_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP1_TX,
		.rx_irq		= INT_24XX_MCBSP1_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP1_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
		.buffer_size	= 0x6F,
	},
	{
		.phys_base	= OMAP34XX_MCBSP2_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP2_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP2_TX,
		.rx_irq		= INT_24XX_MCBSP2_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP2_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
		.buffer_size	= 0x3FF,
	},
	{
		.phys_base	= OMAP34XX_MCBSP3_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP3_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP3_TX,
		.rx_irq		= INT_24XX_MCBSP3_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP3_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
		.buffer_size	= 0x6F,
	},
	{
		.phys_base	= OMAP34XX_MCBSP4_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP4_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP4_TX,
		.rx_irq		= INT_24XX_MCBSP4_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP4_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
		.buffer_size	= 0x6F,
	},
	{
		.phys_base	= OMAP34XX_MCBSP5_BASE,
		.dma_rx_sync	= OMAP24XX_DMA_MCBSP5_RX,
		.dma_tx_sync	= OMAP24XX_DMA_MCBSP5_TX,
		.rx_irq		= INT_24XX_MCBSP5_IRQ_RX,
		.tx_irq		= INT_24XX_MCBSP5_IRQ_TX,
		.ops		= &omap2_mcbsp_ops,
		.buffer_size	= 0x6F,
	},
};
#define OMAP34XX_MCBSP_PDATA_SZ		ARRAY_SIZE(omap34xx_mcbsp_pdata)
#else
#define omap34xx_mcbsp_pdata		NULL
#define OMAP34XX_MCBSP_PDATA_SZ		0
#endif

static struct omap_mcbsp_platform_data omap44xx_mcbsp_pdata[] = {
	{
		.phys_base      = OMAP44XX_MCBSP1_BASE,
		.dma_rx_sync    = OMAP44XX_DMA_MCBSP1_RX,
		.dma_tx_sync    = OMAP44XX_DMA_MCBSP1_TX,
		.rx_irq         = INT_24XX_MCBSP1_IRQ_RX,
		.tx_irq         = INT_24XX_MCBSP1_IRQ_TX,
		.ops            = &omap2_mcbsp_ops,
	},
	{
		.phys_base      = OMAP44XX_MCBSP2_BASE,
		.dma_rx_sync    = OMAP44XX_DMA_MCBSP2_RX,
		.dma_tx_sync    = OMAP44XX_DMA_MCBSP2_TX,
		.rx_irq         = INT_24XX_MCBSP2_IRQ_RX,
		.tx_irq         = INT_24XX_MCBSP2_IRQ_TX,
		.ops            = &omap2_mcbsp_ops,
	},
	{
		.phys_base      = OMAP44XX_MCBSP3_BASE,
		.dma_rx_sync    = OMAP44XX_DMA_MCBSP3_RX,
		.dma_tx_sync    = OMAP44XX_DMA_MCBSP3_TX,
		.rx_irq         = INT_24XX_MCBSP3_IRQ_RX,
		.tx_irq         = INT_24XX_MCBSP3_IRQ_TX,
		.ops            = &omap2_mcbsp_ops,
	},
	{
		.phys_base      = OMAP44XX_MCBSP4_BASE,
		.dma_rx_sync    = OMAP44XX_DMA_MCBSP4_RX,
		.dma_tx_sync    = OMAP44XX_DMA_MCBSP4_TX,
		.rx_irq         = INT_24XX_MCBSP4_IRQ_RX,
		.tx_irq         = INT_24XX_MCBSP4_IRQ_TX,
		.ops            = &omap2_mcbsp_ops,
	},
};
#define OMAP44XX_MCBSP_PDATA_SZ		ARRAY_SIZE(omap44xx_mcbsp_pdata)

static int __init omap2_mcbsp_init(void)
{
	if (cpu_is_omap2420())
		omap_mcbsp_count = OMAP2420_MCBSP_PDATA_SZ;
	if (cpu_is_omap2430())
		omap_mcbsp_count = OMAP2430_MCBSP_PDATA_SZ;
	if (cpu_is_omap34xx())
		omap_mcbsp_count = OMAP34XX_MCBSP_PDATA_SZ;
	if (cpu_is_omap44xx())
		omap_mcbsp_count = OMAP44XX_MCBSP_PDATA_SZ;

	mcbsp_ptr = kzalloc(omap_mcbsp_count * sizeof(struct omap_mcbsp *),
								GFP_KERNEL);
	if (!mcbsp_ptr)
		return -ENOMEM;

	if (cpu_is_omap2420())
		omap_mcbsp_register_board_cfg(omap2420_mcbsp_pdata,
						OMAP2420_MCBSP_PDATA_SZ);
	if (cpu_is_omap2430())
		omap_mcbsp_register_board_cfg(omap2430_mcbsp_pdata,
						OMAP2430_MCBSP_PDATA_SZ);
	if (cpu_is_omap34xx())
		omap_mcbsp_register_board_cfg(omap34xx_mcbsp_pdata,
						OMAP34XX_MCBSP_PDATA_SZ);
	if (cpu_is_omap44xx())
		omap_mcbsp_register_board_cfg(omap44xx_mcbsp_pdata,
						OMAP44XX_MCBSP_PDATA_SZ);

	return omap_mcbsp_init();
}
arch_initcall(omap2_mcbsp_init);
