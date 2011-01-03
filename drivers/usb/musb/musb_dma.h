

#ifndef __MUSB_DMA_H__
#define __MUSB_DMA_H__

struct musb_hw_ep;



#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)

#ifndef CONFIG_MUSB_PIO_ONLY
#define	is_dma_capable()	(1)
#else
#define	is_dma_capable()	(0)
#endif

#ifdef CONFIG_USB_TI_CPPI_DMA
#define	is_cppi_enabled()	1
#else
#define	is_cppi_enabled()	0
#endif

#ifdef CONFIG_USB_TUSB_OMAP_DMA
#define tusb_dma_omap()			1
#else
#define tusb_dma_omap()			0
#endif


enum dma_channel_status {
	
	MUSB_DMA_STATUS_UNKNOWN,
	
	MUSB_DMA_STATUS_FREE,
	
	MUSB_DMA_STATUS_BUSY,
	
	MUSB_DMA_STATUS_BUS_ABORT,
	
	MUSB_DMA_STATUS_CORE_ABORT
};

struct dma_controller;


struct dma_channel {
	void			*private_data;
	
	size_t			max_len;
	size_t			actual_len;
	enum dma_channel_status	status;
	bool			desired_mode;
};


static inline enum dma_channel_status
dma_channel_status(struct dma_channel *c)
{
	return (is_dma_capable() && c) ? c->status : MUSB_DMA_STATUS_UNKNOWN;
}


struct dma_controller {
	int			(*start)(struct dma_controller *);
	int			(*stop)(struct dma_controller *);
	struct dma_channel	*(*channel_alloc)(struct dma_controller *,
					struct musb_hw_ep *, u8 is_tx);
	void			(*channel_release)(struct dma_channel *);
	int			(*channel_program)(struct dma_channel *channel,
							u16 maxpacket, u8 mode,
							dma_addr_t dma_addr,
							u32 length);
	int			(*channel_abort)(struct dma_channel *);
};


extern void musb_dma_completion(struct musb *musb, u8 epnum, u8 transmit);


extern struct dma_controller *__init
dma_controller_create(struct musb *, void __iomem *);

extern void dma_controller_destroy(struct dma_controller *);

#endif	
