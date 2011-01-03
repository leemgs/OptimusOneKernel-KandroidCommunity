

#define DMACH_LOW_LEVEL (1<<28) 

struct s3c64xx_dma_buff;


struct s3c64xx_dma_buff {
	struct s3c64xx_dma_buff *next;

	void			*pw;
	struct pl080s_lli	*lli;
	dma_addr_t		 lli_dma;
};

struct s3c64xx_dmac;

struct s3c2410_dma_chan {
	unsigned char		 number;      
	unsigned char		 in_use;      
	unsigned char		 bit;	      
	unsigned char		 hw_width;
	unsigned char		 peripheral;

	unsigned int		 flags;
	enum s3c2410_dmasrc	 source;


	dma_addr_t		dev_addr;

	struct s3c2410_dma_client *client;
	struct s3c64xx_dmac	*dmac;		

	void __iomem		*regs;

	
	s3c2410_dma_cbfn_t	 callback_fn;	
	s3c2410_dma_opfn_t	 op_fn;		

	
	struct s3c64xx_dma_buff	*curr;		
	struct s3c64xx_dma_buff	*next;		
	struct s3c64xx_dma_buff	*end;		

	
};

#include <plat/dma-core.h>
