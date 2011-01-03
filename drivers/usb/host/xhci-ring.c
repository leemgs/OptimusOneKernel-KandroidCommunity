



#include <linux/scatterlist.h>
#include "xhci.h"


dma_addr_t xhci_trb_virt_to_dma(struct xhci_segment *seg,
		union xhci_trb *trb)
{
	unsigned long segment_offset;

	if (!seg || !trb || trb < seg->trbs)
		return 0;
	
	segment_offset = trb - seg->trbs;
	if (segment_offset > TRBS_PER_SEGMENT)
		return 0;
	return seg->dma + (segment_offset * sizeof(*trb));
}


static inline bool last_trb_on_last_seg(struct xhci_hcd *xhci, struct xhci_ring *ring,
		struct xhci_segment *seg, union xhci_trb *trb)
{
	if (ring == xhci->event_ring)
		return (trb == &seg->trbs[TRBS_PER_SEGMENT]) &&
			(seg->next == xhci->event_ring->first_seg);
	else
		return trb->link.control & LINK_TOGGLE;
}


static inline int last_trb(struct xhci_hcd *xhci, struct xhci_ring *ring,
		struct xhci_segment *seg, union xhci_trb *trb)
{
	if (ring == xhci->event_ring)
		return trb == &seg->trbs[TRBS_PER_SEGMENT];
	else
		return (trb->link.control & TRB_TYPE_BITMASK) == TRB_TYPE(TRB_LINK);
}


static void next_trb(struct xhci_hcd *xhci,
		struct xhci_ring *ring,
		struct xhci_segment **seg,
		union xhci_trb **trb)
{
	if (last_trb(xhci, ring, *seg, *trb)) {
		*seg = (*seg)->next;
		*trb = ((*seg)->trbs);
	} else {
		*trb = (*trb)++;
	}
}


static void inc_deq(struct xhci_hcd *xhci, struct xhci_ring *ring, bool consumer)
{
	union xhci_trb *next = ++(ring->dequeue);
	unsigned long long addr;

	ring->deq_updates++;
	
	while (last_trb(xhci, ring, ring->deq_seg, next)) {
		if (consumer && last_trb_on_last_seg(xhci, ring, ring->deq_seg, next)) {
			ring->cycle_state = (ring->cycle_state ? 0 : 1);
			if (!in_interrupt())
				xhci_dbg(xhci, "Toggle cycle state for ring %p = %i\n",
						ring,
						(unsigned int) ring->cycle_state);
		}
		ring->deq_seg = ring->deq_seg->next;
		ring->dequeue = ring->deq_seg->trbs;
		next = ring->dequeue;
	}
	addr = (unsigned long long) xhci_trb_virt_to_dma(ring->deq_seg, ring->dequeue);
	if (ring == xhci->event_ring)
		xhci_dbg(xhci, "Event ring deq = 0x%llx (DMA)\n", addr);
	else if (ring == xhci->cmd_ring)
		xhci_dbg(xhci, "Command ring deq = 0x%llx (DMA)\n", addr);
	else
		xhci_dbg(xhci, "Ring deq = 0x%llx (DMA)\n", addr);
}


static void inc_enq(struct xhci_hcd *xhci, struct xhci_ring *ring, bool consumer)
{
	u32 chain;
	union xhci_trb *next;
	unsigned long long addr;

	chain = ring->enqueue->generic.field[3] & TRB_CHAIN;
	next = ++(ring->enqueue);

	ring->enq_updates++;
	
	while (last_trb(xhci, ring, ring->enq_seg, next)) {
		if (!consumer) {
			if (ring != xhci->event_ring) {
				
				if (!xhci_link_trb_quirk(xhci)) {
					next->link.control &= ~TRB_CHAIN;
					next->link.control |= chain;
				}
				
				wmb();
				if (next->link.control & TRB_CYCLE)
					next->link.control &= (u32) ~TRB_CYCLE;
				else
					next->link.control |= (u32) TRB_CYCLE;
			}
			
			if (last_trb_on_last_seg(xhci, ring, ring->enq_seg, next)) {
				ring->cycle_state = (ring->cycle_state ? 0 : 1);
				if (!in_interrupt())
					xhci_dbg(xhci, "Toggle cycle state for ring %p = %i\n",
							ring,
							(unsigned int) ring->cycle_state);
			}
		}
		ring->enq_seg = ring->enq_seg->next;
		ring->enqueue = ring->enq_seg->trbs;
		next = ring->enqueue;
	}
	addr = (unsigned long long) xhci_trb_virt_to_dma(ring->enq_seg, ring->enqueue);
	if (ring == xhci->event_ring)
		xhci_dbg(xhci, "Event ring enq = 0x%llx (DMA)\n", addr);
	else if (ring == xhci->cmd_ring)
		xhci_dbg(xhci, "Command ring enq = 0x%llx (DMA)\n", addr);
	else
		xhci_dbg(xhci, "Ring enq = 0x%llx (DMA)\n", addr);
}


static int room_on_ring(struct xhci_hcd *xhci, struct xhci_ring *ring,
		unsigned int num_trbs)
{
	int i;
	union xhci_trb *enq = ring->enqueue;
	struct xhci_segment *enq_seg = ring->enq_seg;

	
	if (enq == ring->dequeue)
		return 1;
	
	for (i = 0; i <= num_trbs; ++i) {
		if (enq == ring->dequeue)
			return 0;
		enq++;
		while (last_trb(xhci, ring, enq_seg, enq)) {
			enq_seg = enq_seg->next;
			enq = enq_seg->trbs;
		}
	}
	return 1;
}

void xhci_set_hc_event_deq(struct xhci_hcd *xhci)
{
	u64 temp;
	dma_addr_t deq;

	deq = xhci_trb_virt_to_dma(xhci->event_ring->deq_seg,
			xhci->event_ring->dequeue);
	if (deq == 0 && !in_interrupt())
		xhci_warn(xhci, "WARN something wrong with SW event ring "
				"dequeue ptr.\n");
	
	temp = xhci_read_64(xhci, &xhci->ir_set->erst_dequeue);
	temp &= ERST_PTR_MASK;
	
	temp &= ~ERST_EHB;
	xhci_dbg(xhci, "// Write event ring dequeue pointer, preserving EHB bit\n");
	xhci_write_64(xhci, ((u64) deq & (u64) ~ERST_PTR_MASK) | temp,
			&xhci->ir_set->erst_dequeue);
}


void xhci_ring_cmd_db(struct xhci_hcd *xhci)
{
	u32 temp;

	xhci_dbg(xhci, "// Ding dong!\n");
	temp = xhci_readl(xhci, &xhci->dba->doorbell[0]) & DB_MASK;
	xhci_writel(xhci, temp | DB_TARGET_HOST, &xhci->dba->doorbell[0]);
	
	xhci_readl(xhci, &xhci->dba->doorbell[0]);
}

static void ring_ep_doorbell(struct xhci_hcd *xhci,
		unsigned int slot_id,
		unsigned int ep_index)
{
	struct xhci_virt_ep *ep;
	unsigned int ep_state;
	u32 field;
	__u32 __iomem *db_addr = &xhci->dba->doorbell[slot_id];

	ep = &xhci->devs[slot_id]->eps[ep_index];
	ep_state = ep->ep_state;
	
	if (!ep->cancels_pending && !(ep_state & SET_DEQ_PENDING)
			&& !(ep_state & EP_HALTED)) {
		field = xhci_readl(xhci, db_addr) & DB_MASK;
		xhci_writel(xhci, field | EPI_TO_DB(ep_index), db_addr);
		
		xhci_readl(xhci, db_addr);
	}
}


static struct xhci_segment *find_trb_seg(
		struct xhci_segment *start_seg,
		union xhci_trb	*trb, int *cycle_state)
{
	struct xhci_segment *cur_seg = start_seg;
	struct xhci_generic_trb *generic_trb;

