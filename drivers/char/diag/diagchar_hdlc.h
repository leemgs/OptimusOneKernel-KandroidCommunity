

#ifndef DIAGCHAR_HDLC
#define DIAGCHAR_HDLC

enum diag_send_state_enum_type {
	DIAG_STATE_START,
	DIAG_STATE_BUSY,
	DIAG_STATE_CRC1,
	DIAG_STATE_CRC2,
	DIAG_STATE_TERM,
	DIAG_STATE_COMPLETE
};

struct diag_send_desc_type {
	const void *pkt;
	const void *last;	
	enum diag_send_state_enum_type state;
	unsigned char terminate;	
};

struct diag_hdlc_dest_type {
	void *dest;
	void *dest_last;
	
	uint16_t crc;
};

struct diag_hdlc_decode_type {
	uint8_t *src_ptr;
	unsigned int src_idx;
	unsigned int src_size;
	uint8_t *dest_ptr;
	unsigned int dest_idx;
	unsigned int dest_size;
	int escaping;

};

void diag_hdlc_encode(struct diag_send_desc_type *src_desc,
		      struct diag_hdlc_dest_type *enc);




#if 1	
void diag_hdlc_encode_mtc(struct diag_send_desc_type* src_desc, struct diag_hdlc_dest_type* enc);
#endif 


int diag_hdlc_decode(struct diag_hdlc_decode_type *hdlc);

#define ESC_CHAR     0x7D
#define CONTROL_CHAR 0x7E
#define ESC_MASK     0x20

#endif
