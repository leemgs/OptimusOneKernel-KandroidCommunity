

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/wakelock.h>
#include <linux/if_arp.h>
#include <linux/msm_rmnet.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <mach/msm_smd.h>


#define SMD_PORT_ETHER0 11


#define RMNET_DATA_LEN 2000

static const char *ch_name[3] = {
	"DATA5",
	"DATA6",
	"DATA7",
};

struct rmnet_private
{
	smd_channel_t *ch;
	struct net_device_stats stats;
	const char *chname;
	struct wake_lock wake_lock;
#ifdef CONFIG_MSM_RMNET_DEBUG
	ktime_t last_packet;
	unsigned long wakeups_xmit;
	unsigned long wakeups_rcv;
	unsigned long timeout_us;
#endif
	struct sk_buff *skb;
	spinlock_t lock;
	struct tasklet_struct tsklt;
	u32 operation_mode;    
};


static int rmnet_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

static int count_this_packet(void *_hdr, int len)
{
	struct ethhdr *hdr = _hdr;

	if (len >= ETH_HLEN && hdr->h_proto == htons(ETH_P_ARP))
		return 0;

	return 1;
}

#ifdef CONFIG_MSM_RMNET_DEBUG
static unsigned long timeout_us;

#ifdef CONFIG_HAS_EARLYSUSPEND

static unsigned long timeout_suspend_us;
static struct device *rmnet0;


static ssize_t timeout_suspend_store(struct device *d, struct device_attribute *attr, const char *buf, size_t n)
{
	timeout_suspend_us = simple_strtoul(buf, NULL, 10);
	return n;
}

static ssize_t timeout_suspend_show(struct device *d,
				    struct device_attribute *attr,
				    char *buf)
{
	return sprintf(buf, "%lu\n", (unsigned long) timeout_suspend_us);
}

static DEVICE_ATTR(timeout_suspend, 0664, timeout_suspend_show, timeout_suspend_store);

static void rmnet_early_suspend(struct early_suspend *handler) {
	if (rmnet0) {
		struct rmnet_private *p = netdev_priv(to_net_dev(rmnet0));
		p->timeout_us = timeout_suspend_us;
	}
}

static void rmnet_late_resume(struct early_suspend *handler) {
	if (rmnet0) {
		struct rmnet_private *p = netdev_priv(to_net_dev(rmnet0));
		p->timeout_us = timeout_us;
	}
}

static struct early_suspend rmnet_power_suspend = {
	.suspend = rmnet_early_suspend,
	.resume = rmnet_late_resume,
};

static int __init rmnet_late_init(void)
{
	register_early_suspend(&rmnet_power_suspend);
	return 0;
}

late_initcall(rmnet_late_init);
#endif


static int rmnet_cause_wakeup(struct rmnet_private *p) {
	int ret = 0;
	ktime_t now;
	if (p->timeout_us == 0) 
		return 0;

	
	now = ktime_get_real();

	if (ktime_us_delta(now, p->last_packet) > p->timeout_us) {
		ret = 1;
	}
	p->last_packet = now;
	return ret;
}

static ssize_t wakeups_xmit_show(struct device *d,
				 struct device_attribute *attr,
				 char *buf)
{
	struct rmnet_private *p = netdev_priv(to_net_dev(d));
	return sprintf(buf, "%lu\n", p->wakeups_xmit);
}

DEVICE_ATTR(wakeups_xmit, 0444, wakeups_xmit_show, NULL);

static ssize_t wakeups_rcv_show(struct device *d, struct device_attribute *attr,
		char *buf)
{
	struct rmnet_private *p = netdev_priv(to_net_dev(d));
	return sprintf(buf, "%lu\n", p->wakeups_rcv);
}

DEVICE_ATTR(wakeups_rcv, 0444, wakeups_rcv_show, NULL);


