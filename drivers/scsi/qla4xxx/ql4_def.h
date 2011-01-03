

#ifndef __QL4_DEF_H
#define __QL4_DEF_H

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/dmapool.h>
#include <linux/mempool.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include <net/tcp.h>
#include <scsi/scsi.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_iscsi.h>


#ifndef PCI_DEVICE_ID_QLOGIC_ISP4010
#define PCI_DEVICE_ID_QLOGIC_ISP4010	0x4010
#endif

#ifndef PCI_DEVICE_ID_QLOGIC_ISP4022
#define PCI_DEVICE_ID_QLOGIC_ISP4022	0x4022
#endif

#ifndef PCI_DEVICE_ID_QLOGIC_ISP4032
#define PCI_DEVICE_ID_QLOGIC_ISP4032	0x4032
#endif

#define QLA_SUCCESS			0
#define QLA_ERROR			1


#define BIT_0	0x1
#define BIT_1	0x2
#define BIT_2	0x4
#define BIT_3	0x8
#define BIT_4	0x10
#define BIT_5	0x20
#define BIT_6	0x40
#define BIT_7	0x80
#define BIT_8	0x100
#define BIT_9	0x200
#define BIT_10	0x400
#define BIT_11	0x800
#define BIT_12	0x1000
#define BIT_13	0x2000
#define BIT_14	0x4000
#define BIT_15	0x8000
#define BIT_16	0x10000
#define BIT_17	0x20000
#define BIT_18	0x40000
#define BIT_19	0x80000
#define BIT_20	0x100000
#define BIT_21	0x200000
#define BIT_22	0x400000
#define BIT_23	0x800000
#define BIT_24	0x1000000
#define BIT_25	0x2000000
#define BIT_26	0x4000000
#define BIT_27	0x8000000
#define BIT_28	0x10000000
#define BIT_29	0x20000000
#define BIT_30	0x40000000
#define BIT_31	0x80000000


#define MAX_HBAS		16
#define MAX_BUSES		1
#define MAX_TARGETS		(MAX_PRST_DEV_DB_ENTRIES +  MAX_DEV_DB_ENTRIES)
#define MAX_LUNS		0xffff
#define MAX_AEN_ENTRIES		256 
#define MAX_DDB_ENTRIES		(MAX_PRST_DEV_DB_ENTRIES + MAX_DEV_DB_ENTRIES)
#define MAX_PDU_ENTRIES		32
#define INVALID_ENTRY		0xFFFF
#define MAX_CMDS_TO_RISC	1024
#define MAX_SRBS		MAX_CMDS_TO_RISC
#define MBOX_AEN_REG_COUNT	5
#define MAX_INIT_RETRIES	5


#define REQUEST_QUEUE_DEPTH		MAX_CMDS_TO_RISC
#define RESPONSE_QUEUE_DEPTH		64
#define QUEUE_SIZE			64
#define DMA_BUFFER_SIZE			512


#define MAC_ADDR_LEN			6	
#define IP_ADDR_LEN			4	
#define DRIVER_NAME			"qla4xxx"

#define MAX_LINKED_CMDS_PER_LUN		3
#define MAX_REQS_SERVICED_PER_INTR	16

#define ISCSI_IPADDR_SIZE		4	
#define ISCSI_ALIAS_SIZE		32	
#define ISCSI_NAME_SIZE			0xE0	

#define LSDW(x) ((u32)((u64)(x)))
#define MSDW(x) ((u32)((((u64)(x)) >> 16) >> 16))


#define MBOX_TOV			60
#define SOFT_RESET_TOV			30
#define RESET_INTR_TOV			3
#define SEMAPHORE_TOV			10
#define ADAPTER_INIT_TOV		120
#define ADAPTER_RESET_TOV		180
#define EXTEND_CMD_TOV			60
#define WAIT_CMD_TOV			30
#define EH_WAIT_CMD_TOV			120
#define FIRMWARE_UP_TOV			60
#define RESET_FIRMWARE_TOV		30
#define LOGOUT_TOV			10
#define IOCB_TOV_MARGIN			10
#define RELOGIN_TOV			18
#define ISNS_DEREG_TOV			5

#define MAX_RESET_HA_RETRIES		2


