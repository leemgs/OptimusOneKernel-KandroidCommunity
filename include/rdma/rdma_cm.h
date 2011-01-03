

#if !defined(RDMA_CM_H)
#define RDMA_CM_H

#include <linux/socket.h>
#include <linux/in6.h>
#include <rdma/ib_addr.h>
#include <rdma/ib_sa.h>


enum rdma_cm_event_type {
	RDMA_CM_EVENT_ADDR_RESOLVED,
	RDMA_CM_EVENT_ADDR_ERROR,
	RDMA_CM_EVENT_ROUTE_RESOLVED,
	RDMA_CM_EVENT_ROUTE_ERROR,
	RDMA_CM_EVENT_CONNECT_REQUEST,
	RDMA_CM_EVENT_CONNECT_RESPONSE,
	RDMA_CM_EVENT_CONNECT_ERROR,
	RDMA_CM_EVENT_UNREACHABLE,
	RDMA_CM_EVENT_REJECTED,
	RDMA_CM_EVENT_ESTABLISHED,
	RDMA_CM_EVENT_DISCONNECTED,
	RDMA_CM_EVENT_DEVICE_REMOVAL,
	RDMA_CM_EVENT_MULTICAST_JOIN,
	RDMA_CM_EVENT_MULTICAST_ERROR,
	RDMA_CM_EVENT_ADDR_CHANGE,
	RDMA_CM_EVENT_TIMEWAIT_EXIT
};

enum rdma_port_space {
	RDMA_PS_SDP   = 0x0001,
	RDMA_PS_IPOIB = 0x0002,
	RDMA_PS_TCP   = 0x0106,
	RDMA_PS_UDP   = 0x0111,
	RDMA_PS_SCTP  = 0x0183
};

struct rdma_addr {
	struct sockaddr_storage src_addr;
	struct sockaddr_storage dst_addr;
	struct rdma_dev_addr dev_addr;
};

struct rdma_route {
	struct rdma_addr addr;
	struct ib_sa_path_rec *path_rec;
	int num_paths;
};

struct rdma_conn_param {
	const void *private_data;
	u8 private_data_len;
	u8 responder_resources;
	u8 initiator_depth;
	u8 flow_control;
	u8 retry_count;		
	u8 rnr_retry_count;
	
	u8 srq;
	u32 qp_num;
};

struct rdma_ud_param {
	const void *private_data;
	u8 private_data_len;
	struct ib_ah_attr ah_attr;
	u32 qp_num;
	u32 qkey;
};

struct rdma_cm_event {
	enum rdma_cm_event_type	 event;
	int			 status;
	union {
		struct rdma_conn_param	conn;
		struct rdma_ud_param	ud;
	} param;
};

struct rdma_cm_id;


typedef int (*rdma_cm_event_handler)(struct rdma_cm_id *id,
				     struct rdma_cm_event *event);

struct rdma_cm_id {
	struct ib_device	*device;
	void			*context;
	struct ib_qp		*qp;
	rdma_cm_event_handler	 event_handler;
	struct rdma_route	 route;
	enum rdma_port_space	 ps;
	u8			 port_num;
};


struct rdma_cm_id *rdma_create_id(rdma_cm_event_handler event_handler,
				  void *context, enum rdma_port_space ps);


void rdma_destroy_id(struct rdma_cm_id *id);


int rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr);


int rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		      struct sockaddr *dst_addr, int timeout_ms);


int rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms);


int rdma_create_qp(struct rdma_cm_id *id, struct ib_pd *pd,
		   struct ib_qp_init_attr *qp_init_attr);


void rdma_destroy_qp(struct rdma_cm_id *id);


int rdma_init_qp_attr(struct rdma_cm_id *id, struct ib_qp_attr *qp_attr,
		       int *qp_attr_mask);


int rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);


int rdma_listen(struct rdma_cm_id *id, int backlog);


int rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);


int rdma_notify(struct rdma_cm_id *id, enum ib_event_type event);


int rdma_reject(struct rdma_cm_id *id, const void *private_data,
		u8 private_data_len);


int rdma_disconnect(struct rdma_cm_id *id);


int rdma_join_multicast(struct rdma_cm_id *id, struct sockaddr *addr,
			void *context);


void rdma_leave_multicast(struct rdma_cm_id *id, struct sockaddr *addr);


void rdma_set_service_type(struct rdma_cm_id *id, int tos);

#endif 
