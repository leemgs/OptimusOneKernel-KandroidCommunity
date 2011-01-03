

#include <linux/capability.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/smp_lock.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/irda.h>
#include <linux/poll.h>

#include <asm/ioctls.h>		
#include <asm/uaccess.h>

#include <net/sock.h>
#include <net/tcp_states.h>

#include <net/irda/af_irda.h>

static int irda_create(struct net *net, struct socket *sock, int protocol);

static const struct proto_ops irda_stream_ops;
static const struct proto_ops irda_seqpacket_ops;
static const struct proto_ops irda_dgram_ops;

#ifdef CONFIG_IRDA_ULTRA
static const struct proto_ops irda_ultra_ops;
#define ULTRA_MAX_DATA 382
#endif 

#define IRDA_MAX_HEADER (TTP_MAX_HEADER)


static int irda_data_indication(void *instance, void *sap, struct sk_buff *skb)
{
	struct irda_sock *self;
	struct sock *sk;
	int err;

	IRDA_DEBUG(3, "%s()\n", __func__);

	self = instance;
	sk = instance;

	err = sock_queue_rcv_skb(sk, skb);
	if (err) {
		IRDA_DEBUG(1, "%s(), error: no more mem!\n", __func__);
		self->rx_flow = FLOW_STOP;

		
		return err;
	}

	return 0;
}


static void irda_disconnect_indication(void *instance, void *sap,
				       LM_REASON reason, struct sk_buff *skb)
{
	struct irda_sock *self;
	struct sock *sk;

	self = instance;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	
	if(skb)
		dev_kfree_skb(skb);

	sk = instance;
	if (sk == NULL) {
		IRDA_DEBUG(0, "%s(%p) : BUG : sk is NULL\n",
			   __func__, self);
		return;
	}

	
	bh_lock_sock(sk);
	if (!sock_flag(sk, SOCK_DEAD) && sk->sk_state != TCP_CLOSE) {
		sk->sk_state     = TCP_CLOSE;
		sk->sk_shutdown |= SEND_SHUTDOWN;

		sk->sk_state_change(sk);

		
		if (self->tsap) {
			irttp_close_tsap(self->tsap);
			self->tsap = NULL;
		}
	}
	bh_unlock_sock(sk);

	
}


static void irda_connect_confirm(void *instance, void *sap,
				 struct qos_info *qos,
				 __u32 max_sdu_size, __u8 max_header_size,
				 struct sk_buff *skb)
{
	struct irda_sock *self;
	struct sock *sk;

	self = instance;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	sk = instance;
	if (sk == NULL) {
		dev_kfree_skb(skb);
		return;
	}

	dev_kfree_skb(skb);
	

	
	self->max_header_size = max_header_size;

	
	self->max_sdu_size_tx = max_sdu_size;

	
	switch (sk->sk_type) {
	case SOCK_STREAM:
		if (max_sdu_size != 0) {
			IRDA_ERROR("%s: max_sdu_size must be 0\n",
				   __func__);
			return;
		}
		self->max_data_size = irttp_get_max_seg_size(self->tsap);
		break;
	case SOCK_SEQPACKET:
		if (max_sdu_size == 0) {
			IRDA_ERROR("%s: max_sdu_size cannot be 0\n",
				   __func__);
			return;
		}
		self->max_data_size = max_sdu_size;
		break;
	default:
		self->max_data_size = irttp_get_max_seg_size(self->tsap);
	}

	IRDA_DEBUG(2, "%s(), max_data_size=%d\n", __func__,
		   self->max_data_size);

	memcpy(&self->qos_tx, qos, sizeof(struct qos_info));

	
	sk->sk_state = TCP_ESTABLISHED;
	sk->sk_state_change(sk);
}


static void irda_connect_indication(void *instance, void *sap,
				    struct qos_info *qos, __u32 max_sdu_size,
				    __u8 max_header_size, struct sk_buff *skb)
{
	struct irda_sock *self;
	struct sock *sk;

	self = instance;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	sk = instance;
	if (sk == NULL) {
		dev_kfree_skb(skb);
		return;
	}

	
	self->max_header_size = max_header_size;

	
	self->max_sdu_size_tx = max_sdu_size;

	
	switch (sk->sk_type) {
	case SOCK_STREAM:
		if (max_sdu_size != 0) {
			IRDA_ERROR("%s: max_sdu_size must be 0\n",
				   __func__);
			kfree_skb(skb);
			return;
		}
		self->max_data_size = irttp_get_max_seg_size(self->tsap);
		break;
	case SOCK_SEQPACKET:
		if (max_sdu_size == 0) {
			IRDA_ERROR("%s: max_sdu_size cannot be 0\n",
				   __func__);
			kfree_skb(skb);
			return;
		}
		self->max_data_size = max_sdu_size;
		break;
	default:
		self->max_data_size = irttp_get_max_seg_size(self->tsap);
	}

	IRDA_DEBUG(2, "%s(), max_data_size=%d\n", __func__,
		   self->max_data_size);

	memcpy(&self->qos_tx, qos, sizeof(struct qos_info));

	skb_queue_tail(&sk->sk_receive_queue, skb);
	sk->sk_state_change(sk);
}


static void irda_connect_response(struct irda_sock *self)
{
	struct sk_buff *skb;

	IRDA_DEBUG(2, "%s()\n", __func__);

	skb = alloc_skb(TTP_MAX_HEADER + TTP_SAR_HEADER,
			GFP_ATOMIC);
	if (skb == NULL) {
		IRDA_DEBUG(0, "%s() Unable to allocate sk_buff!\n",
			   __func__);
		return;
	}

	
	skb_reserve(skb, IRDA_MAX_HEADER);

	irttp_connect_response(self->tsap, self->max_sdu_size_rx, skb);
}


static void irda_flow_indication(void *instance, void *sap, LOCAL_FLOW flow)
{
	struct irda_sock *self;
	struct sock *sk;

	IRDA_DEBUG(2, "%s()\n", __func__);

	self = instance;
	sk = instance;
	BUG_ON(sk == NULL);

	switch (flow) {
	case FLOW_STOP:
		IRDA_DEBUG(1, "%s(), IrTTP wants us to slow down\n",
			   __func__);
		self->tx_flow = flow;
		break;
	case FLOW_START:
		self->tx_flow = flow;
		IRDA_DEBUG(1, "%s(), IrTTP wants us to start again\n",
			   __func__);
		wake_up_interruptible(sk->sk_sleep);
		break;
	default:
		IRDA_DEBUG(0, "%s(), Unknown flow command!\n", __func__);
		
		self->tx_flow = flow;
		break;
	}
}


static void irda_getvalue_confirm(int result, __u16 obj_id,
				  struct ias_value *value, void *priv)
{
	struct irda_sock *self;

	self = (struct irda_sock *) priv;
	if (!self) {
		IRDA_WARNING("%s: lost myself!\n", __func__);
		return;
	}

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	
	iriap_close(self->iriap);
	self->iriap = NULL;

	
	if (result != IAS_SUCCESS) {
		IRDA_DEBUG(1, "%s(), IAS query failed! (%d)\n", __func__,
			   result);

		self->errno = result;	

		
		wake_up_interruptible(&self->query_wait);

		return;
	}

	
	self->ias_result = value;
	self->errno = 0;

	
	wake_up_interruptible(&self->query_wait);
}


