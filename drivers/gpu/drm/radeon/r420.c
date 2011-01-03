
#include <linux/seq_file.h>
#include "drmP.h"
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"
#include "r420d.h"

int r420_mc_init(struct radeon_device *rdev)
{
	int r;

	
	rdev->mc.vram_location = 0xFFFFFFFFUL;
	rdev->mc.gtt_location = 0xFFFFFFFFUL;
	if (rdev->flags & RADEON_IS_AGP) {
		r = radeon_agp_init(rdev);
		if (r) {
			printk(KERN_WARNING "[drm] Disabling AGP\n");
			rdev->flags &= ~RADEON_IS_AGP;
			rdev->mc.gtt_size = radeon_gart_size * 1024 * 1024;
		} else {
			rdev->mc.gtt_location = rdev->mc.agp_base;
		}
	}
	r = radeon_mc_setup(rdev);
	if (r) {
		return r;
	}
	return 0;
}

void r420_pipes_init(struct radeon_device *rdev)
{
	unsigned tmp;
	unsigned gb_pipe_select;
	unsigned num_pipes;

	
	WREG32(0x4274, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	
	if (r100_gui_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait GUI idle while "
		       "programming pipes. Bad things might happen.\n");
	}
	
	gb_pipe_select = RREG32(0x402C);
	num_pipes = ((gb_pipe_select >> 12) & 3) + 1;
	rdev->num_gb_pipes = num_pipes;
	tmp = 0;
	switch (num_pipes) {
	default:
		
		num_pipes = 1;
	case 1:
		tmp = (0 << 1);
		break;
	case 2:
		tmp = (3 << 1);
		break;
	case 3:
		tmp = (6 << 1);
		break;
	case 4:
		tmp = (7 << 1);
		break;
	}
	WREG32(0x42C8, (1 << num_pipes) - 1);
	
	tmp |= (1 << 4) | (1 << 0);
	WREG32(0x4018, tmp);
	if (r100_gui_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait GUI idle while "
		       "programming pipes. Bad things might happen.\n");
	}

	tmp = RREG32(0x170C);
	WREG32(0x170C, tmp | (1 << 31));

	WREG32(R300_RB2D_DSTCACHE_MODE,
	       RREG32(R300_RB2D_DSTCACHE_MODE) |
	       R300_DC_AUTOFLUSH_ENABLE |
	       R300_DC_DC_DISABLE_IGNORE_PE);

	if (r100_gui_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait GUI idle while "
		       "programming pipes. Bad things might happen.\n");
	}

	if (rdev->family == CHIP_RV530) {
		tmp = RREG32(RV530_GB_PIPE_SELECT2);
		if ((tmp & 3) == 3)
			rdev->num_z_pipes = 2;
		else
			rdev->num_z_pipes = 1;
	} else
		rdev->num_z_pipes = 1;

	DRM_INFO("radeon: %d quad pipes, %d z pipes initialized.\n",
		 rdev->num_gb_pipes, rdev->num_z_pipes);
}

u32 r420_mc_rreg(struct radeon_device *rdev, u32 reg)
{
	u32 r;

	WREG32(R_0001F8_MC_IND_INDEX, S_0001F8_MC_IND_ADDR(reg));
	r = RREG32(R_0001FC_MC_IND_DATA);
	return r;
}

void r420_mc_wreg(struct radeon_device *rdev, u32 reg, u32 v)
{
	WREG32(R_0001F8_MC_IND_INDEX, S_0001F8_MC_IND_ADDR(reg) |
		S_0001F8_MC_IND_WR_EN(1));
	WREG32(R_0001FC_MC_IND_DATA, v);
}

static void r420_debugfs(struct radeon_device *rdev)
{
	if (r100_debugfs_rbbm_init(rdev)) {
		DRM_ERROR("Failed to register debugfs file for RBBM !\n");
	}
	if (r420_debugfs_pipes_info_init(rdev)) {
		DRM_ERROR("Failed to register debugfs file for pipes !\n");
	}
}

static void r420_clock_resume(struct radeon_device *rdev)
{
	u32 sclk_cntl;

	if (radeon_dynclks != -1 && radeon_dynclks)
		radeon_atom_set_clock_gating(rdev, 1);
	sclk_cntl = RREG32_PLL(R_00000D_SCLK_CNTL);
	sclk_cntl |= S_00000D_FORCE_CP(1) | S_00000D_FORCE_VIP(1);
	if (rdev->family == CHIP_R420)
		sclk_cntl |= S_00000D_FORCE_PX(1) | S_00000D_FORCE_TX(1);
	WREG32_PLL(R_00000D_SCLK_CNTL, sclk_cntl);
}

static int r420_startup(struct radeon_device *rdev)
{
	int r;

	r300_mc_program(rdev);
	
	r420_clock_resume(rdev);
	
	if (rdev->flags & RADEON_IS_PCIE) {
		r = rv370_pcie_gart_enable(rdev);
		if (r)
			return r;
	}
	if (rdev->flags & RADEON_IS_PCI) {
		r = r100_pci_gart_enable(rdev);
		if (r)
			return r;
	}
	r420_pipes_init(rdev);
	
	rdev->irq.sw_int = true;
	r100_irq_set(rdev);
	
	r = r100_cp_init(rdev, 1024 * 1024);
	if (r) {
		dev_err(rdev->dev, "failled initializing CP (%d).\n", r);
		return r;
	}
	r = r100_wb_init(rdev);
	if (r) {
		dev_err(rdev->dev, "failled initializing WB (%d).\n", r);
	}
	r = r100_ib_init(rdev);
	if (r) {
		dev_err(rdev->dev, "failled initializing IB (%d).\n", r);
		return r;
	}
	return 0;
}

