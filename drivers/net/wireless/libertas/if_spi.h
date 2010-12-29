

#ifndef _LBS_IF_SPI_H_
#define _LBS_IF_SPI_H_

#define IPFIELD_ALIGN_OFFSET 2
#define IF_SPI_CMD_BUF_SIZE 2400



#define IF_SPI_FW_NAME_MAX 30

struct chip_ident {
	u16 chip_id;
	u16 name;
};

#define MAX_MAIN_FW_LOAD_CRC_ERR 10


#define HELPER_FW_LOAD_CHUNK_SZ 64


#define FIRMWARE_DNLD_OK 0x0000


#define SUCCESSFUL_FW_DOWNLOAD_MAGIC 0x88888888



#define IF_SPI_READ_OPERATION_MASK 0x0
#define IF_SPI_WRITE_OPERATION_MASK 0x8000


#define IF_SPI_DEVICEID_CTRL_REG 0x00	
#define IF_SPI_IO_READBASE_REG 0x04 	
#define IF_SPI_IO_WRITEBASE_REG 0x08	
#define IF_SPI_IO_RDWRPORT_REG 0x0C	

#define IF_SPI_CMD_READBASE_REG 0x10	
#define IF_SPI_CMD_WRITEBASE_REG 0x14	
#define IF_SPI_CMD_RDWRPORT_REG 0x18	

#define IF_SPI_DATA_READBASE_REG 0x1C	
#define IF_SPI_DATA_WRITEBASE_REG 0x20	
#define IF_SPI_DATA_RDWRPORT_REG 0x24	

#define IF_SPI_SCRATCH_1_REG 0x28	
#define IF_SPI_SCRATCH_2_REG 0x2C	
#define IF_SPI_SCRATCH_3_REG 0x30	
#define IF_SPI_SCRATCH_4_REG 0x34	

#define IF_SPI_TX_FRAME_SEQ_NUM_REG 0x38 
#define IF_SPI_TX_FRAME_STATUS_REG 0x3C	

#define IF_SPI_HOST_INT_CTRL_REG 0x40	

#define IF_SPI_CARD_INT_CAUSE_REG 0x44	
#define IF_SPI_CARD_INT_STATUS_REG 0x48 
#define IF_SPI_CARD_INT_EVENT_MASK_REG 0x4C 
#define IF_SPI_CARD_INT_STATUS_MASK_REG	0x50 

#define IF_SPI_CARD_INT_RESET_SELECT_REG 0x54 

#define IF_SPI_HOST_INT_CAUSE_REG 0x58	
#define IF_SPI_HOST_INT_STATUS_REG 0x5C	
#define IF_SPI_HOST_INT_EVENT_MASK_REG 0x60 
#define IF_SPI_HOST_INT_STATUS_MASK_REG	0x64 
#define IF_SPI_HOST_INT_RESET_SELECT_REG 0x68 

#define IF_SPI_DELAY_READ_REG 0x6C	
#define IF_SPI_SPU_BUS_MODE_REG 0x70	


#define IF_SPI_DEVICEID_CTRL_REG_TO_CARD_ID(dc) ((dc & 0xffff0000)>>16)
#define IF_SPI_DEVICEID_CTRL_REG_TO_CARD_REV(dc) (dc & 0x000000ff)



#define IF_SPI_HICT_WAKE_UP				(1<<0)

#define IF_SPI_HICT_WLAN_READY				(1<<1)




#define IF_SPI_HICT_TX_DOWNLOAD_OVER_AUTO		(1<<5)

#define IF_SPI_HICT_RX_UPLOAD_OVER_AUTO			(1<<6)

#define IF_SPI_HICT_CMD_DOWNLOAD_OVER_AUTO		(1<<7)

#define IF_SPI_HICT_CMD_UPLOAD_OVER_AUTO		(1<<8)



#define IF_SPI_CIC_TX_DOWNLOAD_OVER			(1<<0)

#define IF_SPI_CIC_RX_UPLOAD_OVER			(1<<1)

#define IF_SPI_CIC_CMD_DOWNLOAD_OVER			(1<<2)

#define IF_SPI_CIC_HOST_EVENT				(1<<3)

#define IF_SPI_CIC_CMD_UPLOAD_OVER			(1<<4)

#define IF_SPI_CIC_POWER_DOWN				(1<<5)