static ssize_t timeout_store(struct device *d, struct device_attribute *attr,
		const char *buf, size_t n)
{
#ifndef CONFIG_HAS_EARLYSUSPEND
	struct rmnet_private *p = netdev_priv(to_net_dev(d));
	p->timeout_us = timeout_us = simple_strtoul(buf, NULL, 10);
#else

	timeout_us = simple_strtoul(buf, NULL, 10);
#endif
	return n;
}

static ssize_t timeout_show(struct device *d, struct device_attribute *attr,
			    char *buf)
{
	struct rmnet_private *p = netdev_priv(to_net_dev(d));
	p = netdev_priv(to_net_dev(d));
	return sprintf(buf, "%lu\n", timeout_us);
}

DEVICE_ATTR(timeout, 0664, timeout_show, timeout_store);
#endif

static __be16 rmnet_ip_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	__be16 protocol = 0;

	skb->dev = dev;

	
	switch (skb->data[0] & 0xf0) {
	case 0x40:
		protocol = htons(ETH_P_IP);
		break;
	case 0x60:
		protocol = htons(ETH_P_IPV6);
		break;
	default:
		pr_err("rmnet_recv() L3 protocol decode error: 0x%02x",
		       skb->data[0] & 0xf0);
		
	}
	return protocol;
}


static void smd_net_data_handler(unsigned long arg)
{
	struct net_device *dev = (struct net_device *) arg;
	struct rmnet_private *p = netdev_priv(dev);
	struct sk_buff *skb;
	void *ptr = 0;
	int sz;
	u32 opmode = p->operation_mode;
	unsigned long flags;

	for (;;) {
		sz = smd_cur_packet_size(p->ch);
		if (sz == 0) break;
		if (smd_read_avail(p->ch) < sz) break;



                
   

		if (RMNET_IS_MODE_IP(opmode) ? (sz > dev->mtu) :
						(sz > (dev->mtu + ETH_HLEN))) {
			pr_err("rmnet_recv() discarding %d len (%d mtu)\n",
				sz, RMNET_IS_MODE_IP(opmode) ?
					dev->mtu : (dev->mtu + ETH_HLEN));
			ptr = 0;
		} else {
			skb = dev_alloc_skb(sz + NET_IP_ALIGN);
			if (skb == NULL) {
				pr_err("rmnet_recv() cannot allocate skb\n");
			} else {
				skb->dev = dev;
				skb_reserve(skb, NET_IP_ALIGN);
				ptr = skb_put(skb, sz);
				wake_lock_timeout(&p->wake_lock, HZ / 2);
				if (smd_read(p->ch, ptr, sz) != sz) {
					pr_err("rmnet_recv() smd lied about avail?!");
					ptr = 0;
					dev_kfree_skb_irq(skb);
				} else {
					
					spin_lock_irqsave(&p->lock, flags);
					opmode = p->operation_mode;
					spin_unlock_irqrestore(&p->lock, flags);

					if (RMNET_IS_MODE_IP(opmode)) {
						
						skb->protocol =
						  rmnet_ip_type_trans(skb, dev);
					} else {
						
						skb->protocol =
						  eth_type_trans(skb, dev);
					}
					if (RMNET_IS_MODE_IP(opmode) ||
					    count_this_packet(ptr, skb->len)) {
#ifdef CONFIG_MSM_RMNET_DEBUG
						p->wakeups_rcv +=
							rmnet_cause_wakeup(p);
#endif
						p->stats.rx_packets++;
						p->stats.rx_bytes += skb->len;
					}
					netif_rx(skb);
				}
				continue;
			}
		}
		if (smd_read(p->ch, ptr, sz) != sz)
			pr_err("rmnet_recv() smd lied about avail?!");
	}
}

static DECLARE_TASKLET(smd_net_data_tasklet, smd_net_data_handler, 0);

