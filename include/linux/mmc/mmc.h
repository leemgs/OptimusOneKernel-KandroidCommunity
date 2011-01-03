

#ifndef MMC_MMC_H
#define MMC_MMC_H


   
#define MMC_GO_IDLE_STATE         0   
#define MMC_SEND_OP_COND          1   
#define MMC_ALL_SEND_CID          2   
#define MMC_SET_RELATIVE_ADDR     3   
#define MMC_SET_DSR               4   
#define MMC_SLEEP_AWAKE		  5   
#define MMC_SWITCH                6   
#define MMC_SELECT_CARD           7   
#define MMC_SEND_EXT_CSD          8   
#define MMC_SEND_CSD              9   
#define MMC_SEND_CID             10   
#define MMC_READ_DAT_UNTIL_STOP  11   
#define MMC_STOP_TRANSMISSION    12   
#define MMC_SEND_STATUS          13   
#define MMC_BUSTEST_R            14   
#define MMC_GO_INACTIVE_STATE    15   
#define MMC_BUSTEST_W            19   
#define MMC_SPI_READ_OCR         58   
#define MMC_SPI_CRC_ON_OFF       59   

  
#define MMC_SET_BLOCKLEN         16   
#define MMC_READ_SINGLE_BLOCK    17   
#define MMC_READ_MULTIPLE_BLOCK  18   

  
#define MMC_WRITE_DAT_UNTIL_STOP 20   

  
#define MMC_SET_BLOCK_COUNT      23   
#define MMC_WRITE_BLOCK          24   
#define MMC_WRITE_MULTIPLE_BLOCK 25   
#define MMC_PROGRAM_CID          26   
#define MMC_PROGRAM_CSD          27   

  
#define MMC_SET_WRITE_PROT       28   
#define MMC_CLR_WRITE_PROT       29   
#define MMC_SEND_WRITE_PROT      30   

  
#define MMC_ERASE_GROUP_START    35   
#define MMC_ERASE_GROUP_END      36   
#define MMC_ERASE                38   

  
#define MMC_FAST_IO              39   
#define MMC_GO_IRQ_STATE         40   

  
#define MMC_LOCK_UNLOCK          42   

  
#define MMC_APP_CMD              55   
#define MMC_GEN_CMD              56   





#define R1_OUT_OF_RANGE		(1 << 31)	
#define R1_ADDRESS_ERROR	(1 << 30)	
#define R1_BLOCK_LEN_ERROR	(1 << 29)	
#define R1_ERASE_SEQ_ERROR      (1 << 28)	
#define R1_ERASE_PARAM		(1 << 27)	
#define R1_WP_VIOLATION		(1 << 26)	
#define R1_CARD_IS_LOCKED	(1 << 25)	
#define R1_LOCK_UNLOCK_FAILED	(1 << 24)	
#define R1_COM_CRC_ERROR	(1 << 23)	
#define R1_ILLEGAL_COMMAND	(1 << 22)	
#define R1_CARD_ECC_FAILED	(1 << 21)	
#define R1_CC_ERROR		(1 << 20)	
#define R1_ERROR		(1 << 19)	
#define R1_UNDERRUN		(1 << 18)	
#define R1_OVERRUN		(1 << 17)	
#define R1_CID_CSD_OVERWRITE	(1 << 16)	
#define R1_WP_ERASE_SKIP	(1 << 15)	
#define R1_CARD_ECC_DISABLED	(1 << 14)	
#define R1_ERASE_RESET		(1 << 13)	
#define R1_STATUS(x)            (x & 0xFFFFE000)
#define R1_CURRENT_STATE(x)	((x & 0x00001E00) >> 9)	
#define R1_READY_FOR_DATA	(1 << 8)	
#define R1_SWITCH_ERROR		(1 << 7)	
#define R1_APP_CMD		(1 << 5)	


#define R1_SPI_IDLE		(1 << 0)
#define R1_SPI_ERASE_RESET	(1 << 1)
#define R1_SPI_ILLEGAL_COMMAND	(1 << 2)
#define R1_SPI_COM_CRC		(1 << 3)
#define R1_SPI_ERASE_SEQ	(1 << 4)
#define R1_SPI_ADDRESS		(1 << 5)
#define R1_SPI_PARAMETER	(1 << 6)

