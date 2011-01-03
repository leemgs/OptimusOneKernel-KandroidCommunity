



struct be_sge {
	u32 pa_lo;
	u32 pa_hi;
	u32 len;
};

#define MCC_WRB_EMBEDDED_MASK	1 	
#define MCC_WRB_SGE_CNT_SHIFT	3	
#define MCC_WRB_SGE_CNT_MASK	0x1F	
struct be_mcc_wrb {
	u32 embedded;		
	u32 payload_length;	
	u32 tag0;		
	u32 tag1;		
	u32 rsvd;		
	union {
		u8 embedded_payload[236]; 
		struct be_sge sgl[19];    
	} payload;
};

#define CQE_FLAGS_VALID_MASK 		(1 << 31)
#define CQE_FLAGS_ASYNC_MASK 		(1 << 30)
#define CQE_FLAGS_COMPLETED_MASK 	(1 << 28)
#define CQE_FLAGS_CONSUMED_MASK 	(1 << 27)


enum {
	MCC_STATUS_SUCCESS = 0x0,

	MCC_STATUS_INSUFFICIENT_PRIVILEGES = 0x1,

	MCC_STATUS_INVALID_PARAMETER = 0x2,

	MCC_STATUS_INSUFFICIENT_RESOURCES = 0x3,

	MCC_STATUS_QUEUE_FLUSHING = 0x4,

	MCC_STATUS_DMA_FAILED = 0x5,
	MCC_STATUS_NOT_SUPPORTED = 66
};

#define CQE_STATUS_COMPL_MASK		0xFFFF
#define CQE_STATUS_COMPL_SHIFT		0	
#define CQE_STATUS_EXTD_MASK		0xFFFF
#define CQE_STATUS_EXTD_SHIFT		16	

struct be_mcc_compl {
	u32 status;		
	u32 tag0;		
	u32 tag1;		
	u32 flags;		
};


#define ASYNC_TRAILER_EVENT_CODE_SHIFT	8	
#define ASYNC_TRAILER_EVENT_CODE_MASK	0xFF
#define ASYNC_EVENT_CODE_LINK_STATE	0x1
struct be_async_event_trailer {
	u32 code;
};

enum {
	ASYNC_EVENT_LINK_DOWN 	= 0x0,
	ASYNC_EVENT_LINK_UP 	= 0x1
};


struct be_async_event_link_state {
	u8 physical_port;
	u8 port_link_status;
	u8 port_duplex;
	u8 port_speed;
	u8 port_fault;
	u8 rsvd0[7];
	struct be_async_event_trailer trailer;
} __packed;

struct be_mcc_mailbox {
	struct be_mcc_wrb wrb;
	struct be_mcc_compl compl;
};

#define CMD_SUBSYSTEM_COMMON	0x1
#define CMD_SUBSYSTEM_ETH 	0x3

#define OPCODE_COMMON_NTWK_MAC_QUERY			1
#define OPCODE_COMMON_NTWK_MAC_SET			2
#define OPCODE_COMMON_NTWK_MULTICAST_SET		3
#define OPCODE_COMMON_NTWK_VLAN_CONFIG  		4
#define OPCODE_COMMON_NTWK_LINK_STATUS_QUERY		5
#define OPCODE_COMMON_WRITE_FLASHROM			7
#define OPCODE_COMMON_CQ_CREATE				12
#define OPCODE_COMMON_EQ_CREATE				13
#define OPCODE_COMMON_MCC_CREATE        		21
#define OPCODE_COMMON_NTWK_RX_FILTER    		34
#define OPCODE_COMMON_GET_FW_VERSION			35
#define OPCODE_COMMON_SET_FLOW_CONTROL			36
#define OPCODE_COMMON_GET_FLOW_CONTROL			37
#define OPCODE_COMMON_SET_FRAME_SIZE			39
#define OPCODE_COMMON_MODIFY_EQ_DELAY			41
#define OPCODE_COMMON_FIRMWARE_CONFIG			42
#define OPCODE_COMMON_NTWK_INTERFACE_CREATE 		50
#define OPCODE_COMMON_NTWK_INTERFACE_DESTROY 		51
#define OPCODE_COMMON_MCC_DESTROY        		53
#define OPCODE_COMMON_CQ_DESTROY        		54
#define OPCODE_COMMON_EQ_DESTROY        		55
#define OPCODE_COMMON_QUERY_FIRMWARE_CONFIG		58
#define OPCODE_COMMON_NTWK_PMAC_ADD			59
#define OPCODE_COMMON_NTWK_PMAC_DEL			60
#define OPCODE_COMMON_FUNCTION_RESET			61

