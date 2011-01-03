

#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "i915_trace.h"
#include "intel_drv.h"
#include <linux/swap.h>
#include <linux/pci.h>

#define I915_GEM_GPU_DOMAINS	(~(I915_GEM_DOMAIN_CPU | I915_GEM_DOMAIN_GTT))

static void i915_gem_object_flush_gpu_write_domain(struct drm_gem_object *obj);
static void i915_gem_object_flush_gtt_write_domain(struct drm_gem_object *obj);
static void i915_gem_object_flush_cpu_write_domain(struct drm_gem_object *obj);
static int i915_gem_object_set_to_cpu_domain(struct drm_gem_object *obj,
					     int write);
static int i915_gem_object_set_cpu_read_domain_range(struct drm_gem_object *obj,
						     uint64_t offset,
						     uint64_t size);
static void i915_gem_object_set_to_full_cpu_read_domain(struct drm_gem_object *obj);
static int i915_gem_object_wait_rendering(struct drm_gem_object *obj);
static int i915_gem_object_bind_to_gtt(struct drm_gem_object *obj,
					   unsigned alignment);
static void i915_gem_clear_fence_reg(struct drm_gem_object *obj);
static int i915_gem_evict_something(struct drm_device *dev, int min_size);
static int i915_gem_evict_from_inactive_list(struct drm_device *dev);
static int i915_gem_phys_pwrite(struct drm_device *dev, struct drm_gem_object *obj,
				struct drm_i915_gem_pwrite *args,
				struct drm_file *file_priv);

static LIST_HEAD(shrink_list);
static DEFINE_SPINLOCK(shrink_list_lock);

int i915_gem_do_init(struct drm_device *dev, unsigned long start,
		     unsigned long end)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	if (start >= end ||
	    (start & (PAGE_SIZE - 1)) != 0 ||
	    (end & (PAGE_SIZE - 1)) != 0) {
		return -EINVAL;
	}

	drm_mm_init(&dev_priv->mm.gtt_space, start,
		    end - start);

	dev->gtt_total = (uint32_t) (end - start);

	return 0;
}

int
i915_gem_init_ioctl(struct drm_device *dev, void *data,
		    struct drm_file *file_priv)
{
	struct drm_i915_gem_init *args = data;
	int ret;

	mutex_lock(&dev->struct_mutex);
	ret = i915_gem_do_init(dev, args->gtt_start, args->gtt_end);
	mutex_unlock(&dev->struct_mutex);

	return ret;
}

int
i915_gem_get_aperture_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct drm_i915_gem_get_aperture *args = data;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	args->aper_size = dev->gtt_total;
	args->aper_available_size = (args->aper_size -
				     atomic_read(&dev->pin_memory));

	return 0;
}



int
i915_gem_create_ioctl(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct drm_i915_gem_create *args = data;
	struct drm_gem_object *obj;
	int ret;
	u32 handle;

	args->size = roundup(args->size, PAGE_SIZE);

	
	obj = drm_gem_object_alloc(dev, args->size);
	if (obj == NULL)
		return -ENOMEM;

	ret = drm_gem_handle_create(file_priv, obj, &handle);
	mutex_lock(&dev->struct_mutex);
	drm_gem_object_handle_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	if (ret)
		return ret;

	args->handle = handle;

	return 0;
}

static inline int
fast_shmem_read(struct page **pages,
		loff_t page_base, int page_offset,
		char __user *data,
		int length)
{
	char __iomem *vaddr;
	int unwritten;

	vaddr = kmap_atomic(pages[page_base >> PAGE_SHIFT], KM_USER0);
	if (vaddr == NULL)
		return -ENOMEM;
	unwritten = __copy_to_user_inatomic(data, vaddr + page_offset, length);
	kunmap_atomic(vaddr, KM_USER0);

	if (unwritten)
		return -EFAULT;

	return 0;
}

static int i915_gem_object_needs_bit17_swizzle(struct drm_gem_object *obj)
{
	drm_i915_private_t *dev_priv = obj->dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	return dev_priv->mm.bit_6_swizzle_x == I915_BIT_6_SWIZZLE_9_10_17 &&
		obj_priv->tiling_mode != I915_TILING_NONE;
}

static inline int
slow_shmem_copy(struct page *dst_page,
		int dst_offset,
		struct page *src_page,
		int src_offset,
		int length)
{
	char *dst_vaddr, *src_vaddr;

	dst_vaddr = kmap_atomic(dst_page, KM_USER0);
	if (dst_vaddr == NULL)
		return -ENOMEM;

	src_vaddr = kmap_atomic(src_page, KM_USER1);
	if (src_vaddr == NULL) {
		kunmap_atomic(dst_vaddr, KM_USER0);
		return -ENOMEM;
	}

	memcpy(dst_vaddr + dst_offset, src_vaddr + src_offset, length);

	kunmap_atomic(src_vaddr, KM_USER1);
	kunmap_atomic(dst_vaddr, KM_USER0);

	return 0;
}

static inline int
slow_shmem_bit17_copy(struct page *gpu_page,
		      int gpu_offset,
		      struct page *cpu_page,
		      int cpu_offset,
		      int length,
		      int is_read)
{
	char *gpu_vaddr, *cpu_vaddr;

	
	if ((page_to_phys(gpu_page) & (1 << 17)) == 0) {
		if (is_read)
			return slow_shmem_copy(cpu_page, cpu_offset,
					       gpu_page, gpu_offset, length);
		else
			return slow_shmem_copy(gpu_page, gpu_offset,
					       cpu_page, cpu_offset, length);
	}

	gpu_vaddr = kmap_atomic(gpu_page, KM_USER0);
	if (gpu_vaddr == NULL)
		return -ENOMEM;

	cpu_vaddr = kmap_atomic(cpu_page, KM_USER1);
	if (cpu_vaddr == NULL) {
		kunmap_atomic(gpu_vaddr, KM_USER0);
		return -ENOMEM;
	}

	
	while (length > 0) {
		int cacheline_end = ALIGN(gpu_offset + 1, 64);
		int this_length = min(cacheline_end - gpu_offset, length);
		int swizzled_gpu_offset = gpu_offset ^ 64;

		if (is_read) {
			memcpy(cpu_vaddr + cpu_offset,
			       gpu_vaddr + swizzled_gpu_offset,
			       this_length);
		} else {
			memcpy(gpu_vaddr + swizzled_gpu_offset,
			       cpu_vaddr + cpu_offset,
			       this_length);
		}
		cpu_offset += this_length;
		gpu_offset += this_length;
		length -= this_length;
	}

	kunmap_atomic(cpu_vaddr, KM_USER1);
	kunmap_atomic(gpu_vaddr, KM_USER0);

	return 0;
}


static int
i915_gem_shmem_pread_fast(struct drm_device *dev, struct drm_gem_object *obj,
			  struct drm_i915_gem_pread *args,
			  struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	ssize_t remain;
	loff_t offset, page_base;
	char __user *user_data;
	int page_offset, page_length;
	int ret;

	user_data = (char __user *) (uintptr_t) args->data_ptr;
	remain = args->size;

	mutex_lock(&dev->struct_mutex);

	ret = i915_gem_object_get_pages(obj, 0);
	if (ret != 0)
		goto fail_unlock;

	ret = i915_gem_object_set_cpu_read_domain_range(obj, args->offset,
							args->size);
	if (ret != 0)
		goto fail_put_pages;

	obj_priv = obj->driver_private;
	offset = args->offset;

	while (remain > 0) {
		
		page_base = (offset & ~(PAGE_SIZE-1));
		page_offset = offset & (PAGE_SIZE-1);
		page_length = remain;
		if ((page_offset + remain) > PAGE_SIZE)
			page_length = PAGE_SIZE - page_offset;

		ret = fast_shmem_read(obj_priv->pages,
				      page_base, page_offset,
				      user_data, page_length);
		if (ret)
			goto fail_put_pages;

		remain -= page_length;
		user_data += page_length;
		offset += page_length;
	}

fail_put_pages:
	i915_gem_object_put_pages(obj);
fail_unlock:
	mutex_unlock(&dev->struct_mutex);

	return ret;
}

static int
i915_gem_object_get_pages_or_evict(struct drm_gem_object *obj)
{
	int ret;

	ret = i915_gem_object_get_pages(obj, __GFP_NORETRY | __GFP_NOWARN);

	
	if (ret == -ENOMEM) {
		struct drm_device *dev = obj->dev;

		ret = i915_gem_evict_something(dev, obj->size);
		if (ret)
			return ret;

		ret = i915_gem_object_get_pages(obj, 0);
	}

	return ret;
}


