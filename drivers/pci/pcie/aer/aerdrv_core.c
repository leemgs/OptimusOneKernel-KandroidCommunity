

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include "aerdrv.h"

static int forceload;
static int nosourceid;
module_param(forceload, bool, 0);
module_param(nosourceid, bool, 0);

int pci_enable_pcie_error_reporting(struct pci_dev *dev)
{
	u16 reg16 = 0;
	int pos;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
	if (!pos)
		return -EIO;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return -EIO;

	pci_read_config_word(dev, pos+PCI_EXP_DEVCTL, &reg16);
	reg16 = reg16 |
		PCI_EXP_DEVCTL_CERE |
		PCI_EXP_DEVCTL_NFERE |
		PCI_EXP_DEVCTL_FERE |
		PCI_EXP_DEVCTL_URRE;
	pci_write_config_word(dev, pos+PCI_EXP_DEVCTL, reg16);

	return 0;
}
EXPORT_SYMBOL_GPL(pci_enable_pcie_error_reporting);

int pci_disable_pcie_error_reporting(struct pci_dev *dev)
{
	u16 reg16 = 0;
	int pos;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return -EIO;

	pci_read_config_word(dev, pos+PCI_EXP_DEVCTL, &reg16);
	reg16 = reg16 & ~(PCI_EXP_DEVCTL_CERE |
			PCI_EXP_DEVCTL_NFERE |
			PCI_EXP_DEVCTL_FERE |
			PCI_EXP_DEVCTL_URRE);
	pci_write_config_word(dev, pos+PCI_EXP_DEVCTL, reg16);

	return 0;
}
EXPORT_SYMBOL_GPL(pci_disable_pcie_error_reporting);

int pci_cleanup_aer_uncorrect_error_status(struct pci_dev *dev)
{
	int pos;
	u32 status, mask;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
	if (!pos)
		return -EIO;

	pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, &status);
	pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_SEVER, &mask);
	if (dev->error_state == pci_channel_io_normal)
		status &= ~mask; 
	else
		status &= mask; 
	pci_write_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, status);

	return 0;
}
EXPORT_SYMBOL_GPL(pci_cleanup_aer_uncorrect_error_status);

#if 0
int pci_cleanup_aer_correct_error_status(struct pci_dev *dev)
{
	int pos;
	u32 status;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
	if (!pos)
		return -EIO;

	pci_read_config_dword(dev, pos + PCI_ERR_COR_STATUS, &status);
	pci_write_config_dword(dev, pos + PCI_ERR_COR_STATUS, status);

	return 0;
}
#endif  

static int set_device_error_reporting(struct pci_dev *dev, void *data)
{
	bool enable = *((bool *)data);

	if (dev->pcie_type == PCIE_RC_PORT ||
	    dev->pcie_type == PCIE_SW_UPSTREAM_PORT ||
	    dev->pcie_type == PCIE_SW_DOWNSTREAM_PORT) {
		if (enable)
			pci_enable_pcie_error_reporting(dev);
		else
			pci_disable_pcie_error_reporting(dev);
	}

	if (enable)
		pcie_set_ecrc_checking(dev);

	return 0;
}


static void set_downstream_devices_error_reporting(struct pci_dev *dev,
						   bool enable)
{
	set_device_error_reporting(dev, &enable);

	if (!dev->subordinate)
		return;
	pci_walk_bus(dev->subordinate, set_device_error_reporting, &enable);
}

static inline int compare_device_id(struct pci_dev *dev,
			struct aer_err_info *e_info)
{
	if (e_info->id == ((dev->bus->number << 8) | dev->devfn)) {
		
		return 1;
	}

	return 0;
}

static int add_error_device(struct aer_err_info *e_info, struct pci_dev *dev)
{
	if (e_info->error_dev_num < AER_MAX_MULTI_ERR_DEVICES) {
		e_info->dev[e_info->error_dev_num] = dev;
		e_info->error_dev_num++;
		return 1;
	}

	return 0;
}


#define	PCI_BUS(x)	(((x) >> 8) & 0xff)