#define OPCODE_ETH_ACPI_CONFIG				2
#define OPCODE_ETH_PROMISCUOUS				3
#define OPCODE_ETH_GET_STATISTICS			4
#define OPCODE_ETH_TX_CREATE				7
#define OPCODE_ETH_RX_CREATE            		8
#define OPCODE_ETH_TX_DESTROY           		9
#define OPCODE_ETH_RX_DESTROY           		10

struct be_cmd_req_hdr {
	u8 opcode;		
	u8 subsystem;		
	u8 port_number;		
	u8 domain;		
	u32 timeout;		
	u32 request_length;	
	u8 version;		
	u8 rsvd[3];		
};

#define RESP_HDR_INFO_OPCODE_SHIFT	0	
#define RESP_HDR_INFO_SUBSYS_SHIFT	8 	
struct be_cmd_resp_hdr {
	u32 info;		
	u32 status;		
	u32 response_length;	
	u32 actual_resp_len;	
};

struct phys_addr {
	u32 lo;
	u32 hi;
};




struct amap_eq_context {
	u8 cidx[13];		
	u8 rsvd0[3];		
	u8 epidx[13];		
	u8 valid;		
	u8 rsvd1;		
	u8 size;		
	u8 pidx[13];		
	u8 rsvd2[3];		
	u8 pd[10];		
	u8 count[3];		
	u8 solevent;		
	u8 stalled;		
	u8 armed;		
	u8 rsvd3[4];		
	u8 func[8];		
	u8 rsvd4;		
	u8 delaymult[10];	
	u8 rsvd5[2];		
	u8 phase[2];		
	u8 nodelay;		
	u8 rsvd6[4];		
	u8 rsvd7[32];		
} __packed;

struct be_cmd_req_eq_create {
	struct be_cmd_req_hdr hdr;
	u16 num_pages;		
	u16 rsvd0;		
	u8 context[sizeof(struct amap_eq_context) / 8];
	struct phys_addr pages[8];
} __packed;

struct be_cmd_resp_eq_create {
	struct be_cmd_resp_hdr resp_hdr;
	u16 eq_id;		
	u16 rsvd0;		
} __packed;


enum {
	MAC_ADDRESS_TYPE_STORAGE = 0x0,
	MAC_ADDRESS_TYPE_NETWORK = 0x1,
	MAC_ADDRESS_TYPE_PD = 0x2,
	MAC_ADDRESS_TYPE_MANAGEMENT = 0x3
};

struct mac_addr {
	u16 size_of_struct;
	u8 addr[ETH_ALEN];
} __packed;

struct be_cmd_req_mac_query {
	struct be_cmd_req_hdr hdr;
	u8 type;
	u8 permanent;
	u16 if_id;
} __packed;

struct be_cmd_resp_mac_query {
	struct be_cmd_resp_hdr hdr;
	struct mac_addr mac;
};


struct be_cmd_req_pmac_add {
	struct be_cmd_req_hdr hdr;
	u32 if_id;
	u8 mac_address[ETH_ALEN];
	u8 rsvd0[2];
} __packed;

struct be_cmd_resp_pmac_add {
	struct be_cmd_resp_hdr hdr;
	u32 pmac_id;
};


struct be_cmd_req_pmac_del {
	struct be_cmd_req_hdr hdr;
	u32 if_id;
	u32 pmac_id;
};



struct amap_cq_context {
	u8 cidx[11];		
	u8 rsvd0;		
	u8 coalescwm[2];	
	u8 nodelay;		
	u8 epidx[11];		
	u8 rsvd1;		
	u8 count[2];		
	u8 valid;		
	u8 solevent;		
	u8 eventable;		
	u8 pidx[11];		
	u8 rsvd2;		
	u8 pd[10];		
	u8 eqid[8];		
	u8 stalled;		
	u8 armed;		
	u8 rsvd3[4];		
	u8 func[8];		
	u8 rsvd4[20];		
	u8 rsvd5[32];		
} __packed;

