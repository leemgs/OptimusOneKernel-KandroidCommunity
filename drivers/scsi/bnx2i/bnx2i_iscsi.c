

#include <scsi/scsi_tcq.h>
#include <scsi/libiscsi.h>
#include "bnx2i.h"

struct scsi_transport_template *bnx2i_scsi_xport_template;
struct iscsi_transport bnx2i_iscsi_transport;
static struct scsi_host_template bnx2i_host_template;


static DEFINE_SPINLOCK(bnx2i_resc_lock); 


static int bnx2i_adapter_ready(struct bnx2i_hba *hba)
{
	int retval = 0;

	if (!hba || !test_bit(ADAPTER_STATE_UP, &hba->adapter_state) ||
	    test_bit(ADAPTER_STATE_GOING_DOWN, &hba->adapter_state) ||
	    test_bit(ADAPTER_STATE_LINK_DOWN, &hba->adapter_state))
		retval = -EPERM;
	return retval;
}


static void bnx2i_get_write_cmd_bd_idx(struct bnx2i_cmd *cmd, u32 buf_off,
				       u32 *start_bd_off, u32 *start_bd_idx)
{
	struct iscsi_bd *bd_tbl = cmd->io_tbl.bd_tbl;
	u32 cur_offset = 0;
	u32 cur_bd_idx = 0;

	if (buf_off) {
		while (buf_off >= (cur_offset + bd_tbl->buffer_length)) {
			cur_offset += bd_tbl->buffer_length;
			cur_bd_idx++;
			bd_tbl++;
		}
	}

	*start_bd_off = buf_off - cur_offset;
	*start_bd_idx = cur_bd_idx;
}


static void bnx2i_setup_write_cmd_bd_info(struct iscsi_task *task)
{
	struct bnx2i_cmd *cmd = task->dd_data;
	u32 start_bd_offset;
	u32 start_bd_idx;
	u32 buffer_offset = 0;
	u32 cmd_len = cmd->req.total_data_transfer_length;

	
	if (!iscsi_task_has_unsol_data(task) && !task->imm_count)
		return;

	
	buffer_offset += task->imm_count;
	if (task->imm_count == cmd_len)
		return;

	if (iscsi_task_has_unsol_data(task)) {
		bnx2i_get_write_cmd_bd_idx(cmd, buffer_offset,
					   &start_bd_offset, &start_bd_idx);
		cmd->req.ud_buffer_offset = start_bd_offset;
		cmd->req.ud_start_bd_index = start_bd_idx;
		buffer_offset += task->unsol_r2t.data_length;
	}

	if (buffer_offset != cmd_len) {
		bnx2i_get_write_cmd_bd_idx(cmd, buffer_offset,
					   &start_bd_offset, &start_bd_idx);
		if ((start_bd_offset > task->conn->session->first_burst) ||
		    (start_bd_idx > scsi_sg_count(cmd->scsi_cmd))) {
			int i = 0;

			iscsi_conn_printk(KERN_ALERT, task->conn,
					  "bnx2i- error, buf offset 0x%x "
					  "bd_valid %d use_sg %d\n",
					  buffer_offset, cmd->io_tbl.bd_valid,
					  scsi_sg_count(cmd->scsi_cmd));
			for (i = 0; i < cmd->io_tbl.bd_valid; i++)
				iscsi_conn_printk(KERN_ALERT, task->conn,
						  "bnx2i err, bd[%d]: len %x\n",
						  i, cmd->io_tbl.bd_tbl[i].\
						  buffer_length);
		}
		cmd->req.sd_buffer_offset = start_bd_offset;
		cmd->req.sd_start_bd_index = start_bd_idx;
	}
}




static int bnx2i_map_scsi_sg(struct bnx2i_hba *hba, struct bnx2i_cmd *cmd)
{
	struct scsi_cmnd *sc = cmd->scsi_cmd;
	struct iscsi_bd *bd = cmd->io_tbl.bd_tbl;
	struct scatterlist *sg;
	int byte_count = 0;
	int bd_count = 0;
	int sg_count;
	int sg_len;
	u64 addr;
	int i;

	BUG_ON(scsi_sg_count(sc) > ISCSI_MAX_BDS_PER_CMD);

	sg_count = scsi_dma_map(sc);

	scsi_for_each_sg(sc, sg, sg_count, i) {
		sg_len = sg_dma_len(sg);
		addr = (u64) sg_dma_address(sg);
		bd[bd_count].buffer_addr_lo = addr & 0xffffffff;
		bd[bd_count].buffer_addr_hi = addr >> 32;
		bd[bd_count].buffer_length = sg_len;
		bd[bd_count].flags = 0;
		if (bd_count == 0)
			bd[bd_count].flags = ISCSI_BD_FIRST_IN_BD_CHAIN;

		byte_count += sg_len;
		bd_count++;
	}

	if (bd_count)
		bd[bd_count - 1].flags |= ISCSI_BD_LAST_IN_BD_CHAIN;

	BUG_ON(byte_count != scsi_bufflen(sc));
	return bd_count;
}


static void bnx2i_iscsi_map_sg_list(struct bnx2i_cmd *cmd)
{
	int bd_count;

	bd_count  = bnx2i_map_scsi_sg(cmd->conn->hba, cmd);
	if (!bd_count) {
		struct iscsi_bd *bd = cmd->io_tbl.bd_tbl;

		bd[0].buffer_addr_lo = bd[0].buffer_addr_hi = 0;
		bd[0].buffer_length = bd[0].flags = 0;
	}
	cmd->io_tbl.bd_valid = bd_count;
}



void bnx2i_iscsi_unmap_sg_list(struct bnx2i_cmd *cmd)
{
	struct scsi_cmnd *sc = cmd->scsi_cmd;

	if (cmd->io_tbl.bd_valid && sc) {
		scsi_dma_unmap(sc);
		cmd->io_tbl.bd_valid = 0;
	}
}

static void bnx2i_setup_cmd_wqe_template(struct bnx2i_cmd *cmd)
{
	memset(&cmd->req, 0x00, sizeof(cmd->req));
	cmd->req.op_code = 0xFF;
	cmd->req.bd_list_addr_lo = (u32) cmd->io_tbl.bd_tbl_dma;
	cmd->req.bd_list_addr_hi =
		(u32) ((u64) cmd->io_tbl.bd_tbl_dma >> 32);

}



static int bnx2i_bind_conn_to_iscsi_cid(struct bnx2i_hba *hba,
					struct bnx2i_conn *bnx2i_conn,
					u32 iscsi_cid)
{
	if (hba && hba->cid_que.conn_cid_tbl[iscsi_cid]) {
		iscsi_conn_printk(KERN_ALERT, bnx2i_conn->cls_conn->dd_data,
				 "conn bind - entry #%d not free\n", iscsi_cid);
		return -EBUSY;
	}

	hba->cid_que.conn_cid_tbl[iscsi_cid] = bnx2i_conn;
	return 0;
}



struct bnx2i_conn *bnx2i_get_conn_from_id(struct bnx2i_hba *hba,
					  u16 iscsi_cid)
{
	if (!hba->cid_que.conn_cid_tbl) {
		printk(KERN_ERR "bnx2i: ERROR - missing conn<->cid table\n");
		return NULL;

	} else if (iscsi_cid >= hba->max_active_conns) {
		printk(KERN_ERR "bnx2i: wrong cid #%d\n", iscsi_cid);
		return NULL;
	}
	return hba->cid_que.conn_cid_tbl[iscsi_cid];
}



static u32 bnx2i_alloc_iscsi_cid(struct bnx2i_hba *hba)
{
	int idx;

	if (!hba->cid_que.cid_free_cnt)
		return -1;

	idx = hba->cid_que.cid_q_cons_idx;
	hba->cid_que.cid_q_cons_idx++;
	if (hba->cid_que.cid_q_cons_idx == hba->cid_que.cid_q_max_idx)
		hba->cid_que.cid_q_cons_idx = 0;

	hba->cid_que.cid_free_cnt--;
	return hba->cid_que.cid_que[idx];
}



