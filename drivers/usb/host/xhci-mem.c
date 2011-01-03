

#include <linux/usb.h>
#include <linux/pci.h>
#include <linux/dmapool.h>

#include "xhci.h"


static struct xhci_segment *xhci_segment_alloc(struct xhci_hcd *xhci, gfp_t flags)
{
	struct xhci_segment *seg;
	dma_addr_t	dma;

	seg = kzalloc(sizeof *seg, flags);
	if (!seg)
		return 0;
	xhci_dbg(xhci, "Allocating priv segment structure at %p\n", seg);

	seg->trbs = dma_pool_alloc(xhci->segment_pool, flags, &dma);
	if (!seg->trbs) {
		kfree(seg);
		return 0;
	}
	xhci_dbg(xhci, "// Allocating segment at %p (virtual) 0x%llx (DMA)\n",
			seg->trbs, (unsigned long long)dma);

	memset(seg->trbs, 0, SEGMENT_SIZE);
	seg->dma = dma;
	seg->next = NULL;

	return seg;
}

static void xhci_segment_free(struct xhci_hcd *xhci, struct xhci_segment *seg)
{
	if (!seg)
		return;
	if (seg->trbs) {
		xhci_dbg(xhci, "Freeing DMA segment at %p (virtual) 0x%llx (DMA)\n",
				seg->trbs, (unsigned long long)seg->dma);
		dma_pool_free(xhci->segment_pool, seg->trbs, seg->dma);
		seg->trbs = NULL;
	}
	xhci_dbg(xhci, "Freeing priv segment structure at %p\n", seg);
	kfree(seg);
}


static void xhci_link_segments(struct xhci_hcd *xhci, struct xhci_segment *prev,
		struct xhci_segment *next, bool link_trbs)
{
	u32 val;

	if (!prev || !next)
		return;
	prev->next = next;
	if (link_trbs) {
		prev->trbs[TRBS_PER_SEGMENT-1].link.segment_ptr = next->dma;

		
		val = prev->trbs[TRBS_PER_SEGMENT-1].link.control;
		val &= ~TRB_TYPE_BITMASK;
		val |= TRB_TYPE(TRB_LINK);
		
		if (xhci_link_trb_quirk(xhci))
			val |= TRB_CHAIN;
		prev->trbs[TRBS_PER_SEGMENT-1].link.control = val;
	}
	xhci_dbg(xhci, "Linking segment 0x%llx to segment 0x%llx (DMA)\n",
			(unsigned long long)prev->dma,
			(unsigned long long)next->dma);
}


void xhci_ring_free(struct xhci_hcd *xhci, struct xhci_ring *ring)
{
	struct xhci_segment *seg;
	struct xhci_segment *first_seg;

	if (!ring || !ring->first_seg)
		return;
	first_seg = ring->first_seg;
	seg = first_seg->next;
	xhci_dbg(xhci, "Freeing ring at %p\n", ring);
	while (seg != first_seg) {
		struct xhci_segment *next = seg->next;
		xhci_segment_free(xhci, seg);
		seg = next;
	}
	xhci_segment_free(xhci, first_seg);
	ring->first_seg = NULL;
	kfree(ring);
}


static struct xhci_ring *xhci_ring_alloc(struct xhci_hcd *xhci,
		unsigned int num_segs, bool link_trbs, gfp_t flags)
{
	struct xhci_ring	*ring;
	struct xhci_segment	*prev;

	ring = kzalloc(sizeof *(ring), flags);
	xhci_dbg(xhci, "Allocating ring at %p\n", ring);
	if (!ring)
		return 0;

	INIT_LIST_HEAD(&ring->td_list);
	if (num_segs == 0)
		return ring;

	ring->first_seg = xhci_segment_alloc(xhci, flags);
	if (!ring->first_seg)
		goto fail;
	num_segs--;

	prev = ring->first_seg;
	while (num_segs > 0) {
		struct xhci_segment	*next;

		next = xhci_segment_alloc(xhci, flags);
		if (!next)
			goto fail;
		xhci_link_segments(xhci, prev, next, link_trbs);

		prev = next;
		num_segs--;
	}
	xhci_link_segments(xhci, prev, ring->first_seg, link_trbs);

	if (link_trbs) {
		
		prev->trbs[TRBS_PER_SEGMENT-1].link.control |= (LINK_TOGGLE);
		xhci_dbg(xhci, "Wrote link toggle flag to"
				" segment %p (virtual), 0x%llx (DMA)\n",
				prev, (unsigned long long)prev->dma);
	}
	
	ring->enqueue = ring->first_seg->trbs;
	ring->enq_seg = ring->first_seg;
	ring->dequeue = ring->enqueue;
	ring->deq_seg = ring->first_seg;
	
	ring->cycle_state = 1;

	return ring;

fail:
	xhci_ring_free(xhci, ring);
	return 0;
}

