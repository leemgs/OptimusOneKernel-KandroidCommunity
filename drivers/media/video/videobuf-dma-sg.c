

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/scatterlist.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#include <media/videobuf-dma-sg.h>

#define MAGIC_DMABUF 0x19721112
#define MAGIC_SG_MEM 0x17890714

#define MAGIC_CHECK(is,should)	if (unlikely((is) != (should))) \
	{ printk(KERN_ERR "magic mismatch: %x (expected %x)\n",is,should); BUG(); }

static int debug;
module_param(debug, int, 0644);

MODULE_DESCRIPTION("helper module to manage video4linux dma sg buffers");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@infradead.org>");
MODULE_LICENSE("GPL");

#define dprintk(level, fmt, arg...)	if (debug >= level) \
	printk(KERN_DEBUG "vbuf-sg: " fmt , ## arg)



struct scatterlist*
videobuf_vmalloc_to_sg(unsigned char *virt, int nr_pages)
{
	struct scatterlist *sglist;
	struct page *pg;
	int i;

	sglist = vmalloc(nr_pages * sizeof(*sglist));
	if (NULL == sglist)
		return NULL;
	memset(sglist, 0, nr_pages * sizeof(*sglist));
	sg_init_table(sglist, nr_pages);
	for (i = 0; i < nr_pages; i++, virt += PAGE_SIZE) {
		pg = vmalloc_to_page(virt);
		if (NULL == pg)
			goto err;
		BUG_ON(PageHighMem(pg));
		sg_set_page(&sglist[i], pg, PAGE_SIZE, 0);
	}
	return sglist;

 err:
	vfree(sglist);
	return NULL;
}

struct scatterlist*
videobuf_pages_to_sg(struct page **pages, int nr_pages, int offset)
{
	struct scatterlist *sglist;
	int i;

	if (NULL == pages[0])
		return NULL;
	sglist = vmalloc(nr_pages * sizeof(*sglist));
	if (NULL == sglist)
		return NULL;
	sg_init_table(sglist, nr_pages);

	if (PageHighMem(pages[0]))
		
		goto highmem;
	sg_set_page(&sglist[0], pages[0], PAGE_SIZE - offset, offset);
	for (i = 1; i < nr_pages; i++) {
		if (NULL == pages[i])
			goto nopage;
		if (PageHighMem(pages[i]))
			goto highmem;
		sg_set_page(&sglist[i], pages[i], PAGE_SIZE, 0);
	}
	return sglist;

 nopage:
	dprintk(2,"sgl: oops - no page\n");
	vfree(sglist);
	return NULL;

 highmem:
	dprintk(2,"sgl: oops - highmem page\n");
	vfree(sglist);
	return NULL;
}



struct videobuf_dmabuf *videobuf_to_dma (struct videobuf_buffer *buf)
{
	struct videobuf_dma_sg_memory *mem = buf->priv;
	BUG_ON(!mem);

	MAGIC_CHECK(mem->magic, MAGIC_SG_MEM);

	return &mem->dma;
}

void videobuf_dma_init(struct videobuf_dmabuf *dma)
{
	memset(dma,0,sizeof(*dma));
	dma->magic = MAGIC_DMABUF;
}

static int videobuf_dma_init_user_locked(struct videobuf_dmabuf *dma,
			int direction, unsigned long data, unsigned long size)
{
	unsigned long first,last;
	int err, rw = 0;

	dma->direction = direction;
	switch (dma->direction) {
	case DMA_FROM_DEVICE:
		rw = READ;
		break;
	case DMA_TO_DEVICE:
		rw = WRITE;
		break;
	default:
		BUG();
	}

	first = (data          & PAGE_MASK) >> PAGE_SHIFT;
	last  = ((data+size-1) & PAGE_MASK) >> PAGE_SHIFT;
	dma->offset   = data & ~PAGE_MASK;
	dma->nr_pages = last-first+1;
	dma->pages = kmalloc(dma->nr_pages * sizeof(struct page*),
			     GFP_KERNEL);
	if (NULL == dma->pages)
		return -ENOMEM;
	dprintk(1,"init user [0x%lx+0x%lx => %d pages]\n",
		data,size,dma->nr_pages);

	err = get_user_pages(current,current->mm,
			     data & PAGE_MASK, dma->nr_pages,
			     rw == READ, 1, 
			     dma->pages, NULL);

	if (err != dma->nr_pages) {
		dma->nr_pages = (err >= 0) ? err : 0;
		dprintk(1,"get_user_pages: err=%d [%d]\n",err,dma->nr_pages);
		return err < 0 ? err : -EINVAL;
	}
	return 0;
}

int videobuf_dma_init_user(struct videobuf_dmabuf *dma, int direction,
			   unsigned long data, unsigned long size)
{
	int ret;
	down_read(&current->mm->mmap_sem);
	ret = videobuf_dma_init_user_locked(dma, direction, data, size);
	up_read(&current->mm->mmap_sem);

	return ret;
}

int videobuf_dma_init_kernel(struct videobuf_dmabuf *dma, int direction,
			     int nr_pages)
{
	dprintk(1,"init kernel [%d pages]\n",nr_pages);
	dma->direction = direction;
	dma->vmalloc = vmalloc_32(nr_pages << PAGE_SHIFT);
	if (NULL == dma->vmalloc) {
		dprintk(1,"vmalloc_32(%d pages) failed\n",nr_pages);
		return -ENOMEM;
	}
	dprintk(1,"vmalloc is at addr 0x%08lx, size=%d\n",
				(unsigned long)dma->vmalloc,
				nr_pages << PAGE_SHIFT);
	memset(dma->vmalloc,0,nr_pages << PAGE_SHIFT);
	dma->nr_pages = nr_pages;
	return 0;
}

int videobuf_dma_init_overlay(struct videobuf_dmabuf *dma, int direction,
			      dma_addr_t addr, int nr_pages)
{
	dprintk(1,"init overlay [%d pages @ bus 0x%lx]\n",
		nr_pages,(unsigned long)addr);
	dma->direction = direction;
	if (0 == addr)
		return -EINVAL;

	dma->bus_addr = addr;
	dma->nr_pages = nr_pages;
	return 0;
}

int videobuf_dma_map(struct videobuf_queue* q, struct videobuf_dmabuf *dma)
{
	MAGIC_CHECK(dma->magic,MAGIC_DMABUF);
	BUG_ON(0 == dma->nr_pages);

	if (dma->pages) {
		dma->sglist = videobuf_pages_to_sg(dma->pages, dma->nr_pages,
						   dma->offset);
	}
	if (dma->vmalloc) {
		dma->sglist = videobuf_vmalloc_to_sg
						(dma->vmalloc,dma->nr_pages);
	}
	if (dma->bus_addr) {
		dma->sglist = vmalloc(sizeof(*dma->sglist));
		if (NULL != dma->sglist) {
			dma->sglen  = 1;
			sg_dma_address(&dma->sglist[0]) = dma->bus_addr & PAGE_MASK;
			dma->sglist[0].offset           = dma->bus_addr & ~PAGE_MASK;
			sg_dma_len(&dma->sglist[0])     = dma->nr_pages * PAGE_SIZE;
		}
	}
	if (NULL == dma->sglist) {
		dprintk(1,"scatterlist is NULL\n");
		return -ENOMEM;
	}
	if (!dma->bus_addr) {
		dma->sglen = dma_map_sg(q->dev, dma->sglist,
					dma->nr_pages, dma->direction);
		if (0 == dma->sglen) {
			printk(KERN_WARNING
			       "%s: videobuf_map_sg failed\n",__func__);
			vfree(dma->sglist);
			dma->sglist = NULL;
			dma->sglen = 0;
			return -ENOMEM;
		}
	}
	return 0;
}

int videobuf_dma_sync(struct videobuf_queue *q, struct videobuf_dmabuf *dma)
{
	MAGIC_CHECK(dma->magic, MAGIC_DMABUF);
	BUG_ON(!dma->sglen);

	dma_sync_sg_for_cpu(q->dev, dma->sglist, dma->nr_pages, dma->direction);
	return 0;
}

int videobuf_dma_unmap(struct videobuf_queue* q,struct videobuf_dmabuf *dma)
{
	MAGIC_CHECK(dma->magic, MAGIC_DMABUF);
	if (!dma->sglen)
		return 0;

	dma_unmap_sg(q->dev, dma->sglist, dma->nr_pages, dma->direction);

	vfree(dma->sglist);
	dma->sglist = NULL;
	dma->sglen = 0;
	return 0;
}

int videobuf_dma_free(struct videobuf_dmabuf *dma)
{
	MAGIC_CHECK(dma->magic,MAGIC_DMABUF);
	BUG_ON(dma->sglen);

	if (dma->pages) {
		int i;
		for (i=0; i < dma->nr_pages; i++)
			page_cache_release(dma->pages[i]);
		kfree(dma->pages);
		dma->pages = NULL;
	}

	vfree(dma->vmalloc);
	dma->vmalloc = NULL;

	if (dma->bus_addr) {
		dma->bus_addr = 0;
	}
	dma->direction = DMA_NONE;
	return 0;
}



int videobuf_sg_dma_map(struct device *dev, struct videobuf_dmabuf *dma)
{
	struct videobuf_queue q;

	q.dev = dev;

	return videobuf_dma_map(&q, dma);
}

int videobuf_sg_dma_unmap(struct device *dev, struct videobuf_dmabuf *dma)
{
	struct videobuf_queue q;

	q.dev = dev;

	return videobuf_dma_unmap(&q, dma);
}



static void
videobuf_vm_open(struct vm_area_struct *vma)
{
	struct videobuf_mapping *map = vma->vm_private_data;

	dprintk(2,"vm_open %p [count=%d,vma=%08lx-%08lx]\n",map,
		map->count,vma->vm_start,vma->vm_end);
	map->count++;
}

static void
videobuf_vm_close(struct vm_area_struct *vma)
{
	struct videobuf_mapping *map = vma->vm_private_data;
	struct videobuf_queue *q = map->q;
	struct videobuf_dma_sg_memory *mem;
	int i;

	dprintk(2,"vm_close %p [count=%d,vma=%08lx-%08lx]\n",map,
		map->count,vma->vm_start,vma->vm_end);

	map->count--;
	if (0 == map->count) {
		dprintk(1,"munmap %p q=%p\n",map,q);
		mutex_lock(&q->vb_lock);
		for (i = 0; i < VIDEO_MAX_FRAME; i++) {
			if (NULL == q->bufs[i])
				continue;
			mem=q->bufs[i]->priv;

			if (!mem)
				continue;

			MAGIC_CHECK(mem->magic,MAGIC_SG_MEM);

			if (q->bufs[i]->map != map)
				continue;
			q->bufs[i]->map   = NULL;
			q->bufs[i]->baddr = 0;
			q->ops->buf_release(q,q->bufs[i]);
		}
		mutex_unlock(&q->vb_lock);
		kfree(map);
	}
	return;
}


static int
videobuf_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;

	dprintk(3,"fault: fault @ %08lx [vma %08lx-%08lx]\n",
		(unsigned long)vmf->virtual_address,vma->vm_start,vma->vm_end);
	page = alloc_page(GFP_USER | __GFP_DMA32);
	if (!page)
		return VM_FAULT_OOM;
	clear_user_highpage(page, (unsigned long)vmf->virtual_address);
	vmf->page = page;
	return 0;
}

