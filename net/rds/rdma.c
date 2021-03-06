
#include <linux/pagemap.h>
#include <linux/rbtree.h>
#include <linux/dma-mapping.h> 

#include "rdma.h"




static unsigned int rds_pages_in_vec(struct rds_iovec *vec)
{
	if ((vec->addr + vec->bytes <= vec->addr) ||
	    (vec->bytes > (u64)UINT_MAX))
		return 0;

	return ((vec->addr + vec->bytes + PAGE_SIZE - 1) >> PAGE_SHIFT) -
		(vec->addr >> PAGE_SHIFT);
}

static struct rds_mr *rds_mr_tree_walk(struct rb_root *root, u64 key,
				       struct rds_mr *insert)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct rds_mr *mr;

	while (*p) {
		parent = *p;
		mr = rb_entry(parent, struct rds_mr, r_rb_node);

		if (key < mr->r_key)
			p = &(*p)->rb_left;
		else if (key > mr->r_key)
			p = &(*p)->rb_right;
		else
			return mr;
	}

	if (insert) {
		rb_link_node(&insert->r_rb_node, parent, p);
		rb_insert_color(&insert->r_rb_node, root);
		atomic_inc(&insert->r_refcount);
	}
	return NULL;
}


static void rds_destroy_mr(struct rds_mr *mr)
{
	struct rds_sock *rs = mr->r_sock;
	void *trans_private = NULL;
	unsigned long flags;

	rdsdebug("RDS: destroy mr key is %x refcnt %u\n",
			mr->r_key, atomic_read(&mr->r_refcount));

	if (test_and_set_bit(RDS_MR_DEAD, &mr->r_state))
		return;

	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	if (!RB_EMPTY_NODE(&mr->r_rb_node))
		rb_erase(&mr->r_rb_node, &rs->rs_rdma_keys);
	trans_private = mr->r_trans_private;
	mr->r_trans_private = NULL;
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);

	if (trans_private)
		mr->r_trans->free_mr(trans_private, mr->r_invalidate);
}

void __rds_put_mr_final(struct rds_mr *mr)
{
	rds_destroy_mr(mr);
	kfree(mr);
}


void rds_rdma_drop_keys(struct rds_sock *rs)
{
	struct rds_mr *mr;
	struct rb_node *node;

	
	while ((node = rb_first(&rs->rs_rdma_keys))) {
		mr = container_of(node, struct rds_mr, r_rb_node);
		if (mr->r_trans == rs->rs_transport)
			mr->r_invalidate = 0;
		rds_mr_put(mr);
	}

	if (rs->rs_transport && rs->rs_transport->flush_mrs)
		rs->rs_transport->flush_mrs();
}


static int rds_pin_pages(unsigned long user_addr, unsigned int nr_pages,
			struct page **pages, int write)
{
	int ret;

	ret = get_user_pages_fast(user_addr, nr_pages, write, pages);

	if (ret >= 0 && ret < nr_pages) {
		while (ret--)
			put_page(pages[ret]);
		ret = -EFAULT;
	}

	return ret;
}

