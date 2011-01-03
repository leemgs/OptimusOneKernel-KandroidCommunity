

#ifndef _ET1310_ADDRESS_MAP_H_
#define _ET1310_ADDRESS_MAP_H_








#define ET_PM_PHY_SW_COMA		0x40
#define ET_PMCSR_INIT			0x38



#define	ET_INTR_TXDMA_ISR	0x00000008
#define ET_INTR_TXDMA_ERR	0x00000010
#define ET_INTR_RXDMA_XFR_DONE	0x00000020
#define ET_INTR_RXDMA_FB_R0_LOW	0x00000040
#define ET_INTR_RXDMA_FB_R1_LOW	0x00000080
#define ET_INTR_RXDMA_STAT_LOW	0x00000100
#define ET_INTR_RXDMA_ERR	0x00000200
#define ET_INTR_WATCHDOG	0x00004000
#define ET_INTR_WOL		0x00008000
#define ET_INTR_PHY		0x00010000
#define ET_INTR_TXMAC		0x00020000
#define ET_INTR_RXMAC		0x00040000
#define ET_INTR_MAC_STAT	0x00080000
#define ET_INTR_SLV_TIMEOUT	0x00100000









#define ET_MSI_VECTOR	0x0000001F
#define ET_MSI_TC	0x00070000



#define ET_LOOP_MAC	0x00000001
#define ET_LOOP_DMA	0x00000002


typedef struct _GLOBAL_t {			
	u32 txq_start_addr;			
	u32 txq_end_addr;			
	u32 rxq_start_addr;			
	u32 rxq_end_addr;			
	u32 pm_csr;				
	u32 unused;				
	u32 int_status;				
	u32 int_mask;				
	u32 int_alias_clr_en;			
	u32 int_status_alias;			
	u32 sw_reset;				
	u32 slv_timer;				
	u32 msi_config;				
	u32 loopback;			
	u32 watchdog_timer;			
} GLOBAL_t, *PGLOBAL_t;








#define ET_TXDMA_CSR_HALT	0x00000001
#define ET_TXDMA_DROP_TLP	0x00000002
#define ET_TXDMA_CACHE_THRS	0x000000F0
#define ET_TXDMA_CACHE_SHIFT	4
#define ET_TXDMA_SNGL_EPKT	0x00000100
#define ET_TXDMA_CLASS		0x00001E00






typedef union _TXDMA_PR_NUM_DES_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:22;	
		u32 pr_ndes:10;	
#else
		u32 pr_ndes:10;	
		u32 unused:22;	
#endif
	} bits;
} TXDMA_PR_NUM_DES_t, *PTXDMA_PR_NUM_DES_t;


#define ET_DMA10_MASK		0x3FF	
#define ET_DMA10_WRAP		0x400
#define ET_DMA4_MASK		0x00F	
#define ET_DMA4_WRAP		0x010

#define INDEX10(x)	((x) & ET_DMA10_MASK)
#define INDEX4(x)	((x) & ET_DMA4_MASK)

extern inline void add_10bit(u32 *v, int n)
{
	*v = INDEX10(*v + n) | (*v & ET_DMA10_WRAP);
}




typedef struct _TXDMA_t {		
	u32 csr;			
	u32 pr_base_hi;			
	u32 pr_base_lo;			
	TXDMA_PR_NUM_DES_t pr_num_des;	
	u32 txq_wr_addr;		
	u32 txq_wr_addr_ext;		
	u32 txq_rd_addr;		
	u32 dma_wb_base_hi;		
	u32 dma_wb_base_lo;		
	u32 service_request;		
	u32 service_complete;		
	u32 cache_rd_index;		
	u32 cache_wr_index;		
	u32 TxDmaError;			
	u32 DescAbortCount;		
	u32 PayloadAbortCnt;		
	u32 WriteBackAbortCnt;		
	u32 DescTimeoutCnt;		
	u32 PayloadTimeoutCnt;		
	u32 WriteBackTimeoutCnt;	
	u32 DescErrorCount;		
	u32 PayloadErrorCnt;		
	u32 WriteBackErrorCnt;		
	u32 DroppedTLPCount;		
	u32 NewServiceComplete;		
	u32 EthernetPacketCount;	
} TXDMA_t, *PTXDMA_t;







typedef union _RXDMA_CSR_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused2:14;		
		u32 halt_status:1;	
		u32 pkt_done_flush:1;	
		u32 pkt_drop_disable:1;	
		u32 unused1:1;		
		u32 fbr1_enable:1;	
		u32 fbr1_size:2;	
		u32 fbr0_enable:1;	
		u32 fbr0_size:2;	
		u32 dma_big_endian:1;	
		u32 pkt_big_endian:1;	
		u32 psr_big_endian:1;	
		u32 fbr_big_endian:1;	
		u32 tc:3;		
		u32 halt:1;		
#else
		u32 halt:1;		
		u32 tc:3;		
		u32 fbr_big_endian:1;	
		u32 psr_big_endian:1;	
		u32 pkt_big_endian:1;	
		u32 dma_big_endian:1;	
		u32 fbr0_size:2;	
		u32 fbr0_enable:1;	
		u32 fbr1_size:2;	
		u32 fbr1_enable:1;	
		u32 unused1:1;		
		u32 pkt_drop_disable:1;	
		u32 pkt_done_flush:1;	
		u32 halt_status:1;	
		u32 unused2:14;		
#endif
	} bits;
} RXDMA_CSR_t, *PRXDMA_CSR_t;






