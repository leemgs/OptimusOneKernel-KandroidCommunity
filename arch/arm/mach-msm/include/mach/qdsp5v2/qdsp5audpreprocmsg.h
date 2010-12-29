

#ifndef QDSP5AUDPREPROCMSG_H
#define QDSP5AUDPREPROCMSG_H





#define AUDPREPROC_CMD_CFG_DONE_MSG 0x0001
#define	AUDPREPROC_CMD_CFG_DONE_MSG_LEN	\
	sizeof(struct audpreproc_cmd_cfg_done_msg)

#define AUD_PREPROC_TYPE_AGC		0x0
#define AUD_PREPROC_NOISE_REDUCTION	0x1
#define AUD_PREPROC_IIR_TUNNING_FILTER	0x2

#define AUD_PREPROC_CONFIG_ENABLED 	-1
#define AUD_PREPROC_CONFIG_DISABLED	 0

struct audpreproc_cmd_cfg_done_msg {
	unsigned short stream_id;
	unsigned short aud_preproc_type;
	signed short aud_preproc_status_flag;
} __attribute__((packed));



#define AUDPREPROC_ERROR_MSG 0x0002
#define AUDPREPROC_ERROR_MSG_LEN \
	sizeof(struct audpreproc_err_msg)

#define AUD_PREPROC_ERR_IDX_WRONG_SAMPLING_FREQUENCY	0x00
#define AUD_PREPROC_ERR_IDX_ENC_NOT_SUPPORTED		0x01

struct audpreproc_err_msg {
	unsigned short stream_id;
	signed short aud_preproc_err_idx;
} __attribute__((packed));



#define AUDPREPROC_CMD_ENC_CFG_DONE_MSG	0x0003
#define AUDPREPROC_CMD_ENC_CFG_DONE_MSG_LEN \
	sizeof(struct audpreproc_cmd_enc_cfg_done_msg)

struct audpreproc_cmd_enc_cfg_done_msg {
	unsigned short stream_id;
	unsigned short rec_enc_type;
} __attribute__((packed));



#define AUDPREPROC_CMD_ENC_PARAM_CFG_DONE_MSG	0x0004
#define AUDPREPROC_CMD_ENC_PARAM_CFG_DONE_MSG_LEN \
	sizeof(struct audpreproc_cmd_enc_param_cfg_done_msg)

struct audpreproc_cmd_enc_param_cfg_done_msg {
	unsigned short stream_id;
} __attribute__((packed));




#define AUDPREPROC_AFE_CMD_AUDIO_RECORD_CFG_DONE_MSG  0x0005
#define AUDPREPROC_AFE_CMD_AUDIO_RECORD_CFG_DONE_MSG_LEN \
	sizeof(struct audpreproc_afe_cmd_audio_record_cfg_done)

struct audpreproc_afe_cmd_audio_record_cfg_done {
	unsigned short stream_id;
} __attribute__((packed));

#endif 