static int _rmnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	smd_channel_t *ch = p->ch;
	int smd_ret;
	struct QMI_QOS_HDR_S *qmih;
	u32 opmode;
	unsigned long flags;

	
	spin_lock_irqsave(&p->lock, flags);
	opmode = p->operation_mode;
	spin_unlock_irqrestore(&p->lock, flags);

	if (RMNET_IS_MODE_QOS(opmode)) {
		qmih = (struct QMI_QOS_HDR_S *)
			skb_push(skb, sizeof(struct QMI_QOS_HDR_S));
		qmih->version = 1;
		qmih->flags = 0;
		qmih->flow_id = skb->mark;
	}

	dev->trans_start = jiffies;
	smd_ret = smd_write(ch, skb->data, skb->len);
	if (smd_ret != skb->len) {
		pr_err("%s: smd_write returned error %d", __func__, smd_ret);
		goto xmit_out;
	}

	if (RMNET_IS_MODE_IP(opmode) ||
	    count_this_packet(skb->data, skb->len)) {
		p->stats.tx_packets++;
		p->stats.tx_bytes += skb->len;
#ifdef CONFIG_MSM_RMNET_DEBUG
		p->wakeups_xmit += rmnet_cause_wakeup(p);
#endif
	}

xmit_out:
	
	dev_kfree_skb_irq(skb);
	return 0;
}

static void _rmnet_resume_flow(unsigned long param)
{
	struct net_device *dev = (struct net_device *)param;
	struct rmnet_private *p = netdev_priv(dev);
	struct sk_buff *skb = NULL;
	unsigned long flags;

	
	spin_lock_irqsave(&p->lock, flags);
	if (p->skb && (smd_write_avail(p->ch) >= p->skb->len)) {
		skb = p->skb;
		p->skb = NULL;
		spin_unlock_irqrestore(&p->lock, flags);
		_rmnet_xmit(skb, dev);
		netif_wake_queue(dev);
	} else
		spin_unlock_irqrestore(&p->lock, flags);
}

static void smd_net_notify(void *_dev, unsigned event)
{
	struct rmnet_private *p = netdev_priv((struct net_device *)_dev);

	if (event != SMD_EVENT_DATA)
		return;

	spin_lock(&p->lock);
	if (p->skb && (smd_write_avail(p->ch) >= p->skb->len))
		tasklet_hi_schedule(&p->tsklt);

	spin_unlock(&p->lock);

	if (smd_read_avail(p->ch) &&
	    (smd_read_avail(p->ch) >= smd_cur_packet_size(p->ch))) {
		smd_net_data_tasklet.data = (unsigned long) _dev;
		tasklet_schedule(&smd_net_data_tasklet);
	}
}

static int __rmnet_open(struct net_device *dev)
{
	int r;
	struct rmnet_private *p = netdev_priv(dev);

	if (!p->ch) {
		r = smd_open(p->chname, &p->ch, dev, smd_net_notify);

		if (r < 0)
			return -ENODEV;
	}

	return 0;
}

static int __rmnet_close(struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	int rc;

	if (p->ch) {
		rc = smd_close(p->ch);
		p->ch = 0;
		return rc;
	} else
		return -EBADF;
}

static int rmnet_open(struct net_device *dev)
{
	int rc = 0;

	pr_info("rmnet_open()\n");

	rc = __rmnet_open(dev);

	netif_start_queue(dev);

	return rc;
}

static int rmnet_stop(struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);

	pr_info("rmnet_stop()\n");

	netif_stop_queue(dev);
	tasklet_kill(&p->tsklt);

	return 0;
}

static int rmnet_change_mtu(struct net_device *dev, int new_mtu)
{
	if (0 > new_mtu || RMNET_DATA_LEN < new_mtu)
		return -EINVAL;

	dev->mtu = new_mtu;

	return 0;
}

