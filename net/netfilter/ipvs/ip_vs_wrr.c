

#define KMSG_COMPONENT "IPVS"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>

#include <net/ip_vs.h>


struct ip_vs_wrr_mark {
	struct list_head *cl;	
	int cw;			
	int mw;			
	int di;			
};



static int gcd(int a, int b)
{
	int c;

	while ((c = a % b)) {
		a = b;
		b = c;
	}
	return b;
}

static int ip_vs_wrr_gcd_weight(struct ip_vs_service *svc)
{
	struct ip_vs_dest *dest;
	int weight;
	int g = 0;

	list_for_each_entry(dest, &svc->destinations, n_list) {
		weight = atomic_read(&dest->weight);
		if (weight > 0) {
			if (g > 0)
				g = gcd(weight, g);
			else
				g = weight;
		}
	}
	return g ? g : 1;
}



static int ip_vs_wrr_max_weight(struct ip_vs_service *svc)
{
	struct ip_vs_dest *dest;
	int new_weight, weight = 0;

	list_for_each_entry(dest, &svc->destinations, n_list) {
		new_weight = atomic_read(&dest->weight);
		if (new_weight > weight)
			weight = new_weight;
	}

	return weight;
}


static int ip_vs_wrr_init_svc(struct ip_vs_service *svc)
{
	struct ip_vs_wrr_mark *mark;

	
	mark = kmalloc(sizeof(struct ip_vs_wrr_mark), GFP_ATOMIC);
	if (mark == NULL) {
		pr_err("%s(): no memory\n", __func__);
		return -ENOMEM;
	}
	mark->cl = &svc->destinations;
	mark->cw = 0;
	mark->mw = ip_vs_wrr_max_weight(svc);
	mark->di = ip_vs_wrr_gcd_weight(svc);
	svc->sched_data = mark;

	return 0;
}


static int ip_vs_wrr_done_svc(struct ip_vs_service *svc)
{
	
	kfree(svc->sched_data);

	return 0;
}


static int ip_vs_wrr_update_svc(struct ip_vs_service *svc)
{
	struct ip_vs_wrr_mark *mark = svc->sched_data;

	mark->cl = &svc->destinations;
	mark->mw = ip_vs_wrr_max_weight(svc);
	mark->di = ip_vs_wrr_gcd_weight(svc);
	if (mark->cw > mark->mw)
		mark->cw = 0;
	return 0;
}



static struct ip_vs_dest *
ip_vs_wrr_schedule(struct ip_vs_service *svc, const struct sk_buff *skb)
{
	struct ip_vs_dest *dest;
	struct ip_vs_wrr_mark *mark = svc->sched_data;
	struct list_head *p;

	IP_VS_DBG(6, "%s(): Scheduling...\n", __func__);

	
	write_lock(&svc->sched_lock);
	p = mark->cl;
	while (1) {
		if (mark->cl == &svc->destinations) {
			

			if (mark->cl == mark->cl->next) {
				
				IP_VS_ERR_RL("WRR: no destination available: "
					     "no destinations present\n");
				dest = NULL;
				goto out;
			}

			mark->cl = svc->destinations.next;
			mark->cw -= mark->di;
			if (mark->cw <= 0) {
				mark->cw = mark->mw;
				
				if (mark->cw == 0) {
					mark->cl = &svc->destinations;
					IP_VS_ERR_RL("WRR: no destination "
						     "available\n");
					dest = NULL;
					goto out;
				}
			}
		} else
			mark->cl = mark->cl->next;

		if (mark->cl != &svc->destinations) {
			
			dest = list_entry(mark->cl, struct ip_vs_dest, n_list);
			if (!(dest->flags & IP_VS_DEST_F_OVERLOAD) &&
			    atomic_read(&dest->weight) >= mark->cw) {
				
				break;
			}
		}

		if (mark->cl == p && mark->cw == mark->di) {
			
			dest = NULL;
			IP_VS_ERR_RL("WRR: no destination available: "
				     "all destinations are overloaded\n");
			goto out;
		}
	}

	IP_VS_DBG_BUF(6, "WRR: server %s:%u "
		      "activeconns %d refcnt %d weight %d\n",
		      IP_VS_DBG_ADDR(svc->af, &dest->addr), ntohs(dest->port),
		      atomic_read(&dest->activeconns),
		      atomic_read(&dest->refcnt),
		      atomic_read(&dest->weight));

  out:
	write_unlock(&svc->sched_lock);
	return dest;
}


static struct ip_vs_scheduler ip_vs_wrr_scheduler = {
	.name =			"wrr",
	.refcnt =		ATOMIC_INIT(0),
	.module =		THIS_MODULE,
	.n_list =		LIST_HEAD_INIT(ip_vs_wrr_scheduler.n_list),
	.init_service =		ip_vs_wrr_init_svc,
	.done_service =		ip_vs_wrr_done_svc,
	.update_service =	ip_vs_wrr_update_svc,
	.schedule =		ip_vs_wrr_schedule,
};

static int __init ip_vs_wrr_init(void)
{
	return register_ip_vs_scheduler(&ip_vs_wrr_scheduler) ;
}

static void __exit ip_vs_wrr_cleanup(void)
{
	unregister_ip_vs_scheduler(&ip_vs_wrr_scheduler);
}

module_init(ip_vs_wrr_init);
module_exit(ip_vs_wrr_cleanup);
MODULE_LICENSE("GPL");
