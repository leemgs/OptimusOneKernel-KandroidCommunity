


#ifndef _PPS_H_
#define _PPS_H_

#include <linux/types.h>

#define PPS_VERSION		"5.3.6"
#define PPS_MAX_SOURCES		16		





#define PPS_API_VERS_1		1
#define PPS_API_VERS		PPS_API_VERS_1	
#define PPS_MAX_NAME_LEN	32


struct pps_ktime {
	__s64 sec;
	__s32 nsec;
	__u32 flags;
};
#define PPS_TIME_INVALID	(1<<0)	

struct pps_kinfo {
	__u32 assert_sequence;		
	__u32 clear_sequence; 		
	struct pps_ktime assert_tu;	
	struct pps_ktime clear_tu;	
	int current_mode;		
};

struct pps_kparams {
	int api_version;		
	int mode;			
	struct pps_ktime assert_off_tu;	
	struct pps_ktime clear_off_tu;	
};




#define PPS_CAPTUREASSERT	0x01	
#define PPS_CAPTURECLEAR	0x02	
#define PPS_CAPTUREBOTH		0x03	

#define PPS_OFFSETASSERT	0x10	
#define PPS_OFFSETCLEAR		0x20	

#define PPS_CANWAIT		0x100	
#define PPS_CANPOLL		0x200	


#define PPS_ECHOASSERT		0x40	
#define PPS_ECHOCLEAR		0x80	


#define PPS_TSFMT_TSPEC		0x1000	
#define PPS_TSFMT_NTPFP		0x2000	




#define PPS_KC_HARDPPS		0	
#define PPS_KC_HARDPPS_PLL	1	
#define PPS_KC_HARDPPS_FLL	2	


struct pps_fdata {
	struct pps_kinfo info;
	struct pps_ktime timeout;
};

#include <linux/ioctl.h>

#define PPS_GETPARAMS		_IOR('p', 0xa1, struct pps_kparams *)
#define PPS_SETPARAMS		_IOW('p', 0xa2, struct pps_kparams *)
#define PPS_GETCAP		_IOR('p', 0xa3, int *)
#define PPS_FETCH		_IOWR('p', 0xa4, struct pps_fdata *)

#endif 
