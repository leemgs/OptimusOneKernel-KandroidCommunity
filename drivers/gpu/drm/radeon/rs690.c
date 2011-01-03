
#include "drmP.h"
#include "radeon.h"
#include "atom.h"
#include "rs690d.h"

static int rs690_mc_wait_for_idle(struct radeon_device *rdev)
{
	unsigned i;
	uint32_t tmp;

	for (i = 0; i < rdev->usec_timeout; i++) {
		
		tmp = RREG32_MC(R_000090_MC_SYSTEM_STATUS);
		if (G_000090_MC_SYSTEM_IDLE(tmp))
			return 0;
		udelay(1);
	}
	return -1;
}

static void rs690_gpu_init(struct radeon_device *rdev)
{
	
	r100_hdp_reset(rdev);
	
	r420_pipes_init(rdev);
	if (rs690_mc_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait MC idle while "
		       "programming pipes. Bad things might happen.\n");
	}
}

void rs690_pm_info(struct radeon_device *rdev)
{
	int index = GetIndexIntoMasterTable(DATA, IntegratedSystemInfo);
	struct _ATOM_INTEGRATED_SYSTEM_INFO *info;
	struct _ATOM_INTEGRATED_SYSTEM_INFO_V2 *info_v2;
	void *ptr;
	uint16_t data_offset;
	uint8_t frev, crev;
	fixed20_12 tmp;

	atom_parse_data_header(rdev->mode_info.atom_context, index, NULL,
			       &frev, &crev, &data_offset);
	ptr = rdev->mode_info.atom_context->bios + data_offset;
	info = (struct _ATOM_INTEGRATED_SYSTEM_INFO *)ptr;
	info_v2 = (struct _ATOM_INTEGRATED_SYSTEM_INFO_V2 *)ptr;
	
	switch (crev) {
	case 1:
		tmp.full = rfixed_const(100);
		rdev->pm.igp_sideport_mclk.full = rfixed_const(info->ulBootUpMemoryClock);
		rdev->pm.igp_sideport_mclk.full = rfixed_div(rdev->pm.igp_sideport_mclk, tmp);
		rdev->pm.igp_system_mclk.full = rfixed_const(le16_to_cpu(info->usK8MemoryClock));
		rdev->pm.igp_ht_link_clk.full = rfixed_const(le16_to_cpu(info->usFSBClock));
		rdev->pm.igp_ht_link_width.full = rfixed_const(info->ucHTLinkWidth);
		break;
	case 2:
		tmp.full = rfixed_const(100);
		rdev->pm.igp_sideport_mclk.full = rfixed_const(info_v2->ulBootUpSidePortClock);
		rdev->pm.igp_sideport_mclk.full = rfixed_div(rdev->pm.igp_sideport_mclk, tmp);
		rdev->pm.igp_system_mclk.full = rfixed_const(info_v2->ulBootUpUMAClock);
		rdev->pm.igp_system_mclk.full = rfixed_div(rdev->pm.igp_system_mclk, tmp);
		rdev->pm.igp_ht_link_clk.full = rfixed_const(info_v2->ulHTLinkFreq);
		rdev->pm.igp_ht_link_clk.full = rfixed_div(rdev->pm.igp_ht_link_clk, tmp);
		rdev->pm.igp_ht_link_width.full = rfixed_const(le16_to_cpu(info_v2->usMinHTLinkWidth));
		break;
	default:
		tmp.full = rfixed_const(100);
		
		
		rdev->pm.igp_sideport_mclk.full = rfixed_const(333);
		
		rdev->pm.igp_system_mclk.full = rfixed_const(100);
		rdev->pm.igp_system_mclk.full = rfixed_div(rdev->pm.igp_system_mclk, tmp);
		rdev->pm.igp_ht_link_clk.full = rfixed_const(200);
		rdev->pm.igp_ht_link_width.full = rfixed_const(8);
		DRM_ERROR("No integrated system info for your GPU, using safe default\n");
		break;
	}
	
	
	tmp.full = rfixed_const(4);
	rdev->pm.k8_bandwidth.full = rfixed_mul(rdev->pm.igp_system_mclk, tmp);
	
	tmp.full = rfixed_const(5);
	rdev->pm.ht_bandwidth.full = rfixed_mul(rdev->pm.igp_ht_link_clk,
						rdev->pm.igp_ht_link_width);
	rdev->pm.ht_bandwidth.full = rfixed_div(rdev->pm.ht_bandwidth, tmp);
	if (tmp.full < rdev->pm.max_bandwidth.full) {
		
		rdev->pm.max_bandwidth.full = tmp.full;
	}
	
	tmp.full = rfixed_const(14);
	rdev->pm.sideport_bandwidth.full = rfixed_mul(rdev->pm.igp_sideport_mclk, tmp);
	tmp.full = rfixed_const(10);
	rdev->pm.sideport_bandwidth.full = rfixed_div(rdev->pm.sideport_bandwidth, tmp);
}

