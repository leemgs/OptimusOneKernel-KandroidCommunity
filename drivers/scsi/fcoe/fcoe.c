

#include <linux/module.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsicam.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_fc.h>
#include <net/rtnetlink.h>

#include <scsi/fc/fc_encaps.h>
#include <scsi/fc/fc_fip.h>

#include <scsi/libfc.h>
#include <scsi/fc_frame.h>
#include <scsi/libfcoe.h>

#include "fcoe.h"

MODULE_AUTHOR("Open-FCoE.org");
MODULE_DESCRIPTION("FCoE");
MODULE_LICENSE("GPL v2");


static unsigned int fcoe_ddp_min;
module_param_named(ddp_min, fcoe_ddp_min, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ddp_min, "Minimum I/O size in bytes for "	\
		 "Direct Data Placement (DDP).");

DEFINE_MUTEX(fcoe_config_mutex);


static DECLARE_COMPLETION(fcoe_flush_completion);



LIST_HEAD(fcoe_hostlist);
DEFINE_PER_CPU(struct fcoe_percpu_s, fcoe_percpu);


static int fcoe_reset(struct Scsi_Host *shost);
static int fcoe_xmit(struct fc_lport *, struct fc_frame *);
static int fcoe_rcv(struct sk_buff *, struct net_device *,
		    struct packet_type *, struct net_device *);
static int fcoe_percpu_receive_thread(void *arg);
static void fcoe_clean_pending_queue(struct fc_lport *lp);
static void fcoe_percpu_clean(struct fc_lport *lp);
static int fcoe_link_ok(struct fc_lport *lp);

static struct fc_lport *fcoe_hostlist_lookup(const struct net_device *);
static int fcoe_hostlist_add(const struct fc_lport *);

static void fcoe_check_wait_queue(struct fc_lport *, struct sk_buff *);
static int fcoe_device_notification(struct notifier_block *, ulong, void *);
static void fcoe_dev_setup(void);
static void fcoe_dev_cleanup(void);
static struct fcoe_interface *
	fcoe_hostlist_lookup_port(const struct net_device *dev);


static struct notifier_block fcoe_notifier = {
	.notifier_call = fcoe_device_notification,
};

static struct scsi_transport_template *scsi_transport_fcoe_sw;

struct fc_function_template fcoe_transport_function = {
	.show_host_node_name = 1,
	.show_host_port_name = 1,
	.show_host_supported_classes = 1,
	.show_host_supported_fc4s = 1,
	.show_host_active_fc4s = 1,
	.show_host_maxframe_size = 1,

	.show_host_port_id = 1,
	.show_host_supported_speeds = 1,
	.get_host_speed = fc_get_host_speed,
	.show_host_speed = 1,
	.show_host_port_type = 1,
	.get_host_port_state = fc_get_host_port_state,
	.show_host_port_state = 1,
	.show_host_symbolic_name = 1,

	.dd_fcrport_size = sizeof(struct fc_rport_libfc_priv),
	.show_rport_maxframe_size = 1,
	.show_rport_supported_classes = 1,

	.show_host_fabric_name = 1,
	.show_starget_node_name = 1,
	.show_starget_port_name = 1,
	.show_starget_port_id = 1,
	.set_rport_dev_loss_tmo = fc_set_rport_loss_tmo,
	.show_rport_dev_loss_tmo = 1,
	.get_fc_host_stats = fc_get_host_stats,
	.issue_fc_host_lip = fcoe_reset,

	.terminate_rport_io = fc_rport_terminate_io,
};

static struct scsi_host_template fcoe_shost_template = {
	.module = THIS_MODULE,
	.name = "FCoE Driver",
	.proc_name = FCOE_NAME,
	.queuecommand = fc_queuecommand,
	.eh_abort_handler = fc_eh_abort,
	.eh_device_reset_handler = fc_eh_device_reset,
	.eh_host_reset_handler = fc_eh_host_reset,
	.slave_alloc = fc_slave_alloc,
	.change_queue_depth = fc_change_queue_depth,
	.change_queue_type = fc_change_queue_type,
	.this_id = -1,
	.cmd_per_lun = 3,
	.can_queue = FCOE_MAX_OUTSTANDING_COMMANDS,
	.use_clustering = ENABLE_CLUSTERING,
	.sg_tablesize = SG_ALL,
	.max_sectors = 0xffff,
};

static int fcoe_fip_recv(struct sk_buff *skb, struct net_device *dev,
			 struct packet_type *ptype,
			 struct net_device *orig_dev);

static int fcoe_interface_setup(struct fcoe_interface *fcoe,
				struct net_device *netdev)
{
	struct fcoe_ctlr *fip = &fcoe->ctlr;
	struct netdev_hw_addr *ha;
	struct net_device *real_dev;
	u8 flogi_maddr[ETH_ALEN];

	fcoe->netdev = netdev;

	
	if ((netdev->priv_flags & IFF_MASTER_ALB) ||
	    (netdev->priv_flags & IFF_SLAVE_INACTIVE) ||
	    (netdev->priv_flags & IFF_MASTER_8023AD)) {
		return -EOPNOTSUPP;
	}

	
	real_dev = (netdev->priv_flags & IFF_802_1Q_VLAN) ?
		vlan_dev_real_dev(netdev) : netdev;
	rcu_read_lock();
	for_each_dev_addr(real_dev, ha) {
		if ((ha->type == NETDEV_HW_ADDR_T_SAN) &&
		    (is_valid_ether_addr(ha->addr))) {
			memcpy(fip->ctl_src_addr, ha->addr, ETH_ALEN);
			fip->spma = 1;
			break;
		}
	}
	rcu_read_unlock();

	
	if (!fip->spma)
		memcpy(fip->ctl_src_addr, netdev->dev_addr, netdev->addr_len);

	
	memcpy(flogi_maddr, (u8[6]) FC_FCOE_FLOGI_MAC, ETH_ALEN);
	dev_unicast_add(netdev, flogi_maddr);
	if (fip->spma)
		dev_unicast_add(netdev, fip->ctl_src_addr);
	dev_mc_add(netdev, FIP_ALL_ENODE_MACS, ETH_ALEN, 0);

	
	fcoe->fcoe_packet_type.func = fcoe_rcv;
	fcoe->fcoe_packet_type.type = __constant_htons(ETH_P_FCOE);
	fcoe->fcoe_packet_type.dev = netdev;
	dev_add_pack(&fcoe->fcoe_packet_type);

	fcoe->fip_packet_type.func = fcoe_fip_recv;
	fcoe->fip_packet_type.type = htons(ETH_P_FIP);
	fcoe->fip_packet_type.dev = netdev;
	dev_add_pack(&fcoe->fip_packet_type);

