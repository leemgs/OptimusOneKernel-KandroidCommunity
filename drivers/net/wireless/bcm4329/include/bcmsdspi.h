



#define sd_err(x)
#define sd_trace(x)
#define sd_info(x)
#define sd_debug(x)
#define sd_data(x)
#define sd_ctrl(x)

#define sd_log(x)

#define SDIOH_ASSERT(exp) \
	do { if (!(exp)) \
		printf("!!!ASSERT fail: file %s lines %d", __FILE__, __LINE__); \
	} while (0)

#define BLOCK_SIZE_4318 64
#define BLOCK_SIZE_4328 512


#define SUCCESS	0
#undef ERROR
#define ERROR	1


#define SDIOH_MODE_SPI		0

#define USE_BLOCKMODE		0x2	
#define USE_MULTIBLOCK		0x4

struct sdioh_info {
	uint cfg_bar;                   	
	uint32 caps;                    	
	uint		bar0;			
	osl_t 		*osh;			
	void		*controller;	

	uint		lockcount; 		
	bool		client_intr_enabled;	
	bool		intr_handler_valid;	
	sdioh_cb_fn_t	intr_handler;		
	void		*intr_handler_arg;	
	bool		initialized;		
	uint32		target_dev;		
	uint32		intmask;		
	void		*sdos_info;		

	uint32		controller_type;	
	uint8		version;		
	uint 		irq;			
	uint32 		intrcount;		
	uint32 		local_intrcount;	
	bool 		host_init_done;		
	bool 		card_init_done;		
	bool 		polled_mode;		

	bool		sd_use_dma;		
	bool 		sd_blockmode;		
						
	bool 		use_client_ints;	
	bool		got_hcint;		
						
	int 		adapter_slot;		
	int 		sd_mode;		
	int 		client_block_size[SDIOD_MAX_IOFUNCS];		
	uint32 		data_xfer_count;	
	uint32		cmd53_wr_data;		
	uint32		card_response;		
	uint32		card_rsp_data;		
	uint16 		card_rca;		
	uint8 		num_funcs;		
	uint32 		com_cis_ptr;
	uint32 		func_cis_ptr[SDIOD_MAX_IOFUNCS];
	void		*dma_buf;
	ulong		dma_phys;
	int 		r_cnt;			
	int 		t_cnt;			
};




extern uint sd_msglevel;




extern uint32 *spi_reg_map(osl_t *osh, uintptr addr, int size);
extern void spi_reg_unmap(osl_t *osh, uintptr addr, int size);


extern int spi_register_irq(sdioh_info_t *sd, uint irq);
extern void spi_free_irq(uint irq, sdioh_info_t *sd);


extern void spi_lock(sdioh_info_t *sd);
extern void spi_unlock(sdioh_info_t *sd);


extern int spi_osinit(sdioh_info_t *sd);
extern void spi_osfree(sdioh_info_t *sd);
