

#ifndef __HIDP_H
#define __HIDP_H

#include <linux/types.h>
#include <net/bluetooth/bluetooth.h>


#define HIDP_HEADER_TRANS_MASK			0xf0
#define HIDP_HEADER_PARAM_MASK			0x0f


#define HIDP_TRANS_HANDSHAKE			0x00
#define HIDP_TRANS_HID_CONTROL			0x10
#define HIDP_TRANS_GET_REPORT			0x40
#define HIDP_TRANS_SET_REPORT			0x50
#define HIDP_TRANS_GET_PROTOCOL			0x60
#define HIDP_TRANS_SET_PROTOCOL			0x70
#define HIDP_TRANS_GET_IDLE			0x80
#define HIDP_TRANS_SET_IDLE			0x90
#define HIDP_TRANS_DATA				0xa0
#define HIDP_TRANS_DATC				0xb0


#define HIDP_HSHK_SUCCESSFUL			0x00
#define HIDP_HSHK_NOT_READY			0x01
#define HIDP_HSHK_ERR_INVALID_REPORT_ID		0x02
#define HIDP_HSHK_ERR_UNSUPPORTED_REQUEST	0x03
#define HIDP_HSHK_ERR_INVALID_PARAMETER		0x04
#define HIDP_HSHK_ERR_UNKNOWN			0x0e
#define HIDP_HSHK_ERR_FATAL			0x0f


#define HIDP_CTRL_NOP				0x00
#define HIDP_CTRL_HARD_RESET			0x01
#define HIDP_CTRL_SOFT_RESET			0x02
#define HIDP_CTRL_SUSPEND			0x03
#define HIDP_CTRL_EXIT_SUSPEND			0x04
#define HIDP_CTRL_VIRTUAL_CABLE_UNPLUG		0x05


#define HIDP_DATA_RTYPE_MASK			0x03
#define HIDP_DATA_RSRVD_MASK			0x0c
#define HIDP_DATA_RTYPE_OTHER			0x00
#define HIDP_DATA_RTYPE_INPUT			0x01
#define HIDP_DATA_RTYPE_OUPUT			0x02
#define HIDP_DATA_RTYPE_FEATURE			0x03


#define HIDP_PROTO_BOOT				0x00
#define HIDP_PROTO_REPORT			0x01


#define HIDPCONNADD	_IOW('H', 200, int)
#define HIDPCONNDEL	_IOW('H', 201, int)
#define HIDPGETCONNLIST	_IOR('H', 210, int)
#define HIDPGETCONNINFO	_IOR('H', 211, int)

#define HIDP_VIRTUAL_CABLE_UNPLUG	0
#define HIDP_BOOT_PROTOCOL_MODE		1
#define HIDP_BLUETOOTH_VENDOR_ID	9

struct hidp_connadd_req {
	int   ctrl_sock;	
	int   intr_sock;	
	__u16 parser;
	__u16 rd_size;
	__u8 __user *rd_data;
	__u8  country;
	__u8  subclass;
	__u16 vendor;
	__u16 product;
	__u16 version;
	__u32 flags;
	__u32 idle_to;
	char  name[128];
};

struct hidp_conndel_req {
	bdaddr_t bdaddr;
	__u32    flags;
};

struct hidp_conninfo {
	bdaddr_t bdaddr;
	__u32    flags;
	__u16    state;
	__u16    vendor;
	__u16    product;
	__u16    version;
	char     name[128];
};

struct hidp_connlist_req {
	__u32  cnum;
	struct hidp_conninfo __user *ci;
};

int hidp_add_connection(struct hidp_connadd_req *req, struct socket *ctrl_sock, struct socket *intr_sock);
int hidp_del_connection(struct hidp_conndel_req *req);
int hidp_get_connlist(struct hidp_connlist_req *req);
int hidp_get_conninfo(struct hidp_conninfo *ci);


struct hidp_session {
	struct list_head list;

	struct hci_conn *conn;

	struct socket *ctrl_sock;
	struct socket *intr_sock;

	bdaddr_t bdaddr;

	unsigned long state;
	unsigned long flags;
	unsigned long idle_to;

	uint ctrl_mtu;
	uint intr_mtu;

	atomic_t terminate;

	unsigned char keys[8];
	unsigned char leds;

	struct input_dev *input;

	struct hid_device *hid;

	struct timer_list timer;

	struct sk_buff_head ctrl_transmit;
	struct sk_buff_head intr_transmit;

	struct hidp_connadd_req *req;
};

static inline void hidp_schedule(struct hidp_session *session)
{
	struct sock *ctrl_sk = session->ctrl_sock->sk;
	struct sock *intr_sk = session->intr_sock->sk;

	wake_up_interruptible(ctrl_sk->sk_sleep);
	wake_up_interruptible(intr_sk->sk_sleep);
}


extern int __init hidp_init_sockets(void);
extern void __exit hidp_cleanup_sockets(void);

#endif 
