

#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/firewire.h>
#include <linux/firewire-constants.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <asm/byteorder.h>

#include "core.h"



int fw_iso_buffer_init(struct fw_iso_buffer *buffer, struct fw_card *card,
		       int page_count, enum dma_data_direction direction)
{
	int i, j;
	dma_addr_t address;

	buffer->page_count = page_count;
	buffer->direction = direction;

	buffer->pages = kmalloc(page_count * sizeof(buffer->pages[0]),
				GFP_KERNEL);
	if (buffer->pages == NULL)
		goto out;

	for (i = 0; i < buffer->page_count; i++) {
		buffer->pages[i] = alloc_page(GFP_KERNEL | GFP_DMA32 | __GFP_ZERO);
		if (buffer->pages[i] == NULL)
			goto out_pages;

		address = dma_map_page(card->device, buffer->pages[i],
				       0, PAGE_SIZE, direction);
		if (dma_mapping_error(card->device, address)) {
			__free_page(buffer->pages[i]);
			goto out_pages;
		}
		set_page_private(buffer->pages[i], address);
	}

	return 0;

 out_pages:
	for (j = 0; j < i; j++) {
		address = page_private(buffer->pages[j]);
		dma_unmap_page(card->device, address,
			       PAGE_SIZE, direction);
		__free_page(buffer->pages[j]);
	}
	kfree(buffer->pages);
 out:
	buffer->pages = NULL;

	return -ENOMEM;
}
EXPORT_SYMBOL(fw_iso_buffer_init);

int fw_iso_buffer_map(struct fw_iso_buffer *buffer, struct vm_area_struct *vma)
{
	unsigned long uaddr;
	int i, err;

	uaddr = vma->vm_start;
	for (i = 0; i < buffer->page_count; i++) {
		err = vm_insert_page(vma, uaddr, buffer->pages[i]);
		if (err)
			return err;

		uaddr += PAGE_SIZE;
	}

	return 0;
}

void fw_iso_buffer_destroy(struct fw_iso_buffer *buffer,
			   struct fw_card *card)
{
	int i;
	dma_addr_t address;

	for (i = 0; i < buffer->page_count; i++) {
		address = page_private(buffer->pages[i]);
		dma_unmap_page(card->device, address,
			       PAGE_SIZE, buffer->direction);
		__free_page(buffer->pages[i]);
	}

	kfree(buffer->pages);
	buffer->pages = NULL;
}
EXPORT_SYMBOL(fw_iso_buffer_destroy);

struct fw_iso_context *fw_iso_context_create(struct fw_card *card,
		int type, int channel, int speed, size_t header_size,
		fw_iso_callback_t callback, void *callback_data)
{
	struct fw_iso_context *ctx;

	ctx = card->driver->allocate_iso_context(card,
						 type, channel, header_size);
	if (IS_ERR(ctx))
		return ctx;

	ctx->card = card;
	ctx->type = type;
	ctx->channel = channel;
	ctx->speed = speed;
	ctx->header_size = header_size;
	ctx->callback = callback;
	ctx->callback_data = callback_data;

	return ctx;
}
EXPORT_SYMBOL(fw_iso_context_create);

void fw_iso_context_destroy(struct fw_iso_context *ctx)
{
	struct fw_card *card = ctx->card;

	card->driver->free_iso_context(ctx);
}
EXPORT_SYMBOL(fw_iso_context_destroy);

int fw_iso_context_start(struct fw_iso_context *ctx,
			 int cycle, int sync, int tags)
{
	return ctx->card->driver->start_iso(ctx, cycle, sync, tags);
}
EXPORT_SYMBOL(fw_iso_context_start);

int fw_iso_context_queue(struct fw_iso_context *ctx,
			 struct fw_iso_packet *packet,
			 struct fw_iso_buffer *buffer,
			 unsigned long payload)
{
	struct fw_card *card = ctx->card;

	return card->driver->queue_iso(ctx, packet, buffer, payload);
}
EXPORT_SYMBOL(fw_iso_context_queue);

