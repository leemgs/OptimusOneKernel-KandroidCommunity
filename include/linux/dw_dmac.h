
#ifndef DW_DMAC_H
#define DW_DMAC_H

#include <linux/dmaengine.h>


struct dw_dma_platform_data {
	unsigned int	nr_channels;
};


enum dw_dma_slave_width {
	DW_DMA_SLAVE_WIDTH_8BIT,
	DW_DMA_SLAVE_WIDTH_16BIT,
	DW_DMA_SLAVE_WIDTH_32BIT,
};


struct dw_dma_slave {
	struct device		*dma_dev;
	dma_addr_t		tx_reg;
	dma_addr_t		rx_reg;
	enum dw_dma_slave_width	reg_width;
	u32			cfg_hi;
	u32			cfg_lo;
};


#define DWC_CFGH_FCMODE		(1 << 0)
#define DWC_CFGH_FIFO_MODE	(1 << 1)
#define DWC_CFGH_PROTCTL(x)	((x) << 2)
#define DWC_CFGH_SRC_PER(x)	((x) << 7)
#define DWC_CFGH_DST_PER(x)	((x) << 11)


#define DWC_CFGL_PRIO(x)	((x) << 5)	
#define DWC_CFGL_LOCK_CH_XFER	(0 << 12)	
#define DWC_CFGL_LOCK_CH_BLOCK	(1 << 12)
#define DWC_CFGL_LOCK_CH_XACT	(2 << 12)
#define DWC_CFGL_LOCK_BUS_XFER	(0 << 14)	
#define DWC_CFGL_LOCK_BUS_BLOCK	(1 << 14)
#define DWC_CFGL_LOCK_BUS_XACT	(2 << 14)
#define DWC_CFGL_LOCK_CH	(1 << 15)	
#define DWC_CFGL_LOCK_BUS	(1 << 16)	
#define DWC_CFGL_HS_DST_POL	(1 << 18)	
#define DWC_CFGL_HS_SRC_POL	(1 << 19)	


struct dw_cyclic_desc {
	struct dw_desc	**desc;
	unsigned long	periods;
	void		(*period_callback)(void *param);
	void		*period_callback_param;
};

struct dw_cyclic_desc *dw_dma_cyclic_prep(struct dma_chan *chan,
		dma_addr_t buf_addr, size_t buf_len, size_t period_len,
		enum dma_data_direction direction);
void dw_dma_cyclic_free(struct dma_chan *chan);
int dw_dma_cyclic_start(struct dma_chan *chan);
void dw_dma_cyclic_stop(struct dma_chan *chan);

dma_addr_t dw_dma_get_src_addr(struct dma_chan *chan);

dma_addr_t dw_dma_get_dst_addr(struct dma_chan *chan);

#endif 
