

#include <linux/module.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/tcp_states.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/termios.h>	
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/compat.h>

#include <net/x25.h>
#include <net/compat.h>

int sysctl_x25_restart_request_timeout = X25_DEFAULT_T20;
int sysctl_x25_call_request_timeout    = X25_DEFAULT_T21;
int sysctl_x25_reset_request_timeout   = X25_DEFAULT_T22;
int sysctl_x25_clear_request_timeout   = X25_DEFAULT_T23;
int sysctl_x25_ack_holdback_timeout    = X25_DEFAULT_T2;
int sysctl_x25_forward                 = 0;

HLIST_HEAD(x25_list);
DEFINE_RWLOCK(x25_list_lock);

static const struct proto_ops x25_proto_ops;

static struct x25_address null_x25_address = {"               "};

#ifdef CONFIG_COMPAT
struct compat_x25_subscrip_struct {
	char device[200-sizeof(compat_ulong_t)];
	compat_ulong_t global_facil_mask;
	compat_uint_t extended;
};
#endif

int x25_addr_ntoa(unsigned char *p, struct x25_address *called_addr,
		  struct x25_address *calling_addr)
{
	unsigned int called_len, calling_len;
	char *called, *calling;
	unsigned int i;

	called_len  = (*p >> 0) & 0x0F;
	calling_len = (*p >> 4) & 0x0F;

	called  = called_addr->x25_addr;
	calling = calling_addr->x25_addr;
	p++;

	for (i = 0; i < (called_len + calling_len); i++) {
		if (i < called_len) {
			if (i % 2 != 0) {
				*called++ = ((*p >> 0) & 0x0F) + '0';
				p++;
			} else {
				*called++ = ((*p >> 4) & 0x0F) + '0';
			}
		} else {
			if (i % 2 != 0) {
				*calling++ = ((*p >> 0) & 0x0F) + '0';
				p++;
			} else {
				*calling++ = ((*p >> 4) & 0x0F) + '0';
			}
		}
	}

	*called = *calling = '\0';

	return 1 + (called_len + calling_len + 1) / 2;
}

int x25_addr_aton(unsigned char *p, struct x25_address *called_addr,
		  struct x25_address *calling_addr)
{
	unsigned int called_len, calling_len;
	char *called, *calling;
	int i;

	called  = called_addr->x25_addr;
	calling = calling_addr->x25_addr;

	called_len  = strlen(called);
	calling_len = strlen(calling);

	*p++ = (calling_len << 4) | (called_len << 0);

	for (i = 0; i < (called_len + calling_len); i++) {
		if (i < called_len) {
			if (i % 2 != 0) {
				*p |= (*called++ - '0') << 0;
				p++;
			} else {
				*p = 0x00;
				*p |= (*called++ - '0') << 4;
			}
		} else {
			if (i % 2 != 0) {
				*p |= (*calling++ - '0') << 0;
				p++;
			} else {
				*p = 0x00;
				*p |= (*calling++ - '0') << 4;
			}
		}
	}

	return 1 + (called_len + calling_len + 1) / 2;
}


static void x25_remove_socket(struct sock *sk)
{
	write_lock_bh(&x25_list_lock);
	sk_del_node_init(sk);
	write_unlock_bh(&x25_list_lock);
}


static void x25_kill_by_device(struct net_device *dev)
{
	struct sock *s;
	struct hlist_node *node;

	write_lock_bh(&x25_list_lock);

	sk_for_each(s, node, &x25_list)
		if (x25_sk(s)->neighbour && x25_sk(s)->neighbour->dev == dev)
			x25_disconnect(s, ENETUNREACH, 0, 0);

	write_unlock_bh(&x25_list_lock);
}


static int x25_device_event(struct notifier_block *this, unsigned long event,
			    void *ptr)
{
	struct net_device *dev = ptr;
	struct x25_neigh *nb;

	if (!net_eq(dev_net(dev), &init_net))
		return NOTIFY_DONE;

	if (dev->type == ARPHRD_X25
#if defined(CONFIG_LLC) || defined(CONFIG_LLC_MODULE)
	 || dev->type == ARPHRD_ETHER
#endif
	 ) {
		switch (event) {
			case NETDEV_UP:
				x25_link_device_up(dev);
				break;
			case NETDEV_GOING_DOWN:
				nb = x25_get_neigh(dev);
				if (nb) {
					x25_terminate_link(nb);
					x25_neigh_put(nb);
				}
				break;
			case NETDEV_DOWN:
				x25_kill_by_device(dev);
				x25_route_device_down(dev);
				x25_link_device_down(dev);
				break;
		}
	}

	return NOTIFY_DONE;
}


static void x25_insert_socket(struct sock *sk)
{
	write_lock_bh(&x25_list_lock);
	sk_add_node(sk, &x25_list);
	write_unlock_bh(&x25_list_lock);
}


static struct sock *x25_find_listener(struct x25_address *addr,
					struct sk_buff *skb)
{
	struct sock *s;
	struct sock *next_best;
	struct hlist_node *node;

	read_lock_bh(&x25_list_lock);
	next_best = NULL;

	sk_for_each(s, node, &x25_list)
		if ((!strcmp(addr->x25_addr,
			x25_sk(s)->source_addr.x25_addr) ||
				!strcmp(addr->x25_addr,
					null_x25_address.x25_addr)) &&
					s->sk_state == TCP_LISTEN) {
			
			if(skb->len > 0 && x25_sk(s)->cudmatchlength > 0) {
				if((memcmp(x25_sk(s)->calluserdata.cuddata,
					skb->data,
					x25_sk(s)->cudmatchlength)) == 0) {
					sock_hold(s);
					goto found;
				 }
			} else
				next_best = s;
		}
	if (next_best) {
		s = next_best;
		sock_hold(s);
		goto found;
	}
	s = NULL;
found:
	read_unlock_bh(&x25_list_lock);
	return s;
}