typedef union _RXDMA_NUM_PKT_DONE_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:24;	
		u32 num_done:8;	
#else
		u32 num_done:8;	
		u32 unused:24;	
#endif
	} bits;
} RXDMA_NUM_PKT_DONE_t, *PRXDMA_NUM_PKT_DONE_t;


typedef union _RXDMA_MAX_PKT_TIME_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:14;		
		u32 time_done:18;	
#else
		u32 time_done:18;	
		u32 unused:14;		
#endif
	} bits;
} RXDMA_MAX_PKT_TIME_t, *PRXDMA_MAX_PKT_TIME_t;












typedef union _RXDMA_PSR_NUM_DES_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:20;		
		u32 psr_ndes:12;	
#else
		u32 psr_ndes:12;	
		u32 unused:20;		
#endif
	} bits;
} RXDMA_PSR_NUM_DES_t, *PRXDMA_PSR_NUM_DES_t;


typedef union _RXDMA_PSR_AVAIL_OFFSET_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:19;		
		u32 psr_avail_wrap:1;	
		u32 psr_avail:12;	
#else
		u32 psr_avail:12;	
		u32 psr_avail_wrap:1;	
		u32 unused:19;		
#endif
	} bits;
} RXDMA_PSR_AVAIL_OFFSET_t, *PRXDMA_PSR_AVAIL_OFFSET_t;


typedef union _RXDMA_PSR_FULL_OFFSET_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:19;		
		u32 psr_full_wrap:1;	
		u32 psr_full:12;	
#else
		u32 psr_full:12;	
		u32 psr_full_wrap:1;	
		u32 unused:19;		
#endif
	} bits;
} RXDMA_PSR_FULL_OFFSET_t, *PRXDMA_PSR_FULL_OFFSET_t;


typedef union _RXDMA_PSR_ACCESS_INDEX_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:27;	
		u32 psr_ai:5;	
#else
		u32 psr_ai:5;	
		u32 unused:27;	
#endif
	} bits;
} RXDMA_PSR_ACCESS_INDEX_t, *PRXDMA_PSR_ACCESS_INDEX_t;


typedef union _RXDMA_PSR_MIN_DES_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:20;	
		u32 psr_min:12;	
#else
		u32 psr_min:12;	
		u32 unused:20;	
#endif
	} bits;
} RXDMA_PSR_MIN_DES_t, *PRXDMA_PSR_MIN_DES_t;






typedef union _RXDMA_FBR_NUM_DES_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:22;		
		u32 fbr_ndesc:10;	
#else
		u32 fbr_ndesc:10;	
		u32 unused:22;		
#endif
	} bits;
} RXDMA_FBR_NUM_DES_t, *PRXDMA_FBR_NUM_DES_t;






typedef union _RXDMA_FBC_RD_INDEX_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:27;	
		u32 fbc_rdi:5;	
#else
		u32 fbc_rdi:5;	
		u32 unused:27;	
#endif
	} bits;
} RXDMA_FBC_RD_INDEX_t, *PRXDMA_FBC_RD_INDEX_t;


typedef union _RXDMA_FBR_MIN_DES_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:22;	
		u32 fbr_min:10;	
#else
		u32 fbr_min:10;	
		u32 unused:22;	
#endif
	} bits;
} RXDMA_FBR_MIN_DES_t, *PRXDMA_FBR_MIN_DES_t;














typedef struct _RXDMA_t {				
	RXDMA_CSR_t csr;				
	u32 dma_wb_base_lo;				
	u32 dma_wb_base_hi;				
	RXDMA_NUM_PKT_DONE_t num_pkt_done;		
	RXDMA_MAX_PKT_TIME_t max_pkt_time;		
	u32 rxq_rd_addr;				
	u32 rxq_rd_addr_ext;			
	u32 rxq_wr_addr;				
	u32 psr_base_lo;				
	u32 psr_base_hi;				
	RXDMA_PSR_NUM_DES_t psr_num_des;		
	RXDMA_PSR_AVAIL_OFFSET_t psr_avail_offset;	
	RXDMA_PSR_FULL_OFFSET_t psr_full_offset;	
	RXDMA_PSR_ACCESS_INDEX_t psr_access_index;	
	RXDMA_PSR_MIN_DES_t psr_min_des;		
	u32 fbr0_base_lo;				
	u32 fbr0_base_hi;				
	RXDMA_FBR_NUM_DES_t fbr0_num_des;		
	u32 fbr0_avail_offset;			
	u32 fbr0_full_offset;			
	RXDMA_FBC_RD_INDEX_t fbr0_rd_index;		
	RXDMA_FBR_MIN_DES_t fbr0_min_des;		
	u32 fbr1_base_lo;				
	u32 fbr1_base_hi;				
	RXDMA_FBR_NUM_DES_t fbr1_num_des;		
	u32 fbr1_avail_offset;			
	u32 fbr1_full_offset;			
	RXDMA_FBC_RD_INDEX_t fbr1_rd_index;		
	RXDMA_FBR_MIN_DES_t fbr1_min_des;		
} RXDMA_t, *PRXDMA_t;







typedef union _TXMAC_CTL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:24;		
		u32 cklseg_diable:1;	
		u32 ckbcnt_disable:1;	
		u32 cksegnum:1;		
		u32 async_disable:1;	
		u32 fc_disable:1;	
		u32 mcif_disable:1;	
		u32 mif_disable:1;	
		u32 txmac_en:1;		