static const struct vm_operations_struct videobuf_vm_ops =
{
	.open     = videobuf_vm_open,
	.close    = videobuf_vm_close,
	.fault    = videobuf_vm_fault,
};





static void *__videobuf_alloc(size_t size)
{
	struct videobuf_dma_sg_memory *mem;
	struct videobuf_buffer *vb;

	vb = kzalloc(size+sizeof(*mem),GFP_KERNEL);

	mem = vb->priv = ((char *)vb)+size;
	mem->magic=MAGIC_SG_MEM;

	videobuf_dma_init(&mem->dma);

	dprintk(1,"%s: allocated at %p(%ld+%ld) & %p(%ld)\n",
		__func__,vb,(long)sizeof(*vb),(long)size-sizeof(*vb),
		mem,(long)sizeof(*mem));

	return vb;
}

static void *__videobuf_to_vmalloc (struct videobuf_buffer *buf)
{
	struct videobuf_dma_sg_memory *mem = buf->priv;
	BUG_ON(!mem);

	MAGIC_CHECK(mem->magic, MAGIC_SG_MEM);

	return mem->dma.vmalloc;
}

static int __videobuf_iolock (struct videobuf_queue* q,
			      struct videobuf_buffer *vb,
			      struct v4l2_framebuffer *fbuf)
{
	int err,pages;
	dma_addr_t bus;
	struct videobuf_dma_sg_memory *mem = vb->priv;
	BUG_ON(!mem);