	return 0;
}

static void fcoe_fip_send(struct fcoe_ctlr *fip, struct sk_buff *skb);
static void fcoe_update_src_mac(struct fcoe_ctlr *fip, u8 *old, u8 *new);
static void fcoe_destroy_work(struct work_struct *work);


static struct fcoe_interface *fcoe_interface_create(struct net_device *netdev)
{
	struct fcoe_interface *fcoe;

	fcoe = kzalloc(sizeof(*fcoe), GFP_KERNEL);
	if (!fcoe) {
		FCOE_NETDEV_DBG(netdev, "Could not allocate fcoe structure\n");
		return NULL;
	}

	dev_hold(netdev);
	kref_init(&fcoe->kref);

	
	fcoe_ctlr_init(&fcoe->ctlr);
	fcoe->ctlr.send = fcoe_fip_send;
	fcoe->ctlr.update_mac = fcoe_update_src_mac;

	fcoe_interface_setup(fcoe, netdev);

	return fcoe;
}


void fcoe_interface_cleanup(struct fcoe_interface *fcoe)
{
	struct net_device *netdev = fcoe->netdev;
	struct fcoe_ctlr *fip = &fcoe->ctlr;
	u8 flogi_maddr[ETH_ALEN];

	
	__dev_remove_pack(&fcoe->fcoe_packet_type);
	__dev_remove_pack(&fcoe->fip_packet_type);
	synchronize_net();

	
	memcpy(flogi_maddr, (u8[6]) FC_FCOE_FLOGI_MAC, ETH_ALEN);
	dev_unicast_delete(netdev, flogi_maddr);
	if (!is_zero_ether_addr(fip->data_src_addr))
		dev_unicast_delete(netdev, fip->data_src_addr);
	if (fip->spma)
		dev_unicast_delete(netdev, fip->ctl_src_addr);
	dev_mc_delete(netdev, FIP_ALL_ENODE_MACS, ETH_ALEN, 0);
}


static void fcoe_interface_release(struct kref *kref)
{
	struct fcoe_interface *fcoe;
	struct net_device *netdev;

	fcoe = container_of(kref, struct fcoe_interface, kref);
	netdev = fcoe->netdev;
	
	fcoe_ctlr_destroy(&fcoe->ctlr);
	kfree(fcoe);
	dev_put(netdev);
}


static inline void fcoe_interface_get(struct fcoe_interface *fcoe)
{
	kref_get(&fcoe->kref);
}


static inline void fcoe_interface_put(struct fcoe_interface *fcoe)
{
	kref_put(&fcoe->kref, fcoe_interface_release);
}


static int fcoe_fip_recv(struct sk_buff *skb, struct net_device *dev,
			 struct packet_type *ptype,
			 struct net_device *orig_dev)
{
	struct fcoe_interface *fcoe;

	fcoe = container_of(ptype, struct fcoe_interface, fip_packet_type);
	fcoe_ctlr_recv(&fcoe->ctlr, skb);
	return 0;
}


static void fcoe_fip_send(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	skb->dev = fcoe_from_ctlr(fip)->netdev;
	dev_queue_xmit(skb);
}


static void fcoe_update_src_mac(struct fcoe_ctlr *fip, u8 *old, u8 *new)
{
	struct fcoe_interface *fcoe;

	fcoe = fcoe_from_ctlr(fip);
	rtnl_lock();
	if (!is_zero_ether_addr(old))
		dev_unicast_delete(fcoe->netdev, old);
	dev_unicast_add(fcoe->netdev, new);
	rtnl_unlock();
}


static int fcoe_lport_config(struct fc_lport *lp)
{
	lp->link_up = 0;
	lp->qfull = 0;
	lp->max_retry_count = 3;
	lp->max_rport_retry_count = 3;
	lp->e_d_tov = 2 * 1000;	
	lp->r_a_tov = 2 * 2 * 1000;
	lp->service_params = (FCP_SPPF_INIT_FCN | FCP_SPPF_RD_XRDY_DIS |
			      FCP_SPPF_RETRY | FCP_SPPF_CONF_COMPL);

	fc_lport_init_stats(lp);

	
	fc_lport_config(lp);

	
	lp->crc_offload = 0;
	lp->seq_offload = 0;
	lp->lro_enabled = 0;
	lp->lro_xid = 0;
	lp->lso_max = 0;

	return 0;
}


static void fcoe_queue_timer(ulong lp)
{
	fcoe_check_wait_queue((struct fc_lport *)lp, NULL);
}


static int fcoe_netdev_config(struct fc_lport *lp, struct net_device *netdev)
{
	u32 mfs;
	u64 wwnn, wwpn;
	struct fcoe_interface *fcoe;
	struct fcoe_port *port;

	
	port = lport_priv(lp);
	fcoe = port->fcoe;

	
	mfs = netdev->mtu - (sizeof(struct fcoe_hdr) +
			     sizeof(struct fcoe_crc_eof));
	if (fc_set_mfs(lp, mfs))
		return -EINVAL;

	
	if (netdev->features & NETIF_F_SG)
		lp->sg_supp = 1;

	if (netdev->features & NETIF_F_FCOE_CRC) {
		lp->crc_offload = 1;
		FCOE_NETDEV_DBG(netdev, "Supports FCCRC offload\n");
	}
	if (netdev->features & NETIF_F_FSO) {
		lp->seq_offload = 1;
		lp->lso_max = netdev->gso_max_size;
		FCOE_NETDEV_DBG(netdev, "Supports LSO for max len 0x%x\n",
				lp->lso_max);
	}
	if (netdev->fcoe_ddp_xid) {
		lp->lro_enabled = 1;
		lp->lro_xid = netdev->fcoe_ddp_xid;
		FCOE_NETDEV_DBG(netdev, "Supports LRO for max xid 0x%x\n",
				lp->lro_xid);
	}
	skb_queue_head_init(&port->fcoe_pending_queue);
	port->fcoe_pending_queue_active = 0;
	setup_timer(&port->timer, fcoe_queue_timer, (unsigned long)lp);

	wwnn = fcoe_wwn_from_mac(netdev->dev_addr, 1, 0);
	fc_set_wwnn(lp, wwnn);
	
	wwpn = fcoe_wwn_from_mac(netdev->dev_addr, 2, 0);
	fc_set_wwpn(lp, wwpn);

	return 0;
}


static int fcoe_shost_config(struct fc_lport *lp, struct Scsi_Host *shost,
				struct device *dev)
{
	int rc = 0;

	
	lp->host = shost;

