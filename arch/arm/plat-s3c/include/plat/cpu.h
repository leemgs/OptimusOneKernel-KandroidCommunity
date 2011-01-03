



#define IODESC_ENT(x) { (unsigned long)S3C24XX_VA_##x, __phys_to_pfn(S3C24XX_PA_##x), S3C24XX_SZ_##x, MT_DEVICE }

#ifndef MHZ
#define MHZ (1000*1000)
#endif

#define print_mhz(m) ((m) / MHZ), (((m) / 1000) % 1000)


struct s3c24xx_uart_resources;
struct platform_device;
struct s3c2410_uartcfg;
struct map_desc;



struct cpu_table {
	unsigned long	idcode;
	unsigned long	idmask;
	void		(*map_io)(void);
	void		(*init_uarts)(struct s3c2410_uartcfg *cfg, int no);
	void		(*init_clocks)(int xtal);
	int		(*init)(void);
	const char	*name;
};

extern void s3c_init_cpu(unsigned long idcode,
			 struct cpu_table *cpus, unsigned int cputab_size);



extern void s3c24xx_init_irq(void);
extern void s3c64xx_init_irq(u32 vic0, u32 vic1);

extern void s3c24xx_init_io(struct map_desc *mach_desc, int size);
extern void s3c64xx_init_io(struct map_desc *mach_desc, int size);

extern void s3c24xx_init_uarts(struct s3c2410_uartcfg *cfg, int no);

extern void s3c24xx_init_clocks(int xtal);

extern void s3c24xx_init_uartdevs(char *name,
				  struct s3c24xx_uart_resources *res,
				  struct s3c2410_uartcfg *cfg, int no);



struct sys_timer;
extern struct sys_timer s3c24xx_timer;



extern struct sysdev_class s3c2410_sysclass;
extern struct sysdev_class s3c2410a_sysclass;
extern struct sysdev_class s3c2412_sysclass;
extern struct sysdev_class s3c2440_sysclass;
extern struct sysdev_class s3c2442_sysclass;
extern struct sysdev_class s3c2443_sysclass;
extern struct sysdev_class s3c6410_sysclass;
extern struct sysdev_class s3c64xx_sysclass;

