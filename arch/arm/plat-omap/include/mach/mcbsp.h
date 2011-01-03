
#ifndef __ASM_ARCH_OMAP_MCBSP_H
#define __ASM_ARCH_OMAP_MCBSP_H

#include <linux/completion.h>
#include <linux/spinlock.h>

#include <mach/hardware.h>
#include <mach/clock.h>

#define OMAP730_MCBSP1_BASE	0xfffb1000
#define OMAP730_MCBSP2_BASE	0xfffb1800

#define OMAP1510_MCBSP1_BASE	0xe1011800
#define OMAP1510_MCBSP2_BASE	0xfffb1000
#define OMAP1510_MCBSP3_BASE	0xe1017000

#define OMAP1610_MCBSP1_BASE	0xe1011800
#define OMAP1610_MCBSP2_BASE	0xfffb1000
#define OMAP1610_MCBSP3_BASE	0xe1017000

#define OMAP24XX_MCBSP1_BASE	0x48074000
#define OMAP24XX_MCBSP2_BASE	0x48076000
#define OMAP2430_MCBSP3_BASE	0x4808c000
#define OMAP2430_MCBSP4_BASE	0x4808e000
#define OMAP2430_MCBSP5_BASE	0x48096000

#define OMAP34XX_MCBSP1_BASE	0x48074000
#define OMAP34XX_MCBSP2_BASE	0x49022000
#define OMAP34XX_MCBSP3_BASE	0x49024000
#define OMAP34XX_MCBSP4_BASE	0x49026000
#define OMAP34XX_MCBSP5_BASE	0x48096000

#define OMAP44XX_MCBSP1_BASE	0x49022000
#define OMAP44XX_MCBSP2_BASE	0x49024000
#define OMAP44XX_MCBSP3_BASE	0x49026000
#define OMAP44XX_MCBSP4_BASE	0x48074000

#if defined(CONFIG_ARCH_OMAP15XX) || defined(CONFIG_ARCH_OMAP16XX) || defined(CONFIG_ARCH_OMAP730)

#define OMAP_MCBSP_REG_DRR2	0x00
#define OMAP_MCBSP_REG_DRR1	0x02
#define OMAP_MCBSP_REG_DXR2	0x04
#define OMAP_MCBSP_REG_DXR1	0x06
#define OMAP_MCBSP_REG_SPCR2	0x08
#define OMAP_MCBSP_REG_SPCR1	0x0a
#define OMAP_MCBSP_REG_RCR2	0x0c
#define OMAP_MCBSP_REG_RCR1	0x0e
#define OMAP_MCBSP_REG_XCR2	0x10
#define OMAP_MCBSP_REG_XCR1	0x12
#define OMAP_MCBSP_REG_SRGR2	0x14
#define OMAP_MCBSP_REG_SRGR1	0x16
#define OMAP_MCBSP_REG_MCR2	0x18
#define OMAP_MCBSP_REG_MCR1	0x1a
#define OMAP_MCBSP_REG_RCERA	0x1c
#define OMAP_MCBSP_REG_RCERB	0x1e
#define OMAP_MCBSP_REG_XCERA	0x20
#define OMAP_MCBSP_REG_XCERB	0x22
#define OMAP_MCBSP_REG_PCR0	0x24
#define OMAP_MCBSP_REG_RCERC	0x26
#define OMAP_MCBSP_REG_RCERD	0x28
#define OMAP_MCBSP_REG_XCERC	0x2A
#define OMAP_MCBSP_REG_XCERD	0x2C
#define OMAP_MCBSP_REG_RCERE	0x2E
#define OMAP_MCBSP_REG_RCERF	0x30
#define OMAP_MCBSP_REG_XCERE	0x32
#define OMAP_MCBSP_REG_XCERF	0x34
#define OMAP_MCBSP_REG_RCERG	0x36
#define OMAP_MCBSP_REG_RCERH	0x38
#define OMAP_MCBSP_REG_XCERG	0x3A
#define OMAP_MCBSP_REG_XCERH	0x3C


#define OMAP_MCBSP_REG_XCCR	0x00
#define OMAP_MCBSP_REG_RCCR	0x00

#define AUDIO_MCBSP_DATAWRITE	(OMAP1510_MCBSP1_BASE + OMAP_MCBSP_REG_DXR1)
#define AUDIO_MCBSP_DATAREAD	(OMAP1510_MCBSP1_BASE + OMAP_MCBSP_REG_DRR1)

#define AUDIO_MCBSP		OMAP_MCBSP1
#define AUDIO_DMA_TX		OMAP_DMA_MCBSP1_TX
#define AUDIO_DMA_RX		OMAP_DMA_MCBSP1_RX