	while (cur_seg->trbs > trb ||
			&cur_seg->trbs[TRBS_PER_SEGMENT - 1] < trb) {
		generic_trb = &cur_seg->trbs[TRBS_PER_SEGMENT - 1].generic;
		if (TRB_TYPE(generic_trb->field[3]) == TRB_LINK &&
				(generic_trb->field[3] & LINK_TOGGLE))
			*cycle_state = ~(*cycle_state) & 0x1;
		cur_seg = cur_seg->next;
		if (cur_seg == start_seg)
			
			return 0;
	}
	return cur_seg;
}


void xhci_find_new_dequeue_state(struct xhci_hcd *xhci,
		unsigned int slot_id, unsigned int ep_index,
		struct xhci_td *cur_td, struct xhci_dequeue_state *state)
{
	struct xhci_virt_device *dev = xhci->devs[slot_id];
	struct xhci_ring *ep_ring = dev->eps[ep_index].ring;
	struct xhci_generic_trb *trb;
	struct xhci_ep_ctx *ep_ctx;
	dma_addr_t addr;

	state->new_cycle_state = 0;
	xhci_dbg(xhci, "Finding segment containing stopped TRB.\n");
	state->new_deq_seg = find_trb_seg(cur_td->start_seg,
			dev->eps[ep_index].stopped_trb,
			&state->new_cycle_state);
	if (!state->new_deq_seg)
		BUG();
	
	xhci_dbg(xhci, "Finding endpoint context\n");
	ep_ctx = xhci_get_ep_ctx(xhci, dev->out_ctx, ep_index);
	state->new_cycle_state = 0x1 & ep_ctx->deq;

	state->new_deq_ptr = cur_td->last_trb;
	xhci_dbg(xhci, "Finding segment containing last TRB in TD.\n");
	state->new_deq_seg = find_trb_seg(state->new_deq_seg,
			state->new_deq_ptr,
			&state->new_cycle_state);
	if (!state->new_deq_seg)
		BUG();

	trb = &state->new_deq_ptr->generic;
	if (TRB_TYPE(trb->field[3]) == TRB_LINK &&
				(trb->field[3] & LINK_TOGGLE))
		state->new_cycle_state = ~(state->new_cycle_state) & 0x1;
	next_trb(xhci, ep_ring, &state->new_deq_seg, &state->new_deq_ptr);

	
	xhci_dbg(xhci, "New dequeue segment = %p (virtual)\n",
			state->new_deq_seg);
	addr = xhci_trb_virt_to_dma(state->new_deq_seg, state->new_deq_ptr);
	xhci_dbg(xhci, "New dequeue pointer = 0x%llx (DMA)\n",
			(unsigned long long) addr);
	xhci_dbg(xhci, "Setting dequeue pointer in internal ring state.\n");
	ep_ring->dequeue = state->new_deq_ptr;
	ep_ring->deq_seg = state->new_deq_seg;
}

static void td_to_noop(struct xhci_hcd *xhci, struct xhci_ring *ep_ring,
		struct xhci_td *cur_td)
{
	struct xhci_segment *cur_seg;
	union xhci_trb *cur_trb;

	for (cur_seg = cur_td->start_seg, cur_trb = cur_td->first_trb;
			true;
			next_trb(xhci, ep_ring, &cur_seg, &cur_trb)) {
		if ((cur_trb->generic.field[3] & TRB_TYPE_BITMASK) ==
				TRB_TYPE(TRB_LINK)) {
			
			cur_trb->generic.field[3] &= ~TRB_CHAIN;
			xhci_dbg(xhci, "Cancel (unchain) link TRB\n");
			xhci_dbg(xhci, "Address = %p (0x%llx dma); "
					"in seg %p (0x%llx dma)\n",
					cur_trb,
					(unsigned long long)xhci_trb_virt_to_dma(cur_seg, cur_trb),
					cur_seg,
					(unsigned long long)cur_seg->dma);
		} else {
			cur_trb->generic.field[0] = 0;
			cur_trb->generic.field[1] = 0;
			cur_trb->generic.field[2] = 0;
			
			cur_trb->generic.field[3] &= TRB_CYCLE;
			cur_trb->generic.field[3] |= TRB_TYPE(TRB_TR_NOOP);
			xhci_dbg(xhci, "Cancel TRB %p (0x%llx dma) "
					"in seg %p (0x%llx dma)\n",
					cur_trb,
					(unsigned long long)xhci_trb_virt_to_dma(cur_seg, cur_trb),
					cur_seg,
					(unsigned long long)cur_seg->dma);
		}
		if (cur_trb == cur_td->last_trb)
			break;
	}
}

static int queue_set_tr_deq(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index, struct xhci_segment *deq_seg,
		union xhci_trb *deq_ptr, u32 cycle_state);

void xhci_queue_new_dequeue_state(struct xhci_hcd *xhci,
		unsigned int slot_id, unsigned int ep_index,
		struct xhci_dequeue_state *deq_state)
{
	struct xhci_virt_ep *ep = &xhci->devs[slot_id]->eps[ep_index];

	xhci_dbg(xhci, "Set TR Deq Ptr cmd, new deq seg = %p (0x%llx dma), "
			"new deq ptr = %p (0x%llx dma), new cycle = %u\n",
			deq_state->new_deq_seg,
			(unsigned long long)deq_state->new_deq_seg->dma,
			deq_state->new_deq_ptr,
			(unsigned long long)xhci_trb_virt_to_dma(deq_state->new_deq_seg, deq_state->new_deq_ptr),
			deq_state->new_cycle_state);
	queue_set_tr_deq(xhci, slot_id, ep_index,
			deq_state->new_deq_seg,
			deq_state->new_deq_ptr,
			(u32) deq_state->new_cycle_state);
	
	ep->ep_state |= SET_DEQ_PENDING;
}


static void handle_stopped_endpoint(struct xhci_hcd *xhci,
		union xhci_trb *trb)
{
	unsigned int slot_id;
	unsigned int ep_index;
	struct xhci_ring *ep_ring;
	struct xhci_virt_ep *ep;
	struct list_head *entry;
	struct xhci_td *cur_td = 0;
	struct xhci_td *last_unlinked_td;

	struct xhci_dequeue_state deq_state;
#ifdef CONFIG_USB_HCD_STAT
	ktime_t stop_time = ktime_get();
#endif

	memset(&deq_state, 0, sizeof(deq_state));
	slot_id = TRB_TO_SLOT_ID(trb->generic.field[3]);
	ep_index = TRB_TO_EP_INDEX(trb->generic.field[3]);
	ep = &xhci->devs[slot_id]->eps[ep_index];
	ep_ring = ep->ring;

	if (list_empty(&ep->cancelled_td_list))
		return;

	
	list_for_each(entry, &ep->cancelled_td_list) {
		cur_td = list_entry(entry, struct xhci_td, cancelled_td_list);
		xhci_dbg(xhci, "Cancelling TD starting at %p, 0x%llx (dma).\n",
				cur_td->first_trb,
				(unsigned long long)xhci_trb_virt_to_dma(cur_td->start_seg, cur_td->first_trb));
		
		if (cur_td == ep->stopped_td)
			xhci_find_new_dequeue_state(xhci, slot_id, ep_index, cur_td,
					&deq_state);
		else
			td_to_noop(xhci, ep_ring, cur_td);
		
		list_del(&cur_td->td_list);
		ep->cancels_pending--;
	}
	last_unlinked_td = cur_td;

	
	if (deq_state.new_deq_ptr && deq_state.new_deq_seg) {
		xhci_queue_new_dequeue_state(xhci,
				slot_id, ep_index, &deq_state);
		xhci_ring_cmd_db(xhci);
	} else {
		
		ring_ep_doorbell(xhci, slot_id, ep_index);
	}

	
	do {
		cur_td = list_entry(ep->cancelled_td_list.next,
				struct xhci_td, cancelled_td_list);
		list_del(&cur_td->cancelled_td_list);

		
#ifdef CONFIG_USB_HCD_STAT
		hcd_stat_update(xhci->tp_stat, cur_td->urb->actual_length,
				ktime_sub(stop_time, cur_td->start_time));
#endif
		cur_td->urb->hcpriv = NULL;
		usb_hcd_unlink_urb_from_ep(xhci_to_hcd(xhci), cur_td->urb);

		xhci_dbg(xhci, "Giveback cancelled URB %p\n", cur_td->urb);
		spin_unlock(&xhci->lock);
		
		usb_hcd_giveback_urb(xhci_to_hcd(xhci), cur_td->urb, 0);
		kfree(cur_td);

		spin_lock(&xhci->lock);
	} while (cur_td != last_unlinked_td);

	
}