static void irda_selective_discovery_indication(discinfo_t *discovery,
						DISCOVERY_MODE mode,
						void *priv)
{
	struct irda_sock *self;

	IRDA_DEBUG(2, "%s()\n", __func__);

	self = (struct irda_sock *) priv;
	if (!self) {
		IRDA_WARNING("%s: lost myself!\n", __func__);
		return;
	}

	
	self->cachedaddr = discovery->daddr;

	
	wake_up_interruptible(&self->query_wait);
}


static void irda_discovery_timeout(u_long priv)
{
	struct irda_sock *self;

	IRDA_DEBUG(2, "%s()\n", __func__);

	self = (struct irda_sock *) priv;
	BUG_ON(self == NULL);

	
	self->cachelog = NULL;
	self->cachedaddr = 0;
	self->errno = -ETIME;

	
	wake_up_interruptible(&self->query_wait);
}


static int irda_open_tsap(struct irda_sock *self, __u8 tsap_sel, char *name)
{
	notify_t notify;

	if (self->tsap) {
		IRDA_WARNING("%s: busy!\n", __func__);
		return -EBUSY;
	}

	
	irda_notify_init(&notify);
	notify.connect_confirm       = irda_connect_confirm;
	notify.connect_indication    = irda_connect_indication;
	notify.disconnect_indication = irda_disconnect_indication;
	notify.data_indication       = irda_data_indication;
	notify.udata_indication	     = irda_data_indication;
	notify.flow_indication       = irda_flow_indication;
	notify.instance = self;
	strncpy(notify.name, name, NOTIFY_MAX_NAME);

	self->tsap = irttp_open_tsap(tsap_sel, DEFAULT_INITIAL_CREDIT,
				     &notify);
	if (self->tsap == NULL) {
		IRDA_DEBUG(0, "%s(), Unable to allocate TSAP!\n",
			   __func__);
		return -ENOMEM;
	}
	
	self->stsap_sel = self->tsap->stsap_sel;

	return 0;
}


#ifdef CONFIG_IRDA_ULTRA
static int irda_open_lsap(struct irda_sock *self, int pid)
{
	notify_t notify;

	if (self->lsap) {
		IRDA_WARNING("%s(), busy!\n", __func__);
		return -EBUSY;
	}

	
	irda_notify_init(&notify);
	notify.udata_indication	= irda_data_indication;
	notify.instance = self;
	strncpy(notify.name, "Ultra", NOTIFY_MAX_NAME);

	self->lsap = irlmp_open_lsap(LSAP_CONNLESS, &notify, pid);
	if (self->lsap == NULL) {
		IRDA_DEBUG( 0, "%s(), Unable to allocate LSAP!\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
#endif 


static int irda_find_lsap_sel(struct irda_sock *self, char *name)
{
	IRDA_DEBUG(2, "%s(%p, %s)\n", __func__, self, name);

	if (self->iriap) {
		IRDA_WARNING("%s(): busy with a previous query\n",
			     __func__);
		return -EBUSY;
	}

	self->iriap = iriap_open(LSAP_ANY, IAS_CLIENT, self,
				 irda_getvalue_confirm);
	if(self->iriap == NULL)
		return -ENOMEM;

	
	self->errno = -EHOSTUNREACH;

	
	iriap_getvaluebyclass_request(self->iriap, self->saddr, self->daddr,
				      name, "IrDA:TinyTP:LsapSel");

	
	if (wait_event_interruptible(self->query_wait, (self->iriap==NULL)))
		
		return -EHOSTUNREACH;

	
	if (self->errno)
	{
		
		if((self->errno == IAS_CLASS_UNKNOWN) ||
		   (self->errno == IAS_ATTRIB_UNKNOWN))
			return (-EADDRNOTAVAIL);
		else
			return (-EHOSTUNREACH);
	}

	
	switch (self->ias_result->type) {
	case IAS_INTEGER:
		IRDA_DEBUG(4, "%s() int=%d\n",
			   __func__, self->ias_result->t.integer);

		if (self->ias_result->t.integer != -1)
			self->dtsap_sel = self->ias_result->t.integer;
		else
			self->dtsap_sel = 0;
		break;
	default:
		self->dtsap_sel = 0;
		IRDA_DEBUG(0, "%s(), bad type!\n", __func__);
		break;
	}
	if (self->ias_result)
		irias_delete_value(self->ias_result);

	if (self->dtsap_sel)
		return 0;

	return -EADDRNOTAVAIL;
}


static int irda_discover_daddr_and_lsap_sel(struct irda_sock *self, char *name)
{
	discinfo_t *discoveries;	
	int	number;			
	int	i;
	int	err = -ENETUNREACH;
	__u32	daddr = DEV_ADDR_ANY;	
	__u8	dtsap_sel = 0x0;	

	IRDA_DEBUG(2, "%s(), name=%s\n", __func__, name);

	
	discoveries = irlmp_get_discoveries(&number, self->mask.word,
					    self->nslots);
	
	if (discoveries == NULL)
		return -ENETUNREACH;	

	
	for(i = 0; i < number; i++) {
		
		self->daddr = discoveries[i].daddr;
		self->saddr = 0x0;
		IRDA_DEBUG(1, "%s(), trying daddr = %08x\n",
			   __func__, self->daddr);

		
		err = irda_find_lsap_sel(self, name);
		switch (err) {
		case 0:
			
			if(daddr != DEV_ADDR_ANY) {
				IRDA_DEBUG(1, "%s(), discovered service ''%s'' in two different devices !!!\n",
					   __func__, name);
				self->daddr = DEV_ADDR_ANY;
				kfree(discoveries);
				return(-ENOTUNIQ);
			}
			
			daddr = self->daddr;
			dtsap_sel = self->dtsap_sel;
			break;
		case -EADDRNOTAVAIL:
			
			break;
		default:
			
			IRDA_DEBUG(0, "%s(), unexpected IAS query failure\n", __func__);
			self->daddr = DEV_ADDR_ANY;
			kfree(discoveries);
			return(-EHOSTUNREACH);
			break;
		}
	}
	
	kfree(discoveries);

	
	if(daddr == DEV_ADDR_ANY) {
		IRDA_DEBUG(1, "%s(), cannot discover service ''%s'' in any device !!!\n",
			   __func__, name);
		self->daddr = DEV_ADDR_ANY;
		return(-EADDRNOTAVAIL);
	}

	
	self->daddr = daddr;
	self->saddr = 0x0;
	self->dtsap_sel = dtsap_sel;

	IRDA_DEBUG(1, "%s(), discovered requested service ''%s'' at address %08x\n",
		   __func__, name, self->daddr);

	return 0;
}


static int irda_getname(struct socket *sock, struct sockaddr *uaddr,
			int *uaddr_len, int peer)
{
	struct sockaddr_irda saddr;
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);

	memset(&saddr, 0, sizeof(saddr));
	if (peer) {
		if (sk->sk_state != TCP_ESTABLISHED)
			return -ENOTCONN;

		saddr.sir_family = AF_IRDA;
		saddr.sir_lsap_sel = self->dtsap_sel;
		saddr.sir_addr = self->daddr;
	} else {
		saddr.sir_family = AF_IRDA;
		saddr.sir_lsap_sel = self->stsap_sel;
		saddr.sir_addr = self->saddr;
	}

	IRDA_DEBUG(1, "%s(), tsap_sel = %#x\n", __func__, saddr.sir_lsap_sel);
	IRDA_DEBUG(1, "%s(), addr = %08x\n", __func__, saddr.sir_addr);

	
	*uaddr_len = sizeof (struct sockaddr_irda);
	memcpy(uaddr, &saddr, *uaddr_len);

	return 0;
}


static int irda_listen(struct socket *sock, int backlog)
{
	struct sock *sk = sock->sk;

	IRDA_DEBUG(2, "%s()\n", __func__);

	if ((sk->sk_type != SOCK_STREAM) && (sk->sk_type != SOCK_SEQPACKET) &&
	    (sk->sk_type != SOCK_DGRAM))
		return -EOPNOTSUPP;

	if (sk->sk_state != TCP_LISTEN) {
		sk->sk_max_ack_backlog = backlog;
		sk->sk_state           = TCP_LISTEN;

		return 0;
	}

	return -EOPNOTSUPP;
}


static int irda_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_irda *addr = (struct sockaddr_irda *) uaddr;
	struct irda_sock *self = irda_sk(sk);
	int err;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	if (addr_len != sizeof(struct sockaddr_irda))
		return -EINVAL;

#ifdef CONFIG_IRDA_ULTRA
	
	if ((sk->sk_type == SOCK_DGRAM) &&
	    (sk->sk_protocol == IRDAPROTO_ULTRA)) {
		self->pid = addr->sir_lsap_sel;
		if (self->pid & 0x80) {
			IRDA_DEBUG(0, "%s(), extension in PID not supp!\n", __func__);
			return -EOPNOTSUPP;
		}
		err = irda_open_lsap(self, self->pid);
		if (err < 0)
			return err;

		
		sock->state = SS_CONNECTED;
		sk->sk_state   = TCP_ESTABLISHED;

		return 0;
	}
#endif 

	self->ias_obj = irias_new_object(addr->sir_name, jiffies);
	if (self->ias_obj == NULL)
		return -ENOMEM;

	err = irda_open_tsap(self, addr->sir_lsap_sel, addr->sir_name);
	if (err < 0) {
		kfree(self->ias_obj->name);
		kfree(self->ias_obj);
		return err;
	}

	
	irias_add_integer_attrib(self->ias_obj, "IrDA:TinyTP:LsapSel",
				 self->stsap_sel, IAS_KERNEL_ATTR);
	irias_insert_object(self->ias_obj);

	return 0;
}


static int irda_accept(struct socket *sock, struct socket *newsock, int flags)
{
	struct sock *sk = sock->sk;
	struct irda_sock *new, *self = irda_sk(sk);
	struct sock *newsk;
	struct sk_buff *skb;
	int err;

	IRDA_DEBUG(2, "%s()\n", __func__);

	err = irda_create(sock_net(sk), newsock, sk->sk_protocol);
	if (err)
		return err;

	if (sock->state != SS_UNCONNECTED)
		return -EINVAL;

	if ((sk = sock->sk) == NULL)
		return -EINVAL;

	if ((sk->sk_type != SOCK_STREAM) && (sk->sk_type != SOCK_SEQPACKET) &&
	    (sk->sk_type != SOCK_DGRAM))
		return -EOPNOTSUPP;

	if (sk->sk_state != TCP_LISTEN)
		return -EINVAL;

	

	
	while (1) {
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb)
			break;

		
		if (flags & O_NONBLOCK)
			return -EWOULDBLOCK;

		err = wait_event_interruptible(*(sk->sk_sleep),
					skb_peek(&sk->sk_receive_queue));
		if (err)
			return err;
	}

	newsk = newsock->sk;
	if (newsk == NULL)
		return -EIO;

	newsk->sk_state = TCP_ESTABLISHED;

	new = irda_sk(newsk);

	
	new->tsap = irttp_dup(self->tsap, new);
	if (!new->tsap) {
		IRDA_DEBUG(0, "%s(), dup failed!\n", __func__);
		kfree_skb(skb);
		return -1;
	}

	new->stsap_sel = new->tsap->stsap_sel;
	new->dtsap_sel = new->tsap->dtsap_sel;
	new->saddr = irttp_get_saddr(new->tsap);
	new->daddr = irttp_get_daddr(new->tsap);

	new->max_sdu_size_tx = self->max_sdu_size_tx;
	new->max_sdu_size_rx = self->max_sdu_size_rx;
	new->max_data_size   = self->max_data_size;
	new->max_header_size = self->max_header_size;

	memcpy(&new->qos_tx, &self->qos_tx, sizeof(struct qos_info));

	
	irttp_listen(self->tsap);

	kfree_skb(skb);
	sk->sk_ack_backlog--;

	newsock->state = SS_CONNECTED;

	irda_connect_response(new);

	return 0;
}