static struct sock *__x25_find_socket(unsigned int lci, struct x25_neigh *nb)
{
	struct sock *s;
	struct hlist_node *node;

	sk_for_each(s, node, &x25_list)
		if (x25_sk(s)->lci == lci && x25_sk(s)->neighbour == nb) {
			sock_hold(s);
			goto found;
		}
	s = NULL;
found:
	return s;
}

struct sock *x25_find_socket(unsigned int lci, struct x25_neigh *nb)
{
	struct sock *s;

	read_lock_bh(&x25_list_lock);
	s = __x25_find_socket(lci, nb);
	read_unlock_bh(&x25_list_lock);
	return s;
}


static unsigned int x25_new_lci(struct x25_neigh *nb)
{
	unsigned int lci = 1;
	struct sock *sk;

	read_lock_bh(&x25_list_lock);

	while ((sk = __x25_find_socket(lci, nb)) != NULL) {
		sock_put(sk);
		if (++lci == 4096) {
			lci = 0;
			break;
		}
	}

	read_unlock_bh(&x25_list_lock);
	return lci;
}


static void __x25_destroy_socket(struct sock *);


static void x25_destroy_timer(unsigned long data)
{
	x25_destroy_socket_from_timer((struct sock *)data);
}


static void __x25_destroy_socket(struct sock *sk)
{
	struct sk_buff *skb;

	x25_stop_heartbeat(sk);
	x25_stop_timer(sk);

	x25_remove_socket(sk);
	x25_clear_queues(sk);		

	while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
		if (skb->sk != sk) {		
			
			sock_set_flag(skb->sk, SOCK_DEAD);
			x25_start_heartbeat(skb->sk);
			x25_sk(skb->sk)->state = X25_STATE_0;
		}

		kfree_skb(skb);
	}

	if (sk_has_allocations(sk)) {
		
		sk->sk_timer.expires  = jiffies + 10 * HZ;
		sk->sk_timer.function = x25_destroy_timer;
		sk->sk_timer.data = (unsigned long)sk;
		add_timer(&sk->sk_timer);
	} else {
		
		__sock_put(sk);
	}
}

void x25_destroy_socket_from_timer(struct sock *sk)
{
	sock_hold(sk);
	bh_lock_sock(sk);
	__x25_destroy_socket(sk);
	bh_unlock_sock(sk);
	sock_put(sk);
}

static void x25_destroy_socket(struct sock *sk)
{
	sock_hold(sk);
	lock_sock(sk);
	__x25_destroy_socket(sk);
	release_sock(sk);
	sock_put(sk);
}



static int x25_setsockopt(struct socket *sock, int level, int optname,
			  char __user *optval, unsigned int optlen)
{
	int opt;
	struct sock *sk = sock->sk;
	int rc = -ENOPROTOOPT;

	if (level != SOL_X25 || optname != X25_QBITINCL)
		goto out;

	rc = -EINVAL;
	if (optlen < sizeof(int))
		goto out;

	rc = -EFAULT;
	if (get_user(opt, (int __user *)optval))
		goto out;

	x25_sk(sk)->qbitincl = !!opt;
	rc = 0;
out:
	return rc;
}

static int x25_getsockopt(struct socket *sock, int level, int optname,
			  char __user *optval, int __user *optlen)
{
	struct sock *sk = sock->sk;
	int val, len, rc = -ENOPROTOOPT;

	if (level != SOL_X25 || optname != X25_QBITINCL)
		goto out;

	rc = -EFAULT;
	if (get_user(len, optlen))
		goto out;

	len = min_t(unsigned int, len, sizeof(int));

	rc = -EINVAL;
	if (len < 0)
		goto out;

	rc = -EFAULT;
	if (put_user(len, optlen))
		goto out;

	val = x25_sk(sk)->qbitincl;
	rc = copy_to_user(optval, &val, len) ? -EFAULT : 0;
out:
	return rc;
}

static int x25_listen(struct socket *sock, int backlog)
{
	struct sock *sk = sock->sk;
	int rc = -EOPNOTSUPP;

	if (sk->sk_state != TCP_LISTEN) {
		memset(&x25_sk(sk)->dest_addr, 0, X25_ADDR_LEN);
		sk->sk_max_ack_backlog = backlog;
		sk->sk_state           = TCP_LISTEN;
		rc = 0;
	}

	return rc;
}

static struct proto x25_proto = {
	.name	  = "X25",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct x25_sock),
};

static struct sock *x25_alloc_socket(struct net *net)
{
	struct x25_sock *x25;
	struct sock *sk = sk_alloc(net, AF_X25, GFP_ATOMIC, &x25_proto);

	if (!sk)
		goto out;

	sock_init_data(NULL, sk);

	x25 = x25_sk(sk);
	skb_queue_head_init(&x25->ack_queue);
	skb_queue_head_init(&x25->fragment_queue);
	skb_queue_head_init(&x25->interrupt_in_queue);
	skb_queue_head_init(&x25->interrupt_out_queue);
out:
	return sk;
}

