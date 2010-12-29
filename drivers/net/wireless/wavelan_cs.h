



#ifndef _WAVELAN_CS_H
#define	_WAVELAN_CS_H




static const unsigned char	MAC_ADDRESSES[][3] =
{
  { 0x08, 0x00, 0x0E },		
  { 0x08, 0x00, 0x6A },		
  { 0x00, 0x00, 0xE1 },		
  { 0x00, 0x60, 0x1D }		
  
};




static const short	channel_bands[] = { 0x30, 0x58, 0x64, 0x7A, 0x80, 0xA8,
				    0xD0, 0xF0, 0xF8, 0x150 };


static const int	fixed_bands[] = { 915e6, 2.425e8, 2.46e8, 2.484e8, 2.4305e8 };






#define	LCCR(base)	(base)		
#define	LCSR(base)	(base)		
#define	HACR(base)	(base+0x1)	
#define	HASR(base)	(base+0x1)	
#define PIORL(base)	(base+0x2)	
#define RPLL(base)	(base+0x2)	
#define PIORH(base)	(base+0x3)	
#define RPLH(base)	(base+0x3)	
#define PIOP(base)	(base+0x4)	
#define MMR(base)	(base+0x6)	
#define MMD(base)	(base+0x7)	



#define HACR_LOF	  (1 << 3)	
#define HACR_PWR_STAT	  (1 << 4)	
#define HACR_TX_DMA_RESET (1 << 5)	
#define HACR_RX_DMA_RESET (1 << 6)	
#define HACR_ROM_WEN	  (1 << 7)	

#define HACR_RESET              (HACR_TX_DMA_RESET | HACR_RX_DMA_RESET)
#define	HACR_DEFAULT		(HACR_PWR_STAT)



#define HASR_MMI_BUSY	(1 << 2)	
#define HASR_LOF	(1 << 3)	
#define HASR_NO_CLK	(1 << 4)	



#define PIORH_SEL_TX	(1 << 5)	
#define MMR_MMI_WR	(1 << 0)	
#define PIORH_MASK	0x1f		
#define RPLH_MASK	0x1f		
#define MMI_ADDR_MASK	0x7e		



#define CIS_ADDR	0x0000		
#define PSA_ADDR	0x0e00		
#define EEPROM_ADDR	0x1000		
#define COR_ADDR	0x4000		



#define COR_CONFIG	(1 << 0)	
#define COR_SW_RESET	(1 << 7)	
#define COR_LEVEL_IRQ	(1 << 6)	



#define RX_BASE		0x0000		
#define TX_BASE		0x2000		
#define UNUSED_BASE	0x2800		
#define RX_SIZE		(TX_BASE-RX_BASE)	
#define RX_SIZE_SHIFT	6		

#define TRUE  1
#define FALSE 0

#define MOD_ENAL 1
#define MOD_PROM 2


#define WAVELAN_ADDR_SIZE	6


#define WAVELAN_MTU	1500

#define	MAXDATAZ		(6 + 6 + 2 + WAVELAN_MTU)




typedef struct psa_t	psa_t;
struct psa_t
{
  
  unsigned char	psa_io_base_addr_1;	
  unsigned char	psa_io_base_addr_2;	
  unsigned char	psa_io_base_addr_3;	
  unsigned char	psa_io_base_addr_4;	
  unsigned char	psa_rem_boot_addr_1;	
  unsigned char	psa_rem_boot_addr_2;	
  unsigned char	psa_rem_boot_addr_3;	
  unsigned char	psa_holi_params;	
  unsigned char	psa_int_req_no;		
  unsigned char	psa_unused0[7];		

