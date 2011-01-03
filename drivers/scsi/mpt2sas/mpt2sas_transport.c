

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport_sas.h>
#include <scsi/scsi_dbg.h>

#include "mpt2sas_base.h"

static struct _sas_node *
_transport_sas_node_find_by_handle(struct MPT2SAS_ADAPTER *ioc, u16 handle)
{
	int i;

	for (i = 0; i < ioc->sas_hba.num_phys; i++)
		if (ioc->sas_hba.phy[i].handle == handle)
			return &ioc->sas_hba;

	return mpt2sas_scsih_expander_find_by_handle(ioc, handle);
}


static enum sas_linkrate
_transport_convert_phy_link_rate(u8 link_rate)
{
	enum sas_linkrate rc;

	switch (link_rate) {
	case MPI2_SAS_NEG_LINK_RATE_1_5:
		rc = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI2_SAS_NEG_LINK_RATE_3_0:
		rc = SAS_LINK_RATE_3_0_GBPS;
		break;
	case MPI2_SAS_NEG_LINK_RATE_6_0:
		rc = SAS_LINK_RATE_6_0_GBPS;
		break;
	case MPI2_SAS_NEG_LINK_RATE_PHY_DISABLED:
		rc = SAS_PHY_DISABLED;
		break;
	case MPI2_SAS_NEG_LINK_RATE_NEGOTIATION_FAILED:
		rc = SAS_LINK_RATE_FAILED;
		break;
	case MPI2_SAS_NEG_LINK_RATE_PORT_SELECTOR:
		rc = SAS_SATA_PORT_SELECTOR;
		break;
	case MPI2_SAS_NEG_LINK_RATE_SMP_RESET_IN_PROGRESS:
		rc = SAS_PHY_RESET_IN_PROGRESS;
		break;
	default:
	case MPI2_SAS_NEG_LINK_RATE_SATA_OOB_COMPLETE:
	case MPI2_SAS_NEG_LINK_RATE_UNKNOWN_LINK_RATE:
		rc = SAS_LINK_RATE_UNKNOWN;
		break;
	}
	return rc;
}


static int
_transport_set_identify(struct MPT2SAS_ADAPTER *ioc, u16 handle,
    struct sas_identify *identify)
{
	Mpi2SasDevicePage0_t sas_device_pg0;
	Mpi2ConfigReply_t mpi_reply;
	u32 device_info;
	u32 ioc_status;

	if (ioc->shost_recovery) {
		printk(MPT2SAS_INFO_FMT "%s: host reset in progress!\n",
		    __func__, ioc->name);
		return -EFAULT;
	}