static int find_device_iter(struct pci_dev *dev, void *data)
{
	int pos;
	u32 status;
	u32 mask;
	u16 reg16;
	int result;
	struct aer_err_info *e_info = (struct aer_err_info *)data;

	
	if (!nosourceid && (PCI_BUS(e_info->id) != 0)) {
		result = compare_device_id(dev, e_info);
		if (result)
			add_error_device(e_info, dev);

		
		if (!e_info->multi_error_valid)
			return result;

		
		if (result)
			return 0;
	}

	
	if (atomic_read(&dev->enable_cnt) == 0)
		return 0;
	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return 0;
	
	pci_read_config_word(dev, pos+PCI_EXP_DEVCTL, &reg16);
	if (!(reg16 & (
		PCI_EXP_DEVCTL_CERE |
		PCI_EXP_DEVCTL_NFERE |
		PCI_EXP_DEVCTL_FERE |
		PCI_EXP_DEVCTL_URRE)))
		return 0;
	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
	if (!pos)
		return 0;

	status = 0;
	mask = 0;
	if (e_info->severity == AER_CORRECTABLE) {
		pci_read_config_dword(dev, pos + PCI_ERR_COR_STATUS, &status);
		pci_read_config_dword(dev, pos + PCI_ERR_COR_MASK, &mask);
		if (status & ~mask) {
			add_error_device(e_info, dev);
			goto added;
		}
	} else {
		pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, &status);
		pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_MASK, &mask);
		if (status & ~mask) {
			add_error_device(e_info, dev);
			goto added;
		}
	}

	return 0;

added:
	if (e_info->multi_error_valid)
		return 0;
	else
		return 1;
}


static void find_source_device(struct pci_dev *parent,
		struct aer_err_info *e_info)
{
	struct pci_dev *dev = parent;
	int result;

	
	result = find_device_iter(dev, e_info);
	if (result)
		return;

	pci_walk_bus(parent->subordinate, find_device_iter, e_info);
}

static int report_error_detected(struct pci_dev *dev, void *data)
{
	pci_ers_result_t vote;
	struct pci_error_handlers *err_handler;
	struct aer_broadcast_data *result_data;
	result_data = (struct aer_broadcast_data *) data;

	dev->error_state = result_data->state;

	if (!dev->driver ||
		!dev->driver->err_handler ||
		!dev->driver->err_handler->error_detected) {
		if (result_data->state == pci_channel_io_frozen &&
			!(dev->hdr_type & PCI_HEADER_TYPE_BRIDGE)) {
			
			dev_printk(KERN_DEBUG, &dev->dev, "device has %s\n",
				   dev->driver ?
				   "no AER-aware driver" : "no driver");
		}
		return 0;
	}

	err_handler = dev->driver->err_handler;
	vote = err_handler->error_detected(dev, result_data->state);
	result_data->result = merge_result(result_data->result, vote);
	return 0;
}

static int report_mmio_enabled(struct pci_dev *dev, void *data)
{
	pci_ers_result_t vote;
	struct pci_error_handlers *err_handler;
	struct aer_broadcast_data *result_data;
	result_data = (struct aer_broadcast_data *) data;

	if (!dev->driver ||
		!dev->driver->err_handler ||
		!dev->driver->err_handler->mmio_enabled)
		return 0;

	err_handler = dev->driver->err_handler;
	vote = err_handler->mmio_enabled(dev);
	result_data->result = merge_result(result_data->result, vote);
	return 0;
}

static int report_slot_reset(struct pci_dev *dev, void *data)
{
	pci_ers_result_t vote;
	struct pci_error_handlers *err_handler;
	struct aer_broadcast_data *result_data;
	result_data = (struct aer_broadcast_data *) data;

	if (!dev->driver ||
		!dev->driver->err_handler ||
		!dev->driver->err_handler->slot_reset)
		return 0;

	err_handler = dev->driver->err_handler;
	vote = err_handler->slot_reset(dev);
	result_data->result = merge_result(result_data->result, vote);
	return 0;
}

static int report_resume(struct pci_dev *dev, void *data)
{
	struct pci_error_handlers *err_handler;

	dev->error_state = pci_channel_io_normal;

	if (!dev->driver ||
		!dev->driver->err_handler ||
		!dev->driver->err_handler->resume)
		return 0;

	err_handler = dev->driver->err_handler;
	err_handler->resume(dev);
	return 0;
}


