



#include <linux/seq_file.h>
#include "drmP.h"


int drm_name_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_minor *minor = node->minor;
	struct drm_device *dev = minor->dev;
	struct drm_master *master = minor->master;

	if (!master)
		return 0;

	if (drm_core_check_feature(dev, DRIVER_USE_PLATFORM_DEVICE)) {
		if (master->unique) {
			seq_printf(m, "%s %s\n",
					dev->driver->platform_device->name,
					master->unique);
		} else {
			seq_printf(m, "%s\n",
				dev->driver->platform_device->name);
		}
	} else {
		if (master->unique) {
			seq_printf(m, "%s %s %s\n",
				dev->driver->pci_driver.name,
				pci_name(dev->pdev), master->unique);
		} else {
			seq_printf(m, "%s %s\n", dev->driver->pci_driver.name,
				pci_name(dev->pdev));
		}
	}

	return 0;
}


int drm_vm_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_local_map *map;
	struct drm_map_list *r_list;

	
	const char *types[] = { "FB", "REG", "SHM", "AGP", "SG", "PCI" };
	const char *type;
	int i;

	mutex_lock(&dev->struct_mutex);
	seq_printf(m, "slot	 offset	      size type flags	 address mtrr\n\n");
	i = 0;
	list_for_each_entry(r_list, &dev->maplist, head) {
		map = r_list->map;
		if (!map)
			continue;
		if (map->type < 0 || map->type > 5)
			type = "??";
		else
			type = types[map->type];

		seq_printf(m, "%4d 0x%016llx 0x%08lx %4.4s  0x%02x 0x%08lx ",
			   i,
			   (unsigned long long)map->offset,
			   map->size, type, map->flags,
			   (unsigned long) r_list->user_token);
		if (map->mtrr < 0)
			seq_printf(m, "none\n");
		else
			seq_printf(m, "%4d\n", map->mtrr);
		i++;
	}
	mutex_unlock(&dev->struct_mutex);
	return 0;
}


int drm_queues_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	int i;
	struct drm_queue *q;

	mutex_lock(&dev->struct_mutex);
	seq_printf(m, "  ctx/flags   use   fin"
		   "   blk/rw/rwf  wait    flushed	   queued"
		   "      locks\n\n");
	for (i = 0; i < dev->queue_count; i++) {
		q = dev->queuelist[i];
		atomic_inc(&q->use_count);
		seq_printf(m,   "%5d/0x%03x %5d %5d"
			   " %5d/%c%c/%c%c%c %5Zd\n",
			   i,
			   q->flags,
			   atomic_read(&q->use_count),
			   atomic_read(&q->finalization),
			   atomic_read(&q->block_count),
			   atomic_read(&q->block_read) ? 'r' : '-',
			   atomic_read(&q->block_write) ? 'w' : '-',
			   waitqueue_active(&q->read_queue) ? 'r' : '-',
			   waitqueue_active(&q->write_queue) ? 'w' : '-',
			   waitqueue_active(&q->flush_queue) ? 'f' : '-',
			   DRM_BUFCOUNT(&q->waitlist));
		atomic_dec(&q->use_count);
	}
	mutex_unlock(&dev->struct_mutex);
	return 0;
}


int drm_bufs_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_device_dma *dma;
	int i, seg_pages;

	mutex_lock(&dev->struct_mutex);
	dma = dev->dma;
	if (!dma) {
		mutex_unlock(&dev->struct_mutex);
		return 0;
	}

	seq_printf(m, " o     size count  free	 segs pages    kB\n\n");
	for (i = 0; i <= DRM_MAX_ORDER; i++) {
		if (dma->bufs[i].buf_count) {
			seg_pages = dma->bufs[i].seg_count * (1 << dma->bufs[i].page_order);
			seq_printf(m, "%2d %8d %5d %5d %5d %5d %5ld\n",
				   i,
				   dma->bufs[i].buf_size,
				   dma->bufs[i].buf_count,
				   atomic_read(&dma->bufs[i].freelist.count),
				   dma->bufs[i].seg_count,
				   seg_pages,
				   seg_pages * PAGE_SIZE / 1024);
		}
	}
	seq_printf(m, "\n");
	for (i = 0; i < dma->buf_count; i++) {
		if (i && !(i % 32))
			seq_printf(m, "\n");
		seq_printf(m, " %d", dma->buflist[i]->list);
	}
	seq_printf(m, "\n");
	mutex_unlock(&dev->struct_mutex);
	return 0;
}