static void bnx2i_free_iscsi_cid(struct bnx2i_hba *hba, u16 iscsi_cid)
{
	int idx;

	if (iscsi_cid == (u16) -1)
		return;

	hba->cid_que.cid_free_cnt++;

	idx = hba->cid_que.cid_q_prod_idx;
	hba->cid_que.cid_que[idx] = iscsi_cid;
	hba->cid_que.conn_cid_tbl[iscsi_cid] = NULL;
	hba->cid_que.cid_q_prod_idx++;
	if (hba->cid_que.cid_q_prod_idx == hba->cid_que.cid_q_max_idx)
		hba->cid_que.cid_q_prod_idx = 0;
}



static int bnx2i_setup_free_cid_que(struct bnx2i_hba *hba)
{
	int mem_size;
	int i;

	mem_size = hba->max_active_conns * sizeof(u32);
	mem_size = (mem_size + (PAGE_SIZE - 1)) & PAGE_MASK;

	hba->cid_que.cid_que_base = kmalloc(mem_size, GFP_KERNEL);
	if (!hba->cid_que.cid_que_base)
		return -ENOMEM;

	mem_size = hba->max_active_conns * sizeof(struct bnx2i_conn *);
	mem_size = (mem_size + (PAGE_SIZE - 1)) & PAGE_MASK;
	hba->cid_que.conn_cid_tbl = kmalloc(mem_size, GFP_KERNEL);
	if (!hba->cid_que.conn_cid_tbl) {
		kfree(hba->cid_que.cid_que_base);
		hba->cid_que.cid_que_base = NULL;
		return -ENOMEM;
	}

	hba->cid_que.cid_que = (u32 *)hba->cid_que.cid_que_base;
	hba->cid_que.cid_q_prod_idx = 0;
	hba->cid_que.cid_q_cons_idx = 0;
	hba->cid_que.cid_q_max_idx = hba->max_active_conns;
	hba->cid_que.cid_free_cnt = hba->max_active_conns;

	for (i = 0; i < hba->max_active_conns; i++) {
		hba->cid_que.cid_que[i] = i;
		hba->cid_que.conn_cid_tbl[i] = NULL;
	}
	return 0;
}



static void bnx2i_release_free_cid_que(struct bnx2i_hba *hba)
{
	kfree(hba->cid_que.cid_que_base);
	hba->cid_que.cid_que_base = NULL;

	kfree(hba->cid_que.conn_cid_tbl);
	hba->cid_que.conn_cid_tbl = NULL;
}



static struct iscsi_endpoint *bnx2i_alloc_ep(struct bnx2i_hba *hba)
{
	struct iscsi_endpoint *ep;
	struct bnx2i_endpoint *bnx2i_ep;

	ep = iscsi_create_endpoint(sizeof(*bnx2i_ep));
	if (!ep) {
		printk(KERN_ERR "bnx2i: Could not allocate ep\n");
		return NULL;
	}

	bnx2i_ep = ep->dd_data;
	INIT_LIST_HEAD(&bnx2i_ep->link);
	bnx2i_ep->state = EP_STATE_IDLE;
	bnx2i_ep->ep_iscsi_cid = (u16) -1;
	bnx2i_ep->hba = hba;
	bnx2i_ep->hba_age = hba->age;
	hba->ofld_conns_active++;
	init_waitqueue_head(&bnx2i_ep->ofld_wait);
	return ep;
}



static void bnx2i_free_ep(struct iscsi_endpoint *ep)
{
	struct bnx2i_endpoint *bnx2i_ep = ep->dd_data;
	unsigned long flags;

	spin_lock_irqsave(&bnx2i_resc_lock, flags);
	bnx2i_ep->state = EP_STATE_IDLE;
	bnx2i_ep->hba->ofld_conns_active--;

	bnx2i_free_iscsi_cid(bnx2i_ep->hba, bnx2i_ep->ep_iscsi_cid);
	if (bnx2i_ep->conn) {
		bnx2i_ep->conn->ep = NULL;
		bnx2i_ep->conn = NULL;
	}

	bnx2i_ep->hba = NULL;
	spin_unlock_irqrestore(&bnx2i_resc_lock, flags);
	iscsi_destroy_endpoint(ep);
}



static int bnx2i_alloc_bdt(struct bnx2i_hba *hba, struct iscsi_session *session,
			   struct bnx2i_cmd *cmd)
{
	struct io_bdt *io = &cmd->io_tbl;
	struct iscsi_bd *bd;

	io->bd_tbl = dma_alloc_coherent(&hba->pcidev->dev,
					ISCSI_MAX_BDS_PER_CMD * sizeof(*bd),
					&io->bd_tbl_dma, GFP_KERNEL);
	if (!io->bd_tbl) {
		iscsi_session_printk(KERN_ERR, session, "Could not "
				     "allocate bdt.\n");
		return -ENOMEM;
	}
	io->bd_valid = 0;
	return 0;
}


static void bnx2i_destroy_cmd_pool(struct bnx2i_hba *hba,
				   struct iscsi_session *session)
{
	int i;

	for (i = 0; i < session->cmds_max; i++) {
		struct iscsi_task *task = session->cmds[i];
		struct bnx2i_cmd *cmd = task->dd_data;

		if (cmd->io_tbl.bd_tbl)
			dma_free_coherent(&hba->pcidev->dev,
					  ISCSI_MAX_BDS_PER_CMD *
					  sizeof(struct iscsi_bd),
					  cmd->io_tbl.bd_tbl,
					  cmd->io_tbl.bd_tbl_dma);
	}

}



static int bnx2i_setup_cmd_pool(struct bnx2i_hba *hba,
				struct iscsi_session *session)
{
	int i;

	for (i = 0; i < session->cmds_max; i++) {
		struct iscsi_task *task = session->cmds[i];
		struct bnx2i_cmd *cmd = task->dd_data;

		
		task->hdr = &cmd->hdr;
		task->hdr_max = sizeof(struct iscsi_hdr);

		if (bnx2i_alloc_bdt(hba, session, cmd))
			goto free_bdts;
	}

	return 0;

free_bdts:
	bnx2i_destroy_cmd_pool(hba, session);
	return -ENOMEM;
}



static int bnx2i_setup_mp_bdt(struct bnx2i_hba *hba)
{
	int rc = 0;
	struct iscsi_bd *mp_bdt;
	u64 addr;

	hba->mp_bd_tbl = dma_alloc_coherent(&hba->pcidev->dev, PAGE_SIZE,
					    &hba->mp_bd_dma, GFP_KERNEL);
	if (!hba->mp_bd_tbl) {
		printk(KERN_ERR "unable to allocate Middle Path BDT\n");
		rc = -1;
		goto out;
	}

	hba->dummy_buffer = dma_alloc_coherent(&hba->pcidev->dev, PAGE_SIZE,
					       &hba->dummy_buf_dma, GFP_KERNEL);
	if (!hba->dummy_buffer) {
		printk(KERN_ERR "unable to alloc Middle Path Dummy Buffer\n");
		dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
				  hba->mp_bd_tbl, hba->mp_bd_dma);
		hba->mp_bd_tbl = NULL;
		rc = -1;
		goto out;
	}

	mp_bdt = (struct iscsi_bd *) hba->mp_bd_tbl;
	addr = (unsigned long) hba->dummy_buf_dma;
	mp_bdt->buffer_addr_lo = addr & 0xffffffff;
	mp_bdt->buffer_addr_hi = addr >> 32;
	mp_bdt->buffer_length = PAGE_SIZE;
	mp_bdt->flags = ISCSI_BD_LAST_IN_BD_CHAIN |
			ISCSI_BD_FIRST_IN_BD_CHAIN;
out:
	return rc;
}



static void bnx2i_free_mp_bdt(struct bnx2i_hba *hba)
{
	if (hba->mp_bd_tbl) {
		dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
				  hba->mp_bd_tbl, hba->mp_bd_dma);
		hba->mp_bd_tbl = NULL;
	}
	if (hba->dummy_buffer) {
		dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
				  hba->dummy_buffer, hba->dummy_buf_dma);
		hba->dummy_buffer = NULL;
	}
		return;
}


void bnx2i_drop_session(struct iscsi_cls_session *cls_session)
{
	iscsi_session_failure(cls_session->dd_data, ISCSI_ERR_CONN_FAILED);
}