	if ((mpt2sas_config_get_sas_device_pg0(ioc, &mpi_reply, &sas_device_pg0,
	    MPI2_SAS_DEVICE_PGAD_FORM_HANDLE, handle))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",

		    ioc->name, __FILE__, __LINE__, __func__);
		return -ENXIO;
	}

	ioc_status = le16_to_cpu(mpi_reply.IOCStatus) &
	    MPI2_IOCSTATUS_MASK;
	if (ioc_status != MPI2_IOCSTATUS_SUCCESS) {
		printk(MPT2SAS_ERR_FMT "handle(0x%04x), ioc_status(0x%04x)"
		    "\nfailure at %s:%d/%s()!\n", ioc->name, handle, ioc_status,
		     __FILE__, __LINE__, __func__);
		return -EIO;
	}

	memset(identify, 0, sizeof(identify));
	device_info = le32_to_cpu(sas_device_pg0.DeviceInfo);

	
	identify->sas_address = le64_to_cpu(sas_device_pg0.SASAddress);

	
	switch (device_info & MPI2_SAS_DEVICE_INFO_MASK_DEVICE_TYPE) {
	case MPI2_SAS_DEVICE_INFO_NO_DEVICE:
		identify->device_type = SAS_PHY_UNUSED;
		break;
	case MPI2_SAS_DEVICE_INFO_END_DEVICE:
		identify->device_type = SAS_END_DEVICE;
		break;
	case MPI2_SAS_DEVICE_INFO_EDGE_EXPANDER:
		identify->device_type = SAS_EDGE_EXPANDER_DEVICE;
		break;
	case MPI2_SAS_DEVICE_INFO_FANOUT_EXPANDER:
		identify->device_type = SAS_FANOUT_EXPANDER_DEVICE;
		break;
	}

	
	if (device_info & MPI2_SAS_DEVICE_INFO_SSP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SSP;
	if (device_info & MPI2_SAS_DEVICE_INFO_STP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_STP;
	if (device_info & MPI2_SAS_DEVICE_INFO_SMP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SMP;
	if (device_info & MPI2_SAS_DEVICE_INFO_SATA_HOST)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SATA;

	
	if (device_info & MPI2_SAS_DEVICE_INFO_SSP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_SSP;
	if (device_info & MPI2_SAS_DEVICE_INFO_STP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_STP;
	if (device_info & MPI2_SAS_DEVICE_INFO_SMP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_SMP;
	if (device_info & MPI2_SAS_DEVICE_INFO_SATA_DEVICE)
		identify->target_port_protocols |= SAS_PROTOCOL_SATA;

	return 0;
}


u8
mpt2sas_transport_done(struct MPT2SAS_ADAPTER *ioc, u16 smid, u8 msix_index,
    u32 reply)
{
	MPI2DefaultReply_t *mpi_reply;

	mpi_reply =  mpt2sas_base_get_reply_virt_addr(ioc, reply);
	if (ioc->transport_cmds.status == MPT2_CMD_NOT_USED)
		return 1;
	if (ioc->transport_cmds.smid != smid)
		return 1;
	ioc->transport_cmds.status |= MPT2_CMD_COMPLETE;
	if (mpi_reply) {
		memcpy(ioc->transport_cmds.reply, mpi_reply,
		    mpi_reply->MsgLength*4);
		ioc->transport_cmds.status |= MPT2_CMD_REPLY_VALID;
	}
	ioc->transport_cmds.status &= ~MPT2_CMD_PENDING;
	complete(&ioc->transport_cmds.done);
	return 1;
}


struct rep_manu_request{
	u8 smp_frame_type;
	u8 function;
	u8 reserved;
	u8 request_length;
};


struct rep_manu_reply{
	u8 smp_frame_type; 
	u8 function; 
	u8 function_result;
	u8 response_length;
	u16 expander_change_count;
	u8 reserved0[2];
	u8 sas_format:1;
	u8 reserved1:7;
	u8 reserved2[3];
	u8 vendor_id[SAS_EXPANDER_VENDOR_ID_LEN];
	u8 product_id[SAS_EXPANDER_PRODUCT_ID_LEN];
	u8 product_rev[SAS_EXPANDER_PRODUCT_REV_LEN];
	u8 component_vendor_id[SAS_EXPANDER_COMPONENT_VENDOR_ID_LEN];
	u16 component_id;
	u8 component_revision_id;
	u8 reserved3;
	u8 vendor_specific[8];
};


static int
_transport_expander_report_manufacture(struct MPT2SAS_ADAPTER *ioc,
    u64 sas_address, struct sas_expander_device *edev)
{
	Mpi2SmpPassthroughRequest_t *mpi_request;
	Mpi2SmpPassthroughReply_t *mpi_reply;
	struct rep_manu_reply *manufacture_reply;
	struct rep_manu_request *manufacture_request;
	int rc;
	u16 smid;
	u32 ioc_state;
	unsigned long timeleft;
	void *psge;
	u32 sgl_flags;
	u8 issue_reset = 0;
	void *data_out = NULL;
	dma_addr_t data_out_dma;
	u32 sz;
	u64 *sas_address_le;
	u16 wait_state_count;

	if (ioc->shost_recovery) {
		printk(MPT2SAS_INFO_FMT "%s: host reset in progress!\n",
		    __func__, ioc->name);
		return -EFAULT;
	}

	mutex_lock(&ioc->transport_cmds.mutex);

	if (ioc->transport_cmds.status != MPT2_CMD_NOT_USED) {
		printk(MPT2SAS_ERR_FMT "%s: transport_cmds in use\n",
		    ioc->name, __func__);
		rc = -EAGAIN;
		goto out;
	}
	ioc->transport_cmds.status = MPT2_CMD_PENDING;

	wait_state_count = 0;
	ioc_state = mpt2sas_base_get_iocstate(ioc, 1);
	while (ioc_state != MPI2_IOC_STATE_OPERATIONAL) {
		if (wait_state_count++ == 10) {
			printk(MPT2SAS_ERR_FMT
			    "%s: failed due to ioc not operational\n",
			    ioc->name, __func__);
			rc = -EFAULT;
			goto out;
		}
		ssleep(1);
		ioc_state = mpt2sas_base_get_iocstate(ioc, 1);
		printk(MPT2SAS_INFO_FMT "%s: waiting for "
		    "operational state(count=%d)\n", ioc->name,
		    __func__, wait_state_count);
	}
	if (wait_state_count)
		printk(MPT2SAS_INFO_FMT "%s: ioc is operational\n",
		    ioc->name, __func__);

	smid = mpt2sas_base_get_smid(ioc, ioc->transport_cb_idx);
	if (!smid) {
		printk(MPT2SAS_ERR_FMT "%s: failed obtaining a smid\n",
		    ioc->name, __func__);
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	mpi_request = mpt2sas_base_get_msg_frame(ioc, smid);
	ioc->transport_cmds.smid = smid;

	sz = sizeof(struct rep_manu_request) + sizeof(struct rep_manu_reply);
	data_out = pci_alloc_consistent(ioc->pdev, sz, &data_out_dma);

	if (!data_out) {
		printk(KERN_ERR "failure at %s:%d/%s()!\n", __FILE__,
		    __LINE__, __func__);
		rc = -ENOMEM;
		mpt2sas_base_free_smid(ioc, smid);
		goto out;
	}

	manufacture_request = data_out;
	manufacture_request->smp_frame_type = 0x40;
	manufacture_request->function = 1;
	manufacture_request->reserved = 0;
	manufacture_request->request_length = 0;

	memset(mpi_request, 0, sizeof(Mpi2SmpPassthroughRequest_t));
	mpi_request->Function = MPI2_FUNCTION_SMP_PASSTHROUGH;
	mpi_request->PhysicalPort = 0xFF;
	mpi_request->VF_ID = 0; 
	mpi_request->VP_ID = 0;
	sas_address_le = (u64 *)&mpi_request->SASAddress;
	*sas_address_le = cpu_to_le64(sas_address);
	mpi_request->RequestDataLength = sizeof(struct rep_manu_request);
	psge = &mpi_request->SGL;

	
	sgl_flags = (MPI2_SGE_FLAGS_SIMPLE_ELEMENT |
	    MPI2_SGE_FLAGS_END_OF_BUFFER | MPI2_SGE_FLAGS_HOST_TO_IOC);
	sgl_flags = sgl_flags << MPI2_SGE_FLAGS_SHIFT;
	ioc->base_add_sg_single(psge, sgl_flags |
	    sizeof(struct rep_manu_request), data_out_dma);

	
	psge += ioc->sge_size;

	
	sgl_flags = (MPI2_SGE_FLAGS_SIMPLE_ELEMENT |
	    MPI2_SGE_FLAGS_LAST_ELEMENT | MPI2_SGE_FLAGS_END_OF_BUFFER |
	    MPI2_SGE_FLAGS_END_OF_LIST);
	sgl_flags = sgl_flags << MPI2_SGE_FLAGS_SHIFT;
	ioc->base_add_sg_single(psge, sgl_flags |
	    sizeof(struct rep_manu_reply), data_out_dma +
	    sizeof(struct rep_manu_request));

	dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT "report_manufacture - "
	    "send to sas_addr(0x%016llx)\n", ioc->name,
	    (unsigned long long)sas_address));
	mpt2sas_base_put_smid_default(ioc, smid);
	init_completion(&ioc->transport_cmds.done);
	timeleft = wait_for_completion_timeout(&ioc->transport_cmds.done,
	    10*HZ);

	if (!(ioc->transport_cmds.status & MPT2_CMD_COMPLETE)) {
		printk(MPT2SAS_ERR_FMT "%s: timeout\n",
		    ioc->name, __func__);
		_debug_dump_mf(mpi_request,
		    sizeof(Mpi2SmpPassthroughRequest_t)/4);
		if (!(ioc->transport_cmds.status & MPT2_CMD_RESET))
			issue_reset = 1;
		goto issue_host_reset;
	}

	dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT "report_manufacture - "
	    "complete\n", ioc->name));

	if (ioc->transport_cmds.status & MPT2_CMD_REPLY_VALID) {
		u8 *tmp;

		mpi_reply = ioc->transport_cmds.reply;

		dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT
		    "report_manufacture - reply data transfer size(%d)\n",
		    ioc->name, le16_to_cpu(mpi_reply->ResponseDataLength)));

		if (le16_to_cpu(mpi_reply->ResponseDataLength) !=
		    sizeof(struct rep_manu_reply))
			goto out;

		manufacture_reply = data_out + sizeof(struct rep_manu_request);
		strncpy(edev->vendor_id, manufacture_reply->vendor_id,
		     SAS_EXPANDER_VENDOR_ID_LEN);
		strncpy(edev->product_id, manufacture_reply->product_id,
		     SAS_EXPANDER_PRODUCT_ID_LEN);
		strncpy(edev->product_rev, manufacture_reply->product_rev,
		     SAS_EXPANDER_PRODUCT_REV_LEN);
		edev->level = manufacture_reply->sas_format;
		if (manufacture_reply->sas_format) {
			strncpy(edev->component_vendor_id,
			    manufacture_reply->component_vendor_id,
			     SAS_EXPANDER_COMPONENT_VENDOR_ID_LEN);
			tmp = (u8 *)&manufacture_reply->component_id;
			edev->component_id = tmp[0] << 8 | tmp[1];
			edev->component_revision_id =
			    manufacture_reply->component_revision_id;
		}
	} else
		dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT
		    "report_manufacture - no reply\n", ioc->name));

 issue_host_reset:
	if (issue_reset)
		mpt2sas_base_hard_reset_handler(ioc, CAN_SLEEP,
		    FORCE_BIG_HAMMER);
 out:
	ioc->transport_cmds.status = MPT2_CMD_NOT_USED;
	if (data_out)
		pci_free_consistent(ioc->pdev, sz, data_out, data_out_dma);

	mutex_unlock(&ioc->transport_cmds.mutex);
	return rc;
}


