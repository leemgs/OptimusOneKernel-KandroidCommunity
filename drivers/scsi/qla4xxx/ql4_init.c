

#include <scsi/iscsi_if.h>
#include "ql4_def.h"
#include "ql4_glbl.h"
#include "ql4_dbg.h"
#include "ql4_inline.h"

static struct ddb_entry * qla4xxx_alloc_ddb(struct scsi_qla_host *ha,
					    uint32_t fw_ddb_index);

static void ql4xxx_set_mac_number(struct scsi_qla_host *ha)
{
	uint32_t value;
	uint8_t func_number;
	unsigned long flags;

	
	spin_lock_irqsave(&ha->hardware_lock, flags);
	value = readw(&ha->reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	func_number = (uint8_t) ((value >> 4) & 0x30);
	switch (value & ISP_CONTROL_FN_MASK) {
	case ISP_CONTROL_FN0_SCSI:
		ha->mac_index = 1;
		break;
	case ISP_CONTROL_FN1_SCSI:
		ha->mac_index = 3;
		break;
	default:
		DEBUG2(printk("scsi%ld: %s: Invalid function number, "
			      "ispControlStatus = 0x%x\n", ha->host_no,
			      __func__, value));
		break;
	}
	DEBUG2(printk("scsi%ld: %s: mac_index %d.\n", ha->host_no, __func__,
		      ha->mac_index));
}


static void qla4xxx_free_ddb(struct scsi_qla_host *ha,
			     struct ddb_entry *ddb_entry)
{
	
	list_del_init(&ddb_entry->list);

	
	ha->fw_ddb_index_map[ddb_entry->fw_ddb_index] =
		(struct ddb_entry *) INVALID_ENTRY;
	ha->tot_ddbs--;

	
	qla4xxx_destroy_sess(ddb_entry);
}


void qla4xxx_free_ddb_list(struct scsi_qla_host *ha)
{
	struct list_head *ptr;
	struct ddb_entry *ddb_entry;

	while (!list_empty(&ha->ddb_list)) {
		ptr = ha->ddb_list.next;
		
		ddb_entry = list_entry(ptr, struct ddb_entry, list);
		qla4xxx_free_ddb(ha, ddb_entry);
	}
}


int qla4xxx_init_rings(struct scsi_qla_host *ha)
{
	unsigned long flags = 0;

	
	spin_lock_irqsave(&ha->hardware_lock, flags);
	ha->request_out = 0;
	ha->request_in = 0;
	ha->request_ptr = &ha->request_ring[ha->request_in];
	ha->req_q_count = REQUEST_QUEUE_DEPTH;

	
	ha->response_in = 0;
	ha->response_out = 0;
	ha->response_ptr = &ha->response_ring[ha->response_out];

	
	ha->shadow_regs->req_q_out = __constant_cpu_to_le32(0);
	ha->shadow_regs->rsp_q_in = __constant_cpu_to_le32(0);
	wmb();

	writel(0, &ha->reg->req_q_in);
	writel(0, &ha->reg->rsp_q_out);
	readl(&ha->reg->rsp_q_out);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_SUCCESS;
}


static int qla4xxx_validate_mac_address(struct scsi_qla_host *ha)
{
	struct flash_sys_info *sys_info;
	dma_addr_t sys_info_dma;
	int status = QLA_ERROR;

	sys_info = dma_alloc_coherent(&ha->pdev->dev, sizeof(*sys_info),
				      &sys_info_dma, GFP_KERNEL);
	if (sys_info == NULL) {
		DEBUG2(printk("scsi%ld: %s: Unable to allocate dma buffer.\n",
			      ha->host_no, __func__));

		goto exit_validate_mac_no_free;
	}
	memset(sys_info, 0, sizeof(*sys_info));

	
	if (qla4xxx_get_flash(ha, sys_info_dma, FLASH_OFFSET_SYS_INFO,
			      sizeof(*sys_info)) != QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: get_flash FLASH_OFFSET_SYS_INFO "
			      "failed\n", ha->host_no, __func__));

		goto exit_validate_mac;
	}

	
	memcpy(ha->my_mac, &sys_info->physAddr[0].address[0],
	       min(sizeof(ha->my_mac),
		   sizeof(sys_info->physAddr[0].address)));
	memcpy(ha->serial_number, &sys_info->acSerialNumber,
	       min(sizeof(ha->serial_number),
		   sizeof(sys_info->acSerialNumber)));

	status = QLA_SUCCESS;

 exit_validate_mac:
	dma_free_coherent(&ha->pdev->dev, sizeof(*sys_info), sys_info,
			  sys_info_dma);

 exit_validate_mac_no_free:
	return status;
}


static int qla4xxx_init_local_data(struct scsi_qla_host *ha)
{
	
	ha->aen_q_count = MAX_AEN_ENTRIES;

	return qla4xxx_get_firmware_status(ha);
}