void rs690_vram_info(struct radeon_device *rdev)
{
	fixed20_12 a;

	rs400_gart_adjust_size(rdev);

	rdev->mc.vram_is_ddr = true;
	rdev->mc.vram_width = 128;

	rdev->mc.real_vram_size = RREG32(RADEON_CONFIG_MEMSIZE);
	rdev->mc.mc_vram_size = rdev->mc.real_vram_size;

	rdev->mc.aper_base = drm_get_resource_start(rdev->ddev, 0);
	rdev->mc.aper_size = drm_get_resource_len(rdev->ddev, 0);

	if (rdev->mc.mc_vram_size > rdev->mc.aper_size)
		rdev->mc.mc_vram_size = rdev->mc.aper_size;

	if (rdev->mc.real_vram_size > rdev->mc.aper_size)
		rdev->mc.real_vram_size = rdev->mc.aper_size;

	rs690_pm_info(rdev);
	
	a.full = rfixed_const(100);
	rdev->pm.sclk.full = rfixed_const(rdev->clock.default_sclk);
	rdev->pm.sclk.full = rfixed_div(rdev->pm.sclk, a);
	a.full = rfixed_const(16);
	
	rdev->pm.core_bandwidth.full = rfixed_div(rdev->pm.sclk, a);
}

void rs690_line_buffer_adjust(struct radeon_device *rdev,
			      struct drm_display_mode *mode1,
			      struct drm_display_mode *mode2)
{
	u32 tmp;

	
	tmp = RREG32(R_006520_DC_LB_MEMORY_SPLIT) & C_006520_DC_LB_MEMORY_SPLIT;
	tmp &= ~C_006520_DC_LB_MEMORY_SPLIT_MODE;
	
	if (mode1 && mode2) {
		if (mode1->hdisplay > mode2->hdisplay) {
			if (mode1->hdisplay > 2560)
				tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1_3Q_D2_1Q;
			else
				tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1HALF_D2HALF;
		} else if (mode2->hdisplay > mode1->hdisplay) {
			if (mode2->hdisplay > 2560)
				tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1_1Q_D2_3Q;
			else
				tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1HALF_D2HALF;
		} else
			tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1HALF_D2HALF;
	} else if (mode1) {
		tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1_ONLY;
	} else if (mode2) {
		tmp |= V_006520_DC_LB_MEMORY_SPLIT_D1_1Q_D2_3Q;
	}
	WREG32(R_006520_DC_LB_MEMORY_SPLIT, tmp);
}

struct rs690_watermark {
	u32        lb_request_fifo_depth;
	fixed20_12 num_line_pair;
	fixed20_12 estimated_width;
	fixed20_12 worst_case_latency;
	fixed20_12 consumption_rate;
	fixed20_12 active_time;
	fixed20_12 dbpp;
	fixed20_12 priority_mark_max;
	fixed20_12 priority_mark;
	fixed20_12 sclk;
};

void rs690_crtc_bandwidth_compute(struct radeon_device *rdev,
				  struct radeon_crtc *crtc,
				  struct rs690_watermark *wm)
{
	struct drm_display_mode *mode = &crtc->base.mode;
	fixed20_12 a, b, c;
	fixed20_12 pclk, request_fifo_depth, tolerable_latency, estimated_width;
	fixed20_12 consumption_time, line_time, chunk_time, read_delay_latency;
	
