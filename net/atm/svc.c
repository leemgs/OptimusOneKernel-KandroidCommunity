




#include <linux/string.h>
#include <linux/net.h>		
#include <linux/errno.h>	
#include <linux/kernel.h>	
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/sched.h>	
#include <linux/fcntl.h>	
#include <linux/init.h>
#include <linux/atm.h>		
#include <linux/atmsap.h>
#include <linux/atmsvc.h>
#include <linux/atmdev.h>
#include <linux/bitops.h>
#include <net/sock.h>		
#include <asm/uaccess.h>

#include "resources.h"
#include "common.h"		
#include "signaling.h"
#include "addr.h"

static int svc_create(struct net *net, struct socket *sock,int protocol);




static int svc_shutdown(struct socket *sock,int how)
{
	return 0;
}


static void svc_disconnect(struct atm_vcc *vcc)
{
	DEFINE_WAIT(wait);
	struct sk_buff *skb;
	struct sock *sk = sk_atm(vcc);

	pr_debug("svc_disconnect %p\n",vcc);
	if (test_bit(ATM_VF_REGIS,&vcc->flags)) {
		prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
		sigd_enq(vcc,as_close,NULL,NULL,NULL);
		while (!test_bit(ATM_VF_RELEASED,&vcc->flags) && sigd) {
			schedule();
			prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
		}
		finish_wait(sk->sk_sleep, &wait);
	}
	
	while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
		atm_return(vcc, skb->truesize);
		pr_debug("LISTEN REL\n");
		sigd_enq2(NULL,as_reject,vcc,NULL,NULL,&vcc->qos,0);
		dev_kfree_skb(skb);
	}
	clear_bit(ATM_VF_REGIS, &vcc->flags);
	
}


static int svc_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct atm_vcc *vcc;

	if (sk)  {
		vcc = ATM_SD(sock);
		pr_debug("svc_release %p\n", vcc);
		clear_bit(ATM_VF_READY, &vcc->flags);
		
		svc_disconnect(vcc);
		vcc_release(sock);
	}
	return 0;
}


static int svc_bind(struct socket *sock,struct sockaddr *sockaddr,
    int sockaddr_len)
{
	DEFINE_WAIT(wait);
	struct sock *sk = sock->sk;
	struct sockaddr_atmsvc *addr;
	struct atm_vcc *vcc;
	int error;

	if (sockaddr_len != sizeof(struct sockaddr_atmsvc))
		return -EINVAL;
	lock_sock(sk);
	if (sock->state == SS_CONNECTED) {
		error = -EISCONN;
		goto out;
	}
	if (sock->state != SS_UNCONNECTED) {
		error = -EINVAL;
		goto out;
	}
	vcc = ATM_SD(sock);
	addr = (struct sockaddr_atmsvc *) sockaddr;
	if (addr->sas_family != AF_ATMSVC) {
		error = -EAFNOSUPPORT;
		goto out;
	}
	clear_bit(ATM_VF_BOUND,&vcc->flags);
	    
	
	if (!test_bit(ATM_VF_HASQOS,&vcc->flags)) {
		error = -EBADFD;
		goto out;
	}
	vcc->local = *addr;
	set_bit(ATM_VF_WAITING, &vcc->flags);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	sigd_enq(vcc,as_bind,NULL,NULL,&vcc->local);
	while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
		schedule();
		prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	}
	finish_wait(sk->sk_sleep, &wait);
	clear_bit(ATM_VF_REGIS,&vcc->flags); 
	if (!sigd) {
		error = -EUNATCH;
		goto out;
	}
	if (!sk->sk_err)
		set_bit(ATM_VF_BOUND,&vcc->flags);
	error = -sk->sk_err;
out:
	release_sock(sk);
	return error;
}


