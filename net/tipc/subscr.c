

#include "core.h"
#include "dbg.h"
#include "name_table.h"
#include "port.h"
#include "ref.h"
#include "subscr.h"



struct subscriber {
	u32 port_ref;
	spinlock_t *lock;
	struct list_head subscriber_list;
	struct list_head subscription_list;
};



struct top_srv {
	u32 user_ref;
	u32 setup_port;
	atomic_t subscription_count;
	struct list_head subscriber_list;
	spinlock_t lock;
};

static struct top_srv topsrv = { 0 };



static u32 htohl(u32 in, int swap)
{
	return swap ? swab32(in) : in;
}



static void subscr_send_event(struct subscription *sub,
			      u32 found_lower,
			      u32 found_upper,
			      u32 event,
			      u32 port_ref,
			      u32 node)
{
	struct iovec msg_sect;

	msg_sect.iov_base = (void *)&sub->evt;
	msg_sect.iov_len = sizeof(struct tipc_event);

	sub->evt.event = htohl(event, sub->swap);
	sub->evt.found_lower = htohl(found_lower, sub->swap);
	sub->evt.found_upper = htohl(found_upper, sub->swap);
	sub->evt.port.ref = htohl(port_ref, sub->swap);
	sub->evt.port.node = htohl(node, sub->swap);
	tipc_send(sub->server_ref, 1, &msg_sect);
}



int tipc_subscr_overlap(struct subscription *sub,
			u32 found_lower,
			u32 found_upper)

{
	if (found_lower < sub->seq.lower)
		found_lower = sub->seq.lower;
	if (found_upper > sub->seq.upper)
		found_upper = sub->seq.upper;
	if (found_lower > found_upper)
		return 0;
	return 1;
}



void tipc_subscr_report_overlap(struct subscription *sub,
				u32 found_lower,
				u32 found_upper,
				u32 event,
				u32 port_ref,
				u32 node,
				int must)
{
	if (!tipc_subscr_overlap(sub, found_lower, found_upper))
		return;
	if (!must && !(sub->filter & TIPC_SUB_PORTS))
		return;

	sub->event_cb(sub, found_lower, found_upper, event, port_ref, node);
}



static void subscr_timeout(struct subscription *sub)
{
	struct port *server_port;

	

	server_port = tipc_port_lock(sub->server_ref);
	if (server_port == NULL)
		return;

	

	if (sub->timeout == TIPC_WAIT_FOREVER) {
		tipc_port_unlock(server_port);
		return;
	}

	

	tipc_nametbl_unsubscribe(sub);

	

	list_del(&sub->subscription_list);

	

	tipc_port_unlock(server_port);

	

	subscr_send_event(sub, sub->evt.s.seq.lower, sub->evt.s.seq.upper,
			  TIPC_SUBSCR_TIMEOUT, 0, 0);

	

	k_term_timer(&sub->timer);
	kfree(sub);
	atomic_dec(&topsrv.subscription_count);
}



static void subscr_del(struct subscription *sub)
{
	tipc_nametbl_unsubscribe(sub);
	list_del(&sub->subscription_list);
	kfree(sub);
	atomic_dec(&topsrv.subscription_count);
}



static void subscr_terminate(struct subscriber *subscriber)
{
	u32 port_ref;
	struct subscription *sub;
	struct subscription *sub_temp;

	

	port_ref = subscriber->port_ref;
	subscriber->port_ref = 0;
	spin_unlock_bh(subscriber->lock);

	

	tipc_shutdown(port_ref);
	tipc_deleteport(port_ref);

	

	list_for_each_entry_safe(sub, sub_temp, &subscriber->subscription_list,
				 subscription_list) {
		if (sub->timeout != TIPC_WAIT_FOREVER) {
			k_cancel_timer(&sub->timer);
			k_term_timer(&sub->timer);
		}
		dbg("Term: Removing sub %u,%u,%u from subscriber %x list\n",
		    sub->seq.type, sub->seq.lower, sub->seq.upper, subscriber);
		subscr_del(sub);
	}

	

	spin_lock_bh(&topsrv.lock);
	list_del(&subscriber->subscriber_list);
	spin_unlock_bh(&topsrv.lock);

	

	spin_lock_bh(subscriber->lock);

	

	kfree(subscriber);
}