static int bnx2i_ep_destroy_list_add(struct bnx2i_hba *hba,
				     struct bnx2i_endpoint *ep)
{
	write_lock_bh(&hba->ep_rdwr_lock);
	list_add_tail(&ep->link, &hba->ep_destroy_list);
	write_unlock_bh(&hba->ep_rdwr_lock);
	return 0;
}


static int bnx2i_ep_destroy_list_del(struct bnx2i_hba *hba,
				     struct bnx2i_endpoint *ep)
{
	write_lock_bh(&hba->ep_rdwr_lock);
	list_del_init(&ep->link);
	write_unlock_bh(&hba->ep_rdwr_lock);

	return 0;
}


static int bnx2i_ep_ofld_list_add(struct bnx2i_hba *hba,
				  struct bnx2i_endpoint *ep)
{
	write_lock_bh(&hba->ep_rdwr_lock);
	list_add_tail(&ep->link, &hba->ep_ofld_list);
	write_unlock_bh(&hba->ep_rdwr_lock);
	return 0;
}


static int bnx2i_ep_ofld_list_del(struct bnx2i_hba *hba,
				  struct bnx2i_endpoint *ep)
{
	write_lock_bh(&hba->ep_rdwr_lock);
	list_del_init(&ep->link);
	write_unlock_bh(&hba->ep_rdwr_lock);
	return 0;
}



struct bnx2i_endpoint *
bnx2i_find_ep_in_ofld_list(struct bnx2i_hba *hba, u32 iscsi_cid)
{
	struct list_head *list;
	struct list_head *tmp;
	struct bnx2i_endpoint *ep;

	read_lock_bh(&hba->ep_rdwr_lock);
	list_for_each_safe(list, tmp, &hba->ep_ofld_list) {
		ep = (struct bnx2i_endpoint *)list;

		if (ep->ep_iscsi_cid == iscsi_cid)
			break;
		ep = NULL;
	}
	read_unlock_bh(&hba->ep_rdwr_lock);

	if (!ep)
		printk(KERN_ERR "l5 cid %d not found\n", iscsi_cid);
	return ep;
}



struct bnx2i_endpoint *
bnx2i_find_ep_in_destroy_list(struct bnx2i_hba *hba, u32 iscsi_cid)
{
	struct list_head *list;
	struct list_head *tmp;
	struct bnx2i_endpoint *ep;

	read_lock_bh(&hba->ep_rdwr_lock);
	list_for_each_safe(list, tmp, &hba->ep_destroy_list) {
		ep = (struct bnx2i_endpoint *)list;

		if (ep->ep_iscsi_cid == iscsi_cid)
			break;
		ep = NULL;
	}
	read_unlock_bh(&hba->ep_rdwr_lock);

	if (!ep)
		printk(KERN_ERR "l5 cid %d not found\n", iscsi_cid);

	return ep;
}


static void bnx2i_setup_host_queue_size(struct bnx2i_hba *hba,
					struct Scsi_Host *shost)
{
	if (test_bit(BNX2I_NX2_DEV_5708, &hba->cnic_dev_type))
		shost->can_queue = ISCSI_MAX_CMDS_PER_HBA_5708;
	else if (test_bit(BNX2I_NX2_DEV_5709, &hba->cnic_dev_type))
		shost->can_queue = ISCSI_MAX_CMDS_PER_HBA_5709;
	else if (test_bit(BNX2I_NX2_DEV_57710, &hba->cnic_dev_type))
		shost->can_queue = ISCSI_MAX_CMDS_PER_HBA_57710;
	else
		shost->can_queue = ISCSI_MAX_CMDS_PER_HBA_5708;
}



struct bnx2i_hba *bnx2i_alloc_hba(struct cnic_dev *cnic)
{
	struct Scsi_Host *shost;
	struct bnx2i_hba *hba;

	shost = iscsi_host_alloc(&bnx2i_host_template, sizeof(*hba), 0);
	if (!shost)
		return NULL;
	shost->dma_boundary = cnic->pcidev->dma_mask;
	shost->transportt = bnx2i_scsi_xport_template;
	shost->max_id = ISCSI_MAX_CONNS_PER_HBA;
	shost->max_channel = 0;
	shost->max_lun = 512;
	shost->max_cmd_len = 16;

	hba = iscsi_host_priv(shost);
	hba->shost = shost;
	hba->netdev = cnic->netdev;
	
	hba->pcidev = cnic->pcidev;
	pci_dev_get(hba->pcidev);
	hba->pci_did = hba->pcidev->device;
	hba->pci_vid = hba->pcidev->vendor;
	hba->pci_sdid = hba->pcidev->subsystem_device;
	hba->pci_svid = hba->pcidev->subsystem_vendor;
	hba->pci_func = PCI_FUNC(hba->pcidev->devfn);
	hba->pci_devno = PCI_SLOT(hba->pcidev->devfn);
	bnx2i_identify_device(hba);

	bnx2i_identify_device(hba);
	bnx2i_setup_host_queue_size(hba, shost);

	if (test_bit(BNX2I_NX2_DEV_5709, &hba->cnic_dev_type)) {
		hba->regview = ioremap_nocache(hba->netdev->base_addr,
					       BNX2_MQ_CONFIG2);
		if (!hba->regview)
			goto ioreg_map_err;
	} else if (test_bit(BNX2I_NX2_DEV_57710, &hba->cnic_dev_type)) {
		hba->regview = ioremap_nocache(hba->netdev->base_addr, 4096);
		if (!hba->regview)
			goto ioreg_map_err;
	}

	if (bnx2i_setup_mp_bdt(hba))
		goto mp_bdt_mem_err;

	INIT_LIST_HEAD(&hba->ep_ofld_list);
	INIT_LIST_HEAD(&hba->ep_destroy_list);
	rwlock_init(&hba->ep_rdwr_lock);

	hba->mtu_supported = BNX2I_MAX_MTU_SUPPORTED;

	
	hba->max_active_conns = ISCSI_MAX_CONNS_PER_HBA;

	if (bnx2i_setup_free_cid_que(hba))
		goto cid_que_err;

	
	if (test_bit(BNX2I_NX2_DEV_57710, &hba->cnic_dev_type)) {
		if (sq_size && sq_size <= BNX2I_5770X_SQ_WQES_MAX)
			hba->max_sqes = sq_size;
		else
			hba->max_sqes = BNX2I_5770X_SQ_WQES_DEFAULT;
	} else {	
		if (sq_size && sq_size <= BNX2I_570X_SQ_WQES_MAX)
			hba->max_sqes = sq_size;
		else
			hba->max_sqes = BNX2I_570X_SQ_WQES_DEFAULT;
	}

	hba->max_rqes = rq_size;
	hba->max_cqes = hba->max_sqes + rq_size;
	if (test_bit(BNX2I_NX2_DEV_57710, &hba->cnic_dev_type)) {
		if (hba->max_cqes > BNX2I_5770X_CQ_WQES_MAX)
			hba->max_cqes = BNX2I_5770X_CQ_WQES_MAX;
	} else if (hba->max_cqes > BNX2I_570X_CQ_WQES_MAX)
		hba->max_cqes = BNX2I_570X_CQ_WQES_MAX;

	hba->num_ccell = hba->max_sqes / 2;

	spin_lock_init(&hba->lock);
	mutex_init(&hba->net_dev_lock);

	if (iscsi_host_add(shost, &hba->pcidev->dev))
		goto free_dump_mem;
	return hba;

free_dump_mem:
	bnx2i_release_free_cid_que(hba);
cid_que_err:
	bnx2i_free_mp_bdt(hba);
mp_bdt_mem_err:
	if (hba->regview) {
		iounmap(hba->regview);
		hba->regview = NULL;
	}
ioreg_map_err:
	pci_dev_put(hba->pcidev);
	scsi_host_put(shost);
	return NULL;
}


void bnx2i_free_hba(struct bnx2i_hba *hba)
{
	struct Scsi_Host *shost = hba->shost;

	iscsi_host_remove(shost);
	INIT_LIST_HEAD(&hba->ep_ofld_list);
	INIT_LIST_HEAD(&hba->ep_destroy_list);
	pci_dev_put(hba->pcidev);

	if (hba->regview) {
		iounmap(hba->regview);
		hba->regview = NULL;
	}
	bnx2i_free_mp_bdt(hba);
	bnx2i_release_free_cid_que(hba);
	iscsi_host_free(shost);
}