static int x25_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
	struct x25_sock *x25;
	int rc = -ESOCKTNOSUPPORT;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	if (sock->type != SOCK_SEQPACKET || protocol)
		goto out;

	rc = -ENOMEM;
	if ((sk = x25_alloc_socket(net)) == NULL)
		goto out;

	x25 = x25_sk(sk);

	sock_init_data(sock, sk);

	x25_init_timers(sk);

	sock->ops    = &x25_proto_ops;
	sk->sk_protocol = protocol;
	sk->sk_backlog_rcv = x25_backlog_rcv;

	x25->t21   = sysctl_x25_call_request_timeout;
	x25->t22   = sysctl_x25_reset_request_timeout;
	x25->t23   = sysctl_x25_clear_request_timeout;
	x25->t2    = sysctl_x25_ack_holdback_timeout;
	x25->state = X25_STATE_0;
	x25->cudmatchlength = 0;
	x25->accptapprv = X25_DENY_ACCPT_APPRV;		
							

	x25->facilities.winsize_in  = X25_DEFAULT_WINDOW_SIZE;
	x25->facilities.winsize_out = X25_DEFAULT_WINDOW_SIZE;
	x25->facilities.pacsize_in  = X25_DEFAULT_PACKET_SIZE;
	x25->facilities.pacsize_out = X25_DEFAULT_PACKET_SIZE;
	x25->facilities.throughput  = X25_DEFAULT_THROUGHPUT;
	x25->facilities.reverse     = X25_DEFAULT_REVERSE;
	x25->dte_facilities.calling_len = 0;
	x25->dte_facilities.called_len = 0;
	memset(x25->dte_facilities.called_ae, '\0',
			sizeof(x25->dte_facilities.called_ae));
	memset(x25->dte_facilities.calling_ae, '\0',
			sizeof(x25->dte_facilities.calling_ae));

	rc = 0;
out:
	return rc;
}

static struct sock *x25_make_new(struct sock *osk)
{
	struct sock *sk = NULL;
	struct x25_sock *x25, *ox25;

	if (osk->sk_type != SOCK_SEQPACKET)
		goto out;

	if ((sk = x25_alloc_socket(sock_net(osk))) == NULL)
		goto out;

	x25 = x25_sk(sk);

	sk->sk_type        = osk->sk_type;
	sk->sk_priority    = osk->sk_priority;
	sk->sk_protocol    = osk->sk_protocol;
	sk->sk_rcvbuf      = osk->sk_rcvbuf;
	sk->sk_sndbuf      = osk->sk_sndbuf;
	sk->sk_state       = TCP_ESTABLISHED;
	sk->sk_backlog_rcv = osk->sk_backlog_rcv;
	sock_copy_flags(sk, osk);

	ox25 = x25_sk(osk);
	x25->t21        = ox25->t21;
	x25->t22        = ox25->t22;
	x25->t23        = ox25->t23;
	x25->t2         = ox25->t2;
	x25->facilities = ox25->facilities;
	x25->qbitincl   = ox25->qbitincl;
	x25->dte_facilities = ox25->dte_facilities;
	x25->cudmatchlength = ox25->cudmatchlength;
	x25->accptapprv = ox25->accptapprv;

	x25_init_timers(sk);
out:
	return sk;
}

static int x25_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct x25_sock *x25;

	if (!sk)
		goto out;

	x25 = x25_sk(sk);

	switch (x25->state) {

		case X25_STATE_0:
		case X25_STATE_2:
			x25_disconnect(sk, 0, 0, 0);
			x25_destroy_socket(sk);
			goto out;

		case X25_STATE_1:
		case X25_STATE_3:
		case X25_STATE_4:
			x25_clear_queues(sk);
			x25_write_internal(sk, X25_CLEAR_REQUEST);
			x25_start_t23timer(sk);
			x25->state = X25_STATE_2;
			sk->sk_state	= TCP_CLOSE;
			sk->sk_shutdown	|= SEND_SHUTDOWN;
			sk->sk_state_change(sk);
			sock_set_flag(sk, SOCK_DEAD);
			sock_set_flag(sk, SOCK_DESTROY);
			break;
	}

	sock_orphan(sk);
out:
	return 0;
}

static int x25_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_x25 *addr = (struct sockaddr_x25 *)uaddr;

	if (!sock_flag(sk, SOCK_ZAPPED) ||
	    addr_len != sizeof(struct sockaddr_x25) ||
	    addr->sx25_family != AF_X25)
		return -EINVAL;

	x25_sk(sk)->source_addr = addr->sx25_addr;
	x25_insert_socket(sk);
	sock_reset_flag(sk, SOCK_ZAPPED);
	SOCK_DEBUG(sk, "x25_bind: socket is bound\n");

	return 0;
}

static int x25_wait_for_connection_establishment(struct sock *sk)
{
	DECLARE_WAITQUEUE(wait, current);
	int rc;

	add_wait_queue_exclusive(sk->sk_sleep, &wait);
	for (;;) {
		__set_current_state(TASK_INTERRUPTIBLE);
		rc = -ERESTARTSYS;
		if (signal_pending(current))
			break;
		rc = sock_error(sk);
		if (rc) {
			sk->sk_socket->state = SS_UNCONNECTED;
			break;
		}
		rc = 0;
		if (sk->sk_state != TCP_ESTABLISHED) {
			release_sock(sk);
			schedule();
			lock_sock(sk);
		} else
			break;
	}
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(sk->sk_sleep, &wait);
	return rc;
}

