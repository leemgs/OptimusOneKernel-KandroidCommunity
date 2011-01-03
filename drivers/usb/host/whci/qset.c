
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/uwb/umc.h>
#include <linux/usb.h>

#include "../../wusbcore/wusbhc.h"

#include "whcd.h"

struct whc_qset *qset_alloc(struct whc *whc, gfp_t mem_flags)
{
	struct whc_qset *qset;
	dma_addr_t dma;

	qset = dma_pool_alloc(whc->qset_pool, mem_flags, &dma);
	if (qset == NULL)
		return NULL;
	memset(qset, 0, sizeof(struct whc_qset));

	qset->qset_dma = dma;
	qset->whc = whc;

	INIT_LIST_HEAD(&qset->list_node);
	INIT_LIST_HEAD(&qset->stds);

	return qset;
}


static void qset_fill_qh(struct whc_qset *qset, struct urb *urb)
{
	struct usb_device *usb_dev = urb->dev;
	struct usb_wireless_ep_comp_descriptor *epcd;
	bool is_out;

	is_out = usb_pipeout(urb->pipe);

	epcd = (struct usb_wireless_ep_comp_descriptor *)qset->ep->extra;

	if (epcd) {
		qset->max_seq = epcd->bMaxSequence;
		qset->max_burst = epcd->bMaxBurst;
	} else {
		qset->max_seq = 2;
		qset->max_burst = 1;
	}

	qset->qh.info1 = cpu_to_le32(
		QH_INFO1_EP(usb_pipeendpoint(urb->pipe))
		| (is_out ? QH_INFO1_DIR_OUT : QH_INFO1_DIR_IN)
		| usb_pipe_to_qh_type(urb->pipe)
		| QH_INFO1_DEV_INFO_IDX(wusb_port_no_to_idx(usb_dev->portnum))
		| QH_INFO1_MAX_PKT_LEN(usb_maxpacket(urb->dev, urb->pipe, is_out))
		);
	qset->qh.info2 = cpu_to_le32(
		QH_INFO2_BURST(qset->max_burst)
		| QH_INFO2_DBP(0)
		| QH_INFO2_MAX_COUNT(3)
		| QH_INFO2_MAX_RETRY(3)
		| QH_INFO2_MAX_SEQ(qset->max_seq - 1)
		);
	
	qset->qh.info3 = cpu_to_le32(
		QH_INFO3_TX_RATE_53_3
		| QH_INFO3_TX_PWR(0) 
		);

	qset->qh.cur_window = cpu_to_le32((1 << qset->max_burst) - 1);
}


void qset_clear(struct whc *whc, struct whc_qset *qset)
{
	qset->td_start = qset->td_end = qset->ntds = 0;

	qset->qh.link = cpu_to_le32(QH_LINK_NTDS(8) | QH_LINK_T);
	qset->qh.status = qset->qh.status & QH_STATUS_SEQ_MASK;
	qset->qh.err_count = 0;
	qset->qh.scratch[0] = 0;
	qset->qh.scratch[1] = 0;
	qset->qh.scratch[2] = 0;

	memset(&qset->qh.overlay, 0, sizeof(qset->qh.overlay));

	init_completion(&qset->remove_complete);
}


void qset_reset(struct whc *whc, struct whc_qset *qset)
{
	qset->reset = 0;

	qset->qh.status &= ~QH_STATUS_SEQ_MASK;
	qset->qh.cur_window = cpu_to_le32((1 << qset->max_burst) - 1);
}


struct whc_qset *get_qset(struct whc *whc, struct urb *urb,
				 gfp_t mem_flags)
{
	struct whc_qset *qset;

	qset = urb->ep->hcpriv;
	if (qset == NULL) {
		qset = qset_alloc(whc, mem_flags);
		if (qset == NULL)
			return NULL;

		qset->ep = urb->ep;
		urb->ep->hcpriv = qset;
		qset_fill_qh(qset, urb);
	}
	return qset;
}

void qset_remove_complete(struct whc *whc, struct whc_qset *qset)
{
	qset->remove = 0;
	list_del_init(&qset->list_node);
	complete(&qset->remove_complete);
}


enum whc_update qset_add_qtds(struct whc *whc, struct whc_qset *qset)
{
	struct whc_std *std;
	enum whc_update update = 0;

