



#define KMSG_COMPONENT "IPVS"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/ip.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/jiffies.h>


#include <linux/fs.h>
#include <linux/sysctl.h>

#include <net/ip_vs.h>



#define CHECK_EXPIRE_INTERVAL   (60*HZ)
#define ENTRY_TIMEOUT           (6*60*HZ)


#define COUNT_FOR_FULL_EXPIRATION   30
static int sysctl_ip_vs_lblc_expiration = 24*60*60*HZ;



#ifndef CONFIG_IP_VS_LBLC_TAB_BITS
#define CONFIG_IP_VS_LBLC_TAB_BITS      10
#endif
#define IP_VS_LBLC_TAB_BITS     CONFIG_IP_VS_LBLC_TAB_BITS
#define IP_VS_LBLC_TAB_SIZE     (1 << IP_VS_LBLC_TAB_BITS)
#define IP_VS_LBLC_TAB_MASK     (IP_VS_LBLC_TAB_SIZE - 1)



struct ip_vs_lblc_entry {
	struct list_head        list;
	int			af;		
	union nf_inet_addr      addr;           
	struct ip_vs_dest       *dest;          
	unsigned long           lastuse;        
};



struct ip_vs_lblc_table {
	struct list_head        bucket[IP_VS_LBLC_TAB_SIZE];  
	atomic_t                entries;        
	int                     max_size;       
	struct timer_list       periodic_timer; 
	int                     rover;          
	int                     counter;        
};




static ctl_table vs_vars_table[] = {
	{
		.procname	= "lblc_expiration",
		.data		= &sysctl_ip_vs_lblc_expiration,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ .ctl_name = 0 }
};

static struct ctl_table_header * sysctl_header;

static inline void ip_vs_lblc_free(struct ip_vs_lblc_entry *en)
{
	list_del(&en->list);
	
	atomic_dec(&en->dest->refcnt);
	kfree(en);
}



static inline unsigned
ip_vs_lblc_hashkey(int af, const union nf_inet_addr *addr)
{
	__be32 addr_fold = addr->ip;

#ifdef CONFIG_IP_VS_IPV6
	if (af == AF_INET6)
		addr_fold = addr->ip6[0]^addr->ip6[1]^
			    addr->ip6[2]^addr->ip6[3];
#endif
	return (ntohl(addr_fold)*2654435761UL) & IP_VS_LBLC_TAB_MASK;
}



static void
ip_vs_lblc_hash(struct ip_vs_lblc_table *tbl, struct ip_vs_lblc_entry *en)
{
	unsigned hash = ip_vs_lblc_hashkey(en->af, &en->addr);

	list_add(&en->list, &tbl->bucket[hash]);
	atomic_inc(&tbl->entries);
}



static inline struct ip_vs_lblc_entry *
ip_vs_lblc_get(int af, struct ip_vs_lblc_table *tbl,
	       const union nf_inet_addr *addr)
{
	unsigned hash = ip_vs_lblc_hashkey(af, addr);
	struct ip_vs_lblc_entry *en;

	list_for_each_entry(en, &tbl->bucket[hash], list)
		if (ip_vs_addr_equal(af, &en->addr, addr))
			return en;

	return NULL;
}



static inline struct ip_vs_lblc_entry *
ip_vs_lblc_new(struct ip_vs_lblc_table *tbl, const union nf_inet_addr *daddr,
	       struct ip_vs_dest *dest)
{
	struct ip_vs_lblc_entry *en;

	en = ip_vs_lblc_get(dest->af, tbl, daddr);
	if (!en) {
		en = kmalloc(sizeof(*en), GFP_ATOMIC);
		if (!en) {
			pr_err("%s(): no memory\n", __func__);
			return NULL;
		}

		en->af = dest->af;
		ip_vs_addr_copy(dest->af, &en->addr, daddr);
		en->lastuse = jiffies;

		atomic_inc(&dest->refcnt);
		en->dest = dest;

		ip_vs_lblc_hash(tbl, en);
	} else if (en->dest != dest) {
		atomic_dec(&en->dest->refcnt);
		atomic_inc(&dest->refcnt);
		en->dest = dest;
	}

	return en;
}



static void ip_vs_lblc_flush(struct ip_vs_lblc_table *tbl)
{
	struct ip_vs_lblc_entry *en, *nxt;
	int i;

	for (i=0; i<IP_VS_LBLC_TAB_SIZE; i++) {
		list_for_each_entry_safe(en, nxt, &tbl->bucket[i], list) {
			ip_vs_lblc_free(en);
			atomic_dec(&tbl->entries);
		}
	}
}


