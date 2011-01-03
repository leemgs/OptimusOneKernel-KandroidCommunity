

#ifndef _ARCH_ARM_MACH_MSM_SMD_RPCROUTER_H
#define _ARCH_ARM_MACH_MSM_SMD_RPCROUTER_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>

#include <mach/msm_smd.h>
#include <mach/msm_rpcrouter.h>



#define RPCROUTER_VERSION			1
#define RPCROUTER_PROCESSORS_MAX		4
#define RPCROUTER_MSGSIZE_MAX			512

#define RPCROUTER_CLIENT_BCAST_ID		0xffffffff
#define RPCROUTER_ROUTER_ADDRESS		0xfffffffe

#define RPCROUTER_PID_LOCAL			1
#define RPCROUTER_PID_REMOTE			0

#define RPCROUTER_CTRL_CMD_DATA			1
#define RPCROUTER_CTRL_CMD_HELLO		2
#define RPCROUTER_CTRL_CMD_BYE			3
#define RPCROUTER_CTRL_CMD_NEW_SERVER		4
#define RPCROUTER_CTRL_CMD_REMOVE_SERVER	5
#define RPCROUTER_CTRL_CMD_REMOVE_CLIENT	6
#define RPCROUTER_CTRL_CMD_RESUME_TX		7
#define RPCROUTER_CTRL_CMD_EXIT			8

#define RPCROUTER_DEFAULT_RX_QUOTA	5

union rr_control_msg {
	uint32_t cmd;
	struct {
		uint32_t cmd;
		uint32_t prog;
		uint32_t vers;
		uint32_t pid;
		uint32_t cid;
	} srv;
	struct {
		uint32_t cmd;
		uint32_t pid;
		uint32_t cid;
	} cli;
};

struct rr_header {
	uint32_t version;
	uint32_t type;
	uint32_t src_pid;
	uint32_t src_cid;
	uint32_t confirm_rx;
	uint32_t size;
	uint32_t dst_pid;
	uint32_t dst_cid;
};



#define RPCROUTER_MAX_REMOTE_SERVERS		100

struct rr_fragment {
	unsigned char data[RPCROUTER_MSGSIZE_MAX];
	uint32_t length;
	struct rr_fragment *next;
};

struct rr_packet {
	struct list_head list;
	struct rr_fragment *first;
	struct rr_fragment *last;
	struct rr_header hdr;
	uint32_t mid;
	uint32_t length;
};

#define PACMARK_LAST(n) ((n) & 0x80000000)
#define PACMARK_MID(n)  (((n) >> 16) & 0xFF)
#define PACMARK_LEN(n)  ((n) & 0xFFFF)

static inline uint32_t PACMARK(uint32_t len, uint32_t mid, uint32_t first,
			       uint32_t last)
{
	return (len & 0xFFFF) |
	  ((mid & 0xFF) << 16) |
	  ((!!first) << 30) |
	  ((!!last) << 31);
}

struct rr_server {
	struct list_head list;

	uint32_t pid;
	uint32_t cid;
	uint32_t prog;
	uint32_t vers;

	dev_t device_number;
	struct cdev cdev;
	struct device *device;
	struct rpcsvr_platform_device p_device;
	char pdev_name[32];
};

struct rr_remote_endpoint {
	uint32_t pid;
	uint32_t cid;

	int tx_quota_cntr;
	spinlock_t quota_lock;
	wait_queue_head_t quota_wait;

	struct list_head list;
};

struct msm_rpc_endpoint {
	struct list_head list;

	
	struct list_head incomplete;

	
	struct list_head read_q;
	spinlock_t read_q_lock;
	struct wake_lock read_q_wake_lock;
	wait_queue_head_t wait_q;
	unsigned flags;

	
	uint32_t pid;
	uint32_t cid;

	
	uint32_t dst_pid;
	uint32_t dst_cid;
	uint32_t dst_prog; 
	uint32_t dst_vers; 

	
	uint32_t reply_pid;
	uint32_t reply_cid;
	uint32_t reply_xid; 
	uint32_t next_pm;   

	
	dev_t dev;
};



int __msm_rpc_read(struct msm_rpc_endpoint *ept,
		   struct rr_fragment **frag,
		   unsigned len, long timeout);

struct msm_rpc_endpoint *msm_rpcrouter_create_local_endpoint(dev_t dev);
int msm_rpcrouter_destroy_local_endpoint(struct msm_rpc_endpoint *ept);

int msm_rpcrouter_create_server_cdev(struct rr_server *server);
int msm_rpcrouter_create_server_pdev(struct rr_server *server);

int msm_rpcrouter_init_devices(void);
void msm_rpcrouter_exit_devices(void);

extern dev_t msm_rpcrouter_devno;
extern struct class *msm_rpcrouter_class;
#endif
