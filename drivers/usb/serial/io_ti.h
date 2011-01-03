

#ifndef _IO_TI_H_
#define _IO_TI_H_


#define DTK_ADDR_SPACE_XDATA		0x03	
#define DTK_ADDR_SPACE_I2C_TYPE_II	0x82	
#define DTK_ADDR_SPACE_I2C_TYPE_III	0x83	


#define UMPMEM_BASE_UART1		0xFFA0  
#define UMPMEM_BASE_UART2		0xFFB0  
#define UMPMEM_OFFS_UART_LSR		0x05    


#define UMP_UART_CHAR5BITS		0x00
#define UMP_UART_CHAR6BITS		0x01
#define UMP_UART_CHAR7BITS		0x02
#define UMP_UART_CHAR8BITS		0x03


#define UMP_UART_NOPARITY		0x00
#define UMP_UART_ODDPARITY		0x01
#define UMP_UART_EVENPARITY		0x02
#define UMP_UART_MARKPARITY		0x03
#define UMP_UART_SPACEPARITY		0x04


#define UMP_UART_STOPBIT1		0x00
#define UMP_UART_STOPBIT15		0x01
#define UMP_UART_STOPBIT2		0x02


#define UMP_UART_LSR_OV_MASK		0x01
#define UMP_UART_LSR_PE_MASK		0x02
#define UMP_UART_LSR_FE_MASK		0x04
#define UMP_UART_LSR_BR_MASK		0x08
#define UMP_UART_LSR_ER_MASK		0x0F
#define UMP_UART_LSR_RX_MASK		0x10
#define UMP_UART_LSR_TX_MASK		0x20

#define UMP_UART_LSR_DATA_MASK		( LSR_PAR_ERR | LSR_FRM_ERR | LSR_BREAK )


#define UMP_MASK_UART_FLAGS_RTS_FLOW		0x0001
#define UMP_MASK_UART_FLAGS_RTS_DISABLE		0x0002
#define UMP_MASK_UART_FLAGS_PARITY		0x0008
#define UMP_MASK_UART_FLAGS_OUT_X_DSR_FLOW	0x0010
#define UMP_MASK_UART_FLAGS_OUT_X_CTS_FLOW	0x0020
#define UMP_MASK_UART_FLAGS_OUT_X		0x0040
#define UMP_MASK_UART_FLAGS_OUT_XA		0x0080
#define UMP_MASK_UART_FLAGS_IN_X		0x0100
#define UMP_MASK_UART_FLAGS_DTR_FLOW		0x0800
#define UMP_MASK_UART_FLAGS_DTR_DISABLE		0x1000
#define UMP_MASK_UART_FLAGS_RECEIVE_MS_INT	0x2000
#define UMP_MASK_UART_FLAGS_AUTO_START_ON_ERR	0x4000

#define UMP_DMA_MODE_CONTINOUS			0x01
#define UMP_PIPE_TRANS_TIMEOUT_ENA		0x80
#define UMP_PIPE_TRANSFER_MODE_MASK		0x03
#define UMP_PIPE_TRANS_TIMEOUT_MASK		0x7C


#define UMP_PORT_DIR_OUT			0x01
#define UMP_PORT_DIR_IN				0x02


#define UMPM_UART1_PORT  			0x03


#define	UMPC_SET_CONFIG 		0x05
#define	UMPC_OPEN_PORT  		0x06
#define	UMPC_CLOSE_PORT 		0x07
#define	UMPC_START_PORT 		0x08
#define	UMPC_STOP_PORT  		0x09
#define	UMPC_TEST_PORT  		0x0A
#define	UMPC_PURGE_PORT 		0x0B

#define	UMPC_COMPLETE_READ		0x80	
#define	UMPC_HARDWARE_RESET		0x81	
#define	UMPC_COPY_DNLD_TO_I2C		0x82	
						

	
	
	
#define	UMPC_WRITE_SFR	  		0x83	

	
#define	UMPC_READ_SFR	  		0x84	

	
#define	UMPC_SET_CLR_DTR		0x85

	
#define	UMPC_SET_CLR_RTS		0x86

	
#define	UMPC_SET_CLR_LOOPBACK		0x87

	
#define	UMPC_SET_CLR_BREAK		0x88

	
#define	UMPC_READ_MSR			0x89

	
	
#define	UMPC_MEMORY_READ   		0x92
#define	UMPC_MEMORY_WRITE  		0x93


#define UMPD_OEDB1_ADDRESS		0xFF08
#define UMPD_OEDB2_ADDRESS		0xFF10

struct out_endpoint_desc_block
{
	__u8 Configuration;
	__u8 XBufAddr;
	__u8 XByteCount;
	__u8 Unused1;
	__u8 Unused2;
	__u8 YBufAddr;
	__u8 YByteCount;
	__u8 BufferSize;
} __attribute__((packed));



struct ump_uart_config		
{
	__u16 wBaudRate;	
	__u16 wFlags;		
	__u8 bDataBits;		
	__u8 bParity;		
	__u8 bStopBits;		
	char cXon;		
	char cXoff;		
	__u8 bUartMode;		
				
} __attribute__((packed));



struct ump_interrupt			
{
	__u8 bICode;			
	__u8 bIInfo;			
}  __attribute__((packed));


#define TIUMP_GET_PORT_FROM_CODE(c)	(((c) >> 4) - 3)
#define TIUMP_GET_FUNC_FROM_CODE(c)	((c) & 0x0f)
#define TIUMP_INTERRUPT_CODE_LSR	0x03
#define TIUMP_INTERRUPT_CODE_MSR	0x04

#endif