static pci_ers_result_t broadcast_error_message(struct pci_dev *dev,
	enum pci_channel_state state,
	char *error_mesg,
	int (*cb)(struct pci_dev *, void *))
{
	struct aer_broadcast_data result_data;

	dev_printk(KERN_DEBUG, &dev->dev, "broadcast %s message\n", error_mesg);
	result_data.state = state;
	if (cb == report_error_detected)
		result_data.result = PCI_ERS_RESULT_CAN_RECOVER;
	else
		result_data.result = PCI_ERS_RESULT_RECOVERED;

	if (dev->hdr_type & PCI_HEADER_TYPE_BRIDGE) {
		
		if (cb == report_error_detected)
			dev->error_state = state;
		pci_walk_bus(dev->subordinate, cb, &result_data);
		if (cb == report_resume) {
			pci_cleanup_aer_uncorrect_error_status(dev);
			dev->error_state = pci_channel_io_normal;
		}
	} else {
		
		pci_walk_bus(dev->bus, cb, &result_data);
	}

	return result_data.result;
}

struct find_aer_service_data {
	struct pcie_port_service_driver *aer_driver;
	int is_downstream;
};

static int find_aer_service_iter(struct device *device, void *data)
{
	struct device_driver *driver;
	struct pcie_port_service_driver *service_driver;
	struct find_aer_service_data *result;

	result = (struct find_aer_service_data *) data;

	if (device->bus == &pcie_port_bus_type) {
		struct pcie_port_data *port_data;

		port_data = pci_get_drvdata(to_pcie_device(device)->port);
		if (port_data->port_type == PCIE_SW_DOWNSTREAM_PORT)
			result->is_downstream = 1;

		driver = device->driver;
		if (driver) {
			service_driver = to_service_driver(driver);
			if (service_driver->service == PCIE_PORT_SERVICE_AER) {
				result->aer_driver = service_driver;
				return 1;
			}
		}
	}

	return 0;
}

static void find_aer_service(struct pci_dev *dev,
		struct find_aer_service_data *data)
{
	int retval;
	retval = device_for_each_child(&dev->dev, data, find_aer_service_iter);
}

static pci_ers_result_t reset_link(struct pcie_device *aerdev,
		struct pci_dev *dev)
{
	struct pci_dev *udev;
	pci_ers_result_t status;
	struct find_aer_service_data data;

	if (dev->hdr_type & PCI_HEADER_TYPE_BRIDGE)
		udev = dev;
	else
		udev = dev->bus->self;

	data.is_downstream = 0;
	data.aer_driver = NULL;
	find_aer_service(udev, &data);

	
	if (!data.aer_driver || !data.aer_driver->reset_link) {
		if (data.is_downstream &&
			aerdev->device.driver &&
			to_service_driver(aerdev->device.driver)->reset_link) {
			data.aer_driver =
				to_service_driver(aerdev->device.driver);
		} else {
			dev_printk(KERN_DEBUG, &dev->dev, "no link-reset "
				   "support\n");
			return PCI_ERS_RESULT_DISCONNECT;
		}
	}

	status = data.aer_driver->reset_link(udev);
	if (status != PCI_ERS_RESULT_RECOVERED) {
		dev_printk(KERN_DEBUG, &dev->dev, "link reset at upstream "
			   "device %s failed\n", pci_name(udev));
		return PCI_ERS_RESULT_DISCONNECT;
	}

	return status;
}


static pci_ers_result_t do_recovery(struct pcie_device *aerdev,
		struct pci_dev *dev,
		int severity)
{
	pci_ers_result_t status, result = PCI_ERS_RESULT_RECOVERED;
	enum pci_channel_state state;

	if (severity == AER_FATAL)
		state = pci_channel_io_frozen;
	else
		state = pci_channel_io_normal;

	status = broadcast_error_message(dev,
			state,
			"error_detected",
			report_error_detected);

	if (severity == AER_FATAL) {
		result = reset_link(aerdev, dev);
		if (result != PCI_ERS_RESULT_RECOVERED) {
			
			return result;
		}
	}

	if (status == PCI_ERS_RESULT_CAN_RECOVER)
		status = broadcast_error_message(dev,
				state,
				"mmio_enabled",
				report_mmio_enabled);

	if (status == PCI_ERS_RESULT_NEED_RESET) {
		
		status = broadcast_error_message(dev,
				state,
				"slot_reset",
				report_slot_reset);
	}

	if (status == PCI_ERS_RESULT_RECOVERED)
		broadcast_error_message(dev,
				state,
				"resume",
				report_resume);

	return status;
}