int fw_iso_context_stop(struct fw_iso_context *ctx)
{
	return ctx->card->driver->stop_iso(ctx);
}
EXPORT_SYMBOL(fw_iso_context_stop);



static int manage_bandwidth(struct fw_card *card, int irm_id, int generation,
			    int bandwidth, bool allocate, __be32 data[2])
{
	int try, new, old = allocate ? BANDWIDTH_AVAILABLE_INITIAL : 0;

	
	for (try = 0; try < 5; try++) {
		new = allocate ? old - bandwidth : old + bandwidth;
		if (new < 0 || new > BANDWIDTH_AVAILABLE_INITIAL)
			break;

		data[0] = cpu_to_be32(old);
		data[1] = cpu_to_be32(new);
		switch (fw_run_transaction(card, TCODE_LOCK_COMPARE_SWAP,
				irm_id, generation, SCODE_100,
				CSR_REGISTER_BASE + CSR_BANDWIDTH_AVAILABLE,
				data, 8)) {
		case RCODE_GENERATION:
			
			return allocate ? -EAGAIN : bandwidth;

		case RCODE_COMPLETE:
			if (be32_to_cpup(data) == old)
				return bandwidth;

			old = be32_to_cpup(data);
			
		}
	}

	return -EIO;
}

static int manage_channel(struct fw_card *card, int irm_id, int generation,
		u32 channels_mask, u64 offset, bool allocate, __be32 data[2])
{
	__be32 c, all, old;
	int i, retry = 5;

	old = all = allocate ? cpu_to_be32(~0) : 0;

	for (i = 0; i < 32; i++) {
		if (!(channels_mask & 1 << i))
			continue;

		c = cpu_to_be32(1 << (31 - i));
		if ((old & c) != (all & c))
			continue;

		data[0] = old;
		data[1] = old ^ c;
		switch (fw_run_transaction(card, TCODE_LOCK_COMPARE_SWAP,
					   irm_id, generation, SCODE_100,
					   offset, data, 8)) {
		case RCODE_GENERATION:
			
			return allocate ? -EAGAIN : i;

		case RCODE_COMPLETE:
			if (data[0] == old)
				return i;

			old = data[0];

			
			if ((data[0] & c) == (data[1] & c))
				continue;

			
		default:
			if (retry--)
				i--;
		}
	}

	return -EIO;
}

static void deallocate_channel(struct fw_card *card, int irm_id,
			       int generation, int channel, __be32 buffer[2])
{
	u32 mask;
	u64 offset;

	mask = channel < 32 ? 1 << channel : 1 << (channel - 32);
	offset = channel < 32 ? CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_HI :
				CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_LO;

	manage_channel(card, irm_id, generation, mask, offset, false, buffer);
}


void fw_iso_resource_manage(struct fw_card *card, int generation,
			    u64 channels_mask, int *channel, int *bandwidth,
			    bool allocate, __be32 buffer[2])
{
	u32 channels_hi = channels_mask;	
	u32 channels_lo = channels_mask >> 32;	
	int irm_id, ret, c = -EINVAL;

	spin_lock_irq(&card->lock);
	irm_id = card->irm_node->node_id;
	spin_unlock_irq(&card->lock);

	if (channels_hi)
		c = manage_channel(card, irm_id, generation, channels_hi,
				CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_HI,
				allocate, buffer);
	if (channels_lo && c < 0) {
		c = manage_channel(card, irm_id, generation, channels_lo,
				CSR_REGISTER_BASE + CSR_CHANNELS_AVAILABLE_LO,
				allocate, buffer);
		if (c >= 0)
			c += 32;
	}
	*channel = c;

	if (allocate && channels_mask != 0 && c < 0)
		*bandwidth = 0;

	if (*bandwidth == 0)
		return;

	ret = manage_bandwidth(card, irm_id, generation, *bandwidth,
			       allocate, buffer);
	if (ret < 0)
		*bandwidth = 0;

	if (allocate && ret < 0 && c >= 0) {
		deallocate_channel(card, irm_id, generation, c, buffer);
		*channel = ret;
	}
}
