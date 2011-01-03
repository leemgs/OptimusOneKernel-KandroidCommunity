





#define SPEEDSTEP_CPU_PIII_C_EARLY	0x00000001  
#define SPEEDSTEP_CPU_PIII_C		0x00000002  
#define SPEEDSTEP_CPU_PIII_T		0x00000003  
#define SPEEDSTEP_CPU_P4M		0x00000004  


#define SPEEDSTEP_CPU_PM		0xFFFFFF03  
#define SPEEDSTEP_CPU_P4D		0xFFFFFF04  
#define SPEEDSTEP_CPU_PCORE		0xFFFFFF05  



#define SPEEDSTEP_HIGH	0x00000000
#define SPEEDSTEP_LOW	0x00000001



extern unsigned int speedstep_detect_processor (void);


extern unsigned int speedstep_get_frequency(unsigned int processor);



extern unsigned int speedstep_get_freqs(unsigned int processor,
	unsigned int *low_speed,
	unsigned int *high_speed,
	unsigned int *transition_latency,
	void (*set_state) (unsigned int state));
