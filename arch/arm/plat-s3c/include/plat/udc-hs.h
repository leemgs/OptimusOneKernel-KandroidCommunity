

enum s3c_hostg_dmamode {
	S3C_HSOTG_DMA_NONE,	
	S3C_HSOTG_DMA_ONLY,	
	S3C_HSOTG_DMA_DRV,	
};


struct s3c_hsotg_plat {
	enum s3c_hostg_dmamode	dma;
	unsigned int		is_osc : 1;
};
