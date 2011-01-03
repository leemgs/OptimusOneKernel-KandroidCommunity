





#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/bitops.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/compat.h>
#include <linux/cdev.h>

#include "dv1394.h"
#include "dv1394-private.h"
#include "highlevel.h"
#include "hosts.h"
#include "ieee1394.h"
#include "ieee1394_core.h"
#include "ieee1394_hotplug.h"
#include "ieee1394_types.h"
#include "nodemgr.h"
#include "ohci1394.h"



#define DV1394_DEBUG_LEVEL 0




#if DV1394_DEBUG_LEVEL >= 2
#define irq_printk( args... ) printk( args )
#else
#define irq_printk( args... ) do {} while (0)
#endif

#if DV1394_DEBUG_LEVEL >= 1
#define debug_printk( args... ) printk( args)
#else
#define debug_printk( args... ) do {} while (0)
#endif



static inline void flush_pci_write(struct ti_ohci *ohci)
{
	mb();
	reg_read(ohci, OHCI1394_IsochronousCycleTimer);
}

static void it_tasklet_func(unsigned long data);
static void ir_tasklet_func(unsigned long data);

#ifdef CONFIG_COMPAT
static long dv1394_compat_ioctl(struct file *file, unsigned int cmd,
			       unsigned long arg);
#endif




static LIST_HEAD(dv1394_cards);
static DEFINE_SPINLOCK(dv1394_cards_lock);



static inline struct video_card* file_to_video_card(struct file *file)
{
	return (struct video_card*) file->private_data;
}



static void frame_reset(struct frame *f)
{
	f->state = FRAME_CLEAR;
	f->done = 0;
	f->n_packets = 0;
	f->frame_begin_timestamp = NULL;
	f->assigned_timestamp = 0;
	f->cip_syt1 = NULL;
	f->cip_syt2 = NULL;
	f->mid_frame_timestamp = NULL;
	f->frame_end_timestamp = NULL;
	f->frame_end_branch = NULL;
}

static struct frame* frame_new(unsigned int frame_num, struct video_card *video)
{
	struct frame *f = kmalloc(sizeof(*f), GFP_KERNEL);
	if (!f)
		return NULL;

	f->video = video;
	f->frame_num = frame_num;

	f->header_pool = pci_alloc_consistent(f->video->ohci->dev, PAGE_SIZE, &f->header_pool_dma);
	if (!f->header_pool) {
		printk(KERN_ERR "dv1394: failed to allocate CIP header pool\n");
		kfree(f);
		return NULL;
	}

	debug_printk("dv1394: frame_new: allocated CIP header pool at virt 0x%08lx (contig) dma 0x%08lx size %ld\n",
		     (unsigned long) f->header_pool, (unsigned long) f->header_pool_dma, PAGE_SIZE);

	f->descriptor_pool_size = MAX_PACKETS * sizeof(struct DMA_descriptor_block);
	
	f->descriptor_pool_size += PAGE_SIZE - (f->descriptor_pool_size%PAGE_SIZE);

	f->descriptor_pool = pci_alloc_consistent(f->video->ohci->dev,
						  f->descriptor_pool_size,
						  &f->descriptor_pool_dma);
	if (!f->descriptor_pool) {
		pci_free_consistent(f->video->ohci->dev, PAGE_SIZE, f->header_pool, f->header_pool_dma);
		kfree(f);
		return NULL;
	}

	debug_printk("dv1394: frame_new: allocated DMA program memory at virt 0x%08lx (contig) dma 0x%08lx size %ld\n",
		     (unsigned long) f->descriptor_pool, (unsigned long) f->descriptor_pool_dma, f->descriptor_pool_size);

	f->data = 0;
	frame_reset(f);

	return f;
}

static void frame_delete(struct frame *f)
{
	pci_free_consistent(f->video->ohci->dev, PAGE_SIZE, f->header_pool, f->header_pool_dma);
	pci_free_consistent(f->video->ohci->dev, f->descriptor_pool_size, f->descriptor_pool, f->descriptor_pool_dma);
	kfree(f);
}






