

#ifndef _QLA4X_FW_H
#define _QLA4X_FW_H


#define MAX_PRST_DEV_DB_ENTRIES		64
#define MIN_DISC_DEV_DB_ENTRY		MAX_PRST_DEV_DB_ENTRIES
#define MAX_DEV_DB_ENTRIES 512



struct port_ctrl_stat_regs {
	__le32 ext_hw_conf;	
	__le32 rsrvd0;		
	__le32 port_ctrl;	
	__le32 port_status;	
	__le32 rsrvd1[32];	
	__le32 gp_out;		
	__le32 gp_in;		
	__le32 rsrvd2[5];	
	__le32 port_err_status; 
};

struct host_mem_cfg_regs {
	__le32 rsrvd0[12];	
	__le32 req_q_out;	
	__le32 rsrvd1[31];	
};


struct isp_reg {
#define MBOX_REG_COUNT 8
	__le32 mailbox[MBOX_REG_COUNT];

	__le32 flash_address;	
	__le32 flash_data;
	__le32 ctrl_status;

	union {
		struct {
			__le32 nvram;
			__le32 reserved1[2]; 
		} __attribute__ ((packed)) isp4010;
		struct {
			__le32 intr_mask;
			__le32 nvram; 
			__le32 semaphore;
		} __attribute__ ((packed)) isp4022;
	} u1;

	__le32 req_q_in;    
	__le32 rsp_q_out;   

	__le32 reserved2[4];	

	union {
		struct {
			__le32 ext_hw_conf; 
			__le32 flow_ctrl;
			__le32 port_ctrl;
			__le32 port_status;

			__le32 reserved3[8]; 

			__le32 req_q_out; 

			__le32 reserved4[23]; 

			__le32 gp_out; 
			__le32 gp_in;

			__le32 reserved5[5];

			__le32 port_err_status; 
		} __attribute__ ((packed)) isp4010;
		struct {
			union {
				struct port_ctrl_stat_regs p0;
				struct host_mem_cfg_regs p1;
			};
		} __attribute__ ((packed)) isp4022;
	} u2;
};				



#define QL4010_DRVR_SEM_BITS	0x00000030
#define QL4010_GPIO_SEM_BITS	0x000000c0
#define QL4010_SDRAM_SEM_BITS	0x00000300
#define QL4010_PHY_SEM_BITS	0x00000c00
#define QL4010_NVRAM_SEM_BITS	0x00003000
#define QL4010_FLASH_SEM_BITS	0x0000c000

#define QL4010_DRVR_SEM_MASK	0x00300000
#define QL4010_GPIO_SEM_MASK	0x00c00000
#define QL4010_SDRAM_SEM_MASK	0x03000000
#define QL4010_PHY_SEM_MASK	0x0c000000
#define QL4010_NVRAM_SEM_MASK	0x30000000
#define QL4010_FLASH_SEM_MASK	0xc0000000


#define QL4022_RESOURCE_MASK_BASE_CODE 0x7
#define QL4022_RESOURCE_BITS_BASE_CODE 0x4


#define QL4022_DRVR_SEM_MASK	(QL4022_RESOURCE_MASK_BASE_CODE << (1+16))
#define QL4022_DDR_RAM_SEM_MASK (QL4022_RESOURCE_MASK_BASE_CODE << (4+16))
#define QL4022_PHY_GIO_SEM_MASK (QL4022_RESOURCE_MASK_BASE_CODE << (7+16))
#define QL4022_NVRAM_SEM_MASK	(QL4022_RESOURCE_MASK_BASE_CODE << (10+16))
#define QL4022_FLASH_SEM_MASK	(QL4022_RESOURCE_MASK_BASE_CODE << (13+16))




#define PORT_CTRL_STAT_PAGE			0	
#define HOST_MEM_CFG_PAGE			1	
#define LOCAL_RAM_CFG_PAGE			2	
#define PROT_STAT_PAGE				3	