struct be_cmd_req_cq_create {
	struct be_cmd_req_hdr hdr;
	u16 num_pages;
	u16 rsvd0;
	u8 context[sizeof(struct amap_cq_context) / 8];
	struct phys_addr pages[8];
} __packed;

struct be_cmd_resp_cq_create {
	struct be_cmd_resp_hdr hdr;
	u16 cq_id;
	u16 rsvd0;
} __packed;



struct amap_mcc_context {
	u8 con_index[14];
	u8 rsvd0[2];
	u8 ring_size[4];
	u8 fetch_wrb;
	u8 fetch_r2t;
	u8 cq_id[10];
	u8 prod_index[14];
	u8 fid[8];
	u8 pdid[9];
	u8 valid;
	u8 rsvd1[32];
	u8 rsvd2[32];
} __packed;

struct be_cmd_req_mcc_create {
	struct be_cmd_req_hdr hdr;
	u16 num_pages;
	u16 rsvd0;
	u8 context[sizeof(struct amap_mcc_context) / 8];
	struct phys_addr pages[8];
} __packed;

struct be_cmd_resp_mcc_create {
	struct be_cmd_resp_hdr hdr;
	u16 id;
	u16 rsvd0;
} __packed;


#define BE_ETH_TX_RING_TYPE_STANDARD    	2
#define BE_ULP1_NUM				1


struct amap_tx_context {
	u8 rsvd0[16];		
	u8 tx_ring_size[4];	
	u8 rsvd1[26];		
	u8 pci_func_id[8];	
	u8 rsvd2[9];		
	u8 ctx_valid;		
	u8 cq_id_send[16];	
	u8 rsvd3[16];		
	u8 rsvd4[32];		
	u8 rsvd5[32];		
	u8 rsvd6[32];		
	u8 rsvd7[32];		
	u8 rsvd8[32];		
	u8 rsvd9[32];		
	u8 rsvd10[32];		
	u8 rsvd11[32];		
	u8 rsvd12[32];		
	u8 rsvd13[32];		
	u8 rsvd14[32];		
	u8 rsvd15[32];		
	u8 rsvd16[32];		
} __packed;

struct be_cmd_req_eth_tx_create {
	struct be_cmd_req_hdr hdr;
	u8 num_pages;
	u8 ulp_num;
	u8 type;
	u8 bound_port;
	u8 context[sizeof(struct amap_tx_context) / 8];
	struct phys_addr pages[8];
} __packed;

struct be_cmd_resp_eth_tx_create {
	struct be_cmd_resp_hdr hdr;
	u16 cid;
	u16 rsvd0;
} __packed;


struct be_cmd_req_eth_rx_create {
	struct be_cmd_req_hdr hdr;
	u16 cq_id;
	u8 frag_size;
	u8 num_pages;
	struct phys_addr pages[2];
	u32 interface_id;
	u16 max_frame_size;
	u16 rsvd0;
	u32 rss_queue;
} __packed;

struct be_cmd_resp_eth_rx_create {
	struct be_cmd_resp_hdr hdr;
	u16 id;
	u8 cpu_id;
	u8 rsvd0;
} __packed;



enum {
	QTYPE_EQ = 1,
	QTYPE_CQ,
	QTYPE_TXQ,
	QTYPE_RXQ,
	QTYPE_MCCQ
};

struct be_cmd_req_q_destroy {
	struct be_cmd_req_hdr hdr;
	u16 id;
	u16 bypass_flush;	
} __packed;




enum be_if_flags {
	BE_IF_FLAGS_RSS = 0x4,
	BE_IF_FLAGS_PROMISCUOUS = 0x8,
	BE_IF_FLAGS_BROADCAST = 0x10,
	BE_IF_FLAGS_UNTAGGED = 0x20,
	BE_IF_FLAGS_ULP = 0x40,
	BE_IF_FLAGS_VLAN_PROMISCUOUS = 0x80,
	BE_IF_FLAGS_VLAN = 0x100,
	BE_IF_FLAGS_MCAST_PROMISCUOUS = 0x200,
	BE_IF_FLAGS_PASS_L2_ERRORS = 0x400,
	BE_IF_FLAGS_PASS_L3L4_ERRORS = 0x800
};