static void frame_prepare(struct video_card *video, unsigned int this_frame)
{
	struct frame *f = video->frames[this_frame];
	int last_frame;

	struct DMA_descriptor_block *block;
	dma_addr_t block_dma;
	struct CIP_header *cip;
	dma_addr_t cip_dma;

	unsigned int n_descriptors, full_packets, packets_per_frame, payload_size;

	
	int empty_packet, first_packet, last_packet, mid_packet;

	__le32 *branch_address, *last_branch_address = NULL;
	unsigned long data_p;
	int first_packet_empty = 0;
	u32 cycleTimer, ct_sec, ct_cyc, ct_off;
	unsigned long irq_flags;

	irq_printk("frame_prepare( %d ) ---------------------\n", this_frame);

	full_packets = 0;



	if (video->pal_or_ntsc == DV1394_PAL)
		packets_per_frame = DV1394_PAL_PACKETS_PER_FRAME;
	else
		packets_per_frame = DV1394_NTSC_PACKETS_PER_FRAME;

	while ( full_packets < packets_per_frame ) {
		empty_packet = first_packet = last_packet = mid_packet = 0;

		data_p = f->data + full_packets * 480;

		
		
		

		

		if (f->n_packets >= MAX_PACKETS) {
			printk(KERN_ERR "dv1394: FATAL ERROR: max packet count exceeded\n");
			return;
		}

		
		block = &(f->descriptor_pool[f->n_packets]);

		
		block_dma = ((unsigned long) block - (unsigned long) f->descriptor_pool) + f->descriptor_pool_dma;


		
		if ( ((unsigned long) &(f->header_pool[f->n_packets]) - (unsigned long) f->header_pool)
		    > PAGE_SIZE) {
			printk(KERN_ERR "dv1394: FATAL ERROR: no room to allocate CIP header\n");
			return;
		}

		cip = &(f->header_pool[f->n_packets]);

		
		cip_dma = (unsigned long) cip % PAGE_SIZE + f->header_pool_dma;

		

		if (video->cip_accum > (video->cip_d - video->cip_n)) {
			empty_packet = 1;
			payload_size = 8;
			video->cip_accum -= (video->cip_d - video->cip_n);
		} else {
			payload_size = 488;
			video->cip_accum += video->cip_n;
		}

		

		if (f->n_packets == 0) {
			first_packet = 1;
		} else if ( full_packets == (packets_per_frame-1) ) {
			last_packet = 1;
		} else if (f->n_packets == packets_per_frame) {
			mid_packet = 1;
		}


		
		
		

		

		
		if (first_packet) {
			f->cip_syt1 = cip;
			if (empty_packet)
				first_packet_empty = 1;

		} else if (first_packet_empty && (f->n_packets == 1) ) {
			
			f->cip_syt2 = cip;
		}

		fill_cip_header(cip,
				
				reg_read(video->ohci, OHCI1394_NodeID) & 0x3F,
				video->continuity_counter,
				video->pal_or_ntsc,
				0xFFFF );

		
		if ( ! empty_packet )
			video->continuity_counter++;

		
		
		

		
		fill_output_more_immediate( &(block->u.out.omi), 1, video->channel, 0, payload_size);

		if (empty_packet) {
			
			fill_output_last( &(block->u.out.u.empty.ol),

					  
					  (first_packet || mid_packet || last_packet) ? 1 : 0,

					  
					  (first_packet || mid_packet || last_packet) ? 1 : 0,

					  sizeof(struct CIP_header), 
					  cip_dma);

			if (first_packet)
				f->frame_begin_timestamp = &(block->u.out.u.empty.ol.q[3]);
			else if (mid_packet)
				f->mid_frame_timestamp = &(block->u.out.u.empty.ol.q[3]);
			else if (last_packet) {
				f->frame_end_timestamp = &(block->u.out.u.empty.ol.q[3]);
				f->frame_end_branch = &(block->u.out.u.empty.ol.q[2]);
			}

			branch_address = &(block->u.out.u.empty.ol.q[2]);
			n_descriptors = 3;
			if (first_packet)
				f->first_n_descriptors = n_descriptors;

		} else { 

			
			fill_output_more( &(block->u.out.u.full.om),
					  sizeof(struct CIP_header), 
					  cip_dma);


			
			

			
			if ( (PAGE_SIZE- ((unsigned long)data_p % PAGE_SIZE) ) < 480 ) {

				

				fill_output_more( &(block->u.out.u.full.u.cross.om),
						  
						  PAGE_SIZE - (data_p % PAGE_SIZE),

						  
						  dma_region_offset_to_bus(&video->dv_buf,
									   data_p - (unsigned long) video->dv_buf.kvirt));

				fill_output_last( &(block->u.out.u.full.u.cross.ol),

						  
						  (first_packet || mid_packet || last_packet) ? 1 : 0,

						  
						  (first_packet || mid_packet || last_packet) ? 1 : 0,

						  
						  480 - (PAGE_SIZE - (data_p % PAGE_SIZE)),

						  
						  dma_region_offset_to_bus(&video->dv_buf,
									   data_p + PAGE_SIZE - (data_p % PAGE_SIZE) - (unsigned long) video->dv_buf.kvirt));

				if (first_packet)
					f->frame_begin_timestamp = &(block->u.out.u.full.u.cross.ol.q[3]);
				else if (mid_packet)
					f->mid_frame_timestamp = &(block->u.out.u.full.u.cross.ol.q[3]);
				else if (last_packet) {
					f->frame_end_timestamp = &(block->u.out.u.full.u.cross.ol.q[3]);
					f->frame_end_branch = &(block->u.out.u.full.u.cross.ol.q[2]);
				}

				branch_address = &(block->u.out.u.full.u.cross.ol.q[2]);

				n_descriptors = 5;
				if (first_packet)
					f->first_n_descriptors = n_descriptors;

				full_packets++;

			} else {
				

				fill_output_last( &(block->u.out.u.full.u.nocross.ol),

						  
						  (first_packet || mid_packet || last_packet) ? 1 : 0,

						  
						  (first_packet || mid_packet || last_packet) ? 1 : 0,

						  480, 


						  
						  dma_region_offset_to_bus(&video->dv_buf,
									   data_p - (unsigned long) video->dv_buf.kvirt));

				if (first_packet)
					f->frame_begin_timestamp = &(block->u.out.u.full.u.nocross.ol.q[3]);
				else if (mid_packet)
					f->mid_frame_timestamp = &(block->u.out.u.full.u.nocross.ol.q[3]);
				else if (last_packet) {
					f->frame_end_timestamp = &(block->u.out.u.full.u.nocross.ol.q[3]);
					f->frame_end_branch = &(block->u.out.u.full.u.nocross.ol.q[2]);
				}

				branch_address = &(block->u.out.u.full.u.nocross.ol.q[2]);

				n_descriptors = 4;
				if (first_packet)
					f->first_n_descriptors = n_descriptors;

				full_packets++;
			}
		}

		

		

		if (last_branch_address) {
			*(last_branch_address) = cpu_to_le32(block_dma | n_descriptors);
		}

		last_branch_address = branch_address;


		f->n_packets++;

	}

	
	*(f->frame_end_branch) = cpu_to_le32(f->descriptor_pool_dma | f->first_n_descriptors);

	
	dma_region_sync_for_device(&video->dv_buf, f->data - (unsigned long) video->dv_buf.kvirt, video->frame_size);

	
	spin_lock_irqsave(&video->spinlock, irq_flags);

	f->state = FRAME_READY;

	video->n_clear_frames--;

	last_frame = video->first_clear_frame - 1;
	if (last_frame == -1)
		last_frame = video->n_frames-1;

	video->first_clear_frame = (video->first_clear_frame + 1) % video->n_frames;

	irq_printk("   frame %d prepared, active_frame = %d, n_clear_frames = %d, first_clear_frame = %d\n last=%d\n",
		   this_frame, video->active_frame, video->n_clear_frames, video->first_clear_frame, last_frame);

	irq_printk("   begin_ts %08lx mid_ts %08lx end_ts %08lx end_br %08lx\n",
		   (unsigned long) f->frame_begin_timestamp,
		   (unsigned long) f->mid_frame_timestamp,
		   (unsigned long) f->frame_end_timestamp,
		   (unsigned long) f->frame_end_branch);

	if (video->active_frame != -1) {

		
		
		if (video->frames[last_frame]->frame_end_branch) {
			u32 temp;

			
			*(video->frames[last_frame]->frame_end_branch) = cpu_to_le32(f->descriptor_pool_dma | f->first_n_descriptors);

			
			wmb();

			
			temp = le32_to_cpu(*(video->frames[last_frame]->frame_end_branch - 2));
			temp &= 0xF7CFFFFF;
			*(video->frames[last_frame]->frame_end_branch - 2) = cpu_to_le32(temp);

			
			flush_pci_write(video->ohci);

			


			irq_printk("     new frame %d linked onto DMA chain\n", this_frame);

		} else {
			printk(KERN_ERR "dv1394: last frame not ready???\n");
		}

	} else {

		u32 transmit_sec, transmit_cyc;
		u32 ts_cyc, ts_off;

		
		video->active_frame = this_frame;

	        
		reg_write(video->ohci, video->ohci_IsoXmitCommandPtr,
			  video->frames[video->active_frame]->descriptor_pool_dma |
			  f->first_n_descriptors);

		

		cycleTimer = reg_read(video->ohci, OHCI1394_IsochronousCycleTimer);

		ct_sec = cycleTimer >> 25;
		ct_cyc = (cycleTimer >> 12) & 0x1FFF;
		ct_off = cycleTimer & 0xFFF;

		transmit_sec = ct_sec;
		transmit_cyc = ct_cyc + 100;

		transmit_sec += transmit_cyc/8000;
		transmit_cyc %= 8000;

		ts_off = ct_off;
		ts_cyc = transmit_cyc + 3;
		ts_cyc %= 8000;

		f->assigned_timestamp = (ts_cyc&0xF) << 12;

		
		if (f->cip_syt1) {
			f->cip_syt1->b[6] = f->assigned_timestamp >> 8;
			f->cip_syt1->b[7] = f->assigned_timestamp & 0xFF;
		}
		if (f->cip_syt2) {
			f->cip_syt2->b[6] = f->assigned_timestamp >> 8;
			f->cip_syt2->b[7] = f->assigned_timestamp & 0xFF;
		}

		

		

		reg_write(video->ohci, video->ohci_IsoXmitContextControlClear, 0xFFFFFFFF);
		wmb();

		

#if 0
		reg_write(video->ohci, video->ohci_IsoXmitContextControlSet,
			  (1 << 31) 
			  | ( (transmit_sec & 0x3) << 29)
			  | (transmit_cyc << 16));
		wmb();
#endif

		video->dma_running = 1;

		
		reg_write(video->ohci, video->ohci_IsoXmitContextControlSet, 0x8000);
		flush_pci_write(video->ohci);

		

		debug_printk("    Cycle = %4u ContextControl = %08x CmdPtr = %08x\n",
			     (reg_read(video->ohci, OHCI1394_IsochronousCycleTimer) >> 12) & 0x1FFF,
			     reg_read(video->ohci, video->ohci_IsoXmitContextControlSet),
			     reg_read(video->ohci, video->ohci_IsoXmitCommandPtr));

		debug_printk("    DMA start - current cycle %4u, transmit cycle %4u (%2u), assigning ts cycle %2u\n",
			     ct_cyc, transmit_cyc, transmit_cyc & 0xF, ts_cyc & 0xF);

#if DV1394_DEBUG_LEVEL >= 2
		{
			
			int i = 0;
			while (i < 20) {
				mb();
				mdelay(1);
				if (reg_read(video->ohci, video->ohci_IsoXmitContextControlSet) & (1 << 10)) {
					printk("DMA ACTIVE after %d msec\n", i);
					break;
				}
				i++;
			}

			printk("set = %08x, cmdPtr = %08x\n",
			       reg_read(video->ohci, video->ohci_IsoXmitContextControlSet),
			       reg_read(video->ohci, video->ohci_IsoXmitCommandPtr)
			       );

			if ( ! (reg_read(video->ohci, video->ohci_IsoXmitContextControlSet) &  (1 << 10)) ) {
				printk("DMA did NOT go active after 20ms, event = %x\n",
				       reg_read(video->ohci, video->ohci_IsoXmitContextControlSet) & 0x1F);
			} else
				printk("DMA is RUNNING!\n");
		}
#endif

	}


	spin_unlock_irqrestore(&video->spinlock, irq_flags);
}