	list_for_each_entry(std, &qset->stds, list_node) {
		struct whc_qtd *qtd;
		uint32_t status;

		if (qset->ntds >= WHCI_QSET_TD_MAX
		    || (qset->pause_after_urb && std->urb != qset->pause_after_urb))
			break;

		if (std->qtd)
			continue; 

		qtd = std->qtd = &qset->qtd[qset->td_end];

		
		if (usb_pipecontrol(std->urb->pipe))
			memcpy(qtd->setup, std->urb->setup_packet, 8);

		status = QTD_STS_ACTIVE | QTD_STS_LEN(std->len);

		if (whc_std_last(std) && usb_pipeout(std->urb->pipe))
			status |= QTD_STS_LAST_PKT;

		
		if (std->ntds_remaining < WHCI_QSET_TD_MAX) {
			int ialt;
			ialt = (qset->td_end + std->ntds_remaining) % WHCI_QSET_TD_MAX;
			status |= QTD_STS_IALT(ialt);
		} else if (usb_pipein(std->urb->pipe))
			qset->pause_after_urb = std->urb;

		if (std->num_pointers)
			qtd->options = cpu_to_le32(QTD_OPT_IOC);
		else
			qtd->options = cpu_to_le32(QTD_OPT_IOC | QTD_OPT_SMALL);
		qtd->page_list_ptr = cpu_to_le64(std->dma_addr);

		qtd->status = cpu_to_le32(status);

		if (QH_STATUS_TO_ICUR(qset->qh.status) == qset->td_end)
			update = WHC_UPDATE_UPDATED;

		if (++qset->td_end >= WHCI_QSET_TD_MAX)
			qset->td_end = 0;
		qset->ntds++;
	}

	return update;
}


static void qset_remove_qtd(struct whc *whc, struct whc_qset *qset)
{
	qset->qtd[qset->td_start].status = 0;

	if (++qset->td_start >= WHCI_QSET_TD_MAX)
		qset->td_start = 0;
	qset->ntds--;
}


void qset_free_std(struct whc *whc, struct whc_std *std)
{
	list_del(&std->list_node);
	if (std->num_pointers) {
		dma_unmap_single(whc->wusbhc.dev, std->dma_addr,
				 std->num_pointers * sizeof(struct whc_page_list_entry),
				 DMA_TO_DEVICE);
		kfree(std->pl_virt);
	}

	kfree(std);
}


static void qset_remove_qtds(struct whc *whc, struct whc_qset *qset,
			     struct urb *urb)
{
	struct whc_std *std, *t;

	list_for_each_entry_safe(std, t, &qset->stds, list_node) {
		if (std->urb != urb)
			break;
		if (std->qtd != NULL)
			qset_remove_qtd(whc, qset);
		qset_free_std(whc, std);
	}
}


static void qset_free_stds(struct whc_qset *qset, struct urb *urb)
{
	struct whc_std *std, *t;

	list_for_each_entry_safe(std, t, &qset->stds, list_node) {
		if (std->urb == urb)
			qset_free_std(qset->whc, std);
	}
}

static int qset_fill_page_list(struct whc *whc, struct whc_std *std, gfp_t mem_flags)
{
	dma_addr_t dma_addr = std->dma_addr;
	dma_addr_t sp, ep;
	size_t std_len = std->len;
	size_t pl_len;
	int p;

	sp = ALIGN(dma_addr, WHCI_PAGE_SIZE);
	ep = dma_addr + std_len;
	std->num_pointers = DIV_ROUND_UP(ep - sp, WHCI_PAGE_SIZE);

	pl_len = std->num_pointers * sizeof(struct whc_page_list_entry);
	std->pl_virt = kmalloc(pl_len, mem_flags);
	if (std->pl_virt == NULL)
		return -ENOMEM;
	std->dma_addr = dma_map_single(whc->wusbhc.dev, std->pl_virt, pl_len, DMA_TO_DEVICE);

	for (p = 0; p < std->num_pointers; p++) {
		std->pl_virt[p].buf_ptr = cpu_to_le64(dma_addr);
		dma_addr = ALIGN(dma_addr + WHCI_PAGE_SIZE, WHCI_PAGE_SIZE);
	}

	return 0;
}


static void urb_dequeue_work(struct work_struct *work)
{
	struct whc_urb *wurb = container_of(work, struct whc_urb, dequeue_work);
	struct whc_qset *qset = wurb->qset;
	struct whc *whc = qset->whc;
	unsigned long flags;

	if (wurb->is_async == true)
		asl_update(whc, WUSBCMD_ASYNC_UPDATED
			   | WUSBCMD_ASYNC_SYNCED_DB
			   | WUSBCMD_ASYNC_QSET_RM);
	else
		pzl_update(whc, WUSBCMD_PERIODIC_UPDATED
			   | WUSBCMD_PERIODIC_SYNCED_DB
			   | WUSBCMD_PERIODIC_QSET_RM);

	spin_lock_irqsave(&whc->lock, flags);
	qset_remove_urb(whc, qset, wurb->urb, wurb->status);
	spin_unlock_irqrestore(&whc->lock, flags);
}