static int rmnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	smd_channel_t *ch = p->ch;
	unsigned long flags;

	if (netif_queue_stopped(dev)) {
		pr_err("fatal: rmnet_xmit called when netif_queue is stopped");
		return 0;
	}

	spin_lock_irqsave(&p->lock, flags);
	if (smd_write_avail(ch) < skb->len) {
		netif_stop_queue(dev);
		p->skb = skb;
		spin_unlock_irqrestore(&p->lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&p->lock, flags);

	_rmnet_xmit(skb, dev);

	return 0;
}

static struct net_device_stats *rmnet_get_stats(struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	return &p->stats;
}

static void rmnet_set_multicast_list(struct net_device *dev)
{
}

static void rmnet_tx_timeout(struct net_device *dev)
{
	pr_info("rmnet_tx_timeout()\n");
}


static const struct net_device_ops rmnet_ops_ether = {
	.ndo_open		= rmnet_open,
	.ndo_stop		= rmnet_stop,
	.ndo_start_xmit		= rmnet_xmit,
	.ndo_get_stats		= rmnet_get_stats,
	.ndo_set_multicast_list = rmnet_set_multicast_list,
	.ndo_tx_timeout		= rmnet_tx_timeout,
	.ndo_do_ioctl		= rmnet_ioctl,
	.ndo_change_mtu		= rmnet_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static const struct net_device_ops rmnet_ops_ip = {
	.ndo_open		= rmnet_open,
	.ndo_stop		= rmnet_stop,
	.ndo_start_xmit		= rmnet_xmit,
	.ndo_get_stats		= rmnet_get_stats,
	.ndo_set_multicast_list = rmnet_set_multicast_list,
	.ndo_tx_timeout		= rmnet_tx_timeout,
	.ndo_do_ioctl		= rmnet_ioctl,
	.ndo_change_mtu		= rmnet_change_mtu,
	.ndo_set_mac_address	= 0,
	.ndo_validate_addr	= 0,
};

static int rmnet_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct rmnet_private *p = netdev_priv(dev);
	u32 old_opmode = p->operation_mode;
	unsigned long flags;
	int prev_mtu = dev->mtu;
	int rc = 0;

	
	switch (cmd) {
	case RMNET_IOCTL_SET_LLP_ETHERNET:  
		
		if (p->operation_mode & RMNET_MODE_LLP_IP) {
			ether_setup(dev);
			random_ether_addr(dev->dev_addr);
			dev->mtu = prev_mtu;

			dev->netdev_ops = &rmnet_ops_ether;
			spin_lock_irqsave(&p->lock, flags);
			p->operation_mode &= ~RMNET_MODE_LLP_IP;
			p->operation_mode |= RMNET_MODE_LLP_ETH;
			spin_unlock_irqrestore(&p->lock, flags);
			pr_info("rmnet_ioctl(): "
				"set Ethernet protocol mode\n");
		}
		break;

	case RMNET_IOCTL_SET_LLP_IP:        
		
		if (p->operation_mode & RMNET_MODE_LLP_ETH) {

			
			dev->header_ops         = 0;  
			dev->type               = ARPHRD_RAWIP;
			dev->hard_header_len    = 0;
			dev->mtu                = prev_mtu;
			dev->addr_len           = 0;
			dev->flags              &= ~(IFF_BROADCAST|
						     IFF_MULTICAST);

			dev->netdev_ops = &rmnet_ops_ip;
			spin_lock_irqsave(&p->lock, flags);
			p->operation_mode &= ~RMNET_MODE_LLP_ETH;
			p->operation_mode |= RMNET_MODE_LLP_IP;
			spin_unlock_irqrestore(&p->lock, flags);
			pr_info("rmnet_ioctl(): set IP protocol mode\n");
		}
		break;

	case RMNET_IOCTL_GET_LLP:           
		ifr->ifr_ifru.ifru_data =
			(void *)(p->operation_mode &
				(RMNET_MODE_LLP_ETH|RMNET_MODE_LLP_IP));
		break;

	case RMNET_IOCTL_SET_QOS_ENABLE:    
		spin_lock_irqsave(&p->lock, flags);
		p->operation_mode |= RMNET_MODE_QOS;
		spin_unlock_irqrestore(&p->lock, flags);
		pr_info("rmnet_ioctl(): set QMI QOS header enable\n");
		break;

	case RMNET_IOCTL_SET_QOS_DISABLE:   
		spin_lock_irqsave(&p->lock, flags);
		p->operation_mode &= ~RMNET_MODE_QOS;
		spin_unlock_irqrestore(&p->lock, flags);
		pr_info("rmnet_ioctl(): set QMI QOS header disable\n");
		break;

	case RMNET_IOCTL_GET_QOS:           
		ifr->ifr_ifru.ifru_data =
			(void *)(p->operation_mode & RMNET_MODE_QOS);
		break;

	case RMNET_IOCTL_GET_OPMODE:        
		ifr->ifr_ifru.ifru_data = (void *)p->operation_mode;
		break;

	case RMNET_IOCTL_OPEN:              
		rc = __rmnet_open(dev);
		pr_info("rmnet_ioctl(): open transport port\n");
		break;

	case RMNET_IOCTL_CLOSE:             
		rc = __rmnet_close(dev);
		pr_info("rmnet_ioctl(): close transport port\n");
		break;

	default:
		pr_err("error: rmnet_ioct called for unsupported cnd %d", cmd);
		return -EINVAL;
	}

	pr_info("rmnet_ioctl(): dev=%s cmd=0x%x opmode old=0x%08x new=0x%08x\n",
		p->chname, cmd, old_opmode, p->operation_mode);
	return rc;
}


