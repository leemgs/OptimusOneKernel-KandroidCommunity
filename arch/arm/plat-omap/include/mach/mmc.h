

#ifndef __OMAP2_MMC_H
#define __OMAP2_MMC_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/mmc/host.h>

#include <mach/board.h>

#define OMAP15XX_NR_MMC		1
#define OMAP16XX_NR_MMC		2
#define OMAP1_MMC_SIZE		0x080
#define OMAP1_MMC1_BASE		0xfffb7800
#define OMAP1_MMC2_BASE		0xfffb7c00	

#define OMAP24XX_NR_MMC		2
#define OMAP34XX_NR_MMC		3
#define OMAP44XX_NR_MMC		5
#define OMAP2420_MMC_SIZE	OMAP1_MMC_SIZE
#define OMAP3_HSMMC_SIZE	0x200
#define OMAP4_HSMMC_SIZE	0x1000
#define OMAP2_MMC1_BASE		0x4809c000
#define OMAP2_MMC2_BASE		0x480b4000
#define OMAP3_MMC3_BASE		0x480ad000
#define OMAP4_MMC4_BASE		0x480d1000
#define OMAP4_MMC5_BASE		0x480d5000
#define OMAP4_MMC_REG_OFFSET	0x100
#define HSMMC5			(1 << 4)
#define HSMMC4			(1 << 3)
#define HSMMC3			(1 << 2)
#define HSMMC2			(1 << 1)
#define HSMMC1			(1 << 0)

#define OMAP_MMC_MAX_SLOTS	2

struct omap_mmc_platform_data {
	
	struct device *dev;

	
	unsigned nr_slots:2;

	
	unsigned int max_freq;

	
	int (* switch_slot)(struct device *dev, int slot);
	
	int (* init)(struct device *dev);
	void (* cleanup)(struct device *dev);
	void (* shutdown)(struct device *dev);

	
	int (*suspend)(struct device *dev, int slot);
	int (*resume)(struct device *dev, int slot);

	
	int (*get_context_loss_count)(struct device *dev);

	u64 dma_mask;

	struct omap_mmc_slot_data {

		
		u8 wires;

		
		unsigned nomux:1;

		
		unsigned cover:1;

		
		unsigned internal_clock:1;

		
		unsigned nonremovable:1;

		
		unsigned power_saving:1;

		int switch_pin;			
		int gpio_wp;			

		int (* set_bus_mode)(struct device *dev, int slot, int bus_mode);
		int (* set_power)(struct device *dev, int slot, int power_on, int vdd);
		int (* get_ro)(struct device *dev, int slot);
		int (*set_sleep)(struct device *dev, int slot, int sleep,
				 int vdd, int cardsleep);

		
		int (* get_cover_state)(struct device *dev, int slot);

		const char *name;
		u32 ocr_mask;

		
		int card_detect_irq;
		int (* card_detect)(int irq);

		unsigned int ban_openended:1;

	} slots[OMAP_MMC_MAX_SLOTS];
};


extern void omap_mmc_notify_cover_event(struct device *dev, int slot, int is_closed);

#if	defined(CONFIG_MMC_OMAP) || defined(CONFIG_MMC_OMAP_MODULE) || \
	defined(CONFIG_MMC_OMAP_HS) || defined(CONFIG_MMC_OMAP_HS_MODULE)
void omap1_init_mmc(struct omap_mmc_platform_data **mmc_data,
				int nr_controllers);
void omap2_init_mmc(struct omap_mmc_platform_data **mmc_data,
				int nr_controllers);
int omap_mmc_add(const char *name, int id, unsigned long base,
				unsigned long size, unsigned int irq,
				struct omap_mmc_platform_data *data);
#else
static inline void omap1_init_mmc(struct omap_mmc_platform_data **mmc_data,
				int nr_controllers)
{
}
static inline void omap2_init_mmc(struct omap_mmc_platform_data **mmc_data,
				int nr_controllers)
{
}
static inline int omap_mmc_add(const char *name, int id, unsigned long base,
				unsigned long size, unsigned int irq,
				struct omap_mmc_platform_data *data)
{
	return 0;
}

#endif
#endif