static int irda_connect(struct socket *sock, struct sockaddr *uaddr,
			int addr_len, int flags)
{
	struct sock *sk = sock->sk;
	struct sockaddr_irda *addr = (struct sockaddr_irda *) uaddr;
	struct irda_sock *self = irda_sk(sk);
	int err;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	
	if ((sk->sk_type == SOCK_DGRAM) && (sk->sk_protocol == IRDAPROTO_ULTRA))
		return -ESOCKTNOSUPPORT;

	if (sk->sk_state == TCP_ESTABLISHED && sock->state == SS_CONNECTING) {
		sock->state = SS_CONNECTED;
		return 0;   
	}

	if (sk->sk_state == TCP_CLOSE && sock->state == SS_CONNECTING) {
		sock->state = SS_UNCONNECTED;
		return -ECONNREFUSED;
	}

	if (sk->sk_state == TCP_ESTABLISHED)
		return -EISCONN;      

	sk->sk_state   = TCP_CLOSE;
	sock->state = SS_UNCONNECTED;

	if (addr_len != sizeof(struct sockaddr_irda))
		return -EINVAL;

	
	if ((!addr->sir_addr) || (addr->sir_addr == DEV_ADDR_ANY)) {
		
		err = irda_discover_daddr_and_lsap_sel(self, addr->sir_name);
		if (err) {
			IRDA_DEBUG(0, "%s(), auto-connect failed!\n", __func__);
			return err;
		}
	} else {
		
		self->daddr = addr->sir_addr;
		IRDA_DEBUG(1, "%s(), daddr = %08x\n", __func__, self->daddr);

		
		if((addr->sir_name[0] != '\0') ||
		   (addr->sir_lsap_sel >= 0x70)) {
			
			err = irda_find_lsap_sel(self, addr->sir_name);
			if (err) {
				IRDA_DEBUG(0, "%s(), connect failed!\n", __func__);
				return err;
			}
		} else {
			
			self->dtsap_sel = addr->sir_lsap_sel;
		}
	}

	
	if (!self->tsap)
		irda_open_tsap(self, LSAP_ANY, addr->sir_name);

	
	sock->state = SS_CONNECTING;
	sk->sk_state   = TCP_SYN_SENT;

	
	err = irttp_connect_request(self->tsap, self->dtsap_sel,
				    self->saddr, self->daddr, NULL,
				    self->max_sdu_size_rx, NULL);
	if (err) {
		IRDA_DEBUG(0, "%s(), connect failed!\n", __func__);
		return err;
	}

	
	if (sk->sk_state != TCP_ESTABLISHED && (flags & O_NONBLOCK))
		return -EINPROGRESS;

	if (wait_event_interruptible(*(sk->sk_sleep),
				     (sk->sk_state != TCP_SYN_SENT)))
		return -ERESTARTSYS;

	if (sk->sk_state != TCP_ESTABLISHED) {
		sock->state = SS_UNCONNECTED;
		err = sock_error(sk);
		return err? err : -ECONNRESET;
	}

	sock->state = SS_CONNECTED;

	
	self->saddr = irttp_get_saddr(self->tsap);

	return 0;
}