	lp->host->max_lun = FCOE_MAX_LUN;
	lp->host->max_id = FCOE_MAX_FCP_TARGET;
	lp->host->max_channel = 0;
	lp->host->transportt = scsi_transport_fcoe_sw;

	
	rc = scsi_add_host(lp->host, dev);
	if (rc) {
		FCOE_NETDEV_DBG(fcoe_netdev(lp), "fcoe_shost_config: "
				"error on scsi_add_host\n");
		return rc;
	}
	sprintf(fc_host_symbolic_name(lp->host), "%s v%s over %s",
		FCOE_NAME, FCOE_VERSION,
		fcoe_netdev(lp)->name);

	return 0;
}


bool fcoe_oem_match(struct fc_frame *fp)
{
	return fc_fcp_is_read(fr_fsp(fp)) &&
		(fr_fsp(fp)->data_len > fcoe_ddp_min);
}


static inline int fcoe_em_config(struct fc_lport *lp)
{
	struct fcoe_port *port = lport_priv(lp);
	struct fcoe_interface *fcoe = port->fcoe;
	struct fcoe_interface *oldfcoe = NULL;
	struct net_device *old_real_dev, *cur_real_dev;
	u16 min_xid = FCOE_MIN_XID;
	u16 max_xid = FCOE_MAX_XID;

	
	if (!lp->lro_enabled || !lp->lro_xid || (lp->lro_xid >= max_xid)) {
		lp->lro_xid = 0;
		goto skip_oem;
	}

	
	if (fcoe->netdev->priv_flags & IFF_802_1Q_VLAN)
		cur_real_dev = vlan_dev_real_dev(fcoe->netdev);
	else
		cur_real_dev = fcoe->netdev;

	list_for_each_entry(oldfcoe, &fcoe_hostlist, list) {
		if (oldfcoe->netdev->priv_flags & IFF_802_1Q_VLAN)
			old_real_dev = vlan_dev_real_dev(oldfcoe->netdev);
		else
			old_real_dev = oldfcoe->netdev;

		if (cur_real_dev == old_real_dev) {
			fcoe->oem = oldfcoe->oem;
			break;
		}
	}

	if (fcoe->oem) {
		if (!fc_exch_mgr_add(lp, fcoe->oem, fcoe_oem_match)) {
			printk(KERN_ERR "fcoe_em_config: failed to add "
			       "offload em:%p on interface:%s\n",
			       fcoe->oem, fcoe->netdev->name);
			return -ENOMEM;
		}
	} else {
		fcoe->oem = fc_exch_mgr_alloc(lp, FC_CLASS_3,
					    FCOE_MIN_XID, lp->lro_xid,
					    fcoe_oem_match);
		if (!fcoe->oem) {
			printk(KERN_ERR "fcoe_em_config: failed to allocate "
			       "em for offload exches on interface:%s\n",
			       fcoe->netdev->name);
			return -ENOMEM;
		}
	}

	
	min_xid += lp->lro_xid + 1;

skip_oem:
	if (!fc_exch_mgr_alloc(lp, FC_CLASS_3, min_xid, max_xid, NULL)) {
		printk(KERN_ERR "fcoe_em_config: failed to "
		       "allocate em on interface %s\n", fcoe->netdev->name);
		return -ENOMEM;
	}

	return 0;
}


static void fcoe_if_destroy(struct fc_lport *lport)
{
	struct fcoe_port *port = lport_priv(lport);
	struct fcoe_interface *fcoe = port->fcoe;
	struct net_device *netdev = fcoe->netdev;

	FCOE_NETDEV_DBG(netdev, "Destroying interface\n");

	
	fc_fabric_logoff(lport);

	
	fc_lport_destroy(lport);
	fc_fcp_destroy(lport);

	
	del_timer_sync(&port->timer);

	
	fcoe_clean_pending_queue(lport);

	
	fcoe_interface_put(fcoe);

	
	fcoe_percpu_clean(lport);

	
	fc_remove_host(lport->host);
	scsi_remove_host(lport->host);

	
	fc_exch_mgr_free(lport);

	
	fc_lport_free_stats(lport);

	
	scsi_host_put(lport->host);
}


static int fcoe_ddp_setup(struct fc_lport *lp, u16 xid,
			     struct scatterlist *sgl, unsigned int sgc)
{
	struct net_device *n = fcoe_netdev(lp);

	if (n->netdev_ops->ndo_fcoe_ddp_setup)
		return n->netdev_ops->ndo_fcoe_ddp_setup(n, xid, sgl, sgc);

	return 0;
}


static int fcoe_ddp_done(struct fc_lport *lp, u16 xid)
{
	struct net_device *n = fcoe_netdev(lp);

	if (n->netdev_ops->ndo_fcoe_ddp_done)
		return n->netdev_ops->ndo_fcoe_ddp_done(n, xid);
	return 0;
}

static struct libfc_function_template fcoe_libfc_fcn_templ = {
	.frame_send = fcoe_xmit,
	.ddp_setup = fcoe_ddp_setup,
	.ddp_done = fcoe_ddp_done,
};


static struct fc_lport *fcoe_if_create(struct fcoe_interface *fcoe,
				       struct device *parent)
{
	int rc;
	struct fc_lport *lport = NULL;
	struct fcoe_port *port;
	struct Scsi_Host *shost;
	struct net_device *netdev = fcoe->netdev;

	FCOE_NETDEV_DBG(netdev, "Create Interface\n");

	shost = libfc_host_alloc(&fcoe_shost_template,
				 sizeof(struct fcoe_port));
	if (!shost) {
		FCOE_NETDEV_DBG(netdev, "Could not allocate host structure\n");
		rc = -ENOMEM;
		goto out;
	}
	lport = shost_priv(shost);
	port = lport_priv(lport);
	port->lport = lport;
	port->fcoe = fcoe;
	INIT_WORK(&port->destroy_work, fcoe_destroy_work);

	
	rc = fcoe_lport_config(lport);
	if (rc) {
		FCOE_NETDEV_DBG(netdev, "Could not configure lport for the "
				"interface\n");
		goto out_host_put;
	}

	
	rc = fcoe_netdev_config(lport, netdev);
	if (rc) {
		FCOE_NETDEV_DBG(netdev, "Could not configure netdev for the "
				"interface\n");
		goto out_lp_destroy;
	}

	
	rc = fcoe_shost_config(lport, shost, parent);
	if (rc) {
		FCOE_NETDEV_DBG(netdev, "Could not configure shost for the "
				"interface\n");
		goto out_lp_destroy;
	}

	
	rc = fcoe_libfc_config(lport, &fcoe_libfc_fcn_templ);
	if (rc) {
		FCOE_NETDEV_DBG(netdev, "Could not configure libfc for the "
				"interface\n");
		goto out_lp_destroy;
	}

	

	
	rc = fcoe_em_config(lport);
	if (rc) {
		FCOE_NETDEV_DBG(netdev, "Could not configure the EM for the "
				"interface\n");
		goto out_lp_destroy;
	}