#define CTX_SIZE(_hcc) (HCC_64BYTE_CONTEXT(_hcc) ? 64 : 32)

struct xhci_container_ctx *xhci_alloc_container_ctx(struct xhci_hcd *xhci,
						    int type, gfp_t flags)
{
	struct xhci_container_ctx *ctx = kzalloc(sizeof(*ctx), flags);
	if (!ctx)
		return NULL;

	BUG_ON((type != XHCI_CTX_TYPE_DEVICE) && (type != XHCI_CTX_TYPE_INPUT));
	ctx->type = type;
	ctx->size = HCC_64BYTE_CONTEXT(xhci->hcc_params) ? 2048 : 1024;
	if (type == XHCI_CTX_TYPE_INPUT)
		ctx->size += CTX_SIZE(xhci->hcc_params);

	ctx->bytes = dma_pool_alloc(xhci->device_pool, flags, &ctx->dma);
	memset(ctx->bytes, 0, ctx->size);
	return ctx;
}

void xhci_free_container_ctx(struct xhci_hcd *xhci,
			     struct xhci_container_ctx *ctx)
{
	dma_pool_free(xhci->device_pool, ctx->bytes, ctx->dma);
	kfree(ctx);
}

struct xhci_input_control_ctx *xhci_get_input_control_ctx(struct xhci_hcd *xhci,
					      struct xhci_container_ctx *ctx)
{
	BUG_ON(ctx->type != XHCI_CTX_TYPE_INPUT);
	return (struct xhci_input_control_ctx *)ctx->bytes;
}

struct xhci_slot_ctx *xhci_get_slot_ctx(struct xhci_hcd *xhci,
					struct xhci_container_ctx *ctx)
{
	if (ctx->type == XHCI_CTX_TYPE_DEVICE)
		return (struct xhci_slot_ctx *)ctx->bytes;

	return (struct xhci_slot_ctx *)
		(ctx->bytes + CTX_SIZE(xhci->hcc_params));
}

struct xhci_ep_ctx *xhci_get_ep_ctx(struct xhci_hcd *xhci,
				    struct xhci_container_ctx *ctx,
				    unsigned int ep_index)
{
	
	ep_index++;
	if (ctx->type == XHCI_CTX_TYPE_INPUT)
		ep_index++;

	return (struct xhci_ep_ctx *)
		(ctx->bytes + (ep_index * CTX_SIZE(xhci->hcc_params)));
}


void xhci_free_virt_device(struct xhci_hcd *xhci, int slot_id)
{
	struct xhci_virt_device *dev;
	int i;

	
	if (slot_id == 0 || !xhci->devs[slot_id])
		return;

	dev = xhci->devs[slot_id];
	xhci->dcbaa->dev_context_ptrs[slot_id] = 0;
	if (!dev)
		return;

	for (i = 0; i < 31; ++i)
		if (dev->eps[i].ring)
			xhci_ring_free(xhci, dev->eps[i].ring);

	if (dev->in_ctx)
		xhci_free_container_ctx(xhci, dev->in_ctx);
	if (dev->out_ctx)
		xhci_free_container_ctx(xhci, dev->out_ctx);

	kfree(xhci->devs[slot_id]);
	xhci->devs[slot_id] = 0;
}

int xhci_alloc_virt_device(struct xhci_hcd *xhci, int slot_id,
		struct usb_device *udev, gfp_t flags)
{
	struct xhci_virt_device *dev;
	int i;

	
	if (slot_id == 0 || xhci->devs[slot_id]) {
		xhci_warn(xhci, "Bad Slot ID %d\n", slot_id);
		return 0;
	}

	xhci->devs[slot_id] = kzalloc(sizeof(*xhci->devs[slot_id]), flags);
	if (!xhci->devs[slot_id])
		return 0;
	dev = xhci->devs[slot_id];

	
	dev->out_ctx = xhci_alloc_container_ctx(xhci, XHCI_CTX_TYPE_DEVICE, flags);
	if (!dev->out_ctx)
		goto fail;