#elif defined(CONFIG_ARCH_OMAP24XX) || defined(CONFIG_ARCH_OMAP34XX) || \
	defined(CONFIG_ARCH_OMAP4)

#define OMAP_MCBSP_REG_DRR2	0x00
#define OMAP_MCBSP_REG_DRR1	0x04
#define OMAP_MCBSP_REG_DXR2	0x08
#define OMAP_MCBSP_REG_DXR1	0x0C
#define OMAP_MCBSP_REG_DRR	0x00
#define OMAP_MCBSP_REG_DXR	0x08
#define OMAP_MCBSP_REG_SPCR2	0x10
#define OMAP_MCBSP_REG_SPCR1	0x14
#define OMAP_MCBSP_REG_RCR2	0x18
#define OMAP_MCBSP_REG_RCR1	0x1C
#define OMAP_MCBSP_REG_XCR2	0x20
#define OMAP_MCBSP_REG_XCR1	0x24
#define OMAP_MCBSP_REG_SRGR2	0x28
#define OMAP_MCBSP_REG_SRGR1	0x2C
#define OMAP_MCBSP_REG_MCR2	0x30
#define OMAP_MCBSP_REG_MCR1	0x34
#define OMAP_MCBSP_REG_RCERA	0x38
#define OMAP_MCBSP_REG_RCERB	0x3C
#define OMAP_MCBSP_REG_XCERA	0x40
#define OMAP_MCBSP_REG_XCERB	0x44
#define OMAP_MCBSP_REG_PCR0	0x48
#define OMAP_MCBSP_REG_RCERC	0x4C
#define OMAP_MCBSP_REG_RCERD	0x50
#define OMAP_MCBSP_REG_XCERC	0x54
#define OMAP_MCBSP_REG_XCERD	0x58
#define OMAP_MCBSP_REG_RCERE	0x5C
#define OMAP_MCBSP_REG_RCERF	0x60
#define OMAP_MCBSP_REG_XCERE	0x64
#define OMAP_MCBSP_REG_XCERF	0x68
#define OMAP_MCBSP_REG_RCERG	0x6C
#define OMAP_MCBSP_REG_RCERH	0x70
#define OMAP_MCBSP_REG_XCERG	0x74
#define OMAP_MCBSP_REG_XCERH	0x78
#define OMAP_MCBSP_REG_SYSCON	0x8C
#define OMAP_MCBSP_REG_THRSH2	0x90
#define OMAP_MCBSP_REG_THRSH1	0x94
#define OMAP_MCBSP_REG_IRQST	0xA0
#define OMAP_MCBSP_REG_IRQEN	0xA4
#define OMAP_MCBSP_REG_WAKEUPEN	0xA8
#define OMAP_MCBSP_REG_XCCR	0xAC
#define OMAP_MCBSP_REG_RCCR	0xB0

#define AUDIO_MCBSP_DATAWRITE	(OMAP24XX_MCBSP2_BASE + OMAP_MCBSP_REG_DXR1)
#define AUDIO_MCBSP_DATAREAD	(OMAP24XX_MCBSP2_BASE + OMAP_MCBSP_REG_DRR1)

#define AUDIO_MCBSP		OMAP_MCBSP2
#define AUDIO_DMA_TX		OMAP24XX_DMA_MCBSP2_TX
#define AUDIO_DMA_RX		OMAP24XX_DMA_MCBSP2_RX

#endif


#define RRST			0x0001
#define RRDY			0x0002
#define RFULL			0x0004
#define RSYNC_ERR		0x0008
#define RINTM(value)		((value)<<4)	
#define ABIS			0x0040
#define DXENA			0x0080
#define CLKSTP(value)		((value)<<11)	
#define RJUST(value)		((value)<<13)	
#define ALB			0x8000
#define DLB			0x8000


#define XRST		0x0001
#define XRDY		0x0002
#define XEMPTY		0x0004
#define XSYNC_ERR	0x0008
#define XINTM(value)	((value)<<4)		
#define GRST		0x0040
#define FRST		0x0080
#define SOFT		0x0100
#define FREE		0x0200


#define CLKRP		0x0001
#define CLKXP		0x0002
#define FSRP		0x0004
#define FSXP		0x0008
#define DR_STAT		0x0010
#define DX_STAT		0x0020
#define CLKS_STAT	0x0040
#define SCLKME		0x0080
#define CLKRM		0x0100
#define CLKXM		0x0200
#define FSRM		0x0400
#define FSXM		0x0800
#define RIOEN		0x1000
#define XIOEN		0x2000
#define IDLE_EN		0x4000


