

#include <linux/acpi.h>
#include <linux/pnp.h>
#include "linux/string.h"
#include "linux/bitops.h"
#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"



#define MCHBAR_I915 0x44
#define MCHBAR_I965 0x48
#define MCHBAR_SIZE (4*4096)

#define DEVEN_REG 0x54
#define   DEVEN_MCHBAR_EN (1 << 28)


static int
intel_alloc_mchbar_resource(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int reg = IS_I965G(dev) ? MCHBAR_I965 : MCHBAR_I915;
	u32 temp_lo, temp_hi = 0;
	u64 mchbar_addr;
	int ret = 0;

	if (IS_I965G(dev))
		pci_read_config_dword(dev_priv->bridge_dev, reg + 4, &temp_hi);
	pci_read_config_dword(dev_priv->bridge_dev, reg, &temp_lo);
	mchbar_addr = ((u64)temp_hi << 32) | temp_lo;

	
#ifdef CONFIG_PNP
	if (mchbar_addr &&
	    pnp_range_reserved(mchbar_addr, mchbar_addr + MCHBAR_SIZE)) {
		ret = 0;
		goto out;
	}
#endif

	
	ret = pci_bus_alloc_resource(dev_priv->bridge_dev->bus, &dev_priv->mch_res,
				     MCHBAR_SIZE, MCHBAR_SIZE,
				     PCIBIOS_MIN_MEM,
				     0,   pcibios_align_resource,
				     dev_priv->bridge_dev);
	if (ret) {
		DRM_DEBUG("failed bus alloc: %d\n", ret);
		dev_priv->mch_res.start = 0;
		goto out;
	}

	if (IS_I965G(dev))
		pci_write_config_dword(dev_priv->bridge_dev, reg + 4,
				       upper_32_bits(dev_priv->mch_res.start));

	pci_write_config_dword(dev_priv->bridge_dev, reg,
			       lower_32_bits(dev_priv->mch_res.start));
out:
	return ret;
}


static bool
intel_setup_mchbar(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int mchbar_reg = IS_I965G(dev) ? MCHBAR_I965 : MCHBAR_I915;
	u32 temp;
	bool need_disable = false, enabled;

	if (IS_I915G(dev) || IS_I915GM(dev)) {
		pci_read_config_dword(dev_priv->bridge_dev, DEVEN_REG, &temp);
		enabled = !!(temp & DEVEN_MCHBAR_EN);
	} else {
		pci_read_config_dword(dev_priv->bridge_dev, mchbar_reg, &temp);
		enabled = temp & 1;
	}

	
	if (enabled)
		goto out;

	if (intel_alloc_mchbar_resource(dev))
		goto out;

	need_disable = true;

	
	if (IS_I915G(dev) || IS_I915GM(dev)) {
		pci_write_config_dword(dev_priv->bridge_dev, DEVEN_REG,
				       temp | DEVEN_MCHBAR_EN);
	} else {
		pci_read_config_dword(dev_priv->bridge_dev, mchbar_reg, &temp);
		pci_write_config_dword(dev_priv->bridge_dev, mchbar_reg, temp | 1);
	}
out:
	return need_disable;
}

static void
intel_teardown_mchbar(struct drm_device *dev, bool disable)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int mchbar_reg = IS_I965G(dev) ? MCHBAR_I965 : MCHBAR_I915;
	u32 temp;

	if (disable) {
		if (IS_I915G(dev) || IS_I915GM(dev)) {
			pci_read_config_dword(dev_priv->bridge_dev, DEVEN_REG, &temp);
			temp &= ~DEVEN_MCHBAR_EN;
			pci_write_config_dword(dev_priv->bridge_dev, DEVEN_REG, temp);
		} else {
			pci_read_config_dword(dev_priv->bridge_dev, mchbar_reg, &temp);
			temp &= ~1;
			pci_write_config_dword(dev_priv->bridge_dev, mchbar_reg, temp);
		}
	}

	if (dev_priv->mch_res.start)
		release_resource(&dev_priv->mch_res);
}


