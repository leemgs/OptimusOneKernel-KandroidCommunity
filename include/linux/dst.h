

#ifndef __DST_H
#define __DST_H

#include <linux/types.h>
#include <linux/connector.h>

#define DST_NAMELEN		32
#define DST_NAME		"dst"

enum {
	
	DST_DEL_NODE	= 0,
	
	DST_ADD_REMOTE,
	
	DST_ADD_EXPORT,
	
	DST_CRYPTO,
	
	DST_SECURITY,
	
	DST_START,
	DST_CMD_MAX
};

struct dst_ctl
{
	
	char			name[DST_NAMELEN];
	
	__u32			flags;
	
	__u32			cmd;
	
	__u32			max_pages;
	
	__u32			trans_scan_timeout;
	
	__u32			trans_max_retries;
	
	__u64			size;
};


struct dst_ctl_ack
{
	struct cn_msg		msg;
	int			error;
	int			unused[3];
};


#define SADDR_MAX_DATA	128

struct saddr {
	
	unsigned short		sa_family;
	
	char			sa_data[SADDR_MAX_DATA];
	
	unsigned short		sa_data_len;
};


struct dst_network_ctl
{
	
	unsigned int		type;
	
	unsigned int		proto;
	
	struct saddr		addr;
};

struct dst_crypto_ctl
{
	
	char			cipher_algo[DST_NAMELEN];
	char			hash_algo[DST_NAMELEN];

	
	unsigned int		cipher_keysize, hash_keysize;
	
	unsigned int		crypto_attached_size;
	
	int			thread_num;
};


#define DST_PERM_READ		(1<<0)
#define DST_PERM_WRITE		(1<<1)


struct dst_secure_user
{
	unsigned int		permissions;
	struct saddr		addr;
};


struct dst_export_ctl
{
	char			device[DST_NAMELEN];
	struct dst_network_ctl	ctl;
};

enum {
	DST_CFG	= 1, 		
	DST_IO,			
	DST_IO_RESPONSE,	
	DST_PING,		
	DST_NCMD_MAX,
};

struct dst_cmd
{
	
	__u32			cmd;
	
	__u32			size;
	
	__u32			csize;
	
	__u32			reserved;
	
	__u64			rw;
	
	__u64			flags;
	
	__u64			id;
	
	__u64			sector;
	
	__u8			hash[0];
};


static inline void dst_convert_cmd(struct dst_cmd *c)
{
	c->cmd = __cpu_to_be32(c->cmd);
	c->csize = __cpu_to_be32(c->csize);
	c->size = __cpu_to_be32(c->size);
	c->sector = __cpu_to_be64(c->sector);
	c->id = __cpu_to_be64(c->id);
	c->flags = __cpu_to_be64(c->flags);
	c->rw = __cpu_to_be64(c->rw);
}


typedef __u64 dst_gen_t;

#ifdef __KERNEL__

#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/device.h>
#include <linux/mempool.h>
#include <linux/net.h>
#include <linux/poll.h>
#include <linux/rbtree.h>

#ifdef CONFIG_DST_DEBUG
#define dprintk(f, a...) printk(KERN_NOTICE f, ##a)
#else
static inline void __attribute__ ((format (printf, 1, 2)))
	dprintk(const char *fmt, ...) {}
#endif

struct dst_node;

struct dst_trans
{
	
	struct dst_node		*n;

	
	struct rb_node		trans_entry;

	
	atomic_t		refcnt;

	
	short			enc;
	
	short			retries;
	
	int			error;

	
	long			send_time;

	
	dst_gen_t		gen;

	
	struct bio		*bio;

	
	struct dst_cmd		cmd;
};

struct dst_crypto_engine
{
	
	struct crypto_hash	*hash;
	struct crypto_ablkcipher	*cipher;

	
	int			page_num;
	struct page		**pages;

	
	int			enc;
	
	struct scatterlist	*src, *dst;

	
	long			timeout;
	
	u64			iv;

	
	void			*private;

	
	int			size;
	void			*data;
};

struct dst_state
{
	
	struct mutex		state_lock;

	
	wait_queue_t 		wait;
	wait_queue_head_t 	*whead;
	
	wait_queue_head_t 	thread_wait;

	
	struct dst_node		*node;

	
	struct dst_network_ctl	ctl;

	
	u32			permissions;

	
	void			(* cleanup)(struct dst_state *st);

	
	struct list_head	request_list;
	spinlock_t		request_lock;

	
	atomic_t		refcnt;

	
	int			need_exit;

	
	struct socket		*socket, *read_socket;

	
	void			*data;
	unsigned int		size;

	
	struct dst_cmd		cmd;
};

struct dst_info
{
	
	u64			size;

	
	char			local[DST_NAMELEN];

	
	struct dst_network_ctl	net;

	
	struct device		device;
};

struct dst_node
{
	struct list_head	node_entry;

	
	char			name[DST_NAMELEN];
	
	char			cache_name[DST_NAMELEN];

	
	struct block_device 	*bdev;
	
	struct dst_state	*state;

	
	struct request_queue	*queue;
	struct gendisk		*disk;

	
	int			thread_num;
	
	int			max_pages;

	
	loff_t			size;

	
	struct dst_info		*info;

	
	struct list_head	security_list;
	struct mutex		security_lock;

	
	atomic_t		refcnt;

	
	int 			(*start)(struct dst_node *);

	
	struct dst_crypto_ctl	crypto;
	u8			*hash_key;
	u8			*cipher_key;

	
	struct thread_pool	*pool;

	
	atomic_long_t		gen;

	
	long			trans_scan_timeout;
	int			trans_max_retries;

	
	struct rb_root		trans_root;
	struct mutex		trans_lock;

	
	struct kmem_cache	*trans_cache;
	mempool_t		*trans_pool;

	
	struct delayed_work 	trans_work;

