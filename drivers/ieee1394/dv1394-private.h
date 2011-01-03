

#ifndef _DV_1394_PRIVATE_H
#define _DV_1394_PRIVATE_H

#include "ieee1394.h"
#include "ohci1394.h"
#include "dma.h"







struct CIP_header { unsigned char b[8]; };

static inline void fill_cip_header(struct CIP_header *cip,
				   unsigned char source_node_id,
				   unsigned long counter,
				   enum pal_or_ntsc format,
				   unsigned long timestamp)
{
	cip->b[0] = source_node_id;
	cip->b[1] = 0x78; 
	cip->b[2] = 0x00;
	cip->b[3] = counter;

	cip->b[4] = 0x80; 

	switch(format) {
	case DV1394_PAL:
		cip->b[5] = 0x80;
		break;
	case DV1394_NTSC:
		cip->b[5] = 0x00;
		break;
	}

	cip->b[6] = timestamp >> 8;
	cip->b[7] = timestamp & 0xFF;
}





struct output_more_immediate { __le32 q[8]; };
struct output_more { __le32 q[4]; };
struct output_last { __le32 q[4]; };
struct input_more { __le32 q[4]; };
struct input_last { __le32 q[4]; };



static inline void fill_output_more_immediate(struct output_more_immediate *omi,
					      unsigned char tag,
					      unsigned char channel,
					      unsigned char sync_tag,
					      unsigned int  payload_size)
{
	omi->q[0] = cpu_to_le32(0x02000000 | 8); 
	omi->q[1] = cpu_to_le32(0);
	omi->q[2] = cpu_to_le32(0);
	omi->q[3] = cpu_to_le32(0);

	
	omi->q[4] = cpu_to_le32(  (0x0 << 16)  
				  | (tag << 14)
				  | (channel << 8)
				  | (TCODE_ISO_DATA << 4)
				  | (sync_tag) );

	
	omi->q[5] = cpu_to_le32((payload_size << 16) | (0x7F << 8) | 0xA0);

	omi->q[6] = cpu_to_le32(0);
	omi->q[7] = cpu_to_le32(0);
}

static inline void fill_output_more(struct output_more *om,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	om->q[0] = cpu_to_le32(data_size);
	om->q[1] = cpu_to_le32(data_phys_addr);
	om->q[2] = cpu_to_le32(0);
	om->q[3] = cpu_to_le32(0);
}

static inline void fill_output_last(struct output_last *ol,
				    int want_timestamp,
				    int want_interrupt,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	u32 temp = 0;
	temp |= 1 << 28; 

	if (want_timestamp) 
		temp |= 1 << 27;

	if (want_interrupt)
		temp |= 3 << 20;

	temp |= 3 << 18; 
	temp |= data_size;

	ol->q[0] = cpu_to_le32(temp);
	ol->q[1] = cpu_to_le32(data_phys_addr);
	ol->q[2] = cpu_to_le32(0);
	ol->q[3] = cpu_to_le32(0);
}



static inline void fill_input_more(struct input_more *im,
				   int want_interrupt,
				   unsigned int data_size,
				   unsigned long data_phys_addr)
{
	u32 temp =  2 << 28; 
	temp |= 8 << 24; 
	if (want_interrupt)
		temp |= 0 << 20; 
	temp |= 0x0 << 16; 
	                       
       	temp |= data_size;

	im->q[0] = cpu_to_le32(temp);
	im->q[1] = cpu_to_le32(data_phys_addr);
	im->q[2] = cpu_to_le32(0); 
	im->q[3] = cpu_to_le32(0); 
}
 
static inline void fill_input_last(struct input_last *il,
				    int want_interrupt,
				    unsigned int data_size,
				    unsigned long data_phys_addr)
{
	u32 temp =  3 << 28; 
	temp |= 8 << 24; 
	if (want_interrupt)
		temp |= 3 << 20; 
	temp |= 0xC << 16; 
	                       
	temp |= data_size;

	il->q[0] = cpu_to_le32(temp);
	il->q[1] = cpu_to_le32(data_phys_addr);
	il->q[2] = cpu_to_le32(1); 
	il->q[3] = cpu_to_le32(data_size); 
}