struct be_cmd_req_if_create {
	struct be_cmd_req_hdr hdr;
	u32 version;		
	u32 capability_flags;
	u32 enable_flags;
	u8 mac_addr[ETH_ALEN];
	u8 rsvd0;
	u8 pmac_invalid; 
	u32 vlan_tag;	 
} __packed;

struct be_cmd_resp_if_create {
	struct be_cmd_resp_hdr hdr;
	u32 interface_id;
	u32 pmac_id;
};


struct be_cmd_req_if_destroy {
	struct be_cmd_req_hdr hdr;
	u32 interface_id;
};


struct be_port_rxf_stats {
	u32 rx_bytes_lsd;	
	u32 rx_bytes_msd;	
	u32 rx_total_frames;	
	u32 rx_unicast_frames;	
	u32 rx_multicast_frames;	
	u32 rx_broadcast_frames;	
	u32 rx_crc_errors;	
	u32 rx_alignment_symbol_errors;	
	u32 rx_pause_frames;	
	u32 rx_control_frames;	
	u32 rx_in_range_errors;	
	u32 rx_out_range_errors;	
	u32 rx_frame_too_long;	
	u32 rx_address_match_errors;	
	u32 rx_vlan_mismatch;	
	u32 rx_dropped_too_small;	
	u32 rx_dropped_too_short;	
	u32 rx_dropped_header_too_small;	
	u32 rx_dropped_tcp_length;	
	u32 rx_dropped_runt;	
	u32 rx_64_byte_packets;	
	u32 rx_65_127_byte_packets;	
	u32 rx_128_256_byte_packets;	
	u32 rx_256_511_byte_packets;	
	u32 rx_512_1023_byte_packets;	
	u32 rx_1024_1518_byte_packets;	
	u32 rx_1519_2047_byte_packets;	
	u32 rx_2048_4095_byte_packets;	
	u32 rx_4096_8191_byte_packets;	
	u32 rx_8192_9216_byte_packets;	
	u32 rx_ip_checksum_errs;	
	u32 rx_tcp_checksum_errs;	
	u32 rx_udp_checksum_errs;	
	u32 rx_non_rss_packets;	
	u32 rx_ipv4_packets;	
	u32 rx_ipv6_packets;	
	u32 rx_ipv4_bytes_lsd;	
	u32 rx_ipv4_bytes_msd;	
	u32 rx_ipv6_bytes_lsd;	
	u32 rx_ipv6_bytes_msd;	
	u32 rx_chute1_packets;	
	u32 rx_chute2_packets;	
	u32 rx_chute3_packets;	
	u32 rx_management_packets;	
	u32 rx_switched_unicast_packets;	
	u32 rx_switched_multicast_packets;	
	u32 rx_switched_broadcast_packets;	
	u32 tx_bytes_lsd;	
	u32 tx_bytes_msd;	
	u32 tx_unicastframes;	
	u32 tx_multicastframes;	
	u32 tx_broadcastframes;	
	u32 tx_pauseframes;	
	u32 tx_controlframes;	
	u32 tx_64_byte_packets;	
	u32 tx_65_127_byte_packets;	
	u32 tx_128_256_byte_packets;	
	u32 tx_256_511_byte_packets;	
	u32 tx_512_1023_byte_packets;	
	u32 tx_1024_1518_byte_packets;	
	u32 tx_1519_2047_byte_packets;	
	u32 tx_2048_4095_byte_packets;	
	u32 tx_4096_8191_byte_packets;	
	u32 tx_8192_9216_byte_packets;	
	u32 rx_fifo_overflow;	
	u32 rx_input_fifo_overflow;	
};

struct be_rxf_stats {
	struct be_port_rxf_stats port[2];
	u32 rx_drops_no_pbuf;	
	u32 rx_drops_no_txpb;	
	u32 rx_drops_no_erx_descr;	
	u32 rx_drops_no_tpre_descr;	
	u32 management_rx_port_packets;	
	u32 management_rx_port_bytes;	
	u32 management_rx_port_pause_frames;	
	u32 management_rx_port_errors;	
	u32 management_tx_port_packets;	
	u32 management_tx_port_bytes;	
	u32 management_tx_port_pause;	
	u32 management_rx_port_rxfifo_overflow;	
	u32 rx_drops_too_many_frags;	
	u32 rx_drops_invalid_ring;	
	u32 forwarded_packets;	
	u32 rx_drops_mtu;	
	u32 rsvd0[15];
};

