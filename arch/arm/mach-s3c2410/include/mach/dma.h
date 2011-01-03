

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H __FILE__

#include <plat/dma.h>
#include <linux/sysdev.h>

#define MAX_DMA_TRANSFER_SIZE   0x100000 



enum dma_ch {
	DMACH_XD0,
	DMACH_XD1,
	DMACH_SDI,
	DMACH_SPI0,
	DMACH_SPI1,
	DMACH_UART0,
	DMACH_UART1,
	DMACH_UART2,
	DMACH_TIMER,
	DMACH_I2S_IN,
	DMACH_I2S_OUT,
	DMACH_PCM_IN,
	DMACH_PCM_OUT,
	DMACH_MIC_IN,
	DMACH_USB_EP1,
	DMACH_USB_EP2,
	DMACH_USB_EP3,
	DMACH_USB_EP4,
	DMACH_UART0_SRC2,	
	DMACH_UART1_SRC2,
	DMACH_UART2_SRC2,
	DMACH_UART3,		
	DMACH_UART3_SRC2,
	DMACH_MAX,		
};

#define DMACH_LOW_LEVEL	(1<<28)	


#ifndef CONFIG_CPU_S3C2443
#define S3C_DMA_CHANNELS		(4)
#else
#define S3C_DMA_CHANNELS		(6)
#endif



enum s3c2410_dma_state {
	S3C2410_DMA_IDLE,
	S3C2410_DMA_RUNNING,
	S3C2410_DMA_PAUSED
};



enum s3c2410_dma_loadst {
	S3C2410_DMALOAD_NONE,
	S3C2410_DMALOAD_1LOADED,
	S3C2410_DMALOAD_1RUNNING,
	S3C2410_DMALOAD_1LOADED_1RUNNING,
};




#define S3C2410_DMAF_SLOW         (1<<0)   
#define S3C2410_DMAF_AUTOSTART    (1<<1)   

#define S3C2410_DMAF_CIRCULAR	(1 << 2)	



struct s3c2410_dma_buf;



struct s3c2410_dma_buf {
	struct s3c2410_dma_buf	*next;
	int			 magic;		
	int			 size;		
	dma_addr_t		 data;		
	dma_addr_t		 ptr;		
	void			*id;		
};



struct s3c2410_dma_stats {
	unsigned long		loads;
	unsigned long		timeout_longest;
	unsigned long		timeout_shortest;
	unsigned long		timeout_avg;
	unsigned long		timeout_failed;
};

struct s3c2410_dma_map;



struct s3c2410_dma_chan {
	
	unsigned char		 number;      
	unsigned char		 in_use;      
	unsigned char		 irq_claimed; 
	unsigned char		 irq_enabled; 
	unsigned char		 xfer_unit;   

	

	enum s3c2410_dma_state	 state;
	enum s3c2410_dma_loadst	 load_state;
	struct s3c2410_dma_client *client;

	
	enum s3c2410_dmasrc	 source;
	enum dma_ch		 req_ch;
	unsigned long		 dev_addr;
	unsigned long		 load_timeout;
	unsigned int		 flags;		

	struct s3c24xx_dma_map	*map;		

	
	void __iomem		*regs;		
	void __iomem		*addr_reg;	
	unsigned int		 irq;		
	unsigned long		 dcon;		

	
	s3c2410_dma_cbfn_t	 callback_fn;	
	s3c2410_dma_opfn_t	 op_fn;		

	
	struct s3c2410_dma_stats *stats;
	struct s3c2410_dma_stats  stats_store;

	
	struct s3c2410_dma_buf	*curr;		
	struct s3c2410_dma_buf	*next;		
	struct s3c2410_dma_buf	*end;		

	
	struct sys_device	dev;
};

typedef unsigned long dma_device_t;

static inline bool s3c_dma_has_circular(void)
{
	return false;
}

#endif 
