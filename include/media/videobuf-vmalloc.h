
#ifndef _VIDEOBUF_VMALLOC_H
#define _VIDEOBUF_VMALLOC_H

#include <media/videobuf-core.h>



struct videobuf_vmalloc_memory
{
	u32                 magic;

	void                *vmalloc;

	
	struct vm_area_struct *vma;
};

void videobuf_queue_vmalloc_init(struct videobuf_queue* q,
			 struct videobuf_queue_ops *ops,
			 void *dev,
			 spinlock_t *irqlock,
			 enum v4l2_buf_type type,
			 enum v4l2_field field,
			 unsigned int msize,
			 void *priv);

void *videobuf_to_vmalloc (struct videobuf_buffer *buf);

void videobuf_vmalloc_free (struct videobuf_buffer *buf);

#endif