	fcoe_interface_get(fcoe);
	return lport;

out_lp_destroy:
	fc_exch_mgr_free(lport);
out_host_put:
	scsi_host_put(lport->host);
out:
	return ERR_PTR(rc);
}


static int __init fcoe_if_init(void)
{
	
	scsi_transport_fcoe_sw =
		fc_attach_transport(&fcoe_transport_function);

	if (!scsi_transport_fcoe_sw) {
		printk(KERN_ERR "fcoe: Failed to attach to the FC transport\n");
		return -ENODEV;
	}

	return 0;
}


int __exit fcoe_if_exit(void)
{
	fc_release_transport(scsi_transport_fcoe_sw);
	scsi_transport_fcoe_sw = NULL;
	return 0;
}


static void fcoe_percpu_thread_create(unsigned int cpu)
{
	struct fcoe_percpu_s *p;
	struct task_struct *thread;

	p = &per_cpu(fcoe_percpu, cpu);

	thread = kthread_create(fcoe_percpu_receive_thread,
				(void *)p, "fcoethread/%d", cpu);

	if (likely(!IS_ERR(thread))) {
		kthread_bind(thread, cpu);
		wake_up_process(thread);

		spin_lock_bh(&p->fcoe_rx_list.lock);
		p->thread = thread;
		spin_unlock_bh(&p->fcoe_rx_list.lock);
	}
}


static void fcoe_percpu_thread_destroy(unsigned int cpu)
{
	struct fcoe_percpu_s *p;
	struct task_struct *thread;
	struct page *crc_eof;
	struct sk_buff *skb;
#ifdef CONFIG_SMP
	struct fcoe_percpu_s *p0;
	unsigned targ_cpu = smp_processor_id();
#endif 

	FCOE_DBG("Destroying receive thread for CPU %d\n", cpu);

	
	p = &per_cpu(fcoe_percpu, cpu);
	spin_lock_bh(&p->fcoe_rx_list.lock);
	thread = p->thread;
	p->thread = NULL;
	crc_eof = p->crc_eof_page;
	p->crc_eof_page = NULL;
	p->crc_eof_offset = 0;
	spin_unlock_bh(&p->fcoe_rx_list.lock);

#ifdef CONFIG_SMP
	
	if (cpu != targ_cpu) {
		p0 = &per_cpu(fcoe_percpu, targ_cpu);
		spin_lock_bh(&p0->fcoe_rx_list.lock);
		if (p0->thread) {
			FCOE_DBG("Moving frames from CPU %d to CPU %d\n",
				 cpu, targ_cpu);

			while ((skb = __skb_dequeue(&p->fcoe_rx_list)) != NULL)
				__skb_queue_tail(&p0->fcoe_rx_list, skb);
			spin_unlock_bh(&p0->fcoe_rx_list.lock);
		} else {
			
			while ((skb = __skb_dequeue(&p->fcoe_rx_list)) != NULL)
				kfree_skb(skb);
			spin_unlock_bh(&p0->fcoe_rx_list.lock);
		}
	} else {
		
		spin_lock_bh(&p->fcoe_rx_list.lock);
		while ((skb = __skb_dequeue(&p->fcoe_rx_list)) != NULL)
			kfree_skb(skb);
		spin_unlock_bh(&p->fcoe_rx_list.lock);
	}
#else
	
	spin_lock_bh(&p->fcoe_rx_list.lock);
	while ((skb = __skb_dequeue(&p->fcoe_rx_list)) != NULL)
		kfree_skb(skb);
	spin_unlock_bh(&p->fcoe_rx_list.lock);
#endif

	if (thread)
		kthread_stop(thread);

	if (crc_eof)
		put_page(crc_eof);
}