static inline uint32_t set_rmask(uint32_t val)
{
	return (val & 0xffff) | (val << 16);
}


static inline uint32_t clr_rmask(uint32_t val)
{
	return 0 | (val << 16);
}


#define CSR_SCSI_PAGE_SELECT			0x00000003
#define CSR_SCSI_INTR_ENABLE			0x00000004	
#define CSR_SCSI_RESET_INTR			0x00000008
#define CSR_SCSI_COMPLETION_INTR		0x00000010
#define CSR_SCSI_PROCESSOR_INTR			0x00000020
#define CSR_INTR_RISC				0x00000040
#define CSR_BOOT_ENABLE				0x00000080
#define CSR_NET_PAGE_SELECT			0x00000300	
#define CSR_FUNC_NUM				0x00000700	
#define CSR_NET_RESET_INTR			0x00000800	
#define CSR_FORCE_SOFT_RESET			0x00002000	
#define CSR_FATAL_ERROR				0x00004000
#define CSR_SOFT_RESET				0x00008000
#define ISP_CONTROL_FN_MASK			CSR_FUNC_NUM
#define ISP_CONTROL_FN0_SCSI			0x0500
#define ISP_CONTROL_FN1_SCSI			0x0700

#define INTR_PENDING				(CSR_SCSI_COMPLETION_INTR |\
						 CSR_SCSI_PROCESSOR_INTR |\
						 CSR_SCSI_RESET_INTR)


#define IMR_SCSI_INTR_ENABLE			0x00000004	


#define NVR_WRITE_ENABLE			0x00000010	






#define GPOR_TOPCAT_RESET			0x00000004


struct shadow_regs {
	
	__le32 req_q_out;	

	
	__le32 rsp_q_in;	
};		  



union external_hw_config_reg {
	struct {
		
		__le32 bReserved0:1;
		__le32 bSDRAMProtectionMethod:2;
		__le32 bSDRAMBanks:1;
		__le32 bSDRAMChipWidth:1;
		__le32 bSDRAMChipSize:2;
		__le32 bParityDisable:1;
		__le32 bExternalMemoryType:1;
		__le32 bFlashBIOSWriteEnable:1;
		__le32 bFlashUpperBankSelect:1;
		__le32 bWriteBurst:2;
		__le32 bReserved1:3;
		__le32 bMask:16;
	};
	uint32_t Asuint32_t;
};




#define MBOX_CMD_ABOUT_FW			0x0009
#define MBOX_CMD_PING				0x000B
#define MBOX_CMD_LUN_RESET			0x0016
#define MBOX_CMD_TARGET_WARM_RESET		0x0017
#define MBOX_CMD_GET_MANAGEMENT_DATA		0x001E
#define MBOX_CMD_GET_FW_STATUS			0x001F
#define MBOX_CMD_SET_ISNS_SERVICE		0x0021
#define ISNS_DISABLE				0
#define ISNS_ENABLE				1
#define MBOX_CMD_COPY_FLASH			0x0024
#define MBOX_CMD_WRITE_FLASH			0x0025
#define MBOX_CMD_READ_FLASH			0x0026
#define MBOX_CMD_CLEAR_DATABASE_ENTRY		0x0031
#define MBOX_CMD_CONN_CLOSE_SESS_LOGOUT		0x0056
#define LOGOUT_OPTION_CLOSE_SESSION		0x01
#define LOGOUT_OPTION_RELOGIN			0x02
#define MBOX_CMD_EXECUTE_IOCB_A64		0x005A
#define MBOX_CMD_INITIALIZE_FIRMWARE		0x0060
#define MBOX_CMD_GET_INIT_FW_CTRL_BLOCK		0x0061
#define MBOX_CMD_REQUEST_DATABASE_ENTRY		0x0062
#define MBOX_CMD_SET_DATABASE_ENTRY		0x0063
#define MBOX_CMD_GET_DATABASE_ENTRY		0x0064
#define DDB_DS_UNASSIGNED			0x00
#define DDB_DS_NO_CONNECTION_ACTIVE		0x01
#define DDB_DS_SESSION_ACTIVE			0x04
#define DDB_DS_SESSION_FAILED			0x06
#define DDB_DS_LOGIN_IN_PROCESS			0x07
#define MBOX_CMD_GET_FW_STATE			0x0069
#define MBOX_CMD_GET_INIT_FW_CTRL_BLOCK_DEFAULTS 0x006A
#define MBOX_CMD_RESTORE_FACTORY_DEFAULTS	0x0087
#define MBOX_CMD_SET_ACB			0x0088
#define MBOX_CMD_GET_ACB			0x0089
#define MBOX_CMD_DISABLE_ACB			0x008A
#define MBOX_CMD_GET_IPV6_NEIGHBOR_CACHE	0x008B
#define MBOX_CMD_GET_IPV6_DEST_CACHE		0x008C
#define MBOX_CMD_GET_IPV6_DEF_ROUTER_LIST	0x008D
#define MBOX_CMD_GET_IPV6_LCL_PREFIX_LIST	0x008E
#define MBOX_CMD_SET_IPV6_NEIGHBOR_CACHE	0x0090
#define MBOX_CMD_GET_IP_ADDR_STATE		0x0091
#define MBOX_CMD_SEND_IPV6_ROUTER_SOL		0x0092
#define MBOX_CMD_GET_DB_ENTRY_CURRENT_IP_ADDR	0x0093