#else
		u32 txmac_en:1;		
		u32 mif_disable:1;	
		u32 mcif_disable:1;	
		u32 fc_disable:1;	
		u32 async_disable:1;	
		u32 cksegnum:1;		
		u32 ckbcnt_disable:1;	
		u32 cklseg_diable:1;	
		u32 unused:24;		
#endif
	} bits;
} TXMAC_CTL_t, *PTXMAC_CTL_t;


typedef union _TXMAC_SHADOW_PTR_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:5;	
		u32 txq_rd_ptr:11;	
		u32 reserved:5;		
		u32 txq_wr_ptr:11;	
#else
		u32 txq_wr_ptr:11;	
		u32 reserved:5;		
		u32 txq_rd_ptr:11;	
		u32 reserved2:5;	
#endif
	} bits;
} TXMAC_SHADOW_PTR_t, *PTXMAC_SHADOW_PTR_t;


typedef union _TXMAC_ERR_CNT_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:20;		
		u32 reserved:4;		
		u32 txq_underrun:4;	
		u32 fifo_underrun:4;	
#else
		u32 fifo_underrun:4;	
		u32 txq_underrun:4;	
		u32 reserved:4;		
		u32 unused:20;		
#endif
	} bits;
} TXMAC_ERR_CNT_t, *PTXMAC_ERR_CNT_t;


typedef union _TXMAC_MAX_FILL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:20;		
		u32 max_fill:12;	
#else
		u32 max_fill:12;	
		u32 unused:20;		
#endif
	} bits;
} TXMAC_MAX_FILL_t, *PTXMAC_MAX_FILL_t;


typedef union _TXMAC_CF_PARAM_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 cfep:16;	
		u32 cfpt:16;	
#else
		u32 cfpt:16;	
		u32 cfep:16;	
#endif
	} bits;
} TXMAC_CF_PARAM_t, *PTXMAC_CF_PARAM_t;


typedef union _TXMAC_TXTEST_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused2:15;		
		u32 reserved1:1;	
		u32 txtest_en:1;	
		u32 unused1:4;		
		u32 txqtest_ptr:11;	
#else
		u32 txqtest_ptr:11;	
		u32 unused1:4;		
		u32 txtest_en:1;	
		u32 reserved1:1;	
		u32 unused2:15;		
#endif
	} bits;
} TXMAC_TXTEST_t, *PTXMAC_TXTEST_t;


typedef union _TXMAC_ERR_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused2:23;		
		u32 fifo_underrun:1;	
		u32 unused1:2;		
		u32 ctrl2_err:1;	
		u32 txq_underrun:1;	
		u32 bcnt_err:1;		
		u32 lseg_err:1;		
		u32 segnum_err:1;	
		u32 seg0_err:1;		
#else
		u32 seg0_err:1;		
		u32 segnum_err:1;	
		u32 lseg_err:1;		
		u32 bcnt_err:1;		
		u32 txq_underrun:1;	
		u32 ctrl2_err:1;	
		u32 unused1:2;		
		u32 fifo_underrun:1;	
		u32 unused2:23;		
#endif
	} bits;
} TXMAC_ERR_t, *PTXMAC_ERR_t;


typedef union _TXMAC_ERR_INT_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused2:23;		
		u32 fifo_underrun:1;	
		u32 unused1:2;		
		u32 ctrl2_err:1;	
		u32 txq_underrun:1;	
		u32 bcnt_err:1;		
		u32 lseg_err:1;		
		u32 segnum_err:1;	
		u32 seg0_err:1;		
#else
		u32 seg0_err:1;		
		u32 segnum_err:1;	
		u32 lseg_err:1;		
		u32 bcnt_err:1;		
		u32 txq_underrun:1;	
		u32 ctrl2_err:1;	
		u32 unused1:2;		
		u32 fifo_underrun:1;	
		u32 unused2:23;		
#endif
	} bits;
} TXMAC_ERR_INT_t, *PTXMAC_ERR_INT_t;


typedef union _TXMAC_CP_CTRL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:30;		
		u32 bp_req:1;		
		u32 bp_xonxoff:1;	
#else
		u32 bp_xonxoff:1;	
		u32 bp_req:1;		
		u32 unused:30;		
#endif
	} bits;
} TXMAC_BP_CTRL_t, *PTXMAC_BP_CTRL_t;


typedef struct _TXMAC_t {		
	TXMAC_CTL_t ctl;		
	TXMAC_SHADOW_PTR_t shadow_ptr;	
	TXMAC_ERR_CNT_t err_cnt;	
	TXMAC_MAX_FILL_t max_fill;	
	TXMAC_CF_PARAM_t cf_param;	
	TXMAC_TXTEST_t tx_test;		
	TXMAC_ERR_t err;		
	TXMAC_ERR_INT_t err_int;	
	TXMAC_BP_CTRL_t bp_ctrl;	
} TXMAC_t, *PTXMAC_t;






typedef union _RXMAC_CTRL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:25;		
		u32 rxmac_int_disable:1;	
		u32 async_disable:1;		
		u32 mif_disable:1;		
		u32 wol_disable:1;		
		u32 pkt_filter_disable:1;	
		u32 mcif_disable:1;		
		u32 rxmac_en:1;			
#else
		u32 rxmac_en:1;			
		u32 mcif_disable:1;		
		u32 pkt_filter_disable:1;	
		u32 wol_disable:1;		
		u32 mif_disable:1;		
		u32 async_disable:1;		
		u32 rxmac_int_disable:1;	
		u32 reserved:25;		
