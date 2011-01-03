
#ifndef _C2_VQ_H_
#define _C2_VQ_H_
#include <linux/sched.h>
#include "c2.h"
#include "c2_wr.h"
#include "c2_provider.h"

struct c2_vq_req {
	u64 reply_msg;		
	wait_queue_head_t wait_object;	
	atomic_t reply_ready;	
	atomic_t refcnt;	
	int event;
	struct iw_cm_id *cm_id;
	struct c2_qp *qp;
};

extern int vq_init(struct c2_dev *c2dev);
extern void vq_term(struct c2_dev *c2dev);

extern struct c2_vq_req *vq_req_alloc(struct c2_dev *c2dev);
extern void vq_req_free(struct c2_dev *c2dev, struct c2_vq_req *req);
extern void vq_req_get(struct c2_dev *c2dev, struct c2_vq_req *req);
extern void vq_req_put(struct c2_dev *c2dev, struct c2_vq_req *req);
extern int vq_send_wr(struct c2_dev *c2dev, union c2wr * wr);

extern void *vq_repbuf_alloc(struct c2_dev *c2dev);
extern void vq_repbuf_free(struct c2_dev *c2dev, void *reply);

extern int vq_wait_for_reply(struct c2_dev *c2dev, struct c2_vq_req *req);
#endif				
