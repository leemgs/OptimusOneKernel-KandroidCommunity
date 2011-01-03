

#ifndef __OMAP2_GPMC_H
#define __OMAP2_GPMC_H


#define GPMC_CS_NUM		8

#define GPMC_CS_CONFIG1		0x00
#define GPMC_CS_CONFIG2		0x04
#define GPMC_CS_CONFIG3		0x08
#define GPMC_CS_CONFIG4		0x0c
#define GPMC_CS_CONFIG5		0x10
#define GPMC_CS_CONFIG6		0x14
#define GPMC_CS_CONFIG7		0x18
#define GPMC_CS_NAND_COMMAND	0x1c
#define GPMC_CS_NAND_ADDRESS	0x20
#define GPMC_CS_NAND_DATA	0x24

#define GPMC_CONFIG		0x50
#define GPMC_STATUS		0x54

#define GPMC_CONFIG1_WRAPBURST_SUPP     (1 << 31)
#define GPMC_CONFIG1_READMULTIPLE_SUPP  (1 << 30)
#define GPMC_CONFIG1_READTYPE_ASYNC     (0 << 29)
#define GPMC_CONFIG1_READTYPE_SYNC      (1 << 29)
#define GPMC_CONFIG1_WRITEMULTIPLE_SUPP (1 << 28)
#define GPMC_CONFIG1_WRITETYPE_ASYNC    (0 << 27)
#define GPMC_CONFIG1_WRITETYPE_SYNC     (1 << 27)
#define GPMC_CONFIG1_CLKACTIVATIONTIME(val) ((val & 3) << 25)
#define GPMC_CONFIG1_PAGE_LEN(val)      ((val & 3) << 23)
#define GPMC_CONFIG1_WAIT_READ_MON      (1 << 22)
#define GPMC_CONFIG1_WAIT_WRITE_MON     (1 << 21)
#define GPMC_CONFIG1_WAIT_MON_IIME(val) ((val & 3) << 18)
#define GPMC_CONFIG1_WAIT_PIN_SEL(val)  ((val & 3) << 16)
#define GPMC_CONFIG1_DEVICESIZE(val)    ((val & 3) << 12)
#define GPMC_CONFIG1_DEVICESIZE_16      GPMC_CONFIG1_DEVICESIZE(1)
#define GPMC_CONFIG1_DEVICETYPE(val)    ((val & 3) << 10)
#define GPMC_CONFIG1_DEVICETYPE_NOR     GPMC_CONFIG1_DEVICETYPE(0)
#define GPMC_CONFIG1_DEVICETYPE_NAND    GPMC_CONFIG1_DEVICETYPE(1)
#define GPMC_CONFIG1_MUXADDDATA         (1 << 9)
#define GPMC_CONFIG1_TIME_PARA_GRAN     (1 << 4)
#define GPMC_CONFIG1_FCLK_DIV(val)      (val & 3)
#define GPMC_CONFIG1_FCLK_DIV2          (GPMC_CONFIG1_FCLK_DIV(1))
#define GPMC_CONFIG1_FCLK_DIV3          (GPMC_CONFIG1_FCLK_DIV(2))
#define GPMC_CONFIG1_FCLK_DIV4          (GPMC_CONFIG1_FCLK_DIV(3))


struct gpmc_timings {
	
	u16 sync_clk;

	
	u16 cs_on;		
	u16 cs_rd_off;		
	u16 cs_wr_off;		

	
	u16 adv_on;		
	u16 adv_rd_off;		
	u16 adv_wr_off;		

	
	u16 we_on;		
	u16 we_off;		

	
	u16 oe_on;		
	u16 oe_off;		

	
	u16 page_burst_access;	
	u16 access;		
	u16 rd_cycle;		
	u16 wr_cycle;		

	
	u16 wr_access;		
	u16 wr_data_mux_bus;	
};

extern unsigned int gpmc_ns_to_ticks(unsigned int time_ns);
extern unsigned int gpmc_ticks_to_ns(unsigned int ticks);
extern unsigned int gpmc_round_ns_to_ticks(unsigned int time_ns);
extern unsigned long gpmc_get_fclk_period(void);

extern void gpmc_cs_write_reg(int cs, int idx, u32 val);
extern u32 gpmc_cs_read_reg(int cs, int idx);
extern int gpmc_cs_calc_divider(int cs, unsigned int sync_clk);
extern int gpmc_cs_set_timings(int cs, const struct gpmc_timings *t);
extern int gpmc_cs_request(int cs, unsigned long size, unsigned long *base);
extern void gpmc_cs_free(int cs);
extern int gpmc_cs_set_reserved(int cs, int reserved);
extern int gpmc_cs_reserved(int cs);
extern int gpmc_prefetch_enable(int cs, int dma_mode,
					unsigned int u32_count, int is_write);
extern void gpmc_prefetch_reset(void);
extern int gpmc_prefetch_status(void);
extern void __init gpmc_init(void);

#endif
