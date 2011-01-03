

#ifndef _ASM_X86_UV_UV_BAU_H
#define _ASM_X86_UV_UV_BAU_H

#include <linux/bitmap.h>
#define BITSPERBYTE 8



#define UV_ITEMS_PER_DESCRIPTOR		8
#define UV_CPUS_PER_ACT_STATUS		32
#define UV_ACT_STATUS_MASK		0x3
#define UV_ACT_STATUS_SIZE		2
#define UV_ADP_SIZE			32
#define UV_DISTRIBUTION_SIZE		256
#define UV_SW_ACK_NPENDING		8
#define UV_NET_ENDPOINT_INTD		0x38
#define UV_DESC_BASE_PNODE_SHIFT	49
#define UV_PAYLOADQ_PNODE_SHIFT		49
#define UV_PTC_BASENAME			"sgi_uv/ptc_statistics"
#define uv_physnodeaddr(x)		((__pa((unsigned long)(x)) & uv_mmask))


#define DESC_STATUS_IDLE		0
#define DESC_STATUS_ACTIVE		1
#define DESC_STATUS_DESTINATION_TIMEOUT	2
#define DESC_STATUS_SOURCE_TIMEOUT	3


#define SOURCE_TIMEOUT_LIMIT		20
#define DESTINATION_TIMEOUT_LIMIT	20


#define DEST_Q_SIZE			17

#define DEST_NUM_RESOURCES		8
#define MAX_CPUS_PER_NODE		32

#define	FLUSH_RETRY			1
#define	FLUSH_GIVEUP			2
#define	FLUSH_COMPLETE			3


struct bau_target_nodemask {
	unsigned long bits[BITS_TO_LONGS(256)];
};


struct bau_local_cpumask {
	unsigned long bits;
};




struct bau_msg_payload {
	unsigned long address;		
	
	unsigned short sending_cpu;	
	
	unsigned short acknowledge_count;
	
	unsigned int reserved1:32;	
};



struct bau_msg_header {
	unsigned int dest_subnodeid:6;	
	
	unsigned int base_dest_nodeid:15; 
				  
	unsigned int command:8;	
	
				
	unsigned int rsvd_1:3;	
	
				
	unsigned int rsvd_2:9;	
	
				
	unsigned int payload_2a:8;
		
	unsigned int payload_2b:8;
		
				
	unsigned int rsvd_3:1;	
	
				
				
	unsigned int replied_to:1;
	

	unsigned int payload_1a:5;
	
	unsigned int payload_1b:8;
	
	unsigned int payload_1c:8;
	
	unsigned int payload_1d:2;
	

	unsigned int rsvd_4:7;	
	
	unsigned int sw_ack_flag:1;
	
				
	unsigned int rsvd_5:6;	
	
	unsigned int rsvd_6:5;	
	
	unsigned int int_both:1;
	
	unsigned int fairness:3;
	
	unsigned int multilevel:1;	
	
				
	unsigned int chaining:1;
	
	unsigned int rsvd_7:21;	
	
};


struct bau_desc {
	struct bau_target_nodemask distribution;
	
	struct bau_msg_header header;
	struct bau_msg_payload payload;
};



struct bau_payload_queue_entry {
	unsigned long address;		
	

	unsigned short sending_cpu;	
	

	unsigned short acknowledge_count; 
	

	unsigned short replied_to:1;	
	
	unsigned short unused1:7;       
	

	unsigned char unused2[2];	
	

	unsigned char sw_ack_vector;	
	

	unsigned char unused4[3];	
	

	int number_of_cpus;		
	

	unsigned char unused5[8];       
	
};


struct bau_msg_status {
	struct bau_local_cpumask seen_by;	
};


struct bau_sw_ack_status {
	struct bau_payload_queue_entry *msg;	
	int watcher;				
};


struct bau_control {
	struct bau_desc *descriptor_base;
	struct bau_payload_queue_entry *bau_msg_head;
	struct bau_payload_queue_entry *va_queue_first;
	struct bau_payload_queue_entry *va_queue_last;
	struct bau_msg_status *msg_statuses;
	int *watching; 
};


struct ptc_stats {
	unsigned long ptc_i;	
	unsigned long requestor;	
	unsigned long requestee;	
	unsigned long alltlb;	
	unsigned long onetlb;	
	unsigned long s_retry;	
	unsigned long d_retry;	
	unsigned long sflush;	
	unsigned long dflush;	
	unsigned long retriesok; 
	unsigned long nomsg;	
	unsigned long multmsg;	
	unsigned long ntargeted;
};

static inline int bau_node_isset(int node, struct bau_target_nodemask *dstp)
{
	return constant_test_bit(node, &dstp->bits[0]);
}
static inline void bau_node_set(int node, struct bau_target_nodemask *dstp)
{
	__set_bit(node, &dstp->bits[0]);
}
static inline void bau_nodes_clear(struct bau_target_nodemask *dstp, int nbits)
{
	bitmap_zero(&dstp->bits[0], nbits);
}

static inline void bau_cpubits_clear(struct bau_local_cpumask *dstp, int nbits)
{
	bitmap_zero(&dstp->bits, nbits);
}

#define cpubit_isset(cpu, bau_local_cpumask) \
	test_bit((cpu), (bau_local_cpumask).bits)

extern void uv_bau_message_intr1(void);
extern void uv_bau_timeout_intr1(void);

#endif 
