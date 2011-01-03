

#ifndef _ADP5588_H
#define _ADP5588_H

#define DEV_ID 0x00		
#define CFG 0x01		
#define INT_STAT 0x02		
#define KEY_LCK_EC_STAT 0x03	
#define Key_EVENTA 0x04		
#define Key_EVENTB 0x05		
#define Key_EVENTC 0x06		
#define Key_EVENTD 0x07		
#define Key_EVENTE 0x08		
#define Key_EVENTF 0x09		
#define Key_EVENTG 0x0A		
#define Key_EVENTH 0x0B		
#define Key_EVENTI 0x0C		
#define Key_EVENTJ 0x0D		
#define KP_LCK_TMR 0x0E		
#define UNLOCK1 0x0F		
#define UNLOCK2 0x10		
#define GPIO_INT_STAT1 0x11	
#define GPIO_INT_STAT2 0x12	
#define GPIO_INT_STAT3 0x13	
#define GPIO_DAT_STAT1 0x14	
#define GPIO_DAT_STAT2 0x15	
#define GPIO_DAT_STAT3 0x16	
#define GPIO_DAT_OUT1 0x17	
#define GPIO_DAT_OUT2 0x18	
#define GPIO_DAT_OUT3 0x19	
#define GPIO_INT_EN1 0x1A	
#define GPIO_INT_EN2 0x1B	
#define GPIO_INT_EN3 0x1C	
#define KP_GPIO1 0x1D		
#define KP_GPIO2 0x1E		
#define KP_GPIO3 0x1F		
#define GPI_EM1 0x20		
#define GPI_EM2 0x21		
#define GPI_EM3 0x22		
#define GPIO_DIR1 0x23		
#define GPIO_DIR2 0x24		
#define GPIO_DIR3 0x25		
#define GPIO_INT_LVL1 0x26	
#define GPIO_INT_LVL2 0x27	
#define GPIO_INT_LVL3 0x28	
#define Debounce_DIS1 0x29	
#define Debounce_DIS2 0x2A	
#define Debounce_DIS3 0x2B	
#define GPIO_PULL1 0x2C		
#define GPIO_PULL2 0x2D		
#define GPIO_PULL3 0x2E		
#define CMP_CFG_STAT 0x30	
#define CMP_CONFG_SENS1 0x31	
#define CMP_CONFG_SENS2 0x32	
#define CMP1_LVL2_TRIP 0x33	
#define CMP1_LVL2_HYS 0x34	
#define CMP1_LVL3_TRIP 0x35	
#define CMP1_LVL3_HYS 0x36	
#define CMP2_LVL2_TRIP 0x37	
#define CMP2_LVL2_HYS 0x38	
#define CMP2_LVL3_TRIP 0x39	
#define CMP2_LVL3_HYS 0x3A	
#define CMP1_ADC_DAT_R1 0x3B	
#define CMP1_ADC_DAT_R2 0x3C	
#define CMP2_ADC_DAT_R1 0x3D	
#define CMP2_ADC_DAT_R2 0x3E	

#define ADP5588_DEVICE_ID_MASK	0xF



#define ADP5588_KEYMAPSIZE	80

struct adp5588_kpad_platform_data {
	int rows;			
	int cols;			
	const unsigned short *keymap;	
	unsigned short keymapsize;	
	unsigned repeat:1;		
	unsigned en_keylock:1;		
	unsigned short unlock_key1;	
	unsigned short unlock_key2;	
};

#endif