#define FW_STATE_READY				0x0000
#define FW_STATE_CONFIG_WAIT			0x0001
#define FW_STATE_WAIT_LOGIN			0x0002
#define FW_STATE_ERROR				0x0004
#define FW_STATE_DHCP_IN_PROGRESS		0x0008


#define FW_ADDSTATE_OPTICAL_MEDIA		0x0001
#define FW_ADDSTATE_DHCP_ENABLED		0x0002
#define FW_ADDSTATE_LINK_UP			0x0010
#define FW_ADDSTATE_ISNS_SVC_ENABLED		0x0020
#define MBOX_CMD_GET_DATABASE_ENTRY_DEFAULTS	0x006B
#define MBOX_CMD_CONN_OPEN_SESS_LOGIN		0x0074
#define MBOX_CMD_GET_CRASH_RECORD		0x0076	
#define MBOX_CMD_GET_CONN_EVENT_LOG		0x0077


#define MBOX_COMPLETION_STATUS			4
#define MBOX_STS_BUSY				0x0007
#define MBOX_STS_INTERMEDIATE_COMPLETION	0x1000
#define MBOX_STS_COMMAND_COMPLETE		0x4000
#define MBOX_STS_COMMAND_ERROR			0x4005

#define MBOX_ASYNC_EVENT_STATUS			8
#define MBOX_ASTS_SYSTEM_ERROR			0x8002
#define MBOX_ASTS_REQUEST_TRANSFER_ERROR	0x8003
#define MBOX_ASTS_RESPONSE_TRANSFER_ERROR	0x8004
#define MBOX_ASTS_PROTOCOL_STATISTIC_ALARM	0x8005
#define MBOX_ASTS_SCSI_COMMAND_PDU_REJECTED	0x8006
#define MBOX_ASTS_LINK_UP			0x8010
#define MBOX_ASTS_LINK_DOWN			0x8011
#define MBOX_ASTS_DATABASE_CHANGED		0x8014
#define MBOX_ASTS_UNSOLICITED_PDU_RECEIVED	0x8015
#define MBOX_ASTS_SELF_TEST_FAILED		0x8016
#define MBOX_ASTS_LOGIN_FAILED			0x8017
#define MBOX_ASTS_DNS				0x8018
#define MBOX_ASTS_HEARTBEAT			0x8019
#define MBOX_ASTS_NVRAM_INVALID			0x801A
#define MBOX_ASTS_MAC_ADDRESS_CHANGED		0x801B
#define MBOX_ASTS_IP_ADDRESS_CHANGED		0x801C
#define MBOX_ASTS_DHCP_LEASE_EXPIRED		0x801D
#define MBOX_ASTS_DHCP_LEASE_ACQUIRED		0x801F
#define MBOX_ASTS_ISNS_UNSOLICITED_PDU_RECEIVED 0x8021
#define MBOX_ASTS_DUPLICATE_IP			0x8025
#define MBOX_ASTS_ARP_COMPLETE			0x8026
#define MBOX_ASTS_SUBNET_STATE_CHANGE		0x8027
#define MBOX_ASTS_RESPONSE_QUEUE_FULL		0x8028
#define MBOX_ASTS_IP_ADDR_STATE_CHANGED		0x8029
#define MBOX_ASTS_IPV6_PREFIX_EXPIRED		0x802B
#define MBOX_ASTS_IPV6_ND_PREFIX_IGNORED	0x802C
#define MBOX_ASTS_IPV6_LCL_PREFIX_IGNORED	0x802D
#define MBOX_ASTS_ICMPV6_ERROR_MSG_RCVD		0x802E

