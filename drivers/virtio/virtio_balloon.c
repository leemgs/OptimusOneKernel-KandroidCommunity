

#include <linux/virtio.h>
#include <linux/virtio_balloon.h>
#include <linux/swap.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/delay.h>

struct virtio_balloon
{
	struct virtio_device *vdev;
	struct virtqueue *inflate_vq, *deflate_vq;

	
	wait_queue_head_t config_change;

	
	struct task_struct *thread;

	
	struct completion acked;

	
	bool tell_host_first;

	
	unsigned int num_pages;
	struct list_head pages;

	
	unsigned int num_pfns;
	u32 pfns[256];
};

static struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_BALLOON, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static u32 page_to_balloon_pfn(struct page *page)
{
	unsigned long pfn = page_to_pfn(page);

	BUILD_BUG_ON(PAGE_SHIFT < VIRTIO_BALLOON_PFN_SHIFT);
	
	return pfn >> (PAGE_SHIFT - VIRTIO_BALLOON_PFN_SHIFT);
}

static void balloon_ack(struct virtqueue *vq)
{
	struct virtio_balloon *vb;
	unsigned int len;

	vb = vq->vq_ops->get_buf(vq, &len);
	if (vb)
		complete(&vb->acked);
}

static void tell_host(struct virtio_balloon *vb, struct virtqueue *vq)
{
	struct scatterlist sg;

	sg_init_one(&sg, vb->pfns, sizeof(vb->pfns[0]) * vb->num_pfns);

	init_completion(&vb->acked);

	
	if (vq->vq_ops->add_buf(vq, &sg, 1, 0, vb) < 0)
		BUG();
	vq->vq_ops->kick(vq);

	
	wait_for_completion(&vb->acked);
}

static void fill_balloon(struct virtio_balloon *vb, size_t num)
{
	
	num = min(num, ARRAY_SIZE(vb->pfns));

	for (vb->num_pfns = 0; vb->num_pfns < num; vb->num_pfns++) {
		struct page *page = alloc_page(GFP_HIGHUSER | __GFP_NORETRY);
		if (!page) {
			if (printk_ratelimit())
				dev_printk(KERN_INFO, &vb->vdev->dev,
					   "Out of puff! Can't get %zu pages\n",
					   num);
			
			msleep(200);
			break;
		}
		vb->pfns[vb->num_pfns] = page_to_balloon_pfn(page);
		totalram_pages--;
		vb->num_pages++;
		list_add(&page->lru, &vb->pages);
	}

	
	if (vb->num_pfns == 0)
		return;

	tell_host(vb, vb->inflate_vq);
}

static void release_pages_by_pfn(const u32 pfns[], unsigned int num)
{
	unsigned int i;

	for (i = 0; i < num; i++) {
		__free_page(pfn_to_page(pfns[i]));
		totalram_pages++;
	}
}

static void leak_balloon(struct virtio_balloon *vb, size_t num)
{
	struct page *page;

	
	num = min(num, ARRAY_SIZE(vb->pfns));

	for (vb->num_pfns = 0; vb->num_pfns < num; vb->num_pfns++) {
		page = list_first_entry(&vb->pages, struct page, lru);
		list_del(&page->lru);
		vb->pfns[vb->num_pfns] = page_to_balloon_pfn(page);
		vb->num_pages--;
	}

	if (vb->tell_host_first) {
		tell_host(vb, vb->deflate_vq);
		release_pages_by_pfn(vb->pfns, vb->num_pfns);
	} else {
		release_pages_by_pfn(vb->pfns, vb->num_pfns);
		tell_host(vb, vb->deflate_vq);
	}
}

static void virtballoon_changed(struct virtio_device *vdev)
{
	struct virtio_balloon *vb = vdev->priv;

	wake_up(&vb->config_change);
}

static inline s64 towards_target(struct virtio_balloon *vb)
{
	u32 v;
	vb->vdev->config->get(vb->vdev,
			      offsetof(struct virtio_balloon_config, num_pages),
			      &v, sizeof(v));
	return (s64)v - vb->num_pages;
}

static void update_balloon_size(struct virtio_balloon *vb)
{
	__le32 actual = cpu_to_le32(vb->num_pages);

	vb->vdev->config->set(vb->vdev,
			      offsetof(struct virtio_balloon_config, actual),
			      &actual, sizeof(actual));
}

static int balloon(void *_vballoon)
{
	struct virtio_balloon *vb = _vballoon;

	set_freezable();
	while (!kthread_should_stop()) {
		s64 diff;

		try_to_freeze();
		wait_event_interruptible(vb->config_change,
					 (diff = towards_target(vb)) != 0
					 || kthread_should_stop()
					 || freezing(current));
		if (diff > 0)
			fill_balloon(vb, diff);
		else if (diff < 0)
			leak_balloon(vb, -diff);
		update_balloon_size(vb);
	}
	return 0;
}

static int virtballoon_probe(struct virtio_device *vdev)
{
	struct virtio_balloon *vb;
	struct virtqueue *vqs[2];
	vq_callback_t *callbacks[] = { balloon_ack, balloon_ack };
	const char *names[] = { "inflate", "deflate" };
	int err;

	vdev->priv = vb = kmalloc(sizeof(*vb), GFP_KERNEL);
	if (!vb) {
		err = -ENOMEM;
		goto out;
	}

	INIT_LIST_HEAD(&vb->pages);
	vb->num_pages = 0;
	init_waitqueue_head(&vb->config_change);
	vb->vdev = vdev;

	
	err = vdev->config->find_vqs(vdev, 2, vqs, callbacks, names);
	if (err)
		goto out_free_vb;

	vb->inflate_vq = vqs[0];
	vb->deflate_vq = vqs[1];

	vb->thread = kthread_run(balloon, vb, "vballoon");
	if (IS_ERR(vb->thread)) {
		err = PTR_ERR(vb->thread);
		goto out_del_vqs;
	}

	vb->tell_host_first
		= virtio_has_feature(vdev, VIRTIO_BALLOON_F_MUST_TELL_HOST);

	return 0;

out_del_vqs:
	vdev->config->del_vqs(vdev);
out_free_vb:
	kfree(vb);
out:
	return err;
}

static void __devexit virtballoon_remove(struct virtio_device *vdev)
{
	struct virtio_balloon *vb = vdev->priv;

	kthread_stop(vb->thread);

	
	while (vb->num_pages)
		leak_balloon(vb, vb->num_pages);

	
	vdev->config->reset(vdev);

	vdev->config->del_vqs(vdev);
	kfree(vb);
}

static unsigned int features[] = { VIRTIO_BALLOON_F_MUST_TELL_HOST };

static struct virtio_driver virtio_balloon = {
	.feature_table = features,
	.feature_table_size = ARRAY_SIZE(features),
	.driver.name =	KBUILD_MODNAME,
	.driver.owner =	THIS_MODULE,
	.id_table =	id_table,
	.probe =	virtballoon_probe,
	.remove =	__devexit_p(virtballoon_remove),
	.config_changed = virtballoon_changed,
};

static int __init init(void)
{
	return register_virtio_driver(&virtio_balloon);
}

static void __exit fini(void)
{
	unregister_virtio_driver(&virtio_balloon);
}
module_init(init);
module_exit(fini);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio balloon driver");
MODULE_LICENSE("GPL");