static struct proto irda_proto = {
	.name	  = "IRDA",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct irda_sock),
};


static int irda_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
	struct irda_sock *self;

	IRDA_DEBUG(2, "%s()\n", __func__);

	if (net != &init_net)
		return -EAFNOSUPPORT;

	
	switch (sock->type) {
	case SOCK_STREAM:     
	case SOCK_SEQPACKET:  
	case SOCK_DGRAM:      
		break;
	default:
		return -ESOCKTNOSUPPORT;
	}

	
	sk = sk_alloc(net, PF_IRDA, GFP_ATOMIC, &irda_proto);
	if (sk == NULL)
		return -ENOMEM;

	self = irda_sk(sk);
	IRDA_DEBUG(2, "%s() : self is %p\n", __func__, self);

	init_waitqueue_head(&self->query_wait);

	switch (sock->type) {
	case SOCK_STREAM:
		sock->ops = &irda_stream_ops;
		self->max_sdu_size_rx = TTP_SAR_DISABLE;
		break;
	case SOCK_SEQPACKET:
		sock->ops = &irda_seqpacket_ops;
		self->max_sdu_size_rx = TTP_SAR_UNBOUND;
		break;
	case SOCK_DGRAM:
		switch (protocol) {
#ifdef CONFIG_IRDA_ULTRA
		case IRDAPROTO_ULTRA:
			sock->ops = &irda_ultra_ops;
			
			self->max_data_size = ULTRA_MAX_DATA - LMP_PID_HEADER;
			self->max_header_size = IRDA_MAX_HEADER + LMP_PID_HEADER;
			break;
#endif 
		case IRDAPROTO_UNITDATA:
			sock->ops = &irda_dgram_ops;
			
			self->max_sdu_size_rx = TTP_SAR_UNBOUND;
			break;
		default:
			sk_free(sk);
			return -ESOCKTNOSUPPORT;
		}
		break;
	default:
		sk_free(sk);
		return -ESOCKTNOSUPPORT;
	}

	
	sock_init_data(sock, sk);	
	sk->sk_family = PF_IRDA;
	sk->sk_protocol = protocol;

	
	self->ckey = irlmp_register_client(0, NULL, NULL, NULL);
	self->mask.word = 0xffff;
	self->rx_flow = self->tx_flow = FLOW_START;
	self->nslots = DISCOVERY_DEFAULT_SLOTS;
	self->daddr = DEV_ADDR_ANY;	
	self->saddr = 0x0;		
	return 0;
}


static void irda_destroy_socket(struct irda_sock *self)
{
	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	
	irlmp_unregister_client(self->ckey);
	irlmp_unregister_service(self->skey);

	
	if (self->ias_obj) {
		irias_delete_object(self->ias_obj);
		self->ias_obj = NULL;
	}

	if (self->iriap) {
		iriap_close(self->iriap);
		self->iriap = NULL;
	}

	if (self->tsap) {
		irttp_disconnect_request(self->tsap, NULL, P_NORMAL);
		irttp_close_tsap(self->tsap);
		self->tsap = NULL;
	}
#ifdef CONFIG_IRDA_ULTRA
	if (self->lsap) {
		irlmp_close_lsap(self->lsap);
		self->lsap = NULL;
	}
#endif 
}


static int irda_release(struct socket *sock)
{
	struct sock *sk = sock->sk;

	IRDA_DEBUG(2, "%s()\n", __func__);

	if (sk == NULL)
		return 0;

	lock_sock(sk);
	sk->sk_state       = TCP_CLOSE;
	sk->sk_shutdown   |= SEND_SHUTDOWN;
	sk->sk_state_change(sk);

	
	irda_destroy_socket(irda_sk(sk));

	sock_orphan(sk);
	sock->sk   = NULL;
	release_sock(sk);

	
	skb_queue_purge(&sk->sk_receive_queue);

	
	sock_put(sk);

	

	return 0;
}


static int irda_sendmsg(struct kiocb *iocb, struct socket *sock,
			struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self;
	struct sk_buff *skb;
	int err = -EPIPE;

	IRDA_DEBUG(4, "%s(), len=%zd\n", __func__, len);

	
	if (msg->msg_flags & ~(MSG_DONTWAIT | MSG_EOR | MSG_CMSG_COMPAT |
			       MSG_NOSIGNAL))
		return -EINVAL;

	if (sk->sk_shutdown & SEND_SHUTDOWN)
		goto out_err;

	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	self = irda_sk(sk);

	

	if (wait_event_interruptible(*(sk->sk_sleep),
	    (self->tx_flow != FLOW_STOP  ||  sk->sk_state != TCP_ESTABLISHED)))
		return -ERESTARTSYS;

	
	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	
	if (len > self->max_data_size) {
		IRDA_DEBUG(2, "%s(), Chopping frame from %zd to %d bytes!\n",
			   __func__, len, self->max_data_size);
		len = self->max_data_size;
	}

	skb = sock_alloc_send_skb(sk, len + self->max_header_size + 16,
				  msg->msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		goto out_err;

	skb_reserve(skb, self->max_header_size + 16);
	skb_reset_transport_header(skb);
	skb_put(skb, len);
	err = memcpy_fromiovec(skb_transport_header(skb), msg->msg_iov, len);
	if (err) {
		kfree_skb(skb);
		goto out_err;
	}

	
	err = irttp_data_request(self->tsap, skb);
	if (err) {
		IRDA_DEBUG(0, "%s(), err=%d\n", __func__, err);
		goto out_err;
	}
	
	return len;

 out_err:
	return sk_stream_error(sk, msg->msg_flags, err);

}


static int irda_recvmsg_dgram(struct kiocb *iocb, struct socket *sock,
			      struct msghdr *msg, size_t size, int flags)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);
	struct sk_buff *skb;
	size_t copied;
	int err;

	IRDA_DEBUG(4, "%s()\n", __func__);

	if ((err = sock_error(sk)) < 0)
		return err;

	skb = skb_recv_datagram(sk, flags & ~MSG_DONTWAIT,
				flags & MSG_DONTWAIT, &err);
	if (!skb)
		return err;

	skb_reset_transport_header(skb);
	copied = skb->len;

	if (copied > size) {
		IRDA_DEBUG(2, "%s(), Received truncated frame (%zd < %zd)!\n",
			   __func__, copied, size);
		copied = size;
		msg->msg_flags |= MSG_TRUNC;
	}
	skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);

	skb_free_datagram(sk, skb);

	
	if (self->rx_flow == FLOW_STOP) {
		if ((atomic_read(&sk->sk_rmem_alloc) << 2) <= sk->sk_rcvbuf) {
			IRDA_DEBUG(2, "%s(), Starting IrTTP\n", __func__);
			self->rx_flow = FLOW_START;
			irttp_flow_request(self->tsap, FLOW_START);
		}
	}

	return copied;
}


