
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include "wq_enet_desc.h"
#include "rq_enet_desc.h"
#include "cq_enet_desc.h"
#include "vnic_resource.h"
#include "vnic_dev.h"
#include "vnic_wq.h"
#include "vnic_rq.h"
#include "vnic_cq.h"
#include "vnic_intr.h"
#include "vnic_stats.h"
#include "vnic_nic.h"
#include "fnic.h"

int fnic_get_vnic_config(struct fnic *fnic)
{
	struct vnic_fc_config *c = &fnic->config;
	int err;

#define GET_CONFIG(m) \
	do { \
		err = vnic_dev_spec(fnic->vdev, \
				    offsetof(struct vnic_fc_config, m), \
				    sizeof(c->m), &c->m); \
		if (err) { \
			shost_printk(KERN_ERR, fnic->lport->host, \
				     "Error getting %s, %d\n", #m, \
				     err); \
			return err; \
		} \
	} while (0);

	GET_CONFIG(node_wwn);
	GET_CONFIG(port_wwn);
	GET_CONFIG(wq_enet_desc_count);
	GET_CONFIG(wq_copy_desc_count);
	GET_CONFIG(rq_desc_count);
	GET_CONFIG(maxdatafieldsize);
	GET_CONFIG(ed_tov);
	GET_CONFIG(ra_tov);
	GET_CONFIG(intr_timer);
	GET_CONFIG(intr_timer_type);
	GET_CONFIG(flags);
	GET_CONFIG(flogi_retries);
	GET_CONFIG(flogi_timeout);
	GET_CONFIG(plogi_retries);
	GET_CONFIG(plogi_timeout);
	GET_CONFIG(io_throttle_count);
	GET_CONFIG(link_down_timeout);
	GET_CONFIG(port_down_timeout);
	GET_CONFIG(port_down_io_retries);
	GET_CONFIG(luns_per_tgt);

	c->wq_enet_desc_count =
		min_t(u32, VNIC_FNIC_WQ_DESCS_MAX,
		      max_t(u32, VNIC_FNIC_WQ_DESCS_MIN,
			    c->wq_enet_desc_count));
	c->wq_enet_desc_count = ALIGN(c->wq_enet_desc_count, 16);

	c->wq_copy_desc_count =
		min_t(u32, VNIC_FNIC_WQ_COPY_DESCS_MAX,
		      max_t(u32, VNIC_FNIC_WQ_COPY_DESCS_MIN,
			    c->wq_copy_desc_count));
	c->wq_copy_desc_count = ALIGN(c->wq_copy_desc_count, 16);

	c->rq_desc_count =
		min_t(u32, VNIC_FNIC_RQ_DESCS_MAX,
		      max_t(u32, VNIC_FNIC_RQ_DESCS_MIN,
			    c->rq_desc_count));
	c->rq_desc_count = ALIGN(c->rq_desc_count, 16);

	c->maxdatafieldsize =
		min_t(u16, VNIC_FNIC_MAXDATAFIELDSIZE_MAX,
		      max_t(u16, VNIC_FNIC_MAXDATAFIELDSIZE_MIN,
			    c->maxdatafieldsize));
	c->ed_tov =
		min_t(u32, VNIC_FNIC_EDTOV_MAX,
		      max_t(u32, VNIC_FNIC_EDTOV_MIN,
			    c->ed_tov));

	c->ra_tov =
		min_t(u32, VNIC_FNIC_RATOV_MAX,
		      max_t(u32, VNIC_FNIC_RATOV_MIN,
			    c->ra_tov));

	c->flogi_retries =
		min_t(u32, VNIC_FNIC_FLOGI_RETRIES_MAX, c->flogi_retries);

	c->flogi_timeout =
		min_t(u32, VNIC_FNIC_FLOGI_TIMEOUT_MAX,
		      max_t(u32, VNIC_FNIC_FLOGI_TIMEOUT_MIN,
			    c->flogi_timeout));

	c->plogi_retries =
		min_t(u32, VNIC_FNIC_PLOGI_RETRIES_MAX, c->plogi_retries);

	c->plogi_timeout =
		min_t(u32, VNIC_FNIC_PLOGI_TIMEOUT_MAX,
		      max_t(u32, VNIC_FNIC_PLOGI_TIMEOUT_MIN,
			    c->plogi_timeout));

	c->io_throttle_count =
		min_t(u32, VNIC_FNIC_IO_THROTTLE_COUNT_MAX,
		      max_t(u32, VNIC_FNIC_IO_THROTTLE_COUNT_MIN,
			    c->io_throttle_count));

	c->link_down_timeout =
		min_t(u32, VNIC_FNIC_LINK_DOWN_TIMEOUT_MAX,
		      c->link_down_timeout);

	c->port_down_timeout =
		min_t(u32, VNIC_FNIC_PORT_DOWN_TIMEOUT_MAX,
		      c->port_down_timeout);

	c->port_down_io_retries =
		min_t(u32, VNIC_FNIC_PORT_DOWN_IO_RETRIES_MAX,
		      c->port_down_io_retries);

	c->luns_per_tgt =
		min_t(u32, VNIC_FNIC_LUNS_PER_TARGET_MAX,
		      max_t(u32, VNIC_FNIC_LUNS_PER_TARGET_MIN,
			    c->luns_per_tgt));

	c->intr_timer = min_t(u16, VNIC_INTR_TIMER_MAX, c->intr_timer);
	c->intr_timer_type = c->intr_timer_type;

	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC MAC addr %02x:%02x:%02x:%02x:%02x:%02x "
		     "wq/wq_copy/rq %d/%d/%d\n",
		     fnic->mac_addr[0], fnic->mac_addr[1], fnic->mac_addr[2],
		     fnic->mac_addr[3], fnic->mac_addr[4], fnic->mac_addr[5],
		     c->wq_enet_desc_count, c->wq_copy_desc_count,
		     c->rq_desc_count);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC node wwn %llx port wwn %llx\n",
		     c->node_wwn, c->port_wwn);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC ed_tov %d ra_tov %d\n",
		     c->ed_tov, c->ra_tov);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC mtu %d intr timer %d\n",
		     c->maxdatafieldsize, c->intr_timer);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC flags 0x%x luns per tgt %d\n",
		     c->flags, c->luns_per_tgt);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC flogi_retries %d flogi timeout %d\n",
		     c->flogi_retries, c->flogi_timeout);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC plogi retries %d plogi timeout %d\n",
		     c->plogi_retries, c->plogi_timeout);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC io throttle count %d link dn timeout %d\n",
		     c->io_throttle_count, c->link_down_timeout);
	shost_printk(KERN_INFO, fnic->lport->host,
		     "vNIC port dn io retries %d port dn timeout %d\n",
		     c->port_down_io_retries, c->port_down_timeout);

	return 0;
}