	xhci_dbg(xhci, "Slot %d output ctx = 0x%llx (dma)\n", slot_id,
			(unsigned long long)dev->out_ctx->dma);

	
	dev->in_ctx = xhci_alloc_container_ctx(xhci, XHCI_CTX_TYPE_INPUT, flags);
	if (!dev->in_ctx)
		goto fail;

	xhci_dbg(xhci, "Slot %d input ctx = 0x%llx (dma)\n", slot_id,
			(unsigned long long)dev->in_ctx->dma);

	
	for (i = 0; i < 31; i++)
		INIT_LIST_HEAD(&dev->eps[i].cancelled_td_list);

	
	dev->eps[0].ring = xhci_ring_alloc(xhci, 1, true, flags);
	if (!dev->eps[0].ring)
		goto fail;

	init_completion(&dev->cmd_completion);
	INIT_LIST_HEAD(&dev->cmd_list);

	
	xhci->dcbaa->dev_context_ptrs[slot_id] = dev->out_ctx->dma;
	xhci_dbg(xhci, "Set slot id %d dcbaa entry %p to 0x%llx\n",
			slot_id,
			&xhci->dcbaa->dev_context_ptrs[slot_id],
			(unsigned long long) xhci->dcbaa->dev_context_ptrs[slot_id]);

	return 1;
fail:
	xhci_free_virt_device(xhci, slot_id);
	return 0;
}


int xhci_setup_addressable_virt_dev(struct xhci_hcd *xhci, struct usb_device *udev)
{
	struct xhci_virt_device *dev;
	struct xhci_ep_ctx	*ep0_ctx;
	struct usb_device	*top_dev;
	struct xhci_slot_ctx    *slot_ctx;
	struct xhci_input_control_ctx *ctrl_ctx;

	dev = xhci->devs[udev->slot_id];
	
	if (udev->slot_id == 0 || !dev) {
		xhci_warn(xhci, "Slot ID %d is not assigned to this device\n",
				udev->slot_id);
		return -EINVAL;
	}
	ep0_ctx = xhci_get_ep_ctx(xhci, dev->in_ctx, 0);
	ctrl_ctx = xhci_get_input_control_ctx(xhci, dev->in_ctx);
	slot_ctx = xhci_get_slot_ctx(xhci, dev->in_ctx);

	
	ctrl_ctx->add_flags = SLOT_FLAG | EP0_FLAG;

	
	slot_ctx->dev_info |= LAST_CTX(1);

	slot_ctx->dev_info |= (u32) udev->route;
	switch (udev->speed) {
	case USB_SPEED_SUPER:
		slot_ctx->dev_info |= (u32) SLOT_SPEED_SS;
		break;
	case USB_SPEED_HIGH:
		slot_ctx->dev_info |= (u32) SLOT_SPEED_HS;
		break;
	case USB_SPEED_FULL:
		slot_ctx->dev_info |= (u32) SLOT_SPEED_FS;
		break;
	case USB_SPEED_LOW:
		slot_ctx->dev_info |= (u32) SLOT_SPEED_LS;
		break;
	case USB_SPEED_VARIABLE:
		xhci_dbg(xhci, "FIXME xHCI doesn't support wireless speeds\n");
		return -EINVAL;
		break;
	default:
		
		BUG();
	}
	
	for (top_dev = udev; top_dev->parent && top_dev->parent->parent;
			top_dev = top_dev->parent)
		;
	slot_ctx->dev_info2 |= (u32) ROOT_HUB_PORT(top_dev->portnum);
	xhci_dbg(xhci, "Set root hub portnum to %d\n", top_dev->portnum);

	
	if ((udev->speed == USB_SPEED_LOW || udev->speed == USB_SPEED_FULL) &&
			udev->tt) {
		slot_ctx->tt_info = udev->tt->hub->slot_id;
		slot_ctx->tt_info |= udev->ttport << 8;
		if (udev->tt->multi)
			slot_ctx->dev_info |= DEV_MTT;
	}
	xhci_dbg(xhci, "udev->tt = %p\n", udev->tt);
	xhci_dbg(xhci, "udev->ttport = 0x%x\n", udev->ttport);

	
	
	ep0_ctx->ep_info2 = EP_TYPE(CTRL_EP);
	
	switch (udev->speed) {
	case USB_SPEED_SUPER:
		ep0_ctx->ep_info2 |= MAX_PACKET(512);
		break;
	case USB_SPEED_HIGH:
	
	case USB_SPEED_FULL:
		ep0_ctx->ep_info2 |= MAX_PACKET(64);
		break;
	case USB_SPEED_LOW:
		ep0_ctx->ep_info2 |= MAX_PACKET(8);
		break;
	case USB_SPEED_VARIABLE:
		xhci_dbg(xhci, "FIXME xHCI doesn't support wireless speeds\n");
		return -EINVAL;
		break;
	default:
		
		BUG();
	}
	
	ep0_ctx->ep_info2 |= MAX_BURST(0);
	ep0_ctx->ep_info2 |= ERROR_COUNT(3);

	ep0_ctx->deq =
		dev->eps[0].ring->first_seg->dma;
	ep0_ctx->deq |= dev->eps[0].ring->cycle_state;

	

	return 0;
}