struct be_erx_stats {
	u32 rx_drops_no_fragments[44];     
	u32 debug_wdma_sent_hold;          
	u32 debug_wdma_pbfree_sent_hold;   
	u32 debug_wdma_zerobyte_pbfree_sent_hold; 
	u32 debug_pmem_pbuf_dealloc;       
};

struct be_hw_stats {
	struct be_rxf_stats rxf;
	u32 rsvd[48];
	struct be_erx_stats erx;
};

struct be_cmd_req_get_stats {
	struct be_cmd_req_hdr hdr;
	u8 rsvd[sizeof(struct be_hw_stats)];
};

struct be_cmd_resp_get_stats {
	struct be_cmd_resp_hdr hdr;
	struct be_hw_stats hw_stats;
};

struct be_cmd_req_vlan_config {
	struct be_cmd_req_hdr hdr;
	u8 interface_id;
	u8 promiscuous;
	u8 untagged;
	u8 num_vlan;
	u16 normal_vlan[64];
} __packed;

struct be_cmd_req_promiscuous_config {
	struct be_cmd_req_hdr hdr;
	u8 port0_promiscuous;
	u8 port1_promiscuous;
	u16 rsvd0;
} __packed;

struct macaddr {
	u8 byte[ETH_ALEN];
};

struct be_cmd_req_mcast_mac_config {
	struct be_cmd_req_hdr hdr;
	u16 num_mac;
	u8 promiscuous;
	u8 interface_id;
	struct macaddr mac[32];
} __packed;

static inline struct be_hw_stats *
hw_stats_from_cmd(struct be_cmd_resp_get_stats *cmd)
{
	return &cmd->hw_stats;
}


struct be_cmd_req_link_status {
	struct be_cmd_req_hdr hdr;
	u32 rsvd;
};

enum {
	PHY_LINK_DUPLEX_NONE = 0x0,
	PHY_LINK_DUPLEX_HALF = 0x1,
	PHY_LINK_DUPLEX_FULL = 0x2
};

enum {
	PHY_LINK_SPEED_ZERO = 0x0, 	
	PHY_LINK_SPEED_10MBPS = 0x1,
	PHY_LINK_SPEED_100MBPS = 0x2,
	PHY_LINK_SPEED_1GBPS = 0x3,
	PHY_LINK_SPEED_10GBPS = 0x4
};

struct be_cmd_resp_link_status {
	struct be_cmd_resp_hdr hdr;
	u8 physical_port;
	u8 mac_duplex;
	u8 mac_speed;
	u8 mac_fault;
	u8 mgmt_mac_duplex;
	u8 mgmt_mac_speed;
	u16 rsvd0;
} __packed;


struct be_cmd_req_get_fw_version {
	struct be_cmd_req_hdr hdr;
	u8 rsvd0[FW_VER_LEN];
	u8 rsvd1[FW_VER_LEN];
} __packed;

struct be_cmd_resp_get_fw_version {
	struct be_cmd_resp_hdr hdr;
	u8 firmware_version_string[FW_VER_LEN];
	u8 fw_on_flash_version_string[FW_VER_LEN];
} __packed;


struct be_cmd_req_set_flow_control {
	struct be_cmd_req_hdr hdr;
	u16 tx_flow_control;
	u16 rx_flow_control;
} __packed;


struct be_cmd_req_get_flow_control {
	struct be_cmd_req_hdr hdr;
	u32 rsvd;
};

struct be_cmd_resp_get_flow_control {
	struct be_cmd_resp_hdr hdr;
	u16 tx_flow_control;
	u16 rx_flow_control;
} __packed;


struct be_cmd_req_modify_eq_delay {
	struct be_cmd_req_hdr hdr;
	u32 num_eq;
	struct {
		u32 eq_id;
		u32 phase;
		u32 delay_multiplier;
	} delay[8];
} __packed;