static int x25_connect(struct socket *sock, struct sockaddr *uaddr,
		       int addr_len, int flags)
{
	struct sock *sk = sock->sk;
	struct x25_sock *x25 = x25_sk(sk);
	struct sockaddr_x25 *addr = (struct sockaddr_x25 *)uaddr;
	struct x25_route *rt;
	int rc = 0;

	lock_sock(sk);
	if (sk->sk_state == TCP_ESTABLISHED && sock->state == SS_CONNECTING) {
		sock->state = SS_CONNECTED;
		goto out; 
	}

	rc = -ECONNREFUSED;
	if (sk->sk_state == TCP_CLOSE && sock->state == SS_CONNECTING) {
		sock->state = SS_UNCONNECTED;
		goto out;
	}

	rc = -EISCONN;	
	if (sk->sk_state == TCP_ESTABLISHED)
		goto out;

	sk->sk_state   = TCP_CLOSE;
	sock->state = SS_UNCONNECTED;

	rc = -EINVAL;
	if (addr_len != sizeof(struct sockaddr_x25) ||
	    addr->sx25_family != AF_X25)
		goto out;

	rc = -ENETUNREACH;
	rt = x25_get_route(&addr->sx25_addr);
	if (!rt)
		goto out;

	x25->neighbour = x25_get_neigh(rt->dev);
	if (!x25->neighbour)
		goto out_put_route;

	x25_limit_facilities(&x25->facilities, x25->neighbour);

	x25->lci = x25_new_lci(x25->neighbour);
	if (!x25->lci)
		goto out_put_neigh;

	rc = -EINVAL;
	if (sock_flag(sk, SOCK_ZAPPED)) 
		goto out_put_neigh;

	if (!strcmp(x25->source_addr.x25_addr, null_x25_address.x25_addr))
		memset(&x25->source_addr, '\0', X25_ADDR_LEN);

	x25->dest_addr = addr->sx25_addr;

	
	sock->state   = SS_CONNECTING;
	sk->sk_state  = TCP_SYN_SENT;

	x25->state = X25_STATE_1;

	x25_write_internal(sk, X25_CALL_REQUEST);

	x25_start_heartbeat(sk);
	x25_start_t21timer(sk);

	
	rc = -EINPROGRESS;
	if (sk->sk_state != TCP_ESTABLISHED && (flags & O_NONBLOCK))
		goto out_put_neigh;

	rc = x25_wait_for_connection_establishment(sk);
	if (rc)
		goto out_put_neigh;

	sock->state = SS_CONNECTED;
	rc = 0;
out_put_neigh:
	if (rc)
		x25_neigh_put(x25->neighbour);
out_put_route:
	x25_route_put(rt);
out:
	release_sock(sk);
	return rc;
}

static int x25_wait_for_data(struct sock *sk, long timeout)
{
	DECLARE_WAITQUEUE(wait, current);
	int rc = 0;

	add_wait_queue_exclusive(sk->sk_sleep, &wait);
	for (;;) {
		__set_current_state(TASK_INTERRUPTIBLE);
		if (sk->sk_shutdown & RCV_SHUTDOWN)
			break;
		rc = -ERESTARTSYS;
		if (signal_pending(current))
			break;
		rc = -EAGAIN;
		if (!timeout)
			break;
		rc = 0;
		if (skb_queue_empty(&sk->sk_receive_queue)) {
			release_sock(sk);
			timeout = schedule_timeout(timeout);
			lock_sock(sk);
		} else
			break;
	}
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(sk->sk_sleep, &wait);
	return rc;
}

static int x25_accept(struct socket *sock, struct socket *newsock, int flags)
{
	struct sock *sk = sock->sk;
	struct sock *newsk;
	struct sk_buff *skb;
	int rc = -EINVAL;

	if (!sk || sk->sk_state != TCP_LISTEN)
		goto out;

	rc = -EOPNOTSUPP;
	if (sk->sk_type != SOCK_SEQPACKET)
		goto out;

	lock_sock(sk);
	rc = x25_wait_for_data(sk, sk->sk_rcvtimeo);
	if (rc)
		goto out2;
	skb = skb_dequeue(&sk->sk_receive_queue);
	rc = -EINVAL;
	if (!skb->sk)
		goto out2;
	newsk		 = skb->sk;
	sock_graft(newsk, newsock);

	
	skb->sk = NULL;
	kfree_skb(skb);
	sk->sk_ack_backlog--;
	newsock->state = SS_CONNECTED;
	rc = 0;
out2:
	release_sock(sk);
out:
	return rc;
}

static int x25_getname(struct socket *sock, struct sockaddr *uaddr,
		       int *uaddr_len, int peer)
{
	struct sockaddr_x25 *sx25 = (struct sockaddr_x25 *)uaddr;
	struct sock *sk = sock->sk;
	struct x25_sock *x25 = x25_sk(sk);

	if (peer) {
		if (sk->sk_state != TCP_ESTABLISHED)
			return -ENOTCONN;
		sx25->sx25_addr = x25->dest_addr;
	} else
		sx25->sx25_addr = x25->source_addr;

	sx25->sx25_family = AF_X25;
	*uaddr_len = sizeof(*sx25);

	return 0;
}