#define IF_SPI_CIS_TX_DOWNLOAD_OVER			(1<<0)
#define IF_SPI_CIS_RX_UPLOAD_OVER			(1<<1)
#define IF_SPI_CIS_CMD_DOWNLOAD_OVER			(1<<2)
#define IF_SPI_CIS_HOST_EVENT				(1<<3)
#define IF_SPI_CIS_CMD_UPLOAD_OVER			(1<<4)
#define IF_SPI_CIS_POWER_DOWN				(1<<5)


#define IF_SPI_HICU_TX_DOWNLOAD_RDY			(1<<0)
#define IF_SPI_HICU_RX_UPLOAD_RDY			(1<<1)
#define IF_SPI_HICU_CMD_DOWNLOAD_RDY			(1<<2)
#define IF_SPI_HICU_CARD_EVENT				(1<<3)
#define IF_SPI_HICU_CMD_UPLOAD_RDY			(1<<4)
#define IF_SPI_HICU_IO_WR_FIFO_OVERFLOW			(1<<5)
#define IF_SPI_HICU_IO_RD_FIFO_UNDERFLOW		(1<<6)
#define IF_SPI_HICU_DATA_WR_FIFO_OVERFLOW		(1<<7)
#define IF_SPI_HICU_DATA_RD_FIFO_UNDERFLOW		(1<<8)
#define IF_SPI_HICU_CMD_WR_FIFO_OVERFLOW		(1<<9)
#define IF_SPI_HICU_CMD_RD_FIFO_UNDERFLOW		(1<<10)



#define IF_SPI_HIST_TX_DOWNLOAD_RDY			(1<<0)

#define IF_SPI_HIST_RX_UPLOAD_RDY			(1<<1)

#define IF_SPI_HIST_CMD_DOWNLOAD_RDY			(1<<2)

#define IF_SPI_HIST_CARD_EVENT				(1<<3)

#define IF_SPI_HIST_CMD_UPLOAD_RDY			(1<<4)

#define IF_SPI_HIST_IO_WR_FIFO_OVERFLOW			(1<<5)

#define IF_SPI_HIST_IO_RD_FIFO_UNDRFLOW			(1<<6)

#define IF_SPI_HIST_DATA_WR_FIFO_OVERFLOW		(1<<7)

#define IF_SPI_HIST_DATA_RD_FIFO_UNDERFLOW		(1<<8)

#define IF_SPI_HIST_CMD_WR_FIFO_OVERFLOW		(1<<9)

#define IF_SPI_HIST_CMD_RD_FIFO_UNDERFLOW		(1<<10)



#define IF_SPI_HISM_TX_DOWNLOAD_RDY			(1<<0)

#define IF_SPI_HISM_RX_UPLOAD_RDY			(1<<1)

#define IF_SPI_HISM_CMD_DOWNLOAD_RDY			(1<<2)

#define IF_SPI_HISM_CARDEVENT				(1<<3)

#define IF_SPI_HISM_CMD_UPLOAD_RDY			(1<<4)

#define IF_SPI_HISM_IO_WR_FIFO_OVERFLOW			(1<<5)

#define IF_SPI_HISM_IO_RD_FIFO_UNDERFLOW		(1<<6)

#define IF_SPI_HISM_DATA_WR_FIFO_OVERFLOW		(1<<7)

#define IF_SPI_HISM_DATA_RD_FIFO_UNDERFLOW		(1<<8)

#define IF_SPI_HISM_CMD_WR_FIFO_OVERFLOW		(1<<9)

#define IF_SPI_HISM_CMD_RD_FIFO_UNDERFLOW		(1<<10)



#define IF_SPI_BUS_MODE_SPI_CLOCK_PHASE_FALLING 0x8
#define IF_SPI_BUS_MODE_SPI_CLOCK_PHASE_RISING 0x0


#define IF_SPI_BUS_MODE_DELAY_METHOD_DUMMY_CLOCK 0x4
#define IF_SPI_BUS_MODE_DELAY_METHOD_TIMED 0x0


#define IF_SPI_BUS_MODE_8_BIT_ADDRESS_16_BIT_DATA 0x00
#define IF_SPI_BUS_MODE_8_BIT_ADDRESS_32_BIT_DATA 0x01
#define IF_SPI_BUS_MODE_16_BIT_ADDRESS_16_BIT_DATA 0x02
#define IF_SPI_BUS_MODE_16_BIT_ADDRESS_32_BIT_DATA 0x03

#endif