	MAGIC_CHECK(mem->magic, MAGIC_SG_MEM);

	switch (vb->memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_USERPTR:
		if (0 == vb->baddr) {
			
			pages = PAGE_ALIGN(vb->size) >> PAGE_SHIFT;
			err = videobuf_dma_init_kernel( &mem->dma,
							DMA_FROM_DEVICE,
							pages );
			if (0 != err)
				return err;
		} else if (vb->memory == V4L2_MEMORY_USERPTR) {
			
			err = videobuf_dma_init_user( &mem->dma,
						      DMA_FROM_DEVICE,
						      vb->baddr,vb->bsize );
			if (0 != err)
				return err;
		} else {
			

			err = videobuf_dma_init_user_locked(&mem->dma,
						      DMA_FROM_DEVICE,
						      vb->baddr, vb->bsize);
			if (0 != err)
				return err;
		}
		break;
	case V4L2_MEMORY_OVERLAY:
		if (NULL == fbuf)
			return -EINVAL;
		
		
		bus   = (dma_addr_t)(unsigned long)fbuf->base + vb->boff;
		pages = PAGE_ALIGN(vb->size) >> PAGE_SHIFT;
		err = videobuf_dma_init_overlay(&mem->dma, DMA_FROM_DEVICE,
						bus, pages);
		if (0 != err)
			return err;
		break;
	default:
		BUG();
	}
	err = videobuf_dma_map(q, &mem->dma);
	if (0 != err)
		return err;