static void inline
frame_put_packet (struct frame *f, struct packet *p)
{
	int section_type = p->data[0] >> 5;           
	int dif_sequence = p->data[1] >> 4;           
	int dif_block = p->data[2];

	
	if (dif_sequence > 11 || dif_block > 149) return;

	switch (section_type) {
	case 0:           
	        memcpy( (void *) f->data + dif_sequence * 150 * 80, p->data, 480);
	        break;

	case 1:           
	        memcpy( (void *) f->data + dif_sequence * 150 * 80 + (1 + dif_block) * 80, p->data, 480);
	        break;

	case 2:           
	        memcpy( (void *) f->data + dif_sequence * 150 * 80 + (3 + dif_block) * 80, p->data, 480);
	        break;

	case 3:           
	        memcpy( (void *) f->data + dif_sequence * 150 * 80 + (6 + dif_block * 16) * 80, p->data, 480);
	        break;

	case 4:           
	        memcpy( (void *) f->data + dif_sequence * 150 * 80 + (7 + (dif_block / 15) + dif_block) * 80, p->data, 480);
	        break;

	default:           
	        break;
	}
}


static void start_dma_receive(struct video_card *video)
{
	if (video->first_run == 1) {
		video->first_run = 0;

		
		video->n_clear_frames = 0;
		video->first_clear_frame = -1;
		video->current_packet = 0;
		video->active_frame = 0;

		
		reg_write(video->ohci, video->ohci_IsoRcvContextControlClear, 0xFFFFFFFF);
		wmb();

		
		reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, 0x40000000);

		
		reg_write(video->ohci, video->ohci_IsoRcvContextMatch, 0xf0000000 | video->channel);

		
		reg_write(video->ohci, video->ohci_IsoRcvCommandPtr,
			  video->frames[0]->descriptor_pool_dma | 1); 
		wmb();

		video->dma_running = 1;

		
		reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, 0x8000);
		flush_pci_write(video->ohci);

		debug_printk("dv1394: DMA started\n");

#if DV1394_DEBUG_LEVEL >= 2
		{
			int i;

			for (i = 0; i < 1000; ++i) {
				mdelay(1);
				if (reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) & (1 << 10)) {
					printk("DMA ACTIVE after %d msec\n", i);
					break;
				}
			}
			if ( reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) &  (1 << 11) ) {
				printk("DEAD, event = %x\n",
					   reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) & 0x1F);
			} else
				printk("RUNNING!\n");
		}
#endif
	} else if ( reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) &  (1 << 11) ) {
		debug_printk("DEAD, event = %x\n",
			     reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) & 0x1F);

		
		reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, (1 << 12));
	}
}




static void receive_packets(struct video_card *video)
{
	struct DMA_descriptor_block *block = NULL;
	dma_addr_t block_dma = 0;
	struct packet *data = NULL;
	dma_addr_t data_dma = 0;
	__le32 *last_branch_address = NULL;
	unsigned long irq_flags;
	int want_interrupt = 0;
	struct frame *f = NULL;
	int i, j;

	spin_lock_irqsave(&video->spinlock, irq_flags);

	for (j = 0; j < video->n_frames; j++) {

		
		if (j > 0 && f != NULL && f->frame_end_branch != NULL)
			*(f->frame_end_branch) = cpu_to_le32(video->frames[j]->descriptor_pool_dma | 1); 

		f = video->frames[j];

		for (i = 0; i < MAX_PACKETS; i++) {
			
			block = &(f->descriptor_pool[i]);
			block_dma = ((unsigned long) block - (unsigned long) f->descriptor_pool) + f->descriptor_pool_dma;

			data = ((struct packet*)video->packet_buf.kvirt) + f->frame_num * MAX_PACKETS + i;
			data_dma = dma_region_offset_to_bus( &video->packet_buf,
							     ((unsigned long) data - (unsigned long) video->packet_buf.kvirt) );

			
			want_interrupt = ((i % (MAX_PACKETS/2)) == 0 || i == (MAX_PACKETS-1));
			fill_input_last( &(block->u.in.il), want_interrupt, 512, data_dma);

			
			last_branch_address = f->frame_end_branch;

			if (last_branch_address != NULL)
				*(last_branch_address) = cpu_to_le32(block_dma | 1); 

			f->frame_end_branch = &(block->u.in.il.q[2]);
		}

	} 

	spin_unlock_irqrestore(&video->spinlock, irq_flags);

}





static int do_dv1394_init(struct video_card *video, struct dv1394_init *init)
{
	unsigned long flags, new_buf_size;
	int i;
	u64 chan_mask;
	int retval = -EINVAL;

	debug_printk("dv1394: initialising %d\n", video->id);
	if (init->api_version != DV1394_API_VERSION)
		return -EINVAL;

	
	if ( (init->n_frames < 2) || (init->n_frames > DV1394_MAX_FRAMES) )
		return -EINVAL;

	if ( (init->format != DV1394_NTSC) && (init->format != DV1394_PAL) )
		return -EINVAL;

	if ( (init->syt_offset == 0) || (init->syt_offset > 50) )
		
		init->syt_offset = 3;

	if (init->channel > 63)
		init->channel = 63;

	chan_mask = (u64)1 << init->channel;

	
	if (init->format == DV1394_NTSC)
		new_buf_size = DV1394_NTSC_FRAME_SIZE * init->n_frames;
	else
		new_buf_size = DV1394_PAL_FRAME_SIZE * init->n_frames;

	
	if (new_buf_size % PAGE_SIZE) new_buf_size += PAGE_SIZE - (new_buf_size % PAGE_SIZE);

	
	if (video->dv_buf.kvirt && video->dv_buf_size != new_buf_size) {
		printk("dv1394: re-sizing the DMA buffer is not allowed\n");
		return -EINVAL;
	}

	
	

	do_dv1394_shutdown(video, 0);

	
	spin_lock_irqsave(&video->ohci->IR_channel_lock, flags);
	if (video->ohci->ISO_channel_usage & chan_mask) {
		spin_unlock_irqrestore(&video->ohci->IR_channel_lock, flags);
		retval = -EBUSY;
		goto err;
	}
	video->ohci->ISO_channel_usage |= chan_mask;
	spin_unlock_irqrestore(&video->ohci->IR_channel_lock, flags);

	video->channel = init->channel;

	
	video->n_frames = init->n_frames;
	video->pal_or_ntsc = init->format;

	video->cip_accum = 0;
	video->continuity_counter = 0;

	video->active_frame = -1;
	video->first_clear_frame = 0;
	video->n_clear_frames = video->n_frames;
	video->dropped_frames = 0;

	video->write_off = 0;

	video->first_run = 1;
	video->current_packet = -1;
	video->first_frame = 0;

	if (video->pal_or_ntsc == DV1394_NTSC) {
		video->cip_n = init->cip_n != 0 ? init->cip_n : CIP_N_NTSC;
		video->cip_d = init->cip_d != 0 ? init->cip_d : CIP_D_NTSC;
		video->frame_size = DV1394_NTSC_FRAME_SIZE;
	} else {
		video->cip_n = init->cip_n != 0 ? init->cip_n : CIP_N_PAL;
		video->cip_d = init->cip_d != 0 ? init->cip_d : CIP_D_PAL;
		video->frame_size = DV1394_PAL_FRAME_SIZE;
	}

	video->syt_offset = init->syt_offset;

	

	if (video->ohci_it_ctx == -1) {
		ohci1394_init_iso_tasklet(&video->it_tasklet, OHCI_ISO_TRANSMIT,
					  it_tasklet_func, (unsigned long) video);

		if (ohci1394_register_iso_tasklet(video->ohci, &video->it_tasklet) < 0) {
			printk(KERN_ERR "dv1394: could not find an available IT DMA context\n");
			retval = -EBUSY;
			goto err;
		}

		video->ohci_it_ctx = video->it_tasklet.context;
		debug_printk("dv1394: claimed IT DMA context %d\n", video->ohci_it_ctx);
	}

	if (video->ohci_ir_ctx == -1) {
		ohci1394_init_iso_tasklet(&video->ir_tasklet, OHCI_ISO_RECEIVE,
					  ir_tasklet_func, (unsigned long) video);

		if (ohci1394_register_iso_tasklet(video->ohci, &video->ir_tasklet) < 0) {
			printk(KERN_ERR "dv1394: could not find an available IR DMA context\n");
			retval = -EBUSY;
			goto err;
		}
		video->ohci_ir_ctx = video->ir_tasklet.context;
		debug_printk("dv1394: claimed IR DMA context %d\n", video->ohci_ir_ctx);
	}

	
	for (i = 0; i < init->n_frames; i++) {
		video->frames[i] = frame_new(i, video);

		if (!video->frames[i]) {
			printk(KERN_ERR "dv1394: Cannot allocate frame structs\n");
			retval = -ENOMEM;
			goto err;
		}
	}

	if (!video->dv_buf.kvirt) {
		
		retval = dma_region_alloc(&video->dv_buf, new_buf_size, video->ohci->dev, PCI_DMA_TODEVICE);
		if (retval)
			goto err;

		video->dv_buf_size = new_buf_size;

		debug_printk("dv1394: Allocated %d frame buffers, total %u pages (%u DMA pages), %lu bytes\n", 
			     video->n_frames, video->dv_buf.n_pages,
			     video->dv_buf.n_dma_pages, video->dv_buf_size);
	}

	
	for (i = 0; i < video->n_frames; i++)
		video->frames[i]->data = (unsigned long) video->dv_buf.kvirt + i * video->frame_size;

	if (!video->packet_buf.kvirt) {
		
		video->packet_buf_size = sizeof(struct packet) * video->n_frames * MAX_PACKETS;
		if (video->packet_buf_size % PAGE_SIZE)
			video->packet_buf_size += PAGE_SIZE - (video->packet_buf_size % PAGE_SIZE);

		retval = dma_region_alloc(&video->packet_buf, video->packet_buf_size,
					  video->ohci->dev, PCI_DMA_FROMDEVICE);
		if (retval)
			goto err;

		debug_printk("dv1394: Allocated %d packets in buffer, total %u pages (%u DMA pages), %lu bytes\n",
				 video->n_frames*MAX_PACKETS, video->packet_buf.n_pages,
				 video->packet_buf.n_dma_pages, video->packet_buf_size);
	}

	
	
	video->ohci_IsoXmitContextControlSet = OHCI1394_IsoXmitContextControlSet+16*video->ohci_it_ctx;
	video->ohci_IsoXmitContextControlClear = OHCI1394_IsoXmitContextControlClear+16*video->ohci_it_ctx;
	video->ohci_IsoXmitCommandPtr = OHCI1394_IsoXmitCommandPtr+16*video->ohci_it_ctx;

	
	reg_write(video->ohci, OHCI1394_IsoXmitIntMaskSet, (1 << video->ohci_it_ctx));
	debug_printk("dv1394: interrupts enabled for IT context %d\n", video->ohci_it_ctx);

	
	
	video->ohci_IsoRcvContextControlSet = OHCI1394_IsoRcvContextControlSet+32*video->ohci_ir_ctx;
	video->ohci_IsoRcvContextControlClear = OHCI1394_IsoRcvContextControlClear+32*video->ohci_ir_ctx;
	video->ohci_IsoRcvCommandPtr = OHCI1394_IsoRcvCommandPtr+32*video->ohci_ir_ctx;
	video->ohci_IsoRcvContextMatch = OHCI1394_IsoRcvContextMatch+32*video->ohci_ir_ctx;

	
	reg_write(video->ohci, OHCI1394_IsoRecvIntMaskSet, (1 << video->ohci_ir_ctx) );
	debug_printk("dv1394: interrupts enabled for IR context %d\n", video->ohci_ir_ctx);

	return 0;

err:
	do_dv1394_shutdown(video, 1);
	return retval;
}