static int __rds_rdma_map(struct rds_sock *rs, struct rds_get_mr_args *args,
				u64 *cookie_ret, struct rds_mr **mr_ret)
{
	struct rds_mr *mr = NULL, *found;
	unsigned int nr_pages;
	struct page **pages = NULL;
	struct scatterlist *sg;
	void *trans_private;
	unsigned long flags;
	rds_rdma_cookie_t cookie;
	unsigned int nents;
	long i;
	int ret;

	if (rs->rs_bound_addr == 0) {
		ret = -ENOTCONN; 
		goto out;
	}

	if (rs->rs_transport->get_mr == NULL) {
		ret = -EOPNOTSUPP;
		goto out;
	}

	nr_pages = rds_pages_in_vec(&args->vec);
	if (nr_pages == 0) {
		ret = -EINVAL;
		goto out;
	}

	rdsdebug("RDS: get_mr addr %llx len %llu nr_pages %u\n",
		args->vec.addr, args->vec.bytes, nr_pages);

	
	pages = kcalloc(nr_pages, sizeof(struct page *), GFP_KERNEL);
	if (pages == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	mr = kzalloc(sizeof(struct rds_mr), GFP_KERNEL);
	if (mr == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	atomic_set(&mr->r_refcount, 1);
	RB_CLEAR_NODE(&mr->r_rb_node);
	mr->r_trans = rs->rs_transport;
	mr->r_sock = rs;

	if (args->flags & RDS_RDMA_USE_ONCE)
		mr->r_use_once = 1;
	if (args->flags & RDS_RDMA_INVALIDATE)
		mr->r_invalidate = 1;
	if (args->flags & RDS_RDMA_READWRITE)
		mr->r_write = 1;

	
	ret = rds_pin_pages(args->vec.addr & PAGE_MASK, nr_pages, pages, 1);
	if (ret < 0)
		goto out;

	nents = ret;
	sg = kcalloc(nents, sizeof(*sg), GFP_KERNEL);
	if (sg == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	WARN_ON(!nents);
	sg_init_table(sg, nents);

	
	for (i = 0 ; i < nents; i++)
		sg_set_page(&sg[i], pages[i], PAGE_SIZE, 0);

	rdsdebug("RDS: trans_private nents is %u\n", nents);

	
	trans_private = rs->rs_transport->get_mr(sg, nents, rs,
						 &mr->r_key);

	if (IS_ERR(trans_private)) {
		for (i = 0 ; i < nents; i++)
			put_page(sg_page(&sg[i]));
		kfree(sg);
		ret = PTR_ERR(trans_private);
		goto out;
	}

	mr->r_trans_private = trans_private;

	rdsdebug("RDS: get_mr put_user key is %x cookie_addr %p\n",
	       mr->r_key, (void *)(unsigned long) args->cookie_addr);

	
	cookie = rds_rdma_make_cookie(mr->r_key, args->vec.addr & ~PAGE_MASK);
	if (cookie_ret)
		*cookie_ret = cookie;

	if (args->cookie_addr && put_user(cookie, (u64 __user *)(unsigned long) args->cookie_addr)) {
		ret = -EFAULT;
		goto out;
	}

	
	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	found = rds_mr_tree_walk(&rs->rs_rdma_keys, mr->r_key, mr);
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);

	BUG_ON(found && found != mr);

	rdsdebug("RDS: get_mr key is %x\n", mr->r_key);
	if (mr_ret) {
		atomic_inc(&mr->r_refcount);
		*mr_ret = mr;
	}

	ret = 0;
out:
	kfree(pages);
	if (mr)
		rds_mr_put(mr);
	return ret;
}

int rds_get_mr(struct rds_sock *rs, char __user *optval, int optlen)
{
	struct rds_get_mr_args args;

	if (optlen != sizeof(struct rds_get_mr_args))
		return -EINVAL;

	if (copy_from_user(&args, (struct rds_get_mr_args __user *)optval,
			   sizeof(struct rds_get_mr_args)))
		return -EFAULT;

	return __rds_rdma_map(rs, &args, NULL, NULL);
}


int rds_free_mr(struct rds_sock *rs, char __user *optval, int optlen)
{
	struct rds_free_mr_args args;
	struct rds_mr *mr;
	unsigned long flags;

	if (optlen != sizeof(struct rds_free_mr_args))
		return -EINVAL;

	if (copy_from_user(&args, (struct rds_free_mr_args __user *)optval,
			   sizeof(struct rds_free_mr_args)))
		return -EFAULT;

	
	if (args.cookie == 0) {
		if (!rs->rs_transport || !rs->rs_transport->flush_mrs)
			return -EINVAL;
		rs->rs_transport->flush_mrs();
		return 0;
	}

	
	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	mr = rds_mr_tree_walk(&rs->rs_rdma_keys, rds_rdma_cookie_key(args.cookie), NULL);
	if (mr) {
		rb_erase(&mr->r_rb_node, &rs->rs_rdma_keys);
		RB_CLEAR_NODE(&mr->r_rb_node);
		if (args.flags & RDS_RDMA_INVALIDATE)
			mr->r_invalidate = 1;
	}
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);

	if (!mr)
		return -EINVAL;

	
	rds_destroy_mr(mr);
	rds_mr_put(mr);
	return 0;
}


void rds_rdma_unuse(struct rds_sock *rs, u32 r_key, int force)
{
	struct rds_mr *mr;
	unsigned long flags;
	int zot_me = 0;

	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	mr = rds_mr_tree_walk(&rs->rs_rdma_keys, r_key, NULL);
	if (mr && (mr->r_use_once || force)) {
		rb_erase(&mr->r_rb_node, &rs->rs_rdma_keys);
		RB_CLEAR_NODE(&mr->r_rb_node);
		zot_me = 1;
	} else if (mr)
		atomic_inc(&mr->r_refcount);
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);

	
	if (mr != NULL) {
		if (mr->r_trans->sync_mr)
			mr->r_trans->sync_mr(mr->r_trans_private, DMA_FROM_DEVICE);

		
		if (zot_me)
			rds_destroy_mr(mr);
		rds_mr_put(mr);
	}
}

