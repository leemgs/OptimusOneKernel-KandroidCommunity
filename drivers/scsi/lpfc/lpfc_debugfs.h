

#ifndef _H_LPFC_DEBUG_FS
#define _H_LPFC_DEBUG_FS

#ifdef CONFIG_SCSI_LPFC_DEBUG_FS
struct lpfc_debugfs_trc {
	char *fmt;
	uint32_t data1;
	uint32_t data2;
	uint32_t data3;
	uint32_t seq_cnt;
	unsigned long jif;
};
#endif


#define LPFC_DISC_TRC_ELS_CMD		0x1	
#define LPFC_DISC_TRC_ELS_RSP		0x2	
#define LPFC_DISC_TRC_ELS_UNSOL		0x4	
#define LPFC_DISC_TRC_ELS_ALL		0x7	
#define LPFC_DISC_TRC_MBOX_VPORT	0x8	
#define LPFC_DISC_TRC_MBOX		0x10	
#define LPFC_DISC_TRC_MBOX_ALL		0x18	
#define LPFC_DISC_TRC_CT		0x20	
#define LPFC_DISC_TRC_DSM		0x40    
#define LPFC_DISC_TRC_RPORT		0x80    
#define LPFC_DISC_TRC_NODE		0x100   

#define LPFC_DISC_TRC_DISCOVERY		0xef    
#endif 
