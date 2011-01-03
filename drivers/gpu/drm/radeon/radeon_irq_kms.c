
#include "drmP.h"
#include "radeon_drm.h"
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"

irqreturn_t radeon_driver_irq_handler_kms(DRM_IRQ_ARGS)
{
	struct drm_device *dev = (struct drm_device *) arg;
	struct radeon_device *rdev = dev->dev_private;

	return radeon_irq_process(rdev);
}

void radeon_driver_irq_preinstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned i;

	
	rdev->irq.sw_int = false;
	for (i = 0; i < 2; i++) {
		rdev->irq.crtc_vblank_int[i] = false;
	}
	radeon_irq_set(rdev);
	
	radeon_irq_process(rdev);
}

int radeon_driver_irq_postinstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;

	dev->max_vblank_count = 0x001fffff;
	rdev->irq.sw_int = true;
	radeon_irq_set(rdev);
	return 0;
}

void radeon_driver_irq_uninstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned i;

	if (rdev == NULL) {
		return;
	}
	
	rdev->irq.sw_int = false;
	for (i = 0; i < 2; i++) {
		rdev->irq.crtc_vblank_int[i] = false;
	}
	radeon_irq_set(rdev);
}

int radeon_irq_kms_init(struct radeon_device *rdev)
{
	int r = 0;
	int num_crtc = 2;

	if (rdev->flags & RADEON_SINGLE_CRTC)
		num_crtc = 1;

	r = drm_vblank_init(rdev->ddev, num_crtc);
	if (r) {
		return r;
	}
	
	rdev->msi_enabled = 0;
	if (rdev->family >= CHIP_RV380) {
		int ret = pci_enable_msi(rdev->pdev);
		if (!ret)
			rdev->msi_enabled = 1;
	}
	drm_irq_install(rdev->ddev);
	rdev->irq.installed = true;
	DRM_INFO("radeon: irq initialized.\n");
	return 0;
}

void radeon_irq_kms_fini(struct radeon_device *rdev)
{
	if (rdev->irq.installed) {
		rdev->irq.installed = false;
		drm_irq_uninstall(rdev->ddev);
		if (rdev->msi_enabled)
			pci_disable_msi(rdev->pdev);
	}
}
