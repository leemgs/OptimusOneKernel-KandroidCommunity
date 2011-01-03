#ifndef __WINBOND_WB35REG_S_H
#define __WINBOND_WB35REG_S_H

#include <linux/spinlock.h>
#include <linux/types.h>
#include <asm/atomic.h>





#define     GetBit( dwData, i)      ( dwData & (0x00000001 << i))
#define     SetBit( dwData, i)      ( dwData | (0x00000001 << i))
#define     ClearBit( dwData, i)    ( dwData & ~(0x00000001 << i))

#define		IGNORE_INCREMENT	0
#define		AUTO_INCREMENT		0
#define		NO_INCREMENT		1
#define REG_DIRECTION(_x,_y)   ((_y)->DIRECT ==0 ? usb_rcvctrlpipe(_x,0) : usb_sndctrlpipe(_x,0))
#define REG_BUF_SIZE(_x)       ((_x)->bRequest== 0x04 ? cpu_to_le16((_x)->wLength) : 4)


#define BB48_DEFAULT_AL2230_11B		0x0033447c
#define BB4C_DEFAULT_AL2230_11B		0x0A00FEFF
#define BB48_DEFAULT_AL2230_11G		0x00332C1B
#define BB4C_DEFAULT_AL2230_11G		0x0A00FEFF


#define BB48_DEFAULT_WB242_11B		0x00292315	
#define BB4C_DEFAULT_WB242_11B		0x0800FEFF	


#define BB48_DEFAULT_WB242_11G		0x00453B24
#define BB4C_DEFAULT_WB242_11G		0x0E00FEFF




#define DEFAULT_CWMIN					31		
#define DEFAULT_CWMAX					1023	
#define DEFAULT_AID						1		

#ifdef _USE_FALLBACK_RATE_
#define DEFAULT_RATE_RETRY_LIMIT		2		
#else
#define DEFAULT_RATE_RETRY_LIMIT		7		
#endif

#define DEFAULT_LONG_RETRY_LIMIT		7		
#define DEFAULT_SHORT_RETRY_LIMIT		7		
#define DEFAULT_PIFST					25		
#define DEFAULT_EIFST					354		
#define DEFAULT_DIFST					45		
#define DEFAULT_SIFST					5		
#define DEFAULT_OSIFST					10		
#define DEFAULT_ATIMWD					0		
#define DEFAULT_SLOT_TIME				20		
#define DEFAULT_MAX_TX_MSDU_LIFE_TIME	512	
#define DEFAULT_BEACON_INTERVAL			500		
#define DEFAULT_PROBE_DELAY_TIME		200		
#define DEFAULT_PROTOCOL_VERSION		0		
#define DEFAULT_MAC_POWER_STATE			2		
#define DEFAULT_DTIM_ALERT_TIME			0


struct wb35_reg_queue {
	struct urb 	*urb;
	void		*pUsbReq;
	void		*Next;
	union {
		u32	VALUE;
		u32	*pBuffer;
	};
	u8		RESERVED[4]; 
	u16		INDEX; 
	u8		RESERVED_VALID;	
	u8		DIRECT; 
};




#define MAX_SQ3_FILTER_SIZE		5
struct wb35_reg {
	
	
	
	u32	U1B0;			
	u32	U1BC_LEDConfigure;
	u32	D00_DmaControl;
	u32	M00_MacControl;
	union {
		struct {
			u32	M04_MulticastAddress1;
			u32	M08_MulticastAddress2;
		};
		u8		Multicast[8];	
	};

	u32	M24_MacControl;
	u32	M28_MacControl;
	u32	M2C_MacControl;
	u32	M38_MacControl;
	u32	M3C_MacControl; 
	u32	M40_MacControl;
	u32	M44_MacControl; 
	u32	M48_MacControl; 
	u32	M4C_MacStatus;
	u32	M60_MacControl; 
	u32	M68_MacControl; 
	u32	M70_MacControl; 
	u32	M74_MacControl; 
	u32	M78_ERPInformation;
	u32	M7C_MacControl; 
	u32	M80_MacControl; 
	u32	M84_MacControl; 
	u32	M88_MacControl; 
	u32	M98_MacControl; 

	
	
	u32	BB0C;	
	u32	BB2C;	
	u32	BB30;	
	u32	BB3C;
	u32	BB48;	
	u32	BB4C;	
	u32	BB50;	
	u32	BB54;
	u32 	BB58;	
	u32	BB5C;	
	u32	BB60;	

	
	
	
	spinlock_t	EP0VM_spin_lock; 
	u32	        EP0VM_status;
	struct wb35_reg_queue *reg_first;
	struct wb35_reg_queue *reg_last;
	atomic_t       RegFireCount;

	
	u8	EP0vm_state;
	u8	mac_power_save;
	u8	EEPROMPhyType; 
						   
						   
						   
						   
	u8	EEPROMRegion;	

	u32	SyncIoPause; 

	u8	LNAValue[4]; 
	u32	SQ3_filter[MAX_SQ3_FILTER_SIZE];
	u32	SQ3_index;

};

#endif