static void subscr_cancel(struct tipc_subscr *s,
			  struct subscriber *subscriber)
{
	struct subscription *sub;
	struct subscription *sub_temp;
	int found = 0;

	

	list_for_each_entry_safe(sub, sub_temp, &subscriber->subscription_list,
				 subscription_list) {
		if (!memcmp(s, &sub->evt.s, sizeof(struct tipc_subscr))) {
			found = 1;
			break;
		}
	}
	if (!found)
		return;

	

	if (sub->timeout != TIPC_WAIT_FOREVER) {
		sub->timeout = TIPC_WAIT_FOREVER;
		spin_unlock_bh(subscriber->lock);
		k_cancel_timer(&sub->timer);
		k_term_timer(&sub->timer);
		spin_lock_bh(subscriber->lock);
	}
	dbg("Cancel: removing sub %u,%u,%u from subscriber %x list\n",
	    sub->seq.type, sub->seq.lower, sub->seq.upper, subscriber);
	subscr_del(sub);
}



static struct subscription *subscr_subscribe(struct tipc_subscr *s,
					     struct subscriber *subscriber)
{
	struct subscription *sub;
	int swap;

	

	swap = !(s->filter & (TIPC_SUB_PORTS | TIPC_SUB_SERVICE));

	

	if (s->filter & htohl(TIPC_SUB_CANCEL, swap)) {
		s->filter &= ~htohl(TIPC_SUB_CANCEL, swap);
		subscr_cancel(s, subscriber);
		return NULL;
	}

	

	if (atomic_read(&topsrv.subscription_count) >= tipc_max_subscriptions) {
		warn("Subscription rejected, subscription limit reached (%u)\n",
		     tipc_max_subscriptions);
		subscr_terminate(subscriber);
		return NULL;
	}

	

	sub = kmalloc(sizeof(*sub), GFP_ATOMIC);
	if (!sub) {
		warn("Subscription rejected, no memory\n");
		subscr_terminate(subscriber);
		return NULL;
	}

	

	sub->seq.type = htohl(s->seq.type, swap);
	sub->seq.lower = htohl(s->seq.lower, swap);
	sub->seq.upper = htohl(s->seq.upper, swap);
	sub->timeout = htohl(s->timeout, swap);
	sub->filter = htohl(s->filter, swap);
	if ((!(sub->filter & TIPC_SUB_PORTS)
	     == !(sub->filter & TIPC_SUB_SERVICE))
	    || (sub->seq.lower > sub->seq.upper)) {
		warn("Subscription rejected, illegal request\n");
		kfree(sub);
		subscr_terminate(subscriber);
		return NULL;
	}
	sub->event_cb = subscr_send_event;
	INIT_LIST_HEAD(&sub->nameseq_list);
	list_add(&sub->subscription_list, &subscriber->subscription_list);
	sub->server_ref = subscriber->port_ref;
	sub->swap = swap;
	memcpy(&sub->evt.s, s, sizeof(struct tipc_subscr));
	atomic_inc(&topsrv.subscription_count);
	if (sub->timeout != TIPC_WAIT_FOREVER) {
		k_init_timer(&sub->timer,
			     (Handler)subscr_timeout, (unsigned long)sub);
		k_start_timer(&sub->timer, sub->timeout);
	}

	return sub;
}



static void subscr_conn_shutdown_event(void *usr_handle,
				       u32 port_ref,
				       struct sk_buff **buf,
				       unsigned char const *data,
				       unsigned int size,
				       int reason)
{
	struct subscriber *subscriber = usr_handle;
	spinlock_t *subscriber_lock;

	if (tipc_port_lock(port_ref) == NULL)
		return;

	subscriber_lock = subscriber->lock;
	subscr_terminate(subscriber);
	spin_unlock_bh(subscriber_lock);
}



static void subscr_conn_msg_event(void *usr_handle,
				  u32 port_ref,
				  struct sk_buff **buf,
				  const unchar *data,
				  u32 size)
{
	struct subscriber *subscriber = usr_handle;
	spinlock_t *subscriber_lock;
	struct subscription *sub;

	

	if (tipc_port_lock(port_ref) == NULL)
		return;

	subscriber_lock = subscriber->lock;

	if (size != sizeof(struct tipc_subscr)) {
		subscr_terminate(subscriber);
		spin_unlock_bh(subscriber_lock);
	} else {
		sub = subscr_subscribe((struct tipc_subscr *)data, subscriber);
		spin_unlock_bh(subscriber_lock);
		if (sub != NULL) {

			

			tipc_nametbl_subscribe(sub);
		}
	}
}