static int
i915_gem_shmem_pread_slow(struct drm_device *dev, struct drm_gem_object *obj,
			  struct drm_i915_gem_pread *args,
			  struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct mm_struct *mm = current->mm;
	struct page **user_pages;
	ssize_t remain;
	loff_t offset, pinned_pages, i;
	loff_t first_data_page, last_data_page, num_pages;
	int shmem_page_index, shmem_page_offset;
	int data_page_index,  data_page_offset;
	int page_length;
	int ret;
	uint64_t data_ptr = args->data_ptr;
	int do_bit17_swizzling;

	remain = args->size;

	
	first_data_page = data_ptr / PAGE_SIZE;
	last_data_page = (data_ptr + args->size - 1) / PAGE_SIZE;
	num_pages = last_data_page - first_data_page + 1;

	user_pages = drm_calloc_large(num_pages, sizeof(struct page *));
	if (user_pages == NULL)
		return -ENOMEM;

	down_read(&mm->mmap_sem);
	pinned_pages = get_user_pages(current, mm, (uintptr_t)args->data_ptr,
				      num_pages, 1, 0, user_pages, NULL);
	up_read(&mm->mmap_sem);
	if (pinned_pages < num_pages) {
		ret = -EFAULT;
		goto fail_put_user_pages;
	}

	do_bit17_swizzling = i915_gem_object_needs_bit17_swizzle(obj);

	mutex_lock(&dev->struct_mutex);

	ret = i915_gem_object_get_pages_or_evict(obj);
	if (ret)
		goto fail_unlock;

	ret = i915_gem_object_set_cpu_read_domain_range(obj, args->offset,
							args->size);
	if (ret != 0)
		goto fail_put_pages;

	obj_priv = obj->driver_private;
	offset = args->offset;

	while (remain > 0) {
		
		shmem_page_index = offset / PAGE_SIZE;
		shmem_page_offset = offset & ~PAGE_MASK;
		data_page_index = data_ptr / PAGE_SIZE - first_data_page;
		data_page_offset = data_ptr & ~PAGE_MASK;

		page_length = remain;
		if ((shmem_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - shmem_page_offset;
		if ((data_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - data_page_offset;

		if (do_bit17_swizzling) {
			ret = slow_shmem_bit17_copy(obj_priv->pages[shmem_page_index],
						    shmem_page_offset,
						    user_pages[data_page_index],
						    data_page_offset,
						    page_length,
						    1);
		} else {
			ret = slow_shmem_copy(user_pages[data_page_index],
					      data_page_offset,
					      obj_priv->pages[shmem_page_index],
					      shmem_page_offset,
					      page_length);
		}
		if (ret)
			goto fail_put_pages;

		remain -= page_length;
		data_ptr += page_length;
		offset += page_length;
	}

fail_put_pages:
	i915_gem_object_put_pages(obj);
fail_unlock:
	mutex_unlock(&dev->struct_mutex);
fail_put_user_pages:
	for (i = 0; i < pinned_pages; i++) {
		SetPageDirty(user_pages[i]);
		page_cache_release(user_pages[i]);
	}
	drm_free_large(user_pages);

	return ret;
}


int
i915_gem_pread_ioctl(struct drm_device *dev, void *data,
		     struct drm_file *file_priv)
{
	struct drm_i915_gem_pread *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;
	obj_priv = obj->driver_private;

	
	if (args->offset > obj->size || args->size > obj->size ||
	    args->offset + args->size > obj->size) {
		drm_gem_object_unreference(obj);
		return -EINVAL;
	}

	if (i915_gem_object_needs_bit17_swizzle(obj)) {
		ret = i915_gem_shmem_pread_slow(dev, obj, args, file_priv);
	} else {
		ret = i915_gem_shmem_pread_fast(dev, obj, args, file_priv);
		if (ret != 0)
			ret = i915_gem_shmem_pread_slow(dev, obj, args,
							file_priv);
	}

	drm_gem_object_unreference(obj);

	return ret;
}



static inline int
fast_user_write(struct io_mapping *mapping,
		loff_t page_base, int page_offset,
		char __user *user_data,
		int length)
{
	char *vaddr_atomic;
	unsigned long unwritten;

	vaddr_atomic = io_mapping_map_atomic_wc(mapping, page_base);
	unwritten = __copy_from_user_inatomic_nocache(vaddr_atomic + page_offset,
						      user_data, length);
	io_mapping_unmap_atomic(vaddr_atomic);
	if (unwritten)
		return -EFAULT;
	return 0;
}



static inline int
slow_kernel_write(struct io_mapping *mapping,
		  loff_t gtt_base, int gtt_offset,
		  struct page *user_page, int user_offset,
		  int length)
{
	char *src_vaddr, *dst_vaddr;
	unsigned long unwritten;

	dst_vaddr = io_mapping_map_atomic_wc(mapping, gtt_base);
	src_vaddr = kmap_atomic(user_page, KM_USER1);
	unwritten = __copy_from_user_inatomic_nocache(dst_vaddr + gtt_offset,
						      src_vaddr + user_offset,
						      length);
	kunmap_atomic(src_vaddr, KM_USER1);
	io_mapping_unmap_atomic(dst_vaddr);
	if (unwritten)
		return -EFAULT;
	return 0;
}

static inline int
fast_shmem_write(struct page **pages,
		 loff_t page_base, int page_offset,
		 char __user *data,
		 int length)
{
	char __iomem *vaddr;
	unsigned long unwritten;

	vaddr = kmap_atomic(pages[page_base >> PAGE_SHIFT], KM_USER0);
	if (vaddr == NULL)
		return -ENOMEM;
	unwritten = __copy_from_user_inatomic(vaddr + page_offset, data, length);
	kunmap_atomic(vaddr, KM_USER0);

	if (unwritten)
		return -EFAULT;
	return 0;
}


static int
i915_gem_gtt_pwrite_fast(struct drm_device *dev, struct drm_gem_object *obj,
			 struct drm_i915_gem_pwrite *args,
			 struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	drm_i915_private_t *dev_priv = dev->dev_private;
	ssize_t remain;
	loff_t offset, page_base;
	char __user *user_data;
	int page_offset, page_length;
	int ret;

	user_data = (char __user *) (uintptr_t) args->data_ptr;
	remain = args->size;
	if (!access_ok(VERIFY_READ, user_data, remain))
		return -EFAULT;


	mutex_lock(&dev->struct_mutex);
	ret = i915_gem_object_pin(obj, 0);
	if (ret) {
		mutex_unlock(&dev->struct_mutex);
		return ret;
	}
	ret = i915_gem_object_set_to_gtt_domain(obj, 1);
	if (ret)
		goto fail;

	obj_priv = obj->driver_private;
	offset = obj_priv->gtt_offset + args->offset;

	while (remain > 0) {
		
		page_base = (offset & ~(PAGE_SIZE-1));
		page_offset = offset & (PAGE_SIZE-1);
		page_length = remain;
		if ((page_offset + remain) > PAGE_SIZE)
			page_length = PAGE_SIZE - page_offset;

		ret = fast_user_write (dev_priv->mm.gtt_mapping, page_base,
				       page_offset, user_data, page_length);

		
		if (ret)
			goto fail;

		remain -= page_length;
		user_data += page_length;
		offset += page_length;
	}

fail:
	i915_gem_object_unpin(obj);
	mutex_unlock(&dev->struct_mutex);

	return ret;
}


static int
i915_gem_gtt_pwrite_slow(struct drm_device *dev, struct drm_gem_object *obj,
			 struct drm_i915_gem_pwrite *args,
			 struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	drm_i915_private_t *dev_priv = dev->dev_private;
	ssize_t remain;
	loff_t gtt_page_base, offset;
	loff_t first_data_page, last_data_page, num_pages;
	loff_t pinned_pages, i;
	struct page **user_pages;
	struct mm_struct *mm = current->mm;
	int gtt_page_offset, data_page_offset, data_page_index, page_length;
	int ret;
	uint64_t data_ptr = args->data_ptr;

	remain = args->size;

	
	first_data_page = data_ptr / PAGE_SIZE;
	last_data_page = (data_ptr + args->size - 1) / PAGE_SIZE;
	num_pages = last_data_page - first_data_page + 1;

	user_pages = drm_calloc_large(num_pages, sizeof(struct page *));
	if (user_pages == NULL)
		return -ENOMEM;

	down_read(&mm->mmap_sem);
	pinned_pages = get_user_pages(current, mm, (uintptr_t)args->data_ptr,
				      num_pages, 0, 0, user_pages, NULL);
	up_read(&mm->mmap_sem);
	if (pinned_pages < num_pages) {
		ret = -EFAULT;
		goto out_unpin_pages;
	}

	mutex_lock(&dev->struct_mutex);
	ret = i915_gem_object_pin(obj, 0);
	if (ret)
		goto out_unlock;

	ret = i915_gem_object_set_to_gtt_domain(obj, 1);
	if (ret)
		goto out_unpin_object;

	obj_priv = obj->driver_private;
	offset = obj_priv->gtt_offset + args->offset;

	while (remain > 0) {
		
		gtt_page_base = offset & PAGE_MASK;
		gtt_page_offset = offset & ~PAGE_MASK;
		data_page_index = data_ptr / PAGE_SIZE - first_data_page;
		data_page_offset = data_ptr & ~PAGE_MASK;

		page_length = remain;
		if ((gtt_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - gtt_page_offset;
		if ((data_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - data_page_offset;

		ret = slow_kernel_write(dev_priv->mm.gtt_mapping,
					gtt_page_base, gtt_page_offset,
					user_pages[data_page_index],
					data_page_offset,
					page_length);

		
		if (ret)
			goto out_unpin_object;

		remain -= page_length;
		offset += page_length;
		data_ptr += page_length;
	}

out_unpin_object:
	i915_gem_object_unpin(obj);
out_unlock:
	mutex_unlock(&dev->struct_mutex);
out_unpin_pages:
	for (i = 0; i < pinned_pages; i++)
		page_cache_release(user_pages[i]);
	drm_free_large(user_pages);

	return ret;
}


static int
i915_gem_shmem_pwrite_fast(struct drm_device *dev, struct drm_gem_object *obj,
			   struct drm_i915_gem_pwrite *args,
			   struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	ssize_t remain;
	loff_t offset, page_base;
	char __user *user_data;
	int page_offset, page_length;
	int ret;

	user_data = (char __user *) (uintptr_t) args->data_ptr;
	remain = args->size;

	mutex_lock(&dev->struct_mutex);

	ret = i915_gem_object_get_pages(obj, 0);
	if (ret != 0)
		goto fail_unlock;

	ret = i915_gem_object_set_to_cpu_domain(obj, 1);
	if (ret != 0)
		goto fail_put_pages;

	obj_priv = obj->driver_private;
	offset = args->offset;
	obj_priv->dirty = 1;

	while (remain > 0) {
		
		page_base = (offset & ~(PAGE_SIZE-1));
		page_offset = offset & (PAGE_SIZE-1);
		page_length = remain;
		if ((page_offset + remain) > PAGE_SIZE)
			page_length = PAGE_SIZE - page_offset;

		ret = fast_shmem_write(obj_priv->pages,
				       page_base, page_offset,
				       user_data, page_length);
		if (ret)
			goto fail_put_pages;

		remain -= page_length;
		user_data += page_length;
		offset += page_length;
	}

fail_put_pages:
	i915_gem_object_put_pages(obj);
fail_unlock:
	mutex_unlock(&dev->struct_mutex);

	return ret;
}


static int
i915_gem_shmem_pwrite_slow(struct drm_device *dev, struct drm_gem_object *obj,
			   struct drm_i915_gem_pwrite *args,
			   struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct mm_struct *mm = current->mm;
	struct page **user_pages;
	ssize_t remain;
	loff_t offset, pinned_pages, i;
	loff_t first_data_page, last_data_page, num_pages;
	int shmem_page_index, shmem_page_offset;
	int data_page_index,  data_page_offset;
	int page_length;
	int ret;
	uint64_t data_ptr = args->data_ptr;
	int do_bit17_swizzling;

	remain = args->size;

	
	first_data_page = data_ptr / PAGE_SIZE;
	last_data_page = (data_ptr + args->size - 1) / PAGE_SIZE;
	num_pages = last_data_page - first_data_page + 1;

	user_pages = drm_calloc_large(num_pages, sizeof(struct page *));
	if (user_pages == NULL)
		return -ENOMEM;

	down_read(&mm->mmap_sem);
	pinned_pages = get_user_pages(current, mm, (uintptr_t)args->data_ptr,
				      num_pages, 0, 0, user_pages, NULL);
	up_read(&mm->mmap_sem);
	if (pinned_pages < num_pages) {
		ret = -EFAULT;
		goto fail_put_user_pages;
	}

	do_bit17_swizzling = i915_gem_object_needs_bit17_swizzle(obj);

	mutex_lock(&dev->struct_mutex);

	ret = i915_gem_object_get_pages_or_evict(obj);
	if (ret)
		goto fail_unlock;

	ret = i915_gem_object_set_to_cpu_domain(obj, 1);
	if (ret != 0)
		goto fail_put_pages;

	obj_priv = obj->driver_private;
	offset = args->offset;
	obj_priv->dirty = 1;

	while (remain > 0) {
		
		shmem_page_index = offset / PAGE_SIZE;
		shmem_page_offset = offset & ~PAGE_MASK;
		data_page_index = data_ptr / PAGE_SIZE - first_data_page;
		data_page_offset = data_ptr & ~PAGE_MASK;

		page_length = remain;
		if ((shmem_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - shmem_page_offset;
		if ((data_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - data_page_offset;

		if (do_bit17_swizzling) {
			ret = slow_shmem_bit17_copy(obj_priv->pages[shmem_page_index],
						    shmem_page_offset,
						    user_pages[data_page_index],
						    data_page_offset,
						    page_length,
						    0);
		} else {
			ret = slow_shmem_copy(obj_priv->pages[shmem_page_index],
					      shmem_page_offset,
					      user_pages[data_page_index],
					      data_page_offset,
					      page_length);
		}
		if (ret)
			goto fail_put_pages;

		remain -= page_length;
		data_ptr += page_length;
		offset += page_length;
	}

fail_put_pages:
	i915_gem_object_put_pages(obj);
fail_unlock:
	mutex_unlock(&dev->struct_mutex);
fail_put_user_pages:
	for (i = 0; i < pinned_pages; i++)
		page_cache_release(user_pages[i]);
	drm_free_large(user_pages);

	return ret;
}


int
i915_gem_pwrite_ioctl(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct drm_i915_gem_pwrite *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret = 0;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;
	obj_priv = obj->driver_private;

	
	if (args->offset > obj->size || args->size > obj->size ||
	    args->offset + args->size > obj->size) {
		drm_gem_object_unreference(obj);
		return -EINVAL;
	}

	
	if (obj_priv->phys_obj)
		ret = i915_gem_phys_pwrite(dev, obj, args, file_priv);
	else if (obj_priv->tiling_mode == I915_TILING_NONE &&
		 dev->gtt_total != 0) {
		ret = i915_gem_gtt_pwrite_fast(dev, obj, args, file_priv);
		if (ret == -EFAULT) {
			ret = i915_gem_gtt_pwrite_slow(dev, obj, args,
						       file_priv);
		}
	} else if (i915_gem_object_needs_bit17_swizzle(obj)) {
		ret = i915_gem_shmem_pwrite_slow(dev, obj, args, file_priv);
	} else {
		ret = i915_gem_shmem_pwrite_fast(dev, obj, args, file_priv);
		if (ret == -EFAULT) {
			ret = i915_gem_shmem_pwrite_slow(dev, obj, args,
							 file_priv);
		}
	}

#if WATCH_PWRITE
	if (ret)
		DRM_INFO("pwrite failed %d\n", ret);
#endif

	drm_gem_object_unreference(obj);

	return ret;
}


int
i915_gem_set_domain_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *file_priv)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_set_domain *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	uint32_t read_domains = args->read_domains;
	uint32_t write_domain = args->write_domain;
	int ret;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	
	if (write_domain & I915_GEM_GPU_DOMAINS)
		return -EINVAL;

	if (read_domains & I915_GEM_GPU_DOMAINS)
		return -EINVAL;

	
	if (write_domain != 0 && read_domains != write_domain)
		return -EINVAL;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;
	obj_priv = obj->driver_private;

	mutex_lock(&dev->struct_mutex);

	intel_mark_busy(dev, obj);

#if WATCH_BUF
	DRM_INFO("set_domain_ioctl %p(%zd), %08x %08x\n",
		 obj, obj->size, read_domains, write_domain);
#endif
	if (read_domains & I915_GEM_DOMAIN_GTT) {
		ret = i915_gem_object_set_to_gtt_domain(obj, write_domain != 0);

		
		if (obj_priv->fence_reg != I915_FENCE_REG_NONE) {
			list_move_tail(&obj_priv->fence_list,
				       &dev_priv->mm.fence_list);
		}

		
		if (ret == -EINVAL)
			ret = 0;
	} else {
		ret = i915_gem_object_set_to_cpu_domain(obj, write_domain != 0);
	}

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	return ret;
}


int
i915_gem_sw_finish_ioctl(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct drm_i915_gem_sw_finish *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret = 0;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	mutex_lock(&dev->struct_mutex);
	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL) {
		mutex_unlock(&dev->struct_mutex);
		return -EBADF;
	}

#if WATCH_BUF
	DRM_INFO("%s: sw_finish %d (%p %zd)\n",
		 __func__, args->handle, obj, obj->size);
#endif
	obj_priv = obj->driver_private;

	
	if (obj_priv->pin_count)
		i915_gem_object_flush_cpu_write_domain(obj);

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	return ret;
}


int
i915_gem_mmap_ioctl(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_i915_gem_mmap *args = data;
	struct drm_gem_object *obj;
	loff_t offset;
	unsigned long addr;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;

	offset = args->offset;

	down_write(&current->mm->mmap_sem);
	addr = do_mmap(obj->filp, 0, args->size,
		       PROT_READ | PROT_WRITE, MAP_SHARED,
		       args->offset);
	up_write(&current->mm->mmap_sem);
	mutex_lock(&dev->struct_mutex);
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	if (IS_ERR((void *)addr))
		return addr;

	args->addr_ptr = (uint64_t) addr;

	return 0;
}


int i915_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct drm_device *dev = obj->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	pgoff_t page_offset;
	unsigned long pfn;
	int ret = 0;
	bool write = !!(vmf->flags & FAULT_FLAG_WRITE);

	
	page_offset = ((unsigned long)vmf->virtual_address - vma->vm_start) >>
		PAGE_SHIFT;

	
	mutex_lock(&dev->struct_mutex);
	if (!obj_priv->gtt_space) {
		ret = i915_gem_object_bind_to_gtt(obj, 0);
		if (ret)
			goto unlock;

		list_add_tail(&obj_priv->list, &dev_priv->mm.inactive_list);

		ret = i915_gem_object_set_to_gtt_domain(obj, write);
		if (ret)
			goto unlock;
	}

	
	if (obj_priv->tiling_mode != I915_TILING_NONE) {
		ret = i915_gem_object_get_fence_reg(obj);
		if (ret)
			goto unlock;
	}

	pfn = ((dev->agp->base + obj_priv->gtt_offset) >> PAGE_SHIFT) +
		page_offset;

	
	ret = vm_insert_pfn(vma, (unsigned long)vmf->virtual_address, pfn);
unlock:
	mutex_unlock(&dev->struct_mutex);

	switch (ret) {
	case 0:
	case -ERESTARTSYS:
		return VM_FAULT_NOPAGE;
	case -ENOMEM:
	case -EAGAIN:
		return VM_FAULT_OOM;
	default:
		return VM_FAULT_SIGBUS;
	}
}


static int
i915_gem_create_mmap_offset(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_gem_mm *mm = dev->mm_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct drm_map_list *list;
	struct drm_local_map *map;
	int ret = 0;

	
	list = &obj->map_list;
	list->map = kzalloc(sizeof(struct drm_map_list), GFP_KERNEL);
	if (!list->map)
		return -ENOMEM;

	map = list->map;
	map->type = _DRM_GEM;
	map->size = obj->size;
	map->handle = obj;

	
	list->file_offset_node = drm_mm_search_free(&mm->offset_manager,
						    obj->size / PAGE_SIZE, 0, 0);
	if (!list->file_offset_node) {
		DRM_ERROR("failed to allocate offset for bo %d\n", obj->name);
		ret = -ENOMEM;
		goto out_free_list;
	}

	list->file_offset_node = drm_mm_get_block(list->file_offset_node,
						  obj->size / PAGE_SIZE, 0);
	if (!list->file_offset_node) {
		ret = -ENOMEM;
		goto out_free_list;
	}

	list->hash.key = list->file_offset_node->start;
	if (drm_ht_insert_item(&mm->offset_hash, &list->hash)) {
		DRM_ERROR("failed to add to map hash\n");
		ret = -ENOMEM;
		goto out_free_mm;
	}

	
	obj_priv->mmap_offset = ((uint64_t) list->hash.key) << PAGE_SHIFT;

	return 0;

out_free_mm:
	drm_mm_put_block(list->file_offset_node);
out_free_list:
	kfree(list->map);

	return ret;
}


void
i915_gem_release_mmap(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	if (dev->dev_mapping)
		unmap_mapping_range(dev->dev_mapping,
				    obj_priv->mmap_offset, obj->size, 1);
}

static void
i915_gem_free_mmap_offset(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct drm_gem_mm *mm = dev->mm_private;
	struct drm_map_list *list;

	list = &obj->map_list;
	drm_ht_remove_item(&mm->offset_hash, &list->hash);

	if (list->file_offset_node) {
		drm_mm_put_block(list->file_offset_node);
		list->file_offset_node = NULL;
	}

	if (list->map) {
		kfree(list->map);
		list->map = NULL;
	}

	obj_priv->mmap_offset = 0;
}


static uint32_t
i915_gem_get_gtt_alignment(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int start, i;

	
	if (IS_I965G(dev) || obj_priv->tiling_mode == I915_TILING_NONE)
		return 4096;

	
	if (IS_I9XX(dev))
		start = 1024*1024;
	else
		start = 512*1024;

	for (i = start; i < obj->size; i <<= 1)
		;

	return i;
}


int
i915_gem_mmap_gtt_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv)
{
	struct drm_i915_gem_mmap_gtt *args = data;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;

	mutex_lock(&dev->struct_mutex);

	obj_priv = obj->driver_private;

	if (obj_priv->madv != I915_MADV_WILLNEED) {
		DRM_ERROR("Attempting to mmap a purgeable buffer\n");
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}


	if (!obj_priv->mmap_offset) {
		ret = i915_gem_create_mmap_offset(obj);
		if (ret) {
			drm_gem_object_unreference(obj);
			mutex_unlock(&dev->struct_mutex);
			return ret;
		}
	}

	args->offset = obj_priv->mmap_offset;

	
	if (!obj_priv->agp_mem) {
		ret = i915_gem_object_bind_to_gtt(obj, 0);
		if (ret) {
			drm_gem_object_unreference(obj);
			mutex_unlock(&dev->struct_mutex);
			return ret;
		}
		list_add_tail(&obj_priv->list, &dev_priv->mm.inactive_list);
	}

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}

void
i915_gem_object_put_pages(struct drm_gem_object *obj)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int page_count = obj->size / PAGE_SIZE;
	int i;

	BUG_ON(obj_priv->pages_refcount == 0);
	BUG_ON(obj_priv->madv == __I915_MADV_PURGED);

	if (--obj_priv->pages_refcount != 0)
		return;

	if (obj_priv->tiling_mode != I915_TILING_NONE)
		i915_gem_object_save_bit_17_swizzle(obj);

	if (obj_priv->madv == I915_MADV_DONTNEED)
		obj_priv->dirty = 0;

	for (i = 0; i < page_count; i++) {
		if (obj_priv->pages[i] == NULL)
			break;

		if (obj_priv->dirty)
			set_page_dirty(obj_priv->pages[i]);

		if (obj_priv->madv == I915_MADV_WILLNEED)
			mark_page_accessed(obj_priv->pages[i]);

		page_cache_release(obj_priv->pages[i]);
	}
	obj_priv->dirty = 0;

	drm_free_large(obj_priv->pages);
	obj_priv->pages = NULL;
}

static void
i915_gem_object_move_to_active(struct drm_gem_object *obj, uint32_t seqno)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	
	if (!obj_priv->active) {
		drm_gem_object_reference(obj);
		obj_priv->active = 1;
	}
	
	spin_lock(&dev_priv->mm.active_list_lock);
	list_move_tail(&obj_priv->list,
		       &dev_priv->mm.active_list);
	spin_unlock(&dev_priv->mm.active_list_lock);
	obj_priv->last_rendering_seqno = seqno;
}

static void
i915_gem_object_move_to_flushing(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	BUG_ON(!obj_priv->active);
	list_move_tail(&obj_priv->list, &dev_priv->mm.flushing_list);
	obj_priv->last_rendering_seqno = 0;
}


static void
i915_gem_object_truncate(struct drm_gem_object *obj)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct inode *inode;

	inode = obj->filp->f_path.dentry->d_inode;
	if (inode->i_op->truncate)
		inode->i_op->truncate (inode);

	obj_priv->madv = __I915_MADV_PURGED;
}

static inline int
i915_gem_object_is_purgeable(struct drm_i915_gem_object *obj_priv)
{
	return obj_priv->madv == I915_MADV_DONTNEED;
}

static void
i915_gem_object_move_to_inactive(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	i915_verify_inactive(dev, __FILE__, __LINE__);
	if (obj_priv->pin_count != 0)
		list_del_init(&obj_priv->list);
	else
		list_move_tail(&obj_priv->list, &dev_priv->mm.inactive_list);

	BUG_ON(!list_empty(&obj_priv->gpu_write_list));

	obj_priv->last_rendering_seqno = 0;
	if (obj_priv->active) {
		obj_priv->active = 0;
		drm_gem_object_unreference(obj);
	}
	i915_verify_inactive(dev, __FILE__, __LINE__);
}


static uint32_t
i915_add_request(struct drm_device *dev, struct drm_file *file_priv,
		 uint32_t flush_domains)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_file_private *i915_file_priv = NULL;
	struct drm_i915_gem_request *request;
	uint32_t seqno;
	int was_empty;
	RING_LOCALS;

	if (file_priv != NULL)
		i915_file_priv = file_priv->driver_priv;

	request = kzalloc(sizeof(*request), GFP_KERNEL);
	if (request == NULL)
		return 0;

	
	seqno = dev_priv->mm.next_gem_seqno;
	dev_priv->mm.next_gem_seqno++;
	if (dev_priv->mm.next_gem_seqno == 0)
		dev_priv->mm.next_gem_seqno++;

	BEGIN_LP_RING(4);
	OUT_RING(MI_STORE_DWORD_INDEX);
	OUT_RING(I915_GEM_HWS_INDEX << MI_STORE_DWORD_INDEX_SHIFT);
	OUT_RING(seqno);

	OUT_RING(MI_USER_INTERRUPT);
	ADVANCE_LP_RING();

	DRM_DEBUG("%d\n", seqno);

	request->seqno = seqno;
	request->emitted_jiffies = jiffies;
	was_empty = list_empty(&dev_priv->mm.request_list);
	list_add_tail(&request->list, &dev_priv->mm.request_list);
	if (i915_file_priv) {
		list_add_tail(&request->client_list,
			      &i915_file_priv->mm.request_list);
	} else {
		INIT_LIST_HEAD(&request->client_list);
	}

	
	if (flush_domains != 0) {
		struct drm_i915_gem_object *obj_priv, *next;

		list_for_each_entry_safe(obj_priv, next,
					 &dev_priv->mm.gpu_write_list,
					 gpu_write_list) {
			struct drm_gem_object *obj = obj_priv->obj;

			if ((obj->write_domain & flush_domains) ==
			    obj->write_domain) {
				uint32_t old_write_domain = obj->write_domain;

				obj->write_domain = 0;
				list_del_init(&obj_priv->gpu_write_list);
				i915_gem_object_move_to_active(obj, seqno);

				trace_i915_gem_object_change_domain(obj,
								    obj->read_domains,
								    old_write_domain);
			}
		}

	}

	if (!dev_priv->mm.suspended) {
		mod_timer(&dev_priv->hangcheck_timer, jiffies + DRM_I915_HANGCHECK_PERIOD);
		if (was_empty)
			queue_delayed_work(dev_priv->wq, &dev_priv->mm.retire_work, HZ);
	}
	return seqno;
}


static uint32_t
i915_retire_commands(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	uint32_t cmd = MI_FLUSH | MI_NO_WRITE_FLUSH;
	uint32_t flush_domains = 0;
	RING_LOCALS;

	
	if (IS_I965G(dev))
		flush_domains |= I915_GEM_DOMAIN_SAMPLER;
	BEGIN_LP_RING(2);
	OUT_RING(cmd);
	OUT_RING(0); 
	ADVANCE_LP_RING();
	return flush_domains;
}


static void
i915_gem_retire_request(struct drm_device *dev,
			struct drm_i915_gem_request *request)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	trace_i915_gem_request_retire(dev, request->seqno);

	
	spin_lock(&dev_priv->mm.active_list_lock);
	while (!list_empty(&dev_priv->mm.active_list)) {
		struct drm_gem_object *obj;
		struct drm_i915_gem_object *obj_priv;

		obj_priv = list_first_entry(&dev_priv->mm.active_list,
					    struct drm_i915_gem_object,
					    list);
		obj = obj_priv->obj;

		
		if (obj_priv->last_rendering_seqno != request->seqno)
			goto out;

#if WATCH_LRU
		DRM_INFO("%s: retire %d moves to inactive list %p\n",
			 __func__, request->seqno, obj);
#endif

		if (obj->write_domain != 0)
			i915_gem_object_move_to_flushing(obj);
		else {
			
			drm_gem_object_reference(obj);
			i915_gem_object_move_to_inactive(obj);
			spin_unlock(&dev_priv->mm.active_list_lock);
			drm_gem_object_unreference(obj);
			spin_lock(&dev_priv->mm.active_list_lock);
		}
	}
out:
	spin_unlock(&dev_priv->mm.active_list_lock);
}


bool
i915_seqno_passed(uint32_t seq1, uint32_t seq2)
{
	return (int32_t)(seq1 - seq2) >= 0;
}

uint32_t
i915_get_gem_seqno(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	return READ_HWSP(dev_priv, I915_GEM_HWS_INDEX);
}


void
i915_gem_retire_requests(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	uint32_t seqno;

	if (!dev_priv->hw_status_page || list_empty(&dev_priv->mm.request_list))
		return;

	seqno = i915_get_gem_seqno(dev);

	while (!list_empty(&dev_priv->mm.request_list)) {
		struct drm_i915_gem_request *request;
		uint32_t retiring_seqno;

		request = list_first_entry(&dev_priv->mm.request_list,
					   struct drm_i915_gem_request,
					   list);
		retiring_seqno = request->seqno;

		if (i915_seqno_passed(seqno, retiring_seqno) ||
		    atomic_read(&dev_priv->mm.wedged)) {
			i915_gem_retire_request(dev, request);

			list_del(&request->list);
			list_del(&request->client_list);
			kfree(request);
		} else
			break;
	}

	if (unlikely (dev_priv->trace_irq_seqno &&
		      i915_seqno_passed(dev_priv->trace_irq_seqno, seqno))) {
		i915_user_irq_put(dev);
		dev_priv->trace_irq_seqno = 0;
	}
}

void
i915_gem_retire_work_handler(struct work_struct *work)
{
	drm_i915_private_t *dev_priv;
	struct drm_device *dev;

	dev_priv = container_of(work, drm_i915_private_t,
				mm.retire_work.work);
	dev = dev_priv->dev;

	mutex_lock(&dev->struct_mutex);
	i915_gem_retire_requests(dev);
	if (!dev_priv->mm.suspended &&
	    !list_empty(&dev_priv->mm.request_list))
		queue_delayed_work(dev_priv->wq, &dev_priv->mm.retire_work, HZ);
	mutex_unlock(&dev->struct_mutex);
}

static int
i915_do_wait_request(struct drm_device *dev, uint32_t seqno, int interruptible)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	u32 ier;
	int ret = 0;

	BUG_ON(seqno == 0);

	if (atomic_read(&dev_priv->mm.wedged))
		return -EIO;

	if (!i915_seqno_passed(i915_get_gem_seqno(dev), seqno)) {
		if (IS_IGDNG(dev))
			ier = I915_READ(DEIER) | I915_READ(GTIER);
		else
			ier = I915_READ(IER);
		if (!ier) {
			DRM_ERROR("something (likely vbetool) disabled "
				  "interrupts, re-enabling\n");
			i915_driver_irq_preinstall(dev);
			i915_driver_irq_postinstall(dev);
		}

		trace_i915_gem_request_wait_begin(dev, seqno);

		dev_priv->mm.waiting_gem_seqno = seqno;
		i915_user_irq_get(dev);
		if (interruptible)
			ret = wait_event_interruptible(dev_priv->irq_queue,
				i915_seqno_passed(i915_get_gem_seqno(dev), seqno) ||
				atomic_read(&dev_priv->mm.wedged));
		else
			wait_event(dev_priv->irq_queue,
				i915_seqno_passed(i915_get_gem_seqno(dev), seqno) ||
				atomic_read(&dev_priv->mm.wedged));

		i915_user_irq_put(dev);
		dev_priv->mm.waiting_gem_seqno = 0;

		trace_i915_gem_request_wait_end(dev, seqno);
	}
	if (atomic_read(&dev_priv->mm.wedged))
		ret = -EIO;

	if (ret && ret != -ERESTARTSYS)
		DRM_ERROR("%s returns %d (awaiting %d at %d)\n",
			  __func__, ret, seqno, i915_get_gem_seqno(dev));

	
	if (ret == 0)
		i915_gem_retire_requests(dev);

	return ret;
}


static int
i915_wait_request(struct drm_device *dev, uint32_t seqno)
{
	return i915_do_wait_request(dev, seqno, 1);
}


int i915_lp_ring_sync(struct drm_device *dev)
{
	uint32_t seqno;
	int ret;

	seqno = i915_add_request(dev, NULL, 0);

	if (seqno == 0)
		return -ENOMEM;

	ret = i915_do_wait_request(dev, seqno, 0);
	BUG_ON(ret == -ERESTARTSYS);
	return ret;
}

static void
i915_gem_flush(struct drm_device *dev,
	       uint32_t invalidate_domains,
	       uint32_t flush_domains)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	uint32_t cmd;
	RING_LOCALS;

#if WATCH_EXEC
	DRM_INFO("%s: invalidate %08x flush %08x\n", __func__,
		  invalidate_domains, flush_domains);
#endif
	trace_i915_gem_request_flush(dev, dev_priv->mm.next_gem_seqno,
				     invalidate_domains, flush_domains);

	if (flush_domains & I915_GEM_DOMAIN_CPU)
		drm_agp_chipset_flush(dev);

	if ((invalidate_domains | flush_domains) & I915_GEM_GPU_DOMAINS) {
		

		cmd = MI_FLUSH | MI_NO_WRITE_FLUSH;
		if ((invalidate_domains|flush_domains) &
		    I915_GEM_DOMAIN_RENDER)
			cmd &= ~MI_NO_WRITE_FLUSH;
		if (!IS_I965G(dev)) {
			
			if (invalidate_domains & I915_GEM_DOMAIN_SAMPLER)
				cmd |= MI_READ_FLUSH;
		}
		if (invalidate_domains & I915_GEM_DOMAIN_INSTRUCTION)
			cmd |= MI_EXE_FLUSH;

#if WATCH_EXEC
		DRM_INFO("%s: queue flush %08x to ring\n", __func__, cmd);
#endif
		BEGIN_LP_RING(2);
		OUT_RING(cmd);
		OUT_RING(MI_NOOP);
		ADVANCE_LP_RING();
	}
}