#endif
	} bits;
} RXMAC_CTRL_t, *PRXMAC_CTRL_t;


typedef union _RXMAC_WOL_CTL_CRC0_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 crc0:16;		
		u32 reserve:4;		
		u32 ignore_pp:1;	
		u32 ignore_mp:1;	
		u32 clr_intr:1;		
		u32 ignore_link_chg:1;	
		u32 ignore_uni:1;	
		u32 ignore_multi:1;	
		u32 ignore_broad:1;	
		u32 valid_crc4:1;	
		u32 valid_crc3:1;	
		u32 valid_crc2:1;	
		u32 valid_crc1:1;	
		u32 valid_crc0:1;	
#else
		u32 valid_crc0:1;	
		u32 valid_crc1:1;	
		u32 valid_crc2:1;	
		u32 valid_crc3:1;	
		u32 valid_crc4:1;	
		u32 ignore_broad:1;	
		u32 ignore_multi:1;	
		u32 ignore_uni:1;	
		u32 ignore_link_chg:1;	
		u32 clr_intr:1;		
		u32 ignore_mp:1;	
		u32 ignore_pp:1;	
		u32 reserve:4;		
		u32 crc0:16;		
#endif
	} bits;
} RXMAC_WOL_CTL_CRC0_t, *PRXMAC_WOL_CTL_CRC0_t;


typedef union _RXMAC_WOL_CRC12_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 crc2:16;	
		u32 crc1:16;	
#else
		u32 crc1:16;	
		u32 crc2:16;	
#endif
	} bits;
} RXMAC_WOL_CRC12_t, *PRXMAC_WOL_CRC12_t;


typedef union _RXMAC_WOL_CRC34_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 crc4:16;	
		u32 crc3:16;	
#else
		u32 crc3:16;	
		u32 crc4:16;	
#endif
	} bits;
} RXMAC_WOL_CRC34_t, *PRXMAC_WOL_CRC34_t;


typedef union _RXMAC_WOL_SA_LO_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 sa3:8;	
		u32 sa4:8;	
		u32 sa5:8;	
		u32 sa6:8;	
#else
		u32 sa6:8;	
		u32 sa5:8;	
		u32 sa4:8;	
		u32 sa3:8;	
#endif
	} bits;
} RXMAC_WOL_SA_LO_t, *PRXMAC_WOL_SA_LO_t;


typedef union _RXMAC_WOL_SA_HI_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:16;	
		u32 sa1:8;		
		u32 sa2:8;		
#else
		u32 sa2:8;		
		u32 sa1:8;		
		u32 reserved:16;	
#endif
	} bits;
} RXMAC_WOL_SA_HI_t, *PRXMAC_WOL_SA_HI_t;




typedef union _RXMAC_UNI_PF_ADDR1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 addr1_3:8;	
		u32 addr1_4:8;	
		u32 addr1_5:8;	
		u32 addr1_6:8;	
#else
		u32 addr1_6:8;	
		u32 addr1_5:8;	
		u32 addr1_4:8;	
		u32 addr1_3:8;	
#endif
	} bits;
} RXMAC_UNI_PF_ADDR1_t, *PRXMAC_UNI_PF_ADDR1_t;


typedef union _RXMAC_UNI_PF_ADDR2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 addr2_3:8;	
		u32 addr2_4:8;	
		u32 addr2_5:8;	
		u32 addr2_6:8;	
#else
		u32 addr2_6:8;	
		u32 addr2_5:8;	
		u32 addr2_4:8;	
		u32 addr2_3:8;	
#endif
	} bits;
} RXMAC_UNI_PF_ADDR2_t, *PRXMAC_UNI_PF_ADDR2_t;


typedef union _RXMAC_UNI_PF_ADDR3_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 addr2_1:8;	
		u32 addr2_2:8;	
		u32 addr1_1:8;	
		u32 addr1_2:8;	
#else
		u32 addr1_2:8;	
		u32 addr1_1:8;	
		u32 addr2_2:8;	
		u32 addr2_1:8;	
#endif
	} bits;
} RXMAC_UNI_PF_ADDR3_t, *PRXMAC_UNI_PF_ADDR3_t;




typedef union _RXMAC_PF_CTRL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused2:9;		
		u32 min_pkt_size:7;	
		u32 unused1:12;		
		u32 filter_frag_en:1;	
		u32 filter_uni_en:1;	
		u32 filter_multi_en:1;	
		u32 filter_broad_en:1;	
#else
		u32 filter_broad_en:1;	
		u32 filter_multi_en:1;	
		u32 filter_uni_en:1;	
		u32 filter_frag_en:1;	
		u32 unused1:12;		
		u32 min_pkt_size:7;	
		u32 unused2:9;		
#endif
	} bits;
} RXMAC_PF_CTRL_t, *PRXMAC_PF_CTRL_t;


typedef union _RXMAC_MCIF_CTRL_MAX_SEG_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:22;	
		u32 max_size:8;	
		u32 fc_en:1;	
		u32 seg_en:1;	
#else
		u32 seg_en:1;	
		u32 fc_en:1;	
		u32 max_size:8;	
		u32 reserved:22;	
#endif
	} bits;
} RXMAC_MCIF_CTRL_MAX_SEG_t, *PRXMAC_MCIF_CTRL_MAX_SEG_t;


typedef union _RXMAC_MCIF_WATER_MARK_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:6;	
		u32 mark_hi:10;	
		u32 reserved1:6;	
		u32 mark_lo:10;	
