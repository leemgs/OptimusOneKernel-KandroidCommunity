

#include "ql4_def.h"
#include "ql4_glbl.h"
#include "ql4_dbg.h"
#include "ql4_inline.h"



static int qla4xxx_mailbox_command(struct scsi_qla_host *ha, uint8_t inCount,
				   uint8_t outCount, uint32_t *mbx_cmd,
				   uint32_t *mbx_sts)
{
	int status = QLA_ERROR;
	uint8_t i;
	u_long wait_count;
	uint32_t intr_status;
	unsigned long flags = 0;

	
	if (!mbx_cmd || !mbx_sts) {
		DEBUG2(printk("scsi%ld: %s: Invalid mbx_cmd or mbx_sts "
			      "pointer\n", ha->host_no, __func__));
		return status;
	}
	
	wait_count = MBOX_TOV * 100;

	while (wait_count--) {
		mutex_lock(&ha->mbox_sem);
		if (!test_bit(AF_MBOX_COMMAND, &ha->flags)) {
			set_bit(AF_MBOX_COMMAND, &ha->flags);
			mutex_unlock(&ha->mbox_sem);
			break;
		}
		mutex_unlock(&ha->mbox_sem);
		if (!wait_count) {
			DEBUG2(printk("scsi%ld: %s: mbox_sem failed\n",
				ha->host_no, __func__));
			return status;
		}
		msleep(10);
	}

	
	spin_lock_irqsave(&ha->hardware_lock, flags);
	intr_status = readl(&ha->reg->ctrl_status);
	if (intr_status & CSR_SCSI_PROCESSOR_INTR) {
		
		qla4xxx_interrupt_service_routine(ha, intr_status);
		clear_bit(AF_MBOX_COMMAND_DONE, &ha->flags);
	}

	
	ha->mbox_status_count = outCount;
	for (i = 0; i < outCount; i++)
		ha->mbox_status[i] = 0;

	
	for (i = 1; i < inCount; i++)
		writel(mbx_cmd[i], &ha->reg->mailbox[i]);

	
	writel(mbx_cmd[0], &ha->reg->mailbox[0]);
	readl(&ha->reg->mailbox[0]);
	writel(set_rmask(CSR_INTR_RISC), &ha->reg->ctrl_status);
	readl(&ha->reg->ctrl_status);
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	

	
	if (outCount == 0) {
		status = QLA_SUCCESS;
		goto mbox_exit;
	}
	
	wait_count = jiffies + MBOX_TOV * HZ;
	while (test_bit(AF_MBOX_COMMAND_DONE, &ha->flags) == 0) {
		if (time_after_eq(jiffies, wait_count))
			break;

		spin_lock_irqsave(&ha->hardware_lock, flags);
		intr_status = readl(&ha->reg->ctrl_status);
		if (intr_status & INTR_PENDING) {
			
			ha->mbox_status_count = outCount;
			qla4xxx_interrupt_service_routine(ha, intr_status);
		}
		spin_unlock_irqrestore(&ha->hardware_lock, flags);
		msleep(10);
	}

	
	if (!test_bit(AF_MBOX_COMMAND_DONE, &ha->flags)) {
		DEBUG2(printk("scsi%ld: Mailbox Cmd 0x%08X timed out ...,"
			      " Scheduling Adapter Reset\n", ha->host_no,
			      mbx_cmd[0]));
		ha->mailbox_timeout_count++;
		mbx_sts[0] = (-1);
		set_bit(DPC_RESET_HA, &ha->dpc_flags);
		goto mbox_exit;
	}

	
	spin_lock_irqsave(&ha->hardware_lock, flags);
	for (i = 0; i < outCount; i++)
		mbx_sts[i] = ha->mbox_status[i];

	
	switch (ha->mbox_status[0]) {
	case MBOX_STS_COMMAND_COMPLETE:
		status = QLA_SUCCESS;
		break;

	case MBOX_STS_INTERMEDIATE_COMPLETION:
		status = QLA_SUCCESS;
		break;

	case MBOX_STS_BUSY:
		DEBUG2( printk("scsi%ld: %s: Cmd = %08X, ISP BUSY\n",
			       ha->host_no, __func__, mbx_cmd[0]));
		ha->mailbox_timeout_count++;
		break;

	default:
		DEBUG2(printk("scsi%ld: %s: **** FAILED, cmd = %08X, "
			      "sts = %08X ****\n", ha->host_no, __func__,
			      mbx_cmd[0], mbx_sts[0]));
		break;
	}
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

mbox_exit:
	mutex_lock(&ha->mbox_sem);
	clear_bit(AF_MBOX_COMMAND, &ha->flags);
	mutex_unlock(&ha->mbox_sem);
	clear_bit(AF_MBOX_COMMAND_DONE, &ha->flags);

	return status;
}