int qset_add_urb(struct whc *whc, struct whc_qset *qset, struct urb *urb,
	gfp_t mem_flags)
{
	struct whc_urb *wurb;
	int remaining = urb->transfer_buffer_length;
	u64 transfer_dma = urb->transfer_dma;
	int ntds_remaining;

	ntds_remaining = DIV_ROUND_UP(remaining, QTD_MAX_XFER_SIZE);
	if (ntds_remaining == 0)
		ntds_remaining = 1;

	wurb = kzalloc(sizeof(struct whc_urb), mem_flags);
	if (wurb == NULL)
		goto err_no_mem;
	urb->hcpriv = wurb;
	wurb->qset = qset;
	wurb->urb = urb;
	INIT_WORK(&wurb->dequeue_work, urb_dequeue_work);

	while (ntds_remaining) {
		struct whc_std *std;
		size_t std_len;

		std = kmalloc(sizeof(struct whc_std), mem_flags);
		if (std == NULL)
			goto err_no_mem;

		std_len = remaining;
		if (std_len > QTD_MAX_XFER_SIZE)
			std_len = QTD_MAX_XFER_SIZE;

		std->urb = urb;
		std->dma_addr = transfer_dma;
		std->len = std_len;
		std->ntds_remaining = ntds_remaining;
		std->qtd = NULL;

		INIT_LIST_HEAD(&std->list_node);
		list_add_tail(&std->list_node, &qset->stds);

		if (std_len > WHCI_PAGE_SIZE) {
			if (qset_fill_page_list(whc, std, mem_flags) < 0)
				goto err_no_mem;
		} else
			std->num_pointers = 0;

		ntds_remaining--;
		remaining -= std_len;
		transfer_dma += std_len;
	}

	return 0;

err_no_mem:
	qset_free_stds(qset, urb);
	return -ENOMEM;
}


void qset_remove_urb(struct whc *whc, struct whc_qset *qset,
			    struct urb *urb, int status)
{
	struct wusbhc *wusbhc = &whc->wusbhc;
	struct whc_urb *wurb = urb->hcpriv;

	usb_hcd_unlink_urb_from_ep(&wusbhc->usb_hcd, urb);
	
	spin_unlock(&whc->lock);
	wusbhc_giveback_urb(wusbhc, urb, status);
	spin_lock(&whc->lock);

	kfree(wurb);
}


static int get_urb_status_from_qtd(struct urb *urb, u32 status)
{
	if (status & QTD_STS_HALTED) {
		if (status & QTD_STS_DBE)
			return usb_pipein(urb->pipe) ? -ENOSR : -ECOMM;
		else if (status & QTD_STS_BABBLE)
			return -EOVERFLOW;
		else if (status & QTD_STS_RCE)
			return -ETIME;
		return -EPIPE;
	}
	if (usb_pipein(urb->pipe)
	    && (urb->transfer_flags & URB_SHORT_NOT_OK)
	    && urb->actual_length < urb->transfer_buffer_length)
		return -EREMOTEIO;
	return 0;
}


void process_inactive_qtd(struct whc *whc, struct whc_qset *qset,
				 struct whc_qtd *qtd)
{
	struct whc_std *std = list_first_entry(&qset->stds, struct whc_std, list_node);
	struct urb *urb = std->urb;
	uint32_t status;
	bool complete;

	status = le32_to_cpu(qtd->status);

	urb->actual_length += std->len - QTD_STS_TO_LEN(status);

	if (usb_pipein(urb->pipe) && (status & QTD_STS_LAST_PKT))
		complete = true;
	else
		complete = whc_std_last(std);

	qset_remove_qtd(whc, qset);
	qset_free_std(whc, std);

	
	if (complete) {
		qset_remove_qtds(whc, qset, urb);
		qset_remove_urb(whc, qset, urb, get_urb_status_from_qtd(urb, status));

		
		if (!(status & QTD_STS_IALT_VALID))
			qset->td_start = qset->td_end
				= QH_STATUS_TO_ICUR(le16_to_cpu(qset->qh.status));
		qset->pause_after_urb = NULL;
	}
}


void process_halted_qtd(struct whc *whc, struct whc_qset *qset,
			       struct whc_qtd *qtd)
{
	struct whc_std *std = list_first_entry(&qset->stds, struct whc_std, list_node);
	struct urb *urb = std->urb;
	int urb_status;

	urb_status = get_urb_status_from_qtd(urb, le32_to_cpu(qtd->status));

	qset_remove_qtds(whc, qset, urb);
	qset_remove_urb(whc, qset, urb, urb_status);

	list_for_each_entry(std, &qset->stds, list_node) {
		if (qset->ntds == 0)
			break;
		qset_remove_qtd(whc, qset);
		std->qtd = NULL;
	}

	qset->remove = 1;
}

void qset_free(struct whc *whc, struct whc_qset *qset)
{
	dma_pool_free(whc->qset_pool, qset, qset->qset_dma);
}


void qset_delete(struct whc *whc, struct whc_qset *qset)
{
	wait_for_completion(&qset->remove_complete);
	qset_free(whc, qset);
}