static int irda_recvmsg_stream(struct kiocb *iocb, struct socket *sock,
			       struct msghdr *msg, size_t size, int flags)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);
	int noblock = flags & MSG_DONTWAIT;
	size_t copied = 0;
	int target, err;
	long timeo;

	IRDA_DEBUG(3, "%s()\n", __func__);

	if ((err = sock_error(sk)) < 0)
		return err;

	if (sock->flags & __SO_ACCEPTCON)
		return(-EINVAL);

	if (flags & MSG_OOB)
		return -EOPNOTSUPP;

	target = sock_rcvlowat(sk, flags & MSG_WAITALL, size);
	timeo = sock_rcvtimeo(sk, noblock);

	msg->msg_namelen = 0;

	do {
		int chunk;
		struct sk_buff *skb = skb_dequeue(&sk->sk_receive_queue);

		if (skb == NULL) {
			DEFINE_WAIT(wait);
			int ret = 0;

			if (copied >= target)
				break;

			prepare_to_wait_exclusive(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);

			
			ret = sock_error(sk);
			if (ret)
				;
			else if (sk->sk_shutdown & RCV_SHUTDOWN)
				;
			else if (noblock)
				ret = -EAGAIN;
			else if (signal_pending(current))
				ret = sock_intr_errno(timeo);
			else if (sk->sk_state != TCP_ESTABLISHED)
				ret = -ENOTCONN;
			else if (skb_peek(&sk->sk_receive_queue) == NULL)
				
				schedule();

			finish_wait(sk->sk_sleep, &wait);

			if (ret)
				return ret;
			if (sk->sk_shutdown & RCV_SHUTDOWN)
				break;

			continue;
		}

		chunk = min_t(unsigned int, skb->len, size);
		if (memcpy_toiovec(msg->msg_iov, skb->data, chunk)) {
			skb_queue_head(&sk->sk_receive_queue, skb);
			if (copied == 0)
				copied = -EFAULT;
			break;
		}
		copied += chunk;
		size -= chunk;

		
		if (!(flags & MSG_PEEK)) {
			skb_pull(skb, chunk);

			
			if (skb->len) {
				IRDA_DEBUG(1, "%s(), back on q!\n",
					   __func__);
				skb_queue_head(&sk->sk_receive_queue, skb);
				break;
			}

			kfree_skb(skb);
		} else {
			IRDA_DEBUG(0, "%s() questionable!?\n", __func__);

			
			skb_queue_head(&sk->sk_receive_queue, skb);
			break;
		}
	} while (size);

	
	if (self->rx_flow == FLOW_STOP) {
		if ((atomic_read(&sk->sk_rmem_alloc) << 2) <= sk->sk_rcvbuf) {
			IRDA_DEBUG(2, "%s(), Starting IrTTP\n", __func__);
			self->rx_flow = FLOW_START;
			irttp_flow_request(self->tsap, FLOW_START);
		}
	}

	return copied;
}


static int irda_sendmsg_dgram(struct kiocb *iocb, struct socket *sock,
			      struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self;
	struct sk_buff *skb;
	int err;

	IRDA_DEBUG(4, "%s(), len=%zd\n", __func__, len);

	if (msg->msg_flags & ~(MSG_DONTWAIT|MSG_CMSG_COMPAT))
		return -EINVAL;

	if (sk->sk_shutdown & SEND_SHUTDOWN) {
		send_sig(SIGPIPE, current, 0);
		return -EPIPE;
	}

	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	self = irda_sk(sk);

	
	if (len > self->max_data_size) {
		IRDA_DEBUG(0, "%s(), Warning to much data! "
			   "Chopping frame from %zd to %d bytes!\n",
			   __func__, len, self->max_data_size);
		len = self->max_data_size;
	}

	skb = sock_alloc_send_skb(sk, len + self->max_header_size,
				  msg->msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		return -ENOBUFS;

	skb_reserve(skb, self->max_header_size);
	skb_reset_transport_header(skb);

	IRDA_DEBUG(4, "%s(), appending user data\n", __func__);
	skb_put(skb, len);
	err = memcpy_fromiovec(skb_transport_header(skb), msg->msg_iov, len);
	if (err) {
		kfree_skb(skb);
		return err;
	}

	
	err = irttp_udata_request(self->tsap, skb);
	if (err) {
		IRDA_DEBUG(0, "%s(), err=%d\n", __func__, err);
		return err;
	}
	return len;
}


#ifdef CONFIG_IRDA_ULTRA
static int irda_sendmsg_ultra(struct kiocb *iocb, struct socket *sock,
			      struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self;
	__u8 pid = 0;
	int bound = 0;
	struct sk_buff *skb;
	int err;

	IRDA_DEBUG(4, "%s(), len=%zd\n", __func__, len);

	if (msg->msg_flags & ~(MSG_DONTWAIT|MSG_CMSG_COMPAT))
		return -EINVAL;

	if (sk->sk_shutdown & SEND_SHUTDOWN) {
		send_sig(SIGPIPE, current, 0);
		return -EPIPE;
	}

	self = irda_sk(sk);

	
	if (msg->msg_name) {
		struct sockaddr_irda *addr = (struct sockaddr_irda *) msg->msg_name;
		
		if (msg->msg_namelen < sizeof(*addr))
			return -EINVAL;
		if (addr->sir_family != AF_IRDA)
			return -EINVAL;

		pid = addr->sir_lsap_sel;
		if (pid & 0x80) {
			IRDA_DEBUG(0, "%s(), extension in PID not supp!\n", __func__);
			return -EOPNOTSUPP;
		}
	} else {
		
		if ((self->lsap == NULL) ||
		    (sk->sk_state != TCP_ESTABLISHED)) {
			IRDA_DEBUG(0, "%s(), socket not bound to Ultra PID.\n",
				   __func__);
			return -ENOTCONN;
		}
		
		bound = 1;
	}

	
	if (len > self->max_data_size) {
		IRDA_DEBUG(0, "%s(), Warning to much data! "
			   "Chopping frame from %zd to %d bytes!\n",
			   __func__, len, self->max_data_size);
		len = self->max_data_size;
	}

	skb = sock_alloc_send_skb(sk, len + self->max_header_size,
				  msg->msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		return -ENOBUFS;

	skb_reserve(skb, self->max_header_size);
	skb_reset_transport_header(skb);

	IRDA_DEBUG(4, "%s(), appending user data\n", __func__);
	skb_put(skb, len);
	err = memcpy_fromiovec(skb_transport_header(skb), msg->msg_iov, len);
	if (err) {
		kfree_skb(skb);
		return err;
	}

	err = irlmp_connless_data_request((bound ? self->lsap : NULL),
					  skb, pid);
	if (err) {
		IRDA_DEBUG(0, "%s(), err=%d\n", __func__, err);
		return err;
	}
	return len;
}
#endif 


static int irda_shutdown(struct socket *sock, int how)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);

	IRDA_DEBUG(1, "%s(%p)\n", __func__, self);

	sk->sk_state       = TCP_CLOSE;
	sk->sk_shutdown   |= SEND_SHUTDOWN;
	sk->sk_state_change(sk);

	if (self->iriap) {
		iriap_close(self->iriap);
		self->iriap = NULL;
	}

	if (self->tsap) {
		irttp_disconnect_request(self->tsap, NULL, P_NORMAL);
		irttp_close_tsap(self->tsap);
		self->tsap = NULL;
	}

	
	self->rx_flow = self->tx_flow = FLOW_START;	
	self->daddr = DEV_ADDR_ANY;	
	self->saddr = 0x0;		

	return 0;
}