int qla4xxx_initialize_fw_cb(struct scsi_qla_host * ha)
{
	struct init_fw_ctrl_blk *init_fw_cb;
	dma_addr_t init_fw_cb_dma;
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];
	int status = QLA_ERROR;

	init_fw_cb = dma_alloc_coherent(&ha->pdev->dev,
					sizeof(struct init_fw_ctrl_blk),
					&init_fw_cb_dma, GFP_KERNEL);
	if (init_fw_cb == NULL) {
		DEBUG2(printk("scsi%ld: %s: Unable to alloc init_cb\n",
			      ha->host_no, __func__));
		return 10;
	}
	memset(init_fw_cb, 0, sizeof(struct init_fw_ctrl_blk));

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_GET_INIT_FW_CTRL_BLOCK;
	mbox_cmd[2] = LSDW(init_fw_cb_dma);
	mbox_cmd[3] = MSDW(init_fw_cb_dma);
	mbox_cmd[4] = sizeof(struct init_fw_ctrl_blk);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		dma_free_coherent(&ha->pdev->dev,
				  sizeof(struct init_fw_ctrl_blk),
				  init_fw_cb, init_fw_cb_dma);
		return status;
	}

	
	qla4xxx_init_rings(ha);

	
	init_fw_cb->pri.rqq_consumer_idx = cpu_to_le16(ha->request_out);
	init_fw_cb->pri.compq_producer_idx = cpu_to_le16(ha->response_in);
	init_fw_cb->pri.rqq_len = __constant_cpu_to_le16(REQUEST_QUEUE_DEPTH);
	init_fw_cb->pri.compq_len = __constant_cpu_to_le16(RESPONSE_QUEUE_DEPTH);
	init_fw_cb->pri.rqq_addr_lo = cpu_to_le32(LSDW(ha->request_dma));
	init_fw_cb->pri.rqq_addr_hi = cpu_to_le32(MSDW(ha->request_dma));
	init_fw_cb->pri.compq_addr_lo = cpu_to_le32(LSDW(ha->response_dma));
	init_fw_cb->pri.compq_addr_hi = cpu_to_le32(MSDW(ha->response_dma));
	init_fw_cb->pri.shdwreg_addr_lo =
		cpu_to_le32(LSDW(ha->shadow_regs_dma));
	init_fw_cb->pri.shdwreg_addr_hi =
		cpu_to_le32(MSDW(ha->shadow_regs_dma));

	
	init_fw_cb->pri.fw_options |=
		__constant_cpu_to_le16(FWOPT_SESSION_MODE |
				       FWOPT_INITIATOR_MODE);
	init_fw_cb->pri.fw_options &= __constant_cpu_to_le16(~FWOPT_TARGET_MODE);

	
	ha->firmware_options = le16_to_cpu(init_fw_cb->pri.fw_options);
	ha->tcp_options = le16_to_cpu(init_fw_cb->pri.ipv4_tcp_opts);
	ha->heartbeat_interval = init_fw_cb->pri.hb_interval;
	memcpy(ha->ip_address, init_fw_cb->pri.ipv4_addr,
	       min(sizeof(ha->ip_address), sizeof(init_fw_cb->pri.ipv4_addr)));
	memcpy(ha->subnet_mask, init_fw_cb->pri.ipv4_subnet,
	       min(sizeof(ha->subnet_mask), sizeof(init_fw_cb->pri.ipv4_subnet)));
	memcpy(ha->gateway, init_fw_cb->pri.ipv4_gw_addr,
	       min(sizeof(ha->gateway), sizeof(init_fw_cb->pri.ipv4_gw_addr)));
	memcpy(ha->name_string, init_fw_cb->pri.iscsi_name,
	       min(sizeof(ha->name_string),
		   sizeof(init_fw_cb->pri.iscsi_name)));
	

	
	ha->port_down_retry_count = le16_to_cpu(init_fw_cb->pri.conn_ka_timeout);
	ha->discovery_wait = ql4xdiscoverywait;

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_INITIALIZE_FIRMWARE;
	mbox_cmd[1] = 0;
	mbox_cmd[2] = LSDW(init_fw_cb_dma);
	mbox_cmd[3] = MSDW(init_fw_cb_dma);
	mbox_cmd[4] = sizeof(struct init_fw_ctrl_blk);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]) ==
	    QLA_SUCCESS)
		status = QLA_SUCCESS;
	 else {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_INITIALIZE_FIRMWARE "
			      "failed w/ status %04X\n", ha->host_no, __func__,
			      mbox_sts[0]));
	}
	dma_free_coherent(&ha->pdev->dev, sizeof(struct init_fw_ctrl_blk),
			  init_fw_cb, init_fw_cb_dma);

	return status;
}