struct srb {
	struct list_head list;	
	struct scsi_qla_host *ha;	
	struct ddb_entry	*ddb;
	uint16_t flags;		

#define SRB_DMA_VALID		BIT_3	
#define SRB_GOT_SENSE		BIT_4	
	uint8_t state;		

#define SRB_NO_QUEUE_STATE	 0	
#define SRB_FREE_STATE		 1
#define SRB_ACTIVE_STATE	 3
#define SRB_ACTIVE_TIMEOUT_STATE 4
#define SRB_SUSPENDED_STATE	 7	

	struct scsi_cmnd *cmd;	
	dma_addr_t dma_handle;	
	atomic_t ref_count;	
	uint32_t fw_ddb_index;
	uint8_t err_id;		
#define SRB_ERR_PORT	   1	
#define SRB_ERR_LOOP	   2	
#define SRB_ERR_DEVICE	   3	
#define SRB_ERR_OTHER	   4

	uint16_t reserved;
	uint16_t iocb_tov;
	uint16_t iocb_cnt;	
	uint16_t cc_stat;
	u_long r_start;		
	u_long u_start;		

	
	uint8_t *req_sense_ptr;
	uint16_t req_sense_len;
	uint16_t reserved2;
};


struct aen {
        uint32_t mbox_sts[MBOX_AEN_REG_COUNT];
};

struct ql4_aen_log {
        int count;
        struct aen entry[MAX_AEN_ENTRIES];
};


struct ddb_entry {
	struct list_head list;	
	struct scsi_qla_host *ha;
	struct iscsi_cls_session *sess;
	struct iscsi_cls_conn *conn;

	atomic_t state;		

	unsigned long flags;	

	unsigned long dev_scan_wait_to_start_relogin;
	unsigned long dev_scan_wait_to_complete_relogin;

	uint16_t os_target_id;	
	uint16_t fw_ddb_index;	
	uint8_t reserved[2];
	uint32_t fw_ddb_device_state; 

	uint32_t CmdSn;
	uint16_t target_session_id;
	uint16_t connection_id;
	uint16_t exe_throttle;	
	uint16_t task_mgmt_timeout; 
	uint16_t default_relogin_timeout; 
	uint16_t tcp_source_port_num;
	uint32_t default_time2wait; 

	atomic_t port_down_timer; 
	atomic_t retry_relogin_timer; 
	atomic_t relogin_timer;	
	atomic_t relogin_retry_count; 

	uint16_t port;
	uint32_t tpgt;
	uint8_t ip_addr[ISCSI_IPADDR_SIZE];
	uint8_t iscsi_name[ISCSI_NAME_SIZE];	
	uint8_t iscsi_alias[0x20];
	uint8_t isid[6];
};


#define DDB_STATE_DEAD		0	
#define DDB_STATE_ONLINE	1	
#define DDB_STATE_MISSING	2	


#define DF_RELOGIN		0	
#define DF_NO_RELOGIN		1	
#define DF_ISNS_DISCOVERED	2	
#define DF_FO_MASKED		3


#include "ql4_fw.h"
#include "ql4_nvram.h"


struct scsi_qla_host {
	
	unsigned long flags;

#define AF_ONLINE			0 
#define AF_INIT_DONE			1 
#define AF_MBOX_COMMAND			2 
#define AF_MBOX_COMMAND_DONE		3 
#define AF_INTERRUPTS_ON		6 
#define AF_GET_CRASH_RECORD		7 
#define AF_LINK_UP			8 
#define AF_IRQ_ATTACHED			10 
#define AF_DISABLE_ACB_COMPLETE		11 

	unsigned long dpc_flags;

#define DPC_RESET_HA			1 
#define DPC_RETRY_RESET_HA		2 
#define DPC_RELOGIN_DEVICE		3 
#define DPC_RESET_HA_DESTROY_DDB_LIST	4 
#define DPC_RESET_HA_INTR		5 
#define DPC_ISNS_RESTART		7 
#define DPC_AEN				9 
#define DPC_GET_DHCP_IP_ADDR		15 

	struct Scsi_Host *host; 
	uint32_t tot_ddbs;

	uint16_t	iocb_cnt;

	
#define SRB_MIN_REQ	128
	mempool_t *srb_mempool;

	
	struct pci_dev *pdev;