struct DMA_descriptor_block {

	union {
		struct {
			
			struct output_more_immediate omi;

			union {
				
				struct {
					struct output_last ol;  
				} empty;

				
				struct {
					struct output_more om;  

					union {
				               
						struct {
							struct output_last ol;  
						} nocross;

				               
						struct {
							struct output_more om;  
							struct output_last ol;  
						} cross;
					} u;

				} full;
			} u;
		} out;

		struct {
			struct input_last il;
		} in;

	} u;

	
	u32 __pad__[12];
};




struct video_card; 

struct frame {

	
	struct video_card *video;

	
	unsigned int frame_num;

	
	enum {
		FRAME_CLEAR = 0,
		FRAME_READY
	} state;

	
	int done;


	
	unsigned long data;

	
#define MAX_PACKETS 500


	
	struct CIP_header *header_pool;
	dma_addr_t         header_pool_dma;


	
	struct DMA_descriptor_block *descriptor_pool;
	dma_addr_t                   descriptor_pool_dma;
	unsigned long                descriptor_pool_size;


	
	unsigned int n_packets;


	

	
	
	__le32 *frame_begin_timestamp;

	
	u32 assigned_timestamp;

	
	struct CIP_header *cip_syt1;

	
	struct CIP_header *cip_syt2;

	
	__le32 *mid_frame_timestamp;
	__le32 *frame_end_timestamp;

	
	__le32 *frame_end_branch;

	
	int first_n_descriptors;
};


struct packet {
	__le16	timestamp;
	u16	invalid;
	u16	iso_header;
	__le16	data_length;
	u32	cip_h1;
	u32	cip_h2;
	unsigned char data[480];
	unsigned char padding[16]; 
};



static struct frame* frame_new(unsigned int frame_num, struct video_card *video);
static void frame_delete(struct frame *f);


static void frame_reset(struct frame *f);


enum modes {
	MODE_RECEIVE,
	MODE_TRANSMIT
};

struct video_card {

	
	struct ti_ohci *ohci;

	
	int id;

	
	struct list_head list;

	
	int ohci_it_ctx;
	struct ohci1394_iso_tasklet it_tasklet;

	
	u32 ohci_IsoXmitContextControlSet;
	u32 ohci_IsoXmitContextControlClear;
	u32 ohci_IsoXmitCommandPtr;

	
	struct ohci1394_iso_tasklet ir_tasklet;
	int ohci_ir_ctx;

	
	u32 ohci_IsoRcvContextControlSet;
	u32 ohci_IsoRcvContextControlClear;
	u32 ohci_IsoRcvCommandPtr;
	u32 ohci_IsoRcvContextMatch;


	

	

	
	unsigned long open;

	
	spinlock_t spinlock;

	
	int dma_running;

	
	struct mutex mtx;

	
	wait_queue_head_t waitq;

	
	struct fasync_struct *fasync;

	
	unsigned long      dv_buf_size;
	struct dma_region  dv_buf;

	
	size_t write_off;

	struct frame *frames[DV1394_MAX_FRAMES];

	

	int n_frames;

	
	int active_frame;
	int first_run;

	

	
	unsigned int first_clear_frame;

	
	unsigned int n_clear_frames;

	
	unsigned int dropped_frames;



	

	unsigned long cip_accum;
	unsigned long cip_n, cip_d;
	unsigned int syt_offset;
	unsigned int continuity_counter;

	enum pal_or_ntsc pal_or_ntsc;

	
	unsigned int frame_size; 

	
	int channel;


	
	struct dma_region packet_buf;
	unsigned long  packet_buf_size;

	unsigned int current_packet;
	int first_frame; 	
	enum modes mode;
};



static inline int video_card_initialized(struct video_card *v)
{
	return v->n_frames > 0;
}

static int do_dv1394_init(struct video_card *video, struct dv1394_init *init);
static int do_dv1394_init_default(struct video_card *video);
static void do_dv1394_shutdown(struct video_card *video, int free_user_buf);




#define CIP_N_NTSC   68000000
#define CIP_D_NTSC 1068000000

#define CIP_N_PAL  1
#define CIP_D_PAL 16

#endif 