static int svc_connect(struct socket *sock,struct sockaddr *sockaddr,
    int sockaddr_len,int flags)
{
	DEFINE_WAIT(wait);
	struct sock *sk = sock->sk;
	struct sockaddr_atmsvc *addr;
	struct atm_vcc *vcc = ATM_SD(sock);
	int error;

	pr_debug("svc_connect %p\n",vcc);
	lock_sock(sk);
	if (sockaddr_len != sizeof(struct sockaddr_atmsvc)) {
		error = -EINVAL;
		goto out;
	}

	switch (sock->state) {
	default:
		error = -EINVAL;
		goto out;
	case SS_CONNECTED:
		error = -EISCONN;
		goto out;
	case SS_CONNECTING:
		if (test_bit(ATM_VF_WAITING, &vcc->flags)) {
			error = -EALREADY;
			goto out;
		}
		sock->state = SS_UNCONNECTED;
		if (sk->sk_err) {
			error = -sk->sk_err;
			goto out;
		}
		break;
	case SS_UNCONNECTED:
		addr = (struct sockaddr_atmsvc *) sockaddr;
		if (addr->sas_family != AF_ATMSVC) {
			error = -EAFNOSUPPORT;
			goto out;
		}
		if (!test_bit(ATM_VF_HASQOS, &vcc->flags)) {
			error = -EBADFD;
			goto out;
		}
		if (vcc->qos.txtp.traffic_class == ATM_ANYCLASS ||
		    vcc->qos.rxtp.traffic_class == ATM_ANYCLASS) {
			error = -EINVAL;
			goto out;
		}
		if (!vcc->qos.txtp.traffic_class &&
		    !vcc->qos.rxtp.traffic_class) {
			error = -EINVAL;
			goto out;
		}
		vcc->remote = *addr;
		set_bit(ATM_VF_WAITING, &vcc->flags);
		prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
		sigd_enq(vcc,as_connect,NULL,NULL,&vcc->remote);
		if (flags & O_NONBLOCK) {
			finish_wait(sk->sk_sleep, &wait);
			sock->state = SS_CONNECTING;
			error = -EINPROGRESS;
			goto out;
		}
		error = 0;
		while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
			schedule();
			if (!signal_pending(current)) {
				prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
				continue;
			}
			pr_debug("*ABORT*\n");
			
			sigd_enq(vcc,as_close,NULL,NULL,NULL);
			while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
				prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
				schedule();
			}
			if (!sk->sk_err)
				while (!test_bit(ATM_VF_RELEASED,&vcc->flags)
				    && sigd) {
					prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
					schedule();
				}
			clear_bit(ATM_VF_REGIS,&vcc->flags);
			clear_bit(ATM_VF_RELEASED,&vcc->flags);
			clear_bit(ATM_VF_CLOSE,&vcc->flags);
			    
			error = -EINTR;
			break;
		}
		finish_wait(sk->sk_sleep, &wait);
		if (error)
			goto out;
		if (!sigd) {
			error = -EUNATCH;
			goto out;
		}
		if (sk->sk_err) {
			error = -sk->sk_err;
			goto out;
		}
	}

	vcc->qos.txtp.max_pcr = SELECT_TOP_PCR(vcc->qos.txtp);
	vcc->qos.txtp.pcr = 0;
	vcc->qos.txtp.min_pcr = 0;

	if (!(error = vcc_connect(sock, vcc->itf, vcc->vpi, vcc->vci)))
		sock->state = SS_CONNECTED;
	else
		(void) svc_disconnect(vcc);
out:
	release_sock(sk);
	return error;
}