static int fcoe_cpu_callback(struct notifier_block *nfb,
			     unsigned long action, void *hcpu)
{
	unsigned cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		FCOE_DBG("CPU %x online: Create Rx thread\n", cpu);
		fcoe_percpu_thread_create(cpu);
		break;
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		FCOE_DBG("CPU %x offline: Remove Rx thread\n", cpu);
		fcoe_percpu_thread_destroy(cpu);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block fcoe_cpu_notifier = {
	.notifier_call = fcoe_cpu_callback,
};


int fcoe_rcv(struct sk_buff *skb, struct net_device *dev,
	     struct packet_type *ptype, struct net_device *olddev)
{
	struct fc_lport *lp;
	struct fcoe_rcv_info *fr;
	struct fcoe_interface *fcoe;
	struct fc_frame_header *fh;
	struct fcoe_percpu_s *fps;
	unsigned int cpu;

	fcoe = container_of(ptype, struct fcoe_interface, fcoe_packet_type);
	lp = fcoe->ctlr.lp;
	if (unlikely(lp == NULL)) {
		FCOE_NETDEV_DBG(dev, "Cannot find hba structure");
		goto err2;
	}
	if (!lp->link_up)
		goto err2;

	FCOE_NETDEV_DBG(dev, "skb_info: len:%d data_len:%d head:%p "
			"data:%p tail:%p end:%p sum:%d dev:%s",
			skb->len, skb->data_len, skb->head, skb->data,
			skb_tail_pointer(skb), skb_end_pointer(skb),
			skb->csum, skb->dev ? skb->dev->name : "<NULL>");

	
	if (unlikely(eth_hdr(skb)->h_proto != htons(ETH_P_FCOE))) {
		FCOE_NETDEV_DBG(dev, "Wrong FC type frame");
		goto err;
	}

	
	if (unlikely((skb->len < FCOE_MIN_FRAME) ||
	    !pskb_may_pull(skb, FCOE_HEADER_LEN)))
		goto err;

	skb_set_transport_header(skb, sizeof(struct fcoe_hdr));
	fh = (struct fc_frame_header *) skb_transport_header(skb);

	fr = fcoe_dev_from_skb(skb);
	fr->fr_dev = lp;
	fr->ptype = ptype;

	
	if (ntoh24(fh->fh_f_ctl) & FC_FC_EX_CTX)
		cpu = ntohs(fh->fh_ox_id) & fc_cpu_mask;
	else
		cpu = smp_processor_id();

	fps = &per_cpu(fcoe_percpu, cpu);
	spin_lock_bh(&fps->fcoe_rx_list.lock);
	if (unlikely(!fps->thread)) {
		
		FCOE_NETDEV_DBG(dev, "CPU is online, but no receive thread "
				"ready for incoming skb- using first online "
				"CPU.\n");

		spin_unlock_bh(&fps->fcoe_rx_list.lock);
		cpu = first_cpu(cpu_online_map);
		fps = &per_cpu(fcoe_percpu, cpu);
		spin_lock_bh(&fps->fcoe_rx_list.lock);
		if (!fps->thread) {
			spin_unlock_bh(&fps->fcoe_rx_list.lock);
			goto err;
		}
	}

	
	__skb_queue_tail(&fps->fcoe_rx_list, skb);
	if (fps->fcoe_rx_list.qlen == 1)
		wake_up_process(fps->thread);

	spin_unlock_bh(&fps->fcoe_rx_list.lock);

	return 0;
err:
	fc_lport_get_stats(lp)->ErrorFrames++;

err2:
	kfree_skb(skb);
	return -1;
}


static inline int fcoe_start_io(struct sk_buff *skb)
{
	int rc;

	skb_get(skb);
	rc = dev_queue_xmit(skb);
	if (rc != 0)
		return rc;
	kfree_skb(skb);
	return 0;
}


static int fcoe_get_paged_crc_eof(struct sk_buff *skb, int tlen)
{
	struct fcoe_percpu_s *fps;
	struct page *page;

	fps = &get_cpu_var(fcoe_percpu);
	page = fps->crc_eof_page;
	if (!page) {
		page = alloc_page(GFP_ATOMIC);
		if (!page) {
			put_cpu_var(fcoe_percpu);
			return -ENOMEM;
		}
		fps->crc_eof_page = page;
		fps->crc_eof_offset = 0;
	}

	get_page(page);
	skb_fill_page_desc(skb, skb_shinfo(skb)->nr_frags, page,
			   fps->crc_eof_offset, tlen);
	skb->len += tlen;
	skb->data_len += tlen;
	skb->truesize += tlen;
	fps->crc_eof_offset += sizeof(struct fcoe_crc_eof);

	if (fps->crc_eof_offset >= PAGE_SIZE) {
		fps->crc_eof_page = NULL;
		fps->crc_eof_offset = 0;
		put_page(page);
	}
	put_cpu_var(fcoe_percpu);
	return 0;
}


u32 fcoe_fc_crc(struct fc_frame *fp)
{
	struct sk_buff *skb = fp_skb(fp);
	struct skb_frag_struct *frag;
	unsigned char *data;
	unsigned long off, len, clen;
	u32 crc;
	unsigned i;

	crc = crc32(~0, skb->data, skb_headlen(skb));

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		frag = &skb_shinfo(skb)->frags[i];
		off = frag->page_offset;
		len = frag->size;
		while (len > 0) {
			clen = min(len, PAGE_SIZE - (off & ~PAGE_MASK));
			data = kmap_atomic(frag->page + (off >> PAGE_SHIFT),
					   KM_SKB_DATA_SOFTIRQ);
			crc = crc32(crc, data + (off & ~PAGE_MASK), clen);
			kunmap_atomic(data, KM_SKB_DATA_SOFTIRQ);
			off += clen;
			len -= clen;
		}
	}
	return crc;
}


int fcoe_xmit(struct fc_lport *lp, struct fc_frame *fp)
{
	int wlen;
	u32 crc;
	struct ethhdr *eh;
	struct fcoe_crc_eof *cp;
	struct sk_buff *skb;
	struct fcoe_dev_stats *stats;
	struct fc_frame_header *fh;
	unsigned int hlen;		
	unsigned int tlen;		
	unsigned int elen;		
	struct fcoe_port *port = lport_priv(lp);
	struct fcoe_interface *fcoe = port->fcoe;
	u8 sof, eof;
	struct fcoe_hdr *hp;

	WARN_ON((fr_len(fp) % sizeof(u32)) != 0);

	fh = fc_frame_header_get(fp);
	skb = fp_skb(fp);
	wlen = skb->len / FCOE_WORD_TO_BYTE;

	if (!lp->link_up) {
		kfree_skb(skb);
		return 0;
	}

	if (unlikely(fh->fh_r_ctl == FC_RCTL_ELS_REQ) &&
	    fcoe_ctlr_els_send(&fcoe->ctlr, skb))
		return 0;

	sof = fr_sof(fp);
	eof = fr_eof(fp);

	elen = sizeof(struct ethhdr);
	hlen = sizeof(struct fcoe_hdr);
	tlen = sizeof(struct fcoe_crc_eof);
	wlen = (skb->len - tlen + sizeof(crc)) / FCOE_WORD_TO_BYTE;

	
	if (likely(lp->crc_offload)) {
		skb->ip_summed = CHECKSUM_PARTIAL;
		skb->csum_start = skb_headroom(skb);
		skb->csum_offset = skb->len;
		crc = 0;
	} else {
		skb->ip_summed = CHECKSUM_NONE;
		crc = fcoe_fc_crc(fp);
	}

	
	if (skb_is_nonlinear(skb)) {
		skb_frag_t *frag;
		if (fcoe_get_paged_crc_eof(skb, tlen)) {
			kfree_skb(skb);
			return -ENOMEM;
		}
		frag = &skb_shinfo(skb)->frags[skb_shinfo(skb)->nr_frags - 1];
		cp = kmap_atomic(frag->page, KM_SKB_DATA_SOFTIRQ)
			+ frag->page_offset;
	} else {
		cp = (struct fcoe_crc_eof *)skb_put(skb, tlen);
	}

	memset(cp, 0, sizeof(*cp));
	cp->fcoe_eof = eof;
	cp->fcoe_crc32 = cpu_to_le32(~crc);

	if (skb_is_nonlinear(skb)) {
		kunmap_atomic(cp, KM_SKB_DATA_SOFTIRQ);
		cp = NULL;
	}

	
	skb_push(skb, elen + hlen);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb->mac_len = elen;
	skb->protocol = htons(ETH_P_FCOE);
	skb->dev = fcoe->netdev;

	
	eh = eth_hdr(skb);
	eh->h_proto = htons(ETH_P_FCOE);
	if (fcoe->ctlr.map_dest)
		fc_fcoe_set_mac(eh->h_dest, fh->fh_d_id);
	else
		
		memcpy(eh->h_dest, fcoe->ctlr.dest_addr, ETH_ALEN);

	if (unlikely(fcoe->ctlr.flogi_oxid != FC_XID_UNKNOWN))
		memcpy(eh->h_source, fcoe->ctlr.ctl_src_addr, ETH_ALEN);
	else
		memcpy(eh->h_source, fcoe->ctlr.data_src_addr, ETH_ALEN);

	hp = (struct fcoe_hdr *)(eh + 1);
	memset(hp, 0, sizeof(*hp));
	if (FC_FCOE_VER)
		FC_FCOE_ENCAPS_VER(hp, FC_FCOE_VER);
	hp->fcoe_sof = sof;

	
	if (lp->seq_offload && fr_max_payload(fp)) {
		skb_shinfo(skb)->gso_type = SKB_GSO_FCOE;
		skb_shinfo(skb)->gso_size = fr_max_payload(fp);
	} else {
		skb_shinfo(skb)->gso_type = 0;
		skb_shinfo(skb)->gso_size = 0;
	}
	
	stats = fc_lport_get_stats(lp);
	stats->TxFrames++;
	stats->TxWords += wlen;

	
	fr_dev(fp) = lp;
	if (port->fcoe_pending_queue.qlen)
		fcoe_check_wait_queue(lp, skb);
	else if (fcoe_start_io(skb))
		fcoe_check_wait_queue(lp, skb);

	return 0;
}


