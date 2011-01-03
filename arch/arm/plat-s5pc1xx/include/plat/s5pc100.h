


extern  int s5pc100_init(void);
extern void s5pc100_map_io(void);
extern void s5pc100_init_clocks(int xtal);
extern  int s5pc100_register_baseclocks(unsigned long xtal);
extern void s5pc100_init_irq(void);
extern void s5pc100_init_io(struct map_desc *mach_desc, int size);
extern void s5pc100_common_init_uarts(struct s3c2410_uartcfg *cfg, int no);
extern void s5pc100_register_clocks(void);
extern void s5pc100_setup_clocks(void);
extern struct sysdev_class s5pc100_sysclass;

#define s5pc100_init_uarts s5pc100_common_init_uarts


extern void s5pc1xx_init_irq(u32 *vic_valid, int num);
extern void s5pc1xx_init_io(struct map_desc *mach_desc, int size);


extern struct clk clk_hpll;
extern struct clk clk_hd0;
extern struct clk clk_pd0;
extern struct clk clk_54m;
extern struct clk clk_dout_mpll2;
extern void s5pc1xx_register_clocks(void);
extern int s5pc1xx_sclk0_ctrl(struct clk *clk, int enable);
extern int s5pc1xx_sclk1_ctrl(struct clk *clk, int enable);


extern struct s3c24xx_uart_resources s5pc1xx_uart_resources[];
extern struct platform_device s3c_device_g2d;
extern struct platform_device s3c_device_g3d;
extern struct platform_device s3c_device_vpp;
extern struct platform_device s3c_device_tvenc;
extern struct platform_device s3c_device_tvscaler;
extern struct platform_device s3c_device_rotator;
extern struct platform_device s3c_device_jpeg;
extern struct platform_device s3c_device_onenand;
extern struct platform_device s3c_device_usb_otghcd;
extern struct platform_device s3c_device_keypad;
extern struct platform_device s3c_device_ts;
extern struct platform_device s3c_device_g3d;
extern struct platform_device s3c_device_smc911x;
extern struct platform_device s3c_device_fimc0;
extern struct platform_device s3c_device_fimc1;
extern struct platform_device s3c_device_mfc;
extern struct platform_device s3c_device_ac97;
extern struct platform_device s3c_device_fimc0;
extern struct platform_device s3c_device_fimc1;
extern struct platform_device s3c_device_fimc2;

