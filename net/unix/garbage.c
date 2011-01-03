

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/un.h>
#include <linux/net.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/file.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#include <net/sock.h>
#include <net/af_unix.h>
#include <net/scm.h>
#include <net/tcp_states.h>



static LIST_HEAD(gc_inflight_list);
static LIST_HEAD(gc_candidates);
static DEFINE_SPINLOCK(unix_gc_lock);
static DECLARE_WAIT_QUEUE_HEAD(unix_gc_wait);

unsigned int unix_tot_inflight;


static struct sock *unix_get_socket(struct file *filp)
{
	struct sock *u_sock = NULL;
	struct inode *inode = filp->f_path.dentry->d_inode;

	
	if (S_ISSOCK(inode->i_mode)) {
		struct socket *sock = SOCKET_I(inode);
		struct sock *s = sock->sk;

		
		if (s && sock->ops && sock->ops->family == PF_UNIX)
			u_sock = s;
	}
	return u_sock;
}



void unix_inflight(struct file *fp)
{
	struct sock *s = unix_get_socket(fp);
	if (s) {
		struct unix_sock *u = unix_sk(s);
		spin_lock(&unix_gc_lock);
		if (atomic_long_inc_return(&u->inflight) == 1) {
			BUG_ON(!list_empty(&u->link));
			list_add_tail(&u->link, &gc_inflight_list);
		} else {
			BUG_ON(list_empty(&u->link));
		}
		unix_tot_inflight++;
		spin_unlock(&unix_gc_lock);
	}
}

void unix_notinflight(struct file *fp)
{
	struct sock *s = unix_get_socket(fp);
	if (s) {
		struct unix_sock *u = unix_sk(s);
		spin_lock(&unix_gc_lock);
		BUG_ON(list_empty(&u->link));
		if (atomic_long_dec_and_test(&u->inflight))
			list_del_init(&u->link);
		unix_tot_inflight--;
		spin_unlock(&unix_gc_lock);
	}
}

static inline struct sk_buff *sock_queue_head(struct sock *sk)
{
	return (struct sk_buff *)&sk->sk_receive_queue;
}

#define receive_queue_for_each_skb(sk, next, skb) \
	for (skb = sock_queue_head(sk)->next, next = skb->next; \
	     skb != sock_queue_head(sk); skb = next, next = skb->next)

static void scan_inflight(struct sock *x, void (*func)(struct unix_sock *),
			  struct sk_buff_head *hitlist)
{
	struct sk_buff *skb;
	struct sk_buff *next;

	spin_lock(&x->sk_receive_queue.lock);
	receive_queue_for_each_skb(x, next, skb) {
		
		if (UNIXCB(skb).fp) {
			bool hit = false;
			
			int nfd = UNIXCB(skb).fp->count;
			struct file **fp = UNIXCB(skb).fp->fp;
			while (nfd--) {
				
				struct sock *sk = unix_get_socket(*fp++);
				if (sk) {
					struct unix_sock *u = unix_sk(sk);

					
					if (u->gc_candidate) {
						hit = true;
						func(u);
					}
				}
			}
			if (hit && hitlist != NULL) {
				__skb_unlink(skb, &x->sk_receive_queue);
				__skb_queue_tail(hitlist, skb);
			}
		}
	}
	spin_unlock(&x->sk_receive_queue.lock);
}

static void scan_children(struct sock *x, void (*func)(struct unix_sock *),
			  struct sk_buff_head *hitlist)
{
	if (x->sk_state != TCP_LISTEN)
		scan_inflight(x, func, hitlist);
	else {
		struct sk_buff *skb;
		struct sk_buff *next;
		struct unix_sock *u;
		LIST_HEAD(embryos);

		
		spin_lock(&x->sk_receive_queue.lock);
		receive_queue_for_each_skb(x, next, skb) {
			u = unix_sk(skb->sk);

			
			BUG_ON(!list_empty(&u->link));
			list_add_tail(&u->link, &embryos);
		}
		spin_unlock(&x->sk_receive_queue.lock);

		while (!list_empty(&embryos)) {
			u = list_entry(embryos.next, struct unix_sock, link);
			scan_inflight(&u->sk, func, hitlist);
			list_del_init(&u->link);
		}
	}
}

static void dec_inflight(struct unix_sock *usk)
{
	atomic_long_dec(&usk->inflight);
}

static void inc_inflight(struct unix_sock *usk)
{
	atomic_long_inc(&usk->inflight);
}

static void inc_inflight_move_tail(struct unix_sock *u)
{
	atomic_long_inc(&u->inflight);
	
	if (u->gc_maybe_cycle)
		list_move_tail(&u->link, &gc_candidates);
}

static bool gc_in_progress = false;

void wait_for_unix_gc(void)
{
	wait_event(unix_gc_wait, gc_in_progress == false);
}


void unix_gc(void)
{
	struct unix_sock *u;
	struct unix_sock *next;
	struct sk_buff_head hitlist;
	struct list_head cursor;
	LIST_HEAD(not_cycle_list);

	spin_lock(&unix_gc_lock);

	
	if (gc_in_progress)
		goto out;

	gc_in_progress = true;
	
	list_for_each_entry_safe(u, next, &gc_inflight_list, link) {
		long total_refs;
		long inflight_refs;

		total_refs = file_count(u->sk.sk_socket->file);
		inflight_refs = atomic_long_read(&u->inflight);

		BUG_ON(inflight_refs < 1);
		BUG_ON(total_refs < inflight_refs);
		if (total_refs == inflight_refs) {
			list_move_tail(&u->link, &gc_candidates);
			u->gc_candidate = 1;
			u->gc_maybe_cycle = 1;
		}
	}

	
	list_for_each_entry(u, &gc_candidates, link)
		scan_children(&u->sk, dec_inflight, NULL);

	
	list_add(&cursor, &gc_candidates);
	while (cursor.next != &gc_candidates) {
		u = list_entry(cursor.next, struct unix_sock, link);

		
		list_move(&cursor, &u->link);

		if (atomic_long_read(&u->inflight) > 0) {
			list_move_tail(&u->link, &not_cycle_list);
			u->gc_maybe_cycle = 0;
			scan_children(&u->sk, inc_inflight_move_tail, NULL);
		}
	}
	list_del(&cursor);

	
	while (!list_empty(&not_cycle_list)) {
		u = list_entry(not_cycle_list.next, struct unix_sock, link);
		u->gc_candidate = 0;
		list_move_tail(&u->link, &gc_inflight_list);
	}

	
	skb_queue_head_init(&hitlist);
	list_for_each_entry(u, &gc_candidates, link)
	scan_children(&u->sk, inc_inflight, &hitlist);

	spin_unlock(&unix_gc_lock);

	
	__skb_queue_purge(&hitlist);

	spin_lock(&unix_gc_lock);

	
	BUG_ON(!list_empty(&gc_candidates));
	gc_in_progress = false;
	wake_up(&unix_gc_wait);

 out:
	spin_unlock(&unix_gc_lock);
}