struct _sas_port *
mpt2sas_transport_port_add(struct MPT2SAS_ADAPTER *ioc, u16 handle,
    u16 parent_handle)
{
	struct _sas_phy *mpt2sas_phy, *next;
	struct _sas_port *mpt2sas_port;
	unsigned long flags;
	struct _sas_node *sas_node;
	struct sas_rphy *rphy;
	int i;
	struct sas_port *port;

	if (!parent_handle)
		return NULL;

	mpt2sas_port = kzalloc(sizeof(struct _sas_port),
	    GFP_KERNEL);
	if (!mpt2sas_port) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return NULL;
	}

	INIT_LIST_HEAD(&mpt2sas_port->port_list);
	INIT_LIST_HEAD(&mpt2sas_port->phy_list);
	spin_lock_irqsave(&ioc->sas_node_lock, flags);
	sas_node = _transport_sas_node_find_by_handle(ioc, parent_handle);
	spin_unlock_irqrestore(&ioc->sas_node_lock, flags);

	if (!sas_node) {
		printk(MPT2SAS_ERR_FMT "%s: Could not find parent(0x%04x)!\n",
		    ioc->name, __func__, parent_handle);
		goto out_fail;
	}

	mpt2sas_port->handle = parent_handle;
	mpt2sas_port->sas_address = sas_node->sas_address;
	if ((_transport_set_identify(ioc, handle,
	    &mpt2sas_port->remote_identify))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		goto out_fail;
	}

	if (mpt2sas_port->remote_identify.device_type == SAS_PHY_UNUSED) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		goto out_fail;
	}

	for (i = 0; i < sas_node->num_phys; i++) {
		if (sas_node->phy[i].remote_identify.sas_address !=
		    mpt2sas_port->remote_identify.sas_address)
			continue;
		list_add_tail(&sas_node->phy[i].port_siblings,
		    &mpt2sas_port->phy_list);
		mpt2sas_port->num_phys++;
	}

	if (!mpt2sas_port->num_phys) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		goto out_fail;
	}

	port = sas_port_alloc_num(sas_node->parent_dev);
	if ((sas_port_add(port))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		goto out_fail;
	}

	list_for_each_entry(mpt2sas_phy, &mpt2sas_port->phy_list,
	    port_siblings) {
		if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
			dev_printk(KERN_INFO, &port->dev, "add: handle(0x%04x)"
			    ", sas_addr(0x%016llx), phy(%d)\n", handle,
			    (unsigned long long)
			    mpt2sas_port->remote_identify.sas_address,
			    mpt2sas_phy->phy_id);
		sas_port_add_phy(port, mpt2sas_phy->phy);
	}

	mpt2sas_port->port = port;
	if (mpt2sas_port->remote_identify.device_type == SAS_END_DEVICE)
		rphy = sas_end_device_alloc(port);
	else
		rphy = sas_expander_alloc(port,
		    mpt2sas_port->remote_identify.device_type);

	rphy->identify = mpt2sas_port->remote_identify;
	if ((sas_rphy_add(rphy))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
	}
	if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
		dev_printk(KERN_INFO, &rphy->dev, "add: handle(0x%04x), "
		    "sas_addr(0x%016llx)\n", handle,
		    (unsigned long long)
		    mpt2sas_port->remote_identify.sas_address);
	mpt2sas_port->rphy = rphy;
	spin_lock_irqsave(&ioc->sas_node_lock, flags);
	list_add_tail(&mpt2sas_port->port_list, &sas_node->sas_port_list);
	spin_unlock_irqrestore(&ioc->sas_node_lock, flags);

	
	if (mpt2sas_port->remote_identify.device_type ==
	    MPI2_SAS_DEVICE_INFO_EDGE_EXPANDER ||
	    mpt2sas_port->remote_identify.device_type ==
	    MPI2_SAS_DEVICE_INFO_FANOUT_EXPANDER)
		_transport_expander_report_manufacture(ioc,
		    mpt2sas_port->remote_identify.sas_address,
		    rphy_to_expander_device(rphy));

	return mpt2sas_port;

 out_fail:
	list_for_each_entry_safe(mpt2sas_phy, next, &mpt2sas_port->phy_list,
	    port_siblings)
		list_del(&mpt2sas_phy->port_siblings);
	kfree(mpt2sas_port);
	return NULL;
}


