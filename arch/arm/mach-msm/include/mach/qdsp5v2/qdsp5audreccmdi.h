

#ifndef QDSP5AUDRECCMDI_H
#define QDSP5AUDRECCMDI_H







#define AUDREC_CMD_MEM_CFG_CMD 0x0000
#define AUDREC_CMD_ARECMEM_CFG_LEN	\
	sizeof(struct audrec_cmd_arecmem_cfg)

struct audrec_cmd_arecmem_cfg {
	unsigned short cmd_id;
	unsigned short audrec_up_pkt_intm_count;
	unsigned short audrec_ext_pkt_start_addr_msw;
	unsigned short audrec_ext_pkt_start_addr_lsw;
	unsigned short audrec_ext_pkt_buf_number;
} __attribute__((packed));





#define UP_AUDREC_PACKET_EXT_PTR 0x0000
#define UP_AUDREC_PACKET_EXT_PTR_LEN	\
	sizeof(up_audrec_packet_ext_ptr)

struct up_audrec_packet_ext_ptr {
	unsigned short cmd_id;
	unsigned short audrec_up_curr_read_count_lsw;
	unsigned short audrec_up_curr_read_count_msw;
} __attribute__((packed));

#endif 
