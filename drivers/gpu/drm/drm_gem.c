

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include "drmP.h"







#if BITS_PER_LONG == 64
#define DRM_FILE_PAGE_OFFSET_START ((0xFFFFFFFFUL >> PAGE_SHIFT) + 1)
#define DRM_FILE_PAGE_OFFSET_SIZE ((0xFFFFFFFFUL >> PAGE_SHIFT) * 16)
#else
#define DRM_FILE_PAGE_OFFSET_START ((0xFFFFFFFUL >> PAGE_SHIFT) + 1)
#define DRM_FILE_PAGE_OFFSET_SIZE ((0xFFFFFFFUL >> PAGE_SHIFT) * 16)
#endif



int
drm_gem_init(struct drm_device *dev)
{
	struct drm_gem_mm *mm;

	spin_lock_init(&dev->object_name_lock);
	idr_init(&dev->object_name_idr);
	atomic_set(&dev->object_count, 0);
	atomic_set(&dev->object_memory, 0);
	atomic_set(&dev->pin_count, 0);
	atomic_set(&dev->pin_memory, 0);
	atomic_set(&dev->gtt_count, 0);
	atomic_set(&dev->gtt_memory, 0);

	mm = kzalloc(sizeof(struct drm_gem_mm), GFP_KERNEL);
	if (!mm) {
		DRM_ERROR("out of memory\n");
		return -ENOMEM;
	}

	dev->mm_private = mm;

	if (drm_ht_create(&mm->offset_hash, 19)) {
		kfree(mm);
		return -ENOMEM;
	}

	if (drm_mm_init(&mm->offset_manager, DRM_FILE_PAGE_OFFSET_START,
			DRM_FILE_PAGE_OFFSET_SIZE)) {
		drm_ht_remove(&mm->offset_hash);
		kfree(mm);
		return -ENOMEM;
	}

	return 0;
}

void
drm_gem_destroy(struct drm_device *dev)
{
	struct drm_gem_mm *mm = dev->mm_private;

	drm_mm_takedown(&mm->offset_manager);
	drm_ht_remove(&mm->offset_hash);
	kfree(mm);
	dev->mm_private = NULL;
}


struct drm_gem_object *
drm_gem_object_alloc(struct drm_device *dev, size_t size)
{
	struct drm_gem_object *obj;

	BUG_ON((size & (PAGE_SIZE - 1)) != 0);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		goto free;

	obj->dev = dev;
	obj->filp = shmem_file_setup("drm mm object", size, VM_NORESERVE);
	if (IS_ERR(obj->filp))
		goto free;

	kref_init(&obj->refcount);
	kref_init(&obj->handlecount);
	obj->size = size;
	if (dev->driver->gem_init_object != NULL &&
	    dev->driver->gem_init_object(obj) != 0) {
		goto fput;
	}
	atomic_inc(&dev->object_count);
	atomic_add(obj->size, &dev->object_memory);
	return obj;
fput:
	fput(obj->filp);
free:
	kfree(obj);
	return NULL;
}
EXPORT_SYMBOL(drm_gem_object_alloc);


static int
drm_gem_handle_delete(struct drm_file *filp, u32 handle)
{
	struct drm_device *dev;
	struct drm_gem_object *obj;

	
	spin_lock(&filp->table_lock);

	
	obj = idr_find(&filp->object_idr, handle);
	if (obj == NULL) {
		spin_unlock(&filp->table_lock);
		return -EINVAL;
	}
	dev = obj->dev;

	
	idr_remove(&filp->object_idr, handle);
	spin_unlock(&filp->table_lock);

	mutex_lock(&dev->struct_mutex);
	drm_gem_object_handle_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}


int
drm_gem_handle_create(struct drm_file *file_priv,
		       struct drm_gem_object *obj,
		       u32 *handlep)
{
	int	ret;

	
again:
	
	if (idr_pre_get(&file_priv->object_idr, GFP_KERNEL) == 0)
		return -ENOMEM;

	
	spin_lock(&file_priv->table_lock);
	ret = idr_get_new_above(&file_priv->object_idr, obj, 1, (int *)handlep);
	spin_unlock(&file_priv->table_lock);
	if (ret == -EAGAIN)
		goto again;

	if (ret != 0)
		return ret;

	drm_gem_object_handle_reference(obj);
	return 0;
}
EXPORT_SYMBOL(drm_gem_handle_create);


struct drm_gem_object *
drm_gem_object_lookup(struct drm_device *dev, struct drm_file *filp,
		      u32 handle)
{
	struct drm_gem_object *obj;

	spin_lock(&filp->table_lock);

	
	obj = idr_find(&filp->object_idr, handle);
	if (obj == NULL) {
		spin_unlock(&filp->table_lock);
		return NULL;
	}

	drm_gem_object_reference(obj);

	spin_unlock(&filp->table_lock);

	return obj;
}
EXPORT_SYMBOL(drm_gem_object_lookup);


int
drm_gem_close_ioctl(struct drm_device *dev, void *data,
		    struct drm_file *file_priv)
{
	struct drm_gem_close *args = data;
	int ret;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	ret = drm_gem_handle_delete(file_priv, args->handle);

	return ret;
}


int
drm_gem_flink_ioctl(struct drm_device *dev, void *data,
		    struct drm_file *file_priv)
{
	struct drm_gem_flink *args = data;
	struct drm_gem_object *obj;
	int ret;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (obj == NULL)
		return -EBADF;

again:
	if (idr_pre_get(&dev->object_name_idr, GFP_KERNEL) == 0) {
		ret = -ENOMEM;
		goto err;
	}

