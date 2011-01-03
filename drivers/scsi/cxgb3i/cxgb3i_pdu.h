

#ifndef __CXGB3I_ULP2_PDU_H__
#define __CXGB3I_ULP2_PDU_H__

struct cpl_iscsi_hdr_norss {
	union opcode_tid ot;
	u16 pdu_len_ddp;
	u16 len;
	u32 seq;
	u16 urg;
	u8 rsvd;
	u8 status;
};

struct cpl_rx_data_ddp_norss {
	union opcode_tid ot;
	u16 urg;
	u16 len;
	u32 seq;
	u32 nxt_seq;
	u32 ulp_crc;
	u32 ddp_status;
};

#define RX_DDP_STATUS_IPP_SHIFT		27	
#define RX_DDP_STATUS_TID_SHIFT		26	
#define RX_DDP_STATUS_COLOR_SHIFT	25	
#define RX_DDP_STATUS_OFFSET_SHIFT	24	
#define RX_DDP_STATUS_ULIMIT_SHIFT	23	
#define RX_DDP_STATUS_TAG_SHIFT		22	
#define RX_DDP_STATUS_DCRC_SHIFT	21	
#define RX_DDP_STATUS_HCRC_SHIFT	20	
#define RX_DDP_STATUS_PAD_SHIFT		19	
#define RX_DDP_STATUS_PPP_SHIFT		18	
#define RX_DDP_STATUS_LLIMIT_SHIFT	17	
#define RX_DDP_STATUS_DDP_SHIFT		16	
#define RX_DDP_STATUS_PMM_SHIFT		15	

#define ULP2_FLAG_DATA_READY		0x1
#define ULP2_FLAG_DATA_DDPED		0x2
#define ULP2_FLAG_HCRC_ERROR		0x10
#define ULP2_FLAG_DCRC_ERROR		0x20
#define ULP2_FLAG_PAD_ERROR		0x40

void cxgb3i_conn_closing(struct s3_conn *c3cn);
void cxgb3i_conn_pdu_ready(struct s3_conn *c3cn);
void cxgb3i_conn_tx_open(struct s3_conn *c3cn);
#endif
