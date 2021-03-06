
#ifndef __BFA_PORTLOG_H__
#define __BFA_PORTLOG_H__

#include "protocol/fc.h"
#include <defs/bfa_defs_types.h>

#define BFA_PL_NLOG_ENTS 256
#define BFA_PL_LOG_REC_INCR(_x) ((_x)++, (_x) %= BFA_PL_NLOG_ENTS)

#define BFA_PL_STRING_LOG_SZ   32   
#define BFA_PL_INT_LOG_SZ      8    

enum bfa_plog_log_type {
	BFA_PL_LOG_TYPE_INVALID = 0,
	BFA_PL_LOG_TYPE_INT 	= 1,
	BFA_PL_LOG_TYPE_STRING 	= 2,
};


struct bfa_plog_rec_s {
	u32        tv;	
	u8         port;	
	u8         mid;	
	u8         eid;	
	u8         log_type; 
	u8         log_num_ints;
	
	u8         rsvd;
	u16        misc;	
	union {
		char            string_log[BFA_PL_STRING_LOG_SZ];
		u32        int_log[BFA_PL_INT_LOG_SZ];
	} log_entry;

};


enum bfa_plog_mid {
	BFA_PL_MID_INVALID 	= 0,
	BFA_PL_MID_DEBUG 	= 1,
	BFA_PL_MID_DRVR 	= 2,
	BFA_PL_MID_HAL 		= 3,
	BFA_PL_MID_HAL_FCXP 	= 4,
	BFA_PL_MID_HAL_UF 	= 5,
	BFA_PL_MID_FCS 		= 6,
	BFA_PL_MID_MAX 		= 7
};

#define BFA_PL_MID_STRLEN    8
struct bfa_plog_mid_strings_s {
	char            m_str[BFA_PL_MID_STRLEN];
};


enum bfa_plog_eid {
	BFA_PL_EID_INVALID 		= 0,
	BFA_PL_EID_IOC_DISABLE 		= 1,
	BFA_PL_EID_IOC_ENABLE 		= 2,
	BFA_PL_EID_PORT_DISABLE 	= 3,
	BFA_PL_EID_PORT_ENABLE 		= 4,
	BFA_PL_EID_PORT_ST_CHANGE 	= 5,
	BFA_PL_EID_TX 			= 6,
	BFA_PL_EID_TX_ACK1 		= 7,
	BFA_PL_EID_TX_RJT 		= 8,
	BFA_PL_EID_TX_BSY 		= 9,
	BFA_PL_EID_RX 			= 10,
	BFA_PL_EID_RX_ACK1 		= 11,
	BFA_PL_EID_RX_RJT 		= 12,
	BFA_PL_EID_RX_BSY 		= 13,
	BFA_PL_EID_CT_IN 		= 14,
	BFA_PL_EID_CT_OUT 		= 15,
	BFA_PL_EID_DRIVER_START 	= 16,
	BFA_PL_EID_RSCN 		= 17,
	BFA_PL_EID_DEBUG 		= 18,
	BFA_PL_EID_MISC 		= 19,
	BFA_PL_EID_MAX 			= 20
};

#define BFA_PL_ENAME_STRLEN    	8
struct bfa_plog_eid_strings_s {
	char            e_str[BFA_PL_ENAME_STRLEN];
};

#define BFA_PL_SIG_LEN	8
#define BFA_PL_SIG_STR  "12pl123"


struct bfa_plog_s {
	char            plog_sig[BFA_PL_SIG_LEN];	
	u8         plog_enabled;
	u8         rsvd[7];
	u32        ticks;
	u16        head;
	u16        tail;
	struct bfa_plog_rec_s  plog_recs[BFA_PL_NLOG_ENTS];
};

void bfa_plog_init(struct bfa_plog_s *plog);
void bfa_plog_str(struct bfa_plog_s *plog, enum bfa_plog_mid mid,
			enum bfa_plog_eid event, u16 misc, char *log_str);
void bfa_plog_intarr(struct bfa_plog_s *plog, enum bfa_plog_mid mid,
			enum bfa_plog_eid event, u16 misc,
			u32 *intarr, u32 num_ints);
void bfa_plog_fchdr(struct bfa_plog_s *plog, enum bfa_plog_mid mid,
			enum bfa_plog_eid event, u16 misc,
			struct fchs_s *fchdr);
void bfa_plog_fchdr_and_pl(struct bfa_plog_s *plog, enum bfa_plog_mid mid,
			enum bfa_plog_eid event, u16 misc,
			struct fchs_s *fchdr, u32 pld_w0);
void bfa_plog_clear(struct bfa_plog_s *plog);
void bfa_plog_enable(struct bfa_plog_s *plog);
void bfa_plog_disable(struct bfa_plog_s *plog);
bfa_boolean_t	bfa_plog_get_setting(struct bfa_plog_s *plog);

#endif 