int x25_rx_call_request(struct sk_buff *skb, struct x25_neigh *nb,
			unsigned int lci)
{
	struct sock *sk;
	struct sock *make;
	struct x25_sock *makex25;
	struct x25_address source_addr, dest_addr;
	struct x25_facilities facilities;
	struct x25_dte_facilities dte_facilities;
	int len, addr_len, rc;

	
	skb_pull(skb, X25_STD_MIN_LEN);

	
	addr_len = x25_addr_ntoa(skb->data, &source_addr, &dest_addr);
	skb_pull(skb, addr_len);

	
	len = skb->data[0] + 1;
	skb_pull(skb,len);

	
	sk = x25_find_listener(&source_addr,skb);
	skb_push(skb,len);

	if (sk != NULL && sk_acceptq_is_full(sk)) {
		goto out_sock_put;
	}

	
	if (sk == NULL) {
		skb_push(skb, addr_len + X25_STD_MIN_LEN);
		if (sysctl_x25_forward &&
				x25_forward_call(&dest_addr, nb, skb, lci) > 0)
		{
			
			kfree_skb(skb);
			rc = 1;
			goto out;
		} else {
			
			goto out_clear_request;
		}
	}

	
	len = x25_negotiate_facilities(skb, sk, &facilities, &dte_facilities);
	if (len == -1)
		goto out_sock_put;

	

	x25_limit_facilities(&facilities, nb);

	
	make = x25_make_new(sk);
	if (!make)
		goto out_sock_put;

	
	skb_pull(skb, len);

	skb->sk     = make;
	make->sk_state = TCP_ESTABLISHED;

	makex25 = x25_sk(make);
	makex25->lci           = lci;
	makex25->dest_addr     = dest_addr;
	makex25->source_addr   = source_addr;
	makex25->neighbour     = nb;
	makex25->facilities    = facilities;
	makex25->dte_facilities= dte_facilities;
	makex25->vc_facil_mask = x25_sk(sk)->vc_facil_mask;
	
	makex25->vc_facil_mask &= ~X25_MASK_REVERSE;
	
	makex25->vc_facil_mask &= ~X25_MASK_CALLING_AE;
	makex25->cudmatchlength = x25_sk(sk)->cudmatchlength;

	
	if(makex25->accptapprv & X25_DENY_ACCPT_APPRV) {
		x25_write_internal(make, X25_CALL_ACCEPTED);
		makex25->state = X25_STATE_3;
	}

	
	skb_copy_from_linear_data(skb, makex25->calluserdata.cuddata, skb->len);
	makex25->calluserdata.cudlength = skb->len;

	sk->sk_ack_backlog++;

	x25_insert_socket(make);

	skb_queue_head(&sk->sk_receive_queue, skb);

	x25_start_heartbeat(make);

	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_data_ready(sk, skb->len);
	rc = 1;
	sock_put(sk);
out:
	return rc;
out_sock_put:
	sock_put(sk);
out_clear_request:
	rc = 0;
	x25_transmit_clear_request(nb, lci, 0x01);
	goto out;
}

static int x25_sendmsg(struct kiocb *iocb, struct socket *sock,
		       struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct x25_sock *x25 = x25_sk(sk);
	struct sockaddr_x25 *usx25 = (struct sockaddr_x25 *)msg->msg_name;
	struct sockaddr_x25 sx25;
	struct sk_buff *skb;
	unsigned char *asmptr;
	int noblock = msg->msg_flags & MSG_DONTWAIT;
	size_t size;
	int qbit = 0, rc = -EINVAL;

	if (msg->msg_flags & ~(MSG_DONTWAIT|MSG_OOB|MSG_EOR|MSG_CMSG_COMPAT))
		goto out;

	
	if (!(msg->msg_flags & (MSG_EOR|MSG_OOB)))
		goto out;

	rc = -EADDRNOTAVAIL;
	if (sock_flag(sk, SOCK_ZAPPED))
		goto out;

	rc = -EPIPE;
	if (sk->sk_shutdown & SEND_SHUTDOWN) {
		send_sig(SIGPIPE, current, 0);
		goto out;
	}

	rc = -ENETUNREACH;
	if (!x25->neighbour)
		goto out;

	if (usx25) {
		rc = -EINVAL;
		if (msg->msg_namelen < sizeof(sx25))
			goto out;
		memcpy(&sx25, usx25, sizeof(sx25));
		rc = -EISCONN;
		if (strcmp(x25->dest_addr.x25_addr, sx25.sx25_addr.x25_addr))
			goto out;
		rc = -EINVAL;
		if (sx25.sx25_family != AF_X25)
			goto out;
	} else {
		
		rc = -ENOTCONN;
		if (sk->sk_state != TCP_ESTABLISHED)
			goto out;

		sx25.sx25_family = AF_X25;
		sx25.sx25_addr   = x25->dest_addr;
	}

	
	if (len > 65535) {
		rc = -EMSGSIZE;
		goto out;
	}

	SOCK_DEBUG(sk, "x25_sendmsg: sendto: Addresses built.\n");

	
	SOCK_DEBUG(sk, "x25_sendmsg: sendto: building packet.\n");

	if ((msg->msg_flags & MSG_OOB) && len > 32)
		len = 32;

	size = len + X25_MAX_L2_LEN + X25_EXT_MIN_LEN;

	skb = sock_alloc_send_skb(sk, size, noblock, &rc);
	if (!skb)
		goto out;
	X25_SKB_CB(skb)->flags = msg->msg_flags;

	skb_reserve(skb, X25_MAX_L2_LEN + X25_EXT_MIN_LEN);

	
	SOCK_DEBUG(sk, "x25_sendmsg: Copying user data\n");

	skb_reset_transport_header(skb);
	skb_put(skb, len);

	rc = memcpy_fromiovec(skb_transport_header(skb), msg->msg_iov, len);
	if (rc)
		goto out_kfree_skb;

	
	if (x25->qbitincl) {
		qbit = skb->data[0];
		skb_pull(skb, 1);
	}

	
	SOCK_DEBUG(sk, "x25_sendmsg: Building X.25 Header.\n");

	if (msg->msg_flags & MSG_OOB) {
		if (x25->neighbour->extended) {
			asmptr    = skb_push(skb, X25_STD_MIN_LEN);
			*asmptr++ = ((x25->lci >> 8) & 0x0F) | X25_GFI_EXTSEQ;
			*asmptr++ = (x25->lci >> 0) & 0xFF;
			*asmptr++ = X25_INTERRUPT;
		} else {
			asmptr    = skb_push(skb, X25_STD_MIN_LEN);
			*asmptr++ = ((x25->lci >> 8) & 0x0F) | X25_GFI_STDSEQ;
			*asmptr++ = (x25->lci >> 0) & 0xFF;
			*asmptr++ = X25_INTERRUPT;
		}
	} else {
		if (x25->neighbour->extended) {
			
			asmptr    = skb_push(skb, X25_EXT_MIN_LEN);
			*asmptr++ = ((x25->lci >> 8) & 0x0F) | X25_GFI_EXTSEQ;
			*asmptr++ = (x25->lci >> 0) & 0xFF;
			*asmptr++ = X25_DATA;
			*asmptr++ = X25_DATA;
		} else {
			
			asmptr    = skb_push(skb, X25_STD_MIN_LEN);
			*asmptr++ = ((x25->lci >> 8) & 0x0F) | X25_GFI_STDSEQ;
			*asmptr++ = (x25->lci >> 0) & 0xFF;
			*asmptr++ = X25_DATA;
		}

		if (qbit)
			skb->data[0] |= X25_Q_BIT;
	}

	SOCK_DEBUG(sk, "x25_sendmsg: Built header.\n");
	SOCK_DEBUG(sk, "x25_sendmsg: Transmitting buffer\n");

	rc = -ENOTCONN;
	if (sk->sk_state != TCP_ESTABLISHED)
		goto out_kfree_skb;

	if (msg->msg_flags & MSG_OOB)
		skb_queue_tail(&x25->interrupt_out_queue, skb);
	else {
		rc = x25_output(sk, skb);
		len = rc;
		if (rc < 0)
			kfree_skb(skb);
		else if (x25->qbitincl)
			len++;
	}

	
	lock_sock(sk);
	x25_kick(sk);
	release_sock(sk);
	rc = len;
out:
	return rc;
out_kfree_skb:
	kfree_skb(skb);
	goto out;
}


