

#ifndef __ASM_ARCH_SPI_H
#define __ASM_ARCH_SPI_H __FILE__

struct s3c2410_spi_info {
	int			 pin_cs;	
	unsigned int		 num_cs;	
	int			 bus_num;       

	void (*gpio_setup)(struct s3c2410_spi_info *spi, int enable);
	void (*set_cs)(struct s3c2410_spi_info *spi, int cs, int pol);
};



extern void s3c24xx_spi_gpiocfg_bus0_gpe11_12_13(struct s3c2410_spi_info *spi,
						 int enable);

extern void s3c24xx_spi_gpiocfg_bus1_gpg5_6_7(struct s3c2410_spi_info *spi,
					      int enable);

extern void s3c24xx_spi_gpiocfg_bus1_gpd8_9_10(struct s3c2410_spi_info *spi,
					       int enable);

#endif 