static inline unsigned int xhci_get_endpoint_interval(struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	unsigned int interval = 0;

	switch (udev->speed) {
	case USB_SPEED_HIGH:
		
		if (usb_endpoint_xfer_control(&ep->desc) ||
				usb_endpoint_xfer_bulk(&ep->desc))
			interval = ep->desc.bInterval;
		
	case USB_SPEED_SUPER:
		if (usb_endpoint_xfer_int(&ep->desc) ||
				usb_endpoint_xfer_isoc(&ep->desc)) {
			if (ep->desc.bInterval == 0)
				interval = 0;
			else
				interval = ep->desc.bInterval - 1;
			if (interval > 15)
				interval = 15;
			if (interval != ep->desc.bInterval + 1)
				dev_warn(&udev->dev, "ep %#x - rounding interval to %d microframes\n",
						ep->desc.bEndpointAddress, 1 << interval);
		}
		break;
	
	case USB_SPEED_FULL:
	case USB_SPEED_LOW:
		if (usb_endpoint_xfer_int(&ep->desc) ||
				usb_endpoint_xfer_isoc(&ep->desc)) {
			interval = fls(8*ep->desc.bInterval) - 1;
			if (interval > 10)
				interval = 10;
			if (interval < 3)
				interval = 3;
			if ((1 << interval) != 8*ep->desc.bInterval)
				dev_warn(&udev->dev, "ep %#x - rounding interval to %d microframes\n",
						ep->desc.bEndpointAddress, 1 << interval);
		}
		break;
	default:
		BUG();
	}
	return EP_INTERVAL(interval);
}

static inline u32 xhci_get_endpoint_type(struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	int in;
	u32 type;

	in = usb_endpoint_dir_in(&ep->desc);
	if (usb_endpoint_xfer_control(&ep->desc)) {
		type = EP_TYPE(CTRL_EP);
	} else if (usb_endpoint_xfer_bulk(&ep->desc)) {
		if (in)
			type = EP_TYPE(BULK_IN_EP);
		else
			type = EP_TYPE(BULK_OUT_EP);
	} else if (usb_endpoint_xfer_isoc(&ep->desc)) {
		if (in)
			type = EP_TYPE(ISOC_IN_EP);
		else
			type = EP_TYPE(ISOC_OUT_EP);
	} else if (usb_endpoint_xfer_int(&ep->desc)) {
		if (in)
			type = EP_TYPE(INT_IN_EP);
		else
			type = EP_TYPE(INT_OUT_EP);
	} else {
		BUG();
	}
	return type;
}