int qla4xxx_get_dhcp_ip_address(struct scsi_qla_host * ha)
{
	struct init_fw_ctrl_blk *init_fw_cb;
	dma_addr_t init_fw_cb_dma;
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	init_fw_cb = dma_alloc_coherent(&ha->pdev->dev,
					sizeof(struct init_fw_ctrl_blk),
					&init_fw_cb_dma, GFP_KERNEL);
	if (init_fw_cb == NULL) {
		printk("scsi%ld: %s: Unable to alloc init_cb\n", ha->host_no,
		       __func__);
		return 10;
	}

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	memset(init_fw_cb, 0, sizeof(struct init_fw_ctrl_blk));
	mbox_cmd[0] = MBOX_CMD_GET_INIT_FW_CTRL_BLOCK;
	mbox_cmd[2] = LSDW(init_fw_cb_dma);
	mbox_cmd[3] = MSDW(init_fw_cb_dma);
	mbox_cmd[4] = sizeof(struct init_fw_ctrl_blk);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: Failed to get init_fw_ctrl_blk\n",
			      ha->host_no, __func__));
		dma_free_coherent(&ha->pdev->dev,
				  sizeof(struct init_fw_ctrl_blk),
				  init_fw_cb, init_fw_cb_dma);
		return QLA_ERROR;
	}

	
	memcpy(ha->ip_address, init_fw_cb->pri.ipv4_addr,
	       min(sizeof(ha->ip_address), sizeof(init_fw_cb->pri.ipv4_addr)));
	memcpy(ha->subnet_mask, init_fw_cb->pri.ipv4_subnet,
	       min(sizeof(ha->subnet_mask), sizeof(init_fw_cb->pri.ipv4_subnet)));
	memcpy(ha->gateway, init_fw_cb->pri.ipv4_gw_addr,
	       min(sizeof(ha->gateway), sizeof(init_fw_cb->pri.ipv4_gw_addr)));

	dma_free_coherent(&ha->pdev->dev, sizeof(struct init_fw_ctrl_blk),
			  init_fw_cb, init_fw_cb_dma);

	return QLA_SUCCESS;
}