#define RWDLEN1(value)		((value)<<5)	
#define RFRLEN1(value)		((value)<<8)	


#define XWDLEN1(value)		((value)<<5)	
#define XFRLEN1(value)		((value)<<8)	


#define RDATDLY(value)		(value)		
#define RFIG			0x0004
#define RCOMPAND(value)		((value)<<3)	
#define RWDLEN2(value)		((value)<<5)	
#define RFRLEN2(value)		((value)<<8)	
#define RPHASE			0x8000


#define XDATDLY(value)		(value)		
#define XFIG			0x0004
#define XCOMPAND(value)		((value)<<3)	
#define XWDLEN2(value)		((value)<<5)	
#define XFRLEN2(value)		((value)<<8)	
#define XPHASE			0x8000


#define CLKGDV(value)		(value)		
#define FWID(value)		((value)<<8)	


#define FPER(value)		(value)		
#define FSGM			0x1000
#define CLKSM			0x2000
#define CLKSP			0x4000
#define GSYNC			0x8000


#define RMCM			0x0001
#define RCBLK(value)		((value)<<2)	
#define RPABLK(value)		((value)<<5)	
#define RPBBLK(value)		((value)<<7)	


#define XMCM(value)		(value)		
#define XCBLK(value)		((value)<<2)	
#define XPABLK(value)		((value)<<5)	
#define XPBBLK(value)		((value)<<7)	


#define EXTCLKGATE		0x8000
#define PPCONNECT		0x4000
#define DXENDLY(value)		((value)<<12)	
#define XFULL_CYCLE		0x0800
#define DILB			0x0020
#define XDMAEN			0x0008
#define XDISABLE		0x0001


#define RFULL_CYCLE		0x0800
#define RDMAEN			0x0008
#define RDISABLE		0x0001


#define CLOCKACTIVITY(value)	((value)<<8)
#define SIDLEMODE(value)	((value)<<3)
#define ENAWAKEUP		0x0004
#define SOFTRST			0x0002


#define MCBSP_DMA_MODE_ELEMENT		0
#define MCBSP_DMA_MODE_THRESHOLD	1
#define MCBSP_DMA_MODE_FRAME		2


#define XEMPTYEOFEN		0x4000
#define XRDYEN			0x0400
#define XEOFEN			0x0200
#define XFSXEN			0x0100
#define XSYNCERREN		0x0080
#define RRDYEN			0x0008
#define REOFEN			0x0004
#define RFSREN			0x0002
#define RSYNCERREN		0x0001


struct omap_mcbsp_reg_cfg {
	u16 spcr2;
	u16 spcr1;
	u16 rcr2;
	u16 rcr1;
	u16 xcr2;
	u16 xcr1;
	u16 srgr2;
	u16 srgr1;
	u16 mcr2;
	u16 mcr1;
	u16 pcr0;
	u16 rcerc;
	u16 rcerd;
	u16 xcerc;
	u16 xcerd;
	u16 rcere;
	u16 rcerf;
	u16 xcere;
	u16 xcerf;
	u16 rcerg;
	u16 rcerh;
	u16 xcerg;
	u16 xcerh;
	u16 xccr;
	u16 rccr;
};

typedef enum {
	OMAP_MCBSP1 = 0,
	OMAP_MCBSP2,
	OMAP_MCBSP3,
	OMAP_MCBSP4,
	OMAP_MCBSP5
} omap_mcbsp_id;

typedef int __bitwise omap_mcbsp_io_type_t;
#define OMAP_MCBSP_IRQ_IO ((__force omap_mcbsp_io_type_t) 1)
#define OMAP_MCBSP_POLL_IO ((__force omap_mcbsp_io_type_t) 2)

typedef enum {
	OMAP_MCBSP_WORD_8 = 0,
	OMAP_MCBSP_WORD_12,
	OMAP_MCBSP_WORD_16,
	OMAP_MCBSP_WORD_20,
	OMAP_MCBSP_WORD_24,
	OMAP_MCBSP_WORD_32,
} omap_mcbsp_word_length;

typedef enum {
	OMAP_MCBSP_CLK_RISING = 0,
	OMAP_MCBSP_CLK_FALLING,
} omap_mcbsp_clk_polarity;

typedef enum {
	OMAP_MCBSP_FS_ACTIVE_HIGH = 0,
	OMAP_MCBSP_FS_ACTIVE_LOW,
} omap_mcbsp_fs_polarity;

typedef enum {
	OMAP_MCBSP_CLK_STP_MODE_NO_DELAY = 0,
	OMAP_MCBSP_CLK_STP_MODE_DELAY,
} omap_mcbsp_clk_stp_mode;