int xhci_endpoint_init(struct xhci_hcd *xhci,
		struct xhci_virt_device *virt_dev,
		struct usb_device *udev,
		struct usb_host_endpoint *ep,
		gfp_t mem_flags)
{
	unsigned int ep_index;
	struct xhci_ep_ctx *ep_ctx;
	struct xhci_ring *ep_ring;
	unsigned int max_packet;
	unsigned int max_burst;

	ep_index = xhci_get_endpoint_index(&ep->desc);
	ep_ctx = xhci_get_ep_ctx(xhci, virt_dev->in_ctx, ep_index);

	
	virt_dev->eps[ep_index].new_ring =
		xhci_ring_alloc(xhci, 1, true, mem_flags);
	if (!virt_dev->eps[ep_index].new_ring)
		return -ENOMEM;
	ep_ring = virt_dev->eps[ep_index].new_ring;
	ep_ctx->deq = ep_ring->first_seg->dma | ep_ring->cycle_state;

	ep_ctx->ep_info = xhci_get_endpoint_interval(udev, ep);

	

	
	if (!usb_endpoint_xfer_isoc(&ep->desc))
		ep_ctx->ep_info2 = ERROR_COUNT(3);
	else
		ep_ctx->ep_info2 = ERROR_COUNT(1);

	ep_ctx->ep_info2 |= xhci_get_endpoint_type(udev, ep);

	
	switch (udev->speed) {
	case USB_SPEED_SUPER:
		max_packet = ep->desc.wMaxPacketSize;
		ep_ctx->ep_info2 |= MAX_PACKET(max_packet);
		
		if (!ep->ss_ep_comp) {
			xhci_warn(xhci, "WARN no SS endpoint companion descriptor.\n");
			max_packet = 0;
		} else {
			max_packet = ep->ss_ep_comp->desc.bMaxBurst;
		}
		ep_ctx->ep_info2 |= MAX_BURST(max_packet);
		break;
	case USB_SPEED_HIGH:
		
		if (usb_endpoint_xfer_isoc(&ep->desc) ||
				usb_endpoint_xfer_int(&ep->desc)) {
			max_burst = (ep->desc.wMaxPacketSize & 0x1800) >> 11;
			ep_ctx->ep_info2 |= MAX_BURST(max_burst);
		}
		
	case USB_SPEED_FULL:
	case USB_SPEED_LOW:
		max_packet = ep->desc.wMaxPacketSize & 0x3ff;
		ep_ctx->ep_info2 |= MAX_PACKET(max_packet);
		break;
	default:
		BUG();
	}
	
	return 0;
}

void xhci_endpoint_zero(struct xhci_hcd *xhci,
		struct xhci_virt_device *virt_dev,
		struct usb_host_endpoint *ep)
{
	unsigned int ep_index;
	struct xhci_ep_ctx *ep_ctx;

	ep_index = xhci_get_endpoint_index(&ep->desc);
	ep_ctx = xhci_get_ep_ctx(xhci, virt_dev->in_ctx, ep_index);

	ep_ctx->ep_info = 0;
	ep_ctx->ep_info2 = 0;
	ep_ctx->deq = 0;
	ep_ctx->tx_info = 0;
	
}


void xhci_endpoint_copy(struct xhci_hcd *xhci,
		struct xhci_container_ctx *in_ctx,
		struct xhci_container_ctx *out_ctx,
		unsigned int ep_index)
{
	struct xhci_ep_ctx *out_ep_ctx;
	struct xhci_ep_ctx *in_ep_ctx;

	out_ep_ctx = xhci_get_ep_ctx(xhci, out_ctx, ep_index);
	in_ep_ctx = xhci_get_ep_ctx(xhci, in_ctx, ep_index);

	in_ep_ctx->ep_info = out_ep_ctx->ep_info;
	in_ep_ctx->ep_info2 = out_ep_ctx->ep_info2;
	in_ep_ctx->deq = out_ep_ctx->deq;
	in_ep_ctx->tx_info = out_ep_ctx->tx_info;
}


void xhci_slot_copy(struct xhci_hcd *xhci,
		struct xhci_container_ctx *in_ctx,
		struct xhci_container_ctx *out_ctx)
{
	struct xhci_slot_ctx *in_slot_ctx;
	struct xhci_slot_ctx *out_slot_ctx;

	in_slot_ctx = xhci_get_slot_ctx(xhci, in_ctx);
	out_slot_ctx = xhci_get_slot_ctx(xhci, out_ctx);

	in_slot_ctx->dev_info = out_slot_ctx->dev_info;
	in_slot_ctx->dev_info2 = out_slot_ctx->dev_info2;
	in_slot_ctx->tt_info = out_slot_ctx->tt_info;
	in_slot_ctx->dev_state = out_slot_ctx->dev_state;
}


