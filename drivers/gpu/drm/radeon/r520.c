
#include "drmP.h"
#include "radeon.h"
#include "atom.h"
#include "r520d.h"



static int r520_mc_wait_for_idle(struct radeon_device *rdev)
{
	unsigned i;
	uint32_t tmp;

	for (i = 0; i < rdev->usec_timeout; i++) {
		
		tmp = RREG32_MC(R520_MC_STATUS);
		if (tmp & R520_MC_STATUS_IDLE) {
			return 0;
		}
		DRM_UDELAY(1);
	}
	return -1;
}

static void r520_gpu_init(struct radeon_device *rdev)
{
	unsigned pipe_select_current, gb_pipe_select, tmp;

	r100_hdp_reset(rdev);
	rv515_vga_render_disable(rdev);
	
	
	if (rdev->family == CHIP_RV530) {
		WREG32(0x4128, 0xFF);
	}
	r420_pipes_init(rdev);
	gb_pipe_select = RREG32(0x402C);
	tmp = RREG32(0x170C);
	pipe_select_current = (tmp >> 2) & 3;
	tmp = (1 << pipe_select_current) |
	      (((gb_pipe_select >> 8) & 0xF) << 4);
	WREG32_PLL(0x000D, tmp);
	if (r520_mc_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait MC idle while "
		       "programming pipes. Bad things might happen.\n");
	}
}

static void r520_vram_get_type(struct radeon_device *rdev)
{
	uint32_t tmp;

	rdev->mc.vram_width = 128;
	rdev->mc.vram_is_ddr = true;
	tmp = RREG32_MC(R520_MC_CNTL0);
	switch ((tmp & R520_MEM_NUM_CHANNELS_MASK) >> R520_MEM_NUM_CHANNELS_SHIFT) {
	case 0:
		rdev->mc.vram_width = 32;
		break;
	case 1:
		rdev->mc.vram_width = 64;
		break;
	case 2:
		rdev->mc.vram_width = 128;
		break;
	case 3:
		rdev->mc.vram_width = 256;
		break;
	default:
		rdev->mc.vram_width = 128;
		break;
	}
	if (tmp & R520_MC_CHANNEL_SIZE)
		rdev->mc.vram_width *= 2;
}

void r520_vram_info(struct radeon_device *rdev)
{
	fixed20_12 a;

	r520_vram_get_type(rdev);

	r100_vram_init_sizes(rdev);
	
	a.full = rfixed_const(100);
	rdev->pm.sclk.full = rfixed_const(rdev->clock.default_sclk);
	rdev->pm.sclk.full = rfixed_div(rdev->pm.sclk, a);
}

void r520_mc_program(struct radeon_device *rdev)
{
	struct rv515_mc_save save;

	
	rv515_mc_stop(rdev, &save);

	
	if (r520_mc_wait_for_idle(rdev))
		dev_warn(rdev->dev, "Wait MC idle timeout before updating MC.\n");
	
	WREG32(R_0000F8_CONFIG_MEMSIZE, rdev->mc.real_vram_size);
	
	WREG32_MC(R_000004_MC_FB_LOCATION,
			S_000004_MC_FB_START(rdev->mc.vram_start >> 16) |
			S_000004_MC_FB_TOP(rdev->mc.vram_end >> 16));
	WREG32(R_000134_HDP_FB_LOCATION,
		S_000134_HDP_FB_START(rdev->mc.vram_start >> 16));
	if (rdev->flags & RADEON_IS_AGP) {
		WREG32_MC(R_000005_MC_AGP_LOCATION,
			S_000005_MC_AGP_START(rdev->mc.gtt_start >> 16) |
			S_000005_MC_AGP_TOP(rdev->mc.gtt_end >> 16));
		WREG32_MC(R_000006_AGP_BASE, lower_32_bits(rdev->mc.agp_base));
		WREG32_MC(R_000007_AGP_BASE_2,
			S_000007_AGP_BASE_ADDR_2(upper_32_bits(rdev->mc.agp_base)));
	} else {
		WREG32_MC(R_000005_MC_AGP_LOCATION, 0xFFFFFFFF);
		WREG32_MC(R_000006_AGP_BASE, 0);
		WREG32_MC(R_000007_AGP_BASE_2, 0);
	}

	rv515_mc_resume(rdev, &save);
}

static int r520_startup(struct radeon_device *rdev)
{
	int r;

	r520_mc_program(rdev);
	
	rv515_clock_startup(rdev);
	
	r520_gpu_init(rdev);
	
	if (rdev->flags & RADEON_IS_PCIE) {
		r = rv370_pcie_gart_enable(rdev);
		if (r)
			return r;
	}
	
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

int r520_resume(struct radeon_device *rdev)
{
	
	if (rdev->flags & RADEON_IS_PCIE)
		rv370_pcie_gart_disable(rdev);
	
	rv515_clock_startup(rdev);
	
	if (radeon_gpu_reset(rdev)) {
		dev_warn(rdev->dev, "GPU reset failed ! (0xE40=0x%08X, 0x7C0=0x%08X)\n",
			RREG32(R_000E40_RBBM_STATUS),
			RREG32(R_0007C0_CP_STAT));
	}
	
	atom_asic_init(rdev->mode_info.atom_context);
	
	rv515_clock_startup(rdev);
	return r520_startup(rdev);
}

int r520_init(struct radeon_device *rdev)
{
	int r;

	
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
	
	r520_vram_info(rdev);
	
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
	r = rv370_pcie_gart_init(rdev);
	if (r)
		return r;
	rv515_set_safe_registers(rdev);
	rdev->accel_working = true;
	r = r520_startup(rdev);
	if (r) {
		
		dev_err(rdev->dev, "Disabling GPU acceleration\n");
		rv515_suspend(rdev);
		r100_cp_fini(rdev);
		r100_wb_fini(rdev);
		r100_ib_fini(rdev);
		rv370_pcie_gart_fini(rdev);
		radeon_agp_fini(rdev);
		radeon_irq_kms_fini(rdev);
		rdev->accel_working = false;
	}
	return 0;
}