	spin_lock(&dev->object_name_lock);
	if (!obj->name) {
		ret = idr_get_new_above(&dev->object_name_idr, obj, 1,
					&obj->name);
		args->name = (uint64_t) obj->name;
		spin_unlock(&dev->object_name_lock);

		if (ret == -EAGAIN)
			goto again;

		if (ret != 0)
			goto err;

		
		drm_gem_object_reference(obj);
	} else {
		args->name = (uint64_t) obj->name;
		spin_unlock(&dev->object_name_lock);
		ret = 0;
	}

err:
	mutex_lock(&dev->struct_mutex);
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	return ret;
}


int
drm_gem_open_ioctl(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_gem_open *args = data;
	struct drm_gem_object *obj;
	int ret;
	u32 handle;

	if (!(dev->driver->driver_features & DRIVER_GEM))
		return -ENODEV;

	spin_lock(&dev->object_name_lock);
	obj = idr_find(&dev->object_name_idr, (int) args->name);
	if (obj)
		drm_gem_object_reference(obj);
	spin_unlock(&dev->object_name_lock);
	if (!obj)
		return -ENOENT;

	ret = drm_gem_handle_create(file_priv, obj, &handle);
	mutex_lock(&dev->struct_mutex);
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
	if (ret)
		return ret;

	args->handle = handle;
	args->size = obj->size;

	return 0;
}


void
drm_gem_open(struct drm_device *dev, struct drm_file *file_private)
{
	idr_init(&file_private->object_idr);
	spin_lock_init(&file_private->table_lock);
}


static int
drm_gem_object_release_handle(int id, void *ptr, void *data)
{
	struct drm_gem_object *obj = ptr;

	drm_gem_object_handle_unreference(obj);

	return 0;
}


void
drm_gem_release(struct drm_device *dev, struct drm_file *file_private)
{
	mutex_lock(&dev->struct_mutex);
	idr_for_each(&file_private->object_idr,
		     &drm_gem_object_release_handle, NULL);

	idr_destroy(&file_private->object_idr);
	mutex_unlock(&dev->struct_mutex);
}


void
drm_gem_object_free(struct kref *kref)
{
	struct drm_gem_object *obj = (struct drm_gem_object *) kref;
	struct drm_device *dev = obj->dev;

	BUG_ON(!mutex_is_locked(&dev->struct_mutex));

	if (dev->driver->gem_free_object != NULL)
		dev->driver->gem_free_object(obj);

	fput(obj->filp);
	atomic_dec(&dev->object_count);
	atomic_sub(obj->size, &dev->object_memory);
	kfree(obj);
}
EXPORT_SYMBOL(drm_gem_object_free);


void
drm_gem_object_handle_free(struct kref *kref)
{
	struct drm_gem_object *obj = container_of(kref,
						  struct drm_gem_object,
						  handlecount);
	struct drm_device *dev = obj->dev;

	
	spin_lock(&dev->object_name_lock);
	if (obj->name) {
		idr_remove(&dev->object_name_idr, obj->name);
		obj->name = 0;
		spin_unlock(&dev->object_name_lock);
		
		drm_gem_object_unreference(obj);
	} else
		spin_unlock(&dev->object_name_lock);

}
EXPORT_SYMBOL(drm_gem_object_handle_free);

void drm_gem_vm_open(struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = vma->vm_private_data;

	drm_gem_object_reference(obj);
}
EXPORT_SYMBOL(drm_gem_vm_open);

void drm_gem_vm_close(struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct drm_device *dev = obj->dev;

	mutex_lock(&dev->struct_mutex);
	drm_vm_close_locked(vma);
	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);
}
EXPORT_SYMBOL(drm_gem_vm_close);



int drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_file *priv = filp->private_data;
	struct drm_device *dev = priv->minor->dev;
	struct drm_gem_mm *mm = dev->mm_private;
	struct drm_local_map *map = NULL;
	struct drm_gem_object *obj;
	struct drm_hash_item *hash;
	int ret = 0;

	mutex_lock(&dev->struct_mutex);

	if (drm_ht_find_item(&mm->offset_hash, vma->vm_pgoff, &hash)) {
		mutex_unlock(&dev->struct_mutex);
		return drm_mmap(filp, vma);
	}

	map = drm_hash_entry(hash, struct drm_map_list, hash)->map;
	if (!map ||
	    ((map->flags & _DRM_RESTRICTED) && !capable(CAP_SYS_ADMIN))) {
		ret =  -EPERM;
		goto out_unlock;
	}

	
	if (map->size < vma->vm_end - vma->vm_start) {
		ret = -EINVAL;
		goto out_unlock;
	}

	obj = map->handle;
	if (!obj->dev->driver->gem_vm_ops) {
		ret = -EINVAL;
		goto out_unlock;
	}

	vma->vm_flags |= VM_RESERVED | VM_IO | VM_PFNMAP | VM_DONTEXPAND;
	vma->vm_ops = obj->dev->driver->gem_vm_ops;
	vma->vm_private_data = map->handle;
	vma->vm_page_prot =  pgprot_writecombine(vm_get_page_prot(vma->vm_flags));

	
	drm_gem_object_reference(obj);

	vma->vm_file = filp;	
	drm_vm_open_locked(vma);

out_unlock:
	mutex_unlock(&dev->struct_mutex);

	return ret;
}
EXPORT_SYMBOL(drm_gem_mmap);
