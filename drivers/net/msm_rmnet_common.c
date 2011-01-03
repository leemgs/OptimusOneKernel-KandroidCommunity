



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

#include "msm_rmnet_sdio.h"


#define RMNET_DATA_LEN 2000

#define DEVICE_ID_INVALID   -1

#define DEVICE_INACTIVE      0
#define DEVICE_ACTIVE        1

#define HEADROOM_FOR_SDIO   8
#define HEADROOM_FOR_QOS    8

struct rmnet_private {
	struct net_device_stats stats;
	uint32_t ch_id;
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
	uint8_t device_up;
};

#ifdef CONFIG_MSM_RMNET_DEBUG
static unsigned long timeout_us;

#ifdef CONFIG_HAS_EARLYSUSPEND

static unsigned long timeout_suspend_us;
static struct device *rmnet0;


static ssize_t timeout_suspend_store(struct device *d,
				     struct device_attribute *attr,
				     const char *buf, size_t n)
{
	timeout_suspend_us = strict_strtoul(buf, NULL, 10);
	return n;
}

static ssize_t timeout_suspend_show(struct device *d,
				    struct device_attribute *attr,
				    char *buf)
{
	return sprintf(buf, "%lu\n", (unsigned long) timeout_suspend_us);
}

static DEVICE_ATTR(timeout_suspend, 0664, timeout_suspend_show,
		   timeout_suspend_store);

static void rmnet_early_suspend(struct early_suspend *handler)
{
	if (rmnet0) {
		struct rmnet_private *p = netdev_priv(to_net_dev(rmnet0));
		p->timeout_us = timeout_suspend_us;
	}
}

