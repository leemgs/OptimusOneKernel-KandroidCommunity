#ifndef QDSP5VFEMSGI_H
#define QDSP5VFEMSGI_H







#define	VFE_MSG_RESET_ACK	0x0000
#define	VFE_MSG_RESET_ACK_LEN	sizeof(vfe_msg_reset_ack)

typedef struct {
} __attribute__((packed)) vfe_msg_reset_ack;




#define	VFE_MSG_START_ACK	0x0001
#define	VFE_MSG_START_ACK_LEN	sizeof(vfe_msg_start_ack)

typedef struct {
} __attribute__((packed)) vfe_msg_start_ack;



#define	VFE_MSG_STOP_ACK	0x0002
#define	VFE_MSG_STOP_ACK_LEN	sizeof(vfe_msg_stop_ack)

typedef struct {
} __attribute__((packed)) vfe_msg_stop_ack;




#define	VFE_MSG_UPDATE_ACK	0x0003
#define	VFE_MSG_UPDATE_ACK_LEN	sizeof(vfe_msg_update_ack)

typedef struct {
} __attribute__((packed)) vfe_msg_update_ack;




#define	VFE_MSG_SNAPSHOT_DONE		0x0004
#define	VFE_MSG_SNAPSHOT_DONE_LEN	\
	sizeof(vfe_msg_snapshot_done)

typedef struct {
} __attribute__((packed)) vfe_msg_snapshot_done;





#define	VFE_MSG_ILLEGAL_CMD	0x0005
#define	VFE_MSG_ILLEGAL_CMD_LEN	\
	sizeof(vfe_msg_illegal_cmd)

typedef struct {
	unsigned int	status;
} __attribute__((packed)) vfe_msg_illegal_cmd;




#define	VFE_MSG_OP1		0x0006
#define	VFE_MSG_OP1_LEN		sizeof(vfe_msg_op1)

typedef struct {
	unsigned int	op1_buf_y_addr;
	unsigned int	op1_buf_cbcr_addr;
	unsigned int	black_level_even_col;
	unsigned int	black_level_odd_col;
	unsigned int	defect_pixels_detected;
	unsigned int	asf_max_edge;
} __attribute__((packed)) vfe_msg_op1; 




#define	VFE_MSG_OP2		0x0007
#define	VFE_MSG_OP2_LEN		sizeof(vfe_msg_op2)

typedef struct {
	unsigned int	op2_buf_y_addr;
	unsigned int	op2_buf_cbcr_addr;
	unsigned int	black_level_even_col;
	unsigned int	black_level_odd_col;
	unsigned int	defect_pixels_detected;
	unsigned int	asf_max_edge;
} __attribute__((packed)) vfe_msg_op2; 




#define	VFE_MSG_STATS_AF	0x0008
#define	VFE_MSG_STATS_AF_LEN	sizeof(vfe_msg_stats_af)

typedef struct {
	unsigned int	af_stats_op_buffer;
} __attribute__((packed)) vfe_msg_stats_af;




#define	VFE_MSG_STATS_WB_EXP		0x0009
#define	VFE_MSG_STATS_WB_EXP_LEN	\
	sizeof(vfe_msg_stats_wb_exp)

typedef struct {
	unsigned int	wb_exp_stats_op_buf;
} __attribute__((packed)) vfe_msg_stats_wb_exp;




#define	VFE_MSG_STATS_HG	0x000A
#define	VFE_MSG_STATS_HG_LEN	sizeof(vfe_msg_stats_hg)

typedef struct {
	unsigned int	hg_stats_op_buf;
} __attribute__((packed)) vfe_msg_stats_hg;




#define	VFE_MSG_EPOCH1		0x000B
#define	VFE_MSG_EPOCH1_LEN	sizeof(vfe_msg_epoch1)

typedef struct {
} __attribute__((packed)) vfe_msg_epoch1;




#define	VFE_MSG_EPOCH2		0x000C
#define	VFE_MSG_EPOCH2_LEN	sizeof(vfe_msg_epoch2)

typedef struct {
} __attribute__((packed)) vfe_msg_epoch2;




#define	VFE_MSG_SYNC_T1_DONE		0x000D
#define	VFE_MSG_SYNC_T1_DONE_LEN	sizeof(vfe_msg_sync_t1_done)

typedef struct {
} __attribute__((packed)) vfe_msg_sync_t1_done;




#define	VFE_MSG_SYNC_T2_DONE		0x000E
#define	VFE_MSG_SYNC_T2_DONE_LEN	sizeof(vfe_msg_sync_t2_done)

typedef struct {
} __attribute__((packed)) vfe_msg_sync_t2_done;




#define	VFE_MSG_ASYNC_T1_DONE		0x000F
#define	VFE_MSG_ASYNC_T1_DONE_LEN	sizeof(vfe_msg_async_t1_done)

typedef struct {
} __attribute__((packed)) vfe_msg_async_t1_done;





#define	VFE_MSG_ASYNC_T2_DONE		0x0010
#define	VFE_MSG_ASYNC_T2_DONE_LEN	sizeof(vfe_msg_async_t2_done)

typedef struct {
} __attribute__((packed)) vfe_msg_async_t2_done;





#define	VFE_MSG_ERROR		0x0011
#define	VFE_MSG_ERROR_LEN	sizeof(vfe_msg_error)

#define	VFE_MSG_ERR_COND_NO_CAMIF_ERR		0x0000
#define	VFE_MSG_ERR_COND_CAMIF_ERR		0x0001
#define	VFE_MSG_ERR_COND_OP1_Y_NO_BUS_OF	0x0000
#define	VFE_MSG_ERR_COND_OP1_Y_BUS_OF		0x0002
#define	VFE_MSG_ERR_COND_OP1_CBCR_NO_BUS_OF	0x0000
#define	VFE_MSG_ERR_COND_OP1_CBCR_BUS_OF	0x0004
#define	VFE_MSG_ERR_COND_OP2_Y_NO_BUS_OF	0x0000
#define	VFE_MSG_ERR_COND_OP2_Y_BUS_OF		0x0008
#define	VFE_MSG_ERR_COND_OP2_CBCR_NO_BUS_OF	0x0000
#define	VFE_MSG_ERR_COND_OP2_CBCR_BUS_OF	0x0010
#define	VFE_MSG_ERR_COND_AF_NO_BUS_OF		0x0000
#define	VFE_MSG_ERR_COND_AF_BUS_OF		0x0020
#define	VFE_MSG_ERR_COND_WB_EXP_NO_BUS_OF	0x0000
#define	VFE_MSG_ERR_COND_WB_EXP_BUS_OF		0x0040
#define	VFE_MSG_ERR_COND_NO_AXI_ERR		0x0000
#define	VFE_MSG_ERR_COND_AXI_ERR		0x0080

#define	VFE_MSG_CAMIF_STS_IDLE			0x0000
#define	VFE_MSG_CAMIF_STS_CAPTURE_DATA		0x0001

typedef struct {
	unsigned int	err_cond;
	unsigned int	camif_sts;
} __attribute__((packed)) vfe_msg_error;


#endif
