
#include <linux/virtio.h>
#include <linux/virtio_ring.h>
#include <linux/virtio_config.h>
#include <linux/device.h>

#ifdef DEBUG

#define BAD_RING(_vq, fmt, args...)				\
	do {							\
		dev_err(&(_vq)->vq.vdev->dev,			\
			"%s:"fmt, (_vq)->vq.name, ##args);	\
		BUG();						\
	} while (0)

#define START_USE(_vq)						\
	do {							\
		if ((_vq)->in_use)				\
			panic("%s:in_use = %i\n",		\
			      (_vq)->vq.name, (_vq)->in_use);	\
		(_vq)->in_use = __LINE__;			\
		mb();						\
	} while (0)
#define END_USE(_vq) \
	do { BUG_ON(!(_vq)->in_use); (_vq)->in_use = 0; mb(); } while(0)
#else
#define BAD_RING(_vq, fmt, args...)				\
	do {							\
		dev_err(&_vq->vq.vdev->dev,			\
			"%s:"fmt, (_vq)->vq.name, ##args);	\
		(_vq)->broken = true;				\
	} while (0)
#define START_USE(vq)
#define END_USE(vq)
#endif

struct vring_virtqueue
{
	struct virtqueue vq;

	
	struct vring vring;

	
	bool broken;

	
	bool indirect;

	
	unsigned int num_free;
	
	unsigned int free_head;
	
	unsigned int num_added;

	
	u16 last_used_idx;

	
	void (*notify)(struct virtqueue *vq);

#ifdef DEBUG
	
	unsigned int in_use;
#endif

	
	void *data[];
};

#define to_vvq(_vq) container_of(_vq, struct vring_virtqueue, vq)


static int vring_add_indirect(struct vring_virtqueue *vq,
			      struct scatterlist sg[],
			      unsigned int out,
			      unsigned int in)
{
	struct vring_desc *desc;
	unsigned head;
	int i;

	desc = kmalloc((out + in) * sizeof(struct vring_desc), GFP_ATOMIC);
	if (!desc)
		return vq->vring.num;

	
	for (i = 0; i < out; i++) {
		desc[i].flags = VRING_DESC_F_NEXT;
		desc[i].addr = sg_phys(sg);
		desc[i].len = sg->length;
		desc[i].next = i+1;
		sg++;
	}
	for (; i < (out + in); i++) {
		desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
		desc[i].addr = sg_phys(sg);
		desc[i].len = sg->length;
		desc[i].next = i+1;
		sg++;
	}

	
	desc[i-1].flags &= ~VRING_DESC_F_NEXT;
	desc[i-1].next = 0;

	
	vq->num_free--;

	
	head = vq->free_head;
	vq->vring.desc[head].flags = VRING_DESC_F_INDIRECT;
	vq->vring.desc[head].addr = virt_to_phys(desc);
	vq->vring.desc[head].len = i * sizeof(struct vring_desc);

	
	vq->free_head = vq->vring.desc[head].next;

	return head;
}

static int vring_add_buf(struct virtqueue *_vq,
			 struct scatterlist sg[],
			 unsigned int out,
			 unsigned int in,
			 void *data)
{
	struct vring_virtqueue *vq = to_vvq(_vq);
	unsigned int i, avail, head, uninitialized_var(prev);

	START_USE(vq);

	BUG_ON(data == NULL);

	
	if (vq->indirect && (out + in) > 1 && vq->num_free) {
		head = vring_add_indirect(vq, sg, out, in);
		if (head != vq->vring.num)
			goto add_head;
	}

	BUG_ON(out + in > vq->vring.num);
	BUG_ON(out + in == 0);

	if (vq->num_free < out + in) {
		pr_debug("Can't add buf len %i - avail = %i\n",
			 out + in, vq->num_free);
		
		if (out)
			vq->notify(&vq->vq);
		END_USE(vq);
		return -ENOSPC;
	}

	
	vq->num_free -= out + in;

	head = vq->free_head;
	for (i = vq->free_head; out; i = vq->vring.desc[i].next, out--) {
		vq->vring.desc[i].flags = VRING_DESC_F_NEXT;
		vq->vring.desc[i].addr = sg_phys(sg);
		vq->vring.desc[i].len = sg->length;
		prev = i;
		sg++;
	}
	for (; in; i = vq->vring.desc[i].next, in--) {
		vq->vring.desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
		vq->vring.desc[i].addr = sg_phys(sg);
		vq->vring.desc[i].len = sg->length;
		prev = i;
		sg++;
	}
	
	vq->vring.desc[prev].flags &= ~VRING_DESC_F_NEXT;

	
	vq->free_head = i;

add_head:
	
	vq->data[head] = data;

	
	avail = (vq->vring.avail->idx + vq->num_added++) % vq->vring.num;
	vq->vring.avail->ring[avail] = head;

	pr_debug("Added buffer head %i to %p\n", head, vq);
	END_USE(vq);

	
	if (vq->indirect)
		return vq->num_free ? vq->vring.num : 0;
	return vq->num_free;
}

static void vring_kick(struct virtqueue *_vq)
{
	struct vring_virtqueue *vq = to_vvq(_vq);
	START_USE(vq);
	
	wmb();

	vq->vring.avail->idx += vq->num_added;
	vq->num_added = 0;

	
	mb();

	if (!(vq->vring.used->flags & VRING_USED_F_NO_NOTIFY))
		
		vq->notify(&vq->vq);

	END_USE(vq);
}

static void detach_buf(struct vring_virtqueue *vq, unsigned int head)
{
	unsigned int i;

	
	vq->data[head] = NULL;

	
	i = head;

	
	if (vq->vring.desc[i].flags & VRING_DESC_F_INDIRECT)
		kfree(phys_to_virt(vq->vring.desc[i].addr));

	while (vq->vring.desc[i].flags & VRING_DESC_F_NEXT) {
		i = vq->vring.desc[i].next;
		vq->num_free++;
	}

	vq->vring.desc[i].next = vq->free_head;
	vq->free_head = head;
	
	vq->num_free++;
}

static inline bool more_used(const struct vring_virtqueue *vq)
{
	return vq->last_used_idx != vq->vring.used->idx;
}

static void *vring_get_buf(struct virtqueue *_vq, unsigned int *len)
{
	struct vring_virtqueue *vq = to_vvq(_vq);
	void *ret;
	unsigned int i;

	START_USE(vq);

	if (unlikely(vq->broken)) {
		END_USE(vq);
		return NULL;
	}

	if (!more_used(vq)) {
		pr_debug("No more buffers in queue\n");
		END_USE(vq);
		return NULL;
	}

	
	rmb();

	i = vq->vring.used->ring[vq->last_used_idx%vq->vring.num].id;
	*len = vq->vring.used->ring[vq->last_used_idx%vq->vring.num].len;

	if (unlikely(i >= vq->vring.num)) {
		BAD_RING(vq, "id %u out of range\n", i);
		return NULL;
	}
	if (unlikely(!vq->data[i])) {
		BAD_RING(vq, "id %u is not a head!\n", i);
		return NULL;
	}

	
	ret = vq->data[i];
	detach_buf(vq, i);
	vq->last_used_idx++;
	END_USE(vq);
	return ret;
}

static void vring_disable_cb(struct virtqueue *_vq)
{
	struct vring_virtqueue *vq = to_vvq(_vq);

	vq->vring.avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;
}

static bool vring_enable_cb(struct virtqueue *_vq)
{
	struct vring_virtqueue *vq = to_vvq(_vq);

	START_USE(vq);

	
	vq->vring.avail->flags &= ~VRING_AVAIL_F_NO_INTERRUPT;
	mb();
	if (unlikely(more_used(vq))) {
		END_USE(vq);
		return false;
	}

	END_USE(vq);
	return true;
}

irqreturn_t vring_interrupt(int irq, void *_vq)
{
	struct vring_virtqueue *vq = to_vvq(_vq);

	if (!more_used(vq)) {
		pr_debug("virtqueue interrupt with no work for %p\n", vq);
		return IRQ_NONE;
	}

	if (unlikely(vq->broken))
		return IRQ_HANDLED;

	pr_debug("virtqueue callback for %p (%p)\n", vq, vq->vq.callback);
	if (vq->vq.callback)
		vq->vq.callback(&vq->vq);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(vring_interrupt);

static struct virtqueue_ops vring_vq_ops = {
	.add_buf = vring_add_buf,
	.get_buf = vring_get_buf,
	.kick = vring_kick,
	.disable_cb = vring_disable_cb,
	.enable_cb = vring_enable_cb,
};

struct virtqueue *vring_new_virtqueue(unsigned int num,
				      unsigned int vring_align,
				      struct virtio_device *vdev,
				      void *pages,
				      void (*notify)(struct virtqueue *),
				      void (*callback)(struct virtqueue *),
				      const char *name)
{
	struct vring_virtqueue *vq;
	unsigned int i;

	
	if (num & (num - 1)) {
		dev_warn(&vdev->dev, "Bad virtqueue length %u\n", num);
		return NULL;
	}

	vq = kmalloc(sizeof(*vq) + sizeof(void *)*num, GFP_KERNEL);
	if (!vq)
		return NULL;

	vring_init(&vq->vring, num, pages, vring_align);
	vq->vq.callback = callback;
	vq->vq.vdev = vdev;
	vq->vq.vq_ops = &vring_vq_ops;
	vq->vq.name = name;
	vq->notify = notify;
	vq->broken = false;
	vq->last_used_idx = 0;
	vq->num_added = 0;
	list_add_tail(&vq->vq.list, &vdev->vqs);
#ifdef DEBUG
	vq->in_use = false;
#endif

	vq->indirect = virtio_has_feature(vdev, VIRTIO_RING_F_INDIRECT_DESC);

	
	if (!callback)
		vq->vring.avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

	
	vq->num_free = num;
	vq->free_head = 0;
	for (i = 0; i < num-1; i++)
		vq->vring.desc[i].next = i+1;

	return &vq->vq;
}
EXPORT_SYMBOL_GPL(vring_new_virtqueue);

void vring_del_virtqueue(struct virtqueue *vq)
{
	list_del(&vq->list);
	kfree(to_vvq(vq));
}
EXPORT_SYMBOL_GPL(vring_del_virtqueue);


void vring_transport_features(struct virtio_device *vdev)
{
	unsigned int i;

	for (i = VIRTIO_TRANSPORT_F_START; i < VIRTIO_TRANSPORT_F_END; i++) {
		switch (i) {
		case VIRTIO_RING_F_INDIRECT_DESC:
			break;
		default:
			
			clear_bit(i, vdev->features);
		}
	}
}
EXPORT_SYMBOL_GPL(vring_transport_features);

MODULE_LICENSE("GPL");