static int
i915_gem_object_wait_rendering(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int ret;

	
	BUG_ON((obj->write_domain & I915_GEM_GPU_DOMAINS) != 0);

	
	if (obj_priv->active) {
#if WATCH_BUF
		DRM_INFO("%s: object %p wait for seqno %08x\n",
			  __func__, obj, obj_priv->last_rendering_seqno);
#endif
		ret = i915_wait_request(dev, obj_priv->last_rendering_seqno);
		if (ret != 0)
			return ret;
	}

	return 0;
}


int
i915_gem_object_unbind(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int ret = 0;

#if WATCH_BUF
	DRM_INFO("%s:%d %p\n", __func__, __LINE__, obj);
	DRM_INFO("gtt_space %p\n", obj_priv->gtt_space);
#endif
	if (obj_priv->gtt_space == NULL)
		return 0;

	if (obj_priv->pin_count != 0) {
		DRM_ERROR("Attempting to unbind pinned buffer\n");
		return -EINVAL;
	}

	
	i915_gem_release_mmap(obj);

	
	ret = i915_gem_object_set_to_cpu_domain(obj, 1);
	if (ret) {
		if (ret != -ERESTARTSYS)
			DRM_ERROR("set_domain failed: %d\n", ret);
		return ret;
	}

	BUG_ON(obj_priv->active);

	
	if (obj_priv->fence_reg != I915_FENCE_REG_NONE)
		i915_gem_clear_fence_reg(obj);

	if (obj_priv->agp_mem != NULL) {
		drm_unbind_agp(obj_priv->agp_mem);
		drm_free_agp(obj_priv->agp_mem, obj->size / PAGE_SIZE);
		obj_priv->agp_mem = NULL;
	}

	i915_gem_object_put_pages(obj);
	BUG_ON(obj_priv->pages_refcount);

	if (obj_priv->gtt_space) {
		atomic_dec(&dev->gtt_count);
		atomic_sub(obj->size, &dev->gtt_memory);

		drm_mm_put_block(obj_priv->gtt_space);
		obj_priv->gtt_space = NULL;
	}

	
	if (!list_empty(&obj_priv->list))
		list_del_init(&obj_priv->list);

	if (i915_gem_object_is_purgeable(obj_priv))
		i915_gem_object_truncate(obj);

	trace_i915_gem_object_unbind(obj);

	return 0;
}