	bool sideport = false;

	if (!crtc->base.enabled) {
		
		wm->lb_request_fifo_depth = 4;
		return;
	}

	if (crtc->vsc.full > rfixed_const(2))
		wm->num_line_pair.full = rfixed_const(2);
	else
		wm->num_line_pair.full = rfixed_const(1);

	b.full = rfixed_const(mode->crtc_hdisplay);
	c.full = rfixed_const(256);
	a.full = rfixed_mul(wm->num_line_pair, b);
	request_fifo_depth.full = rfixed_div(a, c);
	if (a.full < rfixed_const(4)) {
		wm->lb_request_fifo_depth = 4;
	} else {
		wm->lb_request_fifo_depth = rfixed_trunc(request_fifo_depth);
	}

	
	a.full = rfixed_const(mode->clock);
	b.full = rfixed_const(1000);
	a.full = rfixed_div(a, b);
	pclk.full = rfixed_div(b, a);
	if (crtc->rmx_type != RMX_OFF) {
		b.full = rfixed_const(2);
		if (crtc->vsc.full > b.full)
			b.full = crtc->vsc.full;
		b.full = rfixed_mul(b, crtc->hsc);
		c.full = rfixed_const(2);
		b.full = rfixed_div(b, c);
		consumption_time.full = rfixed_div(pclk, b);
	} else {
		consumption_time.full = pclk.full;
	}
	a.full = rfixed_const(1);
	wm->consumption_rate.full = rfixed_div(a, consumption_time);


	
	a.full = rfixed_const(crtc->base.mode.crtc_htotal);
	line_time.full = rfixed_mul(a, pclk);

	
	a.full = rfixed_const(crtc->base.mode.crtc_htotal);
	b.full = rfixed_const(crtc->base.mode.crtc_hdisplay);
	wm->active_time.full = rfixed_mul(line_time, b);
	wm->active_time.full = rfixed_div(wm->active_time, a);

	
	rdev->pm.max_bandwidth = rdev->pm.core_bandwidth;
	if (sideport) {
		if (rdev->pm.max_bandwidth.full > rdev->pm.sideport_bandwidth.full &&
			rdev->pm.sideport_bandwidth.full)
			rdev->pm.max_bandwidth = rdev->pm.sideport_bandwidth;
		read_delay_latency.full = rfixed_const(370 * 800 * 1000);
		read_delay_latency.full = rfixed_div(read_delay_latency,
			rdev->pm.igp_sideport_mclk);
	} else {
		if (rdev->pm.max_bandwidth.full > rdev->pm.k8_bandwidth.full &&
			rdev->pm.k8_bandwidth.full)
			rdev->pm.max_bandwidth = rdev->pm.k8_bandwidth;
		if (rdev->pm.max_bandwidth.full > rdev->pm.ht_bandwidth.full &&
			rdev->pm.ht_bandwidth.full)
			rdev->pm.max_bandwidth = rdev->pm.ht_bandwidth;
		read_delay_latency.full = rfixed_const(5000);
	}

	
	a.full = rfixed_const(16);
	rdev->pm.sclk.full = rfixed_mul(rdev->pm.max_bandwidth, a);
	a.full = rfixed_const(1000);
	rdev->pm.sclk.full = rfixed_div(a, rdev->pm.sclk);
	