static unsigned int irda_poll(struct file * file, struct socket *sock,
			      poll_table *wait)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);
	unsigned int mask;

	IRDA_DEBUG(4, "%s()\n", __func__);

	poll_wait(file, sk->sk_sleep, wait);
	mask = 0;

	
	if (sk->sk_err)
		mask |= POLLERR;
	if (sk->sk_shutdown & RCV_SHUTDOWN) {
		IRDA_DEBUG(0, "%s(), POLLHUP\n", __func__);
		mask |= POLLHUP;
	}

	
	if (!skb_queue_empty(&sk->sk_receive_queue)) {
		IRDA_DEBUG(4, "Socket is readable\n");
		mask |= POLLIN | POLLRDNORM;
	}

	
	switch (sk->sk_type) {
	case SOCK_STREAM:
		if (sk->sk_state == TCP_CLOSE) {
			IRDA_DEBUG(0, "%s(), POLLHUP\n", __func__);
			mask |= POLLHUP;
		}

		if (sk->sk_state == TCP_ESTABLISHED) {
			if ((self->tx_flow == FLOW_START) &&
			    sock_writeable(sk))
			{
				mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
			}
		}
		break;
	case SOCK_SEQPACKET:
		if ((self->tx_flow == FLOW_START) &&
		    sock_writeable(sk))
		{
			mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
		}
		break;
	case SOCK_DGRAM:
		if (sock_writeable(sk))
			mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
		break;
	default:
		break;
	}
	return mask;
}


static int irda_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;

	IRDA_DEBUG(4, "%s(), cmd=%#x\n", __func__, cmd);

	switch (cmd) {
	case TIOCOUTQ: {
		long amount;

		amount = sk->sk_sndbuf - sk_wmem_alloc_get(sk);
		if (amount < 0)
			amount = 0;
		if (put_user(amount, (unsigned int __user *)arg))
			return -EFAULT;
		return 0;
	}

	case TIOCINQ: {
		struct sk_buff *skb;
		long amount = 0L;
		
		if ((skb = skb_peek(&sk->sk_receive_queue)) != NULL)
			amount = skb->len;
		if (put_user(amount, (unsigned int __user *)arg))
			return -EFAULT;
		return 0;
	}

	case SIOCGSTAMP:
		if (sk != NULL)
			return sock_get_timestamp(sk, (struct timeval __user *)arg);
		return -EINVAL;

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
		return -EINVAL;
	default:
		IRDA_DEBUG(1, "%s(), doing device ioctl!\n", __func__);
		return -ENOIOCTLCMD;
	}

	
	return 0;
}

#ifdef CONFIG_COMPAT

static int irda_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	
	return -ENOIOCTLCMD;
}
#endif


static int irda_setsockopt(struct socket *sock, int level, int optname,
			   char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);
	struct irda_ias_set    *ias_opt;
	struct ias_object      *ias_obj;
	struct ias_attrib *	ias_attr;	
	int opt, free_ias = 0;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	if (level != SOL_IRLMP)
		return -ENOPROTOOPT;

	switch (optname) {
	case IRLMP_IAS_SET:
		

		if (optlen != sizeof(struct irda_ias_set))
			return -EINVAL;

		ias_opt = kmalloc(sizeof(struct irda_ias_set), GFP_ATOMIC);
		if (ias_opt == NULL)
			return -ENOMEM;

		
		if (copy_from_user(ias_opt, optval, optlen)) {
			kfree(ias_opt);
			return -EFAULT;
		}

		
		if(ias_opt->irda_class_name[0] == '\0') {
			if(self->ias_obj == NULL) {
				kfree(ias_opt);
				return -EINVAL;
			}
			ias_obj = self->ias_obj;
		} else
			ias_obj = irias_find_object(ias_opt->irda_class_name);

		
		if((!capable(CAP_NET_ADMIN)) &&
		   ((ias_obj == NULL) || (ias_obj != self->ias_obj))) {
			kfree(ias_opt);
			return -EPERM;
		}

		
		if(ias_obj == (struct ias_object *) NULL) {
			
			ias_obj = irias_new_object(ias_opt->irda_class_name,
						   jiffies);
			if (ias_obj == NULL) {
				kfree(ias_opt);
				return -ENOMEM;
			}
			free_ias = 1;
		}

		
		if(irias_find_attrib(ias_obj, ias_opt->irda_attrib_name)) {
			kfree(ias_opt);
			if (free_ias) {
				kfree(ias_obj->name);
				kfree(ias_obj);
			}
			return -EINVAL;
		}

		
		switch(ias_opt->irda_attrib_type) {
		case IAS_INTEGER:
			
			irias_add_integer_attrib(
				ias_obj,
				ias_opt->irda_attrib_name,
				ias_opt->attribute.irda_attrib_int,
				IAS_USER_ATTR);
			break;
		case IAS_OCT_SEQ:
			
			if(ias_opt->attribute.irda_attrib_octet_seq.len >
			   IAS_MAX_OCTET_STRING) {
				kfree(ias_opt);
				if (free_ias) {
					kfree(ias_obj->name);
					kfree(ias_obj);
				}

				return -EINVAL;
			}
			
			irias_add_octseq_attrib(
			      ias_obj,
			      ias_opt->irda_attrib_name,
			      ias_opt->attribute.irda_attrib_octet_seq.octet_seq,
			      ias_opt->attribute.irda_attrib_octet_seq.len,
			      IAS_USER_ATTR);
			break;
		case IAS_STRING:
			
			
			
			
			ias_opt->attribute.irda_attrib_string.string[ias_opt->attribute.irda_attrib_string.len] = '\0';
			
			irias_add_string_attrib(
				ias_obj,
				ias_opt->irda_attrib_name,
				ias_opt->attribute.irda_attrib_string.string,
				IAS_USER_ATTR);
			break;
		default :
			kfree(ias_opt);
			if (free_ias) {
				kfree(ias_obj->name);
				kfree(ias_obj);
			}
			return -EINVAL;
		}
		irias_insert_object(ias_obj);
		kfree(ias_opt);
		break;
	case IRLMP_IAS_DEL:
		

		if (optlen != sizeof(struct irda_ias_set))
			return -EINVAL;

		ias_opt = kmalloc(sizeof(struct irda_ias_set), GFP_ATOMIC);
		if (ias_opt == NULL)
			return -ENOMEM;

		
		if (copy_from_user(ias_opt, optval, optlen)) {
			kfree(ias_opt);
			return -EFAULT;
		}

		
		if(ias_opt->irda_class_name[0] == '\0')
			ias_obj = self->ias_obj;
		else
			ias_obj = irias_find_object(ias_opt->irda_class_name);
		if(ias_obj == (struct ias_object *) NULL) {
			kfree(ias_opt);
			return -EINVAL;
		}

		
		if((!capable(CAP_NET_ADMIN)) &&
		   ((ias_obj == NULL) || (ias_obj != self->ias_obj))) {
			kfree(ias_opt);
			return -EPERM;
		}

		
		ias_attr = irias_find_attrib(ias_obj,
					     ias_opt->irda_attrib_name);
		if(ias_attr == (struct ias_attrib *) NULL) {
			kfree(ias_opt);
			return -EINVAL;
		}

		
		if(ias_attr->value->owner != IAS_USER_ATTR) {
			IRDA_DEBUG(1, "%s(), attempting to delete a kernel attribute\n", __func__);
			kfree(ias_opt);
			return -EPERM;
		}

		
		irias_delete_attrib(ias_obj, ias_attr, 1);
		kfree(ias_opt);
		break;
	case IRLMP_MAX_SDU_SIZE:
		if (optlen < sizeof(int))
			return -EINVAL;

		if (get_user(opt, (int __user *)optval))
			return -EFAULT;

		
		if (sk->sk_type != SOCK_SEQPACKET) {
			IRDA_DEBUG(2, "%s(), setting max_sdu_size = %d\n",
				   __func__, opt);
			self->max_sdu_size_rx = opt;
		} else {
			IRDA_WARNING("%s: not allowed to set MAXSDUSIZE for this socket type!\n",
				     __func__);
			return -ENOPROTOOPT;
		}
		break;
	case IRLMP_HINTS_SET:
		if (optlen < sizeof(int))
			return -EINVAL;

		
		if (get_user(opt, (int __user *)optval))
			return -EFAULT;

		
		if (self->skey)
			irlmp_unregister_service(self->skey);

		self->skey = irlmp_register_service((__u16) opt);
		break;
	case IRLMP_HINT_MASK_SET:
		
		if (optlen < sizeof(int))
			return -EINVAL;

		
		if (get_user(opt, (int __user *)optval))
			return -EFAULT;

		
		self->mask.word = (__u16) opt;
		
		self->mask.word &= 0x7f7f;
		
		if(!self->mask.word)
			self->mask.word = 0xFFFF;

		break;
	default:
		return -ENOPROTOOPT;
	}
	return 0;
}