#else
		u32 mark_lo:10;	
		u32 reserved1:6;	
		u32 mark_hi:10;	
		u32 reserved2:6;	
#endif
	} bits;
} RXMAC_MCIF_WATER_MARK_t, *PRXMAC_MCIF_WATER_MARK_t;


typedef union _RXMAC_RXQ_DIAG_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:6;	
		u32 rd_ptr:10;	
		u32 reserved1:6;	
		u32 wr_ptr:10;	
#else
		u32 wr_ptr:10;	
		u32 reserved1:6;	
		u32 rd_ptr:10;	
		u32 reserved2:6;	
#endif
	} bits;
} RXMAC_RXQ_DIAG_t, *PRXMAC_RXQ_DIAG_t;


typedef union _RXMAC_SPACE_AVAIL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:15;		
		u32 space_avail_en:1;	
		u32 reserved1:6;		
		u32 space_avail:10;	
#else
		u32 space_avail:10;	
		u32 reserved1:6;		
		u32 space_avail_en:1;	
		u32 reserved2:15;		
#endif
	} bits;
} RXMAC_SPACE_AVAIL_t, *PRXMAC_SPACE_AVAIL_t;


typedef union _RXMAC_MIF_CTL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserve:14;		
		u32 drop_pkt_en:1;		
		u32 drop_pkt_mask:17;	
#else
		u32 drop_pkt_mask:17;	
		u32 drop_pkt_en:1;		
		u32 reserve:14;		
#endif
	} bits;
} RXMAC_MIF_CTL_t, *PRXMAC_MIF_CTL_t;


typedef union _RXMAC_ERROR_REG_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserve:28;	
		u32 mif:1;		
		u32 async:1;	
		u32 pkt_filter:1;	
		u32 mcif:1;	
#else
		u32 mcif:1;	
		u32 pkt_filter:1;	
		u32 async:1;	
		u32 mif:1;		
		u32 reserve:28;	
#endif
	} bits;
} RXMAC_ERROR_REG_t, *PRXMAC_ERROR_REG_t;


typedef struct _RXMAC_t {				
	RXMAC_CTRL_t ctrl;				
	RXMAC_WOL_CTL_CRC0_t crc0;			
	RXMAC_WOL_CRC12_t crc12;			
	RXMAC_WOL_CRC34_t crc34;			
	RXMAC_WOL_SA_LO_t sa_lo;			
	RXMAC_WOL_SA_HI_t sa_hi;			
	u32 mask0_word0;				
	u32 mask0_word1;				
	u32 mask0_word2;				
	u32 mask0_word3;				
	u32 mask1_word0;				
	u32 mask1_word1;				
	u32 mask1_word2;				
	u32 mask1_word3;				
	u32 mask2_word0;				
	u32 mask2_word1;				
	u32 mask2_word2;				
	u32 mask2_word3;				
	u32 mask3_word0;				
	u32 mask3_word1;				
	u32 mask3_word2;				
	u32 mask3_word3;				
	u32 mask4_word0;				
	u32 mask4_word1;				
	u32 mask4_word2;				
	u32 mask4_word3;				
	RXMAC_UNI_PF_ADDR1_t uni_pf_addr1;		
	RXMAC_UNI_PF_ADDR2_t uni_pf_addr2;		
	RXMAC_UNI_PF_ADDR3_t uni_pf_addr3;		
	u32 multi_hash1;				
	u32 multi_hash2;				
	u32 multi_hash3;				
	u32 multi_hash4;				
	RXMAC_PF_CTRL_t pf_ctrl;			
	RXMAC_MCIF_CTRL_MAX_SEG_t mcif_ctrl_max_seg;	
	RXMAC_MCIF_WATER_MARK_t mcif_water_mark;	
	RXMAC_RXQ_DIAG_t rxq_diag;			
	RXMAC_SPACE_AVAIL_t space_avail;		

	RXMAC_MIF_CTL_t mif_ctrl;			
	RXMAC_ERROR_REG_t err_reg;			
} RXMAC_t, *PRXMAC_t;







typedef union _MAC_CFG1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 soft_reset:1;		
		u32 sim_reset:1;		
		u32 reserved3:10;		
		u32 reset_rx_mc:1;		
		u32 reset_tx_mc:1;		
		u32 reset_rx_fun:1;	
		u32 reset_tx_fun:1;	
		u32 reserved2:7;		
		u32 loop_back:1;		
		u32 reserved1:2;		
		u32 rx_flow:1;		
		u32 tx_flow:1;		
		u32 syncd_rx_en:1;		
		u32 rx_enable:1;		
		u32 syncd_tx_en:1;		
		u32 tx_enable:1;		
#else
		u32 tx_enable:1;		
		u32 syncd_tx_en:1;		
		u32 rx_enable:1;		
		u32 syncd_rx_en:1;		
		u32 tx_flow:1;		
		u32 rx_flow:1;		
		u32 reserved1:2;		
		u32 loop_back:1;		
		u32 reserved2:7;		
		u32 reset_tx_fun:1;	
		u32 reset_rx_fun:1;	
		u32 reset_tx_mc:1;		
		u32 reset_rx_mc:1;		
		u32 reserved3:10;		
		u32 sim_reset:1;		
		u32 soft_reset:1;		
#endif
	} bits;
} MAC_CFG1_t, *PMAC_CFG1_t;