static struct drm_gem_object *
i915_gem_find_inactive_object(struct drm_device *dev, int min_size)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv;
	struct drm_gem_object *best = NULL;
	struct drm_gem_object *first = NULL;

	
	list_for_each_entry(obj_priv, &dev_priv->mm.inactive_list, list) {
		struct drm_gem_object *obj = obj_priv->obj;
		if (obj->size >= min_size) {
			if ((!obj_priv->dirty ||
			     i915_gem_object_is_purgeable(obj_priv)) &&
			    (!best || obj->size < best->size)) {
				best = obj;
				if (best->size == min_size)
					return best;
			}
			if (!first)
			    first = obj;
		}
	}

	return best ? best : first;
}

static int
i915_gem_evict_everything(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret;
	uint32_t seqno;
	bool lists_empty;

	spin_lock(&dev_priv->mm.active_list_lock);
	lists_empty = (list_empty(&dev_priv->mm.inactive_list) &&
		       list_empty(&dev_priv->mm.flushing_list) &&
		       list_empty(&dev_priv->mm.active_list));
	spin_unlock(&dev_priv->mm.active_list_lock);

	if (lists_empty)
		return -ENOSPC;

	
	i915_gem_flush(dev, I915_GEM_GPU_DOMAINS, I915_GEM_GPU_DOMAINS);
	seqno = i915_add_request(dev, NULL, I915_GEM_GPU_DOMAINS);
	if (seqno == 0)
		return -ENOMEM;

	ret = i915_wait_request(dev, seqno);
	if (ret)
		return ret;

	BUG_ON(!list_empty(&dev_priv->mm.flushing_list));

	ret = i915_gem_evict_from_inactive_list(dev);
	if (ret)
		return ret;

	spin_lock(&dev_priv->mm.active_list_lock);
	lists_empty = (list_empty(&dev_priv->mm.inactive_list) &&
		       list_empty(&dev_priv->mm.flushing_list) &&
		       list_empty(&dev_priv->mm.active_list));
	spin_unlock(&dev_priv->mm.active_list_lock);
	BUG_ON(!lists_empty);

	return 0;
}

static int
i915_gem_evict_something(struct drm_device *dev, int min_size)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	int ret;

	for (;;) {
		i915_gem_retire_requests(dev);

		
		obj = i915_gem_find_inactive_object(dev, min_size);
		if (obj) {
			struct drm_i915_gem_object *obj_priv;

#if WATCH_LRU
			DRM_INFO("%s: evicting %p\n", __func__, obj);
#endif
			obj_priv = obj->driver_private;
			BUG_ON(obj_priv->pin_count != 0);
			BUG_ON(obj_priv->active);

			
			return i915_gem_object_unbind(obj);
		}

		
		if (!list_empty(&dev_priv->mm.request_list)) {
			struct drm_i915_gem_request *request;

			request = list_first_entry(&dev_priv->mm.request_list,
						   struct drm_i915_gem_request,
						   list);

			ret = i915_wait_request(dev, request->seqno);
			if (ret)
				return ret;

			continue;
		}

		
		if (!list_empty(&dev_priv->mm.flushing_list)) {
			struct drm_i915_gem_object *obj_priv;

			
			list_for_each_entry(obj_priv, &dev_priv->mm.flushing_list, list) {
				obj = obj_priv->obj;
				if (obj->size >= min_size)
					break;

				obj = NULL;
			}

			if (obj != NULL) {
				uint32_t seqno;

				i915_gem_flush(dev,
					       obj->write_domain,
					       obj->write_domain);
				seqno = i915_add_request(dev, NULL, obj->write_domain);
				if (seqno == 0)
					return -ENOMEM;

				ret = i915_wait_request(dev, seqno);
				if (ret)
					return ret;

				continue;
			}
		}

		
		if (!list_empty (&dev_priv->mm.inactive_list))
			return i915_gem_evict_from_inactive_list(dev);
		else
			return i915_gem_evict_everything(dev);
	}
}

int
i915_gem_object_get_pages(struct drm_gem_object *obj,
			  gfp_t gfpmask)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int page_count, i;
	struct address_space *mapping;
	struct inode *inode;
	struct page *page;
	int ret;

	if (obj_priv->pages_refcount++ != 0)
		return 0;

	
	page_count = obj->size / PAGE_SIZE;
	BUG_ON(obj_priv->pages != NULL);
	obj_priv->pages = drm_calloc_large(page_count, sizeof(struct page *));
	if (obj_priv->pages == NULL) {
		obj_priv->pages_refcount--;
		return -ENOMEM;
	}

	inode = obj->filp->f_path.dentry->d_inode;
	mapping = inode->i_mapping;
	for (i = 0; i < page_count; i++) {
		page = read_cache_page_gfp(mapping, i,
					   mapping_gfp_mask (mapping) |
					   __GFP_COLD |
					   gfpmask);
		if (IS_ERR(page)) {
			ret = PTR_ERR(page);
			i915_gem_object_put_pages(obj);
			return ret;
		}
		obj_priv->pages[i] = page;
	}

	if (obj_priv->tiling_mode != I915_TILING_NONE)
		i915_gem_object_do_bit_17_swizzle(obj);

	return 0;
}

static void i965_write_fence_reg(struct drm_i915_fence_reg *reg)
{
	struct drm_gem_object *obj = reg->obj;
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int regnum = obj_priv->fence_reg;
	uint64_t val;

	val = (uint64_t)((obj_priv->gtt_offset + obj->size - 4096) &
		    0xfffff000) << 32;
	val |= obj_priv->gtt_offset & 0xfffff000;
	val |= ((obj_priv->stride / 128) - 1) << I965_FENCE_PITCH_SHIFT;
	if (obj_priv->tiling_mode == I915_TILING_Y)
		val |= 1 << I965_FENCE_TILING_Y_SHIFT;
	val |= I965_FENCE_REG_VALID;

	I915_WRITE64(FENCE_REG_965_0 + (regnum * 8), val);
}

static void i915_write_fence_reg(struct drm_i915_fence_reg *reg)
{
	struct drm_gem_object *obj = reg->obj;
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int regnum = obj_priv->fence_reg;
	int tile_width;
	uint32_t fence_reg, val;
	uint32_t pitch_val;

	if ((obj_priv->gtt_offset & ~I915_FENCE_START_MASK) ||
	    (obj_priv->gtt_offset & (obj->size - 1))) {
		WARN(1, "%s: object 0x%08x not 1M or size (0x%zx) aligned\n",
		     __func__, obj_priv->gtt_offset, obj->size);
		return;
	}

	if (obj_priv->tiling_mode == I915_TILING_Y &&
	    HAS_128_BYTE_Y_TILING(dev))
		tile_width = 128;
	else
		tile_width = 512;

	
	pitch_val = obj_priv->stride / tile_width;
	pitch_val = ffs(pitch_val) - 1;

	val = obj_priv->gtt_offset;
	if (obj_priv->tiling_mode == I915_TILING_Y)
		val |= 1 << I830_FENCE_TILING_Y_SHIFT;
	val |= I915_FENCE_SIZE_BITS(obj->size);
	val |= pitch_val << I830_FENCE_PITCH_SHIFT;
	val |= I830_FENCE_REG_VALID;

	if (regnum < 8)
		fence_reg = FENCE_REG_830_0 + (regnum * 4);
	else
		fence_reg = FENCE_REG_945_8 + ((regnum - 8) * 4);
	I915_WRITE(fence_reg, val);
}

static void i830_write_fence_reg(struct drm_i915_fence_reg *reg)
{
	struct drm_gem_object *obj = reg->obj;
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int regnum = obj_priv->fence_reg;
	uint32_t val;
	uint32_t pitch_val;
	uint32_t fence_size_bits;

	if ((obj_priv->gtt_offset & ~I830_FENCE_START_MASK) ||
	    (obj_priv->gtt_offset & (obj->size - 1))) {
		WARN(1, "%s: object 0x%08x not 512K or size aligned\n",
		     __func__, obj_priv->gtt_offset);
		return;
	}

	pitch_val = obj_priv->stride / 128;
	pitch_val = ffs(pitch_val) - 1;
	WARN_ON(pitch_val > I830_FENCE_MAX_PITCH_VAL);

	val = obj_priv->gtt_offset;
	if (obj_priv->tiling_mode == I915_TILING_Y)
		val |= 1 << I830_FENCE_TILING_Y_SHIFT;
	fence_size_bits = I830_FENCE_SIZE_BITS(obj->size);
	WARN_ON(fence_size_bits & ~0x00000f00);
	val |= fence_size_bits;
	val |= pitch_val << I830_FENCE_PITCH_SHIFT;
	val |= I830_FENCE_REG_VALID;

	I915_WRITE(FENCE_REG_830_0 + (regnum * 4), val);
}


int
i915_gem_object_get_fence_reg(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct drm_i915_fence_reg *reg = NULL;
	struct drm_i915_gem_object *old_obj_priv = NULL;
	int i, ret, avail;

	
	if (obj_priv->fence_reg != I915_FENCE_REG_NONE) {
		list_move_tail(&obj_priv->fence_list, &dev_priv->mm.fence_list);
		return 0;
	}

	switch (obj_priv->tiling_mode) {
	case I915_TILING_NONE:
		WARN(1, "allocating a fence for non-tiled object?\n");
		break;
	case I915_TILING_X:
		if (!obj_priv->stride)
			return -EINVAL;
		WARN((obj_priv->stride & (512 - 1)),
		     "object 0x%08x is X tiled but has non-512B pitch\n",
		     obj_priv->gtt_offset);
		break;
	case I915_TILING_Y:
		if (!obj_priv->stride)
			return -EINVAL;
		WARN((obj_priv->stride & (128 - 1)),
		     "object 0x%08x is Y tiled but has non-128B pitch\n",
		     obj_priv->gtt_offset);
		break;
	}

	
	avail = 0;
	for (i = dev_priv->fence_reg_start; i < dev_priv->num_fence_regs; i++) {
		reg = &dev_priv->fence_regs[i];
		if (!reg->obj)
			break;

		old_obj_priv = reg->obj->driver_private;
		if (!old_obj_priv->pin_count)
		    avail++;
	}

	
	if (i == dev_priv->num_fence_regs) {
		struct drm_gem_object *old_obj = NULL;

		if (avail == 0)
			return -ENOSPC;

		list_for_each_entry(old_obj_priv, &dev_priv->mm.fence_list,
				    fence_list) {
			old_obj = old_obj_priv->obj;

			if (old_obj_priv->pin_count)
				continue;

			
			drm_gem_object_reference(old_obj);

			
			if (IS_I965G(dev) || !old_obj_priv->active)
				break;

			
			i915_gem_object_flush_gpu_write_domain(old_obj);
			ret = i915_gem_object_wait_rendering(old_obj);
			if (ret != 0) {
				drm_gem_object_unreference(old_obj);
				return ret;
			}

			break;
		}

		
		i915_gem_release_mmap(old_obj);

		i = old_obj_priv->fence_reg;
		reg = &dev_priv->fence_regs[i];

		old_obj_priv->fence_reg = I915_FENCE_REG_NONE;
		list_del_init(&old_obj_priv->fence_list);

		drm_gem_object_unreference(old_obj);
	}

	obj_priv->fence_reg = i;
	list_add_tail(&obj_priv->fence_list, &dev_priv->mm.fence_list);

	reg->obj = obj;

	if (IS_I965G(dev))
		i965_write_fence_reg(reg);
	else if (IS_I9XX(dev))
		i915_write_fence_reg(reg);
	else
		i830_write_fence_reg(reg);

	trace_i915_gem_object_get_fence(obj, i, obj_priv->tiling_mode);

	return 0;
}


static void
i915_gem_clear_fence_reg(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	if (IS_I965G(dev))
		I915_WRITE64(FENCE_REG_965_0 + (obj_priv->fence_reg * 8), 0);
	else {
		uint32_t fence_reg;

		if (obj_priv->fence_reg < 8)
			fence_reg = FENCE_REG_830_0 + obj_priv->fence_reg * 4;
		else
			fence_reg = FENCE_REG_945_8 + (obj_priv->fence_reg -
						       8) * 4;

		I915_WRITE(fence_reg, 0);
	}

	dev_priv->fence_regs[obj_priv->fence_reg].obj = NULL;
	obj_priv->fence_reg = I915_FENCE_REG_NONE;
	list_del_init(&obj_priv->fence_list);
}


int
i915_gem_object_put_fence_reg(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	if (obj_priv->fence_reg == I915_FENCE_REG_NONE)
		return 0;

	
	if (!IS_I965G(dev)) {
		int ret;

		i915_gem_object_flush_gpu_write_domain(obj);
		i915_gem_object_flush_gtt_write_domain(obj);
		ret = i915_gem_object_wait_rendering(obj);
		if (ret != 0)
			return ret;
	}

	i915_gem_clear_fence_reg (obj);

	return 0;
}


