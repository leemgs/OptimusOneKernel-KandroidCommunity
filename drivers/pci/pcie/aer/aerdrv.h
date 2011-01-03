

#ifndef _AERDRV_H_
#define _AERDRV_H_

#include <linux/workqueue.h>
#include <linux/pcieport_if.h>
#include <linux/aer.h>
#include <linux/interrupt.h>

#define AER_NONFATAL			0
#define AER_FATAL			1
#define AER_CORRECTABLE			2


#define ROOT_ERR_STATUS_MASKS		0x0f

#define SYSTEM_ERROR_INTR_ON_MESG_MASK	(PCI_EXP_RTCTL_SECEE|	\
					PCI_EXP_RTCTL_SENFEE|	\
					PCI_EXP_RTCTL_SEFEE)
#define ROOT_PORT_INTR_ON_MESG_MASK	(PCI_ERR_ROOT_CMD_COR_EN|	\
					PCI_ERR_ROOT_CMD_NONFATAL_EN|	\
					PCI_ERR_ROOT_CMD_FATAL_EN)
#define ERR_COR_ID(d)			(d & 0xffff)
#define ERR_UNCOR_ID(d)			(d >> 16)

#define AER_ERROR_SOURCES_MAX		100

#define AER_LOG_TLP_MASKS		(PCI_ERR_UNC_POISON_TLP|	\
					PCI_ERR_UNC_ECRC|		\
					PCI_ERR_UNC_UNSUP|		\
					PCI_ERR_UNC_COMP_ABORT|		\
					PCI_ERR_UNC_UNX_COMP|		\
					PCI_ERR_UNC_MALF_TLP)

struct header_log_regs {
	unsigned int dw0;
	unsigned int dw1;
	unsigned int dw2;
	unsigned int dw3;
};

#define AER_MAX_MULTI_ERR_DEVICES	5	
struct aer_err_info {
	struct pci_dev *dev[AER_MAX_MULTI_ERR_DEVICES];
	int error_dev_num;

	unsigned int id:16;

	unsigned int severity:2;	
	unsigned int __pad1:5;
	unsigned int multi_error_valid:1;

	unsigned int first_error:5;
	unsigned int __pad2:2;
	unsigned int tlp_header_valid:1;

	unsigned int status;		
	unsigned int mask;		
	struct header_log_regs tlp;	
};

struct aer_err_source {
	unsigned int status;
	unsigned int id;
};

struct aer_rpc {
	struct pcie_device *rpd;	
	struct work_struct dpc_handler;
	struct aer_err_source e_sources[AER_ERROR_SOURCES_MAX];
	unsigned short prod_idx;	
	unsigned short cons_idx;	
	int isr;
	spinlock_t e_lock;		
	struct mutex rpc_mutex;		
	wait_queue_head_t wait_release;
};

struct aer_broadcast_data {
	enum pci_channel_state state;
	enum pci_ers_result result;
};

static inline pci_ers_result_t merge_result(enum pci_ers_result orig,
		enum pci_ers_result new)
{
	if (new == PCI_ERS_RESULT_NONE)
		return orig;

	switch (orig) {
	case PCI_ERS_RESULT_CAN_RECOVER:
	case PCI_ERS_RESULT_RECOVERED:
		orig = new;
		break;
	case PCI_ERS_RESULT_DISCONNECT:
		if (new == PCI_ERS_RESULT_NEED_RESET)
			orig = new;
		break;
	default:
		break;
	}

	return orig;
}

extern struct bus_type pcie_port_bus_type;
extern void aer_enable_rootport(struct aer_rpc *rpc);
extern void aer_delete_rootport(struct aer_rpc *rpc);
extern int aer_init(struct pcie_device *dev);
extern void aer_isr(struct work_struct *work);
extern void aer_print_error(struct pci_dev *dev, struct aer_err_info *info);
extern void aer_print_port_info(struct pci_dev *dev, struct aer_err_info *info);
extern irqreturn_t aer_irq(int irq, void *context);

#ifdef CONFIG_ACPI
extern int aer_osc_setup(struct pcie_device *pciedev);
#else
static inline int aer_osc_setup(struct pcie_device *pciedev)
{
	return 0;
}
#endif

#endif 
