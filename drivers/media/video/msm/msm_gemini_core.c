

#include <linux/module.h>
#include "msm_gemini_hw.h"
#include "msm_gemini_core.h"
#include "msm_gemini_platform.h"
#include "msm_gemini_common.h"

static struct msm_gemini_hw_pingpong fe_pingpong_buf;
static struct msm_gemini_hw_pingpong we_pingpong_buf;

int msm_gemini_core_reset(uint8_t op_mode, void *base, int size)
{
	memset(&fe_pingpong_buf, 0, sizeof(fe_pingpong_buf));
	fe_pingpong_buf.is_fe = 1;
	memset(&we_pingpong_buf, 0, sizeof(we_pingpong_buf));
	msm_gemini_hw_reset(base, size);

	if (op_mode == MSM_GEMINI_MODE_REALTIME_ENCODE) {
		
		msm_gemini_hw_we_buffer_cfg(1);
	} else {
		
		msm_gemini_hw_we_buffer_cfg(0);
	}

	

	return 0;
}

int msm_gemini_core_fe_start(void)
{
	msm_gemini_hw_fe_start();
	return 0;
}


int msm_gemini_core_fe_buf_update(struct msm_gemini_core_buf *buf)
{
	GMN_DBG("%s:%d] 0x%08x %d 0x%08x %d\n", __func__, __LINE__,
		(int) buf->y_buffer_addr, buf->y_len,
		(int) buf->cbcr_buffer_addr, buf->cbcr_len);
	return msm_gemini_hw_pingpong_update(&fe_pingpong_buf, buf);
}

void *msm_gemini_core_fe_pingpong_irq(int gemini_irq_status, void *context)
{
	return msm_gemini_hw_pingpong_irq(&fe_pingpong_buf);
}


int msm_gemini_core_we_buf_update(struct msm_gemini_core_buf *buf)
{
	int rc;
	GMN_DBG("%s:%d] 0x%08x 0x%08x %d\n", __func__, __LINE__,
		(int) buf->y_buffer_addr, (int) buf->cbcr_buffer_addr,
		buf->y_len);
	rc = msm_gemini_hw_pingpong_update(&we_pingpong_buf, buf);
	return 0;
}

void *msm_gemini_core_we_pingpong_irq(int gemini_irq_status, void *context)
{
	GMN_DBG("%s:%d]\n", __func__, __LINE__);

	return msm_gemini_hw_pingpong_irq(&we_pingpong_buf);
}

void *msm_gemini_core_framedone_irq(int gemini_irq_status, void *context)
{
	struct msm_gemini_hw_buf *buf_p;

	GMN_DBG("%s:%d]\n", __func__, __LINE__);

	buf_p = msm_gemini_hw_pingpong_active_buffer(&we_pingpong_buf);
	buf_p->framedone_len = msm_gemini_hw_encode_output_size();

	GMN_DBG("%s:%d] framedone_len %d\n", __func__, __LINE__,
		buf_p->framedone_len);
	return buf_p;
}

void *msm_gemini_core_reset_ack_irq(int gemini_irq_status, void *context)
{
	
	GMN_DBG("%s:%d]\n", __func__, __LINE__);
	return NULL;
}

void *msm_gemini_core_err_irq(int gemini_irq_status, void *context)
{
	GMN_PR_ERR("%s:%d]\n", __func__, gemini_irq_status);
	return NULL;
}

static int (*msm_gemini_irq_handler) (int, void *, void *);

irqreturn_t msm_gemini_core_irq(int irq_num, void *context)
{
	void *data = NULL;

	int gemini_irq_status;

	GMN_DBG("%s:%d] irq_num = %d\n", __func__, __LINE__, irq_num);

	gemini_irq_status = msm_gemini_hw_irq_get_status();

	GMN_DBG("%s:%d] gemini_irq_status = %0x\n", __func__, __LINE__,
		gemini_irq_status);

	msm_gemini_hw_irq_clear();

	if (msm_gemini_hw_irq_is_frame_done(gemini_irq_status)) {
		data = msm_gemini_core_framedone_irq(gemini_irq_status,
			context);
		if (msm_gemini_irq_handler)
			msm_gemini_irq_handler(
				MSM_GEMINI_HW_MASK_COMP_FRAMEDONE,
				context, data);
	}

	if (msm_gemini_hw_irq_is_fe_pingpong(gemini_irq_status)) {
		data = msm_gemini_core_fe_pingpong_irq(gemini_irq_status,
			context);
		if (msm_gemini_irq_handler)
			msm_gemini_irq_handler(MSM_GEMINI_HW_MASK_COMP_FE,
				context, data);
	}

	if (msm_gemini_hw_irq_is_we_pingpong(gemini_irq_status) &&
	    !msm_gemini_hw_irq_is_frame_done(gemini_irq_status)) {
		data = msm_gemini_core_we_pingpong_irq(gemini_irq_status,
			context);
		if (msm_gemini_irq_handler)
			msm_gemini_irq_handler(MSM_GEMINI_HW_MASK_COMP_WE,
				context, data);
	}

	if (msm_gemini_hw_irq_is_reset_ack(gemini_irq_status)) {
		data = msm_gemini_core_reset_ack_irq(gemini_irq_status,
			context);
		if (msm_gemini_irq_handler)
			msm_gemini_irq_handler(
				MSM_GEMINI_HW_MASK_COMP_RESET_ACK,
				context, data);
	}

	
	if (msm_gemini_hw_irq_is_err(gemini_irq_status)) {
		data = msm_gemini_core_err_irq(gemini_irq_status, context);
		if (msm_gemini_irq_handler)
			msm_gemini_irq_handler(MSM_GEMINI_HW_MASK_COMP_ERR,
				context, data);
	}

	return IRQ_HANDLED;
}

void msm_gemini_core_irq_install(int (*irq_handler) (int, void *, void *))
{
	msm_gemini_irq_handler = irq_handler;
}

void msm_gemini_core_irq_remove(void)
{
	msm_gemini_irq_handler = NULL;
}