void
mpt2sas_transport_port_remove(struct MPT2SAS_ADAPTER *ioc, u64 sas_address,
    u16 parent_handle)
{
	int i;
	unsigned long flags;
	struct _sas_port *mpt2sas_port, *next;
	struct _sas_node *sas_node;
	u8 found = 0;
	struct _sas_phy *mpt2sas_phy, *next_phy;

	spin_lock_irqsave(&ioc->sas_node_lock, flags);
	sas_node = _transport_sas_node_find_by_handle(ioc, parent_handle);
	spin_unlock_irqrestore(&ioc->sas_node_lock, flags);
	if (!sas_node)
		return;
	list_for_each_entry_safe(mpt2sas_port, next, &sas_node->sas_port_list,
	    port_list) {
		if (mpt2sas_port->remote_identify.sas_address != sas_address)
			continue;
		found = 1;
		list_del(&mpt2sas_port->port_list);
		goto out;
	}
 out:
	if (!found)
		return;

	for (i = 0; i < sas_node->num_phys; i++) {
		if (sas_node->phy[i].remote_identify.sas_address == sas_address)
			memset(&sas_node->phy[i].remote_identify, 0 ,
			    sizeof(struct sas_identify));
	}

	list_for_each_entry_safe(mpt2sas_phy, next_phy,
	    &mpt2sas_port->phy_list, port_siblings) {
		if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
			dev_printk(KERN_INFO, &mpt2sas_port->port->dev,
			    "remove: parent_handle(0x%04x), "
			    "sas_addr(0x%016llx), phy(%d)\n", parent_handle,
			    (unsigned long long)
			    mpt2sas_port->remote_identify.sas_address,
			    mpt2sas_phy->phy_id);
		sas_port_delete_phy(mpt2sas_port->port, mpt2sas_phy->phy);
		list_del(&mpt2sas_phy->port_siblings);
	}
	sas_port_delete(mpt2sas_port->port);
	kfree(mpt2sas_port);
}