int drm_vblank_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	int crtc;

	mutex_lock(&dev->struct_mutex);
	for (crtc = 0; crtc < dev->num_crtcs; crtc++) {
		seq_printf(m, "CRTC %d enable:     %d\n",
			   crtc, atomic_read(&dev->vblank_refcount[crtc]));
		seq_printf(m, "CRTC %d counter:    %d\n",
			   crtc, drm_vblank_count(dev, crtc));
		seq_printf(m, "CRTC %d last wait:  %d\n",
			   crtc, dev->last_vblank_wait[crtc]);
		seq_printf(m, "CRTC %d in modeset: %d\n",
			   crtc, dev->vblank_inmodeset[crtc]);
	}
	mutex_unlock(&dev->struct_mutex);
	return 0;
}


int drm_clients_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_file *priv;

	mutex_lock(&dev->struct_mutex);
	seq_printf(m, "a dev	pid    uid	magic	  ioctls\n\n");
	list_for_each_entry(priv, &dev->filelist, lhead) {
		seq_printf(m, "%c %3d %5d %5d %10u %10lu\n",
			   priv->authenticated ? 'y' : 'n',
			   priv->minor->index,
			   priv->pid,
			   priv->uid, priv->magic, priv->ioctl_count);
	}
	mutex_unlock(&dev->struct_mutex);
	return 0;
}


int drm_gem_one_name_info(int id, void *ptr, void *data)
{
	struct drm_gem_object *obj = ptr;
	struct seq_file *m = data;

	seq_printf(m, "name %d size %zd\n", obj->name, obj->size);

	seq_printf(m, "%6d %8zd %7d %8d\n",
		   obj->name, obj->size,
		   atomic_read(&obj->handlecount.refcount),
		   atomic_read(&obj->refcount.refcount));
	return 0;
}

int drm_gem_name_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;

	seq_printf(m, "  name     size handles refcount\n");
	idr_for_each(&dev->object_name_idr, drm_gem_one_name_info, m);
	return 0;
}

int drm_gem_object_info(struct seq_file *m, void* data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;

	seq_printf(m, "%d objects\n", atomic_read(&dev->object_count));
	seq_printf(m, "%d object bytes\n", atomic_read(&dev->object_memory));
	seq_printf(m, "%d pinned\n", atomic_read(&dev->pin_count));
	seq_printf(m, "%d pin bytes\n", atomic_read(&dev->pin_memory));
	seq_printf(m, "%d gtt bytes\n", atomic_read(&dev->gtt_memory));
	seq_printf(m, "%d gtt total\n", dev->gtt_total);
	return 0;
}

#if DRM_DEBUG_CODE

int drm_vma_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_vma_entry *pt;
	struct vm_area_struct *vma;
#if defined(__i386__)
	unsigned int pgprot;
#endif

	mutex_lock(&dev->struct_mutex);
	seq_printf(m, "vma use count: %d, high_memory = %p, 0x%08llx\n",
		   atomic_read(&dev->vma_count),
		   high_memory, (u64)virt_to_phys(high_memory));

	list_for_each_entry(pt, &dev->vmalist, head) {
		vma = pt->vma;
		if (!vma)
			continue;
		seq_printf(m,
			   "\n%5d 0x%08lx-0x%08lx %c%c%c%c%c%c 0x%08lx000",
			   pt->pid, vma->vm_start, vma->vm_end,
			   vma->vm_flags & VM_READ ? 'r' : '-',
			   vma->vm_flags & VM_WRITE ? 'w' : '-',
			   vma->vm_flags & VM_EXEC ? 'x' : '-',
			   vma->vm_flags & VM_MAYSHARE ? 's' : 'p',
			   vma->vm_flags & VM_LOCKED ? 'l' : '-',
			   vma->vm_flags & VM_IO ? 'i' : '-',
			   vma->vm_pgoff);

#if defined(__i386__)
		pgprot = pgprot_val(vma->vm_page_prot);
		seq_printf(m, " %c%c%c%c%c%c%c%c%c",
			   pgprot & _PAGE_PRESENT ? 'p' : '-',
			   pgprot & _PAGE_RW ? 'w' : 'r',
			   pgprot & _PAGE_USER ? 'u' : 's',
			   pgprot & _PAGE_PWT ? 't' : 'b',
			   pgprot & _PAGE_PCD ? 'u' : 'c',
			   pgprot & _PAGE_ACCESSED ? 'a' : '-',
			   pgprot & _PAGE_DIRTY ? 'd' : '-',
			   pgprot & _PAGE_PSE ? 'm' : 'k',
			   pgprot & _PAGE_GLOBAL ? 'g' : 'l');
#endif
		seq_printf(m, "\n");
	}
	mutex_unlock(&dev->struct_mutex);
	return 0;
}

#endif