static int
i915_gem_object_bind_to_gtt(struct drm_gem_object *obj, unsigned alignment)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	struct drm_mm_node *free_space;
	gfp_t gfpmask =  __GFP_NORETRY | __GFP_NOWARN;
	int ret;

	if (obj_priv->madv != I915_MADV_WILLNEED) {
		DRM_ERROR("Attempting to bind a purgeable object\n");
		return -EINVAL;
	}

	if (alignment == 0)
		alignment = i915_gem_get_gtt_alignment(obj);
	if (alignment & (i915_gem_get_gtt_alignment(obj) - 1)) {
		DRM_ERROR("Invalid object alignment requested %u\n", alignment);
		return -EINVAL;
	}

 search_free:
	free_space = drm_mm_search_free(&dev_priv->mm.gtt_space,
					obj->size, alignment, 0);
	if (free_space != NULL) {
		obj_priv->gtt_space = drm_mm_get_block(free_space, obj->size,
						       alignment);
		if (obj_priv->gtt_space != NULL) {
			obj_priv->gtt_space->private = obj;
			obj_priv->gtt_offset = obj_priv->gtt_space->start;
		}
	}
	if (obj_priv->gtt_space == NULL) {
		
#if WATCH_LRU
		DRM_INFO("%s: GTT full, evicting something\n", __func__);
#endif
		ret = i915_gem_evict_something(dev, obj->size);
		if (ret)
			return ret;

		goto search_free;
	}

#if WATCH_BUF
	DRM_INFO("Binding object of size %zd at 0x%08x\n",
		 obj->size, obj_priv->gtt_offset);
#endif
	ret = i915_gem_object_get_pages(obj, gfpmask);
	if (ret) {
		drm_mm_put_block(obj_priv->gtt_space);
		obj_priv->gtt_space = NULL;

		if (ret == -ENOMEM) {
			
			ret = i915_gem_evict_something(dev, obj->size);
			if (ret) {
				
				if (gfpmask) {
					gfpmask = 0;
					goto search_free;
				}

				return ret;
			}

			goto search_free;
		}

		return ret;
	}

	
	obj_priv->agp_mem = drm_agp_bind_pages(dev,
					       obj_priv->pages,
					       obj->size >> PAGE_SHIFT,
					       obj_priv->gtt_offset,
					       obj_priv->agp_type);
	if (obj_priv->agp_mem == NULL) {
		i915_gem_object_put_pages(obj);
		drm_mm_put_block(obj_priv->gtt_space);
		obj_priv->gtt_space = NULL;

		ret = i915_gem_evict_something(dev, obj->size);
		if (ret)
			return ret;

		goto search_free;
	}
	atomic_inc(&dev->gtt_count);
	atomic_add(obj->size, &dev->gtt_memory);

	
	BUG_ON(obj->read_domains & I915_GEM_GPU_DOMAINS);
	BUG_ON(obj->write_domain & I915_GEM_GPU_DOMAINS);

	trace_i915_gem_object_bind(obj, obj_priv->gtt_offset);

	return 0;
}

void
i915_gem_clflush_object(struct drm_gem_object *obj)
{
	struct drm_i915_gem_object	*obj_priv = obj->driver_private;

	
	if (obj_priv->pages == NULL)
		return;

	trace_i915_gem_object_clflush(obj);

	drm_clflush_pages(obj_priv->pages, obj->size / PAGE_SIZE);
}


static void
i915_gem_object_flush_gpu_write_domain(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	uint32_t seqno;
	uint32_t old_write_domain;

	if ((obj->write_domain & I915_GEM_GPU_DOMAINS) == 0)
		return;

	
	old_write_domain = obj->write_domain;
	i915_gem_flush(dev, 0, obj->write_domain);
	seqno = i915_add_request(dev, NULL, obj->write_domain);
	BUG_ON(obj->write_domain);
	i915_gem_object_move_to_active(obj, seqno);

	trace_i915_gem_object_change_domain(obj,
					    obj->read_domains,
					    old_write_domain);
}


static void
i915_gem_object_flush_gtt_write_domain(struct drm_gem_object *obj)
{
	uint32_t old_write_domain;

	if (obj->write_domain != I915_GEM_DOMAIN_GTT)
		return;

	
	old_write_domain = obj->write_domain;
	obj->write_domain = 0;

	trace_i915_gem_object_change_domain(obj,
					    obj->read_domains,
					    old_write_domain);
}


static void
i915_gem_object_flush_cpu_write_domain(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	uint32_t old_write_domain;

	if (obj->write_domain != I915_GEM_DOMAIN_CPU)
		return;

	i915_gem_clflush_object(obj);
	drm_agp_chipset_flush(dev);
	old_write_domain = obj->write_domain;
	obj->write_domain = 0;

	trace_i915_gem_object_change_domain(obj,
					    obj->read_domains,
					    old_write_domain);
}


int
i915_gem_object_set_to_gtt_domain(struct drm_gem_object *obj, int write)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	uint32_t old_write_domain, old_read_domains;
	int ret;

	
	if (obj_priv->gtt_space == NULL)
		return -EINVAL;

	i915_gem_object_flush_gpu_write_domain(obj);
	
	ret = i915_gem_object_wait_rendering(obj);
	if (ret != 0)
		return ret;

	old_write_domain = obj->write_domain;
	old_read_domains = obj->read_domains;

	
	if (write)
		obj->read_domains &= I915_GEM_DOMAIN_GTT;

	i915_gem_object_flush_cpu_write_domain(obj);

	
	BUG_ON((obj->write_domain & ~I915_GEM_DOMAIN_GTT) != 0);
	obj->read_domains |= I915_GEM_DOMAIN_GTT;
	if (write) {
		obj->write_domain = I915_GEM_DOMAIN_GTT;
		obj_priv->dirty = 1;
	}

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}


int
i915_gem_object_set_to_display_plane(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	uint32_t old_write_domain, old_read_domains;
	int ret;

	
	if (obj_priv->gtt_space == NULL)
		return -EINVAL;

	i915_gem_object_flush_gpu_write_domain(obj);

	
	if (obj_priv->active) {
#if WATCH_BUF
		DRM_INFO("%s: object %p wait for seqno %08x\n",
			  __func__, obj, obj_priv->last_rendering_seqno);
#endif
		ret = i915_do_wait_request(dev, obj_priv->last_rendering_seqno, 0);
		if (ret != 0)
			return ret;
	}

	old_write_domain = obj->write_domain;
	old_read_domains = obj->read_domains;

	obj->read_domains &= I915_GEM_DOMAIN_GTT;

	i915_gem_object_flush_cpu_write_domain(obj);

	
	BUG_ON((obj->write_domain & ~I915_GEM_DOMAIN_GTT) != 0);
	obj->read_domains |= I915_GEM_DOMAIN_GTT;
	obj->write_domain = I915_GEM_DOMAIN_GTT;
	obj_priv->dirty = 1;

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}


static int
i915_gem_object_set_to_cpu_domain(struct drm_gem_object *obj, int write)
{
	uint32_t old_write_domain, old_read_domains;
	int ret;

	i915_gem_object_flush_gpu_write_domain(obj);
	
	ret = i915_gem_object_wait_rendering(obj);
	if (ret != 0)
		return ret;

	i915_gem_object_flush_gtt_write_domain(obj);

	
	i915_gem_object_set_to_full_cpu_read_domain(obj);

	old_write_domain = obj->write_domain;
	old_read_domains = obj->read_domains;

	
	if ((obj->read_domains & I915_GEM_DOMAIN_CPU) == 0) {
		i915_gem_clflush_object(obj);

		obj->read_domains |= I915_GEM_DOMAIN_CPU;
	}

	
	BUG_ON((obj->write_domain & ~I915_GEM_DOMAIN_CPU) != 0);

	
	if (write) {
		obj->read_domains &= I915_GEM_DOMAIN_CPU;
		obj->write_domain = I915_GEM_DOMAIN_CPU;
	}

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}


static void
i915_gem_object_set_to_gpu_domain(struct drm_gem_object *obj)
{
	struct drm_device		*dev = obj->dev;
	struct drm_i915_gem_object	*obj_priv = obj->driver_private;
	uint32_t			invalidate_domains = 0;
	uint32_t			flush_domains = 0;
	uint32_t			old_read_domains;

	BUG_ON(obj->pending_read_domains & I915_GEM_DOMAIN_CPU);
	BUG_ON(obj->pending_write_domain == I915_GEM_DOMAIN_CPU);

	intel_mark_busy(dev, obj);

#if WATCH_BUF
	DRM_INFO("%s: object %p read %08x -> %08x write %08x -> %08x\n",
		 __func__, obj,
		 obj->read_domains, obj->pending_read_domains,
		 obj->write_domain, obj->pending_write_domain);
#endif
	
	if (obj->pending_write_domain == 0)
		obj->pending_read_domains |= obj->read_domains;
	else
		obj_priv->dirty = 1;

	
	if (obj->write_domain &&
	    obj->write_domain != obj->pending_read_domains) {
		flush_domains |= obj->write_domain;
		invalidate_domains |=
			obj->pending_read_domains & ~obj->write_domain;
	}
	
	invalidate_domains |= obj->pending_read_domains & ~obj->read_domains;
	if ((flush_domains | invalidate_domains) & I915_GEM_DOMAIN_CPU) {
#if WATCH_BUF
		DRM_INFO("%s: CPU domain flush %08x invalidate %08x\n",
			 __func__, flush_domains, invalidate_domains);
#endif
		i915_gem_clflush_object(obj);
	}

	old_read_domains = obj->read_domains;

	
	if (flush_domains == 0 && obj->pending_write_domain == 0)
		obj->pending_write_domain = obj->write_domain;
	obj->read_domains = obj->pending_read_domains;

	dev->invalidate_domains |= invalidate_domains;
	dev->flush_domains |= flush_domains;
#if WATCH_BUF
	DRM_INFO("%s: read %08x write %08x invalidate %08x flush %08x\n",
		 __func__,
		 obj->read_domains, obj->write_domain,
		 dev->invalidate_domains, dev->flush_domains);
#endif

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    obj->write_domain);
}


static void
i915_gem_object_set_to_full_cpu_read_domain(struct drm_gem_object *obj)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	if (!obj_priv->page_cpu_valid)
		return;

	
	if (obj->read_domains & I915_GEM_DOMAIN_CPU) {
		int i;

		for (i = 0; i <= (obj->size - 1) / PAGE_SIZE; i++) {
			if (obj_priv->page_cpu_valid[i])
				continue;
			drm_clflush_pages(obj_priv->pages + i, 1);
		}
	}

	
	kfree(obj_priv->page_cpu_valid);
	obj_priv->page_cpu_valid = NULL;
}


static int
i915_gem_object_set_cpu_read_domain_range(struct drm_gem_object *obj,
					  uint64_t offset, uint64_t size)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	uint32_t old_read_domains;
	int i, ret;

	if (offset == 0 && size == obj->size)
		return i915_gem_object_set_to_cpu_domain(obj, 0);

	i915_gem_object_flush_gpu_write_domain(obj);
	
	ret = i915_gem_object_wait_rendering(obj);
	if (ret != 0)
		return ret;
	i915_gem_object_flush_gtt_write_domain(obj);

	
	if (obj_priv->page_cpu_valid == NULL &&
	    (obj->read_domains & I915_GEM_DOMAIN_CPU) != 0)
		return 0;

	
	if (obj_priv->page_cpu_valid == NULL) {
		obj_priv->page_cpu_valid = kzalloc(obj->size / PAGE_SIZE,
						   GFP_KERNEL);
		if (obj_priv->page_cpu_valid == NULL)
			return -ENOMEM;
	} else if ((obj->read_domains & I915_GEM_DOMAIN_CPU) == 0)
		memset(obj_priv->page_cpu_valid, 0, obj->size / PAGE_SIZE);

	
	for (i = offset / PAGE_SIZE; i <= (offset + size - 1) / PAGE_SIZE;
	     i++) {
		if (obj_priv->page_cpu_valid[i])
			continue;

		drm_clflush_pages(obj_priv->pages + i, 1);

		obj_priv->page_cpu_valid[i] = 1;
	}

	
	BUG_ON((obj->write_domain & ~I915_GEM_DOMAIN_CPU) != 0);

	old_read_domains = obj->read_domains;
	obj->read_domains |= I915_GEM_DOMAIN_CPU;

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    obj->write_domain);

	return 0;
}


static int
i915_gem_object_pin_and_relocate(struct drm_gem_object *obj,
				 struct drm_file *file_priv,
				 struct drm_i915_gem_exec_object *entry,
				 struct drm_i915_gem_relocation_entry *relocs)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int i, ret;
	void __iomem *reloc_page;

	
	ret = i915_gem_object_pin(obj, (uint32_t) entry->alignment);
	if (ret)
		return ret;

	entry->offset = obj_priv->gtt_offset;

	
	for (i = 0; i < entry->relocation_count; i++) {
		struct drm_i915_gem_relocation_entry *reloc= &relocs[i];
		struct drm_gem_object *target_obj;
		struct drm_i915_gem_object *target_obj_priv;
		uint32_t reloc_val, reloc_offset;
		uint32_t __iomem *reloc_entry;

		target_obj = drm_gem_object_lookup(obj->dev, file_priv,
						   reloc->target_handle);
		if (target_obj == NULL) {
			i915_gem_object_unpin(obj);
			return -EBADF;
		}
		target_obj_priv = target_obj->driver_private;

#if WATCH_RELOC
		DRM_INFO("%s: obj %p offset %08x target %d "
			 "read %08x write %08x gtt %08x "
			 "presumed %08x delta %08x\n",
			 __func__,
			 obj,
			 (int) reloc->offset,
			 (int) reloc->target_handle,
			 (int) reloc->read_domains,
			 (int) reloc->write_domain,
			 (int) target_obj_priv->gtt_offset,
			 (int) reloc->presumed_offset,
			 reloc->delta);
#endif

		
		if (target_obj_priv->gtt_space == NULL) {
			DRM_ERROR("No GTT space found for object %d\n",
				  reloc->target_handle);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}

		
		if (reloc->write_domain & I915_GEM_DOMAIN_CPU ||
		    reloc->read_domains & I915_GEM_DOMAIN_CPU) {
			DRM_ERROR("reloc with read/write CPU domains: "
				  "obj %p target %d offset %d "
				  "read %08x write %08x",
				  obj, reloc->target_handle,
				  (int) reloc->offset,
				  reloc->read_domains,
				  reloc->write_domain);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}
		if (reloc->write_domain && target_obj->pending_write_domain &&
		    reloc->write_domain != target_obj->pending_write_domain) {
			DRM_ERROR("Write domain conflict: "
				  "obj %p target %d offset %d "
				  "new %08x old %08x\n",
				  obj, reloc->target_handle,
				  (int) reloc->offset,
				  reloc->write_domain,
				  target_obj->pending_write_domain);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}

		target_obj->pending_read_domains |= reloc->read_domains;
		target_obj->pending_write_domain |= reloc->write_domain;

		
		if (target_obj_priv->gtt_offset == reloc->presumed_offset) {
			drm_gem_object_unreference(target_obj);
			continue;
		}

		
		if (reloc->offset > obj->size - 4) {
			DRM_ERROR("Relocation beyond object bounds: "
				  "obj %p target %d offset %d size %d.\n",
				  obj, reloc->target_handle,
				  (int) reloc->offset, (int) obj->size);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}
		if (reloc->offset & 3) {
			DRM_ERROR("Relocation not 4-byte aligned: "
				  "obj %p target %d offset %d.\n",
				  obj, reloc->target_handle,
				  (int) reloc->offset);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}

		
		if (reloc->delta >= target_obj->size) {
			DRM_ERROR("Relocation beyond target object bounds: "
				  "obj %p target %d delta %d size %d.\n",
				  obj, reloc->target_handle,
				  (int) reloc->delta, (int) target_obj->size);
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}

		ret = i915_gem_object_set_to_gtt_domain(obj, 1);
		if (ret != 0) {
			drm_gem_object_unreference(target_obj);
			i915_gem_object_unpin(obj);
			return -EINVAL;
		}

		
		reloc_offset = obj_priv->gtt_offset + reloc->offset;
		reloc_page = io_mapping_map_atomic_wc(dev_priv->mm.gtt_mapping,
						      (reloc_offset &
						       ~(PAGE_SIZE - 1)));
		reloc_entry = (uint32_t __iomem *)(reloc_page +
						   (reloc_offset & (PAGE_SIZE - 1)));
		reloc_val = target_obj_priv->gtt_offset + reloc->delta;

#if WATCH_BUF
		DRM_INFO("Applied relocation: %p@0x%08x %08x -> %08x\n",
			  obj, (unsigned int) reloc->offset,
			  readl(reloc_entry), reloc_val);
#endif
		writel(reloc_val, reloc_entry);
		io_mapping_unmap_atomic(reloc_page);

		
		reloc->presumed_offset = target_obj_priv->gtt_offset;

		drm_gem_object_unreference(target_obj);
	}

