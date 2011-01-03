

#if !defined (__SSP_H__)
#    define   __SSP_H__





struct ssp_driver {
	int  (*init)		(void);
	void (*exit)		(void);
	void (*acquire)		(void);
	void (*release)		(void);
	int  (*configure)	(int device, int mode, int speed,
				 int frame_size_write, int frame_size_read);
	void (*chip_select)	(int enable);
	void (*set_callbacks)   (void* handle,
				 irqreturn_t (*callback_tx)(void*),
				 irqreturn_t (*callback_rx)(void*));
	void (*enable)		(void);
	void (*disable)		(void);


	int  (*read)		(void);
	int  (*write)		(u16 data);
	int  (*write_read)	(u16 data);
	void (*flush)		(void);
	void (*write_async)	(void* pv, size_t cb);
	size_t (*write_pos)	(void);
};

	
#define SSP_MODE_SPI		(1)
#define SSP_MODE_SSI		(2)
#define SSP_MODE_MICROWIRE	(3)
#define SSP_MODE_I2S		(4)

	
#define DEVICE_EEPROM	0	
#define DEVICE_MAC	1	
#define DEVICE_CODEC	2	
#define DEVICE_TOUCH	3	






extern struct ssp_driver lh7a400_cpld_ssp_driver;

#endif  