static void bnx2i_conn_free_login_resources(struct bnx2i_hba *hba,
					    struct bnx2i_conn *bnx2i_conn)
{
	if (bnx2i_conn->gen_pdu.resp_bd_tbl) {
		dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
				  bnx2i_conn->gen_pdu.resp_bd_tbl,
				  bnx2i_conn->gen_pdu.resp_bd_dma);
		bnx2i_conn->gen_pdu.resp_bd_tbl = NULL;
	}

	if (bnx2i_conn->gen_pdu.req_bd_tbl) {
		dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
				  bnx2i_conn->gen_pdu.req_bd_tbl,
				  bnx2i_conn->gen_pdu.req_bd_dma);
		bnx2i_conn->gen_pdu.req_bd_tbl = NULL;
	}

	if (bnx2i_conn->gen_pdu.resp_buf) {
		dma_free_coherent(&hba->pcidev->dev,
				  ISCSI_DEF_MAX_RECV_SEG_LEN,
				  bnx2i_conn->gen_pdu.resp_buf,
				  bnx2i_conn->gen_pdu.resp_dma_addr);
		bnx2i_conn->gen_pdu.resp_buf = NULL;
	}

	if (bnx2i_conn->gen_pdu.req_buf) {
		dma_free_coherent(&hba->pcidev->dev,
				  ISCSI_DEF_MAX_RECV_SEG_LEN,
				  bnx2i_conn->gen_pdu.req_buf,
				  bnx2i_conn->gen_pdu.req_dma_addr);
		bnx2i_conn->gen_pdu.req_buf = NULL;
	}
}


static int bnx2i_conn_alloc_login_resources(struct bnx2i_hba *hba,
					    struct bnx2i_conn *bnx2i_conn)
{
	
	bnx2i_conn->gen_pdu.req_buf =
		dma_alloc_coherent(&hba->pcidev->dev,
				   ISCSI_DEF_MAX_RECV_SEG_LEN,
				   &bnx2i_conn->gen_pdu.req_dma_addr,
				   GFP_KERNEL);
	if (bnx2i_conn->gen_pdu.req_buf == NULL)
		goto login_req_buf_failure;

	bnx2i_conn->gen_pdu.req_buf_size = 0;
	bnx2i_conn->gen_pdu.req_wr_ptr = bnx2i_conn->gen_pdu.req_buf;

	bnx2i_conn->gen_pdu.resp_buf =
		dma_alloc_coherent(&hba->pcidev->dev,
				   ISCSI_DEF_MAX_RECV_SEG_LEN,
				   &bnx2i_conn->gen_pdu.resp_dma_addr,
				   GFP_KERNEL);
	if (bnx2i_conn->gen_pdu.resp_buf == NULL)
		goto login_resp_buf_failure;

	bnx2i_conn->gen_pdu.resp_buf_size = ISCSI_DEF_MAX_RECV_SEG_LEN;
	bnx2i_conn->gen_pdu.resp_wr_ptr = bnx2i_conn->gen_pdu.resp_buf;

	bnx2i_conn->gen_pdu.req_bd_tbl =
		dma_alloc_coherent(&hba->pcidev->dev, PAGE_SIZE,
				   &bnx2i_conn->gen_pdu.req_bd_dma, GFP_KERNEL);
	if (bnx2i_conn->gen_pdu.req_bd_tbl == NULL)
		goto login_req_bd_tbl_failure;

	bnx2i_conn->gen_pdu.resp_bd_tbl =
		dma_alloc_coherent(&hba->pcidev->dev, PAGE_SIZE,
				   &bnx2i_conn->gen_pdu.resp_bd_dma,
				   GFP_KERNEL);
	if (bnx2i_conn->gen_pdu.resp_bd_tbl == NULL)
		goto login_resp_bd_tbl_failure;

	return 0;

login_resp_bd_tbl_failure:
	dma_free_coherent(&hba->pcidev->dev, PAGE_SIZE,
			  bnx2i_conn->gen_pdu.req_bd_tbl,
			  bnx2i_conn->gen_pdu.req_bd_dma);
	bnx2i_conn->gen_pdu.req_bd_tbl = NULL;

login_req_bd_tbl_failure:
	dma_free_coherent(&hba->pcidev->dev, ISCSI_DEF_MAX_RECV_SEG_LEN,
			  bnx2i_conn->gen_pdu.resp_buf,
			  bnx2i_conn->gen_pdu.resp_dma_addr);
	bnx2i_conn->gen_pdu.resp_buf = NULL;
login_resp_buf_failure:
	dma_free_coherent(&hba->pcidev->dev, ISCSI_DEF_MAX_RECV_SEG_LEN,
			  bnx2i_conn->gen_pdu.req_buf,
			  bnx2i_conn->gen_pdu.req_dma_addr);
	bnx2i_conn->gen_pdu.req_buf = NULL;
login_req_buf_failure:
	iscsi_conn_printk(KERN_ERR, bnx2i_conn->cls_conn->dd_data,
			  "login resource alloc failed!!\n");
	return -ENOMEM;

}



static void bnx2i_iscsi_prep_generic_pdu_bd(struct bnx2i_conn *bnx2i_conn)
{
	struct iscsi_bd *bd_tbl;

	bd_tbl = (struct iscsi_bd *) bnx2i_conn->gen_pdu.req_bd_tbl;

	bd_tbl->buffer_addr_hi =
		(u32) ((u64) bnx2i_conn->gen_pdu.req_dma_addr >> 32);
	bd_tbl->buffer_addr_lo = (u32) bnx2i_conn->gen_pdu.req_dma_addr;
	bd_tbl->buffer_length = bnx2i_conn->gen_pdu.req_wr_ptr -
				bnx2i_conn->gen_pdu.req_buf;
	bd_tbl->reserved0 = 0;
	bd_tbl->flags = ISCSI_BD_LAST_IN_BD_CHAIN |
			ISCSI_BD_FIRST_IN_BD_CHAIN;

	bd_tbl = (struct iscsi_bd  *) bnx2i_conn->gen_pdu.resp_bd_tbl;
	bd_tbl->buffer_addr_hi = (u64) bnx2i_conn->gen_pdu.resp_dma_addr >> 32;
	bd_tbl->buffer_addr_lo = (u32) bnx2i_conn->gen_pdu.resp_dma_addr;
	bd_tbl->buffer_length = ISCSI_DEF_MAX_RECV_SEG_LEN;
	bd_tbl->reserved0 = 0;
	bd_tbl->flags = ISCSI_BD_LAST_IN_BD_CHAIN |
			ISCSI_BD_FIRST_IN_BD_CHAIN;
}



static int bnx2i_iscsi_send_generic_request(struct iscsi_task *task)
{
	struct bnx2i_cmd *cmd = task->dd_data;
	struct bnx2i_conn *bnx2i_conn = cmd->conn;
	int rc = 0;
	char *buf;
	int data_len;

	bnx2i_iscsi_prep_generic_pdu_bd(bnx2i_conn);
	switch (task->hdr->opcode & ISCSI_OPCODE_MASK) {
	case ISCSI_OP_LOGIN:
		bnx2i_send_iscsi_login(bnx2i_conn, task);
		break;
	case ISCSI_OP_NOOP_OUT:
		data_len = bnx2i_conn->gen_pdu.req_buf_size;
		buf = bnx2i_conn->gen_pdu.req_buf;
		if (data_len)
			rc = bnx2i_send_iscsi_nopout(bnx2i_conn, task,
						     RESERVED_ITT,
						     buf, data_len, 1);
		else
			rc = bnx2i_send_iscsi_nopout(bnx2i_conn, task,
						     RESERVED_ITT,
						     NULL, 0, 1);
		break;
	case ISCSI_OP_LOGOUT:
		rc = bnx2i_send_iscsi_logout(bnx2i_conn, task);
		break;
	case ISCSI_OP_SCSI_TMFUNC:
		rc = bnx2i_send_iscsi_tmf(bnx2i_conn, task);
		break;
	default:
		iscsi_conn_printk(KERN_ALERT, bnx2i_conn->cls_conn->dd_data,
				  "send_gen: unsupported op 0x%x\n",
				  task->hdr->opcode);
	}
	return rc;
}