void
i915_gem_detect_bit_6_swizzle(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	uint32_t swizzle_x = I915_BIT_6_SWIZZLE_UNKNOWN;
	uint32_t swizzle_y = I915_BIT_6_SWIZZLE_UNKNOWN;
	bool need_disable;

	if (IS_IGDNG(dev)) {
		
		swizzle_x = I915_BIT_6_SWIZZLE_9_10;
		swizzle_y = I915_BIT_6_SWIZZLE_9;
	} else if (!IS_I9XX(dev)) {
		
		swizzle_x = I915_BIT_6_SWIZZLE_NONE;
		swizzle_y = I915_BIT_6_SWIZZLE_NONE;
	} else if (IS_MOBILE(dev)) {
		uint32_t dcc;

		
		need_disable = intel_setup_mchbar(dev);

		
		dcc = I915_READ(DCC);
		switch (dcc & DCC_ADDRESSING_MODE_MASK) {
		case DCC_ADDRESSING_MODE_SINGLE_CHANNEL:
		case DCC_ADDRESSING_MODE_DUAL_CHANNEL_ASYMMETRIC:
			swizzle_x = I915_BIT_6_SWIZZLE_NONE;
			swizzle_y = I915_BIT_6_SWIZZLE_NONE;
			break;
		case DCC_ADDRESSING_MODE_DUAL_CHANNEL_INTERLEAVED:
			if (dcc & DCC_CHANNEL_XOR_DISABLE) {
				
				swizzle_x = I915_BIT_6_SWIZZLE_9_10;
				swizzle_y = I915_BIT_6_SWIZZLE_9;
			} else if ((dcc & DCC_CHANNEL_XOR_BIT_17) == 0) {
				
				swizzle_x = I915_BIT_6_SWIZZLE_9_10_11;
				swizzle_y = I915_BIT_6_SWIZZLE_9_11;
			} else {
				
				swizzle_x = I915_BIT_6_SWIZZLE_9_10_17;
				swizzle_y = I915_BIT_6_SWIZZLE_9_17;
			}
			break;
		}
		if (dcc == 0xffffffff) {
			DRM_ERROR("Couldn't read from MCHBAR.  "
				  "Disabling tiling.\n");
			swizzle_x = I915_BIT_6_SWIZZLE_UNKNOWN;
			swizzle_y = I915_BIT_6_SWIZZLE_UNKNOWN;
		}

		intel_teardown_mchbar(dev, need_disable);
	} else {
		
		if (I915_READ16(C0DRB3) != I915_READ16(C1DRB3)) {
			swizzle_x = I915_BIT_6_SWIZZLE_NONE;
			swizzle_y = I915_BIT_6_SWIZZLE_NONE;
		} else {
			swizzle_x = I915_BIT_6_SWIZZLE_9_10;
			swizzle_y = I915_BIT_6_SWIZZLE_9;
		}
	}

	dev_priv->mm.bit_6_swizzle_x = swizzle_x;
	dev_priv->mm.bit_6_swizzle_y = swizzle_y;
}



static int
i915_get_fence_size(struct drm_device *dev, int size)
{
	int i;
	int start;

	if (IS_I965G(dev)) {
		
		return ALIGN(size, 4096);
	} else {
		
		if (IS_I9XX(dev))
			start = 1024 * 1024;
		else
			start = 512 * 1024;

		for (i = start; i < size; i <<= 1)
			;

		return i;
	}
}


static bool
i915_tiling_ok(struct drm_device *dev, int stride, int size, int tiling_mode)
{
	int tile_width;

	
	if (tiling_mode == I915_TILING_NONE)
		return true;

	if (!IS_I9XX(dev) ||
	    (tiling_mode == I915_TILING_Y && HAS_128_BYTE_Y_TILING(dev)))
		tile_width = 128;
	else
		tile_width = 512;

	
	if (IS_I965G(dev)) {
		
		if (stride / 128 > I965_FENCE_MAX_PITCH_VAL)
			return false;
	} else if (IS_I9XX(dev)) {
		uint32_t pitch_val = ffs(stride / tile_width) - 1;

		
		if (pitch_val > I915_FENCE_MAX_PITCH_VAL ||
		    size > (I830_FENCE_MAX_SIZE_VAL << 20))
			return false;
	} else {
		uint32_t pitch_val = ffs(stride / tile_width) - 1;

		if (pitch_val > I830_FENCE_MAX_PITCH_VAL ||
		    size > (I830_FENCE_MAX_SIZE_VAL << 19))
			return false;
	}

	
	if (IS_I965G(dev)) {
		if (stride & (tile_width - 1))
			return false;
		return true;
	}

	
	if (stride < tile_width)
		return false;

	if (stride & (stride - 1))
		return false;

	
	if (i915_get_fence_size(dev, size) != size)
		return false;

	return true;
}

static bool
i915_gem_object_fence_offset_ok(struct drm_gem_object *obj, int tiling_mode)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	if (obj_priv->gtt_space == NULL)
		return true;

	if (tiling_mode == I915_TILING_NONE)
		return true;

	if (!IS_I965G(dev)) {
		if (obj_priv->gtt_offset & (obj->size - 1))
			return false;
		if (IS_I9XX(dev)) {
			if (obj_priv->gtt_offset & ~I915_FENCE_START_MASK)
				return false;
		} else {
			if (obj_priv->gtt_offset & ~I830_FENCE_START_MASK)
				return false;
		}
	}

	return true;
}


