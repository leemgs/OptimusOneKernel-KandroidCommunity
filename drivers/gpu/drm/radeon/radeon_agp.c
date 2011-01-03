
#include "drmP.h"
#include "drm.h"
#include "radeon.h"
#include "radeon_drm.h"

#if __OS_HAS_AGP

struct radeon_agpmode_quirk {
	u32 hostbridge_vendor;
	u32 hostbridge_device;
	u32 chip_vendor;
	u32 chip_device;
	u32 subsys_vendor;
	u32 subsys_device;
	u32 default_mode;
};

static struct radeon_agpmode_quirk radeon_agpmode_quirk_list[] = {
	
	{ PCI_VENDOR_ID_INTEL, 0x2550, PCI_VENDOR_ID_ATI, 0x4152, 0x1458, 0x4038, 4},
	
	{ PCI_VENDOR_ID_INTEL, 0x2570, PCI_VENDOR_ID_ATI, 0x4a4e, PCI_VENDOR_ID_DELL, 0x5106, 4},
	
	{ PCI_VENDOR_ID_INTEL, 0x2570, PCI_VENDOR_ID_ATI, 0x5964,
		0x148c, 0x2073, 4},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x4c59,
		PCI_VENDOR_ID_IBM, 0x052f, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x4e50,
		PCI_VENDOR_ID_IBM, 0x0550, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x4c57,
		PCI_VENDOR_ID_IBM, 0x0530, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x4e54,
		PCI_VENDOR_ID_IBM, 0x054f, 2},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x5c61,
		PCI_VENDOR_ID_SONY, 0x816b, 2},
	
	{ PCI_VENDOR_ID_INTEL, 0x3340, PCI_VENDOR_ID_ATI, 0x5c61,
		PCI_VENDOR_ID_SONY, 0x8195, 8},
	
	{ PCI_VENDOR_ID_INTEL, 0x3575, PCI_VENDOR_ID_ATI, 0x4c59,
		PCI_VENDOR_ID_DELL, 0x00e3, 2},
	
	{ PCI_VENDOR_ID_INTEL, 0x3580, PCI_VENDOR_ID_ATI, 0x4c66,
		PCI_VENDOR_ID_DELL, 0x0149, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3580, PCI_VENDOR_ID_ATI, 0x4e50,
		0x1025, 0x0061, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3580, PCI_VENDOR_ID_ATI, 0x4e50,
		0x1025, 0x0064, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3580, PCI_VENDOR_ID_ATI, 0x4e50,
		PCI_VENDOR_ID_ASUSTEK, 0x1942, 1},
	
	{ PCI_VENDOR_ID_INTEL, 0x3580, PCI_VENDOR_ID_ATI, 0x4e50,
		0x10cf, 0x127f, 1},
	
	{ 0x1849, 0x3189, PCI_VENDOR_ID_ATI, 0x5960,
		0x1787, 0x5960, 4},
	
	{ PCI_VENDOR_ID_VIA, 0x0204, PCI_VENDOR_ID_ATI, 0x5960,
		0x17af, 0x2020, 4},
	
	{ PCI_VENDOR_ID_VIA, 0x0269, PCI_VENDOR_ID_ATI, 0x4153,
		PCI_VENDOR_ID_ASUSTEK, 0x003c, 4},
	
	{ PCI_VENDOR_ID_VIA, 0x0305, PCI_VENDOR_ID_ATI, 0x514c,
		PCI_VENDOR_ID_ATI, 0x013a, 2},
	
	{ PCI_VENDOR_ID_VIA, 0x0691, PCI_VENDOR_ID_ATI, 0x5960,
		PCI_VENDOR_ID_ASUSTEK, 0x004c, 2},
	
	{ PCI_VENDOR_ID_VIA, 0x0691, PCI_VENDOR_ID_ATI, 0x5960,
		PCI_VENDOR_ID_ASUSTEK, 0x0054, 2},
	
	{ PCI_VENDOR_ID_VIA, 0x3189, PCI_VENDOR_ID_ATI, 0x514d,
		0x174b, 0x7149, 4},
	
	{ PCI_VENDOR_ID_VIA, 0x3189, PCI_VENDOR_ID_ATI, 0x5960,
		0x1462, 0x0380, 4},
	
	{ PCI_VENDOR_ID_VIA, 0x3189, PCI_VENDOR_ID_ATI, 0x5964,
		0x148c, 0x2073, 4},
	
	{ PCI_VENDOR_ID_ATI, 0xcbb2, PCI_VENDOR_ID_ATI, 0x5c61,
		PCI_VENDOR_ID_SONY, 0x8175, 1},
	
	{ PCI_VENDOR_ID_HP, 0x122e, PCI_VENDOR_ID_ATI, 0x4e47,
		PCI_VENDOR_ID_ATI, 0x0152, 2},
	{ 0, 0, 0, 0, 0, 0, 0 },
};
#endif