static int do_dv1394_init_default(struct video_card *video)
{
	struct dv1394_init init;

	init.api_version = DV1394_API_VERSION;
	init.n_frames = DV1394_MAX_FRAMES / 4;
	init.channel = video->channel;
	init.format = video->pal_or_ntsc;
	init.cip_n = video->cip_n;
	init.cip_d = video->cip_d;
	init.syt_offset = video->syt_offset;

	return do_dv1394_init(video, &init);
}


static void stop_dma(struct video_card *video)
{
	unsigned long flags;
	int i;

	
	spin_lock_irqsave(&video->spinlock, flags);

	video->dma_running = 0;

	if ( (video->ohci_it_ctx == -1) && (video->ohci_ir_ctx == -1) )
		goto out;

	
	if ( (video->active_frame != -1) ||
	    (reg_read(video->ohci, video->ohci_IsoXmitContextControlClear) & (1 << 10)) ||
	    (reg_read(video->ohci, video->ohci_IsoRcvContextControlClear) &  (1 << 10)) ) {

		
		reg_write(video->ohci, video->ohci_IsoXmitContextControlClear, (1 << 15));
		reg_write(video->ohci, video->ohci_IsoRcvContextControlClear, (1 << 15));
		flush_pci_write(video->ohci);

		video->active_frame = -1;
		video->first_run = 1;

		
		i = 0;
		while (i < 1000) {

			
			udelay(100);

			if ( (reg_read(video->ohci, video->ohci_IsoXmitContextControlClear) & (1 << 10)) ||
			    (reg_read(video->ohci, video->ohci_IsoRcvContextControlClear)  & (1 << 10)) ) {
				
				debug_printk("dv1394: stop_dma: DMA not stopped yet\n" );
				mb();
			} else {
				debug_printk("dv1394: stop_dma: DMA stopped safely after %d ms\n", i/10);
				break;
			}

			i++;
		}

		if (i == 1000) {
			printk(KERN_ERR "dv1394: stop_dma: DMA still going after %d ms!\n", i/10);
		}
	}
	else
		debug_printk("dv1394: stop_dma: already stopped.\n");

out:
	spin_unlock_irqrestore(&video->spinlock, flags);
}



static void do_dv1394_shutdown(struct video_card *video, int free_dv_buf)
{
	int i;

	debug_printk("dv1394: shutdown...\n");

	
	stop_dma(video);

	
	if (video->ohci_it_ctx != -1) {
		video->ohci_IsoXmitContextControlSet = 0;
		video->ohci_IsoXmitContextControlClear = 0;
		video->ohci_IsoXmitCommandPtr = 0;

		
		reg_write(video->ohci, OHCI1394_IsoXmitIntMaskClear, (1 << video->ohci_it_ctx));

		
		ohci1394_unregister_iso_tasklet(video->ohci, &video->it_tasklet);
		debug_printk("dv1394: IT context %d released\n", video->ohci_it_ctx);
		video->ohci_it_ctx = -1;
	}

	if (video->ohci_ir_ctx != -1) {
		video->ohci_IsoRcvContextControlSet = 0;
		video->ohci_IsoRcvContextControlClear = 0;
		video->ohci_IsoRcvCommandPtr = 0;
		video->ohci_IsoRcvContextMatch = 0;

		
		reg_write(video->ohci, OHCI1394_IsoRecvIntMaskClear, (1 << video->ohci_ir_ctx));

		
		ohci1394_unregister_iso_tasklet(video->ohci, &video->ir_tasklet);
		debug_printk("dv1394: IR context %d released\n", video->ohci_ir_ctx);
		video->ohci_ir_ctx = -1;
	}

	
	if (video->channel != -1) {
		u64 chan_mask;
		unsigned long flags;

		chan_mask = (u64)1 << video->channel;

		spin_lock_irqsave(&video->ohci->IR_channel_lock, flags);
		video->ohci->ISO_channel_usage &= ~(chan_mask);
		spin_unlock_irqrestore(&video->ohci->IR_channel_lock, flags);

		video->channel = -1;
	}

	
	for (i = 0; i < DV1394_MAX_FRAMES; i++) {
		if (video->frames[i])
			frame_delete(video->frames[i]);
		video->frames[i] = NULL;
	}

	video->n_frames = 0;

	

	if (free_dv_buf) {
		dma_region_free(&video->dv_buf);
		video->dv_buf_size = 0;
	}

	
	dma_region_free(&video->packet_buf);
	video->packet_buf_size = 0;

	debug_printk("dv1394: shutdown OK\n");
}



static int dv1394_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_card *video = file_to_video_card(file);
	int retval = -EINVAL;

	
	if (!mutex_trylock(&video->mtx))
		return -EAGAIN;

	if ( ! video_card_initialized(video) ) {
		retval = do_dv1394_init_default(video);
		if (retval)
			goto out;
	}

	retval = dma_region_mmap(&video->dv_buf, file, vma);
out:
	mutex_unlock(&video->mtx);
	return retval;
}




static unsigned int dv1394_poll(struct file *file, struct poll_table_struct *wait)
{
	struct video_card *video = file_to_video_card(file);
	unsigned int mask = 0;
	unsigned long flags;

	poll_wait(file, &video->waitq, wait);

	spin_lock_irqsave(&video->spinlock, flags);
	if ( video->n_frames == 0 ) {

	} else if ( video->active_frame == -1 ) {
		
		mask |= POLLOUT;
	} else {
		
		if (video->n_clear_frames >0)
			mask |= POLLOUT | POLLIN;
	}
	spin_unlock_irqrestore(&video->spinlock, flags);

	return mask;
}

