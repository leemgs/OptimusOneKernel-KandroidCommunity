

#ifndef QDSP5AUDRECMSG_H
#define QDSP5AUDRECMSG_H





#define AUDREC_FATAL_ERR_MSG 0x0001
#define AUDREC_FATAL_ERR_MSG_LEN	\
	sizeof(struct audrec_fatal_err_msg)

#define AUDREC_FATAL_ERR_MSG_NO_PKT	0x00

struct audrec_fatal_err_msg {
	unsigned short audrec_err_id;
} __attribute__((packed));



#define AUDREC_UP_PACKET_READY_MSG 0x0002
#define AUDREC_UP_PACKET_READY_MSG_LEN	\
	sizeof(struct audrec_up_pkt_ready_msg)

struct  audrec_up_pkt_ready_msg {
	unsigned short audrec_packet_write_cnt_lsw;
	unsigned short audrec_packet_write_cnt_msw;
	unsigned short audrec_up_prev_read_cnt_lsw;
	unsigned short audrec_up_prev_read_cnt_msw;
} __attribute__((packed));


#define AUDREC_CMD_MEM_CFG_DONE_MSG 0x0003



#endif 
