




#define SYS_CLOCK_OFF   0x00050030  



#define SYS_CLOCK_START (IO_START + SYS_CLOCK_OFF)  
#define SYS_CLOCK_BASE  (IO_BASE  + SYS_CLOCK_OFF)  



typedef struct
{
     unsigned int ENABLE;
     unsigned int ESYNC;
     unsigned int SELECT;
} sys_clock_interface;

#define SYS_CLOCK   ((volatile sys_clock_interface *)(SYS_CLOCK_BASE))







#define SYN_EN          1<<0
#define B18M_EN         1<<1
#define CLK3M6_EN       1<<2
#define BUART_EN        1<<3
#define CLK18MU_EN      1<<4
#define FIR_EN          1<<5
#define MIRN_EN         1<<6
#define UARTM_EN        1<<7
#define SIBADC_EN       1<<8
#define ALTD_EN         1<<9
#define CLCLK_EN        1<<10



#define CLK18M_DIV      1<<0
#define MIR_SEL         1<<1
#define SSP_SEL         1<<4
#define MM_DIV          1<<5
#define MM_SEL          1<<6
#define ADC_SEL_2       0<<7
#define ADC_SEL_4       1<<7
#define ADC_SEL_8       3<<7
#define ADC_SEL_16      7<<7
#define ADC_SEL_32      0x0f<<7
#define ADC_SEL_64      0x1f<<7
#define ADC_SEL_128     0x3f<<7
#define ALTD_SEL        1<<13
