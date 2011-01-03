


#define FC_REG_LINK_EVENT		0x0001	
#define FC_REG_RSCN_EVENT		0x0002	
#define FC_REG_CT_EVENT			0x0004	
#define FC_REG_DUMP_EVENT		0x0010	
#define FC_REG_TEMPERATURE_EVENT	0x0020	
#define FC_REG_VPORTRSCN_EVENT		0x0040	
#define FC_REG_ELS_EVENT		0x0080	
#define FC_REG_FABRIC_EVENT		0x0100	
#define FC_REG_SCSI_EVENT		0x0200	
#define FC_REG_BOARD_EVENT		0x0400	
#define FC_REG_ADAPTER_EVENT		0x0800	
#define FC_REG_EVENT_MASK		(FC_REG_LINK_EVENT | \
						FC_REG_RSCN_EVENT | \
						FC_REG_CT_EVENT | \
						FC_REG_DUMP_EVENT | \
						FC_REG_TEMPERATURE_EVENT | \
						FC_REG_VPORTRSCN_EVENT | \
						FC_REG_ELS_EVENT | \
						FC_REG_FABRIC_EVENT | \
						FC_REG_SCSI_EVENT | \
						FC_REG_BOARD_EVENT | \
						FC_REG_ADAPTER_EVENT)

#define LPFC_CRIT_TEMP		0x1
#define LPFC_THRESHOLD_TEMP	0x2
#define LPFC_NORMAL_TEMP	0x3



struct lpfc_rscn_event_header {
	uint32_t event_type;
	uint32_t payload_length; 
	uint32_t rscn_payload[];
};


struct lpfc_els_event_header {
	uint32_t event_type;
	uint32_t subcategory;
	uint8_t wwpn[8];
	uint8_t wwnn[8];
};


#define LPFC_EVENT_PLOGI_RCV		0x01
#define LPFC_EVENT_PRLO_RCV		0x02
#define LPFC_EVENT_ADISC_RCV		0x04
#define LPFC_EVENT_LSRJT_RCV		0x08
#define LPFC_EVENT_LOGO_RCV		0x10


struct lpfc_lsrjt_event {
	struct lpfc_els_event_header header;
	uint32_t command;
	uint32_t reason_code;
	uint32_t explanation;
};


struct lpfc_logo_event {
	struct lpfc_els_event_header header;
	uint8_t logo_wwpn[8];
};


struct lpfc_fabric_event_header {
	uint32_t event_type;
	uint32_t subcategory;
	uint8_t wwpn[8];
	uint8_t wwnn[8];
};


#define LPFC_EVENT_FABRIC_BUSY		0x01
#define LPFC_EVENT_PORT_BUSY		0x02
#define LPFC_EVENT_FCPRDCHKERR		0x04


struct lpfc_fcprdchkerr_event {
	struct lpfc_fabric_event_header header;
	uint32_t lun;
	uint32_t opcode;
	uint32_t fcpiparam;
};



struct lpfc_scsi_event_header {
	uint32_t event_type;
	uint32_t subcategory;
	uint32_t lun;
	uint8_t wwpn[8];
	uint8_t wwnn[8];
};


#define LPFC_EVENT_QFULL	0x0001
#define LPFC_EVENT_DEVBSY	0x0002
#define LPFC_EVENT_CHECK_COND	0x0004
#define LPFC_EVENT_LUNRESET	0x0008
#define LPFC_EVENT_TGTRESET	0x0010
#define LPFC_EVENT_BUSRESET	0x0020
#define LPFC_EVENT_VARQUEDEPTH	0x0040


struct lpfc_scsi_varqueuedepth_event {
	struct lpfc_scsi_event_header scsi_event;
	uint32_t oldval;
	uint32_t newval;
};


struct lpfc_scsi_check_condition_event {
	struct lpfc_scsi_event_header scsi_event;
	uint8_t opcode;
	uint8_t sense_key;
	uint8_t asc;
	uint8_t ascq;
};


#define LPFC_EVENT_PORTINTERR		0x01


struct lpfc_board_event_header {
	uint32_t event_type;
	uint32_t subcategory;
};



#define LPFC_EVENT_ARRIVAL	0x01


struct lpfc_adapter_event_header {
	uint32_t event_type;
	uint32_t subcategory;
};



#define LPFC_CRIT_TEMP		0x1
#define LPFC_THRESHOLD_TEMP	0x2
#define LPFC_NORMAL_TEMP	0x3

struct temp_event {
	uint32_t event_type;
	uint32_t event_code;
	uint32_t data;
};


#define LPFC_BSG_VENDOR_SET_CT_EVENT	1
#define LPFC_BSG_VENDOR_GET_CT_EVENT	2

struct set_ct_event {
	uint32_t command;
	uint32_t ev_req_id;
	uint32_t ev_reg_id;
};

struct get_ct_event {
	uint32_t command;
	uint32_t ev_reg_id;
	uint32_t ev_req_id;
};

struct get_ct_event_reply {
	uint32_t immed_data;
	uint32_t type;
};