int
mpt2sas_transport_add_host_phy(struct MPT2SAS_ADAPTER *ioc, struct _sas_phy
    *mpt2sas_phy, Mpi2SasPhyPage0_t phy_pg0, struct device *parent_dev)
{
	struct sas_phy *phy;
	int phy_index = mpt2sas_phy->phy_id;


	INIT_LIST_HEAD(&mpt2sas_phy->port_siblings);
	phy = sas_phy_alloc(parent_dev, phy_index);
	if (!phy) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -1;
	}
	if ((_transport_set_identify(ioc, mpt2sas_phy->handle,
	    &mpt2sas_phy->identify))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -1;
	}
	phy->identify = mpt2sas_phy->identify;
	mpt2sas_phy->attached_handle = le16_to_cpu(phy_pg0.AttachedDevHandle);
	if (mpt2sas_phy->attached_handle)
		_transport_set_identify(ioc, mpt2sas_phy->attached_handle,
		    &mpt2sas_phy->remote_identify);
	phy->identify.phy_identifier = mpt2sas_phy->phy_id;
	phy->negotiated_linkrate = _transport_convert_phy_link_rate(
	    phy_pg0.NegotiatedLinkRate & MPI2_SAS_NEG_LINK_RATE_MASK_PHYSICAL);
	phy->minimum_linkrate_hw = _transport_convert_phy_link_rate(
	    phy_pg0.HwLinkRate & MPI2_SAS_HWRATE_MIN_RATE_MASK);
	phy->maximum_linkrate_hw = _transport_convert_phy_link_rate(
	    phy_pg0.HwLinkRate >> 4);
	phy->minimum_linkrate = _transport_convert_phy_link_rate(
	    phy_pg0.ProgrammedLinkRate & MPI2_SAS_PRATE_MIN_RATE_MASK);
	phy->maximum_linkrate = _transport_convert_phy_link_rate(
	    phy_pg0.ProgrammedLinkRate >> 4);

	if ((sas_phy_add(phy))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		sas_phy_free(phy);
		return -1;
	}
	if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
		dev_printk(KERN_INFO, &phy->dev,
		    "add: handle(0x%04x), sas_addr(0x%016llx)\n"
		    "\tattached_handle(0x%04x), sas_addr(0x%016llx)\n",
		    mpt2sas_phy->handle, (unsigned long long)
		    mpt2sas_phy->identify.sas_address,
		    mpt2sas_phy->attached_handle,
		    (unsigned long long)
		    mpt2sas_phy->remote_identify.sas_address);
	mpt2sas_phy->phy = phy;
	return 0;
}



int
mpt2sas_transport_add_expander_phy(struct MPT2SAS_ADAPTER *ioc, struct _sas_phy
    *mpt2sas_phy, Mpi2ExpanderPage1_t expander_pg1, struct device *parent_dev)
{
	struct sas_phy *phy;
	int phy_index = mpt2sas_phy->phy_id;

	INIT_LIST_HEAD(&mpt2sas_phy->port_siblings);
	phy = sas_phy_alloc(parent_dev, phy_index);
	if (!phy) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -1;
	}
	if ((_transport_set_identify(ioc, mpt2sas_phy->handle,
	    &mpt2sas_phy->identify))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -1;
	}
	phy->identify = mpt2sas_phy->identify;
	mpt2sas_phy->attached_handle =
	    le16_to_cpu(expander_pg1.AttachedDevHandle);
	if (mpt2sas_phy->attached_handle)
		_transport_set_identify(ioc, mpt2sas_phy->attached_handle,
		    &mpt2sas_phy->remote_identify);
	phy->identify.phy_identifier = mpt2sas_phy->phy_id;
	phy->negotiated_linkrate = _transport_convert_phy_link_rate(
	    expander_pg1.NegotiatedLinkRate &
	    MPI2_SAS_NEG_LINK_RATE_MASK_PHYSICAL);
	phy->minimum_linkrate_hw = _transport_convert_phy_link_rate(
	    expander_pg1.HwLinkRate & MPI2_SAS_HWRATE_MIN_RATE_MASK);
	phy->maximum_linkrate_hw = _transport_convert_phy_link_rate(
	    expander_pg1.HwLinkRate >> 4);
	phy->minimum_linkrate = _transport_convert_phy_link_rate(
	    expander_pg1.ProgrammedLinkRate & MPI2_SAS_PRATE_MIN_RATE_MASK);
	phy->maximum_linkrate = _transport_convert_phy_link_rate(
	    expander_pg1.ProgrammedLinkRate >> 4);

	if ((sas_phy_add(phy))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		sas_phy_free(phy);
		return -1;
	}
	if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
		dev_printk(KERN_INFO, &phy->dev,
		    "add: handle(0x%04x), sas_addr(0x%016llx)\n"
		    "\tattached_handle(0x%04x), sas_addr(0x%016llx)\n",
		    mpt2sas_phy->handle, (unsigned long long)
		    mpt2sas_phy->identify.sas_address,
		    mpt2sas_phy->attached_handle,
		    (unsigned long long)
		    mpt2sas_phy->remote_identify.sas_address);
	mpt2sas_phy->phy = phy;
	return 0;
}


