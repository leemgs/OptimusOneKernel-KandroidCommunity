







extern void s3c2410_gpio_cfgpin(unsigned int pin, unsigned int function);

extern unsigned int s3c2410_gpio_getcfg(unsigned int pin);



extern int s3c2410_gpio_getirq(unsigned int pin);

#ifdef CONFIG_CPU_S3C2400

extern int s3c2400_gpio_getirq(unsigned int pin);

#endif 



extern int s3c2410_gpio_irqfilter(unsigned int pin, unsigned int on,
				  unsigned int config);



extern void s3c2410_gpio_pullup(unsigned int pin, unsigned int to);



extern int s3c2410_gpio_getpull(unsigned int pin);

extern void s3c2410_gpio_setpin(unsigned int pin, unsigned int to);

extern unsigned int s3c2410_gpio_getpin(unsigned int pin);
