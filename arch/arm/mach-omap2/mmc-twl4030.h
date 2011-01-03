

struct twl4030_hsmmc_info {
	u8	mmc;		
	u8	wires;		
	bool	transceiver;	
	bool	ext_clock;	
	bool	cover_only;	
	bool	nonremovable;	
	bool	power_saving;	
	int	gpio_cd;	
	int	gpio_wp;	
	char	*name;		
	struct device *dev;	
	int	ocr_mask;	
};

#if defined(CONFIG_REGULATOR) && \
	(defined(CONFIG_MMC_OMAP) || defined(CONFIG_MMC_OMAP_MODULE) || \
	 defined(CONFIG_MMC_OMAP_HS) || defined(CONFIG_MMC_OMAP_HS_MODULE))

void twl4030_mmc_init(struct twl4030_hsmmc_info *);

#else

static inline void twl4030_mmc_init(struct twl4030_hsmmc_info *info)
{
}

#endif