static void fcoe_percpu_flush_done(struct sk_buff *skb)
{
	complete(&fcoe_flush_completion);
}


int fcoe_percpu_receive_thread(void *arg)
{
	struct fcoe_percpu_s *p = arg;
	u32 fr_len;
	struct fc_lport *lp;
	struct fcoe_rcv_info *fr;
	struct fcoe_dev_stats *stats;
	struct fc_frame_header *fh;
	struct sk_buff *skb;
	struct fcoe_crc_eof crc_eof;
	struct fc_frame *fp;
	u8 *mac = NULL;
	struct fcoe_port *port;
	struct fcoe_hdr *hp;

	set_user_nice(current, -20);

	while (!kthread_should_stop()) {

		spin_lock_bh(&p->fcoe_rx_list.lock);
		while ((skb = __skb_dequeue(&p->fcoe_rx_list)) == NULL) {
			set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_bh(&p->fcoe_rx_list.lock);
			schedule();
			set_current_state(TASK_RUNNING);
			if (kthread_should_stop())
				return 0;
			spin_lock_bh(&p->fcoe_rx_list.lock);
		}
		spin_unlock_bh(&p->fcoe_rx_list.lock);
		fr = fcoe_dev_from_skb(skb);
		lp = fr->fr_dev;
		if (unlikely(lp == NULL)) {
			if (skb->destructor != fcoe_percpu_flush_done)
				FCOE_NETDEV_DBG(skb->dev, "NULL lport in skb");
			kfree_skb(skb);
			continue;
		}

		FCOE_NETDEV_DBG(skb->dev, "skb_info: len:%d data_len:%d "
				"head:%p data:%p tail:%p end:%p sum:%d dev:%s",
				skb->len, skb->data_len,
				skb->head, skb->data, skb_tail_pointer(skb),
				skb_end_pointer(skb), skb->csum,
				skb->dev ? skb->dev->name : "<NULL>");

		
		port = lport_priv(lp);
		if (skb_is_nonlinear(skb))
			skb_linearize(skb);	
		mac = eth_hdr(skb)->h_source;

		
		hp = (struct fcoe_hdr *) skb_network_header(skb);
		fh = (struct fc_frame_header *) skb_transport_header(skb);

		stats = fc_lport_get_stats(lp);
		if (unlikely(FC_FCOE_DECAPS_VER(hp) != FC_FCOE_VER)) {
			if (stats->ErrorFrames < 5)
				printk(KERN_WARNING "fcoe: FCoE version "
				       "mismatch: The frame has "
				       "version %x, but the "
				       "initiator supports version "
				       "%x\n", FC_FCOE_DECAPS_VER(hp),
				       FC_FCOE_VER);
			stats->ErrorFrames++;
			kfree_skb(skb);
			continue;
		}

		skb_pull(skb, sizeof(struct fcoe_hdr));
		fr_len = skb->len - sizeof(struct fcoe_crc_eof);

		stats->RxFrames++;
		stats->RxWords += fr_len / FCOE_WORD_TO_BYTE;

		fp = (struct fc_frame *)skb;
		fc_frame_init(fp);
		fr_dev(fp) = lp;
		fr_sof(fp) = hp->fcoe_sof;

		
		if (skb_copy_bits(skb, fr_len, &crc_eof, sizeof(crc_eof))) {
			kfree_skb(skb);
			continue;
		}
		fr_eof(fp) = crc_eof.fcoe_eof;
		fr_crc(fp) = crc_eof.fcoe_crc32;
		if (pskb_trim(skb, fr_len)) {
			kfree_skb(skb);
			continue;
		}

		
		if (lp->crc_offload && skb->ip_summed == CHECKSUM_UNNECESSARY)
			fr_flags(fp) &= ~FCPHF_CRC_UNCHECKED;
		else
			fr_flags(fp) |= FCPHF_CRC_UNCHECKED;

		fh = fc_frame_header_get(fp);
		if (fh->fh_r_ctl == FC_RCTL_DD_SOL_DATA &&
		    fh->fh_type == FC_TYPE_FCP) {
			fc_exch_recv(lp, fp);
			continue;
		}
		if (fr_flags(fp) & FCPHF_CRC_UNCHECKED) {
			if (le32_to_cpu(fr_crc(fp)) !=
			    ~crc32(~0, skb->data, fr_len)) {
				if (stats->InvalidCRCCount < 5)
					printk(KERN_WARNING "fcoe: dropping "
					       "frame with CRC error\n");
				stats->InvalidCRCCount++;
				stats->ErrorFrames++;
				fc_frame_free(fp);
				continue;
			}
			fr_flags(fp) &= ~FCPHF_CRC_UNCHECKED;
		}
		if (unlikely(port->fcoe->ctlr.flogi_oxid != FC_XID_UNKNOWN) &&
		    fcoe_ctlr_recv_flogi(&port->fcoe->ctlr, fp, mac)) {
			fc_frame_free(fp);
			continue;
		}
		fc_exch_recv(lp, fp);
	}
	return 0;
}