#if WATCH_BUF
	if (0)
		i915_gem_dump_object(obj, 128, __func__, ~0);
#endif
	return 0;
}


static int
i915_dispatch_gem_execbuffer(struct drm_device *dev,
			      struct drm_i915_gem_execbuffer *exec,
			      struct drm_clip_rect *cliprects,
			      uint64_t exec_offset)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int nbox = exec->num_cliprects;
	int i = 0, count;
	uint32_t exec_start, exec_len;
	RING_LOCALS;

	exec_start = (uint32_t) exec_offset + exec->batch_start_offset;
	exec_len = (uint32_t) exec->batch_len;

	trace_i915_gem_request_submit(dev, dev_priv->mm.next_gem_seqno + 1);

	count = nbox ? nbox : 1;

	for (i = 0; i < count; i++) {
		if (i < nbox) {
			int ret = i915_emit_box(dev, cliprects, i,
						exec->DR1, exec->DR4);
			if (ret)
				return ret;
		}

		if (IS_I830(dev) || IS_845G(dev)) {
			BEGIN_LP_RING(4);
			OUT_RING(MI_BATCH_BUFFER);
			OUT_RING(exec_start | MI_BATCH_NON_SECURE);
			OUT_RING(exec_start + exec_len - 4);
			OUT_RING(0);
			ADVANCE_LP_RING();
		} else {
			BEGIN_LP_RING(2);
			if (IS_I965G(dev)) {
				OUT_RING(MI_BATCH_BUFFER_START |
					 (2 << 6) |
					 MI_BATCH_NON_SECURE_I965);
				OUT_RING(exec_start);
			} else {
				OUT_RING(MI_BATCH_BUFFER_START |
					 (2 << 6));
				OUT_RING(exec_start | MI_BATCH_NON_SECURE);
			}
			ADVANCE_LP_RING();
		}
	}

	
	return 0;
}


static int
i915_gem_ring_throttle(struct drm_device *dev, struct drm_file *file_priv)
{
	struct drm_i915_file_private *i915_file_priv = file_priv->driver_priv;
	int ret = 0;
	unsigned long recent_enough = jiffies - msecs_to_jiffies(20);

	mutex_lock(&dev->struct_mutex);
	while (!list_empty(&i915_file_priv->mm.request_list)) {
		struct drm_i915_gem_request *request;

		request = list_first_entry(&i915_file_priv->mm.request_list,
					   struct drm_i915_gem_request,
					   client_list);

		if (time_after_eq(request->emitted_jiffies, recent_enough))
			break;

		ret = i915_wait_request(dev, request->seqno);
		if (ret != 0)
			break;
	}
	mutex_unlock(&dev->struct_mutex);

	return ret;
}

static int
i915_gem_get_relocs_from_user(struct drm_i915_gem_exec_object *exec_list,
			      uint32_t buffer_count,
			      struct drm_i915_gem_relocation_entry **relocs)
{
	uint32_t reloc_count = 0, reloc_index = 0, i;
	int ret;

	*relocs = NULL;
	for (i = 0; i < buffer_count; i++) {
		if (reloc_count + exec_list[i].relocation_count < reloc_count)
			return -EINVAL;
		reloc_count += exec_list[i].relocation_count;
	}

	*relocs = drm_calloc_large(reloc_count, sizeof(**relocs));
	if (*relocs == NULL)
		return -ENOMEM;

	for (i = 0; i < buffer_count; i++) {
		struct drm_i915_gem_relocation_entry __user *user_relocs;

		user_relocs = (void __user *)(uintptr_t)exec_list[i].relocs_ptr;

		ret = copy_from_user(&(*relocs)[reloc_index],
				     user_relocs,
				     exec_list[i].relocation_count *
				     sizeof(**relocs));
		if (ret != 0) {
			drm_free_large(*relocs);
			*relocs = NULL;
			return -EFAULT;
		}

		reloc_index += exec_list[i].relocation_count;
	}

	return 0;
}

static int
i915_gem_put_relocs_to_user(struct drm_i915_gem_exec_object *exec_list,
			    uint32_t buffer_count,
			    struct drm_i915_gem_relocation_entry *relocs)
{
	uint32_t reloc_count = 0, i;
	int ret = 0;

	for (i = 0; i < buffer_count; i++) {
		struct drm_i915_gem_relocation_entry __user *user_relocs;
		int unwritten;

		user_relocs = (void __user *)(uintptr_t)exec_list[i].relocs_ptr;

		unwritten = copy_to_user(user_relocs,
					 &relocs[reloc_count],
					 exec_list[i].relocation_count *
					 sizeof(*relocs));

		if (unwritten) {
			ret = -EFAULT;
			goto err;
		}

		reloc_count += exec_list[i].relocation_count;
	}

err:
	drm_free_large(relocs);

	return ret;
}

static int
i915_gem_check_execbuffer (struct drm_i915_gem_execbuffer *exec,
			   uint64_t exec_offset)
{
	uint32_t exec_start, exec_len;

	exec_start = (uint32_t) exec_offset + exec->batch_start_offset;
	exec_len = (uint32_t) exec->batch_len;

	if ((exec_start | exec_len) & 0x7)
		return -EINVAL;

	if (!exec_start)
		return -EINVAL;

	return 0;
}

int
i915_gem_execbuffer(struct drm_device *dev, void *data,
		    struct drm_file *file_priv)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_execbuffer *args = data;
	struct drm_i915_gem_exec_object *exec_list = NULL;
	struct drm_gem_object **object_list = NULL;
	struct drm_gem_object *batch_obj;
	struct drm_i915_gem_object *obj_priv;
	struct drm_clip_rect *cliprects = NULL;
	struct drm_i915_gem_relocation_entry *relocs;
	int ret, ret2, i, pinned = 0;
	uint64_t exec_offset;
	uint32_t seqno, flush_domains, reloc_index;
	int pin_tries;

#if WATCH_EXEC
	DRM_INFO("buffers_ptr %d buffer_count %d len %08x\n",
		  (int) args->buffers_ptr, args->buffer_count, args->batch_len);
#endif

	if (args->buffer_count < 1) {
		DRM_ERROR("execbuf with %d buffers\n", args->buffer_count);
		return -EINVAL;
	}
	
	exec_list = drm_calloc_large(sizeof(*exec_list), args->buffer_count);
	object_list = drm_calloc_large(sizeof(*object_list), args->buffer_count);
	if (exec_list == NULL || object_list == NULL) {
		DRM_ERROR("Failed to allocate exec or object list "
			  "for %d buffers\n",
			  args->buffer_count);
		ret = -ENOMEM;
		goto pre_mutex_err;
	}
	ret = copy_from_user(exec_list,
			     (struct drm_i915_relocation_entry __user *)
			     (uintptr_t) args->buffers_ptr,
			     sizeof(*exec_list) * args->buffer_count);
	if (ret != 0) {
		DRM_ERROR("copy %d exec entries failed %d\n",
			  args->buffer_count, ret);
		goto pre_mutex_err;
	}

	if (args->num_cliprects != 0) {
		cliprects = kcalloc(args->num_cliprects, sizeof(*cliprects),
				    GFP_KERNEL);
		if (cliprects == NULL)
			goto pre_mutex_err;

		ret = copy_from_user(cliprects,
				     (struct drm_clip_rect __user *)
				     (uintptr_t) args->cliprects_ptr,
				     sizeof(*cliprects) * args->num_cliprects);
		if (ret != 0) {
			DRM_ERROR("copy %d cliprects failed: %d\n",
				  args->num_cliprects, ret);
			goto pre_mutex_err;
		}
	}

	ret = i915_gem_get_relocs_from_user(exec_list, args->buffer_count,
					    &relocs);
	if (ret != 0)
		goto pre_mutex_err;

	mutex_lock(&dev->struct_mutex);

	i915_verify_inactive(dev, __FILE__, __LINE__);

	if (atomic_read(&dev_priv->mm.wedged)) {
		DRM_ERROR("Execbuf while wedged\n");
		mutex_unlock(&dev->struct_mutex);
		ret = -EIO;
		goto pre_mutex_err;
	}

	if (dev_priv->mm.suspended) {
		DRM_ERROR("Execbuf while VT-switched.\n");
		mutex_unlock(&dev->struct_mutex);
		ret = -EBUSY;
		goto pre_mutex_err;
	}

	
	for (i = 0; i < args->buffer_count; i++) {
		object_list[i] = drm_gem_object_lookup(dev, file_priv,
						       exec_list[i].handle);
		if (object_list[i] == NULL) {
			DRM_ERROR("Invalid object handle %d at index %d\n",
				   exec_list[i].handle, i);
			ret = -EBADF;
			goto err;
		}

		obj_priv = object_list[i]->driver_private;
		if (obj_priv->in_execbuffer) {
			DRM_ERROR("Object %p appears more than once in object list\n",
				   object_list[i]);
			ret = -EBADF;
			goto err;
		}
		obj_priv->in_execbuffer = true;
	}

	
	for (pin_tries = 0; ; pin_tries++) {
		ret = 0;
		reloc_index = 0;

		for (i = 0; i < args->buffer_count; i++) {
			object_list[i]->pending_read_domains = 0;
			object_list[i]->pending_write_domain = 0;
			ret = i915_gem_object_pin_and_relocate(object_list[i],
							       file_priv,
							       &exec_list[i],
							       &relocs[reloc_index]);
			if (ret)
				break;
			pinned = i + 1;
			reloc_index += exec_list[i].relocation_count;
		}
		
		if (ret == 0)
			break;

		
		if (ret != -ENOSPC || pin_tries >= 1) {
			if (ret != -ERESTARTSYS) {
				unsigned long long total_size = 0;
				for (i = 0; i < args->buffer_count; i++)
					total_size += object_list[i]->size;
				DRM_ERROR("Failed to pin buffer %d of %d, total %llu bytes: %d\n",
					  pinned+1, args->buffer_count,
					  total_size, ret);
				DRM_ERROR("%d objects [%d pinned], "
					  "%d object bytes [%d pinned], "
					  "%d/%d gtt bytes\n",
					  atomic_read(&dev->object_count),
					  atomic_read(&dev->pin_count),
					  atomic_read(&dev->object_memory),
					  atomic_read(&dev->pin_memory),
					  atomic_read(&dev->gtt_memory),
					  dev->gtt_total);
			}
			goto err;
		}

		
		for (i = 0; i < pinned; i++)
			i915_gem_object_unpin(object_list[i]);
		pinned = 0;

		
		ret = i915_gem_evict_everything(dev);
		if (ret && ret != -ENOSPC)
			goto err;
	}

	
	batch_obj = object_list[args->buffer_count-1];
	if (batch_obj->pending_write_domain) {
		DRM_ERROR("Attempting to use self-modifying batch buffer\n");
		ret = -EINVAL;
		goto err;
	}
	batch_obj->pending_read_domains |= I915_GEM_DOMAIN_COMMAND;

	
	exec_offset = exec_list[args->buffer_count - 1].offset;
	ret = i915_gem_check_execbuffer (args, exec_offset);
	if (ret != 0) {
		DRM_ERROR("execbuf with invalid offset/length\n");
		goto err;
	}

	i915_verify_inactive(dev, __FILE__, __LINE__);

	
	dev->invalidate_domains = 0;
	dev->flush_domains = 0;

	for (i = 0; i < args->buffer_count; i++) {
		struct drm_gem_object *obj = object_list[i];

		
		i915_gem_object_set_to_gpu_domain(obj);
	}

	i915_verify_inactive(dev, __FILE__, __LINE__);

	if (dev->invalidate_domains | dev->flush_domains) {
#if WATCH_EXEC
		DRM_INFO("%s: invalidate_domains %08x flush_domains %08x\n",
			  __func__,
			 dev->invalidate_domains,
			 dev->flush_domains);
#endif
		i915_gem_flush(dev,
			       dev->invalidate_domains,
			       dev->flush_domains);
		if (dev->flush_domains & I915_GEM_GPU_DOMAINS)
			(void)i915_add_request(dev, file_priv,
					       dev->flush_domains);
	}

	for (i = 0; i < args->buffer_count; i++) {
		struct drm_gem_object *obj = object_list[i];
		struct drm_i915_gem_object *obj_priv = obj->driver_private;
		uint32_t old_write_domain = obj->write_domain;

		obj->write_domain = obj->pending_write_domain;
		if (obj->write_domain)
			list_move_tail(&obj_priv->gpu_write_list,
				       &dev_priv->mm.gpu_write_list);
		else
			list_del_init(&obj_priv->gpu_write_list);

		trace_i915_gem_object_change_domain(obj,
						    obj->read_domains,
						    old_write_domain);
	}

	i915_verify_inactive(dev, __FILE__, __LINE__);

#if WATCH_COHERENCY
	for (i = 0; i < args->buffer_count; i++) {
		i915_gem_object_check_coherency(object_list[i],
						exec_list[i].handle);
	}
#endif

#if WATCH_EXEC
	i915_gem_dump_object(batch_obj,
			      args->batch_len,
			      __func__,
			      ~0);
#endif

	
	ret = i915_dispatch_gem_execbuffer(dev, args, cliprects, exec_offset);
	if (ret) {
		DRM_ERROR("dispatch failed %d\n", ret);
		goto err;
	}

	
	flush_domains = i915_retire_commands(dev);

	i915_verify_inactive(dev, __FILE__, __LINE__);

	
	seqno = i915_add_request(dev, file_priv, flush_domains);
	BUG_ON(seqno == 0);
	for (i = 0; i < args->buffer_count; i++) {
		struct drm_gem_object *obj = object_list[i];

		i915_gem_object_move_to_active(obj, seqno);
#if WATCH_LRU
		DRM_INFO("%s: move to exec list %p\n", __func__, obj);
#endif
	}