#define ISNS_EVENT_DATA_RECEIVED		0x0000
#define ISNS_EVENT_CONNECTION_OPENED		0x0001
#define ISNS_EVENT_CONNECTION_FAILED		0x0002
#define MBOX_ASTS_IPSEC_SYSTEM_FATAL_ERROR	0x8022
#define MBOX_ASTS_SUBNET_STATE_CHANGE		0x8027




struct addr_ctrl_blk {
	uint8_t version;	
	uint8_t control;	

	uint16_t fw_options;	
#define	 FWOPT_HEARTBEAT_ENABLE		  0x1000
#define	 FWOPT_SESSION_MODE		  0x0040
#define	 FWOPT_INITIATOR_MODE		  0x0020
#define	 FWOPT_TARGET_MODE		  0x0010

	uint16_t exec_throttle;	
	uint8_t zio_count;	
	uint8_t res0;	
	uint16_t eth_mtu_size;	
	uint16_t add_fw_options;	

	uint8_t hb_interval;	
	uint8_t inst_num; 
	uint16_t res1;		
	uint16_t rqq_consumer_idx;	
	uint16_t compq_producer_idx;	
	uint16_t rqq_len;	
	uint16_t compq_len;	
	uint32_t rqq_addr_lo;	
	uint32_t rqq_addr_hi;	
	uint32_t compq_addr_lo;	
	uint32_t compq_addr_hi;	
	uint32_t shdwreg_addr_lo;	
	uint32_t shdwreg_addr_hi;	

	uint16_t iscsi_opts;	
	uint16_t ipv4_tcp_opts;	
	uint16_t ipv4_ip_opts;	

