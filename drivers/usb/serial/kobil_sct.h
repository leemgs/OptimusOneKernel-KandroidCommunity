#define SUSBCRequest_SetBaudRateParityAndStopBits       1
#define SUSBCR_SBR_MASK				0xFF00
#define SUSBCR_SBR_1200				0x0100
#define SUSBCR_SBR_9600				0x0200
#define SUSBCR_SBR_19200			0x0400
#define SUSBCR_SBR_28800			0x0800
#define SUSBCR_SBR_38400			0x1000
#define SUSBCR_SBR_57600			0x2000
#define SUSBCR_SBR_115200			0x4000

#define SUSBCR_SPASB_MASK			0x0070
#define SUSBCR_SPASB_NoParity			0x0010
#define SUSBCR_SPASB_OddParity			0x0020
#define SUSBCR_SPASB_EvenParity			0x0040

#define SUSBCR_SPASB_STPMASK			0x0003
#define SUSBCR_SPASB_1StopBit			0x0001
#define SUSBCR_SPASB_2StopBits			0x0002

#define SUSBCRequest_SetStatusLinesOrQueues	2
#define SUSBCR_SSL_SETRTS			0x0001
#define SUSBCR_SSL_CLRRTS			0x0002
#define SUSBCR_SSL_SETDTR			0x0004
#define SUSBCR_SSL_CLRDTR			0x0010

#define SUSBCR_SSL_PURGE_TXABORT		0x0100  
#define SUSBCR_SSL_PURGE_RXABORT		0x0200  
#define SUSBCR_SSL_PURGE_TXCLEAR		0x0400  
#define SUSBCR_SSL_PURGE_RXCLEAR		0x0800  

#define SUSBCRequest_GetStatusLineState		4
#define SUSBCR_GSL_RXCHAR			0x0001  
#define SUSBCR_GSL_TXEMPTY			0x0004  
#define SUSBCR_GSL_CTS				0x0008  
#define SUSBCR_GSL_DSR				0x0010  
#define SUSBCR_GSL_RLSD				0x0020  
#define SUSBCR_GSL_BREAK			0x0040  
#define SUSBCR_GSL_ERR				0x0080  
#define SUSBCR_GSL_RING				0x0100  

#define SUSBCRequest_Misc			8
#define SUSBCR_MSC_ResetReader			0x0001	
#define SUSBCR_MSC_ResetAllQueues		0x0002	

#define SUSBCRequest_GetMisc			0x10
#define SUSBCR_MSC_GetFWVersion			0x0001	

#define SUSBCR_MSC_GetHWVersion			0x0002	