#if WATCH_LRU
	i915_dump_lru(dev, __func__);
#endif

	i915_verify_inactive(dev, __FILE__, __LINE__);

err:
	for (i = 0; i < pinned; i++)
		i915_gem_object_unpin(object_list[i]);

	for (i = 0; i < args->buffer_count; i++) {
		if (object_list[i]) {
			obj_priv = object_list[i]->driver_private;
			obj_priv->in_execbuffer = false;
		}
		drm_gem_object_unreference(object_list[i]);
	}

	mutex_unlock(&dev->struct_mutex);

	if (!ret) {
		
		ret = copy_to_user((struct drm_i915_relocation_entry __user *)
				   (uintptr_t) args->buffers_ptr,
				   exec_list,
				   sizeof(*exec_list) * args->buffer_count);
		if (ret) {
			ret = -EFAULT;
			DRM_ERROR("failed to copy %d exec entries "
				  "back to user (%d)\n",
				  args->buffer_count, ret);
		}
	}

	
	ret2 = i915_gem_put_relocs_to_user(exec_list, args->buffer_count,
					   relocs);
	if (ret2 != 0) {
		DRM_ERROR("Failed to copy relocations back out: %d\n", ret2);

		if (ret == 0)
			ret = ret2;
	}

pre_mutex_err:
	drm_free_large(object_list);
	drm_free_large(exec_list);
	kfree(cliprects);

	return ret;
}

int
i915_gem_object_pin(struct drm_gem_object *obj, uint32_t alignment)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	int ret;

	i915_verify_inactive(dev, __FILE__, __LINE__);
	if (obj_priv->gtt_space == NULL) {
		ret = i915_gem_object_bind_to_gtt(obj, alignment);
		if (ret)
			return ret;
	}
	
	if (!IS_I965G(dev) && obj_priv->tiling_mode != I915_TILING_NONE) {
		ret = i915_gem_object_get_fence_reg(obj);
		if (ret != 0) {
			if (ret != -EBUSY && ret != -ERESTARTSYS)
				DRM_ERROR("Failure to install fence: %d\n",
					  ret);
			return ret;
		}
	}
	obj_priv->pin_count++;

	
	if (obj_priv->pin_count == 1) {
		atomic_inc(&dev->pin_count);
		atomic_add(obj->size, &dev->pin_memory);
		if (!obj_priv->active &&
		    (obj->write_domain & I915_GEM_GPU_DOMAINS) == 0 &&
		    !list_empty(&obj_priv->list))
			list_del_init(&obj_priv->list);
	}
	i915_verify_inactive(dev, __FILE__, __LINE__);

	return 0;
}

void
i915_gem_object_unpin(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	i915_verify_inactive(dev, __FILE__, __LINE__);
	obj_priv->pin_count--;
	BUG_ON(obj_priv->pin_count < 0);
	BUG_ON(obj_priv->gtt_space == NULL);

	
	if (obj_priv->pin_count == 0) {
		if (!obj_priv->active &&
		    (obj->write_domain & I915_GEM_GPU_DOMAINS) == 0)
			list_move_tail(&obj_priv->list,
				       &dev_priv->mm.inactive_list);
		atomic_dec(&dev->pin_count);
		atomic_sub(obj->size, &dev->pin_memory);
	}
	i915_verify_inactive(dev, __FILE__, __LINE__);
}

int
i915_gem_pin_ioctl(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_i915_gem_pin *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret;

	mutex_lock(&dev->struct_mutex);

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL) {
		DRM_ERROR("Bad handle in i915_gem_pin_ioctl(): %d\n",
			  args->handle);
		mutex_unlock(&dev->struct_mutex);
		return -EBADF;
	}
	obj_priv = obj->driver_private;

	if (obj_priv->madv != I915_MADV_WILLNEED) {
		DRM_ERROR("Attempting to pin a purgeable buffer\n");
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	if (obj_priv->pin_filp != NULL && obj_priv->pin_filp != file_priv) {
		DRM_ERROR("Already pinned in i915_gem_pin_ioctl(): %d\n",
			  args->handle);
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	obj_priv->user_pin_count++;
	obj_priv->pin_filp = file_priv;
	if (obj_priv->user_pin_count == 1) {
		ret = i915_gem_object_pin(obj, args->alignment);
		if (ret != 0) {
			drm_gem_object_unreference(obj);
			mutex_unlock(&dev->struct_mutex);
			return ret;
		}
	}

	
	i915_gem_object_flush_cpu_write_domain(obj);
	args->offset = obj_priv->gtt_offset;
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}

int
i915_gem_unpin_ioctl(struct drm_device *dev, void *data,
		     struct drm_file *file_priv)
{
	struct drm_i915_gem_pin *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;

	mutex_lock(&dev->struct_mutex);

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL) {
		DRM_ERROR("Bad handle in i915_gem_unpin_ioctl(): %d\n",
			  args->handle);
		mutex_unlock(&dev->struct_mutex);
		return -EBADF;
	}

	obj_priv = obj->driver_private;
	if (obj_priv->pin_filp != file_priv) {
		DRM_ERROR("Not pinned by caller in i915_gem_pin_ioctl(): %d\n",
			  args->handle);
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}
	obj_priv->user_pin_count--;
	if (obj_priv->user_pin_count == 0) {
		obj_priv->pin_filp = NULL;
		i915_gem_object_unpin(obj);
	}

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	return 0;
}

int
i915_gem_busy_ioctl(struct drm_device *dev, void *data,
		    struct drm_file *file_priv)
{
	struct drm_i915_gem_busy *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL) {
		DRM_ERROR("Bad handle in i915_gem_busy_ioctl(): %d\n",
			  args->handle);
		return -EBADF;
	}

	mutex_lock(&dev->struct_mutex);
	
	i915_gem_retire_requests(dev);

	obj_priv = obj->driver_private;
	
	args->busy = obj_priv->active && obj_priv->last_rendering_seqno != 0;

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	return 0;
}

int
i915_gem_throttle_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv)
{
    return i915_gem_ring_throttle(dev, file_priv);
}

int
i915_gem_madvise_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	struct drm_i915_gem_madvise *args = data;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;

	switch (args->madv) {
	case I915_MADV_DONTNEED:
	case I915_MADV_WILLNEED:
	    break;
	default:
	    return -EINVAL;
	}

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL) {
		DRM_ERROR("Bad handle in i915_gem_madvise_ioctl(): %d\n",
			  args->handle);
		return -EBADF;
	}

	mutex_lock(&dev->struct_mutex);
	obj_priv = obj->driver_private;

	if (obj_priv->pin_count) {
		drm_gem_object_unreference(obj);
		mutex_unlock(&dev->struct_mutex);

		DRM_ERROR("Attempted i915_gem_madvise_ioctl() on a pinned object\n");
		return -EINVAL;
	}

	if (obj_priv->madv != __I915_MADV_PURGED)
		obj_priv->madv = args->madv;

	
	if (i915_gem_object_is_purgeable(obj_priv) &&
	    obj_priv->gtt_space == NULL)
		i915_gem_object_truncate(obj);

	args->retained = obj_priv->madv != __I915_MADV_PURGED;

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}

int i915_gem_init_object(struct drm_gem_object *obj)
{
	struct drm_i915_gem_object *obj_priv;

	obj_priv = kzalloc(sizeof(*obj_priv), GFP_KERNEL);
	if (obj_priv == NULL)
		return -ENOMEM;

	
	obj->write_domain = I915_GEM_DOMAIN_CPU;
	obj->read_domains = I915_GEM_DOMAIN_CPU;

	obj_priv->agp_type = AGP_USER_MEMORY;

	obj->driver_private = obj_priv;
	obj_priv->obj = obj;
	obj_priv->fence_reg = I915_FENCE_REG_NONE;
	INIT_LIST_HEAD(&obj_priv->list);
	INIT_LIST_HEAD(&obj_priv->gpu_write_list);
	INIT_LIST_HEAD(&obj_priv->fence_list);
	obj_priv->madv = I915_MADV_WILLNEED;

	trace_i915_gem_object_create(obj);

	return 0;
}

void i915_gem_free_object(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct drm_i915_gem_object *obj_priv = obj->driver_private;

	trace_i915_gem_object_destroy(obj);

	while (obj_priv->pin_count > 0)
		i915_gem_object_unpin(obj);

	if (obj_priv->phys_obj)
		i915_gem_detach_phys_object(dev, obj);

	i915_gem_object_unbind(obj);

	if (obj_priv->mmap_offset)
		i915_gem_free_mmap_offset(obj);

	kfree(obj_priv->page_cpu_valid);
	kfree(obj_priv->bit_17);
	kfree(obj->driver_private);
}