static void bnx2i_cpy_scsi_cdb(struct scsi_cmnd *sc, struct bnx2i_cmd *cmd)
{
	u32 dword;
	int lpcnt;
	u8 *srcp;
	u32 *dstp;
	u32 scsi_lun[2];

	int_to_scsilun(sc->device->lun, (struct scsi_lun *) scsi_lun);
	cmd->req.lun[0] = be32_to_cpu(scsi_lun[0]);
	cmd->req.lun[1] = be32_to_cpu(scsi_lun[1]);

	lpcnt = cmd->scsi_cmd->cmd_len / sizeof(dword);
	srcp = (u8 *) sc->cmnd;
	dstp = (u32 *) cmd->req.cdb;
	while (lpcnt--) {
		memcpy(&dword, (const void *) srcp, 4);
		*dstp = cpu_to_be32(dword);
		srcp += 4;
		dstp++;
	}
	if (sc->cmd_len & 0x3) {
		dword = (u32) srcp[0] | ((u32) srcp[1] << 8);
		*dstp = cpu_to_be32(dword);
	}
}

static void bnx2i_cleanup_task(struct iscsi_task *task)
{
	struct iscsi_conn *conn = task->conn;
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	struct bnx2i_hba *hba = bnx2i_conn->hba;

	
	if (!task->sc || task->state == ISCSI_TASK_PENDING)
		return;
	
	if (task->state == ISCSI_TASK_ABRT_TMF) {
		bnx2i_send_cmd_cleanup_req(hba, task->dd_data);

		spin_unlock_bh(&conn->session->lock);
		wait_for_completion_timeout(&bnx2i_conn->cmd_cleanup_cmpl,
				msecs_to_jiffies(ISCSI_CMD_CLEANUP_TIMEOUT));
		spin_lock_bh(&conn->session->lock);
	}
	bnx2i_iscsi_unmap_sg_list(task->dd_data);
}


static int
bnx2i_mtask_xmit(struct iscsi_conn *conn, struct iscsi_task *task)
{
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	struct bnx2i_cmd *cmd = task->dd_data;

	memset(bnx2i_conn->gen_pdu.req_buf, 0, ISCSI_DEF_MAX_RECV_SEG_LEN);

	bnx2i_setup_cmd_wqe_template(cmd);
	bnx2i_conn->gen_pdu.req_buf_size = task->data_count;
	if (task->data_count) {
		memcpy(bnx2i_conn->gen_pdu.req_buf, task->data,
		       task->data_count);
		bnx2i_conn->gen_pdu.req_wr_ptr =
			bnx2i_conn->gen_pdu.req_buf + task->data_count;
	}
	cmd->conn = conn->dd_data;
	cmd->scsi_cmd = NULL;
	return bnx2i_iscsi_send_generic_request(task);
}


static int bnx2i_task_xmit(struct iscsi_task *task)
{
	struct iscsi_conn *conn = task->conn;
	struct iscsi_session *session = conn->session;
	struct Scsi_Host *shost = iscsi_session_to_shost(session->cls_session);
	struct bnx2i_hba *hba = iscsi_host_priv(shost);
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	struct scsi_cmnd *sc = task->sc;
	struct bnx2i_cmd *cmd = task->dd_data;
	struct iscsi_cmd *hdr = (struct iscsi_cmd *) task->hdr;

	if (!bnx2i_conn->is_bound)
		return -ENOTCONN;

	
	if (!sc)
		return bnx2i_mtask_xmit(conn, task);

	bnx2i_setup_cmd_wqe_template(cmd);
	cmd->req.op_code = ISCSI_OP_SCSI_CMD;
	cmd->conn = bnx2i_conn;
	cmd->scsi_cmd = sc;
	cmd->req.total_data_transfer_length = scsi_bufflen(sc);
	cmd->req.cmd_sn = be32_to_cpu(hdr->cmdsn);

	bnx2i_iscsi_map_sg_list(cmd);
	bnx2i_cpy_scsi_cdb(sc, cmd);

	cmd->req.op_attr = ISCSI_ATTR_SIMPLE;
	if (sc->sc_data_direction == DMA_TO_DEVICE) {
		cmd->req.op_attr |= ISCSI_CMD_REQUEST_WRITE;
		cmd->req.itt = task->itt |
			(ISCSI_TASK_TYPE_WRITE << ISCSI_CMD_REQUEST_TYPE_SHIFT);
		bnx2i_setup_write_cmd_bd_info(task);
	} else {
		if (scsi_bufflen(sc))
			cmd->req.op_attr |= ISCSI_CMD_REQUEST_READ;
		cmd->req.itt = task->itt |
			(ISCSI_TASK_TYPE_READ << ISCSI_CMD_REQUEST_TYPE_SHIFT);
	}

	cmd->req.num_bds = cmd->io_tbl.bd_valid;
	if (!cmd->io_tbl.bd_valid) {
		cmd->req.bd_list_addr_lo = (u32) hba->mp_bd_dma;
		cmd->req.bd_list_addr_hi = (u32) ((u64) hba->mp_bd_dma >> 32);
		cmd->req.num_bds = 1;
	}

	bnx2i_send_iscsi_scsicmd(bnx2i_conn, cmd);
	return 0;
}


static struct iscsi_cls_session *
bnx2i_session_create(struct iscsi_endpoint *ep,
		     uint16_t cmds_max, uint16_t qdepth,
		     uint32_t initial_cmdsn)
{
	struct Scsi_Host *shost;
	struct iscsi_cls_session *cls_session;
	struct bnx2i_hba *hba;
	struct bnx2i_endpoint *bnx2i_ep;

	if (!ep) {
		printk(KERN_ERR "bnx2i: missing ep.\n");
		return NULL;
	}

	bnx2i_ep = ep->dd_data;
	shost = bnx2i_ep->hba->shost;
	hba = iscsi_host_priv(shost);
	if (bnx2i_adapter_ready(hba))
		return NULL;

	
	if (cmds_max > hba->max_sqes)
		cmds_max = hba->max_sqes;
	else if (cmds_max < BNX2I_SQ_WQES_MIN)
		cmds_max = BNX2I_SQ_WQES_MIN;

	cls_session = iscsi_session_setup(&bnx2i_iscsi_transport, shost,
					  cmds_max, 0, sizeof(struct bnx2i_cmd),
					  initial_cmdsn, ISCSI_MAX_TARGET);
	if (!cls_session)
		return NULL;

	if (bnx2i_setup_cmd_pool(hba, cls_session->dd_data))
		goto session_teardown;
	return cls_session;

session_teardown:
	iscsi_session_teardown(cls_session);
	return NULL;
}



static void bnx2i_session_destroy(struct iscsi_cls_session *cls_session)
{
	struct iscsi_session *session = cls_session->dd_data;
	struct Scsi_Host *shost = iscsi_session_to_shost(cls_session);
	struct bnx2i_hba *hba = iscsi_host_priv(shost);

	bnx2i_destroy_cmd_pool(hba, session);
	iscsi_session_teardown(cls_session);
}



static struct iscsi_cls_conn *
bnx2i_conn_create(struct iscsi_cls_session *cls_session, uint32_t cid)
{
	struct Scsi_Host *shost = iscsi_session_to_shost(cls_session);
	struct bnx2i_hba *hba = iscsi_host_priv(shost);
	struct bnx2i_conn *bnx2i_conn;
	struct iscsi_cls_conn *cls_conn;
	struct iscsi_conn *conn;

	cls_conn = iscsi_conn_setup(cls_session, sizeof(*bnx2i_conn),
				    cid);
	if (!cls_conn)
		return NULL;
	conn = cls_conn->dd_data;

	bnx2i_conn = conn->dd_data;
	bnx2i_conn->cls_conn = cls_conn;
	bnx2i_conn->hba = hba;
	
	bnx2i_conn->ep = NULL;
	init_completion(&bnx2i_conn->cmd_cleanup_cmpl);

	if (bnx2i_conn_alloc_login_resources(hba, bnx2i_conn)) {
		iscsi_conn_printk(KERN_ALERT, conn,
				  "conn_new: login resc alloc failed!!\n");
		goto free_conn;
	}

	return cls_conn;

free_conn:
	iscsi_conn_teardown(cls_conn);
	return NULL;
}