static int dv1394_fasync(int fd, struct file *file, int on)
{
	

	struct video_card *video = file_to_video_card(file);

	return fasync_helper(fd, file, on, &video->fasync);
}

static ssize_t dv1394_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct video_card *video = file_to_video_card(file);
	DECLARE_WAITQUEUE(wait, current);
	ssize_t ret;
	size_t cnt;
	unsigned long flags;
	int target_frame;

	
	if (file->f_flags & O_NONBLOCK) {
		if (!mutex_trylock(&video->mtx))
			return -EAGAIN;
	} else {
		if (mutex_lock_interruptible(&video->mtx))
			return -ERESTARTSYS;
	}

	if ( !video_card_initialized(video) ) {
		ret = do_dv1394_init_default(video);
		if (ret) {
			mutex_unlock(&video->mtx);
			return ret;
		}
	}

	ret = 0;
	add_wait_queue(&video->waitq, &wait);

	while (count > 0) {

		

		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irqsave(&video->spinlock, flags);

		target_frame = video->first_clear_frame;

		spin_unlock_irqrestore(&video->spinlock, flags);

		if (video->frames[target_frame]->state == FRAME_CLEAR) {

			
			cnt = video->frame_size - (video->write_off - target_frame * video->frame_size);

		} else {
			
			cnt = 0;
		}

		if (cnt > count)
			cnt = count;

		if (cnt <= 0) {
			
			if (file->f_flags & O_NONBLOCK) {
				if (!ret)
					ret = -EAGAIN;
				break;
			}
			if (signal_pending(current)) {
				if (!ret)
					ret = -ERESTARTSYS;
				break;
			}

			schedule();

			continue; 
		}

		if (copy_from_user(video->dv_buf.kvirt + video->write_off, buffer, cnt)) {
			if (!ret)
				ret = -EFAULT;
			break;
		}

		video->write_off = (video->write_off + cnt) % (video->n_frames * video->frame_size);

		count -= cnt;
		buffer += cnt;
		ret += cnt;

		if (video->write_off == video->frame_size * ((target_frame + 1) % video->n_frames))
				frame_prepare(video, target_frame);
	}

	remove_wait_queue(&video->waitq, &wait);
	set_current_state(TASK_RUNNING);
	mutex_unlock(&video->mtx);
	return ret;
}


static ssize_t dv1394_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
	struct video_card *video = file_to_video_card(file);
	DECLARE_WAITQUEUE(wait, current);
	ssize_t ret;
	size_t cnt;
	unsigned long flags;
	int target_frame;

	
	if (file->f_flags & O_NONBLOCK) {
		if (!mutex_trylock(&video->mtx))
			return -EAGAIN;
	} else {
		if (mutex_lock_interruptible(&video->mtx))
			return -ERESTARTSYS;
	}

	if ( !video_card_initialized(video) ) {
		ret = do_dv1394_init_default(video);
		if (ret) {
			mutex_unlock(&video->mtx);
			return ret;
		}
		video->continuity_counter = -1;

		receive_packets(video);

		start_dma_receive(video);
	}

	ret = 0;
	add_wait_queue(&video->waitq, &wait);

	while (count > 0) {

		

		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irqsave(&video->spinlock, flags);

		target_frame = video->first_clear_frame;

		spin_unlock_irqrestore(&video->spinlock, flags);

		if (target_frame >= 0 &&
			video->n_clear_frames > 0 &&
			video->frames[target_frame]->state == FRAME_CLEAR) {

			
			cnt = video->frame_size - (video->write_off - target_frame * video->frame_size);

		} else {
			
			cnt = 0;
		}

		if (cnt > count)
			cnt = count;

		if (cnt <= 0) {
			
			if (file->f_flags & O_NONBLOCK) {
				if (!ret)
					ret = -EAGAIN;
				break;
			}
			if (signal_pending(current)) {
				if (!ret)
					ret = -ERESTARTSYS;
				break;
			}

			schedule();

			continue; 
		}

		if (copy_to_user(buffer, video->dv_buf.kvirt + video->write_off, cnt)) {
				if (!ret)
					ret = -EFAULT;
				break;
		}

		video->write_off = (video->write_off + cnt) % (video->n_frames * video->frame_size);

		count -= cnt;
		buffer += cnt;
		ret += cnt;

		if (video->write_off == video->frame_size * ((target_frame + 1) % video->n_frames)) {
			spin_lock_irqsave(&video->spinlock, flags);
			video->n_clear_frames--;
			video->first_clear_frame = (video->first_clear_frame + 1) % video->n_frames;
			spin_unlock_irqrestore(&video->spinlock, flags);
		}
	}

	remove_wait_queue(&video->waitq, &wait);
	set_current_state(TASK_RUNNING);
	mutex_unlock(&video->mtx);
	return ret;
}




static long dv1394_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct video_card *video = file_to_video_card(file);
	unsigned long flags;
	int ret = -EINVAL;
	void __user *argp = (void __user *)arg;

	DECLARE_WAITQUEUE(wait, current);

	
	if (file->f_flags & O_NONBLOCK) {
		if (!mutex_trylock(&video->mtx))
			return -EAGAIN;
	} else {
		if (mutex_lock_interruptible(&video->mtx))
			return -ERESTARTSYS;
	}

	switch(cmd)
	{
	case DV1394_IOC_SUBMIT_FRAMES: {
		unsigned int n_submit;

		if ( !video_card_initialized(video) ) {
			ret = do_dv1394_init_default(video);
			if (ret)
				goto out;
		}

		n_submit = (unsigned int) arg;

		if (n_submit > video->n_frames) {
			ret = -EINVAL;
			goto out;
		}

		while (n_submit > 0) {

			add_wait_queue(&video->waitq, &wait);
			set_current_state(TASK_INTERRUPTIBLE);

			spin_lock_irqsave(&video->spinlock, flags);

			
			while (video->frames[video->first_clear_frame]->state != FRAME_CLEAR) {

				spin_unlock_irqrestore(&video->spinlock, flags);

				if (signal_pending(current)) {
					remove_wait_queue(&video->waitq, &wait);
					set_current_state(TASK_RUNNING);
					ret = -EINTR;
					goto out;
				}

				schedule();
				set_current_state(TASK_INTERRUPTIBLE);

				spin_lock_irqsave(&video->spinlock, flags);
			}
			spin_unlock_irqrestore(&video->spinlock, flags);

			remove_wait_queue(&video->waitq, &wait);
			set_current_state(TASK_RUNNING);

			frame_prepare(video, video->first_clear_frame);

			n_submit--;
		}

		ret = 0;
		break;
	}

	case DV1394_IOC_WAIT_FRAMES: {
		unsigned int n_wait;

		if ( !video_card_initialized(video) ) {
			ret = -EINVAL;
			goto out;
		}

		n_wait = (unsigned int) arg;

		

		if (n_wait > (video->n_frames-1) ) {
			ret = -EINVAL;
			goto out;
		}

		add_wait_queue(&video->waitq, &wait);
		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irqsave(&video->spinlock, flags);

		while (video->n_clear_frames < n_wait) {

			spin_unlock_irqrestore(&video->spinlock, flags);

			if (signal_pending(current)) {
				remove_wait_queue(&video->waitq, &wait);
				set_current_state(TASK_RUNNING);
				ret = -EINTR;
				goto out;
			}

			schedule();
			set_current_state(TASK_INTERRUPTIBLE);

			spin_lock_irqsave(&video->spinlock, flags);
		}

		spin_unlock_irqrestore(&video->spinlock, flags);

		remove_wait_queue(&video->waitq, &wait);
		set_current_state(TASK_RUNNING);
		ret = 0;
		break;
	}

	case DV1394_IOC_RECEIVE_FRAMES: {
		unsigned int n_recv;

		if ( !video_card_initialized(video) ) {
			ret = -EINVAL;
			goto out;
		}

		n_recv = (unsigned int) arg;

		
		if (n_recv > (video->n_frames-1) ) {
			ret = -EINVAL;
			goto out;
		}

		spin_lock_irqsave(&video->spinlock, flags);

		
		video->n_clear_frames -= n_recv;

		
		video->first_clear_frame = (video->first_clear_frame + n_recv) % video->n_frames;

		
		video->dropped_frames = 0;

		spin_unlock_irqrestore(&video->spinlock, flags);

		ret = 0;
		break;
	}

	case DV1394_IOC_START_RECEIVE: {
		if ( !video_card_initialized(video) ) {
			ret = do_dv1394_init_default(video);
			if (ret)
				goto out;
		}

		video->continuity_counter = -1;

		receive_packets(video);

		start_dma_receive(video);

		ret = 0;
		break;
	}

	case DV1394_IOC_INIT: {
		struct dv1394_init init;
		if (!argp) {
			ret = do_dv1394_init_default(video);
		} else {
			if (copy_from_user(&init, argp, sizeof(init))) {
				ret = -EFAULT;
				goto out;
			}
			ret = do_dv1394_init(video, &init);
		}
		break;
	}

	case DV1394_IOC_SHUTDOWN:
		do_dv1394_shutdown(video, 0);
		ret = 0;
		break;


        case DV1394_IOC_GET_STATUS: {
		struct dv1394_status status;

		if ( !video_card_initialized(video) ) {
			ret = -EINVAL;
			goto out;
		}

		status.init.api_version = DV1394_API_VERSION;
		status.init.channel = video->channel;
		status.init.n_frames = video->n_frames;
		status.init.format = video->pal_or_ntsc;
		status.init.cip_n = video->cip_n;
		status.init.cip_d = video->cip_d;
		status.init.syt_offset = video->syt_offset;

		status.first_clear_frame = video->first_clear_frame;

		
		spin_lock_irqsave(&video->spinlock, flags);

		status.active_frame = video->active_frame;
		status.n_clear_frames = video->n_clear_frames;

		status.dropped_frames = video->dropped_frames;

		
		video->dropped_frames = 0;

		spin_unlock_irqrestore(&video->spinlock, flags);

		if (copy_to_user(argp, &status, sizeof(status))) {
			ret = -EFAULT;
			goto out;
		}

		ret = 0;
		break;
	}

	default:
		break;
	}

 out:
	mutex_unlock(&video->mtx);
	return ret;
}