static void handle_set_deq_completion(struct xhci_hcd *xhci,
		struct xhci_event_cmd *event,
		union xhci_trb *trb)
{
	unsigned int slot_id;
	unsigned int ep_index;
	struct xhci_ring *ep_ring;
	struct xhci_virt_device *dev;
	struct xhci_ep_ctx *ep_ctx;
	struct xhci_slot_ctx *slot_ctx;

	slot_id = TRB_TO_SLOT_ID(trb->generic.field[3]);
	ep_index = TRB_TO_EP_INDEX(trb->generic.field[3]);
	dev = xhci->devs[slot_id];
	ep_ring = dev->eps[ep_index].ring;
	ep_ctx = xhci_get_ep_ctx(xhci, dev->out_ctx, ep_index);
	slot_ctx = xhci_get_slot_ctx(xhci, dev->out_ctx);

	if (GET_COMP_CODE(event->status) != COMP_SUCCESS) {
		unsigned int ep_state;
		unsigned int slot_state;

		switch (GET_COMP_CODE(event->status)) {
		case COMP_TRB_ERR:
			xhci_warn(xhci, "WARN Set TR Deq Ptr cmd invalid because "
					"of stream ID configuration\n");
			break;
		case COMP_CTX_STATE:
			xhci_warn(xhci, "WARN Set TR Deq Ptr cmd failed due "
					"to incorrect slot or ep state.\n");
			ep_state = ep_ctx->ep_info;
			ep_state &= EP_STATE_MASK;
			slot_state = slot_ctx->dev_state;
			slot_state = GET_SLOT_STATE(slot_state);
			xhci_dbg(xhci, "Slot state = %u, EP state = %u\n",
					slot_state, ep_state);
			break;
		case COMP_EBADSLT:
			xhci_warn(xhci, "WARN Set TR Deq Ptr cmd failed because "
					"slot %u was not enabled.\n", slot_id);
			break;
		default:
			xhci_warn(xhci, "WARN Set TR Deq Ptr cmd with unknown "
					"completion code of %u.\n",
					GET_COMP_CODE(event->status));
			break;
		}
		
	} else {
		xhci_dbg(xhci, "Successful Set TR Deq Ptr cmd, deq = @%08llx\n",
				ep_ctx->deq);
	}

	dev->eps[ep_index].ep_state &= ~SET_DEQ_PENDING;
	ring_ep_doorbell(xhci, slot_id, ep_index);
}

static void handle_reset_ep_completion(struct xhci_hcd *xhci,
		struct xhci_event_cmd *event,
		union xhci_trb *trb)
{
	int slot_id;
	unsigned int ep_index;
	struct xhci_ring *ep_ring;

	slot_id = TRB_TO_SLOT_ID(trb->generic.field[3]);
	ep_index = TRB_TO_EP_INDEX(trb->generic.field[3]);
	ep_ring = xhci->devs[slot_id]->eps[ep_index].ring;
	
	xhci_dbg(xhci, "Ignoring reset ep completion code of %u\n",
			(unsigned int) GET_COMP_CODE(event->status));

	
	if (xhci->quirks & XHCI_RESET_EP_QUIRK) {
		xhci_dbg(xhci, "Queueing configure endpoint command\n");
		xhci_queue_configure_endpoint(xhci,
				xhci->devs[slot_id]->in_ctx->dma, slot_id,
				false);
		xhci_ring_cmd_db(xhci);
	} else {
		
		xhci->devs[slot_id]->eps[ep_index].ep_state &= ~EP_HALTED;
		ring_ep_doorbell(xhci, slot_id, ep_index);
	}
}


static int handle_cmd_in_cmd_wait_list(struct xhci_hcd *xhci,
		struct xhci_virt_device *virt_dev,
		struct xhci_event_cmd *event)
{
	struct xhci_command *command;

	if (list_empty(&virt_dev->cmd_list))
		return 0;

	command = list_entry(virt_dev->cmd_list.next,
			struct xhci_command, cmd_list);
	if (xhci->cmd_ring->dequeue != command->command_trb)
		return 0;

	command->status =
		GET_COMP_CODE(event->status);
	list_del(&command->cmd_list);
	if (command->completion)
		complete(command->completion);
	else
		xhci_free_command(xhci, command);
	return 1;
}

static void handle_cmd_completion(struct xhci_hcd *xhci,
		struct xhci_event_cmd *event)
{
	int slot_id = TRB_TO_SLOT_ID(event->flags);
	u64 cmd_dma;
	dma_addr_t cmd_dequeue_dma;
	struct xhci_input_control_ctx *ctrl_ctx;
	struct xhci_virt_device *virt_dev;
	unsigned int ep_index;
	struct xhci_ring *ep_ring;
	unsigned int ep_state;

	cmd_dma = event->cmd_trb;
	cmd_dequeue_dma = xhci_trb_virt_to_dma(xhci->cmd_ring->deq_seg,
			xhci->cmd_ring->dequeue);
	
	if (cmd_dequeue_dma == 0) {
		xhci->error_bitmask |= 1 << 4;
		return;
	}
	
	if (cmd_dma != (u64) cmd_dequeue_dma) {
		xhci->error_bitmask |= 1 << 5;
		return;
	}
	switch (xhci->cmd_ring->dequeue->generic.field[3] & TRB_TYPE_BITMASK) {
	case TRB_TYPE(TRB_ENABLE_SLOT):
		if (GET_COMP_CODE(event->status) == COMP_SUCCESS)
			xhci->slot_id = slot_id;
		else
			xhci->slot_id = 0;
		complete(&xhci->addr_dev);
		break;
	case TRB_TYPE(TRB_DISABLE_SLOT):
		if (xhci->devs[slot_id])
			xhci_free_virt_device(xhci, slot_id);
		break;
	case TRB_TYPE(TRB_CONFIG_EP):
		virt_dev = xhci->devs[slot_id];
		if (handle_cmd_in_cmd_wait_list(xhci, virt_dev, event))
			break;
		
		ctrl_ctx = xhci_get_input_control_ctx(xhci,
				virt_dev->in_ctx);
		
		ep_index = xhci_last_valid_endpoint(ctrl_ctx->add_flags) - 1;
		ep_ring = xhci->devs[slot_id]->eps[ep_index].ring;
		if (!ep_ring) {
			
			xhci->devs[slot_id]->cmd_status =
				GET_COMP_CODE(event->status);
			complete(&xhci->devs[slot_id]->cmd_completion);
			break;
		}
		ep_state = xhci->devs[slot_id]->eps[ep_index].ep_state;
		xhci_dbg(xhci, "Completed config ep cmd - last ep index = %d, "
				"state = %d\n", ep_index, ep_state);
		if (xhci->quirks & XHCI_RESET_EP_QUIRK &&
				ep_state & EP_HALTED) {
			
			xhci->devs[slot_id]->eps[ep_index].ep_state &=
				~EP_HALTED;
			ring_ep_doorbell(xhci, slot_id, ep_index);
		} else {
			xhci->devs[slot_id]->cmd_status =
				GET_COMP_CODE(event->status);
			complete(&xhci->devs[slot_id]->cmd_completion);
		}
		break;
	case TRB_TYPE(TRB_EVAL_CONTEXT):
		virt_dev = xhci->devs[slot_id];
		if (handle_cmd_in_cmd_wait_list(xhci, virt_dev, event))
			break;
		xhci->devs[slot_id]->cmd_status = GET_COMP_CODE(event->status);
		complete(&xhci->devs[slot_id]->cmd_completion);
		break;
	case TRB_TYPE(TRB_ADDR_DEV):
		xhci->devs[slot_id]->cmd_status = GET_COMP_CODE(event->status);
		complete(&xhci->addr_dev);
		break;
	case TRB_TYPE(TRB_STOP_RING):
		handle_stopped_endpoint(xhci, xhci->cmd_ring->dequeue);
		break;
	case TRB_TYPE(TRB_SET_DEQ):
		handle_set_deq_completion(xhci, event, xhci->cmd_ring->dequeue);
		break;
	case TRB_TYPE(TRB_CMD_NOOP):
		++xhci->noops_handled;
		break;
	case TRB_TYPE(TRB_RESET_EP):
		handle_reset_ep_completion(xhci, event, xhci->cmd_ring->dequeue);
		break;
	default:
		
		xhci->error_bitmask |= 1 << 6;
		break;
	}
	inc_deq(xhci, xhci->cmd_ring, false);
}

