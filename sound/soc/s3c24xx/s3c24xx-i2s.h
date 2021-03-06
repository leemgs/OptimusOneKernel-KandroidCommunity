

#ifndef S3C24XXI2S_H_
#define S3C24XXI2S_H_


#define S3C24XX_CLKSRC_PCLK 0
#define S3C24XX_CLKSRC_MPLL 1


#define S3C24XX_DIV_MCLK	0
#define S3C24XX_DIV_BCLK	1
#define S3C24XX_DIV_PRESCALER	2


#define S3C24XX_PRESCALE(a,b) \
	(((a - 1) << S3C2410_IISPSR_INTSHIFT) | ((b - 1) << S3C2410_IISPSR_EXTSHFIT))

u32 s3c24xx_i2s_get_clockrate(void);

extern struct snd_soc_dai s3c24xx_i2s_dai;

#endif 