	uint16_t iscsi_max_pdu_size;	
	uint8_t ipv4_tos;	
	uint8_t ipv4_ttl;	
	uint8_t acb_version;	
	uint8_t res2;	
	uint16_t def_timeout;	
	uint16_t iscsi_fburst_len;	
	uint16_t iscsi_def_time2wait;	
	uint16_t iscsi_def_time2retain;	
	uint16_t iscsi_max_outstnd_r2t;	
	uint16_t conn_ka_timeout;	
	uint16_t ipv4_port;	
	uint16_t iscsi_max_burst_len;	
	uint32_t res5;		
	uint8_t ipv4_addr[4];	
	uint16_t ipv4_vlan_tag;	
	uint8_t ipv4_addr_state;	
	uint8_t ipv4_cacheid;	
	uint8_t res6[8];	
	uint8_t ipv4_subnet[4];	
	uint8_t res7[12];	
	uint8_t ipv4_gw_addr[4];	
	uint8_t res8[0xc];	
	uint8_t pri_dns_srvr_ip[4];
	uint8_t sec_dns_srvr_ip[4];
	uint16_t min_eph_port;	
	uint16_t max_eph_port;	
	uint8_t res9[4];	
	uint8_t iscsi_alias[32];
	uint8_t res9_1[0x16];	
	uint16_t tgt_portal_grp;
	uint8_t abort_timer;	
	uint8_t ipv4_tcp_wsf;	
	uint8_t res10[6];	
	uint8_t ipv4_sec_ip_addr[4];	
	uint8_t ipv4_dhcp_vid_len;	
	uint8_t ipv4_dhcp_vid[11];	
	uint8_t res11[20];	
	uint8_t ipv4_dhcp_alt_cid_len;	
	uint8_t ipv4_dhcp_alt_cid[11];	
	uint8_t iscsi_name[224];	
	uint8_t res12[32];	
	uint32_t cookie;	
	uint16_t ipv6_port;	
	uint16_t ipv6_opts;	
	uint16_t ipv6_addtl_opts;	
	uint16_t ipv6_tcp_opts;	
	uint8_t ipv6_tcp_wsf;	
	uint16_t ipv6_flow_lbl;	
	uint8_t ipv6_gw_addr[16];	
	uint16_t ipv6_vlan_tag;	
	uint8_t ipv6_lnk_lcl_addr_state;
	uint8_t ipv6_addr0_state;	
	uint8_t ipv6_addr1_state;	
	uint8_t ipv6_gw_state;	
	uint8_t ipv6_traffic_class;	
	uint8_t ipv6_hop_limit;	
	uint8_t ipv6_if_id[8];	
	uint8_t ipv6_addr0[16];	
	uint8_t ipv6_addr1[16];	
	uint32_t ipv6_nd_reach_time;	
	uint32_t ipv6_nd_rexmit_timer;	
	uint32_t ipv6_nd_stale_timeout;	
	uint8_t ipv6_dup_addr_detect_count;	
	uint8_t ipv6_cache_id;	
	uint8_t res13[18];	
	uint32_t ipv6_gw_advrt_mtu;	
	uint8_t res14[140];	
};

struct init_fw_ctrl_blk {
	struct addr_ctrl_blk pri;
	struct addr_ctrl_blk sec;
};



struct dev_db_entry {
	uint16_t options;	
#define DDB_OPT_DISC_SESSION  0x10
#define DDB_OPT_TARGET	      0x02 

	uint16_t exec_throttle;	
	uint16_t exec_count;	
	uint16_t res0;	
	uint16_t iscsi_options;	
	uint16_t tcp_options;	
	uint16_t ip_options;	
	uint16_t iscsi_max_rcv_data_seg_len;	
	uint32_t res1;	
	uint16_t iscsi_max_snd_data_seg_len;	
	uint16_t iscsi_first_burst_len;	
	uint16_t iscsi_def_time2wait;	
	uint16_t iscsi_def_time2retain;	
	uint16_t iscsi_max_outsnd_r2t;	
	uint16_t ka_timeout;	
	uint8_t isid[6];	
	uint16_t tsid;		
	uint16_t port;	
	uint16_t iscsi_max_burst_len;	
	uint16_t def_timeout;	
	uint16_t res2;	
	uint8_t ip_addr[0x10];	
	uint8_t iscsi_alias[0x20];	
	uint8_t tgt_addr[0x20];	
	uint16_t mss;	
	uint16_t res3;	
	uint16_t lcl_port;	
	uint8_t ipv4_tos;	
	uint16_t ipv6_flow_lbl;	
	uint8_t res4[0x36];	
	uint8_t iscsi_name[0xE0];	
	uint8_t ipv6_addr[0x10];
	uint8_t res5[0x10];	
	uint16_t ddb_link;	
	uint16_t chap_tbl_idx;	
	uint16_t tgt_portal_grp; 
	uint8_t tcp_xmt_wsf;	
	uint8_t tcp_rcv_wsf;	
	uint32_t stat_sn;	
	uint32_t exp_stat_sn;	
	uint8_t res6[0x30];	
};





#define FLASH_OFFSET_SYS_INFO	0x02000000
#define FLASH_DEFAULTBLOCKSIZE	0x20000
#define FLASH_EOF_OFFSET	(FLASH_DEFAULTBLOCKSIZE-8) 