int qla4xxx_get_firmware_state(struct scsi_qla_host * ha)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_GET_FW_STATE;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 4, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_GET_FW_STATE failed w/ "
			      "status %04X\n", ha->host_no, __func__,
			      mbox_sts[0]));
		return QLA_ERROR;
	}
	ha->firmware_state = mbox_sts[1];
	ha->board_id = mbox_sts[2];
	ha->addl_fw_state = mbox_sts[3];
	DEBUG2(printk("scsi%ld: %s firmware_state=0x%x\n",
		      ha->host_no, __func__, ha->firmware_state);)

		return QLA_SUCCESS;
}


int qla4xxx_get_firmware_status(struct scsi_qla_host * ha)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_GET_FW_STATUS;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 3, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_GET_FW_STATUS failed w/ "
			      "status %04X\n", ha->host_no, __func__,
			      mbox_sts[0]));
		return QLA_ERROR;
	}
	return QLA_SUCCESS;
}


int qla4xxx_get_fwddb_entry(struct scsi_qla_host *ha,
			    uint16_t fw_ddb_index,
			    struct dev_db_entry *fw_ddb_entry,
			    dma_addr_t fw_ddb_entry_dma,
			    uint32_t *num_valid_ddb_entries,
			    uint32_t *next_ddb_index,
			    uint32_t *fw_ddb_device_state,
			    uint32_t *conn_err_detail,
			    uint16_t *tcp_source_port_num,
			    uint16_t *connection_id)
{
	int status = QLA_ERROR;
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	
	if (fw_ddb_index >= MAX_DDB_ENTRIES) {
		DEBUG2(printk("scsi%ld: %s: index [%d] out of range.\n",
			      ha->host_no, __func__, fw_ddb_index));
		goto exit_get_fwddb;
	}
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_GET_DATABASE_ENTRY;
	mbox_cmd[1] = (uint32_t) fw_ddb_index;
	mbox_cmd[2] = LSDW(fw_ddb_entry_dma);
	mbox_cmd[3] = MSDW(fw_ddb_entry_dma);
	mbox_cmd[4] = sizeof(struct dev_db_entry);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 7, &mbox_cmd[0], &mbox_sts[0]) ==
	    QLA_ERROR) {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_GET_DATABASE_ENTRY failed"
			      " with status 0x%04X\n", ha->host_no, __func__,
			      mbox_sts[0]));
		goto exit_get_fwddb;
	}
	if (fw_ddb_index != mbox_sts[1]) {
		DEBUG2(printk("scsi%ld: %s: index mismatch [%d] != [%d].\n",
			      ha->host_no, __func__, fw_ddb_index,
			      mbox_sts[1]));
		goto exit_get_fwddb;
	}
	if (fw_ddb_entry) {
		dev_info(&ha->pdev->dev, "DDB[%d] MB0 %04x Tot %d Next %d "
			   "State %04x ConnErr %08x %d.%d.%d.%d:%04d \"%s\"\n",
			   fw_ddb_index, mbox_sts[0], mbox_sts[2], mbox_sts[3],
			   mbox_sts[4], mbox_sts[5], fw_ddb_entry->ip_addr[0],
			   fw_ddb_entry->ip_addr[1], fw_ddb_entry->ip_addr[2],
			   fw_ddb_entry->ip_addr[3],
			   le16_to_cpu(fw_ddb_entry->port),
			   fw_ddb_entry->iscsi_name);
	}
	if (num_valid_ddb_entries)
		*num_valid_ddb_entries = mbox_sts[2];
	if (next_ddb_index)
		*next_ddb_index = mbox_sts[3];
	if (fw_ddb_device_state)
		*fw_ddb_device_state = mbox_sts[4];

	
	if (conn_err_detail)
		*conn_err_detail = mbox_sts[5];
	if (tcp_source_port_num)
		*tcp_source_port_num = (uint16_t) mbox_sts[6] >> 16;
	if (connection_id)
		*connection_id = (uint16_t) mbox_sts[6] & 0x00FF;
	status = QLA_SUCCESS;

