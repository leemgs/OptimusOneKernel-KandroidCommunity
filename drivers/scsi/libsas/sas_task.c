#include <linux/kernel.h>
#include <scsi/sas.h>
#include <scsi/libsas.h>


void sas_ssp_task_response(struct device *dev, struct sas_task *task,
			   struct ssp_response_iu *iu)
{
	struct task_status_struct *tstat = &task->task_status;

	tstat->resp = SAS_TASK_COMPLETE;

	if (iu->datapres == 0)
		tstat->stat = iu->status;
	else if (iu->datapres == 1)
		tstat->stat = iu->resp_data[3];
	else if (iu->datapres == 2) {
		tstat->stat = SAM_CHECK_COND;
		tstat->buf_valid_size =
			min_t(int, SAS_STATUS_BUF_SIZE,
			      be32_to_cpu(iu->sense_data_len));
		memcpy(tstat->buf, iu->sense_data, tstat->buf_valid_size);

		if (iu->status != SAM_CHECK_COND)
			dev_printk(KERN_WARNING, dev,
				   "dev %llx sent sense data, but "
				   "stat(%x) is not CHECK CONDITION\n",
				   SAS_ADDR(task->dev->sas_addr),
				   iu->status);
	}
	else
		
		tstat->stat = SAM_CHECK_COND;
}
EXPORT_SYMBOL_GPL(sas_ssp_task_response);