typedef union _MAC_CFG2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved3:16;		
		u32 preamble_len:4;	
		u32 reserved2:2;		
		u32 if_mode:2;		
		u32 reserved1:2;		
		u32 huge_frame:1;		
		u32 len_check:1;		
		u32 undefined:1;		
		u32 pad_crc:1;		
		u32 crc_enable:1;		
		u32 full_duplex:1;		
#else
		u32 full_duplex:1;		
		u32 crc_enable:1;		
		u32 pad_crc:1;		
		u32 undefined:1;		
		u32 len_check:1;		
		u32 huge_frame:1;		
		u32 reserved1:2;		
		u32 if_mode:2;		
		u32 reserved2:2;		
		u32 preamble_len:4;	
		u32 reserved3:16;		
#endif
	} bits;
} MAC_CFG2_t, *PMAC_CFG2_t;


typedef union _MAC_IPG_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:1;		
		u32 non_B2B_ipg_1:7;	
		u32 undefined2:1;		
		u32 non_B2B_ipg_2:7;	
		u32 min_ifg_enforce:8;	
		u32 undefined1:1;		
		u32 B2B_ipg:7;		
#else
		u32 B2B_ipg:7;		
		u32 undefined1:1;		
		u32 min_ifg_enforce:8;	
		u32 non_B2B_ipg_2:7;	
		u32 undefined2:1;		
		u32 non_B2B_ipg_1:7;	
		u32 reserved:1;		
#endif
	} bits;
} MAC_IPG_t, *PMAC_IPG_t;


typedef union _MAC_HFDP_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:8;		
		u32 alt_beb_trunc:4;	
		u32 alt_beb_enable:1;	
		u32 bp_no_backoff:1;	
		u32 no_backoff:1;		
		u32 excess_defer:1;	
		u32 rexmit_max:4;		
		u32 reserved1:2;		
		u32 coll_window:10;	
#else
		u32 coll_window:10;	
		u32 reserved1:2;		
		u32 rexmit_max:4;		
		u32 excess_defer:1;	
		u32 no_backoff:1;		
		u32 bp_no_backoff:1;	
		u32 alt_beb_enable:1;	
		u32 alt_beb_trunc:4;	
		u32 reserved2:8;		
#endif
	} bits;
} MAC_HFDP_t, *PMAC_HFDP_t;


typedef union _MAC_MAX_FM_LEN_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:16;	
		u32 max_len:16;	
#else
		u32 max_len:16;	
		u32 reserved:16;	
#endif
	} bits;
} MAC_MAX_FM_LEN_t, *PMAC_MAX_FM_LEN_t;




typedef union _MAC_TEST_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:29;	
		u32 mac_test:3;	
#else
		u32 mac_test:3;	
		u32 unused:29;	
#endif
	} bits;
} MAC_TEST_t, *PMAC_TEST_t;


typedef union _MII_MGMT_CFG_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reset_mii_mgmt:1;	
		u32 reserved:25;		
		u32 scan_auto_incremt:1;	
		u32 preamble_suppress:1;	
		u32 undefined:1;		
		u32 mgmt_clk_reset:3;	
#else
		u32 mgmt_clk_reset:3;	
		u32 undefined:1;		
		u32 preamble_suppress:1;	
		u32 scan_auto_incremt:1;	
		u32 reserved:25;		
		u32 reset_mii_mgmt:1;	
#endif
	} bits;
} MII_MGMT_CFG_t, *PMII_MGMT_CFG_t;


typedef union _MII_MGMT_CMD_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:30;	
		u32 scan_cycle:1;	
		u32 read_cycle:1;	
#else
		u32 read_cycle:1;	
		u32 scan_cycle:1;	
		u32 reserved:30;	
#endif
	} bits;
} MII_MGMT_CMD_t, *PMII_MGMT_CMD_t;


typedef union _MII_MGMT_ADDR_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved2:19;	
		u32 phy_addr:5;	
		u32 reserved1:3;	
		u32 reg_addr:5;	
#else
		u32 reg_addr:5;	
		u32 reserved1:3;	
		u32 phy_addr:5;	
		u32 reserved2:19;	
#endif
	} bits;
} MII_MGMT_ADDR_t, *PMII_MGMT_ADDR_t;


typedef union _MII_MGMT_CTRL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:16;	
		u32 phy_ctrl:16;	
#else
		u32 phy_ctrl:16;	
		u32 reserved:16;	
#endif
	} bits;
} MII_MGMT_CTRL_t, *PMII_MGMT_CTRL_t;


typedef union _MII_MGMT_STAT_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:16;	
		u32 phy_stat:16;	
#else
		u32 phy_stat:16;	
		u32 reserved:16;	
#endif
	} bits;
} MII_MGMT_STAT_t, *PMII_MGMT_STAT_t;


typedef union _MII_MGMT_INDICATOR_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:29;	
		u32 not_valid:1;	
		u32 scanning:1;	
		u32 busy:1;	
#else
		u32 busy:1;	
		u32 scanning:1;	
		u32 not_valid:1;	
		u32 reserved:29;	
#endif
	} bits;
} MII_MGMT_INDICATOR_t, *PMII_MGMT_INDICATOR_t;


typedef union _MAC_IF_CTRL_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reset_if_module:1;	
		u32 reserved4:3;		
		u32 tbi_mode:1;		
		u32 ghd_mode:1;		
		u32 lhd_mode:1;		
		u32 phy_mode:1;		
		u32 reset_per_mii:1;	
		u32 reserved3:6;		
		u32 speed:1;		
		u32 reset_pe100x:1;	
		u32 reserved2:4;		
		u32 force_quiet:1;		
		u32 no_cipher:1;		
		u32 disable_link_fail:1;	
		u32 reset_gpsi:1;		
		u32 reserved1:6;		
		u32 enab_jab_protect:1;	