	a.full = rfixed_const(256 * 13);
	chunk_time.full = rfixed_mul(rdev->pm.sclk, a);
	a.full = rfixed_const(10);
	chunk_time.full = rfixed_div(chunk_time, a);

	
	if (rfixed_trunc(wm->num_line_pair) > 1) {
		a.full = rfixed_const(3);
		wm->worst_case_latency.full = rfixed_mul(a, chunk_time);
		wm->worst_case_latency.full += read_delay_latency.full;
	} else {
		a.full = rfixed_const(2);
		wm->worst_case_latency.full = rfixed_mul(a, chunk_time);
		wm->worst_case_latency.full += read_delay_latency.full;
	}

	
	if ((2+wm->lb_request_fifo_depth) >= rfixed_trunc(request_fifo_depth)) {
		tolerable_latency.full = line_time.full;
	} else {
		tolerable_latency.full = rfixed_const(wm->lb_request_fifo_depth - 2);
		tolerable_latency.full = request_fifo_depth.full - tolerable_latency.full;
		tolerable_latency.full = rfixed_mul(tolerable_latency, chunk_time);
		tolerable_latency.full = line_time.full - tolerable_latency.full;
	}
	
	wm->dbpp.full = rfixed_const(4 * 8);

	
	a.full = rfixed_const(16);
	wm->priority_mark_max.full = rfixed_const(crtc->base.mode.crtc_hdisplay);
	wm->priority_mark_max.full = rfixed_div(wm->priority_mark_max, a);

	
	estimated_width.full = tolerable_latency.full - wm->worst_case_latency.full;
	estimated_width.full = rfixed_div(estimated_width, consumption_time);
	if (rfixed_trunc(estimated_width) > crtc->base.mode.crtc_hdisplay) {
		wm->priority_mark.full = rfixed_const(10);
	} else {
		a.full = rfixed_const(16);
		wm->priority_mark.full = rfixed_div(estimated_width, a);
		wm->priority_mark.full = wm->priority_mark_max.full - wm->priority_mark.full;
	}
}