static int qla4xxx_fw_ready(struct scsi_qla_host *ha)
{
	uint32_t timeout_count;
	int ready = 0;

	DEBUG2(dev_info(&ha->pdev->dev, "Waiting for Firmware Ready..\n"));
	for (timeout_count = ADAPTER_INIT_TOV; timeout_count > 0;
	     timeout_count--) {
		if (test_and_clear_bit(DPC_GET_DHCP_IP_ADDR, &ha->dpc_flags))
			qla4xxx_get_dhcp_ip_address(ha);

		
		if (qla4xxx_get_firmware_state(ha) != QLA_SUCCESS) {
			DEBUG2(printk("scsi%ld: %s: unable to get firmware "
				      "state\n", ha->host_no, __func__));
			break;

		}

		if (ha->firmware_state & FW_STATE_ERROR) {
			DEBUG2(printk("scsi%ld: %s: an unrecoverable error has"
				      " occurred\n", ha->host_no, __func__));
			break;

		}
		if (ha->firmware_state & FW_STATE_CONFIG_WAIT) {
			
			if (qla4xxx_initialize_fw_cb(ha) == QLA_ERROR)
				break;

			
			continue;
		}

		if (ha->firmware_state == FW_STATE_READY) {
			DEBUG2(dev_info(&ha->pdev->dev, "Firmware Ready..\n"));
			
			DEBUG2(dev_info(&ha->pdev->dev,
					  "scsi%ld: %s: MEDIA TYPE - %s\n",
					  ha->host_no,
					  __func__, (ha->addl_fw_state &
						     FW_ADDSTATE_OPTICAL_MEDIA)
					  != 0 ? "OPTICAL" : "COPPER"));
			DEBUG2(dev_info(&ha->pdev->dev,
					  "scsi%ld: %s: DHCP STATE Enabled "
					  "%s\n",
					  ha->host_no, __func__,
					  (ha->addl_fw_state &
					   FW_ADDSTATE_DHCP_ENABLED) != 0 ?
					  "YES" : "NO"));
			DEBUG2(dev_info(&ha->pdev->dev,
					  "scsi%ld: %s: LINK %s\n",
					  ha->host_no, __func__,
					  (ha->addl_fw_state &
					   FW_ADDSTATE_LINK_UP) != 0 ?
					  "UP" : "DOWN"));
			DEBUG2(dev_info(&ha->pdev->dev,
					  "scsi%ld: %s: iSNS Service "
					  "Started %s\n",
					  ha->host_no, __func__,
					  (ha->addl_fw_state &
					   FW_ADDSTATE_ISNS_SVC_ENABLED) != 0 ?
					  "YES" : "NO"));

			ready = 1;
			break;
		}
		DEBUG2(printk("scsi%ld: %s: waiting on fw, state=%x:%x - "
			      "seconds expired= %d\n", ha->host_no, __func__,
			      ha->firmware_state, ha->addl_fw_state,
			      timeout_count));
		if (is_qla4032(ha) &&
			!(ha->addl_fw_state & FW_ADDSTATE_LINK_UP) &&
			(timeout_count < ADAPTER_INIT_TOV - 5)) {
			break;
		}

		msleep(1000);
	}			

	if (timeout_count == 0)
		DEBUG2(printk("scsi%ld: %s: FW Initialization timed out!\n",
			      ha->host_no, __func__));

	if (ha->firmware_state & FW_STATE_DHCP_IN_PROGRESS)  {
		DEBUG2(printk("scsi%ld: %s: FW is reporting its waiting to"
			      " grab an IP address from DHCP server\n",
			      ha->host_no, __func__));
		ready = 1;
	}

	return ready;
}


static int qla4xxx_init_firmware(struct scsi_qla_host *ha)
{
	int status = QLA_ERROR;

	dev_info(&ha->pdev->dev, "Initializing firmware..\n");
	if (qla4xxx_initialize_fw_cb(ha) == QLA_ERROR) {
		DEBUG2(printk("scsi%ld: %s: Failed to initialize firmware "
			      "control block\n", ha->host_no, __func__));
		return status;
	}
	if (!qla4xxx_fw_ready(ha))
		return status;

	return qla4xxx_get_firmware_status(ha);
}

static struct ddb_entry* qla4xxx_get_ddb_entry(struct scsi_qla_host *ha,
						uint32_t fw_ddb_index,
						uint32_t *new_tgt)
{
	struct dev_db_entry *fw_ddb_entry = NULL;
	dma_addr_t fw_ddb_entry_dma;
	struct ddb_entry *ddb_entry = NULL;
	int found = 0;
	uint32_t device_state;

	*new_tgt = 0;
	
	fw_ddb_entry = dma_alloc_coherent(&ha->pdev->dev,
					  sizeof(*fw_ddb_entry),
					  &fw_ddb_entry_dma, GFP_KERNEL);
	if (fw_ddb_entry == NULL) {
		DEBUG2(printk("scsi%ld: %s: Unable to allocate dma buffer.\n",
			      ha->host_no, __func__));
		return NULL;
	}

	if (qla4xxx_get_fwddb_entry(ha, fw_ddb_index, fw_ddb_entry,
				    fw_ddb_entry_dma, NULL, NULL,
				    &device_state, NULL, NULL, NULL) ==
	    QLA_ERROR) {
		DEBUG2(printk("scsi%ld: %s: failed get_ddb_entry for "
			      "fw_ddb_index %d\n", ha->host_no, __func__,
			      fw_ddb_index));
		return NULL;
	}

	
	DEBUG2(printk("scsi%ld: %s: Looking for ddb[%d]\n", ha->host_no,
		      __func__, fw_ddb_index));
	list_for_each_entry(ddb_entry, &ha->ddb_list, list) {
		if ((memcmp(ddb_entry->iscsi_name, fw_ddb_entry->iscsi_name,
			   ISCSI_NAME_SIZE) == 0) &&
			(ddb_entry->tpgt ==
				le32_to_cpu(fw_ddb_entry->tgt_portal_grp)) &&
			(memcmp(ddb_entry->isid, fw_ddb_entry->isid,
				sizeof(ddb_entry->isid)) == 0)) {
			found++;
			break;
		}
	}

	if (!found) {
		DEBUG2(printk("scsi%ld: %s: ddb[%d] not found - allocating "
			      "new ddb\n", ha->host_no, __func__,
			      fw_ddb_index));
		*new_tgt = 1;
		ddb_entry = qla4xxx_alloc_ddb(ha, fw_ddb_index);
	}

	
	dma_free_coherent(&ha->pdev->dev, sizeof(*fw_ddb_entry), fw_ddb_entry,
			  fw_ddb_entry_dma);

	return ddb_entry;
}