static void handle_port_status(struct xhci_hcd *xhci,
		union xhci_trb *event)
{
	u32 port_id;

	
	if (GET_COMP_CODE(event->generic.field[2]) != COMP_SUCCESS) {
		xhci_warn(xhci, "WARN: xHC returned failed port status event\n");
		xhci->error_bitmask |= 1 << 8;
	}
	
	port_id = GET_PORT_ID(event->generic.field[0]);
	xhci_dbg(xhci, "Port Status Change Event for port %d\n", port_id);

	
	inc_deq(xhci, xhci->event_ring, true);
	xhci_set_hc_event_deq(xhci);

	spin_unlock(&xhci->lock);
	
	usb_hcd_poll_rh_status(xhci_to_hcd(xhci));
	spin_lock(&xhci->lock);
}


static struct xhci_segment *trb_in_td(
		struct xhci_segment *start_seg,
		union xhci_trb	*start_trb,
		union xhci_trb	*end_trb,
		dma_addr_t	suspect_dma)
{
	dma_addr_t start_dma;
	dma_addr_t end_seg_dma;
	dma_addr_t end_trb_dma;
	struct xhci_segment *cur_seg;

	start_dma = xhci_trb_virt_to_dma(start_seg, start_trb);
	cur_seg = start_seg;

	do {
		if (start_dma == 0)
			return 0;
		
		end_seg_dma = xhci_trb_virt_to_dma(cur_seg,
				&cur_seg->trbs[TRBS_PER_SEGMENT - 1]);
		
		end_trb_dma = xhci_trb_virt_to_dma(cur_seg, end_trb);

		if (end_trb_dma > 0) {
			
			if (start_dma <= end_trb_dma) {
				if (suspect_dma >= start_dma && suspect_dma <= end_trb_dma)
					return cur_seg;
			} else {
				
				if ((suspect_dma >= start_dma &&
							suspect_dma <= end_seg_dma) ||
						(suspect_dma >= cur_seg->dma &&
						 suspect_dma <= end_trb_dma))
					return cur_seg;
			}
			return 0;
		} else {
			
			if (suspect_dma >= start_dma && suspect_dma <= end_seg_dma)
				return cur_seg;
		}
		cur_seg = cur_seg->next;
		start_dma = xhci_trb_virt_to_dma(cur_seg, &cur_seg->trbs[0]);
	} while (cur_seg != start_seg);

	return 0;
}


static int handle_tx_event(struct xhci_hcd *xhci,
		struct xhci_transfer_event *event)
{
	struct xhci_virt_device *xdev;
	struct xhci_virt_ep *ep;
	struct xhci_ring *ep_ring;
	unsigned int slot_id;
	int ep_index;
	struct xhci_td *td = 0;
	dma_addr_t event_dma;
	struct xhci_segment *event_seg;
	union xhci_trb *event_trb;
	struct urb *urb = 0;
	int status = -EINPROGRESS;
	struct xhci_ep_ctx *ep_ctx;
	u32 trb_comp_code;

	xhci_dbg(xhci, "In %s\n", __func__);
	slot_id = TRB_TO_SLOT_ID(event->flags);
	xdev = xhci->devs[slot_id];
	if (!xdev) {
		xhci_err(xhci, "ERROR Transfer event pointed to bad slot\n");
		return -ENODEV;
	}

	
	ep_index = TRB_TO_EP_ID(event->flags) - 1;
	xhci_dbg(xhci, "%s - ep index = %d\n", __func__, ep_index);
	ep = &xdev->eps[ep_index];
	ep_ring = ep->ring;
	ep_ctx = xhci_get_ep_ctx(xhci, xdev->out_ctx, ep_index);
	if (!ep_ring || (ep_ctx->ep_info & EP_STATE_MASK) == EP_STATE_DISABLED) {
		xhci_err(xhci, "ERROR Transfer event pointed to disabled endpoint\n");
		return -ENODEV;
	}

	event_dma = event->buffer;
	
