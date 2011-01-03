

#ifndef __FC_SP_H__
#define __FC_SP_H__

#include <protocol/types.h>

#pragma pack(1)

enum auth_els_flags{
	FC_AUTH_ELS_MORE_FRAGS_FLAG 	= 0x80,	
	FC_AUTH_ELS_CONCAT_FLAG 	= 0x40,	
	FC_AUTH_ELS_SEQ_NUM_FLAG 	= 0x01 	
};

enum auth_msg_codes{
	FC_AUTH_MC_AUTH_RJT		= 0x0A,	
	FC_AUTH_MC_AUTH_NEG 		= 0x0B, 
	FC_AUTH_MC_AUTH_DONE 		= 0x0C, 

	FC_AUTH_MC_DHCHAP_CHAL 		= 0x10, 
	FC_AUTH_MC_DHCHAP_REPLY 	= 0x11, 
	FC_AUTH_MC_DHCHAP_SUCC 		= 0x12, 

	FC_AUTH_MC_FCAP_REQ 		= 0x13, 
	FC_AUTH_MC_FCAP_ACK 		= 0x14, 
	FC_AUTH_MC_FCAP_CONF 		= 0x15, 

	FC_AUTH_MC_FCPAP_INIT 		= 0x16, 
	FC_AUTH_MC_FCPAP_ACC 		= 0x17, 
	FC_AUTH_MC_FCPAP_COMP 		= 0x18, 

	FC_AUTH_MC_IKE_SA_INIT 		= 0x22, 
	FC_AUTH_MC_IKE_SA_AUTH 		= 0x23, 
	FC_AUTH_MC_IKE_CREATE_CHILD_SA	= 0x24, 
	FC_AUTH_MC_IKE_INFO 		= 0x25, 
};

enum auth_proto_version{
	FC_AUTH_PROTO_VER_1 	= 1,	
};

enum {
	FC_AUTH_ELS_COMMAND_CODE = 0x90,
	FC_AUTH_PROTO_PARAM_LEN_SZ = 4,	
	FC_AUTH_PROTO_PARAM_VAL_SZ = 4,	
	FC_MAX_AUTH_SECRET_LEN     = 256,
					
	FC_AUTH_NUM_USABLE_PROTO_LEN_SZ = 4,
					
	FC_AUTH_RESP_VALUE_LEN_SZ	= 4,
					
	FC_MAX_CHAP_KEY_LEN	= 256,	
	FC_MAX_AUTH_RETRIES     = 3,	
	FC_MD5_DIGEST_LEN       = 16,	
	FC_SHA1_DIGEST_LEN      = 20,	
	FC_MAX_DHG_SUPPORTED    = 1,	
	FC_MAX_ALG_SUPPORTED    = 1,	
	FC_MAX_PROTO_SUPPORTED  = 1,	
	FC_START_TXN_ID         = 2,	
};

enum auth_proto_id{
	FC_AUTH_PROTO_DHCHAP		= 0x00000001,
	FC_AUTH_PROTO_FCAP 		= 0x00000002,
	FC_AUTH_PROTO_FCPAP 		= 0x00000003,
	FC_AUTH_PROTO_IKEv2 		= 0x00000004,
	FC_AUTH_PROTO_IKEv2_AUTH 	= 0x00000005,
};

struct auth_name_s{
	u16	name_tag;	
	u16	name_len;	
	wwn_t		name;  		
};


enum auth_hash_func{
	FC_AUTH_HASH_FUNC_MD5 		= 0x00000005,
	FC_AUTH_HASH_FUNC_SHA_1 	= 0x00000006,
};

enum auth_dh_gid{
	FC_AUTH_DH_GID_0_DHG_NULL	= 0x00000000,
	FC_AUTH_DH_GID_1_DHG_1024	= 0x00000001,
	FC_AUTH_DH_GID_2_DHG_1280	= 0x00000002,
	FC_AUTH_DH_GID_3_DHG_1536	= 0x00000003,
	FC_AUTH_DH_GID_4_DHG_2048	= 0x00000004,
	FC_AUTH_DH_GID_6_DHG_3072	= 0x00000006,
	FC_AUTH_DH_GID_7_DHG_4096	= 0x00000007,
	FC_AUTH_DH_GID_8_DHG_6144	= 0x00000008,
	FC_AUTH_DH_GID_9_DHG_8192	= 0x00000009,
};

struct auth_els_msg_s {
	u8		auth_els_code;	
	u8 	auth_els_flag; 	
	u8 	auth_msg_code; 	
	u8 	proto_version; 	
	u32	msg_len; 	
	u32	trans_id; 	

	
};


enum auth_neg_param_tags {
	FC_AUTH_NEG_DHCHAP_HASHLIST 	= 0x0001,
	FC_AUTH_NEG_DHCHAP_DHG_ID_LIST 	= 0x0002,
};


struct dhchap_param_format_s {
	u16	tag;		
	u16	word_cnt;

	
};

struct auth_proto_params_s {
	u32	proto_param_len;
	u32	proto_id;

	
};

struct auth_neg_msg_s {
	struct auth_name_s	auth_ini_name;
	u32		usable_auth_protos;
	struct auth_proto_params_s proto_params[1]; 
};

struct auth_dh_val_s {
	u32 dh_val_len;
	u32 dh_val[1];
};

struct auth_dhchap_chal_msg_s {
	struct auth_els_msg_s	hdr;
	struct auth_name_s auth_responder_name;	
	u32 	hash_id;
	u32 	dh_grp_id;
	u32 	chal_val_len;
	char		chal_val[1];

	
};


enum auth_rjt_codes {
	FC_AUTH_RJT_CODE_AUTH_FAILURE 	= 0x01,
	FC_AUTH_RJT_CODE_LOGICAL_ERR	= 0x02,
};

enum auth_rjt_code_exps {
	FC_AUTH_CEXP_AUTH_MECH_NOT_USABLE	= 0x01,
	FC_AUTH_CEXP_DH_GROUP_NOT_USABLE 	= 0x02,
	FC_AUTH_CEXP_HASH_FUNC_NOT_USABLE 	= 0x03,
	FC_AUTH_CEXP_AUTH_XACT_STARTED		= 0x04,
	FC_AUTH_CEXP_AUTH_FAILED 		= 0x05,
	FC_AUTH_CEXP_INCORRECT_PLD 		= 0x06,
	FC_AUTH_CEXP_INCORRECT_PROTO_MSG 	= 0x07,
	FC_AUTH_CEXP_RESTART_AUTH_PROTO 	= 0x08,
	FC_AUTH_CEXP_AUTH_CONCAT_NOT_SUPP 	= 0x09,
	FC_AUTH_CEXP_PROTO_VER_NOT_SUPP 	= 0x0A,
};

enum auth_status {
	FC_AUTH_STATE_INPROGRESS = 0, 	
	FC_AUTH_STATE_FAILED	= 1, 	
	FC_AUTH_STATE_SUCCESS	= 2 	
};

struct auth_rjt_msg_s {
	struct auth_els_msg_s	hdr;
	u8		reason_code;
	u8		reason_code_exp;
	u8		rsvd[2];
};


struct auth_dhchap_neg_msg_s {
	struct auth_els_msg_s hdr;
	struct auth_neg_msg_s nego;
};

struct auth_dhchap_reply_msg_s {
	struct auth_els_msg_s	hdr;

	
};

#pragma pack()

#endif 