static int x25_recvmsg(struct kiocb *iocb, struct socket *sock,
		       struct msghdr *msg, size_t size,
		       int flags)
{
	struct sock *sk = sock->sk;
	struct x25_sock *x25 = x25_sk(sk);
	struct sockaddr_x25 *sx25 = (struct sockaddr_x25 *)msg->msg_name;
	size_t copied;
	int qbit;
	struct sk_buff *skb;
	unsigned char *asmptr;
	int rc = -ENOTCONN;

	
	if (sk->sk_state != TCP_ESTABLISHED)
		goto out;

	if (flags & MSG_OOB) {
		rc = -EINVAL;
		if (sock_flag(sk, SOCK_URGINLINE) ||
		    !skb_peek(&x25->interrupt_in_queue))
			goto out;

		skb = skb_dequeue(&x25->interrupt_in_queue);

		skb_pull(skb, X25_STD_MIN_LEN);

		
		if (x25->qbitincl) {
			asmptr  = skb_push(skb, 1);
			*asmptr = 0x00;
		}

		msg->msg_flags |= MSG_OOB;
	} else {
		
		skb = skb_recv_datagram(sk, flags & ~MSG_DONTWAIT,
					flags & MSG_DONTWAIT, &rc);
		if (!skb)
			goto out;

		qbit = (skb->data[0] & X25_Q_BIT) == X25_Q_BIT;

		skb_pull(skb, x25->neighbour->extended ?
				X25_EXT_MIN_LEN : X25_STD_MIN_LEN);

		if (x25->qbitincl) {
			asmptr  = skb_push(skb, 1);
			*asmptr = qbit;
		}
	}

	skb_reset_transport_header(skb);
	copied = skb->len;

	if (copied > size) {
		copied = size;
		msg->msg_flags |= MSG_TRUNC;
	}

	
	msg->msg_flags |= MSG_EOR;

	rc = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);
	if (rc)
		goto out_free_dgram;

	if (sx25) {
		sx25->sx25_family = AF_X25;
		sx25->sx25_addr   = x25->dest_addr;
	}

	msg->msg_namelen = sizeof(struct sockaddr_x25);

	lock_sock(sk);
	x25_check_rbuf(sk);
	release_sock(sk);
	rc = copied;
out_free_dgram:
	skb_free_datagram(sk, skb);
out:
	return rc;
}


