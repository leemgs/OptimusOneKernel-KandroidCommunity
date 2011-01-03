



extern void s3c6400_common_init_uarts(struct s3c2410_uartcfg *cfg, int no);
extern void s3c6400_register_clocks(unsigned armclk_divlimit);
extern void s3c6400_setup_clocks(void);

#ifdef CONFIG_CPU_S3C6400

extern  int s3c6400_init(void);
extern void s3c6400_init_irq(void);
extern void s3c6400_map_io(void);
extern void s3c6400_init_clocks(int xtal);

#define s3c6400_init_uarts s3c6400_common_init_uarts

#else
#define s3c6400_init_clocks NULL
#define s3c6400_init_uarts NULL
#define s3c6400_map_io NULL
#define s3c6400_init NULL
#endif