static int scratchpad_alloc(struct xhci_hcd *xhci, gfp_t flags)
{
	int i;
	struct device *dev = xhci_to_hcd(xhci)->self.controller;
	int num_sp = HCS_MAX_SCRATCHPAD(xhci->hcs_params2);

	xhci_dbg(xhci, "Allocating %d scratchpad buffers\n", num_sp);

	if (!num_sp)
		return 0;

	xhci->scratchpad = kzalloc(sizeof(*xhci->scratchpad), flags);
	if (!xhci->scratchpad)
		goto fail_sp;

	xhci->scratchpad->sp_array =
		pci_alloc_consistent(to_pci_dev(dev),
				     num_sp * sizeof(u64),
				     &xhci->scratchpad->sp_dma);
	if (!xhci->scratchpad->sp_array)
		goto fail_sp2;

	xhci->scratchpad->sp_buffers = kzalloc(sizeof(void *) * num_sp, flags);
	if (!xhci->scratchpad->sp_buffers)
		goto fail_sp3;

	xhci->scratchpad->sp_dma_buffers =
		kzalloc(sizeof(dma_addr_t) * num_sp, flags);

	if (!xhci->scratchpad->sp_dma_buffers)
		goto fail_sp4;

	xhci->dcbaa->dev_context_ptrs[0] = xhci->scratchpad->sp_dma;
	for (i = 0; i < num_sp; i++) {
		dma_addr_t dma;
		void *buf = pci_alloc_consistent(to_pci_dev(dev),
						 xhci->page_size, &dma);
		if (!buf)
			goto fail_sp5;

		xhci->scratchpad->sp_array[i] = dma;
		xhci->scratchpad->sp_buffers[i] = buf;
		xhci->scratchpad->sp_dma_buffers[i] = dma;
	}

	return 0;

 fail_sp5:
	for (i = i - 1; i >= 0; i--) {
		pci_free_consistent(to_pci_dev(dev), xhci->page_size,
				    xhci->scratchpad->sp_buffers[i],
				    xhci->scratchpad->sp_dma_buffers[i]);
	}
	kfree(xhci->scratchpad->sp_dma_buffers);

 fail_sp4:
	kfree(xhci->scratchpad->sp_buffers);

 fail_sp3:
	pci_free_consistent(to_pci_dev(dev), num_sp * sizeof(u64),
			    xhci->scratchpad->sp_array,
			    xhci->scratchpad->sp_dma);

 fail_sp2:
	kfree(xhci->scratchpad);
	xhci->scratchpad = NULL;

 fail_sp:
	return -ENOMEM;
}

static void scratchpad_free(struct xhci_hcd *xhci)
{
	int num_sp;
	int i;
	struct pci_dev	*pdev = to_pci_dev(xhci_to_hcd(xhci)->self.controller);

	if (!xhci->scratchpad)
		return;

	num_sp = HCS_MAX_SCRATCHPAD(xhci->hcs_params2);

	for (i = 0; i < num_sp; i++) {
		pci_free_consistent(pdev, xhci->page_size,
				    xhci->scratchpad->sp_buffers[i],
				    xhci->scratchpad->sp_dma_buffers[i]);
	}
	kfree(xhci->scratchpad->sp_dma_buffers);
	kfree(xhci->scratchpad->sp_buffers);
	pci_free_consistent(pdev, num_sp * sizeof(u64),
			    xhci->scratchpad->sp_array,
			    xhci->scratchpad->sp_dma);
	kfree(xhci->scratchpad);
	xhci->scratchpad = NULL;
}

struct xhci_command *xhci_alloc_command(struct xhci_hcd *xhci,
		bool allocate_completion, gfp_t mem_flags)
{
	struct xhci_command *command;

	command = kzalloc(sizeof(*command), mem_flags);
	if (!command)
		return NULL;

	command->in_ctx =
		xhci_alloc_container_ctx(xhci, XHCI_CTX_TYPE_INPUT, mem_flags);
	if (!command->in_ctx)
		return NULL;

	if (allocate_completion) {
		command->completion =
			kzalloc(sizeof(struct completion), mem_flags);
		if (!command->completion) {
			xhci_free_container_ctx(xhci, command->in_ctx);
			return NULL;
		}
		init_completion(command->completion);
	}

	command->status = 0;
	INIT_LIST_HEAD(&command->cmd_list);
	return command;
}

void xhci_free_command(struct xhci_hcd *xhci,
		struct xhci_command *command)
{
	xhci_free_container_ctx(xhci,
			command->in_ctx);
	kfree(command->completion);
	kfree(command);
}

