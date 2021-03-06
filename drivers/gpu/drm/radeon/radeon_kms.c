
#include "drmP.h"
#include "drm_sarea.h"
#include "radeon.h"
#include "radeon_drm.h"



int radeon_driver_load_kms(struct drm_device *dev, unsigned long flags)
{
	struct radeon_device *rdev;
	int r;

	rdev = kzalloc(sizeof(struct radeon_device), GFP_KERNEL);
	if (rdev == NULL) {
		return -ENOMEM;
	}
	dev->dev_private = (void *)rdev;

	
	if (drm_device_is_agp(dev)) {
		flags |= RADEON_IS_AGP;
	} else if (drm_device_is_pcie(dev)) {
		flags |= RADEON_IS_PCIE;
	} else {
		flags |= RADEON_IS_PCI;
	}

	
	r = radeon_device_init(rdev, dev, dev->pdev, flags);
	if (r) {
		DRM_ERROR("Fatal error while trying to initialize radeon.\n");
		return r;
	}
	
	r = radeon_modeset_init(rdev);
	if (r) {
		return r;
	}
	return 0;
}

int radeon_driver_unload_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;

	if (rdev == NULL)
		return 0;
	radeon_modeset_fini(rdev);
	radeon_device_fini(rdev);
	kfree(rdev);
	dev->dev_private = NULL;
	return 0;
}



int radeon_info_ioctl(struct drm_device *dev, void *data, struct drm_file *filp)
{
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_info *info;
	uint32_t *value_ptr;
	uint32_t value;

	info = data;
	value_ptr = (uint32_t *)((unsigned long)info->value);
	switch (info->request) {
	case RADEON_INFO_DEVICE_ID:
		value = dev->pci_device;
		break;
	case RADEON_INFO_NUM_GB_PIPES:
		value = rdev->num_gb_pipes;
		break;
	case RADEON_INFO_NUM_Z_PIPES:
		value = rdev->num_z_pipes;
		break;
	case RADEON_INFO_ACCEL_WORKING:
		value = rdev->accel_working;
		break;
	default:
		DRM_DEBUG("Invalid request %d\n", info->request);
		return -EINVAL;
	}
	if (DRM_COPY_TO_USER(value_ptr, &value, sizeof(uint32_t))) {
		DRM_ERROR("copy_to_user\n");
		return -EFAULT;
	}
	return 0;
}



int radeon_driver_firstopen_kms(struct drm_device *dev)
{
	return 0;
}


void radeon_driver_lastclose_kms(struct drm_device *dev)
{
}

int radeon_driver_open_kms(struct drm_device *dev, struct drm_file *file_priv)
{
	return 0;
}

void radeon_driver_postclose_kms(struct drm_device *dev,
				 struct drm_file *file_priv)
{
}

void radeon_driver_preclose_kms(struct drm_device *dev,
				struct drm_file *file_priv)
{
}



u32 radeon_get_vblank_counter_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc > 1) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	return radeon_get_vblank_counter(rdev, crtc);
}

int radeon_enable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc > 1) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	rdev->irq.crtc_vblank_int[crtc] = true;

	return radeon_irq_set(rdev);
}

void radeon_disable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc > 1) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return;
	}

	rdev->irq.crtc_vblank_int[crtc] = false;

	radeon_irq_set(rdev);
}



int radeon_dma_ioctl_kms(struct drm_device *dev, void *data,
			 struct drm_file *file_priv)
{
	
	return -EINVAL;
}

#define KMS_INVALID_IOCTL(name)						\
int name(struct drm_device *dev, void *data, struct drm_file *file_priv)\
{									\
	DRM_ERROR("invalid ioctl with kms %s\n", __func__);		\
	return -EINVAL;							\
}


