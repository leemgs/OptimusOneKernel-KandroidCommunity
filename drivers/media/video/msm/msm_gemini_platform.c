

#include <linux/module.h>
#include <linux/pm_qos_params.h>
#include <linux/clk.h>
#include <mach/clk.h>
#include <linux/io.h>
#include <linux/android_pmem.h>
#include <mach/msm_reqs.h>

#include "msm_gemini_platform.h"
#include "msm_gemini_common.h"

#ifdef CONFIG_MSM_NPA_SYSTEM_BUS

#define MSM_SYSTEM_BUS_RATE	MSM_AXI_FLOW_JPEG_12MP
#else

#define MSM_SYSTEM_BUS_RATE	160000
#endif

void msm_gemini_platform_p2v(struct file  *file)
{
	put_pmem_file(file);
}

uint32_t msm_gemini_platform_v2p(int fd, uint32_t len, struct file **file_p)
{
	unsigned long paddr;
	unsigned long kvstart;
	unsigned long size;
	int rc;

	rc = get_pmem_file(fd, &paddr, &kvstart, &size, file_p);
	if (rc < 0) {
		GMN_PR_ERR("%s: get_pmem_file fd %d error %d\n", __func__, fd,
			rc);
		return 0;
	}

	
	if (len > size) {
		GMN_PR_ERR("%s: invalid offset + len\n", __func__);
		return 0;
	}

	return paddr;
}

static struct clk *jpeg_clk;
static struct clk *jpeg_pclk;

int msm_gemini_platform_clk_enable(void)
{
	
	int rc = -1;
	u32 rate = 144000000;

	if (jpeg_clk  == NULL) {
		jpeg_clk  = clk_get(NULL, "jpeg_clk");
		if (jpeg_clk  == NULL) {
			GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__,
				rc);
			goto fail;
		}
	}

	rc = clk_set_min_rate(jpeg_clk, rate);
	if (rc) {
		GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__, rc);
		goto fail;
	}

	rc = clk_enable(jpeg_clk);
	if (rc) {
		GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__, rc);
		goto fail;
	}

	if (jpeg_pclk == NULL) {
		jpeg_pclk = clk_get(NULL, "jpeg_pclk");
		if (jpeg_pclk == NULL) {
			GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__,
				rc);
			goto fail;
		}
	}

	rc = clk_enable(jpeg_pclk);
	if (rc) {
		GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__, rc);
		goto fail;
	}

	

GMN_DBG("%s:%d]\n", __func__, __LINE__);
	return rc;

fail:
	GMN_PR_ERR("%s:%d] fail rc = %d\n", __func__, __LINE__, rc);
	return rc;
}

int msm_gemini_platform_clk_disable(void)
{
	clk_disable(jpeg_clk);
	clk_put(jpeg_clk);
	jpeg_clk = NULL;

	clk_disable(jpeg_pclk);
	clk_put(jpeg_pclk);
	jpeg_pclk = NULL;

	
	return 0;
}

int msm_gemini_platform_init(struct platform_device *pdev,
	struct resource **mem,
	void **base,
	int *irq,
	irqreturn_t (*handler) (int, void *),
	void *context)
{
	int rc = -1;
	int gemini_irq;
	struct resource *gemini_mem, *gemini_io, *gemini_irq_res;
	void *gemini_base;

	gemini_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!gemini_mem) {
		GMN_PR_ERR("%s: no mem resource?\n", __func__);
		return -ENODEV;
	}

	gemini_irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!gemini_irq_res) {
		GMN_PR_ERR("no irq resource?\n");
		return -ENODEV;
	}
	gemini_irq = gemini_irq_res->start;

	gemini_io = request_mem_region(gemini_mem->start,
		resource_size(gemini_mem), pdev->name);
	if (!gemini_io) {
		GMN_PR_ERR("%s: region already claimed\n", __func__);
		return -EBUSY;
	}

	gemini_base = ioremap(gemini_mem->start, resource_size(gemini_mem));
	if (!gemini_base) {
		rc = -ENOMEM;
		GMN_PR_ERR("%s: ioremap failed\n", __func__);
		goto fail1;
	}

	rc = request_irq(gemini_irq, handler, IRQF_TRIGGER_RISING, "gemini",
		context);
	if (rc) {
		GMN_PR_ERR("%s: request_irq failed, %d, JPEG = %d\n", __func__,
			gemini_irq, INT_JPEG);
		goto fail2;
	}

	rc = msm_gemini_platform_clk_enable();
	if (rc) {
		GMN_PR_ERR("%s: clk failed rc = %d\n", __func__, rc);
		goto fail3;
	}

	*mem  = gemini_mem;
	*base = gemini_base;
	*irq  = gemini_irq;
	GMN_DBG("%s:%d] success\n", __func__, __LINE__);

	return rc;

fail3:
	free_irq(gemini_irq, context);
fail2:
	iounmap(gemini_base);
fail1:
	release_mem_region(gemini_mem->start, resource_size(gemini_mem));
	GMN_DBG("%s:%d] fail\n", __func__, __LINE__);
	return rc;
}

int msm_gemini_platform_release(struct resource *mem, void *base, int irq,
	void *context)
{
	int result;

	result = msm_gemini_platform_clk_disable();

	free_irq(irq, context);

	iounmap(base);
	release_mem_region(mem->start, resource_size(mem));

	GMN_DBG("%s:%d] success\n", __func__, __LINE__);
	return result;
}