static int svc_listen(struct socket *sock,int backlog)
{
	DEFINE_WAIT(wait);
	struct sock *sk = sock->sk;
	struct atm_vcc *vcc = ATM_SD(sock);
	int error;

	pr_debug("svc_listen %p\n",vcc);
	lock_sock(sk);
	
	if (test_bit(ATM_VF_SESSION,&vcc->flags)) {
		error = -EINVAL;
		goto out;
	}
	if (test_bit(ATM_VF_LISTEN, &vcc->flags)) {
		error = -EADDRINUSE;
		goto out;
        }
	set_bit(ATM_VF_WAITING, &vcc->flags);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	sigd_enq(vcc,as_listen,NULL,NULL,&vcc->local);
	while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
		schedule();
		prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	}
	finish_wait(sk->sk_sleep, &wait);
	if (!sigd) {
		error = -EUNATCH;
		goto out;
	}
	set_bit(ATM_VF_LISTEN,&vcc->flags);
	vcc_insert_socket(sk);
	sk->sk_max_ack_backlog = backlog > 0 ? backlog : ATM_BACKLOG_DEFAULT;
	error = -sk->sk_err;
out:
	release_sock(sk);
	return error;
}


static int svc_accept(struct socket *sock,struct socket *newsock,int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	struct atmsvc_msg *msg;
	struct atm_vcc *old_vcc = ATM_SD(sock);
	struct atm_vcc *new_vcc;
	int error;

	lock_sock(sk);

	error = svc_create(sock_net(sk), newsock,0);
	if (error)
		goto out;

	new_vcc = ATM_SD(newsock);

	pr_debug("svc_accept %p -> %p\n",old_vcc,new_vcc);
	while (1) {
		DEFINE_WAIT(wait);

		prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
		while (!(skb = skb_dequeue(&sk->sk_receive_queue)) &&
		       sigd) {
			if (test_bit(ATM_VF_RELEASED,&old_vcc->flags)) break;
			if (test_bit(ATM_VF_CLOSE,&old_vcc->flags)) {
				error = -sk->sk_err;
				break;
			}
			if (flags & O_NONBLOCK) {
				error = -EAGAIN;
				break;
			}
			release_sock(sk);
			schedule();
			lock_sock(sk);
			if (signal_pending(current)) {
				error = -ERESTARTSYS;
				break;
			}
			prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
		}
		finish_wait(sk->sk_sleep, &wait);
		if (error)
			goto out;
		if (!skb) {
			error = -EUNATCH;
			goto out;
		}
		msg = (struct atmsvc_msg *) skb->data;
		new_vcc->qos = msg->qos;
		set_bit(ATM_VF_HASQOS,&new_vcc->flags);
		new_vcc->remote = msg->svc;
		new_vcc->local = msg->local;
		new_vcc->sap = msg->sap;
		error = vcc_connect(newsock, msg->pvc.sap_addr.itf,
				    msg->pvc.sap_addr.vpi, msg->pvc.sap_addr.vci);
		dev_kfree_skb(skb);
		sk->sk_ack_backlog--;
		if (error) {
			sigd_enq2(NULL,as_reject,old_vcc,NULL,NULL,
			    &old_vcc->qos,error);
			error = error == -EAGAIN ? -EBUSY : error;
			goto out;
		}
		
		set_bit(ATM_VF_WAITING, &new_vcc->flags);
		prepare_to_wait(sk_atm(new_vcc)->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
		sigd_enq(new_vcc,as_accept,old_vcc,NULL,NULL);
		while (test_bit(ATM_VF_WAITING, &new_vcc->flags) && sigd) {
			release_sock(sk);
			schedule();
			lock_sock(sk);
			prepare_to_wait(sk_atm(new_vcc)->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
		}
		finish_wait(sk_atm(new_vcc)->sk_sleep, &wait);
		if (!sigd) {
			error = -EUNATCH;
			goto out;
		}
		if (!sk_atm(new_vcc)->sk_err)
			break;
		if (sk_atm(new_vcc)->sk_err != ERESTARTSYS) {
			error = -sk_atm(new_vcc)->sk_err;
			goto out;
		}
	}
	newsock->state = SS_CONNECTED;
out:
	release_sock(sk);
	return error;
}


static int svc_getname(struct socket *sock,struct sockaddr *sockaddr,
    int *sockaddr_len,int peer)
{
	struct sockaddr_atmsvc *addr;

	*sockaddr_len = sizeof(struct sockaddr_atmsvc);
	addr = (struct sockaddr_atmsvc *) sockaddr;
	memcpy(addr,peer ? &ATM_SD(sock)->remote : &ATM_SD(sock)->local,
	    sizeof(struct sockaddr_atmsvc));
	return 0;
}


int svc_change_qos(struct atm_vcc *vcc,struct atm_qos *qos)
{
	struct sock *sk = sk_atm(vcc);
	DEFINE_WAIT(wait);

	set_bit(ATM_VF_WAITING, &vcc->flags);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	sigd_enq2(vcc,as_modify,NULL,NULL,&vcc->local,qos,0);
	while (test_bit(ATM_VF_WAITING, &vcc->flags) &&
	       !test_bit(ATM_VF_RELEASED, &vcc->flags) && sigd) {
		schedule();
		prepare_to_wait(sk->sk_sleep, &wait, TASK_UNINTERRUPTIBLE);
	}
	finish_wait(sk->sk_sleep, &wait);
	if (!sigd) return -EUNATCH;
	return -sk->sk_err;
}


static int svc_setsockopt(struct socket *sock, int level, int optname,
			  char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct atm_vcc *vcc = ATM_SD(sock);
	int value, error = 0;

	lock_sock(sk);
	switch (optname) {
		case SO_ATMSAP:
			if (level != SOL_ATM || optlen != sizeof(struct atm_sap)) {
				error = -EINVAL;
				goto out;
			}
			if (copy_from_user(&vcc->sap, optval, optlen)) {
				error = -EFAULT;
				goto out;
			}
			set_bit(ATM_VF_HASSAP, &vcc->flags);
			break;
		case SO_MULTIPOINT:
			if (level != SOL_ATM || optlen != sizeof(int)) {
				error = -EINVAL;
				goto out;
			}
			if (get_user(value, (int __user *) optval)) {
				error = -EFAULT;
				goto out;
			}
			if (value == 1) {
				set_bit(ATM_VF_SESSION, &vcc->flags);
			} else if (value == 0) {
				clear_bit(ATM_VF_SESSION, &vcc->flags);
			} else {
				error = -EINVAL;
			}
			break;
		default:
			error = vcc_setsockopt(sock, level, optname,
					       optval, optlen);
	}

out:
	release_sock(sk);
	return error;
}


static int svc_getsockopt(struct socket *sock,int level,int optname,
    char __user *optval,int __user *optlen)
{
	struct sock *sk = sock->sk;
	int error = 0, len;

	lock_sock(sk);
	if (!__SO_LEVEL_MATCH(optname, level) || optname != SO_ATMSAP) {
		error = vcc_getsockopt(sock, level, optname, optval, optlen);
		goto out;
	}
	if (get_user(len, optlen)) {
		error = -EFAULT;
		goto out;
	}
	if (len != sizeof(struct atm_sap)) {
		error = -EINVAL;
		goto out;
	}
	if (copy_to_user(optval, &ATM_SD(sock)->sap, sizeof(struct atm_sap))) {
		error = -EFAULT;
		goto out;
	}
out:
	release_sock(sk);
	return error;
}


static int svc_addparty(struct socket *sock, struct sockaddr *sockaddr,
			int sockaddr_len, int flags)
{
	DEFINE_WAIT(wait);
	struct sock *sk = sock->sk;
	struct atm_vcc *vcc = ATM_SD(sock);
	int error;

	lock_sock(sk);
	set_bit(ATM_VF_WAITING, &vcc->flags);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
	sigd_enq(vcc, as_addparty, NULL, NULL,
		 (struct sockaddr_atmsvc *) sockaddr);
	if (flags & O_NONBLOCK) {
		finish_wait(sk->sk_sleep, &wait);
		error = -EINPROGRESS;
		goto out;
	}
	pr_debug("svc_addparty added wait queue\n");
	while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
		schedule();
		prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
	}
	finish_wait(sk->sk_sleep, &wait);
	error = xchg(&sk->sk_err_soft, 0);
out:
	release_sock(sk);
	return error;
}


static int svc_dropparty(struct socket *sock, int ep_ref)
{
	DEFINE_WAIT(wait);
	struct sock *sk = sock->sk;
	struct atm_vcc *vcc = ATM_SD(sock);
	int error;

	lock_sock(sk);
	set_bit(ATM_VF_WAITING, &vcc->flags);
	prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
	sigd_enq2(vcc, as_dropparty, NULL, NULL, NULL, NULL, ep_ref);
	while (test_bit(ATM_VF_WAITING, &vcc->flags) && sigd) {
		schedule();
		prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);
	}
	finish_wait(sk->sk_sleep, &wait);
	if (!sigd) {
		error = -EUNATCH;
		goto out;
	}
	error = xchg(&sk->sk_err_soft, 0);
out:
	release_sock(sk);
	return error;
}