exit_get_fwddb:
	return status;
}


int qla4xxx_set_ddb_entry(struct scsi_qla_host * ha, uint16_t fw_ddb_index,
			  dma_addr_t fw_ddb_entry_dma)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_SET_DATABASE_ENTRY;
	mbox_cmd[1] = (uint32_t) fw_ddb_index;
	mbox_cmd[2] = LSDW(fw_ddb_entry_dma);
	mbox_cmd[3] = MSDW(fw_ddb_entry_dma);
	mbox_cmd[4] = sizeof(struct dev_db_entry);

	return qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]);
}


void qla4xxx_get_crash_record(struct scsi_qla_host * ha)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];
	struct crash_record *crash_record = NULL;
	dma_addr_t crash_record_dma = 0;
	uint32_t crash_record_size = 0;

	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_cmd));

	
	mbox_cmd[0] = MBOX_CMD_GET_CRASH_RECORD;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 5, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: ERROR: Unable to retrieve size!\n",
			      ha->host_no, __func__));
		goto exit_get_crash_record;
	}
	crash_record_size = mbox_sts[4];
	if (crash_record_size == 0) {
		DEBUG2(printk("scsi%ld: %s: ERROR: Crash record size is 0!\n",
			      ha->host_no, __func__));
		goto exit_get_crash_record;
	}

	
	crash_record = dma_alloc_coherent(&ha->pdev->dev, crash_record_size,
					  &crash_record_dma, GFP_KERNEL);
	if (crash_record == NULL)
		goto exit_get_crash_record;

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_cmd));

	mbox_cmd[0] = MBOX_CMD_GET_CRASH_RECORD;
	mbox_cmd[2] = LSDW(crash_record_dma);
	mbox_cmd[3] = MSDW(crash_record_dma);
	mbox_cmd[4] = crash_record_size;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 5, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS)
		goto exit_get_crash_record;

	

exit_get_crash_record:
	if (crash_record)
		dma_free_coherent(&ha->pdev->dev, crash_record_size,
				  crash_record, crash_record_dma);
}


void qla4xxx_get_conn_event_log(struct scsi_qla_host * ha)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];
	struct conn_event_log_entry *event_log = NULL;
	dma_addr_t event_log_dma = 0;
	uint32_t event_log_size = 0;
	uint32_t num_valid_entries;
	uint32_t      oldest_entry = 0;
	uint32_t	max_event_log_entries;
	uint8_t		i;


	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_cmd));

	
	mbox_cmd[0] = MBOX_CMD_GET_CONN_EVENT_LOG;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 5, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS)
		goto exit_get_event_log;

	event_log_size = mbox_sts[4];
	if (event_log_size == 0)
		goto exit_get_event_log;

	
	event_log = dma_alloc_coherent(&ha->pdev->dev, event_log_size,
				       &event_log_dma, GFP_KERNEL);
	if (event_log == NULL)
		goto exit_get_event_log;

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_cmd));

	mbox_cmd[0] = MBOX_CMD_GET_CONN_EVENT_LOG;
	mbox_cmd[2] = LSDW(event_log_dma);
	mbox_cmd[3] = MSDW(event_log_dma);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 5, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: ERROR: Unable to retrieve event "
			      "log!\n", ha->host_no, __func__));
		goto exit_get_event_log;
	}

	
	num_valid_entries = mbox_sts[1];

	max_event_log_entries = event_log_size /
		sizeof(struct conn_event_log_entry);

	if (num_valid_entries > max_event_log_entries)
		oldest_entry = num_valid_entries % max_event_log_entries;

	DEBUG3(printk("scsi%ld: Connection Event Log Dump (%d entries):\n",
		      ha->host_no, num_valid_entries));

	if (ql4xextended_error_logging == 3) {
		if (oldest_entry == 0) {
			
			for (i=0; i < num_valid_entries; i++) {
				qla4xxx_dump_buffer((uint8_t *)event_log+
						    (i*sizeof(*event_log)),
						    sizeof(*event_log));
			}
		}
		else {
			
			for (i=oldest_entry; i < max_event_log_entries; i++) {
				qla4xxx_dump_buffer((uint8_t *)event_log+
						    (i*sizeof(*event_log)),
						    sizeof(*event_log));
			}
			for (i=0; i < oldest_entry; i++) {
				qla4xxx_dump_buffer((uint8_t *)event_log+
						    (i*sizeof(*event_log)),
						    sizeof(*event_log));
			}
		}
	}

