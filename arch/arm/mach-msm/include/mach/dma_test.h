

#ifndef __MSM_DMA_TEST__
#define __MSM_DMA_TEST__

#include <linux/ioctl.h>

#define MSM_DMA_IOC_MAGIC     0x83


struct msm_dma_alloc_req {
	int size;		
	int bufnum;		
};
#define MSM_DMA_IOALLOC _IOWR(MSM_DMA_IOC_MAGIC, 2, struct msm_dma_alloc_req)


#define MSM_DMA_IOFREE _IOW(MSM_DMA_IOC_MAGIC, 3, int)


#define MSM_DMA_IOFREEALL  _IO(MSM_DMA_IOC_MAGIC, 7)


struct msm_dma_bufxfer {
	void *data;
	int size;
	int bufnum;
};
#define MSM_DMA_IOWBUF _IOW(MSM_DMA_IOC_MAGIC, 4, struct msm_dma_bufxfer)
#define MSM_DMA_IORBUF _IOW(MSM_DMA_IOC_MAGIC, 5, struct msm_dma_bufxfer)


struct msm_dma_scopy {
	int srcbuf;
	int destbuf;
	int size;
};
#define MSM_DMA_IOSCOPY _IOW(MSM_DMA_IOC_MAGIC, 6, struct msm_dma_scopy)

#endif 