void rs690_bandwidth_update(struct radeon_device *rdev)
{
	struct drm_display_mode *mode0 = NULL;
	struct drm_display_mode *mode1 = NULL;
	struct rs690_watermark wm0;
	struct rs690_watermark wm1;
	u32 tmp;
	fixed20_12 priority_mark02, priority_mark12, fill_rate;
	fixed20_12 a, b;

	if (rdev->mode_info.crtcs[0]->base.enabled)
		mode0 = &rdev->mode_info.crtcs[0]->base.mode;
	if (rdev->mode_info.crtcs[1]->base.enabled)
		mode1 = &rdev->mode_info.crtcs[1]->base.mode;
	
	if (rdev->disp_priority == 2) {
		tmp = RREG32_MC(R_000104_MC_INIT_MISC_LAT_TIMER);
		tmp &= C_000104_MC_DISP0R_INIT_LAT;
		tmp &= C_000104_MC_DISP1R_INIT_LAT;
		if (mode0)
			tmp |= S_000104_MC_DISP0R_INIT_LAT(1);
		if (mode1)
			tmp |= S_000104_MC_DISP1R_INIT_LAT(1);
		WREG32_MC(R_000104_MC_INIT_MISC_LAT_TIMER, tmp);
	}
	rs690_line_buffer_adjust(rdev, mode0, mode1);

	if ((rdev->family == CHIP_RS690) || (rdev->family == CHIP_RS740))
		WREG32(R_006C9C_DCP_CONTROL, 0);
	if ((rdev->family == CHIP_RS780) || (rdev->family == CHIP_RS880))
		WREG32(R_006C9C_DCP_CONTROL, 2);

	rs690_crtc_bandwidth_compute(rdev, rdev->mode_info.crtcs[0], &wm0);
	rs690_crtc_bandwidth_compute(rdev, rdev->mode_info.crtcs[1], &wm1);

	tmp = (wm0.lb_request_fifo_depth - 1);
	tmp |= (wm1.lb_request_fifo_depth - 1) << 16;
	WREG32(R_006D58_LB_MAX_REQ_OUTSTANDING, tmp);

	if (mode0 && mode1) {
		if (rfixed_trunc(wm0.dbpp) > 64)
			a.full = rfixed_mul(wm0.dbpp, wm0.num_line_pair);
		else
			a.full = wm0.num_line_pair.full;
		if (rfixed_trunc(wm1.dbpp) > 64)
			b.full = rfixed_mul(wm1.dbpp, wm1.num_line_pair);
		else
			b.full = wm1.num_line_pair.full;
		a.full += b.full;
		fill_rate.full = rfixed_div(wm0.sclk, a);
		if (wm0.consumption_rate.full > fill_rate.full) {
			b.full = wm0.consumption_rate.full - fill_rate.full;
			b.full = rfixed_mul(b, wm0.active_time);
			a.full = rfixed_mul(wm0.worst_case_latency,
						wm0.consumption_rate);
			a.full = a.full + b.full;
			b.full = rfixed_const(16 * 1000);
			priority_mark02.full = rfixed_div(a, b);
		} else {
			a.full = rfixed_mul(wm0.worst_case_latency,
						wm0.consumption_rate);
			b.full = rfixed_const(16 * 1000);
			priority_mark02.full = rfixed_div(a, b);
		}
		if (wm1.consumption_rate.full > fill_rate.full) {
			b.full = wm1.consumption_rate.full - fill_rate.full;
			b.full = rfixed_mul(b, wm1.active_time);
			a.full = rfixed_mul(wm1.worst_case_latency,
						wm1.consumption_rate);
			a.full = a.full + b.full;
			b.full = rfixed_const(16 * 1000);
			priority_mark12.full = rfixed_div(a, b);
		} else {
			a.full = rfixed_mul(wm1.worst_case_latency,
						wm1.consumption_rate);
			b.full = rfixed_const(16 * 1000);
			priority_mark12.full = rfixed_div(a, b);
		}
		if (wm0.priority_mark.full > priority_mark02.full)
			priority_mark02.full = wm0.priority_mark.full;
		if (rfixed_trunc(priority_mark02) < 0)
			priority_mark02.full = 0;
		if (wm0.priority_mark_max.full > priority_mark02.full)
			priority_mark02.full = wm0.priority_mark_max.full;
		if (wm1.priority_mark.full > priority_mark12.full)
			priority_mark12.full = wm1.priority_mark.full;
		if (rfixed_trunc(priority_mark12) < 0)
			priority_mark12.full = 0;
		if (wm1.priority_mark_max.full > priority_mark12.full)
			priority_mark12.full = wm1.priority_mark_max.full;
		WREG32(R_006548_D1MODE_PRIORITY_A_CNT, rfixed_trunc(priority_mark02));
		WREG32(R_00654C_D1MODE_PRIORITY_B_CNT, rfixed_trunc(priority_mark02));
		WREG32(R_006D48_D2MODE_PRIORITY_A_CNT, rfixed_trunc(priority_mark12));
		WREG32(R_006D4C_D2MODE_PRIORITY_B_CNT, rfixed_trunc(priority_mark12));
	} else if (mode0) {
		if (rfixed_trunc(wm0.dbpp) > 64)
			a.full = rfixed_mul(wm0.dbpp, wm0.num_line_pair);
		else
			a.full = wm0.num_line_pair.full;
		fill_rate.full = rfixed_div(wm0.sclk, a);
		if (wm0.consumption_rate.full > fill_rate.full) {
			b.full = wm0.consumption_rate.full - fill_rate.full;
			b.full = rfixed_mul(b, wm0.active_time);
			a.full = rfixed_mul(wm0.worst_case_latency,
						wm0.consumption_rate);
			a.full = a.full + b.full;
			b.full = rfixed_const(16 * 1000);
			priority_mark02.full = rfixed_div(a, b);
		} else {
			a.full = rfixed_mul(wm0.worst_case_latency,
						wm0.consumption_rate);
			b.full = rfixed_const(16 * 1000);
			priority_mark02.full = rfixed_div(a, b);
		}
		if (wm0.priority_mark.full > priority_mark02.full)
			priority_mark02.full = wm0.priority_mark.full;
		if (rfixed_trunc(priority_mark02) < 0)
			priority_mark02.full = 0;
		if (wm0.priority_mark_max.full > priority_mark02.full)
			priority_mark02.full = wm0.priority_mark_max.full;
		WREG32(R_006548_D1MODE_PRIORITY_A_CNT, rfixed_trunc(priority_mark02));
		WREG32(R_00654C_D1MODE_PRIORITY_B_CNT, rfixed_trunc(priority_mark02));
		WREG32(R_006D48_D2MODE_PRIORITY_A_CNT,
			S_006D48_D2MODE_PRIORITY_A_OFF(1));
		WREG32(R_006D4C_D2MODE_PRIORITY_B_CNT,
			S_006D4C_D2MODE_PRIORITY_B_OFF(1));
	} else {
		if (rfixed_trunc(wm1.dbpp) > 64)
			a.full = rfixed_mul(wm1.dbpp, wm1.num_line_pair);
		else
			a.full = wm1.num_line_pair.full;
		fill_rate.full = rfixed_div(wm1.sclk, a);
		if (wm1.consumption_rate.full > fill_rate.full) {
			b.full = wm1.consumption_rate.full - fill_rate.full;
			b.full = rfixed_mul(b, wm1.active_time);
			a.full = rfixed_mul(wm1.worst_case_latency,
						wm1.consumption_rate);
			a.full = a.full + b.full;
			b.full = rfixed_const(16 * 1000);
			priority_mark12.full = rfixed_div(a, b);
		} else {
			a.full = rfixed_mul(wm1.worst_case_latency,
						wm1.consumption_rate);
			b.full = rfixed_const(16 * 1000);
			priority_mark12.full = rfixed_div(a, b);
		}
		if (wm1.priority_mark.full > priority_mark12.full)
			priority_mark12.full = wm1.priority_mark.full;
		if (rfixed_trunc(priority_mark12) < 0)
			priority_mark12.full = 0;
		if (wm1.priority_mark_max.full > priority_mark12.full)
			priority_mark12.full = wm1.priority_mark_max.full;
		WREG32(R_006548_D1MODE_PRIORITY_A_CNT,
			S_006548_D1MODE_PRIORITY_A_OFF(1));
		WREG32(R_00654C_D1MODE_PRIORITY_B_CNT,
			S_00654C_D1MODE_PRIORITY_B_OFF(1));
		WREG32(R_006D48_D2MODE_PRIORITY_A_CNT, rfixed_trunc(priority_mark12));
		WREG32(R_006D4C_D2MODE_PRIORITY_B_CNT, rfixed_trunc(priority_mark12));
	}
}

