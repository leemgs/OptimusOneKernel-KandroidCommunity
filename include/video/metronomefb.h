

#ifndef _LINUX_METRONOMEFB_H_
#define _LINUX_METRONOMEFB_H_


struct metromem_cmd {
	u16 opcode;
	u16 args[((64-2)/2)];
	u16 csum;
};


struct metronomefb_par {
	struct metromem_cmd *metromem_cmd;
	unsigned char *metromem_wfm;
	unsigned char *metromem_img;
	u16 *metromem_img_csum;
	u16 *csum_table;
	dma_addr_t metromem_dma;
	struct fb_info *info;
	struct metronome_board *board;
	wait_queue_head_t waitq;
	u8 frame_count;
	int extra_size;
	int dt;
};


struct metronome_board {
	struct module *owner; 
	void (*set_rst)(struct metronomefb_par *, int);
	void (*set_stdby)(struct metronomefb_par *, int);
	void (*cleanup)(struct metronomefb_par *);
	int (*met_wait_event)(struct metronomefb_par *);
	int (*met_wait_event_intr)(struct metronomefb_par *);
	int (*setup_irq)(struct fb_info *);
	int (*setup_fb)(struct metronomefb_par *);
	int (*setup_io)(struct metronomefb_par *);
	int (*get_panel_type)(void);
	unsigned char *metromem;
	int fw;
	int fh;
	int wfm_size;
	struct fb_info *host_fbinfo; 
};

#endif
