

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#include "hardware.h"



#define SA1100_DMA_CHANNELS	6


#define MAX_DMA_SIZE		0x1fff
#define CUT_DMA_SIZE		0x1000


typedef enum {
	DMA_Ser0UDCWr  = DDAR_Ser0UDCWr,   
	DMA_Ser0UDCRd  = DDAR_Ser0UDCRd,   
	DMA_Ser1UARTWr = DDAR_Ser1UARTWr,  
	DMA_Ser1UARTRd = DDAR_Ser1UARTRd,  
	DMA_Ser1SDLCWr = DDAR_Ser1SDLCWr,  
	DMA_Ser1SDLCRd = DDAR_Ser1SDLCRd,  
	DMA_Ser2UARTWr = DDAR_Ser2UARTWr,  
	DMA_Ser2UARTRd = DDAR_Ser2UARTRd,  
	DMA_Ser2HSSPWr = DDAR_Ser2HSSPWr,  
	DMA_Ser2HSSPRd = DDAR_Ser2HSSPRd,  
	DMA_Ser3UARTWr = DDAR_Ser3UARTWr,  
	DMA_Ser3UARTRd = DDAR_Ser3UARTRd,  
	DMA_Ser4MCP0Wr = DDAR_Ser4MCP0Wr,  
	DMA_Ser4MCP0Rd = DDAR_Ser4MCP0Rd,  
	DMA_Ser4MCP1Wr = DDAR_Ser4MCP1Wr,  
	DMA_Ser4MCP1Rd = DDAR_Ser4MCP1Rd,  
	DMA_Ser4SSPWr  = DDAR_Ser4SSPWr,   
	DMA_Ser4SSPRd  = DDAR_Ser4SSPRd    
} dma_device_t;

typedef struct {
	volatile u_long DDAR;
	volatile u_long SetDCSR;
	volatile u_long ClrDCSR;
	volatile u_long RdDCSR;
	volatile dma_addr_t DBSA;
	volatile u_long DBTA;
	volatile dma_addr_t DBSB;
	volatile u_long DBTB;
} dma_regs_t;

typedef void (*dma_callback_t)(void *data);



extern int sa1100_request_dma( dma_device_t device, const char *device_id,
			       dma_callback_t callback, void *data,
			       dma_regs_t **regs );
extern void sa1100_free_dma( dma_regs_t *regs );
extern int sa1100_start_dma( dma_regs_t *regs, dma_addr_t dma_ptr, u_int size );
extern dma_addr_t sa1100_get_dma_pos(dma_regs_t *regs);
extern void sa1100_reset_dma(dma_regs_t *regs);



#define sa1100_stop_dma(regs)	((regs)->ClrDCSR = DCSR_IE|DCSR_RUN)



#define sa1100_resume_dma(regs)	((regs)->SetDCSR = DCSR_IE|DCSR_RUN)



#define sa1100_clear_dma(regs)	((regs)->ClrDCSR = DCSR_IE|DCSR_RUN|DCSR_STRTA|DCSR_STRTB)

#endif 