	return 0;
}

static int __videobuf_sync(struct videobuf_queue *q,
			   struct videobuf_buffer *buf)
{
	struct videobuf_dma_sg_memory *mem = buf->priv;
	BUG_ON(!mem);
	MAGIC_CHECK(mem->magic,MAGIC_SG_MEM);

	return	videobuf_dma_sync(q,&mem->dma);
}

static int __videobuf_mmap_free(struct videobuf_queue *q)
{
	int i;

	for (i = 0; i < VIDEO_MAX_FRAME; i++) {
		if (q->bufs[i]) {
			if (q->bufs[i]->map)
				return -EBUSY;
		}
	}

	return 0;
}

static int __videobuf_mmap_mapper(struct videobuf_queue *q,
			 struct vm_area_struct *vma)
{
	struct videobuf_dma_sg_memory *mem;
	struct videobuf_mapping *map;
	unsigned int first,last,size,i;
	int retval;

	retval = -EINVAL;
	if (!(vma->vm_flags & VM_WRITE)) {
		dprintk(1,"mmap app bug: PROT_WRITE please\n");
		goto done;
	}
	if (!(vma->vm_flags & VM_SHARED)) {
		dprintk(1,"mmap app bug: MAP_SHARED please\n");
		goto done;
	}

	

	
	for (first = 0; first < VIDEO_MAX_FRAME; first++) {
		if (NULL == q->bufs[first])
			continue;
		mem=q->bufs[first]->priv;
		BUG_ON(!mem);
		MAGIC_CHECK(mem->magic,MAGIC_SG_MEM);

		if (V4L2_MEMORY_MMAP != q->bufs[first]->memory)
			continue;
		if (q->bufs[first]->boff == (vma->vm_pgoff << PAGE_SHIFT))
			break;
	}
	if (VIDEO_MAX_FRAME == first) {
		dprintk(1,"mmap app bug: offset invalid [offset=0x%lx]\n",
			(vma->vm_pgoff << PAGE_SHIFT));
		goto done;
	}

	
	for (size = 0, last = first; last < VIDEO_MAX_FRAME; last++) {
		if (NULL == q->bufs[last])
			continue;
		if (V4L2_MEMORY_MMAP != q->bufs[last]->memory)
			continue;
		if (q->bufs[last]->map) {
			retval = -EBUSY;
			goto done;
		}
		size += q->bufs[last]->bsize;
		if (size == (vma->vm_end - vma->vm_start))
			break;
	}
	if (VIDEO_MAX_FRAME == last) {
		dprintk(1,"mmap app bug: size invalid [size=0x%lx]\n",
			(vma->vm_end - vma->vm_start));
		goto done;
	}

	
	retval = -ENOMEM;
	map = kmalloc(sizeof(struct videobuf_mapping),GFP_KERNEL);
	if (NULL == map)
		goto done;