uint32_t rs690_mc_rreg(struct radeon_device *rdev, uint32_t reg)
{
	uint32_t r;

	WREG32(R_000078_MC_INDEX, S_000078_MC_IND_ADDR(reg));
	r = RREG32(R_00007C_MC_DATA);
	WREG32(R_000078_MC_INDEX, ~C_000078_MC_IND_ADDR);
	return r;
}

void rs690_mc_wreg(struct radeon_device *rdev, uint32_t reg, uint32_t v)
{
	WREG32(R_000078_MC_INDEX, S_000078_MC_IND_ADDR(reg) |
		S_000078_MC_IND_WR_EN(1));
	WREG32(R_00007C_MC_DATA, v);
	WREG32(R_000078_MC_INDEX, 0x7F);
}

void rs690_mc_program(struct radeon_device *rdev)
{
	struct rv515_mc_save save;

	
	rv515_mc_stop(rdev, &save);

	
	if (rs690_mc_wait_for_idle(rdev))
		dev_warn(rdev->dev, "Wait MC idle timeout before updating MC.\n");
	
	WREG32_MC(R_000100_MCCFG_FB_LOCATION,
			S_000100_MC_FB_START(rdev->mc.vram_start >> 16) |
			S_000100_MC_FB_TOP(rdev->mc.vram_end >> 16));
	WREG32(R_000134_HDP_FB_LOCATION,
		S_000134_HDP_FB_START(rdev->mc.vram_start >> 16));

	rv515_mc_resume(rdev, &save);
}