#else
		u32 enab_jab_protect:1;	
		u32 reserved1:6;		
		u32 reset_gpsi:1;		
		u32 disable_link_fail:1;	
		u32 no_cipher:1;		
		u32 force_quiet:1;		
		u32 reserved2:4;		
		u32 reset_pe100x:1;	
		u32 speed:1;		
		u32 reserved3:6;		
		u32 reset_per_mii:1;	
		u32 phy_mode:1;		
		u32 lhd_mode:1;		
		u32 ghd_mode:1;		
		u32 tbi_mode:1;		
		u32 reserved4:3;		
		u32 reset_if_module:1;	
#endif
	} bits;
} MAC_IF_CTRL_t, *PMAC_IF_CTRL_t;


typedef union _MAC_IF_STAT_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:22;		
		u32 excess_defer:1;	
		u32 clash:1;		
		u32 phy_jabber:1;		
		u32 phy_link_ok:1;		
		u32 phy_full_duplex:1;	
		u32 phy_speed:1;		
		u32 pe100x_link_fail:1;	
		u32 pe10t_loss_carrie:1;	
		u32 pe10t_sqe_error:1;	
		u32 pe10t_jabber:1;	
#else
		u32 pe10t_jabber:1;	
		u32 pe10t_sqe_error:1;	
		u32 pe10t_loss_carrie:1;	
		u32 pe100x_link_fail:1;	
		u32 phy_speed:1;		
		u32 phy_full_duplex:1;	
		u32 phy_link_ok:1;		
		u32 phy_jabber:1;		
		u32 clash:1;		
		u32 excess_defer:1;	
		u32 reserved:22;		
#endif
	} bits;
} MAC_IF_STAT_t, *PMAC_IF_STAT_t;


typedef union _MAC_STATION_ADDR1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 Octet6:8;	
		u32 Octet5:8;	
		u32 Octet4:8;	
		u32 Octet3:8;	
#else
		u32 Octet3:8;	
		u32 Octet4:8;	
		u32 Octet5:8;	
		u32 Octet6:8;	
#endif
	} bits;
} MAC_STATION_ADDR1_t, *PMAC_STATION_ADDR1_t;


typedef union _MAC_STATION_ADDR2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 Octet2:8;	
		u32 Octet1:8;	
		u32 reserved:16;	
#else
		u32 reserved:16;	
		u32 Octet1:8;	
		u32 Octet2:8;	
#endif
	} bits;
} MAC_STATION_ADDR2_t, *PMAC_STATION_ADDR2_t;


typedef struct _MAC_t {					
	MAC_CFG1_t cfg1;				
	MAC_CFG2_t cfg2;				
	MAC_IPG_t ipg;					
	MAC_HFDP_t hfdp;				
	MAC_MAX_FM_LEN_t max_fm_len;			
	u32 rsv1;					
	u32 rsv2;					
	MAC_TEST_t mac_test;				
	MII_MGMT_CFG_t mii_mgmt_cfg;			
	MII_MGMT_CMD_t mii_mgmt_cmd;			
	MII_MGMT_ADDR_t mii_mgmt_addr;			
	MII_MGMT_CTRL_t mii_mgmt_ctrl;			
	MII_MGMT_STAT_t mii_mgmt_stat;			
	MII_MGMT_INDICATOR_t mii_mgmt_indicator;	
	MAC_IF_CTRL_t if_ctrl;				
	MAC_IF_STAT_t if_stat;				
	MAC_STATION_ADDR1_t station_addr_1;		
	MAC_STATION_ADDR2_t station_addr_2;		
} MAC_t, *PMAC_t;






typedef union _MAC_STAT_REG_1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 tr64:1;	
		u32 tr127:1;	
		u32 tr255:1;	
		u32 tr511:1;	
		u32 tr1k:1;	
		u32 trmax:1;	
		u32 trmgv:1;	
		u32 unused:8;	
		u32 rbyt:1;	
		u32 rpkt:1;	
		u32 rfcs:1;	
		u32 rmca:1;	
		u32 rbca:1;	
		u32 rxcf:1;	
		u32 rxpf:1;	
		u32 rxuo:1;	
		u32 raln:1;	
		u32 rflr:1;	
		u32 rcde:1;	
		u32 rcse:1;	
		u32 rund:1;	
		u32 rovr:1;	
		u32 rfrg:1;	
		u32 rjbr:1;	
		u32 rdrp:1;	
#else
		u32 rdrp:1;	
		u32 rjbr:1;	
		u32 rfrg:1;	
		u32 rovr:1;	
		u32 rund:1;	
		u32 rcse:1;	
		u32 rcde:1;	
		u32 rflr:1;	
		u32 raln:1;	
		u32 rxuo:1;	
		u32 rxpf:1;	
		u32 rxcf:1;	
		u32 rbca:1;	
		u32 rmca:1;	
		u32 rfcs:1;	
		u32 rpkt:1;	
		u32 rbyt:1;	
		u32 unused:8;	
		u32 trmgv:1;	
		u32 trmax:1;	
		u32 tr1k:1;	
		u32 tr511:1;	
		u32 tr255:1;	
		u32 tr127:1;	
		u32 tr64:1;	