static void rmnet_late_resume(struct early_suspend *handler)
{
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


static int rmnet_cause_wakeup(struct rmnet_private *p)
{
	int ret = 0;
	ktime_t now;
	if (p->timeout_us == 0) 
		return 0;

	
	now = ktime_get_real();

	if (ktime_us_delta(now, p->last_packet) > p->timeout_us)
		ret = 1;

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
	p->timeout_us = timeout_us = strict_strtoul(buf, NULL, 10);
#else

	timeout_us = strict_strtoul(buf, NULL, 10);
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



static int rmnet_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

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

static int count_this_packet(void *_hdr, int len)
{
	struct ethhdr *hdr = _hdr;

	if (len >= ETH_HLEN && hdr->h_proto == htons(ETH_P_ARP))
		return 0;

	return 1;
}

static void sdio_recv_notify(void *dev, struct sk_buff *skb)
{
	struct rmnet_private *p = netdev_priv(dev);
	unsigned long flags;
	u32 opmode;

	if (skb) {
		skb->dev = dev;
		
		spin_lock_irqsave(&p->lock, flags);
		opmode = p->operation_mode;
		spin_unlock_irqrestore(&p->lock, flags);

		if (RMNET_IS_MODE_IP(opmode)) {
			
			skb->protocol = rmnet_ip_type_trans(skb, dev);
		} else {
			
			skb->protocol = eth_type_trans(skb, dev);
		}
		if (RMNET_IS_MODE_IP(opmode) ||
		    count_this_packet(skb->data, skb->len)) {
#ifdef CONFIG_MSM_RMNET_DEBUG
			p->wakeups_rcv += rmnet_cause_wakeup(p);
#endif
			p->stats.rx_packets++;
			p->stats.rx_bytes += skb->len;
		}
		netif_rx(skb);
	} else
		pr_err("%s: No skb received", __func__);
}

static int _rmnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	int sdio_ret;
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
	sdio_ret = msm_rmnet_sdio_write(p->ch_id, skb);

	if (sdio_ret != 0) {
		pr_err("%s: write returned error %d", __func__, sdio_ret);
		goto xmit_out;
	}

	if (count_this_packet(skb->data, skb->len)) {
		p->stats.tx_packets++;
		p->stats.tx_bytes += skb->len;
#ifdef CONFIG_MSM_RMNET_DEBUG
		p->wakeups_xmit += rmnet_cause_wakeup(p);
#endif
	}

	return 0;
xmit_out:
	
	dev_kfree_skb_irq(skb);
	return 0;
}

static void sdio_write_done(void *dev, struct sk_buff *skb)
{
	pr_info("%s: write complete\n", __func__);
	dev_kfree_skb_irq(skb);
	netif_wake_queue(dev);
}

static int __rmnet_open(struct net_device *dev)
{
	int r;
	struct rmnet_private *p = netdev_priv(dev);

	pr_info("rmnet_open()\n");

	if (!p->device_up) {
		r = msm_rmnet_sdio_open(p->ch_id, dev,
					sdio_recv_notify, sdio_write_done);

		if (r < 0)
			return -ENODEV;
	}

	p->device_up = DEVICE_ACTIVE;
	return 0;
}

static int rmnet_open(struct net_device *dev)
{
	int rc = 0;

	pr_info("rmnet_open()\n");

	rc = __rmnet_open(dev);

	if (rc == 0)
		netif_start_queue(dev);

	return rc;
}


static int __rmnet_close(struct net_device *dev)
{
	struct rmnet_private *p = netdev_priv(dev);
	int rc = 0;

	if (p->device_up) {
		
		
		p->device_up = DEVICE_INACTIVE;
		return rc;
	} else
		return -EBADF;
}


static int rmnet_stop(struct net_device *dev)
{
	pr_info("rmnet_stop()\n");

	__rmnet_close(dev);
	netif_stop_queue(dev);

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
	if (netif_queue_stopped(dev)) {
		pr_err("fatal: rmnet_xmit called when netif_queue is stopped");
		return 0;
	}

	netif_stop_queue(dev);
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
	.ndo_open = rmnet_open,
	.ndo_stop = rmnet_stop,
	.ndo_start_xmit = rmnet_xmit,
	.ndo_get_stats = rmnet_get_stats,
	.ndo_set_multicast_list = rmnet_set_multicast_list,
	.ndo_tx_timeout = rmnet_tx_timeout,
	.ndo_do_ioctl = rmnet_ioctl,
	.ndo_change_mtu = rmnet_change_mtu,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_validate_addr = eth_validate_addr,
};

static const struct net_device_ops rmnet_ops_ip = {
	.ndo_open = rmnet_open,
	.ndo_stop = rmnet_stop,
	.ndo_start_xmit = rmnet_xmit,
	.ndo_get_stats = rmnet_get_stats,
	.ndo_set_multicast_list = rmnet_set_multicast_list,
	.ndo_tx_timeout = rmnet_tx_timeout,
	.ndo_do_ioctl = rmnet_ioctl,
	.ndo_change_mtu = rmnet_change_mtu,
	.ndo_set_mac_address = 0,
	.ndo_validate_addr = 0,
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

			dev->needed_headroom = HEADROOM_FOR_SDIO +
			  HEADROOM_FOR_QOS;
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

	pr_info("rmnet_ioctl(): dev=%d cmd=0x%x opmode old=0x%08x new=0x%08x\n",
		p->ch_id, cmd, old_opmode, p->operation_mode);
	return rc;
}

static void __init rmnet_setup(struct net_device *dev)
{
	
	dev->netdev_ops = &rmnet_ops_ether;
	ether_setup(dev);

	
	dev->mtu = RMNET_DATA_LEN;
	dev->needed_headroom = HEADROOM_FOR_SDIO + HEADROOM_FOR_QOS ;
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

	printk(KERN_INFO "%s\n", __func__);

#ifdef CONFIG_MSM_RMNET_DEBUG
	timeout_us = 0;
#ifdef CONFIG_HAS_EARLYSUSPEND
	timeout_suspend_us = 0;
#endif
#endif

	for (n = 3; n < 11; n++) {
		dev = alloc_netdev(sizeof(struct rmnet_private),
				   "rmnet%d", rmnet_setup);

		if (!dev)
			return -ENOMEM;

		d = &(dev->dev);
		p = netdev_priv(dev);
		
		p->operation_mode = RMNET_MODE_LLP_ETH;
		p->ch_id = n - 3;
		spin_lock_init(&p->lock);
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
MODULE_DESCRIPTION("MSM RMNET SDIO TRANSPORT");
MODULE_LICENSE("GPL v2");