int
i915_gem_set_tiling(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_i915_gem_set_tiling *args = data;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret = 0;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EINVAL;
	obj_priv = obj->driver_private;

	if (!i915_tiling_ok(dev, args->stride, obj->size, args->tiling_mode)) {
		mutex_lock(&dev->struct_mutex);
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	if (args->tiling_mode == I915_TILING_NONE) {
		args->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
		args->stride = 0;
	} else {
		if (args->tiling_mode == I915_TILING_X)
			args->swizzle_mode = dev_priv->mm.bit_6_swizzle_x;
		else
			args->swizzle_mode = dev_priv->mm.bit_6_swizzle_y;

		
		if (args->swizzle_mode == I915_BIT_6_SWIZZLE_9_17)
			args->swizzle_mode = I915_BIT_6_SWIZZLE_9;
		if (args->swizzle_mode == I915_BIT_6_SWIZZLE_9_10_17)
			args->swizzle_mode = I915_BIT_6_SWIZZLE_9_10;

		
		if (args->swizzle_mode == I915_BIT_6_SWIZZLE_UNKNOWN) {
			args->tiling_mode = I915_TILING_NONE;
			args->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
			args->stride = 0;
		}
	}

	mutex_lock(&dev->struct_mutex);
	if (args->tiling_mode != obj_priv->tiling_mode ||
	    args->stride != obj_priv->stride) {
		
		if (!i915_gem_object_fence_offset_ok(obj, args->tiling_mode))
		    ret = i915_gem_object_unbind(obj);
		else
		    ret = i915_gem_object_put_fence_reg(obj);
		if (ret != 0) {
			WARN(ret != -ERESTARTSYS,
			     "failed to reset object for tiling switch");
			args->tiling_mode = obj_priv->tiling_mode;
			args->stride = obj_priv->stride;
			goto err;
		}

		
		i915_gem_release_mmap(obj);

		obj_priv->tiling_mode = args->tiling_mode;
		obj_priv->stride = args->stride;
	}
err:
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return ret;
}


int
i915_gem_get_tiling(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_i915_gem_get_tiling *args = data;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EINVAL;
	obj_priv = obj->driver_private;

	mutex_lock(&dev->struct_mutex);

	args->tiling_mode = obj_priv->tiling_mode;
	switch (obj_priv->tiling_mode) {
	case I915_TILING_X:
		args->swizzle_mode = dev_priv->mm.bit_6_swizzle_x;
		break;
	case I915_TILING_Y:
		args->swizzle_mode = dev_priv->mm.bit_6_swizzle_y;
		break;
	case I915_TILING_NONE:
		args->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
		break;
	default:
		DRM_ERROR("unknown tiling mode\n");
	}

	
	if (args->swizzle_mode == I915_BIT_6_SWIZZLE_9_17)
		args->swizzle_mode = I915_BIT_6_SWIZZLE_9;
	if (args->swizzle_mode == I915_BIT_6_SWIZZLE_9_10_17)
		args->swizzle_mode = I915_BIT_6_SWIZZLE_9_10;

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}


static int
i915_gem_swizzle_page(struct page *page)
{
	char *vaddr;
	int i;
	char temp[64];

	vaddr = kmap(page);
	if (vaddr == NULL)
		return -ENOMEM;

	for (i = 0; i < PAGE_SIZE; i += 128) {
		memcpy(temp, &vaddr[i], 64);
		memcpy(&vaddr[i], &vaddr[i + 64], 64);
		memcpy(&vaddr[i + 64], temp, 64);
	}

	kunmap(page);

	return 0;
}

void
i915_gem_object_do_bit_17_swizzle(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int page_count = obj->size >> PAGE_SHIFT;
	int i;

	if (dev_priv->mm.bit_6_swizzle_x != I915_BIT_6_SWIZZLE_9_10_17)
		return;

	if (obj_priv->bit_17 == NULL)
		return;

	for (i = 0; i < page_count; i++) {
		char new_bit_17 = page_to_phys(obj_priv->pages[i]) >> 17;
		if ((new_bit_17 & 0x1) !=
		    (test_bit(i, obj_priv->bit_17) != 0)) {
			int ret = i915_gem_swizzle_page(obj_priv->pages[i]);
			if (ret != 0) {
				DRM_ERROR("Failed to swizzle page\n");
				return;
			}
			set_page_dirty(obj_priv->pages[i]);
		}
	}
}

void
i915_gem_object_save_bit_17_swizzle(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int page_count = obj->size >> PAGE_SHIFT;
	int i;

	if (dev_priv->mm.bit_6_swizzle_x != I915_BIT_6_SWIZZLE_9_10_17)
		return;

	if (obj_priv->bit_17 == NULL) {
		obj_priv->bit_17 = kmalloc(BITS_TO_LONGS(page_count) *
					   sizeof(long), GFP_KERNEL);
		if (obj_priv->bit_17 == NULL) {
			DRM_ERROR("Failed to allocate memory for bit 17 "
				  "record\n");
			return;
		}
	}

	for (i = 0; i < page_count; i++) {
		if (page_to_phys(obj_priv->pages[i]) & (1 << 17))
			__set_bit(i, obj_priv->bit_17);
		else
			__clear_bit(i, obj_priv->bit_17);
	}
}