int fnic_set_nic_config(struct fnic *fnic, u8 rss_default_cpu,
			u8 rss_hash_type,
			u8 rss_hash_bits, u8 rss_base_cpu, u8 rss_enable,
			u8 tso_ipid_split_en, u8 ig_vlan_strip_en)
{
	u64 a0, a1;
	u32 nic_cfg;
	int wait = 1000;

	vnic_set_nic_cfg(&nic_cfg, rss_default_cpu,
		rss_hash_type, rss_hash_bits, rss_base_cpu,
		rss_enable, tso_ipid_split_en, ig_vlan_strip_en);

	a0 = nic_cfg;
	a1 = 0;

	return vnic_dev_cmd(fnic->vdev, CMD_NIC_CFG, &a0, &a1, wait);
}

void fnic_get_res_counts(struct fnic *fnic)
{
	fnic->wq_count = vnic_dev_get_res_count(fnic->vdev, RES_TYPE_WQ);
	fnic->raw_wq_count = fnic->wq_count - 1;
	fnic->wq_copy_count = fnic->wq_count - fnic->raw_wq_count;
	fnic->rq_count = vnic_dev_get_res_count(fnic->vdev, RES_TYPE_RQ);
	fnic->cq_count = vnic_dev_get_res_count(fnic->vdev, RES_TYPE_CQ);
	fnic->intr_count = vnic_dev_get_res_count(fnic->vdev,
		RES_TYPE_INTR_CTRL);
}