static int qla4xxx_update_ddb_entry(struct scsi_qla_host *ha,
				    struct ddb_entry *ddb_entry,
				    uint32_t fw_ddb_index)
{
	struct dev_db_entry *fw_ddb_entry = NULL;
	dma_addr_t fw_ddb_entry_dma;
	int status = QLA_ERROR;

	if (ddb_entry == NULL) {
		DEBUG2(printk("scsi%ld: %s: ddb_entry is NULL\n", ha->host_no,
			      __func__));
		goto exit_update_ddb;
	}

	
	fw_ddb_entry = dma_alloc_coherent(&ha->pdev->dev,
					  sizeof(*fw_ddb_entry),
					  &fw_ddb_entry_dma, GFP_KERNEL);
	if (fw_ddb_entry == NULL) {
		DEBUG2(printk("scsi%ld: %s: Unable to allocate dma buffer.\n",
			      ha->host_no, __func__));

		goto exit_update_ddb;
	}

	if (qla4xxx_get_fwddb_entry(ha, fw_ddb_index, fw_ddb_entry,
				    fw_ddb_entry_dma, NULL, NULL,
				    &ddb_entry->fw_ddb_device_state, NULL,
				    &ddb_entry->tcp_source_port_num,
				    &ddb_entry->connection_id) ==
	    QLA_ERROR) {
		DEBUG2(printk("scsi%ld: %s: failed get_ddb_entry for "
			      "fw_ddb_index %d\n", ha->host_no, __func__,
			      fw_ddb_index));

		goto exit_update_ddb;
	}

	status = QLA_SUCCESS;
	ddb_entry->target_session_id = le16_to_cpu(fw_ddb_entry->tsid);
	ddb_entry->task_mgmt_timeout =
		le16_to_cpu(fw_ddb_entry->def_timeout);
	ddb_entry->CmdSn = 0;
	ddb_entry->exe_throttle = le16_to_cpu(fw_ddb_entry->exec_throttle);
	ddb_entry->default_relogin_timeout =
		le16_to_cpu(fw_ddb_entry->def_timeout);
	ddb_entry->default_time2wait = le16_to_cpu(fw_ddb_entry->iscsi_def_time2wait);

	
	ddb_entry->fw_ddb_index = fw_ddb_index;
	ha->fw_ddb_index_map[fw_ddb_index] = ddb_entry;

	ddb_entry->port = le16_to_cpu(fw_ddb_entry->port);
	ddb_entry->tpgt = le32_to_cpu(fw_ddb_entry->tgt_portal_grp);
	memcpy(ddb_entry->isid, fw_ddb_entry->isid, sizeof(ddb_entry->isid));

	memcpy(&ddb_entry->iscsi_name[0], &fw_ddb_entry->iscsi_name[0],
	       min(sizeof(ddb_entry->iscsi_name),
		   sizeof(fw_ddb_entry->iscsi_name)));
	memcpy(&ddb_entry->ip_addr[0], &fw_ddb_entry->ip_addr[0],
	       min(sizeof(ddb_entry->ip_addr), sizeof(fw_ddb_entry->ip_addr)));

	DEBUG2(printk("scsi%ld: %s: ddb[%d] - State= %x status= %d.\n",
		      ha->host_no, __func__, fw_ddb_index,
		      ddb_entry->fw_ddb_device_state, status));

 exit_update_ddb:
	if (fw_ddb_entry)
		dma_free_coherent(&ha->pdev->dev, sizeof(*fw_ddb_entry),
				  fw_ddb_entry, fw_ddb_entry_dma);

	return status;
}


static struct ddb_entry * qla4xxx_alloc_ddb(struct scsi_qla_host *ha,
					    uint32_t fw_ddb_index)
{
	struct ddb_entry *ddb_entry;

	DEBUG2(printk("scsi%ld: %s: fw_ddb_index [%d]\n", ha->host_no,
		      __func__, fw_ddb_index));

	ddb_entry = qla4xxx_alloc_sess(ha);
	if (ddb_entry == NULL) {
		DEBUG2(printk("scsi%ld: %s: Unable to allocate memory "
			      "to add fw_ddb_index [%d]\n",
			      ha->host_no, __func__, fw_ddb_index));
		return ddb_entry;
	}

	ddb_entry->fw_ddb_index = fw_ddb_index;
	atomic_set(&ddb_entry->port_down_timer, ha->port_down_retry_count);
	atomic_set(&ddb_entry->retry_relogin_timer, INVALID_ENTRY);
	atomic_set(&ddb_entry->relogin_timer, 0);
	atomic_set(&ddb_entry->relogin_retry_count, 0);
	atomic_set(&ddb_entry->state, DDB_STATE_ONLINE);
	list_add_tail(&ddb_entry->list, &ha->ddb_list);
	ha->fw_ddb_index_map[fw_ddb_index] = ddb_entry;
	ha->tot_ddbs++;

	return ddb_entry;
}