KMS_INVALID_IOCTL(radeon_cp_init_kms)
KMS_INVALID_IOCTL(radeon_cp_start_kms)
KMS_INVALID_IOCTL(radeon_cp_stop_kms)
KMS_INVALID_IOCTL(radeon_cp_reset_kms)
KMS_INVALID_IOCTL(radeon_cp_idle_kms)
KMS_INVALID_IOCTL(radeon_cp_resume_kms)
KMS_INVALID_IOCTL(radeon_engine_reset_kms)
KMS_INVALID_IOCTL(radeon_fullscreen_kms)
KMS_INVALID_IOCTL(radeon_cp_swap_kms)
KMS_INVALID_IOCTL(radeon_cp_clear_kms)
KMS_INVALID_IOCTL(radeon_cp_vertex_kms)
KMS_INVALID_IOCTL(radeon_cp_indices_kms)
KMS_INVALID_IOCTL(radeon_cp_texture_kms)
KMS_INVALID_IOCTL(radeon_cp_stipple_kms)
KMS_INVALID_IOCTL(radeon_cp_indirect_kms)
KMS_INVALID_IOCTL(radeon_cp_vertex2_kms)
KMS_INVALID_IOCTL(radeon_cp_cmdbuf_kms)
KMS_INVALID_IOCTL(radeon_cp_getparam_kms)
KMS_INVALID_IOCTL(radeon_cp_flip_kms)
KMS_INVALID_IOCTL(radeon_mem_alloc_kms)
KMS_INVALID_IOCTL(radeon_mem_free_kms)
KMS_INVALID_IOCTL(radeon_mem_init_heap_kms)
KMS_INVALID_IOCTL(radeon_irq_emit_kms)
KMS_INVALID_IOCTL(radeon_irq_wait_kms)
KMS_INVALID_IOCTL(radeon_cp_setparam_kms)
KMS_INVALID_IOCTL(radeon_surface_alloc_kms)
KMS_INVALID_IOCTL(radeon_surface_free_kms)


struct drm_ioctl_desc radeon_ioctls_kms[] = {
	DRM_IOCTL_DEF(DRM_RADEON_CP_INIT, radeon_cp_init_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_CP_START, radeon_cp_start_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_CP_STOP, radeon_cp_stop_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_CP_RESET, radeon_cp_reset_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_CP_IDLE, radeon_cp_idle_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_CP_RESUME, radeon_cp_resume_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_RESET, radeon_engine_reset_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_FULLSCREEN, radeon_fullscreen_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_SWAP, radeon_cp_swap_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_CLEAR, radeon_cp_clear_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_VERTEX, radeon_cp_vertex_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_INDICES, radeon_cp_indices_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_TEXTURE, radeon_cp_texture_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_STIPPLE, radeon_cp_stipple_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_INDIRECT, radeon_cp_indirect_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_VERTEX2, radeon_cp_vertex2_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_CMDBUF, radeon_cp_cmdbuf_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GETPARAM, radeon_cp_getparam_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_FLIP, radeon_cp_flip_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_ALLOC, radeon_mem_alloc_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_FREE, radeon_mem_free_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_INIT_HEAP, radeon_mem_init_heap_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_RADEON_IRQ_EMIT, radeon_irq_emit_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_IRQ_WAIT, radeon_irq_wait_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_SETPARAM, radeon_cp_setparam_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_SURF_ALLOC, radeon_surface_alloc_kms, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_SURF_FREE, radeon_surface_free_kms, DRM_AUTH),
	
	DRM_IOCTL_DEF(DRM_RADEON_GEM_INFO, radeon_gem_info_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_CREATE, radeon_gem_create_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_MMAP, radeon_gem_mmap_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_SET_DOMAIN, radeon_gem_set_domain_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_PREAD, radeon_gem_pread_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_PWRITE, radeon_gem_pwrite_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_WAIT_IDLE, radeon_gem_wait_idle_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_CS, radeon_cs_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_INFO, radeon_info_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_SET_TILING, radeon_gem_set_tiling_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_GET_TILING, radeon_gem_get_tiling_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_RADEON_GEM_BUSY, radeon_gem_busy_ioctl, DRM_AUTH),
};
int radeon_max_kms_ioctl = DRM_ARRAY_SIZE(radeon_ioctls_kms);