void fnic_free_vnic_resources(struct fnic *fnic)
{
	unsigned int i;

	for (i = 0; i < fnic->raw_wq_count; i++)
		vnic_wq_free(&fnic->wq[i]);

	for (i = 0; i < fnic->wq_copy_count; i++)
		vnic_wq_copy_free(&fnic->wq_copy[i]);

	for (i = 0; i < fnic->rq_count; i++)
		vnic_rq_free(&fnic->rq[i]);

	for (i = 0; i < fnic->cq_count; i++)
		vnic_cq_free(&fnic->cq[i]);

	for (i = 0; i < fnic->intr_count; i++)
		vnic_intr_free(&fnic->intr[i]);
}

int fnic_alloc_vnic_resources(struct fnic *fnic)
{
	enum vnic_dev_intr_mode intr_mode;
	unsigned int mask_on_assertion;
	unsigned int interrupt_offset;
	unsigned int error_interrupt_enable;
	unsigned int error_interrupt_offset;
	unsigned int i, cq_index;
	unsigned int wq_copy_cq_desc_count;
	int err;

	intr_mode = vnic_dev_get_intr_mode(fnic->vdev);

	shost_printk(KERN_INFO, fnic->lport->host, "vNIC interrupt mode: %s\n",
		     intr_mode == VNIC_DEV_INTR_MODE_INTX ? "legacy PCI INTx" :
		     intr_mode == VNIC_DEV_INTR_MODE_MSI ? "MSI" :
		     intr_mode == VNIC_DEV_INTR_MODE_MSIX ?
		     "MSI-X" : "unknown");

	shost_printk(KERN_INFO, fnic->lport->host, "vNIC resources avail: "
		     "wq %d cp_wq %d raw_wq %d rq %d cq %d intr %d\n",
		     fnic->wq_count, fnic->wq_copy_count, fnic->raw_wq_count,
		     fnic->rq_count, fnic->cq_count, fnic->intr_count);

	
	for (i = 0; i < fnic->raw_wq_count; i++) {
		err = vnic_wq_alloc(fnic->vdev, &fnic->wq[i], i,
			fnic->config.wq_enet_desc_count,
			sizeof(struct wq_enet_desc));
		if (err)
			goto err_out_cleanup;
	}

	
	for (i = 0; i < fnic->wq_copy_count; i++) {
		err = vnic_wq_copy_alloc(fnic->vdev, &fnic->wq_copy[i],
			(fnic->raw_wq_count + i),
			fnic->config.wq_copy_desc_count,
			sizeof(struct fcpio_host_req));
		if (err)
			goto err_out_cleanup;
	}

	
	for (i = 0; i < fnic->rq_count; i++) {
		err = vnic_rq_alloc(fnic->vdev, &fnic->rq[i], i,
			fnic->config.rq_desc_count,
			sizeof(struct rq_enet_desc));
		if (err)
			goto err_out_cleanup;
	}

	
	for (i = 0; i < fnic->rq_count; i++) {
		cq_index = i;
		err = vnic_cq_alloc(fnic->vdev,
			&fnic->cq[cq_index], cq_index,
			fnic->config.rq_desc_count,
			sizeof(struct cq_enet_rq_desc));
		if (err)
			goto err_out_cleanup;
	}

	
	for (i = 0; i < fnic->raw_wq_count; i++) {
		cq_index = fnic->rq_count + i;
		err = vnic_cq_alloc(fnic->vdev, &fnic->cq[cq_index], cq_index,
			fnic->config.wq_enet_desc_count,
			sizeof(struct cq_enet_wq_desc));
		if (err)
			goto err_out_cleanup;
	}

	
	wq_copy_cq_desc_count = (fnic->config.wq_copy_desc_count * 3);
	for (i = 0; i < fnic->wq_copy_count; i++) {
		cq_index = fnic->raw_wq_count + fnic->rq_count + i;
		err = vnic_cq_alloc(fnic->vdev, &fnic->cq[cq_index],
			cq_index,
			wq_copy_cq_desc_count,
			sizeof(struct fcpio_fw_req));
		if (err)
			goto err_out_cleanup;
	}

	for (i = 0; i < fnic->intr_count; i++) {
		err = vnic_intr_alloc(fnic->vdev, &fnic->intr[i], i);
		if (err)
			goto err_out_cleanup;
	}

	fnic->legacy_pba = vnic_dev_get_res(fnic->vdev,
				RES_TYPE_INTR_PBA_LEGACY, 0);

	if (!fnic->legacy_pba && intr_mode == VNIC_DEV_INTR_MODE_INTX) {
		shost_printk(KERN_ERR, fnic->lport->host,
			     "Failed to hook legacy pba resource\n");
		err = -ENODEV;
		goto err_out_cleanup;
	}

	

	switch (intr_mode) {
	case VNIC_DEV_INTR_MODE_INTX:
	case VNIC_DEV_INTR_MODE_MSIX:
		error_interrupt_enable = 1;
		error_interrupt_offset = fnic->err_intr_offset;
		break;
	default:
		error_interrupt_enable = 0;
		error_interrupt_offset = 0;
		break;
	}

	for (i = 0; i < fnic->rq_count; i++) {
		cq_index = i;
		vnic_rq_init(&fnic->rq[i],
			     cq_index,
			     error_interrupt_enable,
			     error_interrupt_offset);
	}

	for (i = 0; i < fnic->raw_wq_count; i++) {
		cq_index = i + fnic->rq_count;
		vnic_wq_init(&fnic->wq[i],
			     cq_index,
			     error_interrupt_enable,
			     error_interrupt_offset);
	}

	for (i = 0; i < fnic->wq_copy_count; i++) {
		vnic_wq_copy_init(&fnic->wq_copy[i],
				  0 ,
				  error_interrupt_enable,
				  error_interrupt_offset);
	}

	for (i = 0; i < fnic->cq_count; i++) {

		switch (intr_mode) {
		case VNIC_DEV_INTR_MODE_MSIX:
			interrupt_offset = i;
			break;
		default:
			interrupt_offset = 0;
			break;
		}

		vnic_cq_init(&fnic->cq[i],
			0 ,
			1 ,
			0 ,
			0 ,
			1 ,
			1 ,
			1 ,
			0 ,
			interrupt_offset,
			0 );
	}

	

	switch (intr_mode) {
	case VNIC_DEV_INTR_MODE_MSI:
	case VNIC_DEV_INTR_MODE_MSIX:
		mask_on_assertion = 1;
		break;
	default:
		mask_on_assertion = 0;
		break;
	}

	for (i = 0; i < fnic->intr_count; i++) {
		vnic_intr_init(&fnic->intr[i],
			fnic->config.intr_timer,
			fnic->config.intr_timer_type,
			mask_on_assertion);
	}

	
	err = vnic_dev_stats_dump(fnic->vdev, &fnic->stats);
	if (err) {
		shost_printk(KERN_ERR, fnic->lport->host,
			     "vnic_dev_stats_dump failed - x%x\n", err);
		goto err_out_cleanup;
	}

	
	vnic_dev_stats_clear(fnic->vdev);

	return 0;

err_out_cleanup:
	fnic_free_vnic_resources(fnic);

	return err;
}