	xhci_dbg(xhci, "%s - checking for list empty\n", __func__);
	if (list_empty(&ep_ring->td_list)) {
		xhci_warn(xhci, "WARN Event TRB for slot %d ep %d with no TDs queued?\n",
				TRB_TO_SLOT_ID(event->flags), ep_index);
		xhci_dbg(xhci, "Event TRB with TRB type ID %u\n",
				(unsigned int) (event->flags & TRB_TYPE_BITMASK)>>10);
		xhci_print_trb_offsets(xhci, (union xhci_trb *) event);
		urb = NULL;
		goto cleanup;
	}
	xhci_dbg(xhci, "%s - getting list entry\n", __func__);
	td = list_entry(ep_ring->td_list.next, struct xhci_td, td_list);

	
	xhci_dbg(xhci, "%s - looking for TD\n", __func__);
	event_seg = trb_in_td(ep_ring->deq_seg, ep_ring->dequeue,
			td->last_trb, event_dma);
	xhci_dbg(xhci, "%s - found event_seg = %p\n", __func__, event_seg);
	if (!event_seg) {
		
		xhci_err(xhci, "ERROR Transfer event TRB DMA ptr not part of current TD\n");
		return -ESHUTDOWN;
	}
	event_trb = &event_seg->trbs[(event_dma - event_seg->dma) / sizeof(*event_trb)];
	xhci_dbg(xhci, "Event TRB with TRB type ID %u\n",
			(unsigned int) (event->flags & TRB_TYPE_BITMASK)>>10);
	xhci_dbg(xhci, "Offset 0x00 (buffer lo) = 0x%x\n",
			lower_32_bits(event->buffer));
	xhci_dbg(xhci, "Offset 0x04 (buffer hi) = 0x%x\n",
			upper_32_bits(event->buffer));
	xhci_dbg(xhci, "Offset 0x08 (transfer length) = 0x%x\n",
			(unsigned int) event->transfer_len);
	xhci_dbg(xhci, "Offset 0x0C (flags) = 0x%x\n",
			(unsigned int) event->flags);

	
	trb_comp_code = GET_COMP_CODE(event->transfer_len);
	switch (trb_comp_code) {
	
	case COMP_SUCCESS:
	case COMP_SHORT_TX:
		break;
	case COMP_STOP:
		xhci_dbg(xhci, "Stopped on Transfer TRB\n");
		break;
	case COMP_STOP_INVAL:
		xhci_dbg(xhci, "Stopped on No-op or Link TRB\n");
		break;
	case COMP_STALL:
		xhci_warn(xhci, "WARN: Stalled endpoint\n");
		ep->ep_state |= EP_HALTED;
		status = -EPIPE;
		break;
	case COMP_TRB_ERR:
		xhci_warn(xhci, "WARN: TRB error on endpoint\n");
		status = -EILSEQ;
		break;
	case COMP_TX_ERR:
		xhci_warn(xhci, "WARN: transfer error on endpoint\n");
		status = -EPROTO;
		break;
	case COMP_BABBLE:
		xhci_warn(xhci, "WARN: babble error on endpoint\n");
		status = -EOVERFLOW;
		break;
	case COMP_DB_ERR:
		xhci_warn(xhci, "WARN: HC couldn't access mem fast enough\n");
		status = -ENOSR;
		break;
	default:
		xhci_warn(xhci, "ERROR Unknown event condition, HC probably busted\n");
		urb = NULL;
		goto cleanup;
	}
	
	
	if (usb_endpoint_xfer_control(&td->urb->ep->desc)) {
		xhci_debug_trb(xhci, xhci->event_ring->dequeue);
		switch (trb_comp_code) {
		case COMP_SUCCESS:
			if (event_trb == ep_ring->dequeue) {
				xhci_warn(xhci, "WARN: Success on ctrl setup TRB without IOC set??\n");
				status = -ESHUTDOWN;
			} else if (event_trb != td->last_trb) {
				xhci_warn(xhci, "WARN: Success on ctrl data TRB without IOC set??\n");
				status = -ESHUTDOWN;
			} else {
				xhci_dbg(xhci, "Successful control transfer!\n");
				status = 0;
			}
			break;
		case COMP_SHORT_TX:
			xhci_warn(xhci, "WARN: short transfer on control ep\n");
			if (td->urb->transfer_flags & URB_SHORT_NOT_OK)
				status = -EREMOTEIO;
			else
				status = 0;
			break;
		case COMP_BABBLE:
			
			if (ep_ctx->ep_info != EP_STATE_HALTED)
				break;
			
		case COMP_STALL:
			
			if (event_trb != ep_ring->dequeue &&
					event_trb != td->last_trb)
				td->urb->actual_length =
					td->urb->transfer_buffer_length
					- TRB_LEN(event->transfer_len);
			else
				td->urb->actual_length = 0;

			ep->stopped_td = td;
			ep->stopped_trb = event_trb;
			xhci_queue_reset_ep(xhci, slot_id, ep_index);
			xhci_cleanup_stalled_ring(xhci, td->urb->dev, ep_index);
			xhci_ring_cmd_db(xhci);
			goto td_cleanup;
		default:
			
			break;
		}
		
		if (event_trb != ep_ring->dequeue) {
			
			if (event_trb == td->last_trb) {
				if (td->urb->actual_length != 0) {
					
					if ((status == -EINPROGRESS ||
								status == 0) &&
							(td->urb->transfer_flags
							 & URB_SHORT_NOT_OK))
						
						status = -EREMOTEIO;
				} else {
					td->urb->actual_length =
						td->urb->transfer_buffer_length;
				}
			} else {
			
				if (trb_comp_code != COMP_STOP_INVAL) {
					
					td->urb->actual_length =
						td->urb->transfer_buffer_length -
						TRB_LEN(event->transfer_len);
					xhci_dbg(xhci, "Waiting for status stage event\n");
					urb = NULL;
					goto cleanup;
				}
			}
		}
	} else {
		switch (trb_comp_code) {
		case COMP_SUCCESS:
			
			if (event_trb != td->last_trb) {
				xhci_warn(xhci, "WARN Successful completion "
						"on short TX\n");
				if (td->urb->transfer_flags & URB_SHORT_NOT_OK)
					status = -EREMOTEIO;
				else
					status = 0;
			} else {
				if (usb_endpoint_xfer_bulk(&td->urb->ep->desc))
					xhci_dbg(xhci, "Successful bulk "
							"transfer!\n");
				else
					xhci_dbg(xhci, "Successful interrupt "
							"transfer!\n");
				status = 0;
			}
			break;
		case COMP_SHORT_TX:
			if (td->urb->transfer_flags & URB_SHORT_NOT_OK)
				status = -EREMOTEIO;
			else
				status = 0;
			break;
		default:
			
			break;
		}
		dev_dbg(&td->urb->dev->dev,
				"ep %#x - asked for %d bytes, "
				"%d bytes untransferred\n",
				td->urb->ep->desc.bEndpointAddress,
				td->urb->transfer_buffer_length,
				TRB_LEN(event->transfer_len));
		
		if (event_trb == td->last_trb) {
			if (TRB_LEN(event->transfer_len) != 0) {
				td->urb->actual_length =
					td->urb->transfer_buffer_length -
					TRB_LEN(event->transfer_len);
				if (td->urb->transfer_buffer_length <
						td->urb->actual_length) {
					xhci_warn(xhci, "HC gave bad length "
							"of %d bytes left\n",
							TRB_LEN(event->transfer_len));
					td->urb->actual_length = 0;
					if (td->urb->transfer_flags &
							URB_SHORT_NOT_OK)
						status = -EREMOTEIO;
					else
						status = 0;
				}
				
				if (status == -EINPROGRESS) {
					if (td->urb->transfer_flags & URB_SHORT_NOT_OK)
						status = -EREMOTEIO;
					else
						status = 0;
				}
			} else {
				td->urb->actual_length = td->urb->transfer_buffer_length;
				
				if (status == -EREMOTEIO)
					status = 0;
			}
		} else {
			
			union xhci_trb *cur_trb;
			struct xhci_segment *cur_seg;

			td->urb->actual_length = 0;
			for (cur_trb = ep_ring->dequeue, cur_seg = ep_ring->deq_seg;
					cur_trb != event_trb;
					next_trb(xhci, ep_ring, &cur_seg, &cur_trb)) {
				if (TRB_TYPE(cur_trb->generic.field[3]) != TRB_TR_NOOP &&
						TRB_TYPE(cur_trb->generic.field[3]) != TRB_LINK)
					td->urb->actual_length +=
						TRB_LEN(cur_trb->generic.field[2]);
			}
			
			if (trb_comp_code != COMP_STOP_INVAL)
				td->urb->actual_length +=
					TRB_LEN(cur_trb->generic.field[2]) -
					TRB_LEN(event->transfer_len);
		}
	}
	if (trb_comp_code == COMP_STOP_INVAL ||
			trb_comp_code == COMP_STOP) {
		
		ep->stopped_td = td;
		ep->stopped_trb = event_trb;
	} else {
		if (trb_comp_code == COMP_STALL ||
				trb_comp_code == COMP_BABBLE) {
			
			ep->stopped_td = td;
			ep->stopped_trb = event_trb;
		} else {
			
			while (ep_ring->dequeue != td->last_trb)
				inc_deq(xhci, ep_ring, false);
			inc_deq(xhci, ep_ring, false);
		}

td_cleanup:
		
		urb = td->urb;
		
		if (urb->actual_length > urb->transfer_buffer_length) {
			xhci_warn(xhci, "URB transfer length is wrong, "
					"xHC issue? req. len = %u, "
					"act. len = %u\n",
					urb->transfer_buffer_length,
					urb->actual_length);
			urb->actual_length = 0;
			if (td->urb->transfer_flags & URB_SHORT_NOT_OK)
				status = -EREMOTEIO;
			else
				status = 0;
		}
		list_del(&td->td_list);
		
		if (!list_empty(&td->cancelled_td_list)) {
			list_del(&td->cancelled_td_list);
			ep->cancels_pending--;
		}
		
		if (usb_endpoint_xfer_control(&urb->ep->desc) ||
			(trb_comp_code != COMP_STALL &&
				trb_comp_code != COMP_BABBLE)) {
			kfree(td);
		}
		urb->hcpriv = NULL;
	}
cleanup:
	inc_deq(xhci, xhci->event_ring, true);
	xhci_set_hc_event_deq(xhci);

	
	if (urb) {
		usb_hcd_unlink_urb_from_ep(xhci_to_hcd(xhci), urb);
		xhci_dbg(xhci, "Giveback URB %p, len = %d, status = %d\n",
				urb, urb->actual_length, status);
		spin_unlock(&xhci->lock);
		usb_hcd_giveback_urb(xhci_to_hcd(xhci), urb, status);
		spin_lock(&xhci->lock);
	}
	return 0;
}