	struct isp_reg __iomem *reg; 
	unsigned long pio_address;
	unsigned long pio_length;
#define MIN_IOBASE_LEN		0x100

	uint16_t req_q_count;
	uint8_t marker_needed;
	uint8_t rsvd1;

	unsigned long host_no;

	
	struct eeprom_data *nvram;
	spinlock_t hardware_lock ____cacheline_aligned;
	uint32_t   eeprom_cmd_data;

	
	uint64_t isr_count;
	uint64_t adapter_error_count;
	uint64_t device_error_count;
	uint64_t total_io_count;
	uint64_t total_mbytes_xferred;
	uint64_t link_failure_count;
	uint64_t invalid_crc_count;
	uint32_t bytes_xfered;
	uint32_t spurious_int_count;
	uint32_t aborted_io_count;
	uint32_t io_timeout_count;
	uint32_t mailbox_timeout_count;
	uint32_t seconds_since_last_intr;
	uint32_t seconds_since_last_heartbeat;
	uint32_t mac_index;

	
	
	uint32_t firmware_version[2];
	uint32_t patch_number;
	uint32_t build_number;
	uint32_t board_id;

	
	
	uint16_t firmware_options;
	uint16_t tcp_options;
	uint8_t ip_address[IP_ADDR_LEN];
	uint8_t subnet_mask[IP_ADDR_LEN];
	uint8_t gateway[IP_ADDR_LEN];
	uint8_t alias[32];
	uint8_t name_string[256];
	uint8_t heartbeat_interval;
	uint8_t rsvd;

	
	uint8_t my_mac[MAC_ADDR_LEN];
	uint8_t serial_number[16];

	
	uint32_t firmware_state;
	uint32_t addl_fw_state;

	
	struct workqueue_struct *dpc_thread;
	struct work_struct dpc_work;

	
	struct timer_list timer;
	uint32_t timer_active;

	
	uint32_t port_down_retry_count;
	uint32_t discovery_wait;
	atomic_t check_relogin_timeouts;
	uint32_t retry_reset_ha_cnt;
	uint32_t isp_reset_timer;	
	uint32_t nic_reset_timer;	
	int eh_start;
	struct list_head free_srb_q;
	uint16_t free_srb_q_count;
	uint16_t num_srbs_allocated;

	
	void *queues;
	dma_addr_t queues_dma;
	unsigned long queues_len;

#define MEM_ALIGN_VALUE \
	    ((max(REQUEST_QUEUE_DEPTH, RESPONSE_QUEUE_DEPTH)) * \
	     sizeof(struct queue_entry))
	
	dma_addr_t request_dma;
	struct queue_entry *request_ring;
	struct queue_entry *request_ptr;
	dma_addr_t response_dma;
	struct queue_entry *response_ring;
	struct queue_entry *response_ptr;
	dma_addr_t shadow_regs_dma;
	struct shadow_regs *shadow_regs;
	uint16_t request_in;	
	uint16_t request_out;
	uint16_t response_in;
	uint16_t response_out;

	
	uint16_t aen_q_count;	
	uint16_t aen_in;	
	uint16_t aen_out;
	struct aen aen_q[MAX_AEN_ENTRIES];

	struct ql4_aen_log aen_log;

	
	struct mutex  mbox_sem;

	
	volatile uint8_t mbox_status_count;
	volatile uint32_t mbox_status[MBOX_REG_COUNT];

	
	struct list_head ddb_list;

	
	struct ddb_entry *fw_ddb_index_map[MAX_DDB_ENTRIES];

	
	struct srb *status_srb;
};

static inline int is_qla4010(struct scsi_qla_host *ha)
{
	return ha->pdev->device == PCI_DEVICE_ID_QLOGIC_ISP4010;
}

static inline int is_qla4022(struct scsi_qla_host *ha)
{
	return ha->pdev->device == PCI_DEVICE_ID_QLOGIC_ISP4022;
}

static inline int is_qla4032(struct scsi_qla_host *ha)
{
	return ha->pdev->device == PCI_DEVICE_ID_QLOGIC_ISP4032;
}

static inline int adapter_up(struct scsi_qla_host *ha)
{
	return (test_bit(AF_ONLINE, &ha->flags) != 0) &&
		(test_bit(AF_LINK_UP, &ha->flags) != 0);
}