static int qla4xxx_build_ddb_list(struct scsi_qla_host *ha)
{
	int status = QLA_SUCCESS;
	uint32_t fw_ddb_index = 0;
	uint32_t next_fw_ddb_index = 0;
	uint32_t ddb_state;
	uint32_t conn_err, err_code;
	struct ddb_entry *ddb_entry;
	uint32_t new_tgt;

	dev_info(&ha->pdev->dev, "Initializing DDBs ...\n");
	for (fw_ddb_index = 0; fw_ddb_index < MAX_DDB_ENTRIES;
	     fw_ddb_index = next_fw_ddb_index) {
		
		if (qla4xxx_get_fwddb_entry(ha, fw_ddb_index, NULL, 0, NULL,
					    &next_fw_ddb_index, &ddb_state,
					    &conn_err, NULL, NULL) ==
		    QLA_ERROR) {
			DEBUG2(printk("scsi%ld: %s: get_ddb_entry, "
				      "fw_ddb_index %d failed", ha->host_no,
				      __func__, fw_ddb_index));
			return QLA_ERROR;
		}

		DEBUG2(printk("scsi%ld: %s: Getting DDB[%d] ddbstate=0x%x, "
			      "next_fw_ddb_index=%d.\n", ha->host_no, __func__,
			      fw_ddb_index, ddb_state, next_fw_ddb_index));

		
		if (ddb_state == DDB_DS_SESSION_FAILED ||
		    ddb_state == DDB_DS_NO_CONNECTION_ACTIVE) {
			
			DEBUG2(printk("scsi%ld: %s: Login to DDB[%d]\n",
				      ha->host_no, __func__, fw_ddb_index));
			err_code = ((conn_err & 0x00ff0000) >> 16);
			if (err_code == 0x1c || err_code == 0x06) {
				DEBUG2(printk("scsi%ld: %s send target "
					      "completed "
					      "or access denied failure\n",
					      ha->host_no, __func__));
			} else {
				qla4xxx_set_ddb_entry(ha, fw_ddb_index, 0);
				if (qla4xxx_get_fwddb_entry(ha, fw_ddb_index,
					NULL, 0, NULL, &next_fw_ddb_index,
					&ddb_state, &conn_err, NULL, NULL)
					== QLA_ERROR) {
					DEBUG2(printk("scsi%ld: %s:"
						"get_ddb_entry %d failed\n",
						ha->host_no,
						__func__, fw_ddb_index));
					return QLA_ERROR;
				}
			}
		}

		if (ddb_state != DDB_DS_SESSION_ACTIVE)
			goto next_one;
		
		DEBUG2(printk("scsi%ld: %s: DDB[%d] added to list\n",
			      ha->host_no, __func__, fw_ddb_index));

		
		ddb_entry = qla4xxx_get_ddb_entry(ha, fw_ddb_index, &new_tgt);
		if (ddb_entry == NULL) {
			DEBUG2(printk("scsi%ld: %s: Unable to allocate memory "
				      "for device at fw_ddb_index %d\n",
				      ha->host_no, __func__, fw_ddb_index));
			return QLA_ERROR;
		}
		
		if (qla4xxx_update_ddb_entry(ha, ddb_entry, fw_ddb_index) ==
		    QLA_ERROR) {
			ha->fw_ddb_index_map[fw_ddb_index] =
				(struct ddb_entry *)INVALID_ENTRY;


			DEBUG2(printk("scsi%ld: %s: update_ddb_entry failed "
				      "for fw_ddb_index %d.\n",
				      ha->host_no, __func__, fw_ddb_index));
			return QLA_ERROR;
		}

next_one:
		
		if (next_fw_ddb_index == 0)
			break;
	}

	dev_info(&ha->pdev->dev, "DDB list done..\n");

	return status;
}

struct qla4_relog_scan {
	int halt_wait;
	uint32_t conn_err;
	uint32_t err_code;
	uint32_t fw_ddb_index;
	uint32_t next_fw_ddb_index;
	uint32_t fw_ddb_device_state;
};

static int qla4_test_rdy(struct scsi_qla_host *ha, struct qla4_relog_scan *rs)
{
	struct ddb_entry *ddb_entry;

	
	rs->err_code = ((rs->conn_err & 0x00ff0000) >> 16);
	if (rs->err_code == 0x1c || rs->err_code == 0x06) {
		DEBUG2(printk(
			       "scsi%ld: %s send target"
			       " completed or "
			       "access denied failure\n",
			       ha->host_no, __func__));
	} else {
		
		rs->halt_wait = 0;

		ddb_entry = qla4xxx_lookup_ddb_by_fw_index(ha,
							   rs->fw_ddb_index);
		if (ddb_entry == NULL)
			return QLA_ERROR;

		if (ddb_entry->dev_scan_wait_to_start_relogin != 0
		    && time_after_eq(jiffies,
				     ddb_entry->
				     dev_scan_wait_to_start_relogin))
		{
			ddb_entry->dev_scan_wait_to_start_relogin = 0;
			qla4xxx_set_ddb_entry(ha, rs->fw_ddb_index, 0);
		}
	}
	return QLA_SUCCESS;
}