static int rs690_startup(struct radeon_device *rdev)
{
	int r;

	rs690_mc_program(rdev);
	
	rv515_clock_startup(rdev);
	
	rs690_gpu_init(rdev);
	
	r = rs400_gart_enable(rdev);
	if (r)
		return r;
	
	rdev->irq.sw_int = true;
	rs600_irq_set(rdev);
	
	r = r100_cp_init(rdev, 1024 * 1024);
	if (r) {
		dev_err(rdev->dev, "failled initializing CP (%d).\n", r);
		return r;
	}
	r = r100_wb_init(rdev);
	if (r)
		dev_err(rdev->dev, "failled initializing WB (%d).\n", r);
	r = r100_ib_init(rdev);
	if (r) {
		dev_err(rdev->dev, "failled initializing IB (%d).\n", r);
		return r;
	}
	return 0;
}

int rs690_resume(struct radeon_device *rdev)
{
	
	rs400_gart_disable(rdev);
	
	rv515_clock_startup(rdev);
	
	if (radeon_gpu_reset(rdev)) {
		dev_warn(rdev->dev, "GPU reset failed ! (0xE40=0x%08X, 0x7C0=0x%08X)\n",
			RREG32(R_000E40_RBBM_STATUS),
			RREG32(R_0007C0_CP_STAT));
	}
	
	atom_asic_init(rdev->mode_info.atom_context);
	
	rv515_clock_startup(rdev);
	return rs690_startup(rdev);
}

int rs690_suspend(struct radeon_device *rdev)
{
	r100_cp_disable(rdev);
	r100_wb_disable(rdev);
	rs600_irq_disable(rdev);
	rs400_gart_disable(rdev);
	return 0;
}

void rs690_fini(struct radeon_device *rdev)
{
	rs690_suspend(rdev);
	r100_cp_fini(rdev);
	r100_wb_fini(rdev);
	r100_ib_fini(rdev);
	radeon_gem_fini(rdev);
	rs400_gart_fini(rdev);
	radeon_irq_kms_fini(rdev);
	radeon_fence_driver_fini(rdev);
	radeon_object_fini(rdev);
	radeon_atombios_fini(rdev);
	kfree(rdev->bios);
	rdev->bios = NULL;
}

int rs690_init(struct radeon_device *rdev)
{
	int r;

	
	rv515_vga_render_disable(rdev);
	
	radeon_scratch_init(rdev);
	
	radeon_surface_init(rdev);
	
	
	if (!radeon_get_bios(rdev)) {
		if (ASIC_IS_AVIVO(rdev))
			return -EINVAL;
	}
	if (rdev->is_atom_bios) {
		r = radeon_atombios_init(rdev);
		if (r)
			return r;
	} else {
		dev_err(rdev->dev, "Expecting atombios for RV515 GPU\n");
		return -EINVAL;
	}
	
	if (radeon_gpu_reset(rdev)) {
		dev_warn(rdev->dev,
			"GPU reset failed ! (0xE40=0x%08X, 0x7C0=0x%08X)\n",
			RREG32(R_000E40_RBBM_STATUS),
			RREG32(R_0007C0_CP_STAT));
	}
	
	if (!radeon_card_posted(rdev) && rdev->bios) {
		DRM_INFO("GPU not posted. posting now...\n");
		atom_asic_init(rdev->mode_info.atom_context);
	}
	
	radeon_get_clock_info(rdev->ddev);
	
	radeon_pm_init(rdev);
	
	rs690_vram_info(rdev);
	
	r = r420_mc_init(rdev);
	if (r)
		return r;
	rv515_debugfs(rdev);
	
	r = radeon_fence_driver_init(rdev);
	if (r)
		return r;
	r = radeon_irq_kms_init(rdev);
	if (r)
		return r;
	
	r = radeon_object_init(rdev);
	if (r)
		return r;
	r = rs400_gart_init(rdev);
	if (r)
		return r;
	rs600_set_safe_registers(rdev);
	rdev->accel_working = true;
	r = rs690_startup(rdev);
	if (r) {
		
		dev_err(rdev->dev, "Disabling GPU acceleration\n");
		rs690_suspend(rdev);
		r100_cp_fini(rdev);
		r100_wb_fini(rdev);
		r100_ib_fini(rdev);
		rs400_gart_fini(rdev);
		radeon_irq_kms_fini(rdev);
		rdev->accel_working = false;
	}
	return 0;
}