static int bnx2i_conn_bind(struct iscsi_cls_session *cls_session,
			   struct iscsi_cls_conn *cls_conn,
			   uint64_t transport_fd, int is_leading)
{
	struct iscsi_conn *conn = cls_conn->dd_data;
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	struct Scsi_Host *shost = iscsi_session_to_shost(cls_session);
	struct bnx2i_hba *hba = iscsi_host_priv(shost);
	struct bnx2i_endpoint *bnx2i_ep;
	struct iscsi_endpoint *ep;
	int ret_code;

	ep = iscsi_lookup_endpoint(transport_fd);
	if (!ep)
		return -EINVAL;

	bnx2i_ep = ep->dd_data;
	if ((bnx2i_ep->state == EP_STATE_TCP_FIN_RCVD) ||
	    (bnx2i_ep->state == EP_STATE_TCP_RST_RCVD))
		
		return -EINVAL;

	if (iscsi_conn_bind(cls_session, cls_conn, is_leading))
		return -EINVAL;

	if (bnx2i_ep->hba != hba) {
		
		iscsi_conn_printk(KERN_ALERT, cls_conn->dd_data,
				  "conn bind, ep=0x%p (%s) does not",
				  bnx2i_ep, bnx2i_ep->hba->netdev->name);
		iscsi_conn_printk(KERN_ALERT, cls_conn->dd_data,
				  "belong to hba (%s)\n",
				  hba->netdev->name);
		return -EEXIST;
	}

	bnx2i_ep->conn = bnx2i_conn;
	bnx2i_conn->ep = bnx2i_ep;
	bnx2i_conn->iscsi_conn_cid = bnx2i_ep->ep_iscsi_cid;
	bnx2i_conn->fw_cid = bnx2i_ep->ep_cid;
	bnx2i_conn->is_bound = 1;

	ret_code = bnx2i_bind_conn_to_iscsi_cid(hba, bnx2i_conn,
						bnx2i_ep->ep_iscsi_cid);

	
	if (test_bit(BNX2I_NX2_DEV_57710, &bnx2i_ep->hba->cnic_dev_type))
		bnx2i_put_rq_buf(bnx2i_conn, 0);

	bnx2i_arm_cq_event_coalescing(bnx2i_conn->ep, CNIC_ARM_CQE);
	return ret_code;
}



static void bnx2i_conn_destroy(struct iscsi_cls_conn *cls_conn)
{
	struct iscsi_conn *conn = cls_conn->dd_data;
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	struct Scsi_Host *shost;
	struct bnx2i_hba *hba;

	shost = iscsi_session_to_shost(iscsi_conn_to_session(cls_conn));
	hba = iscsi_host_priv(shost);

	bnx2i_conn_free_login_resources(hba, bnx2i_conn);
	iscsi_conn_teardown(cls_conn);
}



static int bnx2i_conn_get_param(struct iscsi_cls_conn *cls_conn,
				enum iscsi_param param, char *buf)
{
	struct iscsi_conn *conn = cls_conn->dd_data;
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;
	int len = 0;

	switch (param) {
	case ISCSI_PARAM_CONN_PORT:
		if (bnx2i_conn->ep)
			len = sprintf(buf, "%hu\n",
				      bnx2i_conn->ep->cm_sk->dst_port);
		break;
	case ISCSI_PARAM_CONN_ADDRESS:
		if (bnx2i_conn->ep)
			len = sprintf(buf, NIPQUAD_FMT "\n",
				      NIPQUAD(bnx2i_conn->ep->cm_sk->dst_ip));
		break;
	default:
		return iscsi_conn_get_param(cls_conn, param, buf);
	}

	return len;
}


static int bnx2i_host_get_param(struct Scsi_Host *shost,
				enum iscsi_host_param param, char *buf)
{
	struct bnx2i_hba *hba = iscsi_host_priv(shost);
	int len = 0;

	switch (param) {
	case ISCSI_HOST_PARAM_HWADDRESS:
		len = sysfs_format_mac(buf, hba->cnic->mac_addr, 6);
		break;
	case ISCSI_HOST_PARAM_NETDEV_NAME:
		len = sprintf(buf, "%s\n", hba->netdev->name);
		break;
	default:
		return iscsi_host_get_param(shost, param, buf);
	}
	return len;
}


static int bnx2i_conn_start(struct iscsi_cls_conn *cls_conn)
{
	struct iscsi_conn *conn = cls_conn->dd_data;
	struct bnx2i_conn *bnx2i_conn = conn->dd_data;

	bnx2i_conn->ep->state = EP_STATE_ULP_UPDATE_START;
	bnx2i_update_iscsi_conn(conn);

	
	bnx2i_conn->ep->ofld_timer.expires = 1 * HZ + jiffies;
	bnx2i_conn->ep->ofld_timer.function = bnx2i_ep_ofld_timer;
	bnx2i_conn->ep->ofld_timer.data = (unsigned long) bnx2i_conn->ep;
	add_timer(&bnx2i_conn->ep->ofld_timer);
	
	wait_event_interruptible(bnx2i_conn->ep->ofld_wait,
			bnx2i_conn->ep->state != EP_STATE_ULP_UPDATE_START);

	if (signal_pending(current))
		flush_signals(current);
	del_timer_sync(&bnx2i_conn->ep->ofld_timer);

	iscsi_conn_start(cls_conn);
	return 0;
}



static void bnx2i_conn_get_stats(struct iscsi_cls_conn *cls_conn,
				 struct iscsi_stats *stats)
{
	struct iscsi_conn *conn = cls_conn->dd_data;

	stats->txdata_octets = conn->txdata_octets;
	stats->rxdata_octets = conn->rxdata_octets;
	stats->scsicmd_pdus = conn->scsicmd_pdus_cnt;
	stats->dataout_pdus = conn->dataout_pdus_cnt;
	stats->scsirsp_pdus = conn->scsirsp_pdus_cnt;
	stats->datain_pdus = conn->datain_pdus_cnt;
	stats->r2t_pdus = conn->r2t_pdus_cnt;
	stats->tmfcmd_pdus = conn->tmfcmd_pdus_cnt;
	stats->tmfrsp_pdus = conn->tmfrsp_pdus_cnt;
	stats->custom_length = 3;
	strcpy(stats->custom[2].desc, "eh_abort_cnt");
	stats->custom[2].value = conn->eh_abort_cnt;
	stats->digest_err = 0;
	stats->timeout_err = 0;
	stats->custom_length = 0;
}



static struct bnx2i_hba *bnx2i_check_route(struct sockaddr *dst_addr)
{
	struct sockaddr_in *desti = (struct sockaddr_in *) dst_addr;
	struct bnx2i_hba *hba;
	struct cnic_dev *cnic = NULL;

	bnx2i_reg_dev_all();

	hba = get_adapter_list_head();
	if (hba && hba->cnic)
		cnic = hba->cnic->cm_select_dev(desti, CNIC_ULP_ISCSI);
	if (!cnic) {
		printk(KERN_ALERT "bnx2i: no route,"
		       "can't connect using cnic\n");
		goto no_nx2_route;
	}
	hba = bnx2i_find_hba_for_cnic(cnic);
	if (!hba)
		goto no_nx2_route;

	if (bnx2i_adapter_ready(hba)) {
		printk(KERN_ALERT "bnx2i: check route, hba not found\n");
		goto no_nx2_route;
	}
	if (hba->netdev->mtu > hba->mtu_supported) {
		printk(KERN_ALERT "bnx2i: %s network i/f mtu is set to %d\n",
				  hba->netdev->name, hba->netdev->mtu);
		printk(KERN_ALERT "bnx2i: iSCSI HBA can support mtu of %d\n",
				  hba->mtu_supported);
		goto no_nx2_route;
	}
	return hba;
no_nx2_route:
	return NULL;
}



static int bnx2i_tear_down_conn(struct bnx2i_hba *hba,
				 struct bnx2i_endpoint *ep)
{
	if (test_bit(BNX2I_CNIC_REGISTERED, &hba->reg_with_cnic))
		hba->cnic->cm_destroy(ep->cm_sk);

	if (test_bit(ADAPTER_STATE_GOING_DOWN, &ep->hba->adapter_state))
		ep->state = EP_STATE_DISCONN_COMPL;