static int svc_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	int error, ep_ref;
	struct sockaddr_atmsvc sa;
	struct atm_vcc *vcc = ATM_SD(sock);

	switch (cmd) {
		case ATM_ADDPARTY:
			if (!test_bit(ATM_VF_SESSION, &vcc->flags))
				return -EINVAL;
			if (copy_from_user(&sa, (void __user *) arg, sizeof(sa)))
				return -EFAULT;
			error = svc_addparty(sock, (struct sockaddr *) &sa, sizeof(sa), 0);
			break;
		case ATM_DROPPARTY:
			if (!test_bit(ATM_VF_SESSION, &vcc->flags))
				return -EINVAL;
			if (copy_from_user(&ep_ref, (void __user *) arg, sizeof(int)))
				return -EFAULT;
			error = svc_dropparty(sock, ep_ref);
			break;
		default:
			error = vcc_ioctl(sock, cmd, arg);
	}

	return error;
}

#ifdef CONFIG_COMPAT
static int svc_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	
	if (cmd == COMPAT_ATM_ADDPARTY)
		cmd = ATM_ADDPARTY;

	if (cmd == ATM_ADDPARTY || cmd == ATM_DROPPARTY)
		return svc_ioctl(sock, cmd, arg);
	else
		return vcc_compat_ioctl(sock, cmd, arg);
}
#endif 

