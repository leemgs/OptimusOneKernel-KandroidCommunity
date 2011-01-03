


#ifndef __BF5XX_SPORT_H__
#define __BF5XX_SPORT_H__

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <asm/dma.h>

struct sport_register {
	u16 tcr1;	u16 reserved0;
	u16 tcr2;	u16 reserved1;
	u16 tclkdiv;	u16 reserved2;
	u16 tfsdiv;	u16 reserved3;
	u32 tx;
	u32 reserved_l0;
	u32 rx;
	u32 reserved_l1;
	u16 rcr1;	u16 reserved4;
	u16 rcr2;	u16 reserved5;
	u16 rclkdiv;	u16 reserved6;
	u16 rfsdiv;	u16 reserved7;
	u16 stat;	u16 reserved8;
	u16 chnl;	u16 reserved9;
	u16 mcmc1;	u16 reserved10;
	u16 mcmc2;	u16 reserved11;
	u32 mtcs0;
	u32 mtcs1;
	u32 mtcs2;
	u32 mtcs3;
	u32 mrcs0;
	u32 mrcs1;
	u32 mrcs2;
	u32 mrcs3;
};

#define DESC_ELEMENT_COUNT 9

struct sport_device {
	int dma_rx_chan;
	int dma_tx_chan;
	int err_irq;
	struct sport_register *regs;

	unsigned char *rx_buf;
	unsigned char *tx_buf;
	unsigned int rx_fragsize;
	unsigned int tx_fragsize;
	unsigned int rx_frags;
	unsigned int tx_frags;
	unsigned int wdsize;

	
	void *dummy_buf;
	unsigned int dummy_count;

	
	struct dmasg *dma_rx_desc;
	struct dmasg *dma_tx_desc;
	unsigned int rx_desc_bytes;
	unsigned int tx_desc_bytes;

	unsigned int rx_run:1; 
	unsigned int tx_run:1; 

	struct dmasg *dummy_rx_desc;
	struct dmasg *dummy_tx_desc;

	struct dmasg *curr_rx_desc;
	struct dmasg *curr_tx_desc;

	int rx_curr_frag;
	int tx_curr_frag;

	unsigned int rcr1;
	unsigned int rcr2;
	int rx_tdm_count;

	unsigned int tcr1;
	unsigned int tcr2;
	int tx_tdm_count;

	void (*rx_callback)(void *data);
	void *rx_data;
	void (*tx_callback)(void *data);
	void *tx_data;
	void (*err_callback)(void *data);
	void *err_data;
	unsigned char *tx_dma_buf;
	unsigned char *rx_dma_buf;
#ifdef CONFIG_SND_BF5XX_MMAP_SUPPORT
	dma_addr_t tx_dma_phy;
	dma_addr_t rx_dma_phy;
	int tx_pos;
	int rx_pos;
	unsigned int tx_buffer_size;
	unsigned int rx_buffer_size;
	int tx_delay_pos;
	int once;
#endif
	void *private_data;
};

extern struct sport_device *sport_handle;

struct sport_param {
	int dma_rx_chan;
	int dma_tx_chan;
	int err_irq;
	struct sport_register *regs;
};

struct sport_device *sport_init(struct sport_param *param, unsigned wdsize,
		unsigned dummy_count, void *private_data);

void sport_done(struct sport_device *sport);




int sport_set_multichannel(struct sport_device *sport, int tdm_count,
		u32 mask, int packed);

int sport_config_rx(struct sport_device *sport,
		unsigned int rcr1, unsigned int rcr2,
		unsigned int clkdiv, unsigned int fsdiv);

int sport_config_tx(struct sport_device *sport,
		unsigned int tcr1, unsigned int tcr2,
		unsigned int clkdiv, unsigned int fsdiv);







int sport_config_rx_dma(struct sport_device *sport, void *buf,
		int fragcount, size_t fragsize_bytes);

int sport_config_tx_dma(struct sport_device *sport, void *buf,
		int fragcount, size_t fragsize_bytes);

int sport_tx_start(struct sport_device *sport);
int sport_tx_stop(struct sport_device *sport);
int sport_rx_start(struct sport_device *sport);
int sport_rx_stop(struct sport_device *sport);


unsigned long sport_curr_offset_rx(struct sport_device *sport);
unsigned long sport_curr_offset_tx(struct sport_device *sport);

void sport_incfrag(struct sport_device *sport, int *frag, int tx);
void sport_decfrag(struct sport_device *sport, int *frag, int tx);

int sport_set_rx_callback(struct sport_device *sport,
		       void (*rx_callback)(void *), void *rx_data);
int sport_set_tx_callback(struct sport_device *sport,
		       void (*tx_callback)(void *), void *tx_data);
int sport_set_err_callback(struct sport_device *sport,
		       void (*err_callback)(void *), void *err_data);

int sport_send_and_recv(struct sport_device *sport, u8 *out_data, \
		u8 *in_data, int len);
#endif 