	if (test_bit(BNX2I_NX2_DEV_57710, &hba->cnic_dev_type) &&
	    ep->state == EP_STATE_DISCONN_TIMEDOUT) {
		printk(KERN_ALERT "bnx2i - ERROR - please submit GRC Dump,"
				  " NW/PCIe trace, driver msgs to developers"
				  " for analysis\n");
		return 1;
	}

	ep->state = EP_STATE_CLEANUP_START;
	init_timer(&ep->ofld_timer);
	ep->ofld_timer.expires = 10*HZ + jiffies;
	ep->ofld_timer.function = bnx2i_ep_ofld_timer;
	ep->ofld_timer.data = (unsigned long) ep;
	add_timer(&ep->ofld_timer);

	bnx2i_ep_destroy_list_add(hba, ep);

	
	bnx2i_send_conn_destroy(hba, ep);
	wait_event_interruptible(ep->ofld_wait,
				 (ep->state != EP_STATE_CLEANUP_START));

	if (signal_pending(current))
		flush_signals(current);
	del_timer_sync(&ep->ofld_timer);

	bnx2i_ep_destroy_list_del(hba, ep);

	if (ep->state != EP_STATE_CLEANUP_CMPL)
		
		printk(KERN_ALERT "bnx2i - conn destroy failed\n");

	return 0;
}



static struct iscsi_endpoint *bnx2i_ep_connect(struct Scsi_Host *shost,
					       struct sockaddr *dst_addr,
					       int non_blocking)
{
	u32 iscsi_cid = BNX2I_CID_RESERVED;
	struct sockaddr_in *desti = (struct sockaddr_in *) dst_addr;
	struct sockaddr_in6 *desti6;
	struct bnx2i_endpoint *bnx2i_ep;
	struct bnx2i_hba *hba;
	struct cnic_dev *cnic;
	struct cnic_sockaddr saddr;
	struct iscsi_endpoint *ep;
	int rc = 0;

	if (shost) {
		
		hba = iscsi_host_priv(shost);
		
		bnx2i_register_device(hba);
	} else
		
		hba = bnx2i_check_route(dst_addr);

	if (!hba) {
		rc = -ENOMEM;
		goto check_busy;
	}

	cnic = hba->cnic;
	ep = bnx2i_alloc_ep(hba);
	if (!ep) {
		rc = -ENOMEM;
		goto check_busy;
	}
	bnx2i_ep = ep->dd_data;

	mutex_lock(&hba->net_dev_lock);
	if (bnx2i_adapter_ready(hba)) {
		rc = -EPERM;
		goto net_if_down;
	}

	bnx2i_ep->num_active_cmds = 0;
	iscsi_cid = bnx2i_alloc_iscsi_cid(hba);
	if (iscsi_cid == -1) {
		printk(KERN_ALERT "alloc_ep: unable to allocate iscsi cid\n");
		rc = -ENOMEM;
		goto iscsi_cid_err;
	}
	bnx2i_ep->hba_age = hba->age;

	rc = bnx2i_alloc_qp_resc(hba, bnx2i_ep);
	if (rc != 0) {
		printk(KERN_ALERT "bnx2i: ep_conn, alloc QP resc error\n");
		rc = -ENOMEM;
		goto qp_resc_err;
	}

	bnx2i_ep->ep_iscsi_cid = (u16)iscsi_cid;
	bnx2i_ep->state = EP_STATE_OFLD_START;
	bnx2i_ep_ofld_list_add(hba, bnx2i_ep);

	init_timer(&bnx2i_ep->ofld_timer);
	bnx2i_ep->ofld_timer.expires = 2 * HZ + jiffies;
	bnx2i_ep->ofld_timer.function = bnx2i_ep_ofld_timer;
	bnx2i_ep->ofld_timer.data = (unsigned long) bnx2i_ep;
	add_timer(&bnx2i_ep->ofld_timer);

	bnx2i_send_conn_ofld_req(hba, bnx2i_ep);

	
	wait_event_interruptible(bnx2i_ep->ofld_wait,
				 bnx2i_ep->state != EP_STATE_OFLD_START);

	if (signal_pending(current))
		flush_signals(current);
	del_timer_sync(&bnx2i_ep->ofld_timer);

	bnx2i_ep_ofld_list_del(hba, bnx2i_ep);

	if (bnx2i_ep->state != EP_STATE_OFLD_COMPL) {
		rc = -ENOSPC;
		goto conn_failed;
	}

	rc = cnic->cm_create(cnic, CNIC_ULP_ISCSI, bnx2i_ep->ep_cid,
			     iscsi_cid, &bnx2i_ep->cm_sk, bnx2i_ep);
	if (rc) {
		rc = -EINVAL;
		goto conn_failed;
	}

	bnx2i_ep->cm_sk->rcv_buf = 256 * 1024;
	bnx2i_ep->cm_sk->snd_buf = 256 * 1024;
	clear_bit(SK_TCP_TIMESTAMP, &bnx2i_ep->cm_sk->tcp_flags);

	memset(&saddr, 0, sizeof(saddr));
	if (dst_addr->sa_family == AF_INET) {
		desti = (struct sockaddr_in *) dst_addr;
		saddr.remote.v4 = *desti;
		saddr.local.v4.sin_family = desti->sin_family;
	} else if (dst_addr->sa_family == AF_INET6) {
		desti6 = (struct sockaddr_in6 *) dst_addr;
		saddr.remote.v6 = *desti6;
		saddr.local.v6.sin6_family = desti6->sin6_family;
	}

	bnx2i_ep->timestamp = jiffies;
	bnx2i_ep->state = EP_STATE_CONNECT_START;
	if (!test_bit(BNX2I_CNIC_REGISTERED, &hba->reg_with_cnic)) {
		rc = -EINVAL;
		goto conn_failed;
	} else
		rc = cnic->cm_connect(bnx2i_ep->cm_sk, &saddr);

	if (rc)
		goto release_ep;

	if (bnx2i_map_ep_dbell_regs(bnx2i_ep))
		goto release_ep;
	mutex_unlock(&hba->net_dev_lock);
	return ep;

release_ep:
	if (bnx2i_tear_down_conn(hba, bnx2i_ep)) {
		mutex_unlock(&hba->net_dev_lock);
		return ERR_PTR(rc);
	}
conn_failed:
net_if_down:
iscsi_cid_err:
	bnx2i_free_qp_resc(hba, bnx2i_ep);
qp_resc_err:
	bnx2i_free_ep(ep);
	mutex_unlock(&hba->net_dev_lock);
check_busy:
	bnx2i_unreg_dev_all();
	return ERR_PTR(rc);
}



static int bnx2i_ep_poll(struct iscsi_endpoint *ep, int timeout_ms)
{
	struct bnx2i_endpoint *bnx2i_ep;
	int rc = 0;

	bnx2i_ep = ep->dd_data;
	if ((bnx2i_ep->state == EP_STATE_IDLE) ||
	    (bnx2i_ep->state == EP_STATE_CONNECT_FAILED) ||
	    (bnx2i_ep->state == EP_STATE_OFLD_FAILED))
		return -1;
	if (bnx2i_ep->state == EP_STATE_CONNECT_COMPL)
		return 1;

	rc = wait_event_interruptible_timeout(bnx2i_ep->ofld_wait,
					      ((bnx2i_ep->state ==
						EP_STATE_OFLD_FAILED) ||
					       (bnx2i_ep->state ==
						EP_STATE_CONNECT_FAILED) ||
					       (bnx2i_ep->state ==
						EP_STATE_CONNECT_COMPL)),
					      msecs_to_jiffies(timeout_ms));
	if (!rc || (bnx2i_ep->state == EP_STATE_OFLD_FAILED))
		rc = -1;

	if (rc > 0)
		return 1;
	else if (!rc)
		return 0;	
	else
		return rc;
}