	wait_queue_head_t	wait;
};


struct dst_secure
{
	struct list_head	sec_entry;
	struct dst_secure_user	sec;
};

int dst_process_bio(struct dst_node *n, struct bio *bio);

int dst_node_init_connected(struct dst_node *n, struct dst_network_ctl *r);
int dst_node_init_listened(struct dst_node *n, struct dst_export_ctl *le);

static inline struct dst_state *dst_state_get(struct dst_state *st)
{
	BUG_ON(atomic_read(&st->refcnt) == 0);
	atomic_inc(&st->refcnt);
	return st;
}

void dst_state_put(struct dst_state *st);

struct dst_state *dst_state_alloc(struct dst_node *n);
int dst_state_socket_create(struct dst_state *st);
void dst_state_socket_release(struct dst_state *st);

void dst_state_exit_connected(struct dst_state *st);

int dst_state_schedule_receiver(struct dst_state *st);

void dst_dump_addr(struct socket *sk, struct sockaddr *sa, char *str);

static inline void dst_state_lock(struct dst_state *st)
{
	mutex_lock(&st->state_lock);
}

static inline void dst_state_unlock(struct dst_state *st)
{
	mutex_unlock(&st->state_lock);
}

void dst_poll_exit(struct dst_state *st);
int dst_poll_init(struct dst_state *st);

static inline unsigned int dst_state_poll(struct dst_state *st)
{
	unsigned int revents = POLLHUP | POLLERR;

	dst_state_lock(st);
	if (st->socket)
		revents = st->socket->ops->poll(NULL, st->socket, NULL);
	dst_state_unlock(st);

	return revents;
}

static inline int dst_thread_setup(void *private, void *data)
{
	return 0;
}

void dst_node_put(struct dst_node *n);

static inline struct dst_node *dst_node_get(struct dst_node *n)
{
	atomic_inc(&n->refcnt);
	return n;
}

int dst_data_recv(struct dst_state *st, void *data, unsigned int size);
int dst_recv_cdata(struct dst_state *st, void *cdata);
int dst_data_send_header(struct socket *sock,
		void *data, unsigned int size, int more);

int dst_send_bio(struct dst_state *st, struct dst_cmd *cmd, struct bio *bio);

int dst_process_io(struct dst_state *st);
int dst_export_crypto(struct dst_node *n, struct bio *bio);
int dst_export_send_bio(struct bio *bio);
int dst_start_export(struct dst_node *n);

int __init dst_export_init(void);
void dst_export_exit(void);


struct dst_export_priv
{
	struct list_head		request_entry;
	struct dst_state		*state;
	struct bio			*bio;
	struct dst_cmd			cmd;
};

static inline void dst_trans_get(struct dst_trans *t)
{
	atomic_inc(&t->refcnt);
}

struct dst_trans *dst_trans_search(struct dst_node *node, dst_gen_t gen);
int dst_trans_remove(struct dst_trans *t);
int dst_trans_remove_nolock(struct dst_trans *t);
void dst_trans_put(struct dst_trans *t);


static inline void dst_bio_to_cmd(struct bio *bio, struct dst_cmd *cmd,
		u32 command, u64 id)
{
	cmd->cmd = command;
	cmd->flags = (bio->bi_flags << BIO_POOL_BITS) >> BIO_POOL_BITS;
	cmd->rw = bio->bi_rw;
	cmd->size = bio->bi_size;
	cmd->csize = 0;
	cmd->id = id;
	cmd->sector = bio->bi_sector;
};

int dst_trans_send(struct dst_trans *t);
int dst_trans_crypto(struct dst_trans *t);

int dst_node_crypto_init(struct dst_node *n, struct dst_crypto_ctl *ctl);
void dst_node_crypto_exit(struct dst_node *n);

static inline int dst_need_crypto(struct dst_node *n)
{
	struct dst_crypto_ctl *c = &n->crypto;
	
	return (c->hash_algo[0] | c->cipher_algo[0]);
}

int dst_node_trans_init(struct dst_node *n, unsigned int size);
void dst_node_trans_exit(struct dst_node *n);


struct thread_pool
{
	int			thread_num;
	struct mutex		thread_lock;
	struct list_head	ready_list, active_list;

	wait_queue_head_t	wait;
};

void thread_pool_del_worker(struct thread_pool *p);
void thread_pool_del_worker_id(struct thread_pool *p, unsigned int id);
int thread_pool_add_worker(struct thread_pool *p,
		char *name,
		unsigned int id,
		void *(* init)(void *data),
		void (* cleanup)(void *data),
		void *data);

void thread_pool_destroy(struct thread_pool *p);
struct thread_pool *thread_pool_create(int num, char *name,
		void *(* init)(void *data),
		void (* cleanup)(void *data),
		void *data);

int thread_pool_schedule(struct thread_pool *p,
		int (* setup)(void *stored_private, void *setup_data),
		int (* action)(void *stored_private, void *setup_data),
		void *setup_data, long timeout);
int thread_pool_schedule_private(struct thread_pool *p,
		int (* setup)(void *private, void *data),
		int (* action)(void *private, void *data),
		void *data, long timeout, void *id);

#endif 
#endif 