static int dv1394_open(struct inode *inode, struct file *file)
{
	struct video_card *video = NULL;

	if (file->private_data) {
		video = (struct video_card*) file->private_data;

	} else {
		
		unsigned long flags;
		int idx = ieee1394_file_to_instance(file);

		spin_lock_irqsave(&dv1394_cards_lock, flags);
		if (!list_empty(&dv1394_cards)) {
			struct video_card *p;
			list_for_each_entry(p, &dv1394_cards, list) {
				if ((p->id) == idx) {
					video = p;
					break;
				}
			}
		}
		spin_unlock_irqrestore(&dv1394_cards_lock, flags);

		if (!video) {
			debug_printk("dv1394: OHCI card %d not found", idx);
			return -ENODEV;
		}

		file->private_data = (void*) video;
	}

#ifndef DV1394_ALLOW_MORE_THAN_ONE_OPEN

	if ( test_and_set_bit(0, &video->open) ) {
		
		return -EBUSY;
 	}

#endif

	printk(KERN_INFO "%s: NOTE, the dv1394 interface is unsupported "
	       "and will not be available in the new firewire driver stack. "
	       "Try libraw1394 based programs instead.\n", current->comm);

	return 0;
}


static int dv1394_release(struct inode *inode, struct file *file)
{
	struct video_card *video = file_to_video_card(file);

	
	do_dv1394_shutdown(video, 1);

	
	clear_bit(0, &video->open);

	return 0;
}




static void it_tasklet_func(unsigned long data)
{
	int wake = 0;
	struct video_card *video = (struct video_card*) data;

	spin_lock(&video->spinlock);

	if (!video->dma_running)
		goto out;

	irq_printk("ContextControl = %08x, CommandPtr = %08x\n",
	       reg_read(video->ohci, video->ohci_IsoXmitContextControlSet),
	       reg_read(video->ohci, video->ohci_IsoXmitCommandPtr)
	       );


	if ( (video->ohci_it_ctx != -1) &&
	    (reg_read(video->ohci, video->ohci_IsoXmitContextControlSet) & (1 << 10)) ) {

		struct frame *f;
		unsigned int frame, i;


		if (video->active_frame == -1)
			frame = 0;
		else
			frame = video->active_frame;

		
		for (i = 0; i < video->n_frames; i++, frame = (frame+1) % video->n_frames) {

			irq_printk("IRQ checking frame %d...", frame);
			f = video->frames[frame];
			if (f->state != FRAME_READY) {
				irq_printk("clear, skipping\n");
				
				continue;
			}

			irq_printk("DMA\n");

			
			if ( *(f->frame_begin_timestamp) ) {
				int prev_frame;
				struct frame *prev_f;



				
				irq_printk("  BEGIN\n");

				prev_frame = frame - 1;
				if (prev_frame == -1)
					prev_frame += video->n_frames;
				prev_f = video->frames[prev_frame];

				
				if ( (prev_f->state == FRAME_READY) &&
				    prev_f->done && (!f->done) )
				{
					frame_reset(prev_f);
					video->n_clear_frames++;
					wake = 1;
					video->active_frame = frame;

					irq_printk("  BEGIN - freeing previous frame %d, new active frame is %d\n", prev_frame, frame);
				} else {
					irq_printk("  BEGIN - can't free yet\n");
				}

				f->done = 1;
			}


			
			if ( *(f->mid_frame_timestamp) ) {
				struct frame *next_frame;
				u32 begin_ts, ts_cyc, ts_off;

				*(f->mid_frame_timestamp) = 0;

				begin_ts = le32_to_cpu(*(f->frame_begin_timestamp));

				irq_printk("  MIDDLE - first packet was sent at cycle %4u (%2u), assigned timestamp was (%2u) %4u\n",
					   begin_ts & 0x1FFF, begin_ts & 0xF,
					   f->assigned_timestamp >> 12, f->assigned_timestamp & 0xFFF);

				
				next_frame = video->frames[ (frame+1) % video->n_frames ];

				if (next_frame->state == FRAME_READY) {
					irq_printk("  MIDDLE - next frame is ready, good\n");
				} else {
					debug_printk("dv1394: Underflow! At least one frame has been dropped.\n");
					next_frame = f;
				}

				
				ts_cyc = begin_ts & 0xF;
				
				ts_cyc += f->n_packets + video->syt_offset ;

				ts_off = 0;

				ts_cyc += ts_off/3072;
				ts_off %= 3072;

				next_frame->assigned_timestamp = ((ts_cyc&0xF) << 12) + ts_off;
				if (next_frame->cip_syt1) {
					next_frame->cip_syt1->b[6] = next_frame->assigned_timestamp >> 8;
					next_frame->cip_syt1->b[7] = next_frame->assigned_timestamp & 0xFF;
				}
				if (next_frame->cip_syt2) {
					next_frame->cip_syt2->b[6] = next_frame->assigned_timestamp >> 8;
					next_frame->cip_syt2->b[7] = next_frame->assigned_timestamp & 0xFF;
				}

			}

			
			if ( *(f->frame_end_timestamp) ) {

				*(f->frame_end_timestamp) = 0;

				debug_printk("  END - the frame looped at least once\n");

				video->dropped_frames++;
			}

		} 
	}

	if (wake) {
		kill_fasync(&video->fasync, SIGIO, POLL_OUT);

		
		wake_up_interruptible(&video->waitq);
	}

out:
	spin_unlock(&video->spinlock);
}