static int x25_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;
	struct x25_sock *x25 = x25_sk(sk);
	void __user *argp = (void __user *)arg;
	int rc;

	switch (cmd) {
		case TIOCOUTQ: {
			int amount = sk->sk_sndbuf - sk_wmem_alloc_get(sk);

			if (amount < 0)
				amount = 0;
			rc = put_user(amount, (unsigned int __user *)argp);
			break;
		}

		case TIOCINQ: {
			struct sk_buff *skb;
			int amount = 0;
			
			if ((skb = skb_peek(&sk->sk_receive_queue)) != NULL)
				amount = skb->len;
			rc = put_user(amount, (unsigned int __user *)argp);
			break;
		}

		case SIOCGSTAMP:
			rc = -EINVAL;
			if (sk)
				rc = sock_get_timestamp(sk,
						(struct timeval __user *)argp);
			break;
		case SIOCGSTAMPNS:
			rc = -EINVAL;
			if (sk)
				rc = sock_get_timestampns(sk,
						(struct timespec __user *)argp);
			break;
		case SIOCGIFADDR:
		case SIOCSIFADDR:
		case SIOCGIFDSTADDR:
		case SIOCSIFDSTADDR:
		case SIOCGIFBRDADDR:
		case SIOCSIFBRDADDR:
		case SIOCGIFNETMASK:
		case SIOCSIFNETMASK:
		case SIOCGIFMETRIC:
		case SIOCSIFMETRIC:
			rc = -EINVAL;
			break;
		case SIOCADDRT:
		case SIOCDELRT:
			rc = -EPERM;
			if (!capable(CAP_NET_ADMIN))
				break;
			rc = x25_route_ioctl(cmd, argp);
			break;
		case SIOCX25GSUBSCRIP:
			rc = x25_subscr_ioctl(cmd, argp);
			break;
		case SIOCX25SSUBSCRIP:
			rc = -EPERM;
			if (!capable(CAP_NET_ADMIN))
				break;
			rc = x25_subscr_ioctl(cmd, argp);
			break;
		case SIOCX25GFACILITIES: {
			struct x25_facilities fac = x25->facilities;
			rc = copy_to_user(argp, &fac,
					  sizeof(fac)) ? -EFAULT : 0;
			break;
		}

		case SIOCX25SFACILITIES: {
			struct x25_facilities facilities;
			rc = -EFAULT;
			if (copy_from_user(&facilities, argp,
					   sizeof(facilities)))
				break;
			rc = -EINVAL;
			if (sk->sk_state != TCP_LISTEN &&
			    sk->sk_state != TCP_CLOSE)
				break;
			if (facilities.pacsize_in < X25_PS16 ||
			    facilities.pacsize_in > X25_PS4096)
				break;
			if (facilities.pacsize_out < X25_PS16 ||
			    facilities.pacsize_out > X25_PS4096)
				break;
			if (facilities.winsize_in < 1 ||
			    facilities.winsize_in > 127)
				break;
			if (facilities.throughput < 0x03 ||
			    facilities.throughput > 0xDD)
				break;
			if (facilities.reverse &&
				(facilities.reverse | 0x81)!= 0x81)
				break;
			x25->facilities = facilities;
			rc = 0;
			break;
		}

		case SIOCX25GDTEFACILITIES: {
			rc = copy_to_user(argp, &x25->dte_facilities,
						sizeof(x25->dte_facilities));
			if (rc)
				rc = -EFAULT;
			break;
		}

		case SIOCX25SDTEFACILITIES: {
			struct x25_dte_facilities dtefacs;
			rc = -EFAULT;
			if (copy_from_user(&dtefacs, argp, sizeof(dtefacs)))
				break;
			rc = -EINVAL;
			if (sk->sk_state != TCP_LISTEN &&
					sk->sk_state != TCP_CLOSE)
				break;
			if (dtefacs.calling_len > X25_MAX_AE_LEN)
				break;
			if (dtefacs.calling_ae == NULL)
				break;
			if (dtefacs.called_len > X25_MAX_AE_LEN)
				break;
			if (dtefacs.called_ae == NULL)
				break;
			x25->dte_facilities = dtefacs;
			rc = 0;
			break;
		}

		case SIOCX25GCALLUSERDATA: {
			struct x25_calluserdata cud = x25->calluserdata;
			rc = copy_to_user(argp, &cud,
					  sizeof(cud)) ? -EFAULT : 0;
			break;
		}

		case SIOCX25SCALLUSERDATA: {
			struct x25_calluserdata calluserdata;

			rc = -EFAULT;
			if (copy_from_user(&calluserdata, argp,
					   sizeof(calluserdata)))
				break;
			rc = -EINVAL;
			if (calluserdata.cudlength > X25_MAX_CUD_LEN)
				break;
			x25->calluserdata = calluserdata;
			rc = 0;
			break;
		}

		case SIOCX25GCAUSEDIAG: {
			struct x25_causediag causediag;
			causediag = x25->causediag;
			rc = copy_to_user(argp, &causediag,
					  sizeof(causediag)) ? -EFAULT : 0;
			break;
		}

		case SIOCX25SCUDMATCHLEN: {
			struct x25_subaddr sub_addr;
			rc = -EINVAL;
			if(sk->sk_state != TCP_CLOSE)
				break;
			rc = -EFAULT;
			if (copy_from_user(&sub_addr, argp,
					sizeof(sub_addr)))
				break;
			rc = -EINVAL;
			if(sub_addr.cudmatchlength > X25_MAX_CUD_LEN)
				break;
			x25->cudmatchlength = sub_addr.cudmatchlength;
			rc = 0;
			break;
		}

		case SIOCX25CALLACCPTAPPRV: {
			rc = -EINVAL;
			if (sk->sk_state != TCP_CLOSE)
				break;
			x25->accptapprv = X25_ALLOW_ACCPT_APPRV;
			rc = 0;
			break;
		}

		case SIOCX25SENDCALLACCPT:  {
			rc = -EINVAL;
			if (sk->sk_state != TCP_ESTABLISHED)
				break;
			if (x25->accptapprv)	
				break;
			x25_write_internal(sk, X25_CALL_ACCEPTED);
			x25->state = X25_STATE_3;
			rc = 0;
			break;
		}

		default:
			rc = -ENOIOCTLCMD;
			break;
	}

	return rc;
}

static struct net_proto_family x25_family_ops = {
	.family =	AF_X25,
	.create =	x25_create,
	.owner	=	THIS_MODULE,
};

#ifdef CONFIG_COMPAT
static int compat_x25_subscr_ioctl(unsigned int cmd,
		struct compat_x25_subscrip_struct __user *x25_subscr32)
{
	struct compat_x25_subscrip_struct x25_subscr;
	struct x25_neigh *nb;
	struct net_device *dev;
	int rc = -EINVAL;

	rc = -EFAULT;
	if (copy_from_user(&x25_subscr, x25_subscr32, sizeof(*x25_subscr32)))
		goto out;

