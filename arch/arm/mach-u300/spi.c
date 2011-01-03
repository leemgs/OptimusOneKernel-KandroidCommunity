
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/spi/spi.h>
#include <linux/amba/pl022.h>
#include <linux/err.h>
#include "padmux.h"


#ifdef CONFIG_MACH_U300_SPIDUMMY
static void select_dummy_chip(u32 chipselect)
{
	pr_debug("CORE: %s called with CS=0x%x (%s)\n",
		 __func__,
		 chipselect,
		 chipselect ? "unselect chip" : "select chip");
	
}

struct pl022_config_chip dummy_chip_info = {
	
	.lbm = LOOPBACK_ENABLED,
	
	.com_mode = INTERRUPT_TRANSFER,
	.iface = SSP_INTERFACE_MOTOROLA_SPI,
	
	.hierarchy = SSP_MASTER,
	
	.slave_tx_disable = 0,
	
	.endian_tx = SSP_TX_LSB,
	.endian_rx = SSP_RX_LSB,
	.data_size = SSP_DATA_BITS_8, 
	.rx_lev_trig = SSP_RX_1_OR_MORE_ELEM,
	.tx_lev_trig = SSP_TX_1_OR_MORE_EMPTY_LOC,
	.clk_phase = SSP_CLK_SECOND_EDGE,
	.clk_pol = SSP_CLK_POL_IDLE_LOW,
	.ctrl_len = SSP_BITS_12,
	.wait_state = SSP_MWIRE_WAIT_ZERO,
	.duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
	
	.cs_control = select_dummy_chip,
};
#endif

static struct spi_board_info u300_spi_devices[] = {
#ifdef CONFIG_MACH_U300_SPIDUMMY
	{
		
		.modalias       = "spi-dummy",
		
		.platform_data  = NULL,
		
		.controller_data = &dummy_chip_info,
		
		.max_speed_hz   = 1000000,
		.bus_num        = 0, 
		.chip_select    = 0,
		
		.mode           = 0,
	},
#endif
};

static struct pl022_ssp_controller ssp_platform_data = {
	
	.bus_id = 0,
	
	.enable_dma = 0,
	
	.num_chipselect = 3,
};


void __init u300_spi_init(struct amba_device *adev)
{
	struct pmx *pmx;

	adev->dev.platform_data = &ssp_platform_data;
	
	pmx = pmx_get(&adev->dev, U300_APP_PMX_SPI_SETTING);

	if (IS_ERR(pmx))
		dev_warn(&adev->dev, "Could not get padmux handle\n");
	else {
		int ret;

		ret = pmx_activate(&adev->dev, pmx);
		if (IS_ERR_VALUE(ret))
			dev_warn(&adev->dev, "Could not activate padmuxing\n");
	}

}
void __init u300_spi_register_board_devices(void)
{
	
	spi_register_board_info(u300_spi_devices, ARRAY_SIZE(u300_spi_devices));
}
