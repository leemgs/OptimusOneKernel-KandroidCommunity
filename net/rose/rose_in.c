
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <net/ax25.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/tcp_states.h>
#include <asm/system.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <net/rose.h>


static int rose_state1_machine(struct sock *sk, struct sk_buff *skb, int frametype)
{
	struct rose_sock *rose = rose_sk(sk);

	switch (frametype) {
	case ROSE_CALL_ACCEPTED:
		rose_stop_timer(sk);
		rose_start_idletimer(sk);
		rose->condition = 0x00;
		rose->vs        = 0;
		rose->va        = 0;
		rose->vr        = 0;
		rose->vl        = 0;
		rose->state     = ROSE_STATE_3;
		sk->sk_state	= TCP_ESTABLISHED;
		if (!sock_flag(sk, SOCK_DEAD))
			sk->sk_state_change(sk);
		break;

	case ROSE_CLEAR_REQUEST:
		rose_write_internal(sk, ROSE_CLEAR_CONFIRMATION);
		rose_disconnect(sk, ECONNREFUSED, skb->data[3], skb->data[4]);
		rose->neighbour->use--;
		break;

	default:
		break;
	}

	return 0;
}


static int rose_state2_machine(struct sock *sk, struct sk_buff *skb, int frametype)
{
	struct rose_sock *rose = rose_sk(sk);

	switch (frametype) {
	case ROSE_CLEAR_REQUEST:
		rose_write_internal(sk, ROSE_CLEAR_CONFIRMATION);
		rose_disconnect(sk, 0, skb->data[3], skb->data[4]);
		rose->neighbour->use--;
		break;

	case ROSE_CLEAR_CONFIRMATION:
		rose_disconnect(sk, 0, -1, -1);
		rose->neighbour->use--;
		break;

	default:
		break;
	}

	return 0;
}


static int rose_state3_machine(struct sock *sk, struct sk_buff *skb, int frametype, int ns, int nr, int q, int d, int m)
{
	struct rose_sock *rose = rose_sk(sk);
	int queued = 0;

	switch (frametype) {
	case ROSE_RESET_REQUEST:
		rose_stop_timer(sk);
		rose_start_idletimer(sk);
		rose_write_internal(sk, ROSE_RESET_CONFIRMATION);
		rose->condition = 0x00;
		rose->vs        = 0;
		rose->vr        = 0;
		rose->va        = 0;
		rose->vl        = 0;
		rose_requeue_frames(sk);
		break;

	case ROSE_CLEAR_REQUEST:
		rose_write_internal(sk, ROSE_CLEAR_CONFIRMATION);
		rose_disconnect(sk, 0, skb->data[3], skb->data[4]);
		rose->neighbour->use--;
		break;

	case ROSE_RR:
	case ROSE_RNR:
		if (!rose_validate_nr(sk, nr)) {
			rose_write_internal(sk, ROSE_RESET_REQUEST);
			rose->condition = 0x00;
			rose->vs        = 0;
			rose->vr        = 0;
			rose->va        = 0;
			rose->vl        = 0;
			rose->state     = ROSE_STATE_4;
			rose_start_t2timer(sk);
			rose_stop_idletimer(sk);
		} else {
			rose_frames_acked(sk, nr);
			if (frametype == ROSE_RNR) {
				rose->condition |= ROSE_COND_PEER_RX_BUSY;
			} else {
				rose->condition &= ~ROSE_COND_PEER_RX_BUSY;
			}
		}
		break;

	case ROSE_DATA:	
		rose->condition &= ~ROSE_COND_PEER_RX_BUSY;
		if (!rose_validate_nr(sk, nr)) {
			rose_write_internal(sk, ROSE_RESET_REQUEST);
			rose->condition = 0x00;
			rose->vs        = 0;
			rose->vr        = 0;
			rose->va        = 0;
			rose->vl        = 0;
			rose->state     = ROSE_STATE_4;
			rose_start_t2timer(sk);
			rose_stop_idletimer(sk);
			break;
		}
		rose_frames_acked(sk, nr);
		if (ns == rose->vr) {
			rose_start_idletimer(sk);
			if (sock_queue_rcv_skb(sk, skb) == 0) {
				rose->vr = (rose->vr + 1) % ROSE_MODULUS;
				queued = 1;
			} else {
				
				rose_write_internal(sk, ROSE_RESET_REQUEST);
				rose->condition = 0x00;
				rose->vs        = 0;
				rose->vr        = 0;
				rose->va        = 0;
				rose->vl        = 0;
				rose->state     = ROSE_STATE_4;
				rose_start_t2timer(sk);
				rose_stop_idletimer(sk);
				break;
			}
			if (atomic_read(&sk->sk_rmem_alloc) >
			    (sk->sk_rcvbuf >> 1))
				rose->condition |= ROSE_COND_OWN_RX_BUSY;
		}
		
		if (((rose->vl + sysctl_rose_window_size) % ROSE_MODULUS) == rose->vr) {
			rose->condition &= ~ROSE_COND_ACK_PENDING;
			rose_stop_timer(sk);
			rose_enquiry_response(sk);
		} else {
			rose->condition |= ROSE_COND_ACK_PENDING;
			rose_start_hbtimer(sk);
		}
		break;

	default:
		printk(KERN_WARNING "ROSE: unknown %02X in state 3\n", frametype);
		break;
	}

	return queued;
}