#endif
	} bits;
} MAC_STAT_REG_1_t, *PMAC_STAT_REG_1_t;


typedef union _MAC_STAT_REG_2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:12;	
		u32 tjbr:1;	
		u32 tfcs:1;	
		u32 txcf:1;	
		u32 tovr:1;	
		u32 tund:1;	
		u32 tfrg:1;	
		u32 tbyt:1;	
		u32 tpkt:1;	
		u32 tmca:1;	
		u32 tbca:1;	
		u32 txpf:1;	
		u32 tdfr:1;	
		u32 tedf:1;	
		u32 tscl:1;	
		u32 tmcl:1;	
		u32 tlcl:1;	
		u32 txcl:1;	
		u32 tncl:1;	
		u32 tpfh:1;	
		u32 tdrp:1;	
#else
		u32 tdrp:1;	
		u32 tpfh:1;	
		u32 tncl:1;	
		u32 txcl:1;	
		u32 tlcl:1;	
		u32 tmcl:1;	
		u32 tscl:1;	
		u32 tedf:1;	
		u32 tdfr:1;	
		u32 txpf:1;	
		u32 tbca:1;	
		u32 tmca:1;	
		u32 tpkt:1;	
		u32 tbyt:1;	
		u32 tfrg:1;	
		u32 tund:1;	
		u32 tovr:1;	
		u32 txcf:1;	
		u32 tfcs:1;	
		u32 tjbr:1;	
		u32 unused:12;	
#endif
	} bits;
} MAC_STAT_REG_2_t, *PMAC_STAT_REG_2_t;


typedef struct _MAC_STAT_t {		
	u32 pad[32];		

	
	u32 TR64;			

	
	u32 TR127;			

	
	u32 TR255;			

	
	u32 TR511;			

	
	u32 TR1K;			

	
	u32 TRMax;			

	
	u32 TRMgv;			

	
	u32 RByt;			

	
	u32 RPkt;			

	
	u32 RFcs;			

	
	u32 RMca;			

	
	u32 RBca;			

	
	u32 RxCf;			

	
	u32 RxPf;			

	
	u32 RxUo;			

	
	u32 RAln;			

	
	u32 RFlr;			

	
	u32 RCde;			

	
	u32 RCse;			

	
	u32 RUnd;			

	
	u32 ROvr;			

	
	u32 RFrg;			

	
	u32 RJbr;			

	
	u32 RDrp;			

	
	u32 TByt;			

	
	u32 TPkt;			

	
	u32 TMca;			

	
	u32 TBca;			

	
	u32 TxPf;			

	
	u32 TDfr;			

	
	u32 TEdf;			

	
	u32 TScl;			

	
	u32 TMcl;			

	
	u32 TLcl;			

	
	u32 TXcl;			

	
	u32 TNcl;			

	
	u32 TPfh;			

	
	u32 TDrp;			

	
	u32 TJbr;			

	
	u32 TFcs;			

	
	u32 TxCf;			

	
	u32 TOvr;			

	
	u32 TUnd;			

	
	u32 TFrg;			

	
	MAC_STAT_REG_1_t Carry1;	

	
	MAC_STAT_REG_2_t Carry2;	

	
	MAC_STAT_REG_1_t Carry1M;	

	
	MAC_STAT_REG_2_t Carry2M;	
} MAC_STAT_t, *PMAC_STAT_t;








#define ET_MMC_ENABLE		1
#define ET_MMC_ARB_DISABLE	2
#define ET_MMC_RXMAC_DISABLE	4
#define ET_MMC_TXMAC_DISABLE	8
#define ET_MMC_TXDMA_DISABLE	16
#define ET_MMC_RXDMA_DISABLE	32
#define ET_MMC_FORCE_CE		64



#define ET_SRAM_REQ_ACCESS	1
#define ET_SRAM_WR_ACCESS	2
#define ET_SRAM_IS_CTRL		4




typedef struct _MMC_t {			
	u32 mmc_ctrl;		
	u32 sram_access;	
	u32 sram_word1;		
	u32 sram_word2;		
	u32 sram_word3;		
	u32 sram_word4;		
} MMC_t, *PMMC_t;









#if 0
typedef struct _EXP_ROM_t {

} EXP_ROM_t, *PEXP_ROM_t;
#endif





typedef struct _ADDRESS_MAP_t {
	GLOBAL_t global;
	
	u8 unused_global[4096 - sizeof(GLOBAL_t)];
	TXDMA_t txdma;
	
	u8 unused_txdma[4096 - sizeof(TXDMA_t)];
	RXDMA_t rxdma;
	
	u8 unused_rxdma[4096 - sizeof(RXDMA_t)];
	TXMAC_t txmac;
	
	u8 unused_txmac[4096 - sizeof(TXMAC_t)];
	RXMAC_t rxmac;
	
	u8 unused_rxmac[4096 - sizeof(RXMAC_t)];
	MAC_t mac;
	
	u8 unused_mac[4096 - sizeof(MAC_t)];
	MAC_STAT_t macStat;
	
	u8 unused_mac_stat[4096 - sizeof(MAC_STAT_t)];
	MMC_t mmc;
	
	u8 unused_mmc[4096 - sizeof(MMC_t)];
	
	u8 unused_[1015808];


#if 0
	EXP_ROM_t exp_rom;
#endif

	u8 unused_exp_rom[4096];	
	u8 unused__[524288];	
} ADDRESS_MAP_t, *PADDRESS_MAP_t;

#endif 
