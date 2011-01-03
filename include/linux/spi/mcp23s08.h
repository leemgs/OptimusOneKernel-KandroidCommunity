


struct mcp23s08_chip_info {
	bool	is_present;		
	u8	pullups;		
};

struct mcp23s08_platform_data {
	
	struct mcp23s08_chip_info	chip[4];

	
	unsigned	base;

	void		*context;	

	int		(*setup)(struct spi_device *spi,
					int gpio, unsigned ngpio,
					void *context);
	int		(*teardown)(struct spi_device *spi,
					int gpio, unsigned ngpio,
					void *context);
};