void rds_rdma_free_op(struct rds_rdma_op *ro)
{
	unsigned int i;

	for (i = 0; i < ro->r_nents; i++) {
		struct page *page = sg_page(&ro->r_sg[i]);

		
		if (!ro->r_write)
			set_page_dirty(page);
		put_page(page);
	}

	kfree(ro->r_notifier);
	kfree(ro);
}


static struct rds_rdma_op *rds_rdma_prepare(struct rds_sock *rs,
					    struct rds_rdma_args *args)
{
	struct rds_iovec vec;
	struct rds_rdma_op *op = NULL;
	unsigned int nr_pages;
	unsigned int max_pages;
	unsigned int nr_bytes;
	struct page **pages = NULL;
	struct rds_iovec __user *local_vec;
	struct scatterlist *sg;
	unsigned int nr;
	unsigned int i, j;
	int ret;


	if (rs->rs_bound_addr == 0) {
		ret = -ENOTCONN; 
		goto out;
	}

	if (args->nr_local > (u64)UINT_MAX) {
		ret = -EMSGSIZE;
		goto out;
	}

	nr_pages = 0;
	max_pages = 0;

	local_vec = (struct rds_iovec __user *)(unsigned long) args->local_vec_addr;

	
	for (i = 0; i < args->nr_local; i++) {
		if (copy_from_user(&vec, &local_vec[i],
				   sizeof(struct rds_iovec))) {
			ret = -EFAULT;
			goto out;
		}

		nr = rds_pages_in_vec(&vec);
		if (nr == 0) {
			ret = -EINVAL;
			goto out;
		}

		max_pages = max(nr, max_pages);
		nr_pages += nr;
	}