struct be_cmd_resp_modify_eq_delay {
	struct be_cmd_resp_hdr hdr;
	u32 rsvd0;
} __packed;


struct be_cmd_req_query_fw_cfg {
	struct be_cmd_req_hdr hdr;
	u32 rsvd[30];
};

struct be_cmd_resp_query_fw_cfg {
	struct be_cmd_resp_hdr hdr;
	u32 be_config_number;
	u32 asic_revision;
	u32 phys_port;
	u32 function_cap;
	u32 rsvd[26];
};


struct flashrom_params {
	u32 op_code;
	u32 op_type;
	u32 data_buf_size;
	u32 offset;
	u8 data_buf[4];
};

struct be_cmd_write_flashrom {
	struct be_cmd_req_hdr hdr;
	struct flashrom_params params;
};

extern int be_pci_fnum_get(struct be_adapter *adapter);
extern int be_cmd_POST(struct be_adapter *adapter);
extern int be_cmd_mac_addr_query(struct be_adapter *adapter, u8 *mac_addr,
			u8 type, bool permanent, u32 if_handle);
extern int be_cmd_pmac_add(struct be_adapter *adapter, u8 *mac_addr,
			u32 if_id, u32 *pmac_id);
extern int be_cmd_pmac_del(struct be_adapter *adapter, u32 if_id, u32 pmac_id);
extern int be_cmd_if_create(struct be_adapter *adapter, u32 cap_flags,
			u32 en_flags, u8 *mac, bool pmac_invalid,
			u32 *if_handle, u32 *pmac_id);
extern int be_cmd_if_destroy(struct be_adapter *adapter, u32 if_handle);
extern int be_cmd_eq_create(struct be_adapter *adapter,
			struct be_queue_info *eq, int eq_delay);
extern int be_cmd_cq_create(struct be_adapter *adapter,
			struct be_queue_info *cq, struct be_queue_info *eq,
			bool sol_evts, bool no_delay,
			int num_cqe_dma_coalesce);
extern int be_cmd_mccq_create(struct be_adapter *adapter,
			struct be_queue_info *mccq,
			struct be_queue_info *cq);
extern int be_cmd_txq_create(struct be_adapter *adapter,
			struct be_queue_info *txq,
			struct be_queue_info *cq);
extern int be_cmd_rxq_create(struct be_adapter *adapter,
			struct be_queue_info *rxq, u16 cq_id,
			u16 frag_size, u16 max_frame_size, u32 if_id,
			u32 rss);
extern int be_cmd_q_destroy(struct be_adapter *adapter, struct be_queue_info *q,
			int type);
extern int be_cmd_link_status_query(struct be_adapter *adapter,
			bool *link_up);
extern int be_cmd_reset(struct be_adapter *adapter);
extern int be_cmd_get_stats(struct be_adapter *adapter,
			struct be_dma_mem *nonemb_cmd);
extern int be_cmd_get_fw_ver(struct be_adapter *adapter, char *fw_ver);

extern int be_cmd_modify_eqd(struct be_adapter *adapter, u32 eq_id, u32 eqd);
extern int be_cmd_vlan_config(struct be_adapter *adapter, u32 if_id,
			u16 *vtag_array, u32 num, bool untagged,
			bool promiscuous);
extern int be_cmd_promiscuous_config(struct be_adapter *adapter,
			u8 port_num, bool en);
extern int be_cmd_multicast_set(struct be_adapter *adapter, u32 if_id,
			struct dev_mc_list *mc_list, u32 mc_count);
extern int be_cmd_set_flow_control(struct be_adapter *adapter,
			u32 tx_fc, u32 rx_fc);
extern int be_cmd_get_flow_control(struct be_adapter *adapter,
			u32 *tx_fc, u32 *rx_fc);
extern int be_cmd_query_fw_cfg(struct be_adapter *adapter,
			u32 *port_num, u32 *cap);
extern int be_cmd_reset_function(struct be_adapter *adapter);
extern int be_process_mcc(struct be_adapter *adapter);
extern int be_cmd_write_flashrom(struct be_adapter *adapter,
			struct be_dma_mem *cmd, u32 flash_oper,
			u32 flash_opcode, u32 buf_size);