static int rose_state4_machine(struct sock *sk, struct sk_buff *skb, int frametype)
{
	struct rose_sock *rose = rose_sk(sk);

	switch (frametype) {
	case ROSE_RESET_REQUEST:
		rose_write_internal(sk, ROSE_RESET_CONFIRMATION);
	case ROSE_RESET_CONFIRMATION:
		rose_stop_timer(sk);
		rose_start_idletimer(sk);
		rose->condition = 0x00;
		rose->va        = 0;
		rose->vr        = 0;
		rose->vs        = 0;
		rose->vl        = 0;
		rose->state     = ROSE_STATE_3;
		rose_requeue_frames(sk);
		break;

	case ROSE_CLEAR_REQUEST:
		rose_write_internal(sk, ROSE_CLEAR_CONFIRMATION);
		rose_disconnect(sk, 0, skb->data[3], skb->data[4]);
		rose->neighbour->use--;
		break;

	default:
		break;
	}

	return 0;
}


static int rose_state5_machine(struct sock *sk, struct sk_buff *skb, int frametype)
{
	if (frametype == ROSE_CLEAR_REQUEST) {
		rose_write_internal(sk, ROSE_CLEAR_CONFIRMATION);
		rose_disconnect(sk, 0, skb->data[3], skb->data[4]);
		rose_sk(sk)->neighbour->use--;
	}

	return 0;
}


int rose_process_rx_frame(struct sock *sk, struct sk_buff *skb)
{
	struct rose_sock *rose = rose_sk(sk);
	int queued = 0, frametype, ns, nr, q, d, m;

	if (rose->state == ROSE_STATE_0)
		return 0;

	frametype = rose_decode(skb, &ns, &nr, &q, &d, &m);

	switch (rose->state) {
	case ROSE_STATE_1:
		queued = rose_state1_machine(sk, skb, frametype);
		break;
	case ROSE_STATE_2:
		queued = rose_state2_machine(sk, skb, frametype);
		break;
	case ROSE_STATE_3:
		queued = rose_state3_machine(sk, skb, frametype, ns, nr, q, d, m);
		break;
	case ROSE_STATE_4:
		queued = rose_state4_machine(sk, skb, frametype);
		break;
	case ROSE_STATE_5:
		queued = rose_state5_machine(sk, skb, frametype);
		break;
	}

	rose_kick(sk);

	return queued;
}