void xhci_mem_cleanup(struct xhci_hcd *xhci)
{
	struct pci_dev	*pdev = to_pci_dev(xhci_to_hcd(xhci)->self.controller);
	int size;
	int i;

	
	if (xhci->ir_set) {
		xhci_writel(xhci, 0, &xhci->ir_set->erst_size);
		xhci_write_64(xhci, 0, &xhci->ir_set->erst_base);
		xhci_write_64(xhci, 0, &xhci->ir_set->erst_dequeue);
	}
	size = sizeof(struct xhci_erst_entry)*(xhci->erst.num_entries);
	if (xhci->erst.entries)
		pci_free_consistent(pdev, size,
				xhci->erst.entries, xhci->erst.erst_dma_addr);
	xhci->erst.entries = NULL;
	xhci_dbg(xhci, "Freed ERST\n");
	if (xhci->event_ring)
		xhci_ring_free(xhci, xhci->event_ring);
	xhci->event_ring = NULL;
	xhci_dbg(xhci, "Freed event ring\n");

	xhci_write_64(xhci, 0, &xhci->op_regs->cmd_ring);
	if (xhci->cmd_ring)
		xhci_ring_free(xhci, xhci->cmd_ring);
	xhci->cmd_ring = NULL;
	xhci_dbg(xhci, "Freed command ring\n");

	for (i = 1; i < MAX_HC_SLOTS; ++i)
		xhci_free_virt_device(xhci, i);

	if (xhci->segment_pool)
		dma_pool_destroy(xhci->segment_pool);
	xhci->segment_pool = NULL;
	xhci_dbg(xhci, "Freed segment pool\n");

	if (xhci->device_pool)
		dma_pool_destroy(xhci->device_pool);
	xhci->device_pool = NULL;
	xhci_dbg(xhci, "Freed device context pool\n");

	xhci_write_64(xhci, 0, &xhci->op_regs->dcbaa_ptr);
	if (xhci->dcbaa)
		pci_free_consistent(pdev, sizeof(*xhci->dcbaa),
				xhci->dcbaa, xhci->dcbaa->dma);
	xhci->dcbaa = NULL;

	scratchpad_free(xhci);
	xhci->page_size = 0;
	xhci->page_shift = 0;
}