static void fcoe_check_wait_queue(struct fc_lport *lp, struct sk_buff *skb)
{
	struct fcoe_port *port = lport_priv(lp);
	int rc;

	spin_lock_bh(&port->fcoe_pending_queue.lock);

	if (skb)
		__skb_queue_tail(&port->fcoe_pending_queue, skb);

	if (port->fcoe_pending_queue_active)
		goto out;
	port->fcoe_pending_queue_active = 1;

	while (port->fcoe_pending_queue.qlen) {
		
		port->fcoe_pending_queue.qlen++;
		skb = __skb_dequeue(&port->fcoe_pending_queue);

		spin_unlock_bh(&port->fcoe_pending_queue.lock);
		rc = fcoe_start_io(skb);
		spin_lock_bh(&port->fcoe_pending_queue.lock);

		if (rc) {
			__skb_queue_head(&port->fcoe_pending_queue, skb);
			
			port->fcoe_pending_queue.qlen--;
			break;
		}
		
		port->fcoe_pending_queue.qlen--;
	}

	if (port->fcoe_pending_queue.qlen < FCOE_LOW_QUEUE_DEPTH)
		lp->qfull = 0;
	if (port->fcoe_pending_queue.qlen && !timer_pending(&port->timer))
		mod_timer(&port->timer, jiffies + 2);
	port->fcoe_pending_queue_active = 0;
out:
	if (port->fcoe_pending_queue.qlen > FCOE_MAX_QUEUE_DEPTH)
		lp->qfull = 1;
	spin_unlock_bh(&port->fcoe_pending_queue.lock);
	return;
}


static void fcoe_dev_setup(void)
{
	register_netdevice_notifier(&fcoe_notifier);
}


static void fcoe_dev_cleanup(void)
{
	unregister_netdevice_notifier(&fcoe_notifier);
}


static int fcoe_device_notification(struct notifier_block *notifier,
				    ulong event, void *ptr)
{
	struct fc_lport *lp = NULL;
	struct net_device *netdev = ptr;
	struct fcoe_interface *fcoe;
	struct fcoe_port *port;
	struct fcoe_dev_stats *stats;
	u32 link_possible = 1;
	u32 mfs;
	int rc = NOTIFY_OK;

	list_for_each_entry(fcoe, &fcoe_hostlist, list) {
		if (fcoe->netdev == netdev) {
			lp = fcoe->ctlr.lp;
			break;
		}
	}
	if (lp == NULL) {
		rc = NOTIFY_DONE;
		goto out;
	}

	switch (event) {
	case NETDEV_DOWN:
	case NETDEV_GOING_DOWN:
		link_possible = 0;
		break;
	case NETDEV_UP:
	case NETDEV_CHANGE:
		break;
	case NETDEV_CHANGEMTU:
		mfs = netdev->mtu - (sizeof(struct fcoe_hdr) +
				     sizeof(struct fcoe_crc_eof));
		if (mfs >= FC_MIN_MAX_FRAME)
			fc_set_mfs(lp, mfs);
		break;
	case NETDEV_REGISTER:
		break;
	case NETDEV_UNREGISTER:
		list_del(&fcoe->list);
		port = lport_priv(fcoe->ctlr.lp);
		fcoe_interface_cleanup(fcoe);
		schedule_work(&port->destroy_work);
		goto out;
		break;
	default:
		FCOE_NETDEV_DBG(netdev, "Unknown event %ld "
				"from netdev netlink\n", event);
	}
	if (link_possible && !fcoe_link_ok(lp))
		fcoe_ctlr_link_up(&fcoe->ctlr);
	else if (fcoe_ctlr_link_down(&fcoe->ctlr)) {
		stats = fc_lport_get_stats(lp);
		stats->LinkFailureCount++;
		fcoe_clean_pending_queue(lp);
	}
out:
	return rc;
}


static struct net_device *fcoe_if_to_netdev(const char *buffer)
{
	char *cp;
	char ifname[IFNAMSIZ + 2];

	if (buffer) {
		strlcpy(ifname, buffer, IFNAMSIZ);
		cp = ifname + strlen(ifname);
		while (--cp >= ifname && *cp == '\n')
			*cp = '\0';
		return dev_get_by_name(&init_net, ifname);
	}
	return NULL;
}


static int fcoe_destroy(const char *buffer, struct kernel_param *kp)
{
	struct fcoe_interface *fcoe;
	struct net_device *netdev;
	int rc = 0;

	mutex_lock(&fcoe_config_mutex);
#ifdef CONFIG_FCOE_MODULE
	
	if (THIS_MODULE->state != MODULE_STATE_LIVE) {
		rc = -ENODEV;
		goto out_nodev;
	}
#endif

	netdev = fcoe_if_to_netdev(buffer);
	if (!netdev) {
		rc = -ENODEV;
		goto out_nodev;
	}

	rtnl_lock();
	fcoe = fcoe_hostlist_lookup_port(netdev);
	if (!fcoe) {
		rtnl_unlock();
		rc = -ENODEV;
		goto out_putdev;
	}
	list_del(&fcoe->list);
	fcoe_interface_cleanup(fcoe);
	rtnl_unlock();
	fcoe_if_destroy(fcoe->ctlr.lp);
out_putdev:
	dev_put(netdev);
out_nodev:
	mutex_unlock(&fcoe_config_mutex);
	return rc;
}

static void fcoe_destroy_work(struct work_struct *work)
{
	struct fcoe_port *port;

	port = container_of(work, struct fcoe_port, destroy_work);
	mutex_lock(&fcoe_config_mutex);
	fcoe_if_destroy(port->lport);
	mutex_unlock(&fcoe_config_mutex);
}


static int fcoe_create(const char *buffer, struct kernel_param *kp)
{
	int rc;
	struct fcoe_interface *fcoe;
	struct fc_lport *lport;
	struct net_device *netdev;

	mutex_lock(&fcoe_config_mutex);
#ifdef CONFIG_FCOE_MODULE
	
	if (THIS_MODULE->state != MODULE_STATE_LIVE) {
		rc = -ENODEV;
		goto out_nodev;
	}
#endif

	rtnl_lock();
	netdev = fcoe_if_to_netdev(buffer);
	if (!netdev) {
		rc = -ENODEV;
		goto out_nodev;
	}

	
	if (fcoe_hostlist_lookup(netdev)) {
		rc = -EEXIST;
		goto out_putdev;
	}

	fcoe = fcoe_interface_create(netdev);
	if (!fcoe) {
		rc = -ENOMEM;
		goto out_putdev;
	}

	lport = fcoe_if_create(fcoe, &netdev->dev);
	if (IS_ERR(lport)) {
		printk(KERN_ERR "fcoe: Failed to create interface (%s)\n",
		       netdev->name);
		rc = -EIO;
		fcoe_interface_cleanup(fcoe);
		goto out_free;
	}

	
	fcoe->ctlr.lp = lport;

	
	fcoe_hostlist_add(lport);

	
	lport->boot_time = jiffies;
	fc_fabric_login(lport);
	if (!fcoe_link_ok(lport))
		fcoe_ctlr_link_up(&fcoe->ctlr);

	rc = 0;
out_free:
	
	fcoe_interface_put(fcoe);
out_putdev:
	dev_put(netdev);
out_nodev:
	rtnl_unlock();
	mutex_unlock(&fcoe_config_mutex);
	return rc;
}