void
mpt2sas_transport_update_links(struct MPT2SAS_ADAPTER *ioc,
    u16 handle, u16 attached_handle, u8 phy_number, u8 link_rate)
{
	unsigned long flags;
	struct _sas_node *sas_node;
	struct _sas_phy *mpt2sas_phy;

	if (ioc->shost_recovery) {
		printk(MPT2SAS_INFO_FMT "%s: host reset in progress!\n",
			__func__, ioc->name);
		return;
	}

	spin_lock_irqsave(&ioc->sas_node_lock, flags);
	sas_node = _transport_sas_node_find_by_handle(ioc, handle);
	spin_unlock_irqrestore(&ioc->sas_node_lock, flags);
	if (!sas_node)
		return;

	mpt2sas_phy = &sas_node->phy[phy_number];
	mpt2sas_phy->attached_handle = attached_handle;
	if (attached_handle && (link_rate >= MPI2_SAS_NEG_LINK_RATE_1_5))
		_transport_set_identify(ioc, mpt2sas_phy->attached_handle,
		    &mpt2sas_phy->remote_identify);
	else
		memset(&mpt2sas_phy->remote_identify, 0 , sizeof(struct
		    sas_identify));

	if (mpt2sas_phy->phy)
		mpt2sas_phy->phy->negotiated_linkrate =
		    _transport_convert_phy_link_rate(link_rate);

	if ((ioc->logging_level & MPT_DEBUG_TRANSPORT))
		dev_printk(KERN_INFO, &mpt2sas_phy->phy->dev,
		    "refresh: handle(0x%04x), sas_addr(0x%016llx),\n"
		    "\tlink_rate(0x%02x), phy(%d)\n"
		    "\tattached_handle(0x%04x), sas_addr(0x%016llx)\n",
		    handle, (unsigned long long)
		    mpt2sas_phy->identify.sas_address, link_rate,
		    phy_number, attached_handle,
		    (unsigned long long)
		    mpt2sas_phy->remote_identify.sas_address);
}

static inline void *
phy_to_ioc(struct sas_phy *phy)
{
	struct Scsi_Host *shost = dev_to_shost(phy->dev.parent);
	return shost_priv(shost);
}

static inline void *
rphy_to_ioc(struct sas_rphy *rphy)
{
	struct Scsi_Host *shost = dev_to_shost(rphy->dev.parent->parent);
	return shost_priv(shost);
}


static int
_transport_get_linkerrors(struct sas_phy *phy)
{
	struct MPT2SAS_ADAPTER *ioc = phy_to_ioc(phy);
	struct _sas_phy *mpt2sas_phy;
	Mpi2ConfigReply_t mpi_reply;
	Mpi2SasPhyPage1_t phy_pg1;
	int i;

	for (i = 0, mpt2sas_phy = NULL; i < ioc->sas_hba.num_phys &&
	    !mpt2sas_phy; i++) {
		if (ioc->sas_hba.phy[i].phy != phy)
			continue;
		mpt2sas_phy = &ioc->sas_hba.phy[i];
	}

	if (!mpt2sas_phy) 
		return -EINVAL;

	if ((mpt2sas_config_get_phy_pg1(ioc, &mpi_reply, &phy_pg1,
		    mpt2sas_phy->phy_id))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -ENXIO;
	}

	if (mpi_reply.IOCStatus || mpi_reply.IOCLogInfo)
		printk(MPT2SAS_INFO_FMT "phy(%d), ioc_status"
		    "(0x%04x), loginfo(0x%08x)\n", ioc->name,
		    mpt2sas_phy->phy_id,
		    le16_to_cpu(mpi_reply.IOCStatus),
		    le32_to_cpu(mpi_reply.IOCLogInfo));

	phy->invalid_dword_count = le32_to_cpu(phy_pg1.InvalidDwordCount);
	phy->running_disparity_error_count =
	    le32_to_cpu(phy_pg1.RunningDisparityErrorCount);
	phy->loss_of_dword_sync_count =
	    le32_to_cpu(phy_pg1.LossDwordSynchCount);
	phy->phy_reset_problem_count =
	    le32_to_cpu(phy_pg1.PhyResetProblemCount);
	return 0;
}


static int
_transport_get_enclosure_identifier(struct sas_rphy *rphy, u64 *identifier)
{
	struct MPT2SAS_ADAPTER *ioc = rphy_to_ioc(rphy);
	struct _sas_node *sas_expander;
	unsigned long flags;

	spin_lock_irqsave(&ioc->sas_node_lock, flags);
	sas_expander = mpt2sas_scsih_expander_find_by_sas_address(ioc,
	    rphy->identify.sas_address);
	spin_unlock_irqrestore(&ioc->sas_node_lock, flags);

	if (!sas_expander)
		return -ENXIO;

	*identifier = sas_expander->enclosure_logical_id;
	return 0;
}


static int
_transport_get_bay_identifier(struct sas_rphy *rphy)
{
	struct MPT2SAS_ADAPTER *ioc = rphy_to_ioc(rphy);
	struct _sas_device *sas_device;
	unsigned long flags;

	spin_lock_irqsave(&ioc->sas_device_lock, flags);
	sas_device = mpt2sas_scsih_sas_device_find_by_sas_address(ioc,
	    rphy->identify.sas_address);
	spin_unlock_irqrestore(&ioc->sas_device_lock, flags);

	if (!sas_device)
		return -ENXIO;

	return sas_device->slot;
}