static int bnx2i_ep_tcp_conn_active(struct bnx2i_endpoint *bnx2i_ep)
{
	int ret;
	int cnic_dev_10g = 0;

	if (test_bit(BNX2I_NX2_DEV_57710, &bnx2i_ep->hba->cnic_dev_type))
		cnic_dev_10g = 1;

	switch (bnx2i_ep->state) {
	case EP_STATE_CONNECT_START:
	case EP_STATE_CLEANUP_FAILED:
	case EP_STATE_OFLD_FAILED:
	case EP_STATE_DISCONN_TIMEDOUT:
		ret = 0;
		break;
	case EP_STATE_CONNECT_COMPL:
	case EP_STATE_ULP_UPDATE_START:
	case EP_STATE_ULP_UPDATE_COMPL:
	case EP_STATE_TCP_FIN_RCVD:
	case EP_STATE_ULP_UPDATE_FAILED:
		ret = 1;
		break;
	case EP_STATE_TCP_RST_RCVD:
		ret = 0;
		break;
	case EP_STATE_CONNECT_FAILED:
		if (cnic_dev_10g)
			ret = 1;
		else
			ret = 0;
		break;
	default:
		ret = 0;
	}

	return ret;
}



static void bnx2i_ep_disconnect(struct iscsi_endpoint *ep)
{
	struct bnx2i_endpoint *bnx2i_ep;
	struct bnx2i_conn *bnx2i_conn = NULL;
	struct iscsi_session *session = NULL;
	struct iscsi_conn *conn;
	struct cnic_dev *cnic;
	struct bnx2i_hba *hba;

	bnx2i_ep = ep->dd_data;

	
	while ((bnx2i_ep->state == EP_STATE_CONNECT_START) &&
		!time_after(jiffies, bnx2i_ep->timestamp + (12 * HZ)))
		msleep(250);

	if (bnx2i_ep->conn) {
		bnx2i_conn = bnx2i_ep->conn;
		conn = bnx2i_conn->cls_conn->dd_data;
		session = conn->session;

		spin_lock_bh(&session->lock);
		bnx2i_conn->is_bound = 0;
		spin_unlock_bh(&session->lock);
	}

	hba = bnx2i_ep->hba;
	if (bnx2i_ep->state == EP_STATE_IDLE)
		goto return_bnx2i_ep;
	cnic = hba->cnic;

	mutex_lock(&hba->net_dev_lock);

	if (!test_bit(ADAPTER_STATE_UP, &hba->adapter_state))
		goto free_resc;
	if (bnx2i_ep->hba_age != hba->age)
		goto free_resc;

	if (!bnx2i_ep_tcp_conn_active(bnx2i_ep))
		goto destory_conn;

	bnx2i_ep->state = EP_STATE_DISCONN_START;

	init_timer(&bnx2i_ep->ofld_timer);
	bnx2i_ep->ofld_timer.expires = 10*HZ + jiffies;
	bnx2i_ep->ofld_timer.function = bnx2i_ep_ofld_timer;
	bnx2i_ep->ofld_timer.data = (unsigned long) bnx2i_ep;
	add_timer(&bnx2i_ep->ofld_timer);

	if (test_bit(BNX2I_CNIC_REGISTERED, &hba->reg_with_cnic)) {
		int close = 0;

		if (session) {
			spin_lock_bh(&session->lock);
			if (session->state == ISCSI_STATE_LOGGING_OUT)
				close = 1;
			spin_unlock_bh(&session->lock);
		}
		if (close)
			cnic->cm_close(bnx2i_ep->cm_sk);
		else
			cnic->cm_abort(bnx2i_ep->cm_sk);
	} else
		goto free_resc;

	
	wait_event_interruptible(bnx2i_ep->ofld_wait,
				 bnx2i_ep->state != EP_STATE_DISCONN_START);

	if (signal_pending(current))
		flush_signals(current);
	del_timer_sync(&bnx2i_ep->ofld_timer);

destory_conn:
	if (bnx2i_tear_down_conn(hba, bnx2i_ep)) {
		mutex_unlock(&hba->net_dev_lock);
		return;
	}
free_resc:
	mutex_unlock(&hba->net_dev_lock);
	bnx2i_free_qp_resc(hba, bnx2i_ep);
return_bnx2i_ep:
	if (bnx2i_conn)
		bnx2i_conn->ep = NULL;

	bnx2i_free_ep(ep);

	if (!hba->ofld_conns_active)
		bnx2i_unreg_dev_all();
}



static int bnx2i_nl_set_path(struct Scsi_Host *shost, struct iscsi_path *params)
{
	struct bnx2i_hba *hba = iscsi_host_priv(shost);
	char *buf = (char *) params;
	u16 len = sizeof(*params);

	
	hba->cnic->iscsi_nl_msg_recv(hba->cnic, ISCSI_UEVENT_PATH_UPDATE, buf,
				     len);

	return 0;
}



static struct scsi_host_template bnx2i_host_template = {
	.module			= THIS_MODULE,
	.name			= "Broadcom Offload iSCSI Initiator",
	.proc_name		= "bnx2i",
	.queuecommand		= iscsi_queuecommand,
	.eh_abort_handler	= iscsi_eh_abort,
	.eh_device_reset_handler = iscsi_eh_device_reset,
	.eh_target_reset_handler = iscsi_eh_target_reset,
	.can_queue		= 1024,
	.max_sectors		= 127,
	.cmd_per_lun		= 32,
	.this_id		= -1,
	.use_clustering		= ENABLE_CLUSTERING,
	.sg_tablesize		= ISCSI_MAX_BDS_PER_CMD,
	.shost_attrs		= bnx2i_dev_attributes,
};

struct iscsi_transport bnx2i_iscsi_transport = {
	.owner			= THIS_MODULE,
	.name			= "bnx2i",
	.caps			= CAP_RECOVERY_L0 | CAP_HDRDGST |
				  CAP_MULTI_R2T | CAP_DATADGST |
				  CAP_DATA_PATH_OFFLOAD,
	.param_mask		= ISCSI_MAX_RECV_DLENGTH |
				  ISCSI_MAX_XMIT_DLENGTH |
				  ISCSI_HDRDGST_EN |
				  ISCSI_DATADGST_EN |
				  ISCSI_INITIAL_R2T_EN |
				  ISCSI_MAX_R2T |
				  ISCSI_IMM_DATA_EN |
				  ISCSI_FIRST_BURST |
				  ISCSI_MAX_BURST |
				  ISCSI_PDU_INORDER_EN |
				  ISCSI_DATASEQ_INORDER_EN |
				  ISCSI_ERL |
				  ISCSI_CONN_PORT |
				  ISCSI_CONN_ADDRESS |
				  ISCSI_EXP_STATSN |
				  ISCSI_PERSISTENT_PORT |
				  ISCSI_PERSISTENT_ADDRESS |
				  ISCSI_TARGET_NAME | ISCSI_TPGT |
				  ISCSI_USERNAME | ISCSI_PASSWORD |
				  ISCSI_USERNAME_IN | ISCSI_PASSWORD_IN |
				  ISCSI_FAST_ABORT | ISCSI_ABORT_TMO |
				  ISCSI_LU_RESET_TMO |
				  ISCSI_PING_TMO | ISCSI_RECV_TMO |
				  ISCSI_IFACE_NAME | ISCSI_INITIATOR_NAME,
	.host_param_mask	= ISCSI_HOST_HWADDRESS | ISCSI_HOST_NETDEV_NAME,
	.create_session		= bnx2i_session_create,
	.destroy_session	= bnx2i_session_destroy,
	.create_conn		= bnx2i_conn_create,
	.bind_conn		= bnx2i_conn_bind,
	.destroy_conn		= bnx2i_conn_destroy,
	.set_param		= iscsi_set_param,
	.get_conn_param		= bnx2i_conn_get_param,
	.get_session_param	= iscsi_session_get_param,
	.get_host_param		= bnx2i_host_get_param,
	.start_conn		= bnx2i_conn_start,
	.stop_conn		= iscsi_conn_stop,
	.send_pdu		= iscsi_conn_send_pdu,
	.xmit_task		= bnx2i_task_xmit,
	.get_stats		= bnx2i_conn_get_stats,
	
	.ep_connect		= bnx2i_ep_connect,
	.ep_poll		= bnx2i_ep_poll,
	.ep_disconnect		= bnx2i_ep_disconnect,
	.set_path		= bnx2i_nl_set_path,
	
	.session_recovery_timedout = iscsi_session_recovery_timedout,
	.cleanup_task		= bnx2i_cleanup_task,
};