	size = 0;
	for (i = first; i <= last; i++) {
		if (NULL == q->bufs[i])
			continue;
		q->bufs[i]->map   = map;
		q->bufs[i]->baddr = vma->vm_start + size;
		size += q->bufs[i]->bsize;
	}

	map->count    = 1;
	map->start    = vma->vm_start;
	map->end      = vma->vm_end;
	map->q        = q;
	vma->vm_ops   = &videobuf_vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_RESERVED;
	vma->vm_flags &= ~VM_IO; 
	vma->vm_private_data = map;
	dprintk(1,"mmap %p: q=%p %08lx-%08lx pgoff %08lx bufs %d-%d\n",
		map,q,vma->vm_start,vma->vm_end,vma->vm_pgoff,first,last);
	retval = 0;

 done:
	return retval;
}

static int __videobuf_copy_to_user ( struct videobuf_queue *q,
				char __user *data, size_t count,
				int nonblocking )
{
	struct videobuf_dma_sg_memory *mem = q->read_buf->priv;
	BUG_ON(!mem);
	MAGIC_CHECK(mem->magic,MAGIC_SG_MEM);

	
	if (count > q->read_buf->size - q->read_off)
		count = q->read_buf->size - q->read_off;

	if (copy_to_user(data, mem->dma.vmalloc+q->read_off, count))
		return -EFAULT;

	return count;
}

static int __videobuf_copy_stream ( struct videobuf_queue *q,
				char __user *data, size_t count, size_t pos,
				int vbihack, int nonblocking )
{
	unsigned int  *fc;
	struct videobuf_dma_sg_memory *mem = q->read_buf->priv;
	BUG_ON(!mem);
	MAGIC_CHECK(mem->magic,MAGIC_SG_MEM);

	if (vbihack) {
		
		fc  = (unsigned int*)mem->dma.vmalloc;
		fc += (q->read_buf->size>>2) -1;
		*fc = q->read_buf->field_count >> 1;
		dprintk(1,"vbihack: %d\n",*fc);
	}

	
	count = __videobuf_copy_to_user (q,data,count,nonblocking);

	if ( (count==-EFAULT) && (0 == pos) )
		return -EFAULT;

	return count;
}

static struct videobuf_qtype_ops sg_ops = {
	.magic        = MAGIC_QTYPE_OPS,

	.alloc        = __videobuf_alloc,
	.iolock       = __videobuf_iolock,
	.sync         = __videobuf_sync,
	.mmap_free    = __videobuf_mmap_free,
	.mmap_mapper  = __videobuf_mmap_mapper,
	.video_copy_to_user = __videobuf_copy_to_user,
	.copy_stream  = __videobuf_copy_stream,
	.vmalloc      = __videobuf_to_vmalloc,
};

void *videobuf_sg_alloc(size_t size)
{
	struct videobuf_queue q;

	
	q.int_ops = &sg_ops;

	q.msize = size;

	return videobuf_alloc(&q);
}

void videobuf_queue_sg_init(struct videobuf_queue* q,
			 struct videobuf_queue_ops *ops,
			 struct device *dev,
			 spinlock_t *irqlock,
			 enum v4l2_buf_type type,
			 enum v4l2_field field,
			 unsigned int msize,
			 void *priv)
{
	videobuf_queue_core_init(q, ops, dev, irqlock, type, field, msize,
				 priv, &sg_ops);
}



EXPORT_SYMBOL_GPL(videobuf_vmalloc_to_sg);

EXPORT_SYMBOL_GPL(videobuf_to_dma);
EXPORT_SYMBOL_GPL(videobuf_dma_init);
EXPORT_SYMBOL_GPL(videobuf_dma_init_user);
EXPORT_SYMBOL_GPL(videobuf_dma_init_kernel);
EXPORT_SYMBOL_GPL(videobuf_dma_init_overlay);
EXPORT_SYMBOL_GPL(videobuf_dma_map);
EXPORT_SYMBOL_GPL(videobuf_dma_sync);
EXPORT_SYMBOL_GPL(videobuf_dma_unmap);
EXPORT_SYMBOL_GPL(videobuf_dma_free);

EXPORT_SYMBOL_GPL(videobuf_sg_dma_map);
EXPORT_SYMBOL_GPL(videobuf_sg_dma_unmap);
EXPORT_SYMBOL_GPL(videobuf_sg_alloc);

EXPORT_SYMBOL_GPL(videobuf_queue_sg_init);