static inline struct scsi_qla_host* to_qla_host(struct Scsi_Host *shost)
{
	return (struct scsi_qla_host *)shost->hostdata;
}

static inline void __iomem* isp_semaphore(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u1.isp4010.nvram :
		&ha->reg->u1.isp4022.semaphore);
}

static inline void __iomem* isp_nvram(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u1.isp4010.nvram :
		&ha->reg->u1.isp4022.nvram);
}

static inline void __iomem* isp_ext_hw_conf(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u2.isp4010.ext_hw_conf :
		&ha->reg->u2.isp4022.p0.ext_hw_conf);
}

static inline void __iomem* isp_port_status(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u2.isp4010.port_status :
		&ha->reg->u2.isp4022.p0.port_status);
}

static inline void __iomem* isp_port_ctrl(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u2.isp4010.port_ctrl :
		&ha->reg->u2.isp4022.p0.port_ctrl);
}

static inline void __iomem* isp_port_error_status(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u2.isp4010.port_err_status :
		&ha->reg->u2.isp4022.p0.port_err_status);
}

static inline void __iomem * isp_gp_out(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		&ha->reg->u2.isp4010.gp_out :
		&ha->reg->u2.isp4022.p0.gp_out);
}

static inline int eeprom_ext_hw_conf_offset(struct scsi_qla_host *ha)
{
	return (is_qla4010(ha) ?
		offsetof(struct eeprom_data, isp4010.ext_hw_conf) / 2 :
		offsetof(struct eeprom_data, isp4022.ext_hw_conf) / 2);
}

int ql4xxx_sem_spinlock(struct scsi_qla_host * ha, u32 sem_mask, u32 sem_bits);
void ql4xxx_sem_unlock(struct scsi_qla_host * ha, u32 sem_mask);
int ql4xxx_sem_lock(struct scsi_qla_host * ha, u32 sem_mask, u32 sem_bits);

static inline int ql4xxx_lock_flash(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		return ql4xxx_sem_spinlock(a, QL4010_FLASH_SEM_MASK,
					   QL4010_FLASH_SEM_BITS);
	else
		return ql4xxx_sem_spinlock(a, QL4022_FLASH_SEM_MASK,
					   (QL4022_RESOURCE_BITS_BASE_CODE |
					    (a->mac_index)) << 13);
}

static inline void ql4xxx_unlock_flash(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		ql4xxx_sem_unlock(a, QL4010_FLASH_SEM_MASK);
	else
		ql4xxx_sem_unlock(a, QL4022_FLASH_SEM_MASK);
}

static inline int ql4xxx_lock_nvram(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		return ql4xxx_sem_spinlock(a, QL4010_NVRAM_SEM_MASK,
					   QL4010_NVRAM_SEM_BITS);
	else
		return ql4xxx_sem_spinlock(a, QL4022_NVRAM_SEM_MASK,
					   (QL4022_RESOURCE_BITS_BASE_CODE |
					    (a->mac_index)) << 10);
}

static inline void ql4xxx_unlock_nvram(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		ql4xxx_sem_unlock(a, QL4010_NVRAM_SEM_MASK);
	else
		ql4xxx_sem_unlock(a, QL4022_NVRAM_SEM_MASK);
}

static inline int ql4xxx_lock_drvr(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		return ql4xxx_sem_lock(a, QL4010_DRVR_SEM_MASK,
				       QL4010_DRVR_SEM_BITS);
	else
		return ql4xxx_sem_lock(a, QL4022_DRVR_SEM_MASK,
				       (QL4022_RESOURCE_BITS_BASE_CODE |
					(a->mac_index)) << 1);
}

static inline void ql4xxx_unlock_drvr(struct scsi_qla_host *a)
{
	if (is_qla4010(a))
		ql4xxx_sem_unlock(a, QL4010_DRVR_SEM_MASK);
	else
		ql4xxx_sem_unlock(a, QL4022_DRVR_SEM_MASK);
}




#define PRESERVE_DDB_LIST	0
#define REBUILD_DDB_LIST	1


#define PROCESS_ALL_AENS	 0
#define FLUSH_DDB_CHANGED_AENS	 1
#define RELOGIN_DDB_CHANGED_AENS 2

#endif	