static void ir_tasklet_func(unsigned long data)
{
	int wake = 0;
	struct video_card *video = (struct video_card*) data;

	spin_lock(&video->spinlock);

	if (!video->dma_running)
		goto out;

	if ( (video->ohci_ir_ctx != -1) &&
	    (reg_read(video->ohci, video->ohci_IsoRcvContextControlSet) & (1 << 10)) ) {

		int sof=0; 
		struct frame *f;
		u16 packet_length, packet_time;
		int i, dbc=0;
		struct DMA_descriptor_block *block = NULL;
		u16 xferstatus;

		int next_i, prev_i;
		struct DMA_descriptor_block *next = NULL;
		dma_addr_t next_dma = 0;
		struct DMA_descriptor_block *prev = NULL;

		
		for (i = 0; i < video->n_frames*MAX_PACKETS; i++) {
			struct packet *p = dma_region_i(&video->packet_buf, struct packet, video->current_packet);

			
			dma_region_sync_for_cpu(&video->packet_buf,
						(unsigned long) p - (unsigned long) video->packet_buf.kvirt,
						sizeof(struct packet));

			packet_length = le16_to_cpu(p->data_length);
			packet_time   = le16_to_cpu(p->timestamp);

			irq_printk("received packet %02d, timestamp=%04x, length=%04x, sof=%02x%02x\n", video->current_packet,
				   packet_time, packet_length,
				   p->data[0], p->data[1]);

			
			f = video->frames[video->current_packet / MAX_PACKETS];
			block = &(f->descriptor_pool[video->current_packet % MAX_PACKETS]);
			xferstatus = le32_to_cpu(block->u.in.il.q[3]) >> 16;
			xferstatus &= 0x1F;
			irq_printk("ir_tasklet_func: xferStatus/resCount [%d] = 0x%08x\n", i, le32_to_cpu(block->u.in.il.q[3]) );

			
			f = video->frames[video->active_frame];

			
			if (packet_length > 8 && xferstatus == 0x11) {
				
				
				sof = ( (p->data[0] >> 5) == 0 && (p->data[1] >> 4) == 0);

				dbc = (int) (p->cip_h1 >> 24);
				if ( video->continuity_counter != -1 && dbc > ((video->continuity_counter + 1) % 256) )
				{
					printk(KERN_WARNING "dv1394: discontinuity detected, dropping all frames\n" );
					video->dropped_frames += video->n_clear_frames + 1;
					video->first_frame = 0;
					video->n_clear_frames = 0;
					video->first_clear_frame = -1;
				}
				video->continuity_counter = dbc;

				if (!video->first_frame) {
					if (sof) {
						video->first_frame = 1;
					}

				} else if (sof) {
					
					frame_reset(f);  
					video->n_clear_frames++;
					if (video->n_clear_frames > video->n_frames) {
						video->dropped_frames++;
						printk(KERN_WARNING "dv1394: dropped a frame during reception\n" );
						video->n_clear_frames = video->n_frames-1;
						video->first_clear_frame = (video->first_clear_frame + 1) % video->n_frames;
					}
					if (video->first_clear_frame == -1)
						video->first_clear_frame = video->active_frame;

					
					video->active_frame = (video->active_frame + 1) % video->n_frames;
					f = video->frames[video->active_frame];
					irq_printk("   frame received, active_frame = %d, n_clear_frames = %d, first_clear_frame = %d\n",
						   video->active_frame, video->n_clear_frames, video->first_clear_frame);
				}
				if (video->first_frame) {
					if (sof) {
						
						f->state = FRAME_READY;
					}

					
					if (f->n_packets > (video->frame_size / 480)) {
						printk(KERN_ERR "frame buffer overflow during receive\n");
					}

					frame_put_packet(f, p);

				} 
			}

			
			else if (xferstatus == 0) {
				break;
			}

			
			block->u.in.il.q[3] = cpu_to_le32(512);

			
			next_i = video->current_packet;
			f = video->frames[next_i / MAX_PACKETS];
			next = &(f->descriptor_pool[next_i % MAX_PACKETS]);
			next_dma = ((unsigned long) block - (unsigned long) f->descriptor_pool) + f->descriptor_pool_dma;
			next->u.in.il.q[0] |= cpu_to_le32(3 << 20); 
			next->u.in.il.q[2] = cpu_to_le32(0); 

			
			prev_i = (next_i == 0) ? (MAX_PACKETS * video->n_frames - 1) : (next_i - 1);
			f = video->frames[prev_i / MAX_PACKETS];
			prev = &(f->descriptor_pool[prev_i % MAX_PACKETS]);
			if (prev_i % (MAX_PACKETS/2)) {
				prev->u.in.il.q[0] &= ~cpu_to_le32(3 << 20); 
			} else {
				prev->u.in.il.q[0] |= cpu_to_le32(3 << 20); 
			}
			prev->u.in.il.q[2] = cpu_to_le32(next_dma | 1); 
			wmb();

			
			reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, (1 << 12));

			
			video->current_packet = (video->current_packet + 1) % (MAX_PACKETS * video->n_frames);

		} 

		wake = 1; 

	} 

	if (wake) {
		kill_fasync(&video->fasync, SIGIO, POLL_IN);

		
		wake_up_interruptible(&video->waitq);
	}

out:
	spin_unlock(&video->spinlock);
}

static struct cdev dv1394_cdev;
static const struct file_operations dv1394_fops=
{
	.owner =	THIS_MODULE,
	.poll =         dv1394_poll,
	.unlocked_ioctl = dv1394_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = dv1394_compat_ioctl,
#endif
	.mmap =		dv1394_mmap,
	.open =		dv1394_open,
	.write =        dv1394_write,
	.read =         dv1394_read,
	.release =	dv1394_release,
	.fasync =       dv1394_fasync,
};




#ifdef MODULE
static const struct ieee1394_device_id dv1394_id_table[] = {
	{
		.match_flags	= IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
		.specifier_id	= AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
		.version	= AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{ }
};

MODULE_DEVICE_TABLE(ieee1394, dv1394_id_table);
#endif 

static struct hpsb_protocol_driver dv1394_driver = {
	.name = "dv1394",
};




static int dv1394_init(struct ti_ohci *ohci, enum pal_or_ntsc format, enum modes mode)
{
	struct video_card *video;
	unsigned long flags;
	int i;

	video = kzalloc(sizeof(*video), GFP_KERNEL);
	if (!video) {
		printk(KERN_ERR "dv1394: cannot allocate video_card\n");
		return -1;
	}

	video->ohci = ohci;
	
	video->id = ohci->host->id << 2;
	if (format == DV1394_NTSC)
		video->id |= mode;
	else
		video->id |= 2 + mode;

	video->ohci_it_ctx = -1;
	video->ohci_ir_ctx = -1;

	video->ohci_IsoXmitContextControlSet = 0;
	video->ohci_IsoXmitContextControlClear = 0;
	video->ohci_IsoXmitCommandPtr = 0;

	video->ohci_IsoRcvContextControlSet = 0;
	video->ohci_IsoRcvContextControlClear = 0;
	video->ohci_IsoRcvCommandPtr = 0;
	video->ohci_IsoRcvContextMatch = 0;

	video->n_frames = 0; 
	video->channel = 63; 
	video->active_frame = -1;

	
	video->pal_or_ntsc = format;
	video->cip_n = 0; 
	video->cip_d = 0;
	video->syt_offset = 0;
	video->mode = mode;

	for (i = 0; i < DV1394_MAX_FRAMES; i++)
		video->frames[i] = NULL;

	dma_region_init(&video->dv_buf);
	video->dv_buf_size = 0;
	dma_region_init(&video->packet_buf);
	video->packet_buf_size = 0;

	clear_bit(0, &video->open);
	spin_lock_init(&video->spinlock);
	video->dma_running = 0;
	mutex_init(&video->mtx);
	init_waitqueue_head(&video->waitq);
	video->fasync = NULL;

	spin_lock_irqsave(&dv1394_cards_lock, flags);
	INIT_LIST_HEAD(&video->list);
	list_add_tail(&video->list, &dv1394_cards);
	spin_unlock_irqrestore(&dv1394_cards_lock, flags);

	debug_printk("dv1394: dv1394_init() OK on ID %d\n", video->id);
	return 0;
}

static void dv1394_remove_host(struct hpsb_host *host)
{
	struct video_card *video, *tmp_video;
	unsigned long flags;
	int found_ohci_card = 0;

	do {
		video = NULL;
		spin_lock_irqsave(&dv1394_cards_lock, flags);
		list_for_each_entry(tmp_video, &dv1394_cards, list) {
			if ((tmp_video->id >> 2) == host->id) {
				list_del(&tmp_video->list);
				video = tmp_video;
				found_ohci_card = 1;
				break;
			}
		}
		spin_unlock_irqrestore(&dv1394_cards_lock, flags);

		if (video) {
			do_dv1394_shutdown(video, 1);
			kfree(video);
		}
	} while (video);

	if (found_ohci_card)
		device_destroy(hpsb_protocol_class, MKDEV(IEEE1394_MAJOR,
			   IEEE1394_MINOR_BLOCK_DV1394 * 16 + (host->id << 2)));
}

static void dv1394_add_host(struct hpsb_host *host)
{
	struct ti_ohci *ohci;
	int id = host->id;

	
	if (strcmp(host->driver->name, OHCI1394_DRIVER_NAME))
		return;

	ohci = (struct ti_ohci *)host->hostdata;

	device_create(hpsb_protocol_class, NULL,
		      MKDEV(IEEE1394_MAJOR,
			    IEEE1394_MINOR_BLOCK_DV1394 * 16 + (id<<2)),
		      NULL, "dv1394-%d", id);

	dv1394_init(ohci, DV1394_NTSC, MODE_RECEIVE);
	dv1394_init(ohci, DV1394_NTSC, MODE_TRANSMIT);
	dv1394_init(ohci, DV1394_PAL, MODE_RECEIVE);
	dv1394_init(ohci, DV1394_PAL, MODE_TRANSMIT);
}




static void dv1394_host_reset(struct hpsb_host *host)
{
	struct ti_ohci *ohci;
	struct video_card *video = NULL, *tmp_vid;
	unsigned long flags;

	
	if (strcmp(host->driver->name, OHCI1394_DRIVER_NAME))
		return;

	ohci = (struct ti_ohci *)host->hostdata;


	
	spin_lock_irqsave(&dv1394_cards_lock, flags);
	list_for_each_entry(tmp_vid, &dv1394_cards, list) {
		if ((tmp_vid->id >> 2) == host->id) {
			video = tmp_vid;
			break;
		}
	}
	spin_unlock_irqrestore(&dv1394_cards_lock, flags);

	if (!video)
		return;


	spin_lock_irqsave(&video->spinlock, flags);

	if (!video->dma_running)
		goto out;

	
	if (video->ohci_it_ctx != -1) {
		u32 ctx;

		ctx = reg_read(video->ohci, video->ohci_IsoXmitContextControlSet);

		
		if ( (ctx & (1<<15)) &&
		    !(ctx & (1<<10)) ) {

			debug_printk("dv1394: IT context stopped due to bus reset; waking it up\n");

			
			video->dropped_frames++;

			

			
			reg_write(video->ohci, video->ohci_IsoXmitContextControlClear, (1 << 15));
			flush_pci_write(video->ohci);

			
			reg_write(video->ohci, video->ohci_IsoXmitContextControlSet, (1 << 15));
			flush_pci_write(video->ohci);

			
			reg_write(video->ohci, video->ohci_IsoXmitContextControlSet, (1 << 12));
			flush_pci_write(video->ohci);

			irq_printk("dv1394: AFTER IT restart ctx 0x%08x ptr 0x%08x\n",
				   reg_read(video->ohci, video->ohci_IsoXmitContextControlSet),
				   reg_read(video->ohci, video->ohci_IsoXmitCommandPtr));
		}
	}

	
	if (video->ohci_ir_ctx != -1) {
		u32 ctx;

		ctx = reg_read(video->ohci, video->ohci_IsoRcvContextControlSet);

		
		if ( (ctx & (1<<15)) &&
		    !(ctx & (1<<10)) ) {

			debug_printk("dv1394: IR context stopped due to bus reset; waking it up\n");

			
			video->dropped_frames++;

			
			

			
			reg_write(video->ohci, video->ohci_IsoRcvContextControlClear, (1 << 15));
			flush_pci_write(video->ohci);

			
			reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, (1 << 15));
			flush_pci_write(video->ohci);

			
			reg_write(video->ohci, video->ohci_IsoRcvContextControlSet, (1 << 12));
			flush_pci_write(video->ohci);

			irq_printk("dv1394: AFTER IR restart ctx 0x%08x ptr 0x%08x\n",
				   reg_read(video->ohci, video->ohci_IsoRcvContextControlSet),
				   reg_read(video->ohci, video->ohci_IsoRcvCommandPtr));
		}
	}

out:
	spin_unlock_irqrestore(&video->spinlock, flags);

	
	wake_up_interruptible(&video->waitq);
}