static int qla4_scan_for_relogin(struct scsi_qla_host *ha,
				 struct qla4_relog_scan *rs)
{
	int error;

	
	for (rs->fw_ddb_index = 0; rs->fw_ddb_index < MAX_DDB_ENTRIES;
	     rs->fw_ddb_index = rs->next_fw_ddb_index) {
		if (qla4xxx_get_fwddb_entry(ha, rs->fw_ddb_index, NULL, 0,
					    NULL, &rs->next_fw_ddb_index,
					    &rs->fw_ddb_device_state,
					    &rs->conn_err, NULL, NULL)
		    == QLA_ERROR)
			return QLA_ERROR;

		if (rs->fw_ddb_device_state == DDB_DS_LOGIN_IN_PROCESS)
			rs->halt_wait = 0;

		if (rs->fw_ddb_device_state == DDB_DS_SESSION_FAILED ||
		    rs->fw_ddb_device_state == DDB_DS_NO_CONNECTION_ACTIVE) {
			error = qla4_test_rdy(ha, rs);
			if (error)
				return error;
		}

		
		if (rs->next_fw_ddb_index == 0)
			break;
	}
	return QLA_SUCCESS;
}


static int qla4xxx_devices_ready(struct scsi_qla_host *ha)
{
	int error;
	unsigned long discovery_wtime;
	struct qla4_relog_scan rs;

	discovery_wtime = jiffies + (ql4xdiscoverywait * HZ);

	DEBUG(printk("Waiting (%d) for devices ...\n", ql4xdiscoverywait));
	do {
		
		qla4xxx_get_firmware_state(ha);
		if (test_and_clear_bit(DPC_AEN, &ha->dpc_flags)) {
			
			qla4xxx_process_aen(ha, RELOGIN_DDB_CHANGED_AENS);
		}

		
		rs.halt_wait = 1;

		error = qla4_scan_for_relogin(ha, &rs);

		if (rs.halt_wait) {
			DEBUG2(printk("scsi%ld: %s: Delay halted.  Devices "
				      "Ready.\n", ha->host_no, __func__));
			return QLA_SUCCESS;
		}

		msleep(2000);
	} while (!time_after_eq(jiffies, discovery_wtime));

	DEBUG3(qla4xxx_get_conn_event_log(ha));

	return QLA_SUCCESS;
}

static void qla4xxx_flush_AENS(struct scsi_qla_host *ha)
{
	unsigned long wtime;

	
	wtime = jiffies + (2 * HZ);
	do {
		if (qla4xxx_get_firmware_state(ha) == QLA_SUCCESS)
			if (ha->firmware_state & (BIT_2 | BIT_0))
				return;

		if (test_and_clear_bit(DPC_AEN, &ha->dpc_flags))
			qla4xxx_process_aen(ha, FLUSH_DDB_CHANGED_AENS);

		msleep(1000);
	} while (!time_after_eq(jiffies, wtime));

}

static int qla4xxx_initialize_ddb_list(struct scsi_qla_host *ha)
{
	uint16_t fw_ddb_index;
	int status = QLA_SUCCESS;

	
	if (!list_empty(&ha->ddb_list))
		qla4xxx_free_ddb_list(ha);

	for (fw_ddb_index = 0; fw_ddb_index < MAX_DDB_ENTRIES; fw_ddb_index++)
		ha->fw_ddb_index_map[fw_ddb_index] =
			(struct ddb_entry *)INVALID_ENTRY;

	ha->tot_ddbs = 0;

	qla4xxx_flush_AENS(ha);

	
	if ((status = qla4xxx_build_ddb_list(ha)) == QLA_ERROR)
		return status;

	
	qla4xxx_devices_ready(ha);

	
	if (test_and_clear_bit(DPC_AEN, &ha->dpc_flags))
		qla4xxx_process_aen(ha, PROCESS_ALL_AENS);

	return status;
}


int qla4xxx_reinitialize_ddb_list(struct scsi_qla_host *ha)
{
	int status = QLA_SUCCESS;
	struct ddb_entry *ddb_entry, *detemp;

	
	list_for_each_entry_safe(ddb_entry, detemp, &ha->ddb_list, list) {
		qla4xxx_update_ddb_entry(ha, ddb_entry,
					 ddb_entry->fw_ddb_index);
		if (ddb_entry->fw_ddb_device_state == DDB_DS_SESSION_ACTIVE) {
			atomic_set(&ddb_entry->state, DDB_STATE_ONLINE);
			DEBUG2(printk ("scsi%ld: %s: ddb index [%d] marked "
				       "ONLINE\n", ha->host_no, __func__,
				       ddb_entry->fw_ddb_index));
		} else if (atomic_read(&ddb_entry->state) == DDB_STATE_ONLINE)
			qla4xxx_mark_device_missing(ha, ddb_entry);
	}
	return status;
}


int qla4xxx_relogin_device(struct scsi_qla_host *ha,
			   struct ddb_entry * ddb_entry)
{
	uint16_t relogin_timer;

	relogin_timer = max(ddb_entry->default_relogin_timeout,
			    (uint16_t)RELOGIN_TOV);
	atomic_set(&ddb_entry->relogin_timer, relogin_timer);

	DEBUG2(printk("scsi%ld: Relogin index [%d]. TOV=%d\n", ha->host_no,
		      ddb_entry->fw_ddb_index, relogin_timer));

	qla4xxx_set_ddb_entry(ha, ddb_entry->fw_ddb_index, 0);

	return QLA_SUCCESS;
}