void xhci_handle_event(struct xhci_hcd *xhci)
{
	union xhci_trb *event;
	int update_ptrs = 1;
	int ret;

	xhci_dbg(xhci, "In %s\n", __func__);
	if (!xhci->event_ring || !xhci->event_ring->dequeue) {
		xhci->error_bitmask |= 1 << 1;
		return;
	}

	event = xhci->event_ring->dequeue;
	
	if ((event->event_cmd.flags & TRB_CYCLE) !=
			xhci->event_ring->cycle_state) {
		xhci->error_bitmask |= 1 << 2;
		return;
	}
	xhci_dbg(xhci, "%s - OS owns TRB\n", __func__);

	
	switch ((event->event_cmd.flags & TRB_TYPE_BITMASK)) {
	case TRB_TYPE(TRB_COMPLETION):
		xhci_dbg(xhci, "%s - calling handle_cmd_completion\n", __func__);
		handle_cmd_completion(xhci, &event->event_cmd);
		xhci_dbg(xhci, "%s - returned from handle_cmd_completion\n", __func__);
		break;
	case TRB_TYPE(TRB_PORT_STATUS):
		xhci_dbg(xhci, "%s - calling handle_port_status\n", __func__);
		handle_port_status(xhci, event);
		xhci_dbg(xhci, "%s - returned from handle_port_status\n", __func__);
		update_ptrs = 0;
		break;
	case TRB_TYPE(TRB_TRANSFER):
		xhci_dbg(xhci, "%s - calling handle_tx_event\n", __func__);
		ret = handle_tx_event(xhci, &event->trans_event);
		xhci_dbg(xhci, "%s - returned from handle_tx_event\n", __func__);
		if (ret < 0)
			xhci->error_bitmask |= 1 << 9;
		else
			update_ptrs = 0;
		break;
	default:
		xhci->error_bitmask |= 1 << 3;
	}

	if (update_ptrs) {
		
		inc_deq(xhci, xhci->event_ring, true);
		xhci_set_hc_event_deq(xhci);
	}
	
	xhci_handle_event(xhci);
}




static void queue_trb(struct xhci_hcd *xhci, struct xhci_ring *ring,
		bool consumer,
		u32 field1, u32 field2, u32 field3, u32 field4)
{
	struct xhci_generic_trb *trb;

	trb = &ring->enqueue->generic;
	trb->field[0] = field1;
	trb->field[1] = field2;
	trb->field[2] = field3;
	trb->field[3] = field4;
	inc_enq(xhci, ring, consumer);
}


static int prepare_ring(struct xhci_hcd *xhci, struct xhci_ring *ep_ring,
		u32 ep_state, unsigned int num_trbs, gfp_t mem_flags)
{
	
	xhci_dbg(xhci, "Endpoint state = 0x%x\n", ep_state);
	switch (ep_state) {
	case EP_STATE_DISABLED:
		
		xhci_warn(xhci, "WARN urb submitted to disabled ep\n");
		return -ENOENT;
	case EP_STATE_ERROR:
		xhci_warn(xhci, "WARN waiting for error on ep to be cleared\n");
		
		
		return -EINVAL;
	case EP_STATE_HALTED:
		xhci_dbg(xhci, "WARN halted endpoint, queueing URB anyway.\n");
	case EP_STATE_STOPPED:
	case EP_STATE_RUNNING:
		break;
	default:
		xhci_err(xhci, "ERROR unknown endpoint state for ep\n");
		
		return -EINVAL;
	}
	if (!room_on_ring(xhci, ep_ring, num_trbs)) {
		
		xhci_err(xhci, "ERROR no room on ep ring\n");
		return -ENOMEM;
	}
	return 0;
}

static int prepare_transfer(struct xhci_hcd *xhci,
		struct xhci_virt_device *xdev,
		unsigned int ep_index,
		unsigned int num_trbs,
		struct urb *urb,
		struct xhci_td **td,
		gfp_t mem_flags)
{
	int ret;
	struct xhci_ep_ctx *ep_ctx = xhci_get_ep_ctx(xhci, xdev->out_ctx, ep_index);
	ret = prepare_ring(xhci, xdev->eps[ep_index].ring,
			ep_ctx->ep_info & EP_STATE_MASK,
			num_trbs, mem_flags);
	if (ret)
		return ret;
	*td = kzalloc(sizeof(struct xhci_td), mem_flags);
	if (!*td)
		return -ENOMEM;
	INIT_LIST_HEAD(&(*td)->td_list);
	INIT_LIST_HEAD(&(*td)->cancelled_td_list);

	ret = usb_hcd_link_urb_to_ep(xhci_to_hcd(xhci), urb);
	if (unlikely(ret)) {
		kfree(*td);
		return ret;
	}

	(*td)->urb = urb;
	urb->hcpriv = (void *) (*td);
	
	list_add_tail(&(*td)->td_list, &xdev->eps[ep_index].ring->td_list);
	(*td)->start_seg = xdev->eps[ep_index].ring->enq_seg;
	(*td)->first_trb = xdev->eps[ep_index].ring->enqueue;

	return 0;
}

static unsigned int count_sg_trbs_needed(struct xhci_hcd *xhci, struct urb *urb)
{
	int num_sgs, num_trbs, running_total, temp, i;
	struct scatterlist *sg;

	sg = NULL;
	num_sgs = urb->num_sgs;
	temp = urb->transfer_buffer_length;

	xhci_dbg(xhci, "count sg list trbs: \n");
	num_trbs = 0;
	for_each_sg(urb->sg->sg, sg, num_sgs, i) {
		unsigned int previous_total_trbs = num_trbs;
		unsigned int len = sg_dma_len(sg);

		
		running_total = TRB_MAX_BUFF_SIZE -
			(sg_dma_address(sg) & ((1 << TRB_MAX_BUFF_SHIFT) - 1));
		if (running_total != 0)
			num_trbs++;

		
		while (running_total < sg_dma_len(sg)) {
			num_trbs++;
			running_total += TRB_MAX_BUFF_SIZE;
		}
		xhci_dbg(xhci, " sg #%d: dma = %#llx, len = %#x (%d), num_trbs = %d\n",
				i, (unsigned long long)sg_dma_address(sg),
				len, len, num_trbs - previous_total_trbs);

		len = min_t(int, len, temp);
		temp -= len;
		if (temp == 0)
			break;
	}
	xhci_dbg(xhci, "\n");
	if (!in_interrupt())
		dev_dbg(&urb->dev->dev, "ep %#x - urb len = %d, sglist used, num_trbs = %d\n",
				urb->ep->desc.bEndpointAddress,
				urb->transfer_buffer_length,
				num_trbs);
	return num_trbs;
}

static void check_trb_math(struct urb *urb, int num_trbs, int running_total)
{
	if (num_trbs != 0)
		dev_dbg(&urb->dev->dev, "%s - ep %#x - Miscalculated number of "
				"TRBs, %d left\n", __func__,
				urb->ep->desc.bEndpointAddress, num_trbs);
	if (running_total != urb->transfer_buffer_length)
		dev_dbg(&urb->dev->dev, "%s - ep %#x - Miscalculated tx length, "
				"queued %#x (%d), asked for %#x (%d)\n",
				__func__,
				urb->ep->desc.bEndpointAddress,
				running_total, running_total,
				urb->transfer_buffer_length,
				urb->transfer_buffer_length);
}

static void giveback_first_trb(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index, int start_cycle,
		struct xhci_generic_trb *start_trb, struct xhci_td *td)
{
	
