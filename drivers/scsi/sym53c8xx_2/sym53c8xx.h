

#ifndef SYM53C8XX_H
#define SYM53C8XX_H



#define	SYM_CONF_DMA_ADDRESSING_MODE CONFIG_SCSI_SYM53C8XX_DMA_ADDRESSING_MODE


#if 1
#define SYM_CONF_NVRAM_SUPPORT		(1)
#endif


#if 1
#define	SYM_LINUX_PROC_INFO_SUPPORT
#define SYM_LINUX_USER_COMMAND_SUPPORT
#define SYM_LINUX_USER_INFO_SUPPORT
#define SYM_LINUX_DEBUG_CONTROL_SUPPORT
#endif


#define SYM_CONF_GENERIC_SUPPORT	(1)


#ifndef CONFIG_SCSI_SYM53C8XX_MAX_TAGS
#define CONFIG_SCSI_SYM53C8XX_MAX_TAGS	(8)
#endif

#if	CONFIG_SCSI_SYM53C8XX_MAX_TAGS < 2
#define SYM_CONF_MAX_TAG	(2)
#elif	CONFIG_SCSI_SYM53C8XX_MAX_TAGS > 256
#define SYM_CONF_MAX_TAG	(256)
#else
#define	SYM_CONF_MAX_TAG	CONFIG_SCSI_SYM53C8XX_MAX_TAGS
#endif

#ifndef	CONFIG_SCSI_SYM53C8XX_DEFAULT_TAGS
#define	CONFIG_SCSI_SYM53C8XX_DEFAULT_TAGS	SYM_CONF_MAX_TAG
#endif


#if	SYM_CONF_MAX_TAG <= 64
#define SYM_CONF_MAX_TAG_ORDER	(6)
#elif	SYM_CONF_MAX_TAG <= 128
#define SYM_CONF_MAX_TAG_ORDER	(7)
#else
#define SYM_CONF_MAX_TAG_ORDER	(8)
#endif


#define SYM_CONF_MAX_SG		(96)


struct sym_driver_setup {
	u_short	max_tag;
	u_char	burst_order;
	u_char	scsi_led;
	u_char	scsi_diff;
	u_char	irq_mode;
	u_char	scsi_bus_check;
	u_char	host_id;

	u_char	verbose;
	u_char	settle_delay;
	u_char	use_nvram;
	u_long	excludes[8];
};

#define SYM_SETUP_MAX_TAG		sym_driver_setup.max_tag
#define SYM_SETUP_BURST_ORDER		sym_driver_setup.burst_order
#define SYM_SETUP_SCSI_LED		sym_driver_setup.scsi_led
#define SYM_SETUP_SCSI_DIFF		sym_driver_setup.scsi_diff
#define SYM_SETUP_IRQ_MODE		sym_driver_setup.irq_mode
#define SYM_SETUP_SCSI_BUS_CHECK	sym_driver_setup.scsi_bus_check
#define SYM_SETUP_HOST_ID		sym_driver_setup.host_id
#define boot_verbose			sym_driver_setup.verbose


#define SYM_LINUX_DRIVER_SETUP	{				\
	.max_tag	= CONFIG_SCSI_SYM53C8XX_DEFAULT_TAGS,	\
	.burst_order	= 7,					\
	.scsi_led	= 1,					\
	.scsi_diff	= 1,					\
	.irq_mode	= 0,					\
	.scsi_bus_check	= 1,					\
	.host_id	= 7,					\
	.verbose	= 0,					\
	.settle_delay	= 3,					\
	.use_nvram	= 1,					\
}

extern struct sym_driver_setup sym_driver_setup;
extern unsigned int sym_debug_flags;
#define DEBUG_FLAGS	sym_debug_flags


#ifndef SYM_CONF_MAX_TARGET
#define SYM_CONF_MAX_TARGET	(16)
#endif


#ifndef SYM_CONF_MAX_LUN
#define SYM_CONF_MAX_LUN	(64)
#endif








#define SYM_CONF_IARB_MAX 3
#define SYM_CONF_SET_IARB_ON_ARB_LOST 1


#define SYM_SETUP_RESIDUAL_SUPPORT 1

#endif 