	pages = kcalloc(max_pages, sizeof(struct page *), GFP_KERNEL);
	if (pages == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	op = kzalloc(offsetof(struct rds_rdma_op, r_sg[nr_pages]), GFP_KERNEL);
	if (op == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	op->r_write = !!(args->flags & RDS_RDMA_READWRITE);
	op->r_fence = !!(args->flags & RDS_RDMA_FENCE);
	op->r_notify = !!(args->flags & RDS_RDMA_NOTIFY_ME);
	op->r_recverr = rs->rs_recverr;
	WARN_ON(!nr_pages);
	sg_init_table(op->r_sg, nr_pages);

	if (op->r_notify || op->r_recverr) {
		
		op->r_notifier = kmalloc(sizeof(struct rds_notifier), GFP_KERNEL);
		if (!op->r_notifier) {
			ret = -ENOMEM;
			goto out;
		}
		op->r_notifier->n_user_token = args->user_token;
		op->r_notifier->n_status = RDS_RDMA_SUCCESS;
	}

	
	op->r_key = rds_rdma_cookie_key(args->cookie);
	op->r_remote_addr = args->remote_vec.addr + rds_rdma_cookie_offset(args->cookie);

	nr_bytes = 0;

	rdsdebug("RDS: rdma prepare nr_local %llu rva %llx rkey %x\n",
	       (unsigned long long)args->nr_local,
	       (unsigned long long)args->remote_vec.addr,
	       op->r_key);

	for (i = 0; i < args->nr_local; i++) {
		if (copy_from_user(&vec, &local_vec[i],
				   sizeof(struct rds_iovec))) {
			ret = -EFAULT;
			goto out;
		}

		nr = rds_pages_in_vec(&vec);
		if (nr == 0) {
			ret = -EINVAL;
			goto out;
		}

		rs->rs_user_addr = vec.addr;
		rs->rs_user_bytes = vec.bytes;

		
		if (nr > max_pages || op->r_nents + nr > nr_pages) {
			ret = -EINVAL;
			goto out;
		}
		
		ret = rds_pin_pages(vec.addr & PAGE_MASK, nr, pages, !op->r_write);
		if (ret < 0)
			goto out;

		rdsdebug("RDS: nr_bytes %u nr %u vec.bytes %llu vec.addr %llx\n",
		       nr_bytes, nr, vec.bytes, vec.addr);

		nr_bytes += vec.bytes;

		for (j = 0; j < nr; j++) {
			unsigned int offset = vec.addr & ~PAGE_MASK;

			sg = &op->r_sg[op->r_nents + j];
			sg_set_page(sg, pages[j],
					min_t(unsigned int, vec.bytes, PAGE_SIZE - offset),
					offset);

			rdsdebug("RDS: sg->offset %x sg->len %x vec.addr %llx vec.bytes %llu\n",
			       sg->offset, sg->length, vec.addr, vec.bytes);

			vec.addr += sg->length;
			vec.bytes -= sg->length;
		}

		op->r_nents += nr;
	}


	if (nr_bytes > args->remote_vec.bytes) {
		rdsdebug("RDS nr_bytes %u remote_bytes %u do not match\n",
				nr_bytes,
				(unsigned int) args->remote_vec.bytes);
		ret = -EINVAL;
		goto out;
	}
	op->r_bytes = nr_bytes;

	ret = 0;
out:
	kfree(pages);
	if (ret) {
		if (op)
			rds_rdma_free_op(op);
		op = ERR_PTR(ret);
	}
	return op;
}


int rds_cmsg_rdma_args(struct rds_sock *rs, struct rds_message *rm,
			  struct cmsghdr *cmsg)
{
	struct rds_rdma_op *op;

	if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct rds_rdma_args))
	 || rm->m_rdma_op != NULL)
		return -EINVAL;

	op = rds_rdma_prepare(rs, CMSG_DATA(cmsg));
	if (IS_ERR(op))
		return PTR_ERR(op);
	rds_stats_inc(s_send_rdma);
	rm->m_rdma_op = op;
	return 0;
}


int rds_cmsg_rdma_dest(struct rds_sock *rs, struct rds_message *rm,
			  struct cmsghdr *cmsg)
{
	unsigned long flags;
	struct rds_mr *mr;
	u32 r_key;
	int err = 0;

	if (cmsg->cmsg_len < CMSG_LEN(sizeof(rds_rdma_cookie_t))
	 || rm->m_rdma_cookie != 0)
		return -EINVAL;

	memcpy(&rm->m_rdma_cookie, CMSG_DATA(cmsg), sizeof(rm->m_rdma_cookie));

	
	r_key = rds_rdma_cookie_key(rm->m_rdma_cookie);

	spin_lock_irqsave(&rs->rs_rdma_lock, flags);
	mr = rds_mr_tree_walk(&rs->rs_rdma_keys, r_key, NULL);
	if (mr == NULL)
		err = -EINVAL;	
	else
		atomic_inc(&mr->r_refcount);
	spin_unlock_irqrestore(&rs->rs_rdma_lock, flags);

	if (mr) {
		mr->r_trans->sync_mr(mr->r_trans_private, DMA_TO_DEVICE);
		rm->m_rdma_mr = mr;
	}
	return err;
}


int rds_cmsg_rdma_map(struct rds_sock *rs, struct rds_message *rm,
			  struct cmsghdr *cmsg)
{
	if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct rds_get_mr_args))
	 || rm->m_rdma_cookie != 0)
		return -EINVAL;

	return __rds_rdma_map(rs, CMSG_DATA(cmsg), &rm->m_rdma_cookie, &rm->m_rdma_mr);
}