static void __init rmnet_setup(struct net_device *dev)
{
	
	dev->netdev_ops = &rmnet_ops_ether;
	ether_setup(dev);

	
	dev->mtu = RMNET_DATA_LEN;

	random_ether_addr(dev->dev_addr);

	dev->watchdog_timeo = 1000; 
}


static int __init rmnet_init(void)
{
	int ret;
	struct device *d;
	struct net_device *dev;
	struct rmnet_private *p;
	unsigned n;

	printk("%s\n", __func__);

#ifdef CONFIG_MSM_RMNET_DEBUG
	timeout_us = 0;
#ifdef CONFIG_HAS_EARLYSUSPEND
	timeout_suspend_us = 0;
#endif
#endif

	for (n = 0; n < 3; n++) {
		dev = alloc_netdev(sizeof(struct rmnet_private),
				   "rmnet%d", rmnet_setup);

		if (!dev)
			return -ENOMEM;

		d = &(dev->dev);
		p = netdev_priv(dev);
		p->chname = ch_name[n];
		
		p->operation_mode = RMNET_MODE_LLP_ETH;
		p->skb = NULL;
		spin_lock_init(&p->lock);
		tasklet_init(&p->tsklt, _rmnet_resume_flow,
				(unsigned long)dev);
		wake_lock_init(&p->wake_lock, WAKE_LOCK_SUSPEND, ch_name[n]);
#ifdef CONFIG_MSM_RMNET_DEBUG
		p->timeout_us = timeout_us;
		p->wakeups_xmit = p->wakeups_rcv = 0;
#endif

		ret = register_netdev(dev);
		if (ret) {
			free_netdev(dev);
			return ret;
		}

#ifdef CONFIG_MSM_RMNET_DEBUG
		if (device_create_file(d, &dev_attr_timeout))
			continue;
		if (device_create_file(d, &dev_attr_wakeups_xmit))
			continue;
		if (device_create_file(d, &dev_attr_wakeups_rcv))
			continue;
#ifdef CONFIG_HAS_EARLYSUSPEND
		if (device_create_file(d, &dev_attr_timeout_suspend))
			continue;

		
		if (n == 0)
			rmnet0 = d;
#endif
#endif
	}
	return 0;
}

module_init(rmnet_init);
