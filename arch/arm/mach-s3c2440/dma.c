

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>

#include <mach/map.h>
#include <mach/dma.h>

#include <plat/dma-plat.h>
#include <plat/cpu.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <plat/regs-ac97.h>
#include <plat/regs-dma.h>
#include <mach/regs-mem.h>
#include <mach/regs-lcd.h>
#include <mach/regs-sdi.h>
#include <plat/regs-iis.h>
#include <plat/regs-spi.h>

static struct s3c24xx_dma_map __initdata s3c2440_dma_mappings[] = {
	[DMACH_XD0] = {
		.name		= "xdreq0",
		.channels[0]	= S3C2410_DCON_CH0_XDREQ0 | DMA_CH_VALID,
	},
	[DMACH_XD1] = {
		.name		= "xdreq1",
		.channels[1]	= S3C2410_DCON_CH1_XDREQ1 | DMA_CH_VALID,
	},
	[DMACH_SDI] = {
		.name		= "sdi",
		.channels[0]	= S3C2410_DCON_CH0_SDI | DMA_CH_VALID,
		.channels[1]	= S3C2440_DCON_CH1_SDI | DMA_CH_VALID,
		.channels[2]	= S3C2410_DCON_CH2_SDI | DMA_CH_VALID,
		.channels[3]	= S3C2410_DCON_CH3_SDI | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_SPI0] = {
		.name		= "spi0",
		.channels[1]	= S3C2410_DCON_CH1_SPI | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_SPI + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + S3C2410_SPRDAT,
	},
	[DMACH_SPI1] = {
		.name		= "spi1",
		.channels[3]	= S3C2410_DCON_CH3_SPI | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_SPI + 0x20 + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + 0x20 + S3C2410_SPRDAT,
	},
	[DMACH_UART0] = {
		.name		= "uart0",
		.channels[0]	= S3C2410_DCON_CH0_UART0 | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1] = {
		.name		= "uart1",
		.channels[1]	= S3C2410_DCON_CH1_UART1 | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2] = {
		.name		= "uart2",
		.channels[3]	= S3C2410_DCON_CH3_UART2 | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
	[DMACH_TIMER] = {
		.name		= "timer",
		.channels[0]	= S3C2410_DCON_CH0_TIMER | DMA_CH_VALID,
		.channels[2]	= S3C2410_DCON_CH2_TIMER | DMA_CH_VALID,
		.channels[3]	= S3C2410_DCON_CH3_TIMER | DMA_CH_VALID,
	},
	[DMACH_I2S_IN] = {
		.name		= "i2s-sdi",
		.channels[1]	= S3C2410_DCON_CH1_I2SSDI | DMA_CH_VALID,
		.channels[2]	= S3C2410_DCON_CH2_I2SSDI | DMA_CH_VALID,
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_I2S_OUT] = {
		.name		= "i2s-sdo",
		.channels[0]	= S3C2440_DCON_CH0_I2SSDO | DMA_CH_VALID,
		.channels[2]	= S3C2410_DCON_CH2_I2SSDO | DMA_CH_VALID,
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_PCM_IN] = {
		.name		= "pcm-in",
		.channels[0]	= S3C2440_DCON_CH0_PCMIN | DMA_CH_VALID,
		.channels[2]	= S3C2440_DCON_CH2_PCMIN | DMA_CH_VALID,
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_PCM_OUT] = {
		.name		= "pcm-out",
		.channels[1]	= S3C2440_DCON_CH1_PCMOUT | DMA_CH_VALID,
		.channels[3]	= S3C2440_DCON_CH3_PCMOUT | DMA_CH_VALID,
		.hw_addr.to	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_MIC_IN] = {
		.name		= "mic-in",
		.channels[2]	= S3C2440_DCON_CH2_MICIN | DMA_CH_VALID,
		.channels[3]	= S3C2440_DCON_CH3_MICIN | DMA_CH_VALID,
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_MIC_DATA,
	},
	[DMACH_USB_EP1] = {
		.name		= "usb-ep1",
		.channels[0]	= S3C2410_DCON_CH0_USBEP1 | DMA_CH_VALID,
	},
	[DMACH_USB_EP2] = {
		.name		= "usb-ep2",
		.channels[1]	= S3C2410_DCON_CH1_USBEP2 | DMA_CH_VALID,
	},
	[DMACH_USB_EP3] = {
		.name		= "usb-ep3",
		.channels[2]	= S3C2410_DCON_CH2_USBEP3 | DMA_CH_VALID,
	},
	[DMACH_USB_EP4] = {
		.name		= "usb-ep4",
		.channels[3]	= S3C2410_DCON_CH3_USBEP4 | DMA_CH_VALID,
	},
};

static void s3c2440_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c24xx_dma_map *map)
{
	chan->dcon = map->channels[chan->number] & ~DMA_CH_VALID;
}

static struct s3c24xx_dma_selection __initdata s3c2440_dma_sel = {
	.select		= s3c2440_dma_select,
	.dcon_mask	= 7 << 24,
	.map		= s3c2440_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c2440_dma_mappings),
};

static struct s3c24xx_dma_order __initdata s3c2440_dma_order = {
	.channels	= {
		[DMACH_SDI]	= {
			.list	= {
				[0]	= 3 | DMA_CH_VALID,
				[1]	= 2 | DMA_CH_VALID,
				[2]	= 1 | DMA_CH_VALID,
				[3]	= 0 | DMA_CH_VALID,
			},
		},
		[DMACH_I2S_IN]	= {
			.list	= {
				[0]	= 1 | DMA_CH_VALID,
				[1]	= 2 | DMA_CH_VALID,
			},
		},
		[DMACH_I2S_OUT]	= {
			.list	= {
				[0]	= 2 | DMA_CH_VALID,
				[1]	= 1 | DMA_CH_VALID,
			},
		},
		[DMACH_PCM_IN] = {
			.list	= {
				[0]	= 2 | DMA_CH_VALID,
				[1]	= 1 | DMA_CH_VALID,
			},
		},
		[DMACH_PCM_OUT] = {
			.list	= {
				[0]	= 1 | DMA_CH_VALID,
				[1]	= 3 | DMA_CH_VALID,
			},
		},
		[DMACH_MIC_IN] = {
			.list	= {
				[0]	= 3 | DMA_CH_VALID,
				[1]	= 2 | DMA_CH_VALID,
			},
		},
	},
};

static int __init s3c2440_dma_add(struct sys_device *sysdev)
{
	s3c2410_dma_init();
	s3c24xx_dma_order_set(&s3c2440_dma_order);
	return s3c24xx_dma_init_map(&s3c2440_dma_sel);
}

static struct sysdev_driver s3c2440_dma_driver = {
	.add	= s3c2440_dma_add,
};

static int __init s3c2440_dma_init(void)
{
	return sysdev_driver_register(&s3c2440_sysclass, &s3c2440_dma_driver);
}

arch_initcall(s3c2440_dma_init);

