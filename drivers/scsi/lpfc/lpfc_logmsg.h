

#define LOG_ELS		0x00000001	
#define LOG_DISCOVERY	0x00000002	
#define LOG_MBOX	0x00000004	
#define LOG_INIT	0x00000008	
#define LOG_LINK_EVENT	0x00000010	
#define LOG_IP		0x00000020	
#define LOG_FCP		0x00000040	
#define LOG_NODE	0x00000080	
#define LOG_TEMP	0x00000100	
#define LOG_BG		0x00000200	
#define LOG_MISC	0x00000400	
#define LOG_SLI		0x00000800	
#define LOG_FCP_ERROR	0x00001000	
#define LOG_LIBDFC	0x00002000	
#define LOG_VPORT	0x00004000	
#define LOF_SECURITY	0x00008000	
#define LOG_EVENT	0x00010000	
#define LOG_ALL_MSG	0xffffffff	

#define lpfc_printf_vlog(vport, level, mask, fmt, arg...) \
do { \
	{ if (((mask) & (vport)->cfg_log_verbose) || (level[1] <= '3')) \
		dev_printk(level, &((vport)->phba->pcidev)->dev, "%d:(%d):" \
			   fmt, (vport)->phba->brd_no, vport->vpi, ##arg); } \
} while (0)

#define lpfc_printf_log(phba, level, mask, fmt, arg...) \
do { \
	{ uint32_t log_verbose = (phba)->pport ? \
				 (phba)->pport->cfg_log_verbose : \
				 (phba)->cfg_log_verbose; \
	  if (((mask) & log_verbose) || (level[1] <= '3')) \
		dev_printk(level, &((phba)->pcidev)->dev, "%d:" \
			   fmt, phba->brd_no, ##arg); \
	} \
} while (0)