	rc = -EINVAL;
	dev = x25_dev_get(x25_subscr.device);
	if (dev == NULL)
		goto out;

	nb = x25_get_neigh(dev);
	if (nb == NULL)
		goto out_dev_put;

	dev_put(dev);

	if (cmd == SIOCX25GSUBSCRIP) {
		x25_subscr.extended = nb->extended;
		x25_subscr.global_facil_mask = nb->global_facil_mask;
		rc = copy_to_user(x25_subscr32, &x25_subscr,
				sizeof(*x25_subscr32)) ? -EFAULT : 0;
	} else {
		rc = -EINVAL;
		if (x25_subscr.extended == 0 || x25_subscr.extended == 1) {
			rc = 0;
			nb->extended = x25_subscr.extended;
			nb->global_facil_mask = x25_subscr.global_facil_mask;
		}
	}
	x25_neigh_put(nb);
out:
	return rc;
out_dev_put:
	dev_put(dev);
	goto out;
}

static int compat_x25_ioctl(struct socket *sock, unsigned int cmd,
				unsigned long arg)
{
	void __user *argp = compat_ptr(arg);
	struct sock *sk = sock->sk;

	int rc = -ENOIOCTLCMD;

	switch(cmd) {
	case TIOCOUTQ:
	case TIOCINQ:
		rc = x25_ioctl(sock, cmd, (unsigned long)argp);
		break;
	case SIOCGSTAMP:
		rc = -EINVAL;
		if (sk)
			rc = compat_sock_get_timestamp(sk,
					(struct timeval __user*)argp);
		break;
	case SIOCGSTAMPNS:
		rc = -EINVAL;
		if (sk)
			rc = compat_sock_get_timestampns(sk,
					(struct timespec __user*)argp);
		break;
	case SIOCGIFADDR:
	case SIOCSIFADDR:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCGIFMETRIC:
	case SIOCSIFMETRIC:
		rc = -EINVAL;
		break;
	case SIOCADDRT:
	case SIOCDELRT:
		rc = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			break;
		rc = x25_route_ioctl(cmd, argp);
		break;
	case SIOCX25GSUBSCRIP:
		rc = compat_x25_subscr_ioctl(cmd, argp);
		break;
	case SIOCX25SSUBSCRIP:
		rc = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			break;
		rc = compat_x25_subscr_ioctl(cmd, argp);
		break;
	case SIOCX25GFACILITIES:
	case SIOCX25SFACILITIES:
	case SIOCX25GDTEFACILITIES:
	case SIOCX25SDTEFACILITIES:
	case SIOCX25GCALLUSERDATA:
	case SIOCX25SCALLUSERDATA:
	case SIOCX25GCAUSEDIAG:
	case SIOCX25SCUDMATCHLEN:
	case SIOCX25CALLACCPTAPPRV:
	case SIOCX25SENDCALLACCPT:
		rc = x25_ioctl(sock, cmd, (unsigned long)argp);
		break;
	default:
		rc = -ENOIOCTLCMD;
		break;
	}
	return rc;
}
#endif

static const struct proto_ops SOCKOPS_WRAPPED(x25_proto_ops) = {
	.family =	AF_X25,
	.owner =	THIS_MODULE,
	.release =	x25_release,
	.bind =		x25_bind,
	.connect =	x25_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	x25_accept,
	.getname =	x25_getname,
	.poll =		datagram_poll,
	.ioctl =	x25_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_x25_ioctl,
#endif
	.listen =	x25_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	x25_setsockopt,
	.getsockopt =	x25_getsockopt,
	.sendmsg =	x25_sendmsg,
	.recvmsg =	x25_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

SOCKOPS_WRAP(x25_proto, AF_X25);

static struct packet_type x25_packet_type __read_mostly = {
	.type =	cpu_to_be16(ETH_P_X25),
	.func =	x25_lapb_receive_frame,
};

static struct notifier_block x25_dev_notifier = {
	.notifier_call = x25_device_event,
};

void x25_kill_by_neigh(struct x25_neigh *nb)
{
	struct sock *s;
	struct hlist_node *node;

	write_lock_bh(&x25_list_lock);

	sk_for_each(s, node, &x25_list)
		if (x25_sk(s)->neighbour == nb)
			x25_disconnect(s, ENETUNREACH, 0, 0);

	write_unlock_bh(&x25_list_lock);

	
	x25_clear_forward_by_dev(nb->dev);
}

static int __init x25_init(void)
{
	int rc = proto_register(&x25_proto, 0);

	if (rc != 0)
		goto out;

	sock_register(&x25_family_ops);

	dev_add_pack(&x25_packet_type);

	register_netdevice_notifier(&x25_dev_notifier);

	printk(KERN_INFO "X.25 for Linux Version 0.2\n");

#ifdef CONFIG_SYSCTL
	x25_register_sysctl();
#endif
	x25_proc_init();
out:
	return rc;
}
module_init(x25_init);

static void __exit x25_exit(void)
{
	x25_proc_exit();
	x25_link_free();
	x25_route_free();

#ifdef CONFIG_SYSCTL
	x25_unregister_sysctl();
#endif

	unregister_netdevice_notifier(&x25_dev_notifier);

	dev_remove_pack(&x25_packet_type);

	sock_unregister(AF_X25);
	proto_unregister(&x25_proto);
}
module_exit(x25_exit);

MODULE_AUTHOR("Jonathan Naylor <g4klx@g4klx.demon.co.uk>");
MODULE_DESCRIPTION("The X.25 Packet Layer network layer protocol");
MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_X25);
