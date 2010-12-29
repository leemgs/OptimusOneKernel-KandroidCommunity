


struct msm_spi_platform_data {
	u32 max_clock_speed;
	const char *clk_name;
	const char *pclk_name;
	int (*gpio_config)(void);
	void (*gpio_release)(void);
	int (*dma_config)(void);
	const char *rsl_id;
	uint32_t pm_lat;
};
