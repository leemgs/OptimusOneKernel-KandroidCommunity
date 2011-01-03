

#ifndef VIDC_INIT_H
#define VIDC_INIT_H

#include "vidc_type.h"

#define VIDC_MAX_NUM_CLIENTS 4
#define MAX_VIDEO_NUM_OF_BUFF 100

enum buffer_dir {
	BUFFER_TYPE_INPUT,
	BUFFER_TYPE_OUTPUT
};

struct buf_addr_table {
	unsigned long user_vaddr;
	unsigned long kernel_vaddr;
	unsigned long phy_addr;
	int pmem_fd;
	struct file *file;
};

struct video_client_ctx {
	void *vcd_handle;
	u32 num_of_input_buffers;
	u32 num_of_output_buffers;
	struct buf_addr_table input_buf_addr_table[MAX_VIDEO_NUM_OF_BUFF];
	struct buf_addr_table output_buf_addr_table[MAX_VIDEO_NUM_OF_BUFF];
	struct list_head msg_queue;
	struct mutex msg_queue_lock;
	wait_queue_head_t msg_wait;
	struct completion event;
	u32 event_status;
	u32 seq_header_set;
	u32 stop_msg;
};

void __iomem *vidc_get_ioaddr(void);
int vidc_load_firmware(void);
void vidc_release_firmware(void);
u32 vidc_lookup_addr_table(struct video_client_ctx *client_ctx,
	enum buffer_dir buffer_type, u32 search_with_user_vaddr,
	unsigned long *user_vaddr, unsigned long *kernel_vaddr,
	unsigned long *phy_addr, int *pmem_fd, struct file **file,
	s32 *buffer_index);
u32 vidc_insert_addr_table(struct video_client_ctx *client_ctx,
	enum buffer_dir buffer_type, unsigned long user_vaddr,
	unsigned long *kernel_vaddr, int pmem_fd,
	unsigned long buffer_addr_offset,
	unsigned int max_num_buffers);
u32 vidc_delete_addr_table(struct video_client_ctx *client_ctx,
	enum buffer_dir buffer_type, unsigned long user_vaddr,
	unsigned long *kernel_vaddr);

u32 vidc_timer_create(void (*pf_timer_handler)(void *),
	void *p_user_data, void **pp_timer_handle);
void  vidc_timer_release(void *p_timer_handle);
void  vidc_timer_start(void *p_timer_handle, u32 n_time_out);
void  vidc_timer_stop(void *p_timer_handle);


#endif