static int qla4xxx_config_nvram(struct scsi_qla_host *ha)
{
	unsigned long flags;
	union external_hw_config_reg extHwConfig;

	DEBUG2(printk("scsi%ld: %s: Get EEProm parameters \n", ha->host_no,
		      __func__));
	if (ql4xxx_lock_flash(ha) != QLA_SUCCESS)
		return (QLA_ERROR);
	if (ql4xxx_lock_nvram(ha) != QLA_SUCCESS) {
		ql4xxx_unlock_flash(ha);
		return (QLA_ERROR);
	}

	
	dev_info(&ha->pdev->dev, "Configuring NVRAM ...\n");
	if (qla4xxx_is_nvram_configuration_valid(ha) == QLA_SUCCESS) {
		spin_lock_irqsave(&ha->hardware_lock, flags);
		extHwConfig.Asuint32_t =
			rd_nvram_word(ha, eeprom_ext_hw_conf_offset(ha));
		spin_unlock_irqrestore(&ha->hardware_lock, flags);
	} else {
		
		dev_warn(&ha->pdev->dev,
			   "scsi%ld: %s: EEProm checksum invalid.  "
			   "Please update your EEPROM\n", ha->host_no,
			   __func__);

		
		if (is_qla4010(ha))
			extHwConfig.Asuint32_t = 0x1912;
		else if (is_qla4022(ha) | is_qla4032(ha))
			extHwConfig.Asuint32_t = 0x0023;
	}
	DEBUG(printk("scsi%ld: %s: Setting extHwConfig to 0xFFFF%04x\n",
		     ha->host_no, __func__, extHwConfig.Asuint32_t));

	spin_lock_irqsave(&ha->hardware_lock, flags);
	writel((0xFFFF << 16) | extHwConfig.Asuint32_t, isp_ext_hw_conf(ha));
	readl(isp_ext_hw_conf(ha));
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	ql4xxx_unlock_nvram(ha);
	ql4xxx_unlock_flash(ha);

	return (QLA_SUCCESS);
}

static void qla4x00_pci_config(struct scsi_qla_host *ha)
{
	uint16_t w;
	int status;

	dev_info(&ha->pdev->dev, "Configuring PCI space...\n");

	pci_set_master(ha->pdev);
	status = pci_set_mwi(ha->pdev);
	
	pci_read_config_word(ha->pdev, PCI_COMMAND, &w);
	w |= PCI_COMMAND_PARITY | PCI_COMMAND_SERR;
	w &= ~PCI_COMMAND_INTX_DISABLE;
	pci_write_config_word(ha->pdev, PCI_COMMAND, w);
}