static struct hpsb_highlevel dv1394_highlevel = {
	.name =		"dv1394",
	.add_host =	dv1394_add_host,
	.remove_host =	dv1394_remove_host,
	.host_reset =   dv1394_host_reset,
};

#ifdef CONFIG_COMPAT

#define DV1394_IOC32_INIT       _IOW('#', 0x06, struct dv1394_init32)
#define DV1394_IOC32_GET_STATUS _IOR('#', 0x0c, struct dv1394_status32)

struct dv1394_init32 {
	u32 api_version;
	u32 channel;
	u32 n_frames;
	u32 format;
	u32 cip_n;
	u32 cip_d;
	u32 syt_offset;
};

struct dv1394_status32 {
	struct dv1394_init32 init;
	s32 active_frame;
	u32 first_clear_frame;
	u32 n_clear_frames;
	u32 dropped_frames;
};



static int handle_dv1394_init(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dv1394_init32 dv32;
	struct dv1394_init dv;
	mm_segment_t old_fs;
	int ret;

	if (file->f_op->unlocked_ioctl != dv1394_ioctl)
		return -EFAULT;

	if (copy_from_user(&dv32, (void __user *)arg, sizeof(dv32)))
		return -EFAULT;

	dv.api_version = dv32.api_version;
	dv.channel = dv32.channel;
	dv.n_frames = dv32.n_frames;
	dv.format = dv32.format;
	dv.cip_n = (unsigned long)dv32.cip_n;
	dv.cip_d = (unsigned long)dv32.cip_d;
	dv.syt_offset = dv32.syt_offset;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = dv1394_ioctl(file, DV1394_IOC_INIT, (unsigned long)&dv);
	set_fs(old_fs);

	return ret;
}

static int handle_dv1394_get_status(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dv1394_status32 dv32;
	struct dv1394_status dv;
	mm_segment_t old_fs;
	int ret;

	if (file->f_op->unlocked_ioctl != dv1394_ioctl)
		return -EFAULT;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = dv1394_ioctl(file, DV1394_IOC_GET_STATUS, (unsigned long)&dv);
	set_fs(old_fs);

	if (!ret) {
		dv32.init.api_version = dv.init.api_version;
		dv32.init.channel = dv.init.channel;
		dv32.init.n_frames = dv.init.n_frames;
		dv32.init.format = dv.init.format;
		dv32.init.cip_n = (u32)dv.init.cip_n;
		dv32.init.cip_d = (u32)dv.init.cip_d;
		dv32.init.syt_offset = dv.init.syt_offset;
		dv32.active_frame = dv.active_frame;
		dv32.first_clear_frame = dv.first_clear_frame;
		dv32.n_clear_frames = dv.n_clear_frames;
		dv32.dropped_frames = dv.dropped_frames;

		if (copy_to_user((struct dv1394_status32 __user *)arg, &dv32, sizeof(dv32)))
			ret = -EFAULT;
	}

	return ret;
}



static long dv1394_compat_ioctl(struct file *file, unsigned int cmd,
			       unsigned long arg)
{
	switch (cmd) {
	case DV1394_IOC_SHUTDOWN:
	case DV1394_IOC_SUBMIT_FRAMES:
	case DV1394_IOC_WAIT_FRAMES:
	case DV1394_IOC_RECEIVE_FRAMES:
	case DV1394_IOC_START_RECEIVE:
		return dv1394_ioctl(file, cmd, arg);

	case DV1394_IOC32_INIT:
		return handle_dv1394_init(file, cmd, arg);
	case DV1394_IOC32_GET_STATUS:
		return handle_dv1394_get_status(file, cmd, arg);
	default:
		return -ENOIOCTLCMD;
	}
}

#endif 




MODULE_AUTHOR("Dan Maas <dmaas@dcine.com>, Dan Dennedy <dan@dennedy.org>");
MODULE_DESCRIPTION("driver for DV input/output on OHCI board");
MODULE_SUPPORTED_DEVICE("dv1394");
MODULE_LICENSE("GPL");

static void __exit dv1394_exit_module(void)
{
	hpsb_unregister_protocol(&dv1394_driver);
	hpsb_unregister_highlevel(&dv1394_highlevel);
	cdev_del(&dv1394_cdev);
}

static int __init dv1394_init_module(void)
{
	int ret;

	cdev_init(&dv1394_cdev, &dv1394_fops);
	dv1394_cdev.owner = THIS_MODULE;
	ret = cdev_add(&dv1394_cdev, IEEE1394_DV1394_DEV, 16);
	if (ret) {
		printk(KERN_ERR "dv1394: unable to register character device\n");
		return ret;
	}

	hpsb_register_highlevel(&dv1394_highlevel);

	ret = hpsb_register_protocol(&dv1394_driver);
	if (ret) {
		printk(KERN_ERR "dv1394: failed to register protocol\n");
		hpsb_unregister_highlevel(&dv1394_highlevel);
		cdev_del(&dv1394_cdev);
		return ret;
	}

	return 0;
}

module_init(dv1394_init_module);
module_exit(dv1394_exit_module);