	wmb();
	start_trb->field[3] |= start_cycle;
	ring_ep_doorbell(xhci, slot_id, ep_index);
}


int xhci_queue_intr_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ep_ctx *ep_ctx = xhci_get_ep_ctx(xhci,
			xhci->devs[slot_id]->out_ctx, ep_index);
	int xhci_interval;
	int ep_interval;

	xhci_interval = EP_INTERVAL_TO_UFRAMES(ep_ctx->ep_info);
	ep_interval = urb->interval;
	
	if (urb->dev->speed == USB_SPEED_LOW ||
			urb->dev->speed == USB_SPEED_FULL)
		ep_interval *= 8;
	
	if (xhci_interval != ep_interval) {
		if (!printk_ratelimit())
			dev_dbg(&urb->dev->dev, "Driver uses different interval"
					" (%d microframe%s) than xHCI "
					"(%d microframe%s)\n",
					ep_interval,
					ep_interval == 1 ? "" : "s",
					xhci_interval,
					xhci_interval == 1 ? "" : "s");
		urb->interval = xhci_interval;
		
		if (urb->dev->speed == USB_SPEED_LOW ||
				urb->dev->speed == USB_SPEED_FULL)
			urb->interval /= 8;
	}
	return xhci_queue_bulk_tx(xhci, GFP_ATOMIC, urb, slot_id, ep_index);
}

static int queue_bulk_sg_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ring *ep_ring;
	unsigned int num_trbs;
	struct xhci_td *td;
	struct scatterlist *sg;
	int num_sgs;
	int trb_buff_len, this_sg_len, running_total;
	bool first_trb;
	u64 addr;

	struct xhci_generic_trb *start_trb;
	int start_cycle;

	ep_ring = xhci->devs[slot_id]->eps[ep_index].ring;
	num_trbs = count_sg_trbs_needed(xhci, urb);
	num_sgs = urb->num_sgs;

	trb_buff_len = prepare_transfer(xhci, xhci->devs[slot_id],
			ep_index, num_trbs, urb, &td, mem_flags);
	if (trb_buff_len < 0)
		return trb_buff_len;
	
	start_trb = &ep_ring->enqueue->generic;
	start_cycle = ep_ring->cycle_state;

	running_total = 0;
	
	sg = urb->sg->sg;
	addr = (u64) sg_dma_address(sg);
	this_sg_len = sg_dma_len(sg);
	trb_buff_len = TRB_MAX_BUFF_SIZE -
		(addr & ((1 << TRB_MAX_BUFF_SHIFT) - 1));
	trb_buff_len = min_t(int, trb_buff_len, this_sg_len);
	if (trb_buff_len > urb->transfer_buffer_length)
		trb_buff_len = urb->transfer_buffer_length;
	xhci_dbg(xhci, "First length to xfer from 1st sglist entry = %u\n",
			trb_buff_len);

	first_trb = true;
	
	do {
		u32 field = 0;
		u32 length_field = 0;

		
		if (first_trb)
			first_trb = false;
		else
			field |= ep_ring->cycle_state;

		
		if (num_trbs > 1) {
			field |= TRB_CHAIN;
		} else {
			
			td->last_trb = ep_ring->enqueue;
			field |= TRB_IOC;
		}
		xhci_dbg(xhci, " sg entry: dma = %#x, len = %#x (%d), "
				"64KB boundary at %#x, end dma = %#x\n",
				(unsigned int) addr, trb_buff_len, trb_buff_len,
				(unsigned int) (addr + TRB_MAX_BUFF_SIZE) & ~(TRB_MAX_BUFF_SIZE - 1),
				(unsigned int) addr + trb_buff_len);
		if (TRB_MAX_BUFF_SIZE -
				(addr & ((1 << TRB_MAX_BUFF_SHIFT) - 1)) < trb_buff_len) {
			xhci_warn(xhci, "WARN: sg dma xfer crosses 64KB boundaries!\n");
			xhci_dbg(xhci, "Next boundary at %#x, end dma = %#x\n",
					(unsigned int) (addr + TRB_MAX_BUFF_SIZE) & ~(TRB_MAX_BUFF_SIZE - 1),
					(unsigned int) addr + trb_buff_len);
		}
		length_field = TRB_LEN(trb_buff_len) |
			TD_REMAINDER(urb->transfer_buffer_length - running_total) |
			TRB_INTR_TARGET(0);
		queue_trb(xhci, ep_ring, false,
				lower_32_bits(addr),
				upper_32_bits(addr),
				length_field,
				
				field | TRB_ISP | TRB_TYPE(TRB_NORMAL));
		--num_trbs;
		running_total += trb_buff_len;

		
		this_sg_len -= trb_buff_len;
		if (this_sg_len == 0) {
			--num_sgs;
			if (num_sgs == 0)
				break;
			sg = sg_next(sg);
			addr = (u64) sg_dma_address(sg);
			this_sg_len = sg_dma_len(sg);
		} else {
			addr += trb_buff_len;
		}

		trb_buff_len = TRB_MAX_BUFF_SIZE -
			(addr & ((1 << TRB_MAX_BUFF_SHIFT) - 1));
		trb_buff_len = min_t(int, trb_buff_len, this_sg_len);
		if (running_total + trb_buff_len > urb->transfer_buffer_length)
			trb_buff_len =
				urb->transfer_buffer_length - running_total;
	} while (running_total < urb->transfer_buffer_length);

	check_trb_math(urb, num_trbs, running_total);
	giveback_first_trb(xhci, slot_id, ep_index, start_cycle, start_trb, td);
	return 0;
}


int xhci_queue_bulk_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ring *ep_ring;
	struct xhci_td *td;
	int num_trbs;
	struct xhci_generic_trb *start_trb;
	bool first_trb;
	int start_cycle;
	u32 field, length_field;

	int running_total, trb_buff_len, ret;
	u64 addr;

	if (urb->sg)
		return queue_bulk_sg_tx(xhci, mem_flags, urb, slot_id, ep_index);

	ep_ring = xhci->devs[slot_id]->eps[ep_index].ring;

	num_trbs = 0;
	
	running_total = TRB_MAX_BUFF_SIZE -
		(urb->transfer_dma & ((1 << TRB_MAX_BUFF_SHIFT) - 1));

	
	if (running_total != 0 || urb->transfer_buffer_length == 0)
		num_trbs++;
	
	while (running_total < urb->transfer_buffer_length) {
		num_trbs++;
		running_total += TRB_MAX_BUFF_SIZE;
	}
	

	if (!in_interrupt())
		dev_dbg(&urb->dev->dev, "ep %#x - urb len = %#x (%d), addr = %#llx, num_trbs = %d\n",
				urb->ep->desc.bEndpointAddress,
				urb->transfer_buffer_length,
				urb->transfer_buffer_length,
				(unsigned long long)urb->transfer_dma,
				num_trbs);

	ret = prepare_transfer(xhci, xhci->devs[slot_id], ep_index,
			num_trbs, urb, &td, mem_flags);
	if (ret < 0)
		return ret;

	
	start_trb = &ep_ring->enqueue->generic;
	start_cycle = ep_ring->cycle_state;

	running_total = 0;
	
	addr = (u64) urb->transfer_dma;
	trb_buff_len = TRB_MAX_BUFF_SIZE -
		(urb->transfer_dma & ((1 << TRB_MAX_BUFF_SHIFT) - 1));
	if (urb->transfer_buffer_length < trb_buff_len)
		trb_buff_len = urb->transfer_buffer_length;

	first_trb = true;

	
	do {
		field = 0;

		
		if (first_trb)
			first_trb = false;
		else
			field |= ep_ring->cycle_state;

		
		if (num_trbs > 1) {
			field |= TRB_CHAIN;
		} else {
			
			td->last_trb = ep_ring->enqueue;
			field |= TRB_IOC;
		}
		length_field = TRB_LEN(trb_buff_len) |
			TD_REMAINDER(urb->transfer_buffer_length - running_total) |
			TRB_INTR_TARGET(0);
		queue_trb(xhci, ep_ring, false,
				lower_32_bits(addr),
				upper_32_bits(addr),
				length_field,
				
				field | TRB_ISP | TRB_TYPE(TRB_NORMAL));
		--num_trbs;
		running_total += trb_buff_len;

		
		addr += trb_buff_len;
		trb_buff_len = urb->transfer_buffer_length - running_total;
		if (trb_buff_len > TRB_MAX_BUFF_SIZE)
			trb_buff_len = TRB_MAX_BUFF_SIZE;
	} while (running_total < urb->transfer_buffer_length);

	check_trb_math(urb, num_trbs, running_total);
	giveback_first_trb(xhci, slot_id, ep_index, start_cycle, start_trb, td);
	return 0;
}