typedef enum {
	OMAP_MCBSP_SPI_MASTER = 0,
	OMAP_MCBSP_SPI_SLAVE,
} omap_mcbsp_spi_mode;

struct omap_mcbsp_spi_cfg {
	omap_mcbsp_spi_mode		spi_mode;
	omap_mcbsp_clk_polarity		rx_clock_polarity;
	omap_mcbsp_clk_polarity		tx_clock_polarity;
	omap_mcbsp_fs_polarity		fsx_polarity;
	u8				clk_div;
	omap_mcbsp_clk_stp_mode		clk_stp_mode;
	omap_mcbsp_word_length		word_length;
};


struct omap_mcbsp_ops {
	void (*request)(unsigned int);
	void (*free)(unsigned int);
};

struct omap_mcbsp_platform_data {
	unsigned long phys_base;
	u8 dma_rx_sync, dma_tx_sync;
	u16 rx_irq, tx_irq;
	struct omap_mcbsp_ops *ops;
#ifdef CONFIG_ARCH_OMAP34XX
	u16 buffer_size;
#endif
};

struct omap_mcbsp {
	struct device *dev;
	unsigned long phys_base;
	void __iomem *io_base;
	u8 id;
	u8 free;
	omap_mcbsp_word_length rx_word_length;
	omap_mcbsp_word_length tx_word_length;

	omap_mcbsp_io_type_t io_type; 
	
	int rx_irq;
	int tx_irq;

	
	u8 dma_rx_sync;
	short dma_rx_lch;
	u8 dma_tx_sync;
	short dma_tx_lch;

	
	struct completion tx_irq_completion;
	struct completion rx_irq_completion;
	struct completion tx_dma_completion;
	struct completion rx_dma_completion;

	
	spinlock_t lock;
	struct omap_mcbsp_platform_data *pdata;
	struct clk *iclk;
	struct clk *fclk;
#ifdef CONFIG_ARCH_OMAP34XX
	int dma_op_mode;
	u16 max_tx_thres;
	u16 max_rx_thres;
#endif
};
extern struct omap_mcbsp **mcbsp_ptr;
extern int omap_mcbsp_count;

int omap_mcbsp_init(void);
void omap_mcbsp_register_board_cfg(struct omap_mcbsp_platform_data *config,
					int size);
void omap_mcbsp_config(unsigned int id, const struct omap_mcbsp_reg_cfg * config);
#ifdef CONFIG_ARCH_OMAP34XX
void omap_mcbsp_set_tx_threshold(unsigned int id, u16 threshold);
void omap_mcbsp_set_rx_threshold(unsigned int id, u16 threshold);
u16 omap_mcbsp_get_max_tx_threshold(unsigned int id);
u16 omap_mcbsp_get_max_rx_threshold(unsigned int id);
int omap_mcbsp_get_dma_op_mode(unsigned int id);
#else
static inline void omap_mcbsp_set_tx_threshold(unsigned int id, u16 threshold)
{ }
static inline void omap_mcbsp_set_rx_threshold(unsigned int id, u16 threshold)
{ }
static inline u16 omap_mcbsp_get_max_tx_threshold(unsigned int id) { return 0; }
static inline u16 omap_mcbsp_get_max_rx_threshold(unsigned int id) { return 0; }
static inline int omap_mcbsp_get_dma_op_mode(unsigned int id) { return 0; }
#endif
int omap_mcbsp_request(unsigned int id);
void omap_mcbsp_free(unsigned int id);
void omap_mcbsp_start(unsigned int id, int tx, int rx);
void omap_mcbsp_stop(unsigned int id, int tx, int rx);
void omap_mcbsp_xmit_word(unsigned int id, u32 word);
u32 omap_mcbsp_recv_word(unsigned int id);

int omap_mcbsp_xmit_buffer(unsigned int id, dma_addr_t buffer, unsigned int length);
int omap_mcbsp_recv_buffer(unsigned int id, dma_addr_t buffer, unsigned int length);
int omap_mcbsp_spi_master_xmit_word_poll(unsigned int id, u32 word);
int omap_mcbsp_spi_master_recv_word_poll(unsigned int id, u32 * word);



void omap_mcbsp_set_spi_mode(unsigned int id, const struct omap_mcbsp_spi_cfg * spi_cfg);


int omap_mcbsp_pollread(unsigned int id, u16 * buf);
int omap_mcbsp_pollwrite(unsigned int id, u16 buf);
int omap_mcbsp_set_io_type(unsigned int id, omap_mcbsp_io_type_t io_type);

#endif