struct sys_info_phys_addr {
	uint8_t address[6];	
	uint8_t filler[2];	
};

struct flash_sys_info {
	uint32_t cookie;	
	uint32_t physAddrCount; 
	struct sys_info_phys_addr physAddr[4]; 
	uint8_t vendorId[128];	
	uint8_t productId[128]; 
	uint32_t serialNumber;	

	
	uint32_t pciDeviceVendor;	
	uint32_t pciDeviceId;	
	uint32_t pciSubsysVendor;	
	uint32_t pciSubsysId;	

	
	uint32_t crumbs;	

	uint32_t enterpriseNumber;	

	uint32_t mtu;		
	uint32_t reserved0;	
	uint32_t crumbs2;	
	uint8_t acSerialNumber[16];	
	uint32_t crumbs3;	

	
	uint32_t reserved1[39]; 
};	

struct crash_record {
	uint16_t fw_major_version;	
	uint16_t fw_minor_version;	
	uint16_t fw_patch_version;	
	uint16_t fw_build_version;	

	uint8_t build_date[16]; 
	uint8_t build_time[16]; 
	uint8_t build_user[16]; 
	uint8_t card_serial_num[16];	

	uint32_t time_of_crash_in_secs; 
	uint32_t time_of_crash_in_ms;	

	uint16_t out_RISC_sd_num_frames;	
	uint16_t OAP_sd_num_words;	
	uint16_t IAP_sd_num_frames;	
	uint16_t in_RISC_sd_num_words;	

	uint8_t reserved1[28];	

	uint8_t out_RISC_reg_dump[256]; 
	uint8_t in_RISC_reg_dump[256];	
	uint8_t in_out_RISC_stack_dump[0];	
};

struct conn_event_log_entry {
#define MAX_CONN_EVENT_LOG_ENTRIES	100
	uint32_t timestamp_sec; 
	uint32_t timestamp_ms;	
	uint16_t device_index;	
	uint16_t fw_conn_state; 
	uint8_t event_type;	
	uint8_t error_code;	
	uint16_t error_code_detail;	
	uint8_t num_consecutive_events; 
	uint8_t rsvd[3];	
};


#define IOCB_MAX_CDB_LEN	    16	
#define IOCB_MAX_SENSEDATA_LEN	    32	
#define IOCB_MAX_EXT_SENSEDATA_LEN  60  


struct qla4_header {
	uint8_t entryType;
#define ET_STATUS		 0x03
#define ET_MARKER		 0x04
#define ET_CONT_T1		 0x0A
#define ET_STATUS_CONTINUATION	 0x10
#define ET_CMND_T3		 0x19
#define ET_PASSTHRU0		 0x3A
#define ET_PASSTHRU_STATUS	 0x3C

	uint8_t entryStatus;
	uint8_t systemDefined;
	uint8_t entryCount;

	
};


struct queue_entry {
	uint8_t data[60];
	uint32_t signature;

};



#define COMMAND_SEG_A64	  1
#define CONTINUE_SEG_A64  5



struct data_seg_a64 {
	struct {
		uint32_t addrLow;
		uint32_t addrHigh;

	} base;

	uint32_t count;

};



struct command_t3_entry {
	struct qla4_header hdr;	

	uint32_t handle;	
	uint16_t target;	
	uint16_t connection_id; 

	uint8_t control_flags;	

	
#define CF_WRITE		0x20
#define CF_READ			0x40
#define CF_NO_DATA		0x00

	
#define CF_HEAD_TAG		0x03
#define CF_ORDERED_TAG		0x02
#define CF_SIMPLE_TAG		0x01

	
	uint8_t state_flags;	
	uint8_t cmdRefNum;	
	uint8_t reserved1;	
	uint8_t cdb[IOCB_MAX_CDB_LEN];	
	struct scsi_lun lun;	
	uint32_t cmdSeqNum;	
	uint16_t timeout;	
	uint16_t dataSegCnt;	
	uint32_t ttlByteCnt;	
	struct data_seg_a64 dataseg[COMMAND_SEG_A64];	

};