static int
_transport_phy_reset(struct sas_phy *phy, int hard_reset)
{
	struct MPT2SAS_ADAPTER *ioc = phy_to_ioc(phy);
	struct _sas_phy *mpt2sas_phy;
	Mpi2SasIoUnitControlReply_t mpi_reply;
	Mpi2SasIoUnitControlRequest_t mpi_request;
	int i;

	for (i = 0, mpt2sas_phy = NULL; i < ioc->sas_hba.num_phys &&
	    !mpt2sas_phy; i++) {
		if (ioc->sas_hba.phy[i].phy != phy)
			continue;
		mpt2sas_phy = &ioc->sas_hba.phy[i];
	}

	if (!mpt2sas_phy) 
		return -EINVAL;

	memset(&mpi_request, 0, sizeof(Mpi2SasIoUnitControlReply_t));
	mpi_request.Function = MPI2_FUNCTION_SAS_IO_UNIT_CONTROL;
	mpi_request.Operation = hard_reset ?
	    MPI2_SAS_OP_PHY_HARD_RESET : MPI2_SAS_OP_PHY_LINK_RESET;
	mpi_request.PhyNum = mpt2sas_phy->phy_id;

	if ((mpt2sas_base_sas_iounit_control(ioc, &mpi_reply, &mpi_request))) {
		printk(MPT2SAS_ERR_FMT "failure at %s:%d/%s()!\n",
		    ioc->name, __FILE__, __LINE__, __func__);
		return -ENXIO;
	}

	if (mpi_reply.IOCStatus || mpi_reply.IOCLogInfo)
		printk(MPT2SAS_INFO_FMT "phy(%d), ioc_status"
		    "(0x%04x), loginfo(0x%08x)\n", ioc->name,
		    mpt2sas_phy->phy_id,
		    le16_to_cpu(mpi_reply.IOCStatus),
		    le32_to_cpu(mpi_reply.IOCLogInfo));

	return 0;
}


static int
_transport_smp_handler(struct Scsi_Host *shost, struct sas_rphy *rphy,
    struct request *req)
{
	struct MPT2SAS_ADAPTER *ioc = shost_priv(shost);
	Mpi2SmpPassthroughRequest_t *mpi_request;
	Mpi2SmpPassthroughReply_t *mpi_reply;
	int rc;
	u16 smid;
	u32 ioc_state;
	unsigned long timeleft;
	void *psge;
	u32 sgl_flags;
	u8 issue_reset = 0;
	dma_addr_t dma_addr_in = 0;
	dma_addr_t dma_addr_out = 0;
	u16 wait_state_count;
	struct request *rsp = req->next_rq;

	if (!rsp) {
		printk(MPT2SAS_ERR_FMT "%s: the smp response space is "
		    "missing\n", ioc->name, __func__);
		return -EINVAL;
	}

	
	if (req->bio->bi_vcnt > 1 || rsp->bio->bi_vcnt > 1) {
		printk(MPT2SAS_ERR_FMT "%s: multiple segments req %u %u, "
		    "rsp %u %u\n", ioc->name, __func__, req->bio->bi_vcnt,
		    blk_rq_bytes(req), rsp->bio->bi_vcnt, blk_rq_bytes(rsp));
		return -EINVAL;
	}

	if (ioc->shost_recovery) {
		printk(MPT2SAS_INFO_FMT "%s: host reset in progress!\n",
		    __func__, ioc->name);
		return -EFAULT;
	}

	rc = mutex_lock_interruptible(&ioc->transport_cmds.mutex);
	if (rc)
		return rc;

	if (ioc->transport_cmds.status != MPT2_CMD_NOT_USED) {
		printk(MPT2SAS_ERR_FMT "%s: transport_cmds in use\n", ioc->name,
		    __func__);
		rc = -EAGAIN;
		goto out;
	}
	ioc->transport_cmds.status = MPT2_CMD_PENDING;

	wait_state_count = 0;
	ioc_state = mpt2sas_base_get_iocstate(ioc, 1);
	while (ioc_state != MPI2_IOC_STATE_OPERATIONAL) {
		if (wait_state_count++ == 10) {
			printk(MPT2SAS_ERR_FMT
			    "%s: failed due to ioc not operational\n",
			    ioc->name, __func__);
			rc = -EFAULT;
			goto out;
		}
		ssleep(1);
		ioc_state = mpt2sas_base_get_iocstate(ioc, 1);
		printk(MPT2SAS_INFO_FMT "%s: waiting for "
		    "operational state(count=%d)\n", ioc->name,
		    __func__, wait_state_count);
	}
	if (wait_state_count)
		printk(MPT2SAS_INFO_FMT "%s: ioc is operational\n",
		    ioc->name, __func__);