exit_get_event_log:
	if (event_log)
		dma_free_coherent(&ha->pdev->dev, event_log_size, event_log,
				  event_log_dma);
}


int qla4xxx_reset_lun(struct scsi_qla_host * ha, struct ddb_entry * ddb_entry,
		      int lun)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];
	int status = QLA_SUCCESS;

	DEBUG2(printk("scsi%ld:%d:%d: lun reset issued\n", ha->host_no,
		      ddb_entry->os_target_id, lun));

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_LUN_RESET;
	mbox_cmd[1] = ddb_entry->fw_ddb_index;
	mbox_cmd[2] = lun << 8;
	mbox_cmd[5] = 0x01;	

	qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]);
	if (mbox_sts[0] != MBOX_STS_COMMAND_COMPLETE &&
	    mbox_sts[0] != MBOX_STS_COMMAND_ERROR)
		status = QLA_ERROR;

	return status;
}


int qla4xxx_reset_target(struct scsi_qla_host *ha,
			 struct ddb_entry *ddb_entry)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];
	int status = QLA_SUCCESS;

	DEBUG2(printk("scsi%ld:%d: target reset issued\n", ha->host_no,
		      ddb_entry->os_target_id));

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_TARGET_WARM_RESET;
	mbox_cmd[1] = ddb_entry->fw_ddb_index;
	mbox_cmd[5] = 0x01;	

	qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0],
				&mbox_sts[0]);
	if (mbox_sts[0] != MBOX_STS_COMMAND_COMPLETE &&
	    mbox_sts[0] != MBOX_STS_COMMAND_ERROR)
		status = QLA_ERROR;

	return status;
}

int qla4xxx_get_flash(struct scsi_qla_host * ha, dma_addr_t dma_addr,
		      uint32_t offset, uint32_t len)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_READ_FLASH;
	mbox_cmd[1] = LSDW(dma_addr);
	mbox_cmd[2] = MSDW(dma_addr);
	mbox_cmd[3] = offset;
	mbox_cmd[4] = len;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 2, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_READ_FLASH, failed w/ "
		    "status %04X %04X, offset %08x, len %08x\n", ha->host_no,
		    __func__, mbox_sts[0], mbox_sts[1], offset, len));
		return QLA_ERROR;
	}
	return QLA_SUCCESS;
}


int qla4xxx_get_fw_version(struct scsi_qla_host * ha)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	
	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_ABOUT_FW;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 5, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: MBOX_CMD_ABOUT_FW failed w/ "
		    "status %04X\n", ha->host_no, __func__, mbox_sts[0]));
		return QLA_ERROR;
	}

	
	ha->firmware_version[0] = mbox_sts[1];
	ha->firmware_version[1] = mbox_sts[2];
	ha->patch_number = mbox_sts[3];
	ha->build_number = mbox_sts[4];

	return QLA_SUCCESS;
}

static int qla4xxx_get_default_ddb(struct scsi_qla_host *ha,
				   dma_addr_t dma_addr)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_GET_DATABASE_ENTRY_DEFAULTS;
	mbox_cmd[2] = LSDW(dma_addr);
	mbox_cmd[3] = MSDW(dma_addr);

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 1, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		DEBUG2(printk("scsi%ld: %s: failed status %04X\n",
		     ha->host_no, __func__, mbox_sts[0]));
		return QLA_ERROR;
	}
	return QLA_SUCCESS;
}