static void handle_error_source(struct pcie_device *aerdev,
	struct pci_dev *dev,
	struct aer_err_info *info)
{
	pci_ers_result_t status = 0;
	int pos;

	if (info->severity == AER_CORRECTABLE) {
		
		pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
		if (pos)
			pci_write_config_dword(dev, pos + PCI_ERR_COR_STATUS,
					info->status);
	} else {
		status = do_recovery(aerdev, dev, info->severity);
		if (status == PCI_ERS_RESULT_RECOVERED) {
			dev_printk(KERN_DEBUG, &dev->dev, "AER driver "
				   "successfully recovered\n");
		} else {
			
			dev_printk(KERN_DEBUG, &dev->dev, "AER driver didn't "
				   "recover\n");
		}
	}
}


void aer_enable_rootport(struct aer_rpc *rpc)
{
	struct pci_dev *pdev = rpc->rpd->port;
	int pos, aer_pos;
	u16 reg16;
	u32 reg32;

	pos = pci_find_capability(pdev, PCI_CAP_ID_EXP);
	
	pci_read_config_word(pdev, pos+PCI_EXP_DEVSTA, &reg16);
	pci_write_config_word(pdev, pos+PCI_EXP_DEVSTA, reg16);

	
	pci_read_config_word(pdev, pos + PCI_EXP_RTCTL, &reg16);
	reg16 &= ~(SYSTEM_ERROR_INTR_ON_MESG_MASK);
	pci_write_config_word(pdev, pos + PCI_EXP_RTCTL, reg16);

	aer_pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_ERR);
	
	pci_read_config_dword(pdev, aer_pos + PCI_ERR_ROOT_STATUS, &reg32);
	pci_write_config_dword(pdev, aer_pos + PCI_ERR_ROOT_STATUS, reg32);
	pci_read_config_dword(pdev, aer_pos + PCI_ERR_COR_STATUS, &reg32);
	pci_write_config_dword(pdev, aer_pos + PCI_ERR_COR_STATUS, reg32);
	pci_read_config_dword(pdev, aer_pos + PCI_ERR_UNCOR_STATUS, &reg32);
	pci_write_config_dword(pdev, aer_pos + PCI_ERR_UNCOR_STATUS, reg32);

	
	set_downstream_devices_error_reporting(pdev, true);

	
	pci_write_config_dword(pdev,
		aer_pos + PCI_ERR_ROOT_COMMAND,
		ROOT_PORT_INTR_ON_MESG_MASK);
}


static void disable_root_aer(struct aer_rpc *rpc)
{
	struct pci_dev *pdev = rpc->rpd->port;
	u32 reg32;
	int pos;

	
	set_downstream_devices_error_reporting(pdev, false);

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_ERR);
	
	pci_write_config_dword(pdev, pos + PCI_ERR_ROOT_COMMAND, 0);

	
	pci_read_config_dword(pdev, pos + PCI_ERR_ROOT_STATUS, &reg32);
	pci_write_config_dword(pdev, pos + PCI_ERR_ROOT_STATUS, reg32);
}


static struct aer_err_source *get_e_source(struct aer_rpc *rpc)
{
	struct aer_err_source *e_source;
	unsigned long flags;

	
	spin_lock_irqsave(&rpc->e_lock, flags);
	if (rpc->prod_idx == rpc->cons_idx) {
		spin_unlock_irqrestore(&rpc->e_lock, flags);
		return NULL;
	}
	e_source = &rpc->e_sources[rpc->cons_idx];
	rpc->cons_idx++;
	if (rpc->cons_idx == AER_ERROR_SOURCES_MAX)
		rpc->cons_idx = 0;
	spin_unlock_irqrestore(&rpc->e_lock, flags);

	return e_source;
}