int xhci_queue_ctrl_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ring *ep_ring;
	int num_trbs;
	int ret;
	struct usb_ctrlrequest *setup;
	struct xhci_generic_trb *start_trb;
	int start_cycle;
	u32 field, length_field;
	struct xhci_td *td;

	ep_ring = xhci->devs[slot_id]->eps[ep_index].ring;

	
	if (!urb->setup_packet)
		return -EINVAL;

	if (!in_interrupt())
		xhci_dbg(xhci, "Queueing ctrl tx for slot id %d, ep %d\n",
				slot_id, ep_index);
	
	num_trbs = 2;
	
	if (urb->transfer_buffer_length > 0)
		num_trbs++;
	ret = prepare_transfer(xhci, xhci->devs[slot_id], ep_index, num_trbs,
			urb, &td, mem_flags);
	if (ret < 0)
		return ret;

	
	start_trb = &ep_ring->enqueue->generic;
	start_cycle = ep_ring->cycle_state;

	
	
	setup = (struct usb_ctrlrequest *) urb->setup_packet;
	queue_trb(xhci, ep_ring, false,
			
			setup->bRequestType | setup->bRequest << 8 | setup->wValue << 16,
			setup->wIndex | setup->wLength << 16,
			TRB_LEN(8) | TRB_INTR_TARGET(0),
			
			TRB_IDT | TRB_TYPE(TRB_SETUP));

	
	field = 0;
	length_field = TRB_LEN(urb->transfer_buffer_length) |
		TD_REMAINDER(urb->transfer_buffer_length) |
		TRB_INTR_TARGET(0);
	if (urb->transfer_buffer_length > 0) {
		if (setup->bRequestType & USB_DIR_IN)
			field |= TRB_DIR_IN;
		queue_trb(xhci, ep_ring, false,
				lower_32_bits(urb->transfer_dma),
				upper_32_bits(urb->transfer_dma),
				length_field,
				
				field | TRB_ISP | TRB_TYPE(TRB_DATA) | ep_ring->cycle_state);
	}

	
	td->last_trb = ep_ring->enqueue;

	
	
	if (urb->transfer_buffer_length > 0 && setup->bRequestType & USB_DIR_IN)
		field = 0;
	else
		field = TRB_DIR_IN;
	queue_trb(xhci, ep_ring, false,
			0,
			0,
			TRB_INTR_TARGET(0),
			
			field | TRB_IOC | TRB_TYPE(TRB_STATUS) | ep_ring->cycle_state);

	giveback_first_trb(xhci, slot_id, ep_index, start_cycle, start_trb, td);
	return 0;
}




static int queue_command(struct xhci_hcd *xhci, u32 field1, u32 field2,
		u32 field3, u32 field4, bool command_must_succeed)
{
	int reserved_trbs = xhci->cmd_ring_reserved_trbs;
	if (!command_must_succeed)
		reserved_trbs++;

	if (!room_on_ring(xhci, xhci->cmd_ring, reserved_trbs)) {
		if (!in_interrupt())
			xhci_err(xhci, "ERR: No room for command on command ring\n");
		if (command_must_succeed)
			xhci_err(xhci, "ERR: Reserved TRB counting for "
					"unfailable commands failed.\n");
		return -ENOMEM;
	}
	queue_trb(xhci, xhci->cmd_ring, false, field1, field2, field3,
			field4 | xhci->cmd_ring->cycle_state);
	return 0;
}


static int queue_cmd_noop(struct xhci_hcd *xhci)
{
	return queue_command(xhci, 0, 0, 0, TRB_TYPE(TRB_CMD_NOOP), false);
}


void *xhci_setup_one_noop(struct xhci_hcd *xhci)
{
	if (queue_cmd_noop(xhci) < 0)
		return NULL;
	xhci->noops_submitted++;
	return xhci_ring_cmd_db;
}


int xhci_queue_slot_control(struct xhci_hcd *xhci, u32 trb_type, u32 slot_id)
{
	return queue_command(xhci, 0, 0, 0,
			TRB_TYPE(trb_type) | SLOT_ID_FOR_TRB(slot_id), false);
}


int xhci_queue_address_device(struct xhci_hcd *xhci, dma_addr_t in_ctx_ptr,
		u32 slot_id)
{
	return queue_command(xhci, lower_32_bits(in_ctx_ptr),
			upper_32_bits(in_ctx_ptr), 0,
			TRB_TYPE(TRB_ADDR_DEV) | SLOT_ID_FOR_TRB(slot_id),
			false);
}


int xhci_queue_configure_endpoint(struct xhci_hcd *xhci, dma_addr_t in_ctx_ptr,
		u32 slot_id, bool command_must_succeed)
{
	return queue_command(xhci, lower_32_bits(in_ctx_ptr),
			upper_32_bits(in_ctx_ptr), 0,
			TRB_TYPE(TRB_CONFIG_EP) | SLOT_ID_FOR_TRB(slot_id),
			command_must_succeed);
}


int xhci_queue_evaluate_context(struct xhci_hcd *xhci, dma_addr_t in_ctx_ptr,
		u32 slot_id)
{
	return queue_command(xhci, lower_32_bits(in_ctx_ptr),
			upper_32_bits(in_ctx_ptr), 0,
			TRB_TYPE(TRB_EVAL_CONTEXT) | SLOT_ID_FOR_TRB(slot_id),
			false);
}

int xhci_queue_stop_endpoint(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index)
{
	u32 trb_slot_id = SLOT_ID_FOR_TRB(slot_id);
	u32 trb_ep_index = EP_ID_FOR_TRB(ep_index);
	u32 type = TRB_TYPE(TRB_STOP_RING);

	return queue_command(xhci, 0, 0, 0,
			trb_slot_id | trb_ep_index | type, false);
}


static int queue_set_tr_deq(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index, struct xhci_segment *deq_seg,
		union xhci_trb *deq_ptr, u32 cycle_state)
{
	dma_addr_t addr;
	u32 trb_slot_id = SLOT_ID_FOR_TRB(slot_id);
	u32 trb_ep_index = EP_ID_FOR_TRB(ep_index);
	u32 type = TRB_TYPE(TRB_SET_DEQ);

	addr = xhci_trb_virt_to_dma(deq_seg, deq_ptr);
	if (addr == 0) {
		xhci_warn(xhci, "WARN Cannot submit Set TR Deq Ptr\n");
		xhci_warn(xhci, "WARN deq seg = %p, deq pt = %p\n",
				deq_seg, deq_ptr);
		return 0;
	}
	return queue_command(xhci, lower_32_bits(addr) | cycle_state,
			upper_32_bits(addr), 0,
			trb_slot_id | trb_ep_index | type, false);
}

int xhci_queue_reset_ep(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index)
{
	u32 trb_slot_id = SLOT_ID_FOR_TRB(slot_id);
	u32 trb_ep_index = EP_ID_FOR_TRB(ep_index);
	u32 type = TRB_TYPE(TRB_RESET_EP);

	return queue_command(xhci, 0, 0, 0, trb_slot_id | trb_ep_index | type,
			false);
}
