

#ifndef __LINUX_USB_MUSB_H
#define __LINUX_USB_MUSB_H


enum musb_mode {
	MUSB_UNDEFINED = 0,
	MUSB_HOST,		
	MUSB_PERIPHERAL,	
	MUSB_OTG		
};

struct clk;

struct musb_hdrc_eps_bits {
	const char	name[16];
	u8		bits;
};

struct musb_hdrc_config {
	
	unsigned	multipoint:1;	
	unsigned	dyn_fifo:1;	
	unsigned	soft_con:1;	
	unsigned	utm_16:1;	
	unsigned	big_endian:1;	
	unsigned	mult_bulk_tx:1;	
	unsigned	mult_bulk_rx:1;	
	unsigned	high_iso_tx:1;	
	unsigned	high_iso_rx:1;	
	unsigned	dma:1;		
	unsigned	vendor_req:1;	

	u8		num_eps;	
	u8		dma_channels;	
	u8		dyn_fifo_size;	
	u8		vendor_ctrl;	
	u8		vendor_stat;	
	u8		dma_req_chan;	
	u8		ram_bits;	

	struct musb_hdrc_eps_bits *eps_bits;
#ifdef CONFIG_BLACKFIN
        
        unsigned int    gpio_vrsel;
#endif

};

struct musb_hdrc_platform_data {
	
	u8		mode;

	
	const char	*clock;

	
	int		(*set_vbus)(struct device *dev, int is_on);

	
	u8		power;

	
	u8		min_power;

	
	u8		potpgt;

	
	int		(*set_power)(int state);

	
	int		(*set_clock)(struct clk *clock, int is_on);

	
	struct musb_hdrc_config	*config;
};




#define	TUSB6010_OSCCLK_60	16667	
#define	TUSB6010_REFCLK_24	41667	
#define	TUSB6010_REFCLK_19	52083	

#ifdef	CONFIG_ARCH_OMAP2

extern int __init tusb6010_setup_interface(
		struct musb_hdrc_platform_data *data,
		unsigned ps_refclk, unsigned waitpin,
		unsigned async_cs, unsigned sync_cs,
		unsigned irq, unsigned dmachan);

extern int tusb6010_platform_retime(unsigned is_refclk);

#endif	

#endif 