static int irda_extract_ias_value(struct irda_ias_set *ias_opt,
				  struct ias_value *ias_value)
{
	
	switch (ias_value->type) {
	case IAS_INTEGER:
		
		ias_opt->attribute.irda_attrib_int = ias_value->t.integer;
		break;
	case IAS_OCT_SEQ:
		
		ias_opt->attribute.irda_attrib_octet_seq.len = ias_value->len;
		
		memcpy(ias_opt->attribute.irda_attrib_octet_seq.octet_seq,
		       ias_value->t.oct_seq, ias_value->len);
		break;
	case IAS_STRING:
		
		ias_opt->attribute.irda_attrib_string.len = ias_value->len;
		ias_opt->attribute.irda_attrib_string.charset = ias_value->charset;
		
		memcpy(ias_opt->attribute.irda_attrib_string.string,
		       ias_value->t.string, ias_value->len);
		
		ias_opt->attribute.irda_attrib_string.string[ias_value->len] = '\0';
		break;
	case IAS_MISSING:
	default :
		return -EINVAL;
	}

	
	ias_opt->irda_attrib_type = ias_value->type;

	return 0;
}


static int irda_getsockopt(struct socket *sock, int level, int optname,
			   char __user *optval, int __user *optlen)
{
	struct sock *sk = sock->sk;
	struct irda_sock *self = irda_sk(sk);
	struct irda_device_list list;
	struct irda_device_info *discoveries;
	struct irda_ias_set *	ias_opt;	
	struct ias_object *	ias_obj;	
	struct ias_attrib *	ias_attr;	
	int daddr = DEV_ADDR_ANY;	
	int val = 0;
	int len = 0;
	int err;
	int offset, total;

	IRDA_DEBUG(2, "%s(%p)\n", __func__, self);

	if (level != SOL_IRLMP)
		return -ENOPROTOOPT;

	if (get_user(len, optlen))
		return -EFAULT;

	if(len < 0)
		return -EINVAL;

	switch (optname) {
	case IRLMP_ENUMDEVICES:
		
		discoveries = irlmp_get_discoveries(&list.len, self->mask.word,
						    self->nslots);
		
		if (discoveries == NULL)
			return -EAGAIN;		
		err = 0;

		
		if (copy_to_user(optval, &list,
				 sizeof(struct irda_device_list) -
				 sizeof(struct irda_device_info)))
			err = -EFAULT;

		
		offset = sizeof(struct irda_device_list) -
			sizeof(struct irda_device_info);

		
		if(list.len > 2048)
		{
			err = -EINVAL;
			goto bed;
		}
		total = offset + (list.len * sizeof(struct irda_device_info));
		if (total > len)
			total = len;
		if (copy_to_user(optval+offset, discoveries, total - offset))
			err = -EFAULT;

		
		if (put_user(total, optlen))
			err = -EFAULT;
bed:
		
		kfree(discoveries);
		if (err)
			return err;
		break;
	case IRLMP_MAX_SDU_SIZE:
		val = self->max_data_size;
		len = sizeof(int);
		if (put_user(len, optlen))
			return -EFAULT;

		if (copy_to_user(optval, &val, len))
			return -EFAULT;
		break;
	case IRLMP_IAS_GET:
		

		
		if (len != sizeof(struct irda_ias_set))
			return -EINVAL;

		ias_opt = kmalloc(sizeof(struct irda_ias_set), GFP_ATOMIC);
		if (ias_opt == NULL)
			return -ENOMEM;

		
		if (copy_from_user(ias_opt, optval, len)) {
			kfree(ias_opt);
			return -EFAULT;
		}

		
		if(ias_opt->irda_class_name[0] == '\0')
			ias_obj = self->ias_obj;
		else
			ias_obj = irias_find_object(ias_opt->irda_class_name);
		if(ias_obj == (struct ias_object *) NULL) {
			kfree(ias_opt);
			return -EINVAL;
		}

		
		ias_attr = irias_find_attrib(ias_obj,
					     ias_opt->irda_attrib_name);
		if(ias_attr == (struct ias_attrib *) NULL) {
			kfree(ias_opt);
			return -EINVAL;
		}

		
		err = irda_extract_ias_value(ias_opt, ias_attr->value);
		if(err) {
			kfree(ias_opt);
			return err;
		}

		
		if (copy_to_user(optval, ias_opt,
				 sizeof(struct irda_ias_set))) {
			kfree(ias_opt);
			return -EFAULT;
		}
		
		kfree(ias_opt);
		break;
	case IRLMP_IAS_QUERY:
		

		
		if (len != sizeof(struct irda_ias_set))
			return -EINVAL;

		ias_opt = kmalloc(sizeof(struct irda_ias_set), GFP_ATOMIC);
		if (ias_opt == NULL)
			return -ENOMEM;

		
		if (copy_from_user(ias_opt, optval, len)) {
			kfree(ias_opt);
			return -EFAULT;
		}

		
		if(self->daddr != DEV_ADDR_ANY) {
			
			daddr = self->daddr;
		} else {
			
			daddr = ias_opt->daddr;
			if((!daddr) || (daddr == DEV_ADDR_ANY)) {
				kfree(ias_opt);
				return -EINVAL;
			}
		}

		
		if (self->iriap) {
			IRDA_WARNING("%s: busy with a previous query\n",
				     __func__);
			kfree(ias_opt);
			return -EBUSY;
		}

		self->iriap = iriap_open(LSAP_ANY, IAS_CLIENT, self,
					 irda_getvalue_confirm);

		if (self->iriap == NULL) {
			kfree(ias_opt);
			return -ENOMEM;
		}

		
		self->errno = -EHOSTUNREACH;

		
		iriap_getvaluebyclass_request(self->iriap,
					      self->saddr, daddr,
					      ias_opt->irda_class_name,
					      ias_opt->irda_attrib_name);

		
		if (wait_event_interruptible(self->query_wait,
					     (self->iriap == NULL))) {
			
			kfree(ias_opt);
			
			return -EHOSTUNREACH;
		}

		
		if (self->errno)
		{
			kfree(ias_opt);
			
			if((self->errno == IAS_CLASS_UNKNOWN) ||
			   (self->errno == IAS_ATTRIB_UNKNOWN))
				return (-EADDRNOTAVAIL);
			else
				return (-EHOSTUNREACH);
		}

		
		err = irda_extract_ias_value(ias_opt, self->ias_result);
		if (self->ias_result)
			irias_delete_value(self->ias_result);
		if (err) {
			kfree(ias_opt);
			return err;
		}

		
		if (copy_to_user(optval, ias_opt,
				 sizeof(struct irda_ias_set))) {
			kfree(ias_opt);
			return -EFAULT;
		}
		
		kfree(ias_opt);
		break;
	case IRLMP_WAITDEVICE:
		

		
		if (len != sizeof(int))
			return -EINVAL;
		
		if (get_user(val, (int __user *)optval))
			return -EFAULT;

		
		irlmp_update_client(self->ckey, self->mask.word,
				    irda_selective_discovery_indication,
				    NULL, (void *) self);

		
		irlmp_discovery_request(self->nslots);

		
		if (!self->cachedaddr) {
			int ret = 0;

			IRDA_DEBUG(1, "%s(), nothing discovered yet, going to sleep...\n", __func__);

			
			self->errno = 0;
			setup_timer(&self->watchdog, irda_discovery_timeout,
					(unsigned long)self);
			self->watchdog.expires = jiffies + (val * HZ/1000);
			add_timer(&(self->watchdog));

			
			__wait_event_interruptible(self->query_wait,
			      (self->cachedaddr != 0 || self->errno == -ETIME),
						   ret);

			
			if(timer_pending(&(self->watchdog)))
				del_timer(&(self->watchdog));

			IRDA_DEBUG(1, "%s(), ...waking up !\n", __func__);

			if (ret != 0)
				return ret;
		}
		else
			IRDA_DEBUG(1, "%s(), found immediately !\n",
				   __func__);

		
		irlmp_update_client(self->ckey, self->mask.word,
				    NULL, NULL, NULL);

		
		if (!self->cachedaddr)
			return -EAGAIN;		
		daddr = self->cachedaddr;
		
		self->cachedaddr = 0;

		
		if (put_user(daddr, (int __user *)optval))
			return -EFAULT;

		break;
	default:
		return -ENOPROTOOPT;
	}

	return 0;
}