static int get_device_error_info(struct pci_dev *dev, struct aer_err_info *info)
{
	int pos, temp;

	info->status = 0;
	info->tlp_header_valid = 0;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);

	
	if (!pos)
		return 1;

	if (info->severity == AER_CORRECTABLE) {
		pci_read_config_dword(dev, pos + PCI_ERR_COR_STATUS,
			&info->status);
		pci_read_config_dword(dev, pos + PCI_ERR_COR_MASK,
			&info->mask);
		if (!(info->status & ~info->mask))
			return 0;
	} else if (dev->hdr_type & PCI_HEADER_TYPE_BRIDGE ||
		info->severity == AER_NONFATAL) {

		
		pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS,
			&info->status);
		pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_MASK,
			&info->mask);
		if (!(info->status & ~info->mask))
			return 0;

		
		pci_read_config_dword(dev, pos + PCI_ERR_CAP, &temp);
		info->first_error = PCI_ERR_CAP_FEP(temp);

		if (info->status & AER_LOG_TLP_MASKS) {
			info->tlp_header_valid = 1;
			pci_read_config_dword(dev,
				pos + PCI_ERR_HEADER_LOG, &info->tlp.dw0);
			pci_read_config_dword(dev,
				pos + PCI_ERR_HEADER_LOG + 4, &info->tlp.dw1);
			pci_read_config_dword(dev,
				pos + PCI_ERR_HEADER_LOG + 8, &info->tlp.dw2);
			pci_read_config_dword(dev,
				pos + PCI_ERR_HEADER_LOG + 12, &info->tlp.dw3);
		}
	}

	return 1;
}

static inline void aer_process_err_devices(struct pcie_device *p_device,
			struct aer_err_info *e_info)
{
	int i;

	if (!e_info->dev[0]) {
		dev_printk(KERN_DEBUG, &p_device->port->dev,
				"can't find device of ID%04x\n",
				e_info->id);
	}

	
	for (i = 0; i < e_info->error_dev_num && e_info->dev[i]; i++) {
		if (get_device_error_info(e_info->dev[i], e_info))
			aer_print_error(e_info->dev[i], e_info);
	}
	for (i = 0; i < e_info->error_dev_num && e_info->dev[i]; i++) {
		if (get_device_error_info(e_info->dev[i], e_info))
			handle_error_source(p_device, e_info->dev[i], e_info);
	}
}


static void aer_isr_one_error(struct pcie_device *p_device,
		struct aer_err_source *e_src)
{
	struct aer_err_info *e_info;
	int i;

	
	e_info = kmalloc(sizeof(struct aer_err_info), GFP_KERNEL);
	if (e_info == NULL) {
		dev_printk(KERN_DEBUG, &p_device->port->dev,
			"Can't allocate mem when processing AER errors\n");
		return;
	}

	
	for (i = 1; i & ROOT_ERR_STATUS_MASKS ; i <<= 2) {
		if (i > 4)
			break;
		if (!(e_src->status & i))
			continue;

		memset(e_info, 0, sizeof(struct aer_err_info));

		
		if (i & PCI_ERR_ROOT_COR_RCV) {
			e_info->id = ERR_COR_ID(e_src->id);
			e_info->severity = AER_CORRECTABLE;
		} else {
			e_info->id = ERR_UNCOR_ID(e_src->id);
			e_info->severity = ((e_src->status >> 6) & 1);
		}
		if (e_src->status &
			(PCI_ERR_ROOT_MULTI_COR_RCV |
			 PCI_ERR_ROOT_MULTI_UNCOR_RCV))
			e_info->multi_error_valid = 1;

		aer_print_port_info(p_device->port, e_info);

		find_source_device(p_device->port, e_info);
		aer_process_err_devices(p_device, e_info);
	}

	kfree(e_info);
}


void aer_isr(struct work_struct *work)
{
	struct aer_rpc *rpc = container_of(work, struct aer_rpc, dpc_handler);
	struct pcie_device *p_device = rpc->rpd;
	struct aer_err_source *e_src;

	mutex_lock(&rpc->rpc_mutex);
	e_src = get_e_source(rpc);
	while (e_src) {
		aer_isr_one_error(p_device, e_src);
		e_src = get_e_source(rpc);
	}
	mutex_unlock(&rpc->rpc_mutex);

	wake_up(&rpc->wait_release);
}


void aer_delete_rootport(struct aer_rpc *rpc)
{
	
	disable_root_aer(rpc);

	kfree(rpc);
}


int aer_init(struct pcie_device *dev)
{
	if (aer_osc_setup(dev) && !forceload)
		return -ENXIO;

	return 0;
}