#define R2_SPI_CARD_LOCKED	(1 << 8)
#define R2_SPI_WP_ERASE_SKIP	(1 << 9)	
#define R2_SPI_LOCK_UNLOCK_FAIL	R2_SPI_WP_ERASE_SKIP
#define R2_SPI_ERROR		(1 << 10)
#define R2_SPI_CC_ERROR		(1 << 11)
#define R2_SPI_CARD_ECC_ERROR	(1 << 12)
#define R2_SPI_WP_VIOLATION	(1 << 13)
#define R2_SPI_ERASE_PARAM	(1 << 14)
#define R2_SPI_OUT_OF_RANGE	(1 << 15)	
#define R2_SPI_CSD_OVERWRITE	R2_SPI_OUT_OF_RANGE



struct _mmc_csd {
	u8  csd_structure;
	u8  spec_vers;
	u8  taac;
	u8  nsac;
	u8  tran_speed;
	u16 ccc;
	u8  read_bl_len;
	u8  read_bl_partial;
	u8  write_blk_misalign;
	u8  read_blk_misalign;
	u8  dsr_imp;
	u16 c_size;
	u8  vdd_r_curr_min;
	u8  vdd_r_curr_max;
	u8  vdd_w_curr_min;
	u8  vdd_w_curr_max;
	u8  c_size_mult;
	union {
		struct { 
			u8  erase_grp_size;
			u8  erase_grp_mult;
		} v31;
		struct { 
			u8  sector_size;
			u8  erase_grp_size;
		} v22;
	} erase;
	u8  wp_grp_size;
	u8  wp_grp_enable;
	u8  default_ecc;
	u8  r2w_factor;
	u8  write_bl_len;
	u8  write_bl_partial;
	u8  file_format_grp;
	u8  copy;
	u8  perm_write_protect;
	u8  tmp_write_protect;
	u8  file_format;
	u8  ecc;
};


#define MMC_CARD_BUSY	0x80000000	


#define CCC_BASIC		(1<<0)	
					
					
#define CCC_STREAM_READ		(1<<1)	
					
#define CCC_BLOCK_READ		(1<<2)	
					
#define CCC_STREAM_WRITE	(1<<3)	
					
#define CCC_BLOCK_WRITE		(1<<4)	
					
#define CCC_ERASE		(1<<5)	
					
#define CCC_WRITE_PROT		(1<<6)	
					
#define CCC_LOCK_CARD		(1<<7)	
					
#define CCC_APP_SPEC		(1<<8)	
					
#define CCC_IO_MODE		(1<<9)	
					
#define CCC_SWITCH		(1<<10)	
					
					
					



#define CSD_STRUCT_VER_1_0  0           
#define CSD_STRUCT_VER_1_1  1           
#define CSD_STRUCT_VER_1_2  2           
#define CSD_STRUCT_EXT_CSD  3           

#define CSD_SPEC_VER_0      0           
#define CSD_SPEC_VER_1      1           
#define CSD_SPEC_VER_2      2           
#define CSD_SPEC_VER_3      3           
#define CSD_SPEC_VER_4      4           



#define EXT_CSD_BUS_WIDTH	183	
#define EXT_CSD_HS_TIMING	185	
#define EXT_CSD_CARD_TYPE	196	
#define EXT_CSD_REV		192	
#define EXT_CSD_SEC_CNT		212	
#define EXT_CSD_S_A_TIMEOUT	217



#define EXT_CSD_CMD_SET_NORMAL		(1<<0)
#define EXT_CSD_CMD_SET_SECURE		(1<<1)
#define EXT_CSD_CMD_SET_CPSECURE	(1<<2)

#define EXT_CSD_CARD_TYPE_26	(1<<0)	
#define EXT_CSD_CARD_TYPE_52	(1<<1)	

#define EXT_CSD_BUS_WIDTH_1	0	
#define EXT_CSD_BUS_WIDTH_4	1	
#define EXT_CSD_BUS_WIDTH_8	2	



#define MMC_SWITCH_MODE_CMD_SET		0x00	
#define MMC_SWITCH_MODE_SET_BITS	0x01	
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02	
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	

#endif  