struct continuation_t1_entry {
	struct qla4_header hdr;

	struct data_seg_a64 dataseg[CONTINUE_SEG_A64];

};


#define COMMAND_SEG	COMMAND_SEG_A64
#define CONTINUE_SEG	CONTINUE_SEG_A64

#define ET_COMMAND	ET_CMND_T3
#define ET_CONTINUE	ET_CONT_T1


struct qla4_marker_entry {
	struct qla4_header hdr;	

	uint32_t system_defined; 
	uint16_t target;	
	uint16_t modifier;	
#define MM_LUN_RESET		0
#define MM_TGT_WARM_RESET	1

	uint16_t flags;		
	uint16_t reserved1;	
	struct scsi_lun lun;	
	uint64_t reserved2;	
	uint64_t reserved3;	
	uint64_t reserved4;	
	uint64_t reserved5;	
	uint64_t reserved6;	
};


struct status_entry {
	struct qla4_header hdr;	

	uint32_t handle;	

	uint8_t scsiStatus;	
#define SCSI_CHECK_CONDITION		  0x02

	uint8_t iscsiFlags;	
#define ISCSI_FLAG_RESIDUAL_UNDER	  0x02
#define ISCSI_FLAG_RESIDUAL_OVER	  0x04

	uint8_t iscsiResponse;	

	uint8_t completionStatus;	
#define SCS_COMPLETE			  0x00
#define SCS_INCOMPLETE			  0x01
#define SCS_RESET_OCCURRED		  0x04
#define SCS_ABORTED			  0x05
#define SCS_TIMEOUT			  0x06
#define SCS_DATA_OVERRUN		  0x07
#define SCS_DATA_UNDERRUN		  0x15
#define SCS_QUEUE_FULL			  0x1C
#define SCS_DEVICE_UNAVAILABLE		  0x28
#define SCS_DEVICE_LOGGED_OUT		  0x29

	uint8_t reserved1;	

	
	uint8_t state_flags;	

	uint16_t senseDataByteCnt;	
	uint32_t residualByteCnt;	
	uint32_t bidiResidualByteCnt;	
	uint32_t expSeqNum;	
	uint32_t maxCmdSeqNum;	
	uint8_t senseData[IOCB_MAX_SENSEDATA_LEN];	

};


struct status_cont_entry {
       struct qla4_header hdr; 
       uint8_t ext_sense_data[IOCB_MAX_EXT_SENSEDATA_LEN]; 
};

struct passthru0 {
	struct qla4_header hdr;		       
	uint32_t handle;	
	uint16_t target;	
	uint16_t connectionID;	
#define ISNS_DEFAULT_SERVER_CONN_ID	((uint16_t)0x8000)

	uint16_t controlFlags;	
#define PT_FLAG_ETHERNET_FRAME		0x8000
#define PT_FLAG_ISNS_PDU		0x8000
#define PT_FLAG_SEND_BUFFER		0x0200
#define PT_FLAG_WAIT_4_RESPONSE		0x0100

	uint16_t timeout;	
#define PT_DEFAULT_TIMEOUT		30 

	struct data_seg_a64 outDataSeg64;	
	uint32_t res1;		
	struct data_seg_a64 inDataSeg64;	
	uint8_t res2[20];	
};

struct passthru_status {
	struct qla4_header hdr;		       
	uint32_t handle;	
	uint16_t target;	
	uint16_t connectionID;	

	uint8_t completionStatus;	
#define PASSTHRU_STATUS_COMPLETE		0x01

	uint8_t residualFlags;	

	uint16_t timeout;	
	uint16_t portNumber;	
	uint8_t res1[10];	
	uint32_t outResidual;	
	uint8_t res2[12];	
	uint32_t inResidual;	
	uint8_t res4[16];	
};

#endif 