static inline void ip_vs_lblc_full_check(struct ip_vs_service *svc)
{
	struct ip_vs_lblc_table *tbl = svc->sched_data;
	struct ip_vs_lblc_entry *en, *nxt;
	unsigned long now = jiffies;
	int i, j;

	for (i=0, j=tbl->rover; i<IP_VS_LBLC_TAB_SIZE; i++) {
		j = (j + 1) & IP_VS_LBLC_TAB_MASK;

		write_lock(&svc->sched_lock);
		list_for_each_entry_safe(en, nxt, &tbl->bucket[j], list) {
			if (time_before(now,
					en->lastuse + sysctl_ip_vs_lblc_expiration))
				continue;

			ip_vs_lblc_free(en);
			atomic_dec(&tbl->entries);
		}
		write_unlock(&svc->sched_lock);
	}
	tbl->rover = j;
}



static void ip_vs_lblc_check_expire(unsigned long data)
{
	struct ip_vs_service *svc = (struct ip_vs_service *) data;
	struct ip_vs_lblc_table *tbl = svc->sched_data;
	unsigned long now = jiffies;
	int goal;
	int i, j;
	struct ip_vs_lblc_entry *en, *nxt;

	if ((tbl->counter % COUNT_FOR_FULL_EXPIRATION) == 0) {
		
		ip_vs_lblc_full_check(svc);
		tbl->counter = 1;
		goto out;
	}

	if (atomic_read(&tbl->entries) <= tbl->max_size) {
		tbl->counter++;
		goto out;
	}

	goal = (atomic_read(&tbl->entries) - tbl->max_size)*4/3;
	if (goal > tbl->max_size/2)
		goal = tbl->max_size/2;

	for (i=0, j=tbl->rover; i<IP_VS_LBLC_TAB_SIZE; i++) {
		j = (j + 1) & IP_VS_LBLC_TAB_MASK;

		write_lock(&svc->sched_lock);
		list_for_each_entry_safe(en, nxt, &tbl->bucket[j], list) {
			if (time_before(now, en->lastuse + ENTRY_TIMEOUT))
				continue;

			ip_vs_lblc_free(en);
			atomic_dec(&tbl->entries);
			goal--;
		}
		write_unlock(&svc->sched_lock);
		if (goal <= 0)
			break;
	}
	tbl->rover = j;

  out:
	mod_timer(&tbl->periodic_timer, jiffies+CHECK_EXPIRE_INTERVAL);
}


static int ip_vs_lblc_init_svc(struct ip_vs_service *svc)
{
	int i;
	struct ip_vs_lblc_table *tbl;

	
	tbl = kmalloc(sizeof(*tbl), GFP_ATOMIC);
	if (tbl == NULL) {
		pr_err("%s(): no memory\n", __func__);
		return -ENOMEM;
	}
	svc->sched_data = tbl;
	IP_VS_DBG(6, "LBLC hash table (memory=%Zdbytes) allocated for "
		  "current service\n", sizeof(*tbl));

	
	for (i=0; i<IP_VS_LBLC_TAB_SIZE; i++) {
		INIT_LIST_HEAD(&tbl->bucket[i]);
	}
	tbl->max_size = IP_VS_LBLC_TAB_SIZE*16;
	tbl->rover = 0;
	tbl->counter = 1;

	
	setup_timer(&tbl->periodic_timer, ip_vs_lblc_check_expire,
			(unsigned long)svc);
	mod_timer(&tbl->periodic_timer, jiffies + CHECK_EXPIRE_INTERVAL);

	return 0;
}


static int ip_vs_lblc_done_svc(struct ip_vs_service *svc)
{
	struct ip_vs_lblc_table *tbl = svc->sched_data;

	
	del_timer_sync(&tbl->periodic_timer);

	
	ip_vs_lblc_flush(tbl);

	
	kfree(tbl);
	IP_VS_DBG(6, "LBLC hash table (memory=%Zdbytes) released\n",
		  sizeof(*tbl));

	return 0;
}