static int qla4xxx_start_firmware_from_flash(struct scsi_qla_host *ha)
{
	int status = QLA_ERROR;
	uint32_t max_wait_time;
	unsigned long flags;
	uint32_t mbox_status;

	dev_info(&ha->pdev->dev, "Starting firmware ...\n");

	
	DEBUG(printk("scsi%d: %s: Start firmware from flash ROM\n",
		     ha->host_no, __func__));

	spin_lock_irqsave(&ha->hardware_lock, flags);
	writel(jiffies, &ha->reg->mailbox[7]);
	if (is_qla4022(ha) | is_qla4032(ha))
		writel(set_rmask(NVR_WRITE_ENABLE),
		       &ha->reg->u1.isp4022.nvram);

        writel(2, &ha->reg->mailbox[6]);
        readl(&ha->reg->mailbox[6]);

	writel(set_rmask(CSR_BOOT_ENABLE), &ha->reg->ctrl_status);
	readl(&ha->reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	
	max_wait_time = FIRMWARE_UP_TOV * 4;
	do {
		uint32_t ctrl_status;

		spin_lock_irqsave(&ha->hardware_lock, flags);
		ctrl_status = readw(&ha->reg->ctrl_status);
		mbox_status = readw(&ha->reg->mailbox[0]);
		spin_unlock_irqrestore(&ha->hardware_lock, flags);

		if (ctrl_status & set_rmask(CSR_SCSI_PROCESSOR_INTR))
			break;
		if (mbox_status == MBOX_STS_COMMAND_COMPLETE)
			break;

		DEBUG2(printk("scsi%ld: %s: Waiting for boot firmware to "
			      "complete... ctrl_sts=0x%x, remaining=%d\n",
			      ha->host_no, __func__, ctrl_status,
			      max_wait_time));

		msleep(250);
	} while ((max_wait_time--));

	if (mbox_status == MBOX_STS_COMMAND_COMPLETE) {
		DEBUG(printk("scsi%ld: %s: Firmware has started\n",
			     ha->host_no, __func__));

		spin_lock_irqsave(&ha->hardware_lock, flags);
		writel(set_rmask(CSR_SCSI_PROCESSOR_INTR),
		       &ha->reg->ctrl_status);
		readl(&ha->reg->ctrl_status);
		spin_unlock_irqrestore(&ha->hardware_lock, flags);

		status = QLA_SUCCESS;
	} else {
		printk(KERN_INFO "scsi%ld: %s: Boot firmware failed "
		       "-  mbox status 0x%x\n", ha->host_no, __func__,
		       mbox_status);
		status = QLA_ERROR;
	}
	return status;
}

int ql4xxx_lock_drvr_wait(struct scsi_qla_host *a)
{
#define QL4_LOCK_DRVR_WAIT	60
#define QL4_LOCK_DRVR_SLEEP	1

	int drvr_wait = QL4_LOCK_DRVR_WAIT;
	while (drvr_wait) {
		if (ql4xxx_lock_drvr(a) == 0) {
			ssleep(QL4_LOCK_DRVR_SLEEP);
			if (drvr_wait) {
				DEBUG2(printk("scsi%ld: %s: Waiting for "
					      "Global Init Semaphore(%d)...\n",
					      a->host_no,
					      __func__, drvr_wait));
			}
			drvr_wait -= QL4_LOCK_DRVR_SLEEP;
		} else {
			DEBUG2(printk("scsi%ld: %s: Global Init Semaphore "
				      "acquired\n", a->host_no, __func__));
			return QLA_SUCCESS;
		}
	}
	return QLA_ERROR;
}


static int qla4xxx_start_firmware(struct scsi_qla_host *ha)
{
	unsigned long flags = 0;
	uint32_t mbox_status;
	int status = QLA_ERROR;
	int soft_reset = 1;
	int config_chip = 0;

	if (is_qla4022(ha) | is_qla4032(ha))
		ql4xxx_set_mac_number(ha);

	if (ql4xxx_lock_drvr_wait(ha) != QLA_SUCCESS)
		return QLA_ERROR;

	spin_lock_irqsave(&ha->hardware_lock, flags);

	DEBUG2(printk("scsi%ld: %s: port_ctrl	= 0x%08X\n", ha->host_no,
		      __func__, readw(isp_port_ctrl(ha))));
	DEBUG(printk("scsi%ld: %s: port_status = 0x%08X\n", ha->host_no,
		     __func__, readw(isp_port_status(ha))));

	
	if ((readw(isp_port_ctrl(ha)) & 0x8000) != 0) {
		DEBUG(printk("scsi%ld: %s: Hardware has already been "
			     "initialized\n", ha->host_no, __func__));

		
		mbox_status = readw(&ha->reg->mailbox[0]);

		DEBUG2(printk("scsi%ld: %s: H/W Config complete - mbox[0]= "
			      "0x%x\n", ha->host_no, __func__, mbox_status));

		
		if (mbox_status == 0) {
			
			config_chip = 1;
			soft_reset = 0;
		} else {
			writel(set_rmask(CSR_SCSI_PROCESSOR_INTR),
			       &ha->reg->ctrl_status);
			readl(&ha->reg->ctrl_status);
			spin_unlock_irqrestore(&ha->hardware_lock, flags);
			if (qla4xxx_get_firmware_state(ha) == QLA_SUCCESS) {
				DEBUG2(printk("scsi%ld: %s: Get firmware "
					      "state -- state = 0x%x\n",
					      ha->host_no,
					      __func__, ha->firmware_state));
				
				if (ha->firmware_state &
				    FW_STATE_CONFIG_WAIT) {
					DEBUG2(printk("scsi%ld: %s: Firmware "
						      "in known state -- "
						      "config and "
						      "boot, state = 0x%x\n",
						      ha->host_no, __func__,
						      ha->firmware_state));
					config_chip = 1;
					soft_reset = 0;
				}
			} else {
				DEBUG2(printk("scsi%ld: %s: Firmware in "
					      "unknown state -- resetting,"
					      " state = "
					      "0x%x\n", ha->host_no, __func__,
					      ha->firmware_state));
			}
			spin_lock_irqsave(&ha->hardware_lock, flags);
		}
	} else {
		DEBUG(printk("scsi%ld: %s: H/W initialization hasn't been "
			     "started - resetting\n", ha->host_no, __func__));
	}
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	DEBUG(printk("scsi%ld: %s: Flags soft_rest=%d, config= %d\n ",
		     ha->host_no, __func__, soft_reset, config_chip));
	if (soft_reset) {
		DEBUG(printk("scsi%ld: %s: Issue Soft Reset\n", ha->host_no,
			     __func__));
		status = qla4xxx_soft_reset(ha);
		if (status == QLA_ERROR) {
			DEBUG(printk("scsi%d: %s: Soft Reset failed!\n",
				     ha->host_no, __func__));
			ql4xxx_unlock_drvr(ha);
			return QLA_ERROR;
		}
		config_chip = 1;

		
		if (ql4xxx_lock_drvr_wait(ha) != QLA_SUCCESS)
			return QLA_ERROR;
	}

	if (config_chip) {
		if ((status = qla4xxx_config_nvram(ha)) == QLA_SUCCESS)
			status = qla4xxx_start_firmware_from_flash(ha);
	}

	ql4xxx_unlock_drvr(ha);
	if (status == QLA_SUCCESS) {
		qla4xxx_get_fw_version(ha);
		if (test_and_clear_bit(AF_GET_CRASH_RECORD, &ha->flags))
			qla4xxx_get_crash_record(ha);
	} else {
		DEBUG(printk("scsi%ld: %s: Firmware has NOT started\n",
			     ha->host_no, __func__));
	}
	return status;
}



int qla4xxx_initialize_adapter(struct scsi_qla_host *ha,
			       uint8_t renew_ddb_list)
{
	int status = QLA_ERROR;
	int8_t ip_address[IP_ADDR_LEN] = {0} ;

	ha->eeprom_cmd_data = 0;

	qla4x00_pci_config(ha);

	qla4xxx_disable_intrs(ha);

	
	if (qla4xxx_start_firmware(ha) == QLA_ERROR)
		goto exit_init_hba;

	if (qla4xxx_validate_mac_address(ha) == QLA_ERROR)
		goto exit_init_hba;

	if (qla4xxx_init_local_data(ha) == QLA_ERROR)
		goto exit_init_hba;

	status = qla4xxx_init_firmware(ha);
	if (status == QLA_ERROR)
		goto exit_init_hba;

	
	if (ha->firmware_state & FW_STATE_DHCP_IN_PROGRESS)
		goto exit_init_online;

	
	if (memcmp(ha->ip_address, ip_address, IP_ADDR_LEN) == 0 ||
	    memcmp(ha->subnet_mask, ip_address, IP_ADDR_LEN) == 0)
		goto exit_init_online;

	if (renew_ddb_list == PRESERVE_DDB_LIST) {
		
		qla4xxx_reinitialize_ddb_list(ha);
	} else if (renew_ddb_list == REBUILD_DDB_LIST) {
		
		status = qla4xxx_initialize_ddb_list(ha);
		if (status == QLA_ERROR) {
			DEBUG2(printk("%s(%ld) Error occurred during build"
				      "ddb list\n", __func__, ha->host_no));
			goto exit_init_hba;
		}

	}
	if (!ha->tot_ddbs) {
		DEBUG2(printk("scsi%ld: Failed to initialize devices or none "
			      "present in Firmware device database\n",
			      ha->host_no));
	}

exit_init_online:
	set_bit(AF_ONLINE, &ha->flags);
exit_init_hba:
	return status;
}


static void qla4xxx_add_device_dynamically(struct scsi_qla_host *ha,
					   uint32_t fw_ddb_index)
{
	struct ddb_entry * ddb_entry;
	uint32_t new_tgt;

	
	ddb_entry = qla4xxx_get_ddb_entry(ha, fw_ddb_index, &new_tgt);
	if (ddb_entry == NULL) {
		DEBUG2(printk(KERN_WARNING
			      "scsi%ld: Unable to allocate memory to add "
			      "fw_ddb_index %d\n", ha->host_no, fw_ddb_index));
		return;
	}

	if (!new_tgt && (ddb_entry->fw_ddb_index != fw_ddb_index)) {
		
		qla4xxx_free_ddb(ha, ddb_entry);
		ddb_entry = qla4xxx_alloc_ddb(ha, fw_ddb_index);
		if (ddb_entry == NULL) {
			DEBUG2(printk(KERN_WARNING
				"scsi%ld: Unable to allocate memory"
				" to add fw_ddb_index %d\n",
				ha->host_no, fw_ddb_index));
			return;
		}
	}
	if (qla4xxx_update_ddb_entry(ha, ddb_entry, fw_ddb_index) ==
				    QLA_ERROR) {
		ha->fw_ddb_index_map[fw_ddb_index] =
					(struct ddb_entry *)INVALID_ENTRY;
		DEBUG2(printk(KERN_WARNING
			      "scsi%ld: failed to add new device at index "
			      "[%d]\n Unable to retrieve fw ddb entry\n",
			      ha->host_no, fw_ddb_index));
		qla4xxx_free_ddb(ha, ddb_entry);
		return;
	}

	if (qla4xxx_add_sess(ddb_entry)) {
		DEBUG2(printk(KERN_WARNING
			      "scsi%ld: failed to add new device at index "
			      "[%d]\n Unable to add connection and session\n",
			      ha->host_no, fw_ddb_index));
		qla4xxx_free_ddb(ha, ddb_entry);
	}
}


int qla4xxx_process_ddb_changed(struct scsi_qla_host *ha,
				uint32_t fw_ddb_index, uint32_t state)
{
	struct ddb_entry * ddb_entry;
	uint32_t old_fw_ddb_device_state;

	
	if (fw_ddb_index >= MAX_DDB_ENTRIES)
		return QLA_ERROR;

	
	ddb_entry = qla4xxx_lookup_ddb_by_fw_index(ha, fw_ddb_index);
	
	if (ddb_entry == NULL) {
		if (state == DDB_DS_SESSION_ACTIVE)
			qla4xxx_add_device_dynamically(ha, fw_ddb_index);
		return QLA_SUCCESS;
	}

	
	old_fw_ddb_device_state = ddb_entry->fw_ddb_device_state;
	DEBUG2(printk("scsi%ld: %s DDB - old state= 0x%x, new state=0x%x for "
		      "index [%d]\n", ha->host_no, __func__,
		      ddb_entry->fw_ddb_device_state, state, fw_ddb_index));
	if (old_fw_ddb_device_state == state &&
	    state == DDB_DS_SESSION_ACTIVE) {
		
		return QLA_SUCCESS;
	}

	ddb_entry->fw_ddb_device_state = state;
	
	if (ddb_entry->fw_ddb_device_state == DDB_DS_SESSION_ACTIVE) {
		atomic_set(&ddb_entry->state, DDB_STATE_ONLINE);
		atomic_set(&ddb_entry->port_down_timer,
			   ha->port_down_retry_count);
		atomic_set(&ddb_entry->relogin_retry_count, 0);
		atomic_set(&ddb_entry->relogin_timer, 0);
		clear_bit(DF_RELOGIN, &ddb_entry->flags);
		clear_bit(DF_NO_RELOGIN, &ddb_entry->flags);
		iscsi_unblock_session(ddb_entry->sess);
		iscsi_session_event(ddb_entry->sess,
				    ISCSI_KEVENT_CREATE_SESSION);
		
	} else {
		
		
		if (atomic_read(&ddb_entry->state) == DDB_STATE_ONLINE)
			qla4xxx_mark_device_missing(ha, ddb_entry);
		
		if (ddb_entry->fw_ddb_device_state == DDB_DS_SESSION_FAILED &&
		    !test_bit(DF_RELOGIN, &ddb_entry->flags) &&
		    !test_bit(DF_NO_RELOGIN, &ddb_entry->flags) &&
		    !test_bit(DF_ISNS_DISCOVERED, &ddb_entry->flags)) {
			
			
			atomic_set(&ddb_entry->relogin_timer, 0);
			atomic_set(&ddb_entry->retry_relogin_timer,
				   ddb_entry->default_time2wait + 4);
		}
	}

	return QLA_SUCCESS;
}

