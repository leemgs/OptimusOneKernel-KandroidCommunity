

struct pxa2xx_udc_mach_info {
        int  (*udc_is_connected)(void);		
        void (*udc_command)(int cmd);
#define	PXA2XX_UDC_CMD_CONNECT		0	
#define	PXA2XX_UDC_CMD_DISCONNECT	1	

	
	bool	gpio_vbus_inverted;
	u16	gpio_vbus;			
	bool	gpio_pullup_inverted;
	u16	gpio_pullup;			
};