	smid = mpt2sas_base_get_smid(ioc, ioc->transport_cb_idx);
	if (!smid) {
		printk(MPT2SAS_ERR_FMT "%s: failed obtaining a smid\n",
		    ioc->name, __func__);
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	mpi_request = mpt2sas_base_get_msg_frame(ioc, smid);
	ioc->transport_cmds.smid = smid;

	memset(mpi_request, 0, sizeof(Mpi2SmpPassthroughRequest_t));
	mpi_request->Function = MPI2_FUNCTION_SMP_PASSTHROUGH;
	mpi_request->PhysicalPort = 0xFF;
	mpi_request->VF_ID = 0; 
	mpi_request->VP_ID = 0;
	*((u64 *)&mpi_request->SASAddress) = (rphy) ?
	    cpu_to_le64(rphy->identify.sas_address) :
	    cpu_to_le64(ioc->sas_hba.sas_address);
	mpi_request->RequestDataLength = cpu_to_le16(blk_rq_bytes(req) - 4);
	psge = &mpi_request->SGL;

	
	sgl_flags = (MPI2_SGE_FLAGS_SIMPLE_ELEMENT |
	    MPI2_SGE_FLAGS_END_OF_BUFFER | MPI2_SGE_FLAGS_HOST_TO_IOC);
	sgl_flags = sgl_flags << MPI2_SGE_FLAGS_SHIFT;
	dma_addr_out = pci_map_single(ioc->pdev, bio_data(req->bio),
		blk_rq_bytes(req), PCI_DMA_BIDIRECTIONAL);
	if (!dma_addr_out) {
		mpt2sas_base_free_smid(ioc, le16_to_cpu(smid));
		goto unmap;
	}

	ioc->base_add_sg_single(psge, sgl_flags | (blk_rq_bytes(req) - 4),
	    dma_addr_out);

	
	psge += ioc->sge_size;

	
	sgl_flags = (MPI2_SGE_FLAGS_SIMPLE_ELEMENT |
	    MPI2_SGE_FLAGS_LAST_ELEMENT | MPI2_SGE_FLAGS_END_OF_BUFFER |
	    MPI2_SGE_FLAGS_END_OF_LIST);
	sgl_flags = sgl_flags << MPI2_SGE_FLAGS_SHIFT;
	dma_addr_in = pci_map_single(ioc->pdev, bio_data(rsp->bio),
				     blk_rq_bytes(rsp), PCI_DMA_BIDIRECTIONAL);
	if (!dma_addr_in) {
		mpt2sas_base_free_smid(ioc, le16_to_cpu(smid));
		goto unmap;
	}

	ioc->base_add_sg_single(psge, sgl_flags | (blk_rq_bytes(rsp) + 4),
	    dma_addr_in);

	dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT "%s - "
	    "sending smp request\n", ioc->name, __func__));

	mpt2sas_base_put_smid_default(ioc, smid);
	init_completion(&ioc->transport_cmds.done);
	timeleft = wait_for_completion_timeout(&ioc->transport_cmds.done,
	    10*HZ);

	if (!(ioc->transport_cmds.status & MPT2_CMD_COMPLETE)) {
		printk(MPT2SAS_ERR_FMT "%s : timeout\n",
		    __func__, ioc->name);
		_debug_dump_mf(mpi_request,
		    sizeof(Mpi2SmpPassthroughRequest_t)/4);
		if (!(ioc->transport_cmds.status & MPT2_CMD_RESET))
			issue_reset = 1;
		goto issue_host_reset;
	}

	dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT "%s - "
	    "complete\n", ioc->name, __func__));

	if (ioc->transport_cmds.status & MPT2_CMD_REPLY_VALID) {

		mpi_reply = ioc->transport_cmds.reply;

		dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT
		    "%s - reply data transfer size(%d)\n",
		    ioc->name, __func__,
		    le16_to_cpu(mpi_reply->ResponseDataLength)));

		memcpy(req->sense, mpi_reply, sizeof(*mpi_reply));
		req->sense_len = sizeof(*mpi_reply);
		req->resid_len = 0;
		rsp->resid_len -= mpi_reply->ResponseDataLength;
	} else {
		dtransportprintk(ioc, printk(MPT2SAS_DEBUG_FMT
		    "%s - no reply\n", ioc->name, __func__));
		rc = -ENXIO;
	}

 issue_host_reset:
	if (issue_reset) {
		mpt2sas_base_hard_reset_handler(ioc, CAN_SLEEP,
		    FORCE_BIG_HAMMER);
		rc = -ETIMEDOUT;
	}

 unmap:
	if (dma_addr_out)
		pci_unmap_single(ioc->pdev, dma_addr_out, blk_rq_bytes(req),
		    PCI_DMA_BIDIRECTIONAL);
	if (dma_addr_in)
		pci_unmap_single(ioc->pdev, dma_addr_in, blk_rq_bytes(rsp),
		    PCI_DMA_BIDIRECTIONAL);

 out:
	ioc->transport_cmds.status = MPT2_CMD_NOT_USED;
	mutex_unlock(&ioc->transport_cmds.mutex);
	return rc;
}

struct sas_function_template mpt2sas_transport_functions = {
	.get_linkerrors		= _transport_get_linkerrors,
	.get_enclosure_identifier = _transport_get_enclosure_identifier,
	.get_bay_identifier	= _transport_get_bay_identifier,
	.phy_reset		= _transport_phy_reset,
	.smp_handler		= _transport_smp_handler,
};

struct scsi_transport_template *mpt2sas_transport_template;