static int
i915_gem_evict_from_inactive_list(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	while (!list_empty(&dev_priv->mm.inactive_list)) {
		struct drm_gem_object *obj;
		int ret;

		obj = list_first_entry(&dev_priv->mm.inactive_list,
				       struct drm_i915_gem_object,
				       list)->obj;

		ret = i915_gem_object_unbind(obj);
		if (ret != 0) {
			DRM_ERROR("Error unbinding object: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

int
i915_gem_idle(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	uint32_t seqno, cur_seqno, last_seqno;
	int stuck, ret;

	mutex_lock(&dev->struct_mutex);

	if (dev_priv->mm.suspended || dev_priv->ring.ring_obj == NULL) {
		mutex_unlock(&dev->struct_mutex);
		return 0;
	}

	
	dev_priv->mm.suspended = 1;
	del_timer(&dev_priv->hangcheck_timer);

	
	mutex_unlock(&dev->struct_mutex);
	cancel_delayed_work_sync(&dev_priv->mm.retire_work);
	mutex_lock(&dev->struct_mutex);

	i915_kernel_lost_context(dev);

	
	i915_gem_flush(dev, I915_GEM_GPU_DOMAINS, I915_GEM_GPU_DOMAINS);
	seqno = i915_add_request(dev, NULL, I915_GEM_GPU_DOMAINS);

	if (seqno == 0) {
		mutex_unlock(&dev->struct_mutex);
		return -ENOMEM;
	}

	dev_priv->mm.waiting_gem_seqno = seqno;
	last_seqno = 0;
	stuck = 0;
	for (;;) {
		cur_seqno = i915_get_gem_seqno(dev);
		if (i915_seqno_passed(cur_seqno, seqno))
			break;
		if (last_seqno == cur_seqno) {
			if (stuck++ > 100) {
				DRM_ERROR("hardware wedged\n");
				atomic_set(&dev_priv->mm.wedged, 1);
				DRM_WAKEUP(&dev_priv->irq_queue);
				break;
			}
		}
		msleep(10);
		last_seqno = cur_seqno;
	}
	dev_priv->mm.waiting_gem_seqno = 0;

	i915_gem_retire_requests(dev);

	spin_lock(&dev_priv->mm.active_list_lock);
	if (!atomic_read(&dev_priv->mm.wedged)) {
		
		WARN_ON(!list_empty(&dev_priv->mm.active_list));
		WARN_ON(!list_empty(&dev_priv->mm.flushing_list));
		
		WARN_ON(!list_empty(&dev_priv->mm.request_list));
	}

	
	while (!list_empty(&dev_priv->mm.active_list)) {
		struct drm_gem_object *obj;
		uint32_t old_write_domain;

		obj = list_first_entry(&dev_priv->mm.active_list,
				       struct drm_i915_gem_object,
				       list)->obj;
		old_write_domain = obj->write_domain;
		obj->write_domain &= ~I915_GEM_GPU_DOMAINS;
		i915_gem_object_move_to_inactive(obj);

		trace_i915_gem_object_change_domain(obj,
						    obj->read_domains,
						    old_write_domain);
	}
	spin_unlock(&dev_priv->mm.active_list_lock);

	while (!list_empty(&dev_priv->mm.flushing_list)) {
		struct drm_gem_object *obj;
		uint32_t old_write_domain;

		obj = list_first_entry(&dev_priv->mm.flushing_list,
				       struct drm_i915_gem_object,
				       list)->obj;
		old_write_domain = obj->write_domain;
		obj->write_domain &= ~I915_GEM_GPU_DOMAINS;
		i915_gem_object_move_to_inactive(obj);

		trace_i915_gem_object_change_domain(obj,
						    obj->read_domains,
						    old_write_domain);
	}


	
	ret = i915_gem_evict_from_inactive_list(dev);
	WARN_ON(!list_empty(&dev_priv->mm.inactive_list));
	if (ret) {
		mutex_unlock(&dev->struct_mutex);
		return ret;
	}

	i915_gem_cleanup_ringbuffer(dev);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static int
i915_gem_init_hws(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	int ret;

	
	if (!I915_NEED_GFX_HWS(dev))
		return 0;

	obj = drm_gem_object_alloc(dev, 4096);
	if (obj == NULL) {
		DRM_ERROR("Failed to allocate status page\n");
		return -ENOMEM;
	}
	obj_priv = obj->driver_private;
	obj_priv->agp_type = AGP_USER_CACHED_MEMORY;

	ret = i915_gem_object_pin(obj, 4096);
	if (ret != 0) {
		drm_gem_object_unreference(obj);
		return ret;
	}

	dev_priv->status_gfx_addr = obj_priv->gtt_offset;

	dev_priv->hw_status_page = kmap(obj_priv->pages[0]);
	if (dev_priv->hw_status_page == NULL) {
		DRM_ERROR("Failed to map status page.\n");
		memset(&dev_priv->hws_map, 0, sizeof(dev_priv->hws_map));
		i915_gem_object_unpin(obj);
		drm_gem_object_unreference(obj);
		return -EINVAL;
	}
	dev_priv->hws_obj = obj;
	memset(dev_priv->hw_status_page, 0, PAGE_SIZE);
	I915_WRITE(HWS_PGA, dev_priv->status_gfx_addr);
	I915_READ(HWS_PGA); 
	DRM_DEBUG("hws offset: 0x%08x\n", dev_priv->status_gfx_addr);

	return 0;
}

static void
i915_gem_cleanup_hws(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;

	if (dev_priv->hws_obj == NULL)
		return;

	obj = dev_priv->hws_obj;
	obj_priv = obj->driver_private;

	kunmap(obj_priv->pages[0]);
	i915_gem_object_unpin(obj);
	drm_gem_object_unreference(obj);
	dev_priv->hws_obj = NULL;

	memset(&dev_priv->hws_map, 0, sizeof(dev_priv->hws_map));
	dev_priv->hw_status_page = NULL;

	
	I915_WRITE(HWS_PGA, 0x1ffff000);
}

int
i915_gem_init_ringbuffer(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct drm_i915_gem_object *obj_priv;
	drm_i915_ring_buffer_t *ring = &dev_priv->ring;
	int ret;
	u32 head;

	ret = i915_gem_init_hws(dev);
	if (ret != 0)
		return ret;

	obj = drm_gem_object_alloc(dev, 128 * 1024);
	if (obj == NULL) {
		DRM_ERROR("Failed to allocate ringbuffer\n");
		i915_gem_cleanup_hws(dev);
		return -ENOMEM;
	}
	obj_priv = obj->driver_private;

	ret = i915_gem_object_pin(obj, 4096);
	if (ret != 0) {
		drm_gem_object_unreference(obj);
		i915_gem_cleanup_hws(dev);
		return ret;
	}

	
	ring->Size = obj->size;

	ring->map.offset = dev->agp->base + obj_priv->gtt_offset;
	ring->map.size = obj->size;
	ring->map.type = 0;
	ring->map.flags = 0;
	ring->map.mtrr = 0;

	drm_core_ioremap_wc(&ring->map, dev);
	if (ring->map.handle == NULL) {
		DRM_ERROR("Failed to map ringbuffer.\n");
		memset(&dev_priv->ring, 0, sizeof(dev_priv->ring));
		i915_gem_object_unpin(obj);
		drm_gem_object_unreference(obj);
		i915_gem_cleanup_hws(dev);
		return -EINVAL;
	}
	ring->ring_obj = obj;
	ring->virtual_start = ring->map.handle;

	
	I915_WRITE(PRB0_CTL, 0);
	I915_WRITE(PRB0_TAIL, 0);
	I915_WRITE(PRB0_HEAD, 0);

	
	I915_WRITE(PRB0_START, obj_priv->gtt_offset);
	head = I915_READ(PRB0_HEAD) & HEAD_ADDR;

	
	if (head != 0) {
		DRM_ERROR("Ring head not reset to zero "
			  "ctl %08x head %08x tail %08x start %08x\n",
			  I915_READ(PRB0_CTL),
			  I915_READ(PRB0_HEAD),
			  I915_READ(PRB0_TAIL),
			  I915_READ(PRB0_START));
		I915_WRITE(PRB0_HEAD, 0);

		DRM_ERROR("Ring head forced to zero "
			  "ctl %08x head %08x tail %08x start %08x\n",
			  I915_READ(PRB0_CTL),
			  I915_READ(PRB0_HEAD),
			  I915_READ(PRB0_TAIL),
			  I915_READ(PRB0_START));
	}

	I915_WRITE(PRB0_CTL,
		   ((obj->size - 4096) & RING_NR_PAGES) |
		   RING_NO_REPORT |
		   RING_VALID);

	head = I915_READ(PRB0_HEAD) & HEAD_ADDR;

	
	if (head != 0) {
		DRM_ERROR("Ring initialization failed "
			  "ctl %08x head %08x tail %08x start %08x\n",
			  I915_READ(PRB0_CTL),
			  I915_READ(PRB0_HEAD),
			  I915_READ(PRB0_TAIL),
			  I915_READ(PRB0_START));
		return -EIO;
	}

	
	if (!drm_core_check_feature(dev, DRIVER_MODESET))
		i915_kernel_lost_context(dev);
	else {
		ring->head = I915_READ(PRB0_HEAD) & HEAD_ADDR;
		ring->tail = I915_READ(PRB0_TAIL) & TAIL_ADDR;
		ring->space = ring->head - (ring->tail + 8);
		if (ring->space < 0)
			ring->space += ring->Size;
	}

	return 0;
}

void
i915_gem_cleanup_ringbuffer(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	if (dev_priv->ring.ring_obj == NULL)
		return;

	drm_core_ioremapfree(&dev_priv->ring.map, dev);

	i915_gem_object_unpin(dev_priv->ring.ring_obj);
	drm_gem_object_unreference(dev_priv->ring.ring_obj);
	dev_priv->ring.ring_obj = NULL;
	memset(&dev_priv->ring, 0, sizeof(dev_priv->ring));

	i915_gem_cleanup_hws(dev);
}

int
i915_gem_entervt_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret;

	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return 0;

	if (atomic_read(&dev_priv->mm.wedged)) {
		DRM_ERROR("Reenabling wedged hardware, good luck\n");
		atomic_set(&dev_priv->mm.wedged, 0);
	}

	mutex_lock(&dev->struct_mutex);
	dev_priv->mm.suspended = 0;

	ret = i915_gem_init_ringbuffer(dev);
	if (ret != 0) {
		mutex_unlock(&dev->struct_mutex);
		return ret;
	}

	spin_lock(&dev_priv->mm.active_list_lock);
	BUG_ON(!list_empty(&dev_priv->mm.active_list));
	spin_unlock(&dev_priv->mm.active_list_lock);

	BUG_ON(!list_empty(&dev_priv->mm.flushing_list));
	BUG_ON(!list_empty(&dev_priv->mm.inactive_list));
	BUG_ON(!list_empty(&dev_priv->mm.request_list));
	mutex_unlock(&dev->struct_mutex);

	drm_irq_install(dev);

	return 0;
}

int
i915_gem_leavevt_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return 0;

	drm_irq_uninstall(dev);
	return i915_gem_idle(dev);
}

void
i915_gem_lastclose(struct drm_device *dev)
{
	int ret;

	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return;

	ret = i915_gem_idle(dev);
	if (ret)
		DRM_ERROR("failed to idle hardware: %d\n", ret);
}

void
i915_gem_load(struct drm_device *dev)
{
	int i;
	drm_i915_private_t *dev_priv = dev->dev_private;

	spin_lock_init(&dev_priv->mm.active_list_lock);
	INIT_LIST_HEAD(&dev_priv->mm.active_list);
	INIT_LIST_HEAD(&dev_priv->mm.flushing_list);
	INIT_LIST_HEAD(&dev_priv->mm.gpu_write_list);
	INIT_LIST_HEAD(&dev_priv->mm.inactive_list);
	INIT_LIST_HEAD(&dev_priv->mm.request_list);
	INIT_LIST_HEAD(&dev_priv->mm.fence_list);
	INIT_DELAYED_WORK(&dev_priv->mm.retire_work,
			  i915_gem_retire_work_handler);
	dev_priv->mm.next_gem_seqno = 1;

	spin_lock(&shrink_list_lock);
	list_add(&dev_priv->mm.shrink_list, &shrink_list);
	spin_unlock(&shrink_list_lock);

	
	dev_priv->fence_reg_start = 3;

	if (IS_I965G(dev) || IS_I945G(dev) || IS_I945GM(dev) || IS_G33(dev))
		dev_priv->num_fence_regs = 16;
	else
		dev_priv->num_fence_regs = 8;

	
	if (IS_I965G(dev)) {
		for (i = 0; i < 16; i++)
			I915_WRITE64(FENCE_REG_965_0 + (i * 8), 0);
	} else {
		for (i = 0; i < 8; i++)
			I915_WRITE(FENCE_REG_830_0 + (i * 4), 0);
		if (IS_I945G(dev) || IS_I945GM(dev) || IS_G33(dev))
			for (i = 0; i < 8; i++)
				I915_WRITE(FENCE_REG_945_8 + (i * 4), 0);
	}

	i915_gem_detect_bit_6_swizzle(dev);
}


int i915_gem_init_phys_object(struct drm_device *dev,
			      int id, int size)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_phys_object *phys_obj;
	int ret;

	if (dev_priv->mm.phys_objs[id - 1] || !size)
		return 0;

	phys_obj = kzalloc(sizeof(struct drm_i915_gem_phys_object), GFP_KERNEL);
	if (!phys_obj)
		return -ENOMEM;

	phys_obj->id = id;

	phys_obj->handle = drm_pci_alloc(dev, size, 0);
	if (!phys_obj->handle) {
		ret = -ENOMEM;
		goto kfree_obj;
	}
#ifdef CONFIG_X86
	set_memory_wc((unsigned long)phys_obj->handle->vaddr, phys_obj->handle->size / PAGE_SIZE);
#endif

	dev_priv->mm.phys_objs[id - 1] = phys_obj;

	return 0;
kfree_obj:
	kfree(phys_obj);
	return ret;
}

void i915_gem_free_phys_object(struct drm_device *dev, int id)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_phys_object *phys_obj;

	if (!dev_priv->mm.phys_objs[id - 1])
		return;

	phys_obj = dev_priv->mm.phys_objs[id - 1];
	if (phys_obj->cur_obj) {
		i915_gem_detach_phys_object(dev, phys_obj->cur_obj);
	}

#ifdef CONFIG_X86
	set_memory_wb((unsigned long)phys_obj->handle->vaddr, phys_obj->handle->size / PAGE_SIZE);
#endif
	drm_pci_free(dev, phys_obj->handle);
	kfree(phys_obj);
	dev_priv->mm.phys_objs[id - 1] = NULL;
}

void i915_gem_free_all_phys_object(struct drm_device *dev)
{
	int i;

	for (i = I915_GEM_PHYS_CURSOR_0; i <= I915_MAX_PHYS_OBJECT; i++)
		i915_gem_free_phys_object(dev, i);
}

void i915_gem_detach_phys_object(struct drm_device *dev,
				 struct drm_gem_object *obj)
{
	struct drm_i915_gem_object *obj_priv;
	int i;
	int ret;
	int page_count;

	obj_priv = obj->driver_private;
	if (!obj_priv->phys_obj)
		return;

	ret = i915_gem_object_get_pages(obj, 0);
	if (ret)
		goto out;

	page_count = obj->size / PAGE_SIZE;

	for (i = 0; i < page_count; i++) {
		char *dst = kmap_atomic(obj_priv->pages[i], KM_USER0);
		char *src = obj_priv->phys_obj->handle->vaddr + (i * PAGE_SIZE);

		memcpy(dst, src, PAGE_SIZE);
		kunmap_atomic(dst, KM_USER0);
	}
	drm_clflush_pages(obj_priv->pages, page_count);
	drm_agp_chipset_flush(dev);

	i915_gem_object_put_pages(obj);
out:
	obj_priv->phys_obj->cur_obj = NULL;
	obj_priv->phys_obj = NULL;
}

int
i915_gem_attach_phys_object(struct drm_device *dev,
			    struct drm_gem_object *obj, int id)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj_priv;
	int ret = 0;
	int page_count;
	int i;

	if (id > I915_MAX_PHYS_OBJECT)
		return -EINVAL;

	obj_priv = obj->driver_private;

	if (obj_priv->phys_obj) {
		if (obj_priv->phys_obj->id == id)
			return 0;
		i915_gem_detach_phys_object(dev, obj);
	}


	
	if (!dev_priv->mm.phys_objs[id - 1]) {
		ret = i915_gem_init_phys_object(dev, id,
						obj->size);
		if (ret) {
			DRM_ERROR("failed to init phys object %d size: %zu\n", id, obj->size);
			goto out;
		}
	}

	
	obj_priv->phys_obj = dev_priv->mm.phys_objs[id - 1];
	obj_priv->phys_obj->cur_obj = obj;

	ret = i915_gem_object_get_pages(obj, 0);
	if (ret) {
		DRM_ERROR("failed to get page list\n");
		goto out;
	}

	page_count = obj->size / PAGE_SIZE;

	for (i = 0; i < page_count; i++) {
		char *src = kmap_atomic(obj_priv->pages[i], KM_USER0);
		char *dst = obj_priv->phys_obj->handle->vaddr + (i * PAGE_SIZE);

		memcpy(dst, src, PAGE_SIZE);
		kunmap_atomic(src, KM_USER0);
	}

	i915_gem_object_put_pages(obj);

	return 0;
out:
	return ret;
}

static int
i915_gem_phys_pwrite(struct drm_device *dev, struct drm_gem_object *obj,
		     struct drm_i915_gem_pwrite *args,
		     struct drm_file *file_priv)
{
	struct drm_i915_gem_object *obj_priv = obj->driver_private;
	void *obj_addr;
	int ret;
	char __user *user_data;

	user_data = (char __user *) (uintptr_t) args->data_ptr;
	obj_addr = obj_priv->phys_obj->handle->vaddr + args->offset;

	DRM_DEBUG("obj_addr %p, %lld\n", obj_addr, args->size);
	ret = copy_from_user(obj_addr, user_data, args->size);
	if (ret)
		return -EFAULT;

	drm_agp_chipset_flush(dev);
	return 0;
}

void i915_gem_release(struct drm_device * dev, struct drm_file *file_priv)
{
	struct drm_i915_file_private *i915_file_priv = file_priv->driver_priv;

	
	mutex_lock(&dev->struct_mutex);
	while (!list_empty(&i915_file_priv->mm.request_list))
		list_del_init(i915_file_priv->mm.request_list.next);
	mutex_unlock(&dev->struct_mutex);
}

static int
i915_gem_shrink(int nr_to_scan, gfp_t gfp_mask)
{
	drm_i915_private_t *dev_priv, *next_dev;
	struct drm_i915_gem_object *obj_priv, *next_obj;
	int cnt = 0;
	int would_deadlock = 1;

	
	if (nr_to_scan == 0) {
		spin_lock(&shrink_list_lock);
		list_for_each_entry(dev_priv, &shrink_list, mm.shrink_list) {
			struct drm_device *dev = dev_priv->dev;

			if (mutex_trylock(&dev->struct_mutex)) {
				list_for_each_entry(obj_priv,
						    &dev_priv->mm.inactive_list,
						    list)
					cnt++;
				mutex_unlock(&dev->struct_mutex);
			}
		}
		spin_unlock(&shrink_list_lock);

		return (cnt / 100) * sysctl_vfs_cache_pressure;
	}

	spin_lock(&shrink_list_lock);

	
	list_for_each_entry_safe(dev_priv, next_dev,
				 &shrink_list, mm.shrink_list) {
		struct drm_device *dev = dev_priv->dev;

		if (! mutex_trylock(&dev->struct_mutex))
			continue;

		spin_unlock(&shrink_list_lock);

		i915_gem_retire_requests(dev);

		list_for_each_entry_safe(obj_priv, next_obj,
					 &dev_priv->mm.inactive_list,
					 list) {
			if (i915_gem_object_is_purgeable(obj_priv)) {
				i915_gem_object_unbind(obj_priv->obj);
				if (--nr_to_scan <= 0)
					break;
			}
		}

		spin_lock(&shrink_list_lock);
		mutex_unlock(&dev->struct_mutex);

		would_deadlock = 0;

		if (nr_to_scan <= 0)
			break;
	}

	
	list_for_each_entry_safe(dev_priv, next_dev,
				 &shrink_list, mm.shrink_list) {
		struct drm_device *dev = dev_priv->dev;

		if (! mutex_trylock(&dev->struct_mutex))
			continue;

		spin_unlock(&shrink_list_lock);

		list_for_each_entry_safe(obj_priv, next_obj,
					 &dev_priv->mm.inactive_list,
					 list) {
			if (nr_to_scan > 0) {
				i915_gem_object_unbind(obj_priv->obj);
				nr_to_scan--;
			} else
				cnt++;
		}

		spin_lock(&shrink_list_lock);
		mutex_unlock(&dev->struct_mutex);

		would_deadlock = 0;
	}

	spin_unlock(&shrink_list_lock);

	if (would_deadlock)
		return -1;
	else if (cnt > 0)
		return (cnt / 100) * sysctl_vfs_cache_pressure;
	else
		return 0;
}

static struct shrinker shrinker = {
	.shrink = i915_gem_shrink,
	.seeks = DEFAULT_SEEKS,
};

__init void
i915_gem_shrinker_init(void)
{
    register_shrinker(&shrinker);
}

__exit void
i915_gem_shrinker_exit(void)
{
    unregister_shrinker(&shrinker);
}