static const struct proto_ops svc_proto_ops = {
	.family =	PF_ATMSVC,
	.owner =	THIS_MODULE,

	.release =	svc_release,
	.bind =		svc_bind,
	.connect =	svc_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	svc_accept,
	.getname =	svc_getname,
	.poll =		vcc_poll,
	.ioctl =	svc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	svc_compat_ioctl,
#endif
	.listen =	svc_listen,
	.shutdown =	svc_shutdown,
	.setsockopt =	svc_setsockopt,
	.getsockopt =	svc_getsockopt,
	.sendmsg =	vcc_sendmsg,
	.recvmsg =	vcc_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};


static int svc_create(struct net *net, struct socket *sock,int protocol)
{
	int error;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	sock->ops = &svc_proto_ops;
	error = vcc_create(net, sock, protocol, AF_ATMSVC);
	if (error) return error;
	ATM_SD(sock)->local.sas_family = AF_ATMSVC;
	ATM_SD(sock)->remote.sas_family = AF_ATMSVC;
	return 0;
}


static struct net_proto_family svc_family_ops = {
	.family = PF_ATMSVC,
	.create = svc_create,
	.owner = THIS_MODULE,
};




int __init atmsvc_init(void)
{
	return sock_register(&svc_family_ops);
}

void atmsvc_exit(void)
{
	sock_unregister(PF_ATMSVC);
}