int r420_resume(struct radeon_device *rdev)
{
	
	if (rdev->flags & RADEON_IS_PCIE)
		rv370_pcie_gart_disable(rdev);
	if (rdev->flags & RADEON_IS_PCI)
		r100_pci_gart_disable(rdev);
	
	r420_clock_resume(rdev);
	
	if (radeon_gpu_reset(rdev)) {
		dev_warn(rdev->dev, "GPU reset failed ! (0xE40=0x%08X, 0x7C0=0x%08X)\n",
			RREG32(R_000E40_RBBM_STATUS),
			RREG32(R_0007C0_CP_STAT));
	}
	
	if (rdev->is_atom_bios) {
		atom_asic_init(rdev->mode_info.atom_context);
	} else {
		radeon_combios_asic_init(rdev->ddev);
	}
	
	r420_clock_resume(rdev);

	return r420_startup(rdev);
}

int r420_suspend(struct radeon_device *rdev)
{
	r100_cp_disable(rdev);
	r100_wb_disable(rdev);
	r100_irq_disable(rdev);
	if (rdev->flags & RADEON_IS_PCIE)
		rv370_pcie_gart_disable(rdev);
	if (rdev->flags & RADEON_IS_PCI)
		r100_pci_gart_disable(rdev);
	return 0;
}

void r420_fini(struct radeon_device *rdev)
{
	r100_cp_fini(rdev);
	r100_wb_fini(rdev);
	r100_ib_fini(rdev);
	radeon_gem_fini(rdev);
	if (rdev->flags & RADEON_IS_PCIE)
		rv370_pcie_gart_fini(rdev);
	if (rdev->flags & RADEON_IS_PCI)
		r100_pci_gart_fini(rdev);
	radeon_agp_fini(rdev);
	radeon_irq_kms_fini(rdev);
	radeon_fence_driver_fini(rdev);
	radeon_object_fini(rdev);
	if (rdev->is_atom_bios) {
		radeon_atombios_fini(rdev);
	} else {
		radeon_combios_fini(rdev);
	}
	kfree(rdev->bios);
	rdev->bios = NULL;
}

int r420_init(struct radeon_device *rdev)
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
		if (r) {
			return r;
		}
	} else {
		r = radeon_combios_init(rdev);
		if (r) {
			return r;
		}
	}
	
	if (radeon_gpu_reset(rdev)) {
		dev_warn(rdev->dev,
			"GPU reset failed ! (0xE40=0x%08X, 0x7C0=0x%08X)\n",
			RREG32(R_000E40_RBBM_STATUS),
			RREG32(R_0007C0_CP_STAT));
	}
	
	if (!radeon_card_posted(rdev) && rdev->bios) {
		DRM_INFO("GPU not posted. posting now...\n");
		if (rdev->is_atom_bios) {
			atom_asic_init(rdev->mode_info.atom_context);
		} else {
			radeon_combios_asic_init(rdev->ddev);
		}
	}
	
	radeon_get_clock_info(rdev->ddev);
	
	radeon_pm_init(rdev);
	
	r300_vram_info(rdev);
	
	r = r420_mc_init(rdev);
	if (r) {
		return r;
	}
	r420_debugfs(rdev);
	
	r = radeon_fence_driver_init(rdev);
	if (r) {
		return r;
	}
	r = radeon_irq_kms_init(rdev);
	if (r) {
		return r;
	}
	
	r = radeon_object_init(rdev);
	if (r) {
		return r;
	}
	if (rdev->flags & RADEON_IS_PCIE) {
		r = rv370_pcie_gart_init(rdev);
		if (r)
			return r;
	}
	if (rdev->flags & RADEON_IS_PCI) {
		r = r100_pci_gart_init(rdev);
		if (r)
			return r;
	}
	r300_set_reg_safe(rdev);
	rdev->accel_working = true;
	r = r420_startup(rdev);
	if (r) {
		
		dev_err(rdev->dev, "Disabling GPU acceleration\n");
		r420_suspend(rdev);
		r100_cp_fini(rdev);
		r100_wb_fini(rdev);
		r100_ib_fini(rdev);
		if (rdev->flags & RADEON_IS_PCIE)
			rv370_pcie_gart_fini(rdev);
		if (rdev->flags & RADEON_IS_PCI)
			r100_pci_gart_fini(rdev);
		radeon_agp_fini(rdev);
		radeon_irq_kms_fini(rdev);
		rdev->accel_working = false;
	}
	return 0;
}


#if defined(CONFIG_DEBUG_FS)
static int r420_debugfs_pipes_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct radeon_device *rdev = dev->dev_private;
	uint32_t tmp;

	tmp = RREG32(R400_GB_PIPE_SELECT);
	seq_printf(m, "GB_PIPE_SELECT 0x%08x\n", tmp);
	tmp = RREG32(R300_GB_TILE_CONFIG);
	seq_printf(m, "GB_TILE_CONFIG 0x%08x\n", tmp);
	tmp = RREG32(R300_DST_PIPE_CONFIG);
	seq_printf(m, "DST_PIPE_CONFIG 0x%08x\n", tmp);
	return 0;
}

static struct drm_info_list r420_pipes_info_list[] = {
	{"r420_pipes_info", r420_debugfs_pipes_info, 0, NULL},
};
#endif

int r420_debugfs_pipes_info_init(struct radeon_device *rdev)
{
#if defined(CONFIG_DEBUG_FS)
	return radeon_debugfs_add_files(rdev, r420_pipes_info_list, 1);
#else
	return 0;
#endif
}
