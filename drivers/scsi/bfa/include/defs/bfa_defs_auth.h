
#ifndef __BFA_DEFS_AUTH_H__
#define __BFA_DEFS_AUTH_H__

#include <defs/bfa_defs_types.h>

#define PUBLIC_KEY			15409
#define PRIVATE_KEY			19009
#define KEY_LEN				32399
#define BFA_AUTH_SECRET_STRING_LEN	256
#define BFA_AUTH_FAIL_TIMEOUT		0xFF


enum bfa_auth_status {
	BFA_AUTH_STATUS_NONE 	= 0,	
	BFA_AUTH_UNINIT 	= 1,	
	BFA_AUTH_NEG_SEND 	= 2,	
	BFA_AUTH_CHAL_WAIT 	= 3,	
	BFA_AUTH_NEG_RETRY 	= 4,	
	BFA_AUTH_REPLY_SEND 	= 5,	
	BFA_AUTH_STATUS_WAIT 	= 6,	
	BFA_AUTH_SUCCESS 	= 7,	
	BFA_AUTH_FAILED 	= 8,	
	BFA_AUTH_STATUS_UNKNOWN = 9,	
};

struct auth_proto_stats_s {
	u32        auth_rjts;
	u32        auth_negs;
	u32        auth_dones;

	u32        dhchap_challenges;
	u32        dhchap_replies;
	u32        dhchap_successes;
};


struct bfa_auth_stats_s {
	u32           auth_failures;	
	u32           auth_successes;	
	struct auth_proto_stats_s auth_rx_stats; 
	struct auth_proto_stats_s auth_tx_stats; 
};


enum bfa_auth_algo {
	BFA_AUTH_ALGO_MD5 	= 1,	
	BFA_AUTH_ALGO_SHA1 	= 2,	
	BFA_AUTH_ALGO_MS 	= 3,	
	BFA_AUTH_ALGO_SM 	= 4,	
};


enum bfa_auth_group {
	BFA_AUTH_GROUP_DHNULL 	= 0,	
	BFA_AUTH_GROUP_DH768 	= 1,	
	BFA_AUTH_GROUP_DH1024 	= 2,	
	BFA_AUTH_GROUP_DH1280 	= 4,	
	BFA_AUTH_GROUP_DH1536 	= 8,	

	BFA_AUTH_GROUP_ALL 	= 256	
};


enum bfa_auth_secretsource {
	BFA_AUTH_SECSRC_LOCAL 	= 1,	
	BFA_AUTH_SECSRC_RADIUS 	= 2,	
	BFA_AUTH_SECSRC_TACACS 	= 3,	
};


struct bfa_auth_attr_s {
	enum bfa_auth_status 	status;
	enum bfa_auth_algo 	algo;
	enum bfa_auth_group 	dh_grp;
	u16		rjt_code;
	u16		rjt_code_exp;
	u8			secret_set;
	u8			resv[7];
};

#endif 