static struct net_proto_family irda_family_ops = {
	.family = PF_IRDA,
	.create = irda_create,
	.owner	= THIS_MODULE,
};

static const struct proto_ops SOCKOPS_WRAPPED(irda_stream_ops) = {
	.family =	PF_IRDA,
	.owner =	THIS_MODULE,
	.release =	irda_release,
	.bind =		irda_bind,
	.connect =	irda_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	irda_accept,
	.getname =	irda_getname,
	.poll =		irda_poll,
	.ioctl =	irda_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	irda_compat_ioctl,
#endif
	.listen =	irda_listen,
	.shutdown =	irda_shutdown,
	.setsockopt =	irda_setsockopt,
	.getsockopt =	irda_getsockopt,
	.sendmsg =	irda_sendmsg,
	.recvmsg =	irda_recvmsg_stream,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct proto_ops SOCKOPS_WRAPPED(irda_seqpacket_ops) = {
	.family =	PF_IRDA,
	.owner =	THIS_MODULE,
	.release =	irda_release,
	.bind =		irda_bind,
	.connect =	irda_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	irda_accept,
	.getname =	irda_getname,
	.poll =		datagram_poll,
	.ioctl =	irda_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	irda_compat_ioctl,
#endif
	.listen =	irda_listen,
	.shutdown =	irda_shutdown,
	.setsockopt =	irda_setsockopt,
	.getsockopt =	irda_getsockopt,
	.sendmsg =	irda_sendmsg,
	.recvmsg =	irda_recvmsg_dgram,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct proto_ops SOCKOPS_WRAPPED(irda_dgram_ops) = {
	.family =	PF_IRDA,
	.owner =	THIS_MODULE,
	.release =	irda_release,
	.bind =		irda_bind,
	.connect =	irda_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	irda_accept,
	.getname =	irda_getname,
	.poll =		datagram_poll,
	.ioctl =	irda_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	irda_compat_ioctl,
#endif
	.listen =	irda_listen,
	.shutdown =	irda_shutdown,
	.setsockopt =	irda_setsockopt,
	.getsockopt =	irda_getsockopt,
	.sendmsg =	irda_sendmsg_dgram,
	.recvmsg =	irda_recvmsg_dgram,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

#ifdef CONFIG_IRDA_ULTRA
static const struct proto_ops SOCKOPS_WRAPPED(irda_ultra_ops) = {
	.family =	PF_IRDA,
	.owner =	THIS_MODULE,
	.release =	irda_release,
	.bind =		irda_bind,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	irda_getname,
	.poll =		datagram_poll,
	.ioctl =	irda_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	irda_compat_ioctl,
#endif
	.listen =	sock_no_listen,
	.shutdown =	irda_shutdown,
	.setsockopt =	irda_setsockopt,
	.getsockopt =	irda_getsockopt,
	.sendmsg =	irda_sendmsg_ultra,
	.recvmsg =	irda_recvmsg_dgram,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};
#endif 

SOCKOPS_WRAP(irda_stream, PF_IRDA);
SOCKOPS_WRAP(irda_seqpacket, PF_IRDA);
SOCKOPS_WRAP(irda_dgram, PF_IRDA);
#ifdef CONFIG_IRDA_ULTRA
SOCKOPS_WRAP(irda_ultra, PF_IRDA);
#endif 


int __init irsock_init(void)
{
	int rc = proto_register(&irda_proto, 0);

	if (rc == 0)
		rc = sock_register(&irda_family_ops);

	return rc;
}


void irsock_cleanup(void)
{
	sock_unregister(PF_IRDA);
	proto_unregister(&irda_proto);
}
