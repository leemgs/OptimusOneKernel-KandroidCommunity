

struct pca953x_platform_data {
	
	unsigned	gpio_base;

	
	uint16_t	invert;

	void		*context;	

	int		(*setup)(struct i2c_client *client,
				unsigned gpio, unsigned ngpio,
				void *context);
	int		(*teardown)(struct i2c_client *client,
				unsigned gpio, unsigned ngpio,
				void *context);
	char		**names;
};