static void subscr_named_msg_event(void *usr_handle,
				   u32 port_ref,
				   struct sk_buff **buf,
				   const unchar *data,
				   u32 size,
				   u32 importance,
				   struct tipc_portid const *orig,
				   struct tipc_name_seq const *dest)
{
	static struct iovec msg_sect = {NULL, 0};

	struct subscriber *subscriber;
	u32 server_port_ref;

	

	subscriber = kzalloc(sizeof(struct subscriber), GFP_ATOMIC);
	if (subscriber == NULL) {
		warn("Subscriber rejected, no memory\n");
		return;
	}
	INIT_LIST_HEAD(&subscriber->subscription_list);
	INIT_LIST_HEAD(&subscriber->subscriber_list);

	

	tipc_createport(topsrv.user_ref,
			subscriber,
			importance,
			NULL,
			NULL,
			subscr_conn_shutdown_event,
			NULL,
			NULL,
			subscr_conn_msg_event,
			NULL,
			&subscriber->port_ref);
	if (subscriber->port_ref == 0) {
		warn("Subscriber rejected, unable to create port\n");
		kfree(subscriber);
		return;
	}
	tipc_connect2port(subscriber->port_ref, orig);

	

	subscriber->lock = tipc_port_lock(subscriber->port_ref)->publ.lock;

	

	spin_lock_bh(&topsrv.lock);
	list_add(&subscriber->subscriber_list, &topsrv.subscriber_list);
	spin_unlock_bh(&topsrv.lock);

	

	server_port_ref = subscriber->port_ref;
	spin_unlock_bh(subscriber->lock);

	

	tipc_send(server_port_ref, 1, &msg_sect);

	

	if (size != 0) {
		subscr_conn_msg_event(subscriber, server_port_ref,
				      buf, data, size);
	}
}

int tipc_subscr_start(void)
{
	struct tipc_name_seq seq = {TIPC_TOP_SRV, TIPC_TOP_SRV, TIPC_TOP_SRV};
	int res = -1;

	memset(&topsrv, 0, sizeof (topsrv));
	spin_lock_init(&topsrv.lock);
	INIT_LIST_HEAD(&topsrv.subscriber_list);

	spin_lock_bh(&topsrv.lock);
	res = tipc_attach(&topsrv.user_ref, NULL, NULL);
	if (res) {
		spin_unlock_bh(&topsrv.lock);
		return res;
	}

	res = tipc_createport(topsrv.user_ref,
			      NULL,
			      TIPC_CRITICAL_IMPORTANCE,
			      NULL,
			      NULL,
			      NULL,
			      NULL,
			      subscr_named_msg_event,
			      NULL,
			      NULL,
			      &topsrv.setup_port);
	if (res)
		goto failed;

	res = tipc_nametbl_publish_rsv(topsrv.setup_port, TIPC_NODE_SCOPE, &seq);
	if (res)
		goto failed;

	spin_unlock_bh(&topsrv.lock);
	return 0;

failed:
	err("Failed to create subscription service\n");
	tipc_detach(topsrv.user_ref);
	topsrv.user_ref = 0;
	spin_unlock_bh(&topsrv.lock);
	return res;
}

void tipc_subscr_stop(void)
{
	struct subscriber *subscriber;
	struct subscriber *subscriber_temp;
	spinlock_t *subscriber_lock;

	if (topsrv.user_ref) {
		tipc_deleteport(topsrv.setup_port);
		list_for_each_entry_safe(subscriber, subscriber_temp,
					 &topsrv.subscriber_list,
					 subscriber_list) {
			subscriber_lock = subscriber->lock;
			spin_lock_bh(subscriber_lock);
			subscr_terminate(subscriber);
			spin_unlock_bh(subscriber_lock);
		}
		tipc_detach(topsrv.user_ref);
		topsrv.user_ref = 0;
	}
}


int tipc_ispublished(struct tipc_name const *name)
{
	u32 domain = 0;

	return(tipc_nametbl_translate(name->type, name->instance,&domain) != 0);
}