module_param_call(create, fcoe_create, NULL, NULL, S_IWUSR);
__MODULE_PARM_TYPE(create, "string");
MODULE_PARM_DESC(create, "Create fcoe fcoe using net device passed in.");
module_param_call(destroy, fcoe_destroy, NULL, NULL, S_IWUSR);
__MODULE_PARM_TYPE(destroy, "string");
MODULE_PARM_DESC(destroy, "Destroy fcoe fcoe");


int fcoe_link_ok(struct fc_lport *lp)
{
	struct fcoe_port *port = lport_priv(lp);
	struct net_device *dev = port->fcoe->netdev;
	struct ethtool_cmd ecmd = { ETHTOOL_GSET };

	if ((dev->flags & IFF_UP) && netif_carrier_ok(dev) &&
	    (!dev_ethtool_get_settings(dev, &ecmd))) {
		lp->link_supported_speeds &=
			~(FC_PORTSPEED_1GBIT | FC_PORTSPEED_10GBIT);
		if (ecmd.supported & (SUPPORTED_1000baseT_Half |
				      SUPPORTED_1000baseT_Full))
			lp->link_supported_speeds |= FC_PORTSPEED_1GBIT;
		if (ecmd.supported & SUPPORTED_10000baseT_Full)
			lp->link_supported_speeds |=
				FC_PORTSPEED_10GBIT;
		if (ecmd.speed == SPEED_1000)
			lp->link_speed = FC_PORTSPEED_1GBIT;
		if (ecmd.speed == SPEED_10000)
			lp->link_speed = FC_PORTSPEED_10GBIT;

		return 0;
	}
	return -1;
}


void fcoe_percpu_clean(struct fc_lport *lp)
{
	struct fcoe_percpu_s *pp;
	struct fcoe_rcv_info *fr;
	struct sk_buff_head *list;
	struct sk_buff *skb, *next;
	struct sk_buff *head;
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		pp = &per_cpu(fcoe_percpu, cpu);
		spin_lock_bh(&pp->fcoe_rx_list.lock);
		list = &pp->fcoe_rx_list;
		head = list->next;
		for (skb = head; skb != (struct sk_buff *)list;
		     skb = next) {
			next = skb->next;
			fr = fcoe_dev_from_skb(skb);
			if (fr->fr_dev == lp) {
				__skb_unlink(skb, list);
				kfree_skb(skb);
			}
		}

		if (!pp->thread || !cpu_online(cpu)) {
			spin_unlock_bh(&pp->fcoe_rx_list.lock);
			continue;
		}

		skb = dev_alloc_skb(0);
		if (!skb) {
			spin_unlock_bh(&pp->fcoe_rx_list.lock);
			continue;
		}
		skb->destructor = fcoe_percpu_flush_done;

		__skb_queue_tail(&pp->fcoe_rx_list, skb);
		if (pp->fcoe_rx_list.qlen == 1)
			wake_up_process(pp->thread);
		spin_unlock_bh(&pp->fcoe_rx_list.lock);

		wait_for_completion(&fcoe_flush_completion);
	}
}


void fcoe_clean_pending_queue(struct fc_lport *lp)
{
	struct fcoe_port  *port = lport_priv(lp);
	struct sk_buff *skb;

	spin_lock_bh(&port->fcoe_pending_queue.lock);
	while ((skb = __skb_dequeue(&port->fcoe_pending_queue)) != NULL) {
		spin_unlock_bh(&port->fcoe_pending_queue.lock);
		kfree_skb(skb);
		spin_lock_bh(&port->fcoe_pending_queue.lock);
	}
	spin_unlock_bh(&port->fcoe_pending_queue.lock);
}


int fcoe_reset(struct Scsi_Host *shost)
{
	struct fc_lport *lport = shost_priv(shost);
	fc_lport_reset(lport);
	return 0;
}


static struct fcoe_interface *
fcoe_hostlist_lookup_port(const struct net_device *dev)
{
	struct fcoe_interface *fcoe;

	list_for_each_entry(fcoe, &fcoe_hostlist, list) {
		if (fcoe->netdev == dev)
			return fcoe;
	}
	return NULL;
}


static struct fc_lport *fcoe_hostlist_lookup(const struct net_device *netdev)
{
	struct fcoe_interface *fcoe;

	fcoe = fcoe_hostlist_lookup_port(netdev);
	return (fcoe) ? fcoe->ctlr.lp : NULL;
}


static int fcoe_hostlist_add(const struct fc_lport *lport)
{
	struct fcoe_interface *fcoe;
	struct fcoe_port *port;

	fcoe = fcoe_hostlist_lookup_port(fcoe_netdev(lport));
	if (!fcoe) {
		port = lport_priv(lport);
		fcoe = port->fcoe;
		list_add_tail(&fcoe->list, &fcoe_hostlist);
	}
	return 0;
}


static int __init fcoe_init(void)
{
	unsigned int cpu;
	int rc = 0;
	struct fcoe_percpu_s *p;

	mutex_lock(&fcoe_config_mutex);

	for_each_possible_cpu(cpu) {
		p = &per_cpu(fcoe_percpu, cpu);
		skb_queue_head_init(&p->fcoe_rx_list);
	}

	for_each_online_cpu(cpu)
		fcoe_percpu_thread_create(cpu);

	
	rc = register_hotcpu_notifier(&fcoe_cpu_notifier);
	if (rc)
		goto out_free;

	
	fcoe_dev_setup();

	rc = fcoe_if_init();
	if (rc)
		goto out_free;

	mutex_unlock(&fcoe_config_mutex);
	return 0;

out_free:
	for_each_online_cpu(cpu) {
		fcoe_percpu_thread_destroy(cpu);
	}
	mutex_unlock(&fcoe_config_mutex);
	return rc;
}
module_init(fcoe_init);


static void __exit fcoe_exit(void)
{
	unsigned int cpu;
	struct fcoe_interface *fcoe, *tmp;
	struct fcoe_port *port;

	mutex_lock(&fcoe_config_mutex);

	fcoe_dev_cleanup();

	
	rtnl_lock();
	list_for_each_entry_safe(fcoe, tmp, &fcoe_hostlist, list) {
		list_del(&fcoe->list);
		port = lport_priv(fcoe->ctlr.lp);
		fcoe_interface_cleanup(fcoe);
		schedule_work(&port->destroy_work);
	}
	rtnl_unlock();

	unregister_hotcpu_notifier(&fcoe_cpu_notifier);

	for_each_online_cpu(cpu)
		fcoe_percpu_thread_destroy(cpu);

	mutex_unlock(&fcoe_config_mutex);

	
	flush_scheduled_work();

	
	fcoe_if_exit();
}
module_exit(fcoe_exit);