int xhci_mem_init(struct xhci_hcd *xhci, gfp_t flags)
{
	dma_addr_t	dma;
	struct device	*dev = xhci_to_hcd(xhci)->self.controller;
	unsigned int	val, val2;
	u64		val_64;
	struct xhci_segment	*seg;
	u32 page_size;
	int i;

	page_size = xhci_readl(xhci, &xhci->op_regs->page_size);
	xhci_dbg(xhci, "Supported page size register = 0x%x\n", page_size);
	for (i = 0; i < 16; i++) {
		if ((0x1 & page_size) != 0)
			break;
		page_size = page_size >> 1;
	}
	if (i < 16)
		xhci_dbg(xhci, "Supported page size of %iK\n", (1 << (i+12)) / 1024);
	else
		xhci_warn(xhci, "WARN: no supported page size\n");
	
	xhci->page_shift = 12;
	xhci->page_size = 1 << xhci->page_shift;
	xhci_dbg(xhci, "HCD page size set to %iK\n", xhci->page_size / 1024);

	
	val = HCS_MAX_SLOTS(xhci_readl(xhci, &xhci->cap_regs->hcs_params1));
	xhci_dbg(xhci, "// xHC can handle at most %d device slots.\n",
			(unsigned int) val);
	val2 = xhci_readl(xhci, &xhci->op_regs->config_reg);
	val |= (val2 & ~HCS_SLOTS_MASK);
	xhci_dbg(xhci, "// Setting Max device slots reg = 0x%x.\n",
			(unsigned int) val);
	xhci_writel(xhci, val, &xhci->op_regs->config_reg);

	
	xhci->dcbaa = pci_alloc_consistent(to_pci_dev(dev),
			sizeof(*xhci->dcbaa), &dma);
	if (!xhci->dcbaa)
		goto fail;
	memset(xhci->dcbaa, 0, sizeof *(xhci->dcbaa));
	xhci->dcbaa->dma = dma;
	xhci_dbg(xhci, "// Device context base array address = 0x%llx (DMA), %p (virt)\n",
			(unsigned long long)xhci->dcbaa->dma, xhci->dcbaa);
	xhci_write_64(xhci, dma, &xhci->op_regs->dcbaa_ptr);

	
	xhci->segment_pool = dma_pool_create("xHCI ring segments", dev,
			SEGMENT_SIZE, 64, xhci->page_size);

	
	xhci->device_pool = dma_pool_create("xHCI input/output contexts", dev,
			2112, 64, xhci->page_size);
	if (!xhci->segment_pool || !xhci->device_pool)
		goto fail;

	
	xhci->cmd_ring = xhci_ring_alloc(xhci, 1, true, flags);
	if (!xhci->cmd_ring)
		goto fail;
	xhci_dbg(xhci, "Allocated command ring at %p\n", xhci->cmd_ring);
	xhci_dbg(xhci, "First segment DMA is 0x%llx\n",
			(unsigned long long)xhci->cmd_ring->first_seg->dma);

	
	val_64 = xhci_read_64(xhci, &xhci->op_regs->cmd_ring);
	val_64 = (val_64 & (u64) CMD_RING_RSVD_BITS) |
		(xhci->cmd_ring->first_seg->dma & (u64) ~CMD_RING_RSVD_BITS) |
		xhci->cmd_ring->cycle_state;
	xhci_dbg(xhci, "// Setting command ring address to 0x%x\n", val);
	xhci_write_64(xhci, val_64, &xhci->op_regs->cmd_ring);
	xhci_dbg_cmd_ptrs(xhci);

	val = xhci_readl(xhci, &xhci->cap_regs->db_off);
	val &= DBOFF_MASK;
	xhci_dbg(xhci, "// Doorbell array is located at offset 0x%x"
			" from cap regs base addr\n", val);
	xhci->dba = (void *) xhci->cap_regs + val;
	xhci_dbg_regs(xhci);
	xhci_print_run_regs(xhci);
	
	xhci->ir_set = (void *) xhci->run_regs->ir_set;

	
	xhci_dbg(xhci, "// Allocating event ring\n");
	xhci->event_ring = xhci_ring_alloc(xhci, ERST_NUM_SEGS, false, flags);
	if (!xhci->event_ring)
		goto fail;

	xhci->erst.entries = pci_alloc_consistent(to_pci_dev(dev),
			sizeof(struct xhci_erst_entry)*ERST_NUM_SEGS, &dma);
	if (!xhci->erst.entries)
		goto fail;
	xhci_dbg(xhci, "// Allocated event ring segment table at 0x%llx\n",
			(unsigned long long)dma);

	memset(xhci->erst.entries, 0, sizeof(struct xhci_erst_entry)*ERST_NUM_SEGS);
	xhci->erst.num_entries = ERST_NUM_SEGS;
	xhci->erst.erst_dma_addr = dma;
	xhci_dbg(xhci, "Set ERST to 0; private num segs = %i, virt addr = %p, dma addr = 0x%llx\n",
			xhci->erst.num_entries,
			xhci->erst.entries,
			(unsigned long long)xhci->erst.erst_dma_addr);

	
	for (val = 0, seg = xhci->event_ring->first_seg; val < ERST_NUM_SEGS; val++) {
		struct xhci_erst_entry *entry = &xhci->erst.entries[val];
		entry->seg_addr = seg->dma;
		entry->seg_size = TRBS_PER_SEGMENT;
		entry->rsvd = 0;
		seg = seg->next;
	}

	
	val = xhci_readl(xhci, &xhci->ir_set->erst_size);
	val &= ERST_SIZE_MASK;
	val |= ERST_NUM_SEGS;
	xhci_dbg(xhci, "// Write ERST size = %i to ir_set 0 (some bits preserved)\n",
			val);
	xhci_writel(xhci, val, &xhci->ir_set->erst_size);

	xhci_dbg(xhci, "// Set ERST entries to point to event ring.\n");
	
	xhci_dbg(xhci, "// Set ERST base address for ir_set 0 = 0x%llx\n",
			(unsigned long long)xhci->erst.erst_dma_addr);
	val_64 = xhci_read_64(xhci, &xhci->ir_set->erst_base);
	val_64 &= ERST_PTR_MASK;
	val_64 |= (xhci->erst.erst_dma_addr & (u64) ~ERST_PTR_MASK);
	xhci_write_64(xhci, val_64, &xhci->ir_set->erst_base);

	
	xhci_set_hc_event_deq(xhci);
	xhci_dbg(xhci, "Wrote ERST address to ir_set 0.\n");
	xhci_print_ir_set(xhci, xhci->ir_set, 0);

	
	init_completion(&xhci->addr_dev);
	for (i = 0; i < MAX_HC_SLOTS; ++i)
		xhci->devs[i] = 0;

	if (scratchpad_alloc(xhci, flags))
		goto fail;

	return 0;

fail:
	xhci_warn(xhci, "Couldn't initialize memory\n");
	xhci_mem_cleanup(xhci);
	return -ENOMEM;
}