int radeon_agp_init(struct radeon_device *rdev)
{
#if __OS_HAS_AGP
	struct radeon_agpmode_quirk *p = radeon_agpmode_quirk_list;
	struct drm_agp_mode mode;
	struct drm_agp_info info;
	uint32_t agp_status;
	int default_mode;
	bool is_v3;
	int ret;

	
	if (!rdev->ddev->agp->acquired) {
		ret = drm_agp_acquire(rdev->ddev);
		if (ret) {
			DRM_ERROR("Unable to acquire AGP: %d\n", ret);
			return ret;
		}
	}

	ret = drm_agp_info(rdev->ddev, &info);
	if (ret) {
		DRM_ERROR("Unable to get AGP info: %d\n", ret);
		return ret;
	}
	mode.mode = info.mode;
	agp_status = (RREG32(RADEON_AGP_STATUS) | RADEON_AGPv3_MODE) & mode.mode;
	is_v3 = !!(agp_status & RADEON_AGPv3_MODE);

	if (is_v3) {
		default_mode = (agp_status & RADEON_AGPv3_8X_MODE) ? 8 : 4;
	} else {
		if (agp_status & RADEON_AGP_4X_MODE) {
			default_mode = 4;
		} else if (agp_status & RADEON_AGP_2X_MODE) {
			default_mode = 2;
		} else {
			default_mode = 1;
		}
	}

	
	while (p && p->chip_device != 0) {
		if (info.id_vendor == p->hostbridge_vendor &&
		    info.id_device == p->hostbridge_device &&
		    rdev->pdev->vendor == p->chip_vendor &&
		    rdev->pdev->device == p->chip_device &&
		    rdev->pdev->subsystem_vendor == p->subsys_vendor &&
		    rdev->pdev->subsystem_device == p->subsys_device) {
			default_mode = p->default_mode;
		}
		++p;
	}

	if (radeon_agpmode > 0) {
		if ((radeon_agpmode < (is_v3 ? 4 : 1)) ||
		    (radeon_agpmode > (is_v3 ? 8 : 4)) ||
		    (radeon_agpmode & (radeon_agpmode - 1))) {
			DRM_ERROR("Illegal AGP Mode: %d (valid %s), leaving at %d\n",
				  radeon_agpmode, is_v3 ? "4, 8" : "1, 2, 4",
				  default_mode);
			radeon_agpmode = default_mode;
		} else {
			DRM_INFO("AGP mode requested: %d\n", radeon_agpmode);
		}
	} else {
		radeon_agpmode = default_mode;
	}

	mode.mode &= ~RADEON_AGP_MODE_MASK;
	if (is_v3) {
		switch (radeon_agpmode) {
		case 8:
			mode.mode |= RADEON_AGPv3_8X_MODE;
			break;
		case 4:
		default:
			mode.mode |= RADEON_AGPv3_4X_MODE;
			break;
		}
	} else {
		switch (radeon_agpmode) {
		case 4:
			mode.mode |= RADEON_AGP_4X_MODE;
			break;
		case 2:
			mode.mode |= RADEON_AGP_2X_MODE;
			break;
		case 1:
		default:
			mode.mode |= RADEON_AGP_1X_MODE;
			break;
		}
	}

	mode.mode &= ~RADEON_AGP_FW_MODE; 
	ret = drm_agp_enable(rdev->ddev, mode);
	if (ret) {
		DRM_ERROR("Unable to enable AGP (mode = 0x%lx)\n", mode.mode);
		return ret;
	}

	rdev->mc.agp_base = rdev->ddev->agp->agp_info.aper_base;
	rdev->mc.gtt_size = rdev->ddev->agp->agp_info.aper_size << 20;

	
	if (rdev->family < CHIP_R200) {
		WREG32(RADEON_AGP_CNTL, RREG32(RADEON_AGP_CNTL) | 0x000e0000);
	}
	return 0;
#else
	return 0;
#endif
}

void radeon_agp_resume(struct radeon_device *rdev)
{
#if __OS_HAS_AGP
	int r;
	if (rdev->flags & RADEON_IS_AGP) {
		r = radeon_agp_init(rdev);
		if (r)
			dev_warn(rdev->dev, "radeon AGP reinit failed\n");
	}
#endif
}

void radeon_agp_fini(struct radeon_device *rdev)
{
#if __OS_HAS_AGP
	if (rdev->flags & RADEON_IS_AGP) {
		if (rdev->ddev->agp && rdev->ddev->agp->acquired) {
			drm_agp_release(rdev->ddev);
		}
	}
#endif
}
