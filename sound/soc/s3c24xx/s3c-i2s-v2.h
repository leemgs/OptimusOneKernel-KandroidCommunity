



#ifndef __SND_SOC_S3C24XX_S3C_I2SV2_I2S_H
#define __SND_SOC_S3C24XX_S3C_I2SV2_I2S_H __FILE__

#define S3C_I2SV2_DIV_BCLK	(1)
#define S3C_I2SV2_DIV_RCLK	(2)
#define S3C_I2SV2_DIV_PRESCALER	(3)


struct s3c_i2sv2_info {
	struct device	*dev;
	void __iomem	*regs;

	struct clk	*iis_pclk;
	struct clk	*iis_cclk;
	struct clk	*iis_clk;

	unsigned char	 master;

	struct s3c24xx_pcm_dma_params	*dma_playback;
	struct s3c24xx_pcm_dma_params	*dma_capture;

	u32		 suspend_iismod;
	u32		 suspend_iiscon;
	u32		 suspend_iispsr;
};

struct s3c_i2sv2_rate_calc {
	unsigned int	clk_div;	
	unsigned int	fs_div;		
};

extern int s3c_i2sv2_iis_calc_rate(struct s3c_i2sv2_rate_calc *info,
				   unsigned int *fstab,
				   unsigned int rate, struct clk *clk);


extern int s3c_i2sv2_probe(struct platform_device *pdev,
			   struct snd_soc_dai *dai,
			   struct s3c_i2sv2_info *i2s,
			   unsigned long base);


extern int s3c_i2sv2_register_dai(struct snd_soc_dai *dai);

#endif 