static int qla4xxx_req_ddb_entry(struct scsi_qla_host *ha, uint32_t *ddb_index)
{
	uint32_t mbox_cmd[MBOX_REG_COUNT];
	uint32_t mbox_sts[MBOX_REG_COUNT];

	memset(&mbox_cmd, 0, sizeof(mbox_cmd));
	memset(&mbox_sts, 0, sizeof(mbox_sts));

	mbox_cmd[0] = MBOX_CMD_REQUEST_DATABASE_ENTRY;
	mbox_cmd[1] = MAX_PRST_DEV_DB_ENTRIES;

	if (qla4xxx_mailbox_command(ha, MBOX_REG_COUNT, 3, &mbox_cmd[0], &mbox_sts[0]) !=
	    QLA_SUCCESS) {
		if (mbox_sts[0] == MBOX_STS_COMMAND_ERROR) {
			*ddb_index = mbox_sts[2];
		} else {
			DEBUG2(printk("scsi%ld: %s: failed status %04X\n",
			     ha->host_no, __func__, mbox_sts[0]));
			return QLA_ERROR;
		}
	} else {
		*ddb_index = MAX_PRST_DEV_DB_ENTRIES;
	}

	return QLA_SUCCESS;
}


int qla4xxx_send_tgts(struct scsi_qla_host *ha, char *ip, uint16_t port)
{
	struct dev_db_entry *fw_ddb_entry;
	dma_addr_t fw_ddb_entry_dma;
	uint32_t ddb_index;
	int ret_val = QLA_SUCCESS;


	fw_ddb_entry = dma_alloc_coherent(&ha->pdev->dev,
					  sizeof(*fw_ddb_entry),
					  &fw_ddb_entry_dma, GFP_KERNEL);
	if (!fw_ddb_entry) {
		DEBUG2(printk("scsi%ld: %s: Unable to allocate dma buffer.\n",
			      ha->host_no, __func__));
		ret_val = QLA_ERROR;
		goto qla4xxx_send_tgts_exit;
	}

	ret_val = qla4xxx_get_default_ddb(ha, fw_ddb_entry_dma);
	if (ret_val != QLA_SUCCESS)
		goto qla4xxx_send_tgts_exit;

	ret_val = qla4xxx_req_ddb_entry(ha, &ddb_index);
	if (ret_val != QLA_SUCCESS)
		goto qla4xxx_send_tgts_exit;

	memset(fw_ddb_entry->iscsi_alias, 0,
	       sizeof(fw_ddb_entry->iscsi_alias));

	memset(fw_ddb_entry->iscsi_name, 0,
	       sizeof(fw_ddb_entry->iscsi_name));

	memset(fw_ddb_entry->ip_addr, 0, sizeof(fw_ddb_entry->ip_addr));
	memset(fw_ddb_entry->tgt_addr, 0,
	       sizeof(fw_ddb_entry->tgt_addr));

	fw_ddb_entry->options = (DDB_OPT_DISC_SESSION | DDB_OPT_TARGET);
	fw_ddb_entry->port = cpu_to_le16(ntohs(port));

	fw_ddb_entry->ip_addr[0] = *ip;
	fw_ddb_entry->ip_addr[1] = *(ip + 1);
	fw_ddb_entry->ip_addr[2] = *(ip + 2);
	fw_ddb_entry->ip_addr[3] = *(ip + 3);

	ret_val = qla4xxx_set_ddb_entry(ha, ddb_index, fw_ddb_entry_dma);

qla4xxx_send_tgts_exit:
	dma_free_coherent(&ha->pdev->dev, sizeof(*fw_ddb_entry),
			  fw_ddb_entry, fw_ddb_entry_dma);
	return ret_val;
}