static inline struct ip_vs_dest *
__ip_vs_lblc_schedule(struct ip_vs_service *svc)
{
	struct ip_vs_dest *dest, *least;
	int loh, doh;

	
	list_for_each_entry(dest, &svc->destinations, n_list) {
		if (dest->flags & IP_VS_DEST_F_OVERLOAD)
			continue;
		if (atomic_read(&dest->weight) > 0) {
			least = dest;
			loh = atomic_read(&least->activeconns) * 50
				+ atomic_read(&least->inactconns);
			goto nextstage;
		}
	}
	return NULL;

	
  nextstage:
	list_for_each_entry_continue(dest, &svc->destinations, n_list) {
		if (dest->flags & IP_VS_DEST_F_OVERLOAD)
			continue;

		doh = atomic_read(&dest->activeconns) * 50
			+ atomic_read(&dest->inactconns);
		if (loh * atomic_read(&dest->weight) >
		    doh * atomic_read(&least->weight)) {
			least = dest;
			loh = doh;
		}
	}

	IP_VS_DBG_BUF(6, "LBLC: server %s:%d "
		      "activeconns %d refcnt %d weight %d overhead %d\n",
		      IP_VS_DBG_ADDR(least->af, &least->addr),
		      ntohs(least->port),
		      atomic_read(&least->activeconns),
		      atomic_read(&least->refcnt),
		      atomic_read(&least->weight), loh);

	return least;
}



static inline int
is_overloaded(struct ip_vs_dest *dest, struct ip_vs_service *svc)
{
	if (atomic_read(&dest->activeconns) > atomic_read(&dest->weight)) {
		struct ip_vs_dest *d;

		list_for_each_entry(d, &svc->destinations, n_list) {
			if (atomic_read(&d->activeconns)*2
			    < atomic_read(&d->weight)) {
				return 1;
			}
		}
	}
	return 0;
}



static struct ip_vs_dest *
ip_vs_lblc_schedule(struct ip_vs_service *svc, const struct sk_buff *skb)
{
	struct ip_vs_lblc_table *tbl = svc->sched_data;
	struct ip_vs_iphdr iph;
	struct ip_vs_dest *dest = NULL;
	struct ip_vs_lblc_entry *en;

	ip_vs_fill_iphdr(svc->af, skb_network_header(skb), &iph);

	IP_VS_DBG(6, "%s(): Scheduling...\n", __func__);

	
	read_lock(&svc->sched_lock);
	en = ip_vs_lblc_get(svc->af, tbl, &iph.daddr);
	if (en) {
		
		en->lastuse = jiffies;

		

		if (en->dest->flags & IP_VS_DEST_F_AVAILABLE)
			dest = en->dest;
	}
	read_unlock(&svc->sched_lock);

	
	if (dest && atomic_read(&dest->weight) > 0 && !is_overloaded(dest, svc))
		goto out;

	
	dest = __ip_vs_lblc_schedule(svc);
	if (!dest) {
		IP_VS_ERR_RL("LBLC: no destination available\n");
		return NULL;
	}

	
	write_lock(&svc->sched_lock);
	ip_vs_lblc_new(tbl, &iph.daddr, dest);
	write_unlock(&svc->sched_lock);

out:
	IP_VS_DBG_BUF(6, "LBLC: destination IP address %s --> server %s:%d\n",
		      IP_VS_DBG_ADDR(svc->af, &iph.daddr),
		      IP_VS_DBG_ADDR(svc->af, &dest->addr), ntohs(dest->port));

	return dest;
}



static struct ip_vs_scheduler ip_vs_lblc_scheduler =
{
	.name =			"lblc",
	.refcnt =		ATOMIC_INIT(0),
	.module =		THIS_MODULE,
	.n_list =		LIST_HEAD_INIT(ip_vs_lblc_scheduler.n_list),
	.init_service =		ip_vs_lblc_init_svc,
	.done_service =		ip_vs_lblc_done_svc,
	.schedule =		ip_vs_lblc_schedule,
};


static int __init ip_vs_lblc_init(void)
{
	int ret;

	sysctl_header = register_sysctl_paths(net_vs_ctl_path, vs_vars_table);
	ret = register_ip_vs_scheduler(&ip_vs_lblc_scheduler);
	if (ret)
		unregister_sysctl_table(sysctl_header);
	return ret;
}


static void __exit ip_vs_lblc_cleanup(void)
{
	unregister_sysctl_table(sysctl_header);
	unregister_ip_vs_scheduler(&ip_vs_lblc_scheduler);
}


module_init(ip_vs_lblc_init);
module_exit(ip_vs_lblc_cleanup);
MODULE_LICENSE("GPL");
