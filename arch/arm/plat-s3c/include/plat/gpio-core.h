



struct s3c_gpio_chip;


struct s3c_gpio_pm {
	void (*save)(struct s3c_gpio_chip *chip);
	void (*resume)(struct s3c_gpio_chip *chip);
};

struct s3c_gpio_cfg;


struct s3c_gpio_chip {
	struct gpio_chip	chip;
	struct s3c_gpio_cfg	*config;
	struct s3c_gpio_pm	*pm;
	void __iomem		*base;
#ifdef CONFIG_PM
	u32			pm_save[4];
#endif
};

static inline struct s3c_gpio_chip *to_s3c_gpio(struct gpio_chip *gpc)
{
	return container_of(gpc, struct s3c_gpio_chip, chip);
}


extern void s3c_gpiolib_add(struct s3c_gpio_chip *chip);



#ifdef CONFIG_S3C_GPIO_TRACK
extern struct s3c_gpio_chip *s3c_gpios[S3C_GPIO_END];

static inline struct s3c_gpio_chip *s3c_gpiolib_getchip(unsigned int chip)
{
	return (chip < S3C_GPIO_END) ? s3c_gpios[chip] : NULL;
}
#else


static inline void s3c_gpiolib_track(struct s3c_gpio_chip *chip) { }
#endif

#ifdef CONFIG_PM
extern struct s3c_gpio_pm s3c_gpio_pm_1bit;
extern struct s3c_gpio_pm s3c_gpio_pm_2bit;
extern struct s3c_gpio_pm s3c_gpio_pm_4bit;
#define __gpio_pm(x) x
#else
#define s3c_gpio_pm_1bit NULL
#define s3c_gpio_pm_2bit NULL
#define s3c_gpio_pm_4bit NULL
#define __gpio_pm(x) NULL

#endif 