  unsigned char	psa_univ_mac_addr[WAVELAN_ADDR_SIZE];	
  unsigned char	psa_local_mac_addr[WAVELAN_ADDR_SIZE];	
  unsigned char	psa_univ_local_sel;	
#define		PSA_UNIVERSAL	0		
#define		PSA_LOCAL	1		
  unsigned char	psa_comp_number;	
#define		PSA_COMP_PC_AT_915	0 	
#define		PSA_COMP_PC_MC_915	1 	
#define		PSA_COMP_PC_AT_2400	2 	
#define		PSA_COMP_PC_MC_2400	3 	
#define		PSA_COMP_PCMCIA_915	4 	
  unsigned char	psa_thr_pre_set;	
  unsigned char	psa_feature_select;	
#define		PSA_FEATURE_CALL_CODE	0x01 	
  unsigned char	psa_subband;		
#define		PSA_SUBBAND_915		0	
#define		PSA_SUBBAND_2425	1	
#define		PSA_SUBBAND_2460	2	
#define		PSA_SUBBAND_2484	3	
#define		PSA_SUBBAND_2430_5	4	
  unsigned char	psa_quality_thr;	
  unsigned char	psa_mod_delay;		
  unsigned char	psa_nwid[2];		
  unsigned char	psa_nwid_select;	
  unsigned char	psa_encryption_select;	
  unsigned char	psa_encryption_key[8];	
  unsigned char	psa_databus_width;	
  unsigned char	psa_call_code[8];	
  unsigned char	psa_nwid_prefix[2];	
  unsigned char	psa_reserved[2];	
  unsigned char	psa_conf_status;	
  unsigned char	psa_crc[2];		
  unsigned char	psa_crc_status;		
};


#define	PSA_SIZE	64


#define	psaoff(p,f) 	((unsigned short) ((void *)(&((psa_t *) ((void *) NULL + (p)))->f) - (void *) NULL))




typedef struct mmw_t	mmw_t;
struct mmw_t
{
  unsigned char	mmw_encr_key[8];	
  unsigned char	mmw_encr_enable;	
#define	MMW_ENCR_ENABLE_MODE	0x02	
#define	MMW_ENCR_ENABLE_EN	0x01	
  unsigned char	mmw_unused0[1];		
  unsigned char	mmw_des_io_invert;	
#define	MMW_DES_IO_INVERT_RES	0x0F	
#define	MMW_DES_IO_INVERT_CTRL	0xF0	
  unsigned char	mmw_unused1[5];		
  unsigned char	mmw_loopt_sel;		
#define	MMW_LOOPT_SEL_DIS_NWID	0x40	
#define	MMW_LOOPT_SEL_INT	0x20	
#define	MMW_LOOPT_SEL_LS	0x10	
#define MMW_LOOPT_SEL_LT3A	0x08	
#define	MMW_LOOPT_SEL_LT3B	0x04	
#define	MMW_LOOPT_SEL_LT3C	0x02	
#define	MMW_LOOPT_SEL_LT3D	0x01	
  unsigned char	mmw_jabber_enable;	
  
  unsigned char	mmw_freeze;		
  
  unsigned char	mmw_anten_sel;		
#define MMW_ANTEN_SEL_SEL	0x01	
#define	MMW_ANTEN_SEL_ALG_EN	0x02	
  unsigned char	mmw_ifs;		
  
  unsigned char	mmw_mod_delay;	 	
  unsigned char	mmw_jam_time;		
  unsigned char	mmw_unused2[1];		
  unsigned char	mmw_thr_pre_set;	
  
  unsigned char	mmw_decay_prm;		
  unsigned char	mmw_decay_updat_prm;	
  unsigned char	mmw_quality_thr;	
  
  unsigned char	mmw_netw_id_l;		
  unsigned char	mmw_netw_id_h;		
  

  
  unsigned char	mmw_mode_select;	
  unsigned char	mmw_unused3[1];		
  unsigned char	mmw_fee_ctrl;		
#define	MMW_FEE_CTRL_PRE	0x10	
#define	MMW_FEE_CTRL_DWLD	0x08	
#define	MMW_FEE_CTRL_CMD	0x07	
#define	MMW_FEE_CTRL_READ	0x06	
#define	MMW_FEE_CTRL_WREN	0x04	
#define	MMW_FEE_CTRL_WRITE	0x05	
#define	MMW_FEE_CTRL_WRALL	0x04	
#define	MMW_FEE_CTRL_WDS	0x04	
#define	MMW_FEE_CTRL_PRREAD	0x16	
#define	MMW_FEE_CTRL_PREN	0x14	
#define	MMW_FEE_CTRL_PRCLEAR	0x17	
#define	MMW_FEE_CTRL_PRWRITE	0x15	
#define	MMW_FEE_CTRL_PRDS	0x14	
  

  unsigned char	mmw_fee_addr;		
#define	MMW_FEE_ADDR_CHANNEL	0xF0	
#define	MMW_FEE_ADDR_OFFSET	0x0F	
#define	MMW_FEE_ADDR_EN		0xC0	
#define	MMW_FEE_ADDR_DS		0x00	
#define	MMW_FEE_ADDR_ALL	0x40	
#define	MMW_FEE_ADDR_CLEAR	0xFF	

  unsigned char	mmw_fee_data_l;		
  unsigned char	mmw_fee_data_h;		
  unsigned char	mmw_ext_ant;		
#define	MMW_EXT_ANT_EXTANT	0x01	
#define	MMW_EXT_ANT_POL		0x02	
#define	MMW_EXT_ANT_INTERNAL	0x00	
#define	MMW_EXT_ANT_EXTERNAL	0x03	
#define	MMW_EXT_ANT_IQ_TEST	0x1C	
} __attribute__((packed));


#define	MMW_SIZE	37


#define	mmwoff(p,f) 	(unsigned short)((void *)(&((mmw_t *)((void *)0 + (p)))->f) - (void *)0)



typedef struct mmr_t	mmr_t;
struct mmr_t
{
  unsigned char	mmr_unused0[8];		
  unsigned char	mmr_des_status;		
  unsigned char	mmr_des_avail;		
#define	MMR_DES_AVAIL_DES	0x55		
#define	MMR_DES_AVAIL_AES	0x33		
  unsigned char	mmr_des_io_invert;	
  unsigned char	mmr_unused1[5];		
  unsigned char	mmr_dce_status;		
#define	MMR_DCE_STATUS_RX_BUSY		0x01	
#define	MMR_DCE_STATUS_LOOPT_IND	0x02	
#define	MMR_DCE_STATUS_TX_BUSY		0x04	
#define	MMR_DCE_STATUS_JBR_EXPIRED	0x08	
#define MMR_DCE_STATUS			0x0F	
  unsigned char	mmr_dsp_id;		
  unsigned char	mmr_unused2[2];		
  unsigned char	mmr_correct_nwid_l;	
  unsigned char	mmr_correct_nwid_h;	
  
  unsigned char	mmr_wrong_nwid_l;	
  unsigned char	mmr_wrong_nwid_h;	
  unsigned char	mmr_thr_pre_set;	
#define	MMR_THR_PRE_SET		0x3F		
#define	MMR_THR_PRE_SET_CUR	0x80		
  unsigned char	mmr_signal_lvl;		
#define	MMR_SIGNAL_LVL		0x3F		
#define	MMR_SIGNAL_LVL_VALID	0x80		
  unsigned char	mmr_silence_lvl;	
#define	MMR_SILENCE_LVL		0x3F		
#define	MMR_SILENCE_LVL_VALID	0x80		
  unsigned char	mmr_sgnl_qual;		
#define	MMR_SGNL_QUAL		0x0F		
#define	MMR_SGNL_QUAL_ANT	0x80		
  unsigned char	mmr_netw_id_l;		
  unsigned char	mmr_unused3[3];		

  
  unsigned char	mmr_fee_status;		
#define	MMR_FEE_STATUS_ID	0xF0		
#define	MMR_FEE_STATUS_DWLD	0x08		
#define	MMR_FEE_STATUS_BUSY	0x04		
  unsigned char	mmr_unused4[1];		
  unsigned char	mmr_fee_data_l;		
  unsigned char	mmr_fee_data_h;		
};


#define	MMR_SIZE	36


#define	mmroff(p,f) 	(unsigned short)((void *)(&((mmr_t *)((void *)0 + (p)))->f) - (void *)0)



typedef union mm_t
{
  struct mmw_t	w;	
  struct mmr_t	r;	
} mm_t;

#endif 
