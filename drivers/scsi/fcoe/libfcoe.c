

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/errno.h>
#include <linux/bitops.h>
#include <net/rtnetlink.h>

#include <scsi/fc/fc_els.h>
#include <scsi/fc/fc_fs.h>
#include <scsi/fc/fc_fip.h>
#include <scsi/fc/fc_encaps.h>
#include <scsi/fc/fc_fcoe.h>

#include <scsi/libfc.h>
#include <scsi/libfcoe.h>

MODULE_AUTHOR("Open-FCoE.org");
MODULE_DESCRIPTION("FIP discovery protocol support for FCoE HBAs");
MODULE_LICENSE("GPL v2");

#define	FCOE_CTLR_MIN_FKA	500		
#define	FCOE_CTLR_DEF_FKA	FIP_DEF_FKA	

static void fcoe_ctlr_timeout(unsigned long);
static void fcoe_ctlr_link_work(struct work_struct *);
static void fcoe_ctlr_recv_work(struct work_struct *);

static u8 fcoe_all_fcfs[ETH_ALEN] = FIP_ALL_FCF_MACS;

unsigned int libfcoe_debug_logging;
module_param_named(debug_logging, libfcoe_debug_logging, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(debug_logging, "a bit mask of logging levels");

#define LIBFCOE_LOGGING     0x01 
#define LIBFCOE_FIP_LOGGING 0x02 

#define LIBFCOE_CHECK_LOGGING(LEVEL, CMD)				\
do {                                                            	\
	if (unlikely(libfcoe_debug_logging & LEVEL))			\
		do {							\
			CMD;						\
		} while (0);						\
} while (0)

#define LIBFCOE_DBG(fmt, args...)					\
	LIBFCOE_CHECK_LOGGING(LIBFCOE_LOGGING,				\
			      printk(KERN_INFO "libfcoe: " fmt, ##args);)

#define LIBFCOE_FIP_DBG(fmt, args...)					\
	LIBFCOE_CHECK_LOGGING(LIBFCOE_FIP_LOGGING,			\
			      printk(KERN_INFO "fip: " fmt, ##args);)


static inline int fcoe_ctlr_mtu_valid(const struct fcoe_fcf *fcf)
{
	return (fcf->flags & FIP_FL_SOL) != 0;
}


static inline int fcoe_ctlr_fcf_usable(struct fcoe_fcf *fcf)
{
	u16 flags = FIP_FL_SOL | FIP_FL_AVAIL;

	return (fcf->flags & flags) == flags;
}


void fcoe_ctlr_init(struct fcoe_ctlr *fip)
{
	fip->state = FIP_ST_LINK_WAIT;
	INIT_LIST_HEAD(&fip->fcfs);
	spin_lock_init(&fip->lock);
	fip->flogi_oxid = FC_XID_UNKNOWN;
	setup_timer(&fip->timer, fcoe_ctlr_timeout, (unsigned long)fip);
	INIT_WORK(&fip->link_work, fcoe_ctlr_link_work);
	INIT_WORK(&fip->recv_work, fcoe_ctlr_recv_work);
	skb_queue_head_init(&fip->fip_recv_list);
}
EXPORT_SYMBOL(fcoe_ctlr_init);


static void fcoe_ctlr_reset_fcfs(struct fcoe_ctlr *fip)
{
	struct fcoe_fcf *fcf;
	struct fcoe_fcf *next;

	fip->sel_fcf = NULL;
	list_for_each_entry_safe(fcf, next, &fip->fcfs, list) {
		list_del(&fcf->list);
		kfree(fcf);
	}
	fip->fcf_count = 0;
	fip->sel_time = 0;
}


void fcoe_ctlr_destroy(struct fcoe_ctlr *fip)
{
	cancel_work_sync(&fip->recv_work);
	spin_lock_bh(&fip->fip_recv_list.lock);
	__skb_queue_purge(&fip->fip_recv_list);
	spin_unlock_bh(&fip->fip_recv_list.lock);

	spin_lock_bh(&fip->lock);
	fip->state = FIP_ST_DISABLED;
	fcoe_ctlr_reset_fcfs(fip);
	spin_unlock_bh(&fip->lock);
	del_timer_sync(&fip->timer);
	cancel_work_sync(&fip->link_work);
}
EXPORT_SYMBOL(fcoe_ctlr_destroy);


static inline u32 fcoe_ctlr_fcoe_size(struct fcoe_ctlr *fip)
{
	
	return fip->lp->mfs + sizeof(struct fc_frame_header) +
		sizeof(struct fcoe_hdr) + sizeof(struct fcoe_crc_eof);
}


static void fcoe_ctlr_solicit(struct fcoe_ctlr *fip, struct fcoe_fcf *fcf)
{
	struct sk_buff *skb;
	struct fip_sol {
		struct ethhdr eth;
		struct fip_header fip;
		struct {
			struct fip_mac_desc mac;
			struct fip_wwn_desc wwnn;
			struct fip_size_desc size;
		} __attribute__((packed)) desc;
	}  __attribute__((packed)) *sol;
	u32 fcoe_size;

	skb = dev_alloc_skb(sizeof(*sol));
	if (!skb)
		return;

	sol = (struct fip_sol *)skb->data;

	memset(sol, 0, sizeof(*sol));
	memcpy(sol->eth.h_dest, fcf ? fcf->fcf_mac : fcoe_all_fcfs, ETH_ALEN);
	memcpy(sol->eth.h_source, fip->ctl_src_addr, ETH_ALEN);
	sol->eth.h_proto = htons(ETH_P_FIP);

	sol->fip.fip_ver = FIP_VER_ENCAPS(FIP_VER);
	sol->fip.fip_op = htons(FIP_OP_DISC);
	sol->fip.fip_subcode = FIP_SC_SOL;
	sol->fip.fip_dl_len = htons(sizeof(sol->desc) / FIP_BPW);
	sol->fip.fip_flags = htons(FIP_FL_FPMA);
	if (fip->spma)
		sol->fip.fip_flags |= htons(FIP_FL_SPMA);

	sol->desc.mac.fd_desc.fip_dtype = FIP_DT_MAC;
	sol->desc.mac.fd_desc.fip_dlen = sizeof(sol->desc.mac) / FIP_BPW;
	memcpy(sol->desc.mac.fd_mac, fip->ctl_src_addr, ETH_ALEN);

	sol->desc.wwnn.fd_desc.fip_dtype = FIP_DT_NAME;
	sol->desc.wwnn.fd_desc.fip_dlen = sizeof(sol->desc.wwnn) / FIP_BPW;
	put_unaligned_be64(fip->lp->wwnn, &sol->desc.wwnn.fd_wwn);

	fcoe_size = fcoe_ctlr_fcoe_size(fip);
	sol->desc.size.fd_desc.fip_dtype = FIP_DT_FCOE_SIZE;
	sol->desc.size.fd_desc.fip_dlen = sizeof(sol->desc.size) / FIP_BPW;
	sol->desc.size.fd_size = htons(fcoe_size);

	skb_put(skb, sizeof(*sol));
	skb->protocol = htons(ETH_P_FIP);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	fip->send(fip, skb);

	if (!fcf)
		fip->sol_time = jiffies;
}


void fcoe_ctlr_link_up(struct fcoe_ctlr *fip)
{
	spin_lock_bh(&fip->lock);
	if (fip->state == FIP_ST_NON_FIP || fip->state == FIP_ST_AUTO) {
		fip->last_link = 1;
		fip->link = 1;
		spin_unlock_bh(&fip->lock);
		fc_linkup(fip->lp);
	} else if (fip->state == FIP_ST_LINK_WAIT) {
		fip->state = FIP_ST_AUTO;
		fip->last_link = 1;
		fip->link = 1;
		spin_unlock_bh(&fip->lock);
		LIBFCOE_FIP_DBG("%s", "setting AUTO mode.\n");
		fc_linkup(fip->lp);
		fcoe_ctlr_solicit(fip, NULL);
	} else
		spin_unlock_bh(&fip->lock);
}
EXPORT_SYMBOL(fcoe_ctlr_link_up);


static int fcoe_ctlr_reset(struct fcoe_ctlr *fip, enum fip_state new_state)
{
	struct fc_lport *lp = fip->lp;
	int link_dropped;

	spin_lock_bh(&fip->lock);
	fcoe_ctlr_reset_fcfs(fip);
	del_timer(&fip->timer);
	fip->state = new_state;
	fip->ctlr_ka_time = 0;
	fip->port_ka_time = 0;
	fip->sol_time = 0;
	fip->flogi_oxid = FC_XID_UNKNOWN;
	fip->map_dest = 0;
	fip->last_link = 0;
	link_dropped = fip->link;
	fip->link = 0;
	spin_unlock_bh(&fip->lock);

	if (link_dropped)
		fc_linkdown(lp);

	if (new_state == FIP_ST_ENABLED) {
		fcoe_ctlr_solicit(fip, NULL);
		fc_linkup(lp);
		link_dropped = 0;
	}
	return link_dropped;
}


int fcoe_ctlr_link_down(struct fcoe_ctlr *fip)
{
	return fcoe_ctlr_reset(fip, FIP_ST_LINK_WAIT);
}
EXPORT_SYMBOL(fcoe_ctlr_link_down);


static void fcoe_ctlr_send_keep_alive(struct fcoe_ctlr *fip, int ports, u8 *sa)
{
	struct sk_buff *skb;
	struct fip_kal {
		struct ethhdr eth;
		struct fip_header fip;
		struct fip_mac_desc mac;
	} __attribute__((packed)) *kal;
	struct fip_vn_desc *vn;
	u32 len;
	struct fc_lport *lp;
	struct fcoe_fcf *fcf;

	fcf = fip->sel_fcf;
	lp = fip->lp;
	if (!fcf || !fc_host_port_id(lp->host))
		return;

	len = fcoe_ctlr_fcoe_size(fip) + sizeof(struct ethhdr);
	BUG_ON(len < sizeof(*kal) + sizeof(*vn));
	skb = dev_alloc_skb(len);
	if (!skb)
		return;

	kal = (struct fip_kal *)skb->data;
	memset(kal, 0, len);
	memcpy(kal->eth.h_dest, fcf->fcf_mac, ETH_ALEN);
	memcpy(kal->eth.h_source, sa, ETH_ALEN);
	kal->eth.h_proto = htons(ETH_P_FIP);

	kal->fip.fip_ver = FIP_VER_ENCAPS(FIP_VER);
	kal->fip.fip_op = htons(FIP_OP_CTRL);
	kal->fip.fip_subcode = FIP_SC_KEEP_ALIVE;
	kal->fip.fip_dl_len = htons((sizeof(kal->mac) +
				    ports * sizeof(*vn)) / FIP_BPW);
	kal->fip.fip_flags = htons(FIP_FL_FPMA);
	if (fip->spma)
		kal->fip.fip_flags |= htons(FIP_FL_SPMA);

	kal->mac.fd_desc.fip_dtype = FIP_DT_MAC;
	kal->mac.fd_desc.fip_dlen = sizeof(kal->mac) / FIP_BPW;
	memcpy(kal->mac.fd_mac, fip->ctl_src_addr, ETH_ALEN);

	if (ports) {
		vn = (struct fip_vn_desc *)(kal + 1);
		vn->fd_desc.fip_dtype = FIP_DT_VN_ID;
		vn->fd_desc.fip_dlen = sizeof(*vn) / FIP_BPW;
		memcpy(vn->fd_mac, fip->data_src_addr, ETH_ALEN);
		hton24(vn->fd_fc_id, fc_host_port_id(lp->host));
		put_unaligned_be64(lp->wwpn, &vn->fd_wwpn);
	}

	skb_put(skb, len);
	skb->protocol = htons(ETH_P_FIP);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	fip->send(fip, skb);
}


static int fcoe_ctlr_encaps(struct fcoe_ctlr *fip,
			    u8 dtype, struct sk_buff *skb)
{
	struct fip_encaps_head {
		struct ethhdr eth;
		struct fip_header fip;
		struct fip_encaps encaps;
	} __attribute__((packed)) *cap;
	struct fip_mac_desc *mac;
	struct fcoe_fcf *fcf;
	size_t dlen;
	u16 fip_flags;

	fcf = fip->sel_fcf;
	if (!fcf)
		return -ENODEV;

	
	fip_flags = fcf->flags;
	fip_flags &= fip->spma ? FIP_FL_SPMA | FIP_FL_FPMA : FIP_FL_FPMA;
	if (!fip_flags)
		return -ENODEV;

	dlen = sizeof(struct fip_encaps) + skb->len;	
	cap = (struct fip_encaps_head *)skb_push(skb, sizeof(*cap));

	memset(cap, 0, sizeof(*cap));
	memcpy(cap->eth.h_dest, fcf->fcf_mac, ETH_ALEN);
	memcpy(cap->eth.h_source, fip->ctl_src_addr, ETH_ALEN);
	cap->eth.h_proto = htons(ETH_P_FIP);

	cap->fip.fip_ver = FIP_VER_ENCAPS(FIP_VER);
	cap->fip.fip_op = htons(FIP_OP_LS);
	cap->fip.fip_subcode = FIP_SC_REQ;
	cap->fip.fip_dl_len = htons((dlen + sizeof(*mac)) / FIP_BPW);
	cap->fip.fip_flags = htons(fip_flags);

	cap->encaps.fd_desc.fip_dtype = dtype;
	cap->encaps.fd_desc.fip_dlen = dlen / FIP_BPW;

	mac = (struct fip_mac_desc *)skb_put(skb, sizeof(*mac));
	memset(mac, 0, sizeof(mac));
	mac->fd_desc.fip_dtype = FIP_DT_MAC;
	mac->fd_desc.fip_dlen = sizeof(*mac) / FIP_BPW;
	if (dtype != FIP_DT_FLOGI)
		memcpy(mac->fd_mac, fip->data_src_addr, ETH_ALEN);
	else if (fip->spma)
		memcpy(mac->fd_mac, fip->ctl_src_addr, ETH_ALEN);

	skb->protocol = htons(ETH_P_FIP);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	return 0;
}


int fcoe_ctlr_els_send(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	struct fc_frame_header *fh;
	u16 old_xid;
	u8 op;

	fh = (struct fc_frame_header *)skb->data;
	op = *(u8 *)(fh + 1);

	if (op == ELS_FLOGI) {
		old_xid = fip->flogi_oxid;
		fip->flogi_oxid = ntohs(fh->fh_ox_id);
		if (fip->state == FIP_ST_AUTO) {
			if (old_xid == FC_XID_UNKNOWN)
				fip->flogi_count = 0;
			fip->flogi_count++;
			if (fip->flogi_count < 3)
				goto drop;
			fip->map_dest = 1;
			return 0;
		}
		if (fip->state == FIP_ST_NON_FIP)
			fip->map_dest = 1;
	}

	if (fip->state == FIP_ST_NON_FIP)
		return 0;

	switch (op) {
	case ELS_FLOGI:
		op = FIP_DT_FLOGI;
		break;
	case ELS_FDISC:
		if (ntoh24(fh->fh_s_id))
			return 0;
		op = FIP_DT_FDISC;
		break;
	case ELS_LOGO:
		if (fip->state != FIP_ST_ENABLED)
			return 0;
		if (ntoh24(fh->fh_d_id) != FC_FID_FLOGI)
			return 0;
		op = FIP_DT_LOGO;
		break;
	case ELS_LS_ACC:
		if (fip->flogi_oxid == FC_XID_UNKNOWN)
			return 0;
		if (!ntoh24(fh->fh_s_id))
			return 0;
		if (fip->state == FIP_ST_AUTO)
			return 0;
		
		fip->flogi_oxid = FC_XID_UNKNOWN;
		fc_fcoe_set_mac(fip->data_src_addr, fh->fh_s_id);
		return 0;
	default:
		if (fip->state != FIP_ST_ENABLED)
			goto drop;
		return 0;
	}
	if (fcoe_ctlr_encaps(fip, op, skb))
		goto drop;
	fip->send(fip, skb);
	return -EINPROGRESS;
drop:
	kfree_skb(skb);
	return -EINVAL;
}
EXPORT_SYMBOL(fcoe_ctlr_els_send);


static void fcoe_ctlr_age_fcfs(struct fcoe_ctlr *fip)
{
	struct fcoe_fcf *fcf;
	struct fcoe_fcf *next;
	unsigned long sel_time = 0;

	list_for_each_entry_safe(fcf, next, &fip->fcfs, list) {
		if (time_after(jiffies, fcf->time + fcf->fka_period * 3 +
			       msecs_to_jiffies(FIP_FCF_FUZZ * 3))) {
			if (fip->sel_fcf == fcf)
				fip->sel_fcf = NULL;
			list_del(&fcf->list);
			WARN_ON(!fip->fcf_count);
			fip->fcf_count--;
			kfree(fcf);
		} else if (fcoe_ctlr_mtu_valid(fcf) &&
			   (!sel_time || time_before(sel_time, fcf->time))) {
			sel_time = fcf->time;
		}
	}
	if (sel_time) {
		sel_time += msecs_to_jiffies(FCOE_CTLR_START_DELAY);
		fip->sel_time = sel_time;
		if (time_before(sel_time, fip->timer.expires))
			mod_timer(&fip->timer, sel_time);
	} else {
		fip->sel_time = 0;
	}
}


static int fcoe_ctlr_parse_adv(struct sk_buff *skb, struct fcoe_fcf *fcf)
{
	struct fip_header *fiph;
	struct fip_desc *desc = NULL;
	struct fip_wwn_desc *wwn;
	struct fip_fab_desc *fab;
	struct fip_fka_desc *fka;
	unsigned long t;
	size_t rlen;
	size_t dlen;

	memset(fcf, 0, sizeof(*fcf));
	fcf->fka_period = msecs_to_jiffies(FCOE_CTLR_DEF_FKA);

	fiph = (struct fip_header *)skb->data;
	fcf->flags = ntohs(fiph->fip_flags);

	rlen = ntohs(fiph->fip_dl_len) * 4;
	if (rlen + sizeof(*fiph) > skb->len)
		return -EINVAL;

	desc = (struct fip_desc *)(fiph + 1);
	while (rlen > 0) {
		dlen = desc->fip_dlen * FIP_BPW;
		if (dlen < sizeof(*desc) || dlen > rlen)
			return -EINVAL;
		switch (desc->fip_dtype) {
		case FIP_DT_PRI:
			if (dlen != sizeof(struct fip_pri_desc))
				goto len_err;
			fcf->pri = ((struct fip_pri_desc *)desc)->fd_pri;
			break;
		case FIP_DT_MAC:
			if (dlen != sizeof(struct fip_mac_desc))
				goto len_err;
			memcpy(fcf->fcf_mac,
			       ((struct fip_mac_desc *)desc)->fd_mac,
			       ETH_ALEN);
			if (!is_valid_ether_addr(fcf->fcf_mac)) {
				LIBFCOE_FIP_DBG("Invalid MAC address "
						"in FIP adv\n");
				return -EINVAL;
			}
			break;
		case FIP_DT_NAME:
			if (dlen != sizeof(struct fip_wwn_desc))
				goto len_err;
			wwn = (struct fip_wwn_desc *)desc;
			fcf->switch_name = get_unaligned_be64(&wwn->fd_wwn);
			break;
		case FIP_DT_FAB:
			if (dlen != sizeof(struct fip_fab_desc))
				goto len_err;
			fab = (struct fip_fab_desc *)desc;
			fcf->fabric_name = get_unaligned_be64(&fab->fd_wwn);
			fcf->vfid = ntohs(fab->fd_vfid);
			fcf->fc_map = ntoh24(fab->fd_map);
			break;
		case FIP_DT_FKA:
			if (dlen != sizeof(struct fip_fka_desc))
				goto len_err;
			fka = (struct fip_fka_desc *)desc;
			t = ntohl(fka->fd_fka_period);
			if (t >= FCOE_CTLR_MIN_FKA)
				fcf->fka_period = msecs_to_jiffies(t);
			break;
		case FIP_DT_MAP_OUI:
		case FIP_DT_FCOE_SIZE:
		case FIP_DT_FLOGI:
		case FIP_DT_FDISC:
		case FIP_DT_LOGO:
		case FIP_DT_ELP:
		default:
			LIBFCOE_FIP_DBG("unexpected descriptor type %x "
					"in FIP adv\n", desc->fip_dtype);
			
			if (desc->fip_dtype < FIP_DT_VENDOR_BASE)
				return -EINVAL;
			continue;
		}
		desc = (struct fip_desc *)((char *)desc + dlen);
		rlen -= dlen;
	}
	if (!fcf->fc_map || (fcf->fc_map & 0x10000))
		return -EINVAL;
	if (!fcf->switch_name || !fcf->fabric_name)
		return -EINVAL;
	return 0;

len_err:
	LIBFCOE_FIP_DBG("FIP length error in descriptor type %x len %zu\n",
			desc->fip_dtype, dlen);
	return -EINVAL;
}


static void fcoe_ctlr_recv_adv(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	struct fcoe_fcf *fcf;
	struct fcoe_fcf new;
	struct fcoe_fcf *found;
	unsigned long sol_tov = msecs_to_jiffies(FCOE_CTRL_SOL_TOV);
	int first = 0;
	int mtu_valid;

	if (fcoe_ctlr_parse_adv(skb, &new))
		return;

	spin_lock_bh(&fip->lock);
	first = list_empty(&fip->fcfs);
	found = NULL;
	list_for_each_entry(fcf, &fip->fcfs, list) {
		if (fcf->switch_name == new.switch_name &&
		    fcf->fabric_name == new.fabric_name &&
		    fcf->fc_map == new.fc_map &&
		    compare_ether_addr(fcf->fcf_mac, new.fcf_mac) == 0) {
			found = fcf;
			break;
		}
	}
	if (!found) {
		if (fip->fcf_count >= FCOE_CTLR_FCF_LIMIT)
			goto out;

		fcf = kmalloc(sizeof(*fcf), GFP_ATOMIC);
		if (!fcf)
			goto out;

		fip->fcf_count++;
		memcpy(fcf, &new, sizeof(new));
		list_add(&fcf->list, &fip->fcfs);
	} else {
		
		if (fcf == fip->sel_fcf) {
			fip->ctlr_ka_time -= fcf->fka_period;
			fip->ctlr_ka_time += new.fka_period;
			if (time_before(fip->ctlr_ka_time, fip->timer.expires))
				mod_timer(&fip->timer, fip->ctlr_ka_time);
		} else if (!fcoe_ctlr_fcf_usable(fcf))
			fcf->flags = new.flags;
		fcf->fka_period = new.fka_period;
		memcpy(fcf->fcf_mac, new.fcf_mac, ETH_ALEN);
	}
	mtu_valid = fcoe_ctlr_mtu_valid(fcf);
	fcf->time = jiffies;
	if (!found) {
		LIBFCOE_FIP_DBG("New FCF for fab %llx map %x val %d\n",
				fcf->fabric_name, fcf->fc_map, mtu_valid);
	}

	
	if (!mtu_valid)
		fcoe_ctlr_solicit(fip, fcf);

	
	if (first && time_after(jiffies, fip->sol_time + sol_tov))
		fcoe_ctlr_solicit(fip, NULL);

	
	if (mtu_valid && !fip->sel_time && fcoe_ctlr_fcf_usable(fcf)) {
		fip->sel_time = jiffies +
				msecs_to_jiffies(FCOE_CTLR_START_DELAY);
		if (!timer_pending(&fip->timer) ||
		    time_before(fip->sel_time, fip->timer.expires))
			mod_timer(&fip->timer, fip->sel_time);
	}
out:
	spin_unlock_bh(&fip->lock);
}


static void fcoe_ctlr_recv_els(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	struct fc_lport *lp = fip->lp;
	struct fip_header *fiph;
	struct fc_frame *fp;
	struct fc_frame_header *fh = NULL;
	struct fip_desc *desc;
	struct fip_encaps *els;
	struct fcoe_dev_stats *stats;
	enum fip_desc_type els_dtype = 0;
	u8 els_op;
	u8 sub;
	u8 granted_mac[ETH_ALEN] = { 0 };
	size_t els_len = 0;
	size_t rlen;
	size_t dlen;

	fiph = (struct fip_header *)skb->data;
	sub = fiph->fip_subcode;
	if (sub != FIP_SC_REQ && sub != FIP_SC_REP)
		goto drop;

	rlen = ntohs(fiph->fip_dl_len) * 4;
	if (rlen + sizeof(*fiph) > skb->len)
		goto drop;

	desc = (struct fip_desc *)(fiph + 1);
	while (rlen > 0) {
		dlen = desc->fip_dlen * FIP_BPW;
		if (dlen < sizeof(*desc) || dlen > rlen)
			goto drop;
		switch (desc->fip_dtype) {
		case FIP_DT_MAC:
			if (dlen != sizeof(struct fip_mac_desc))
				goto len_err;
			memcpy(granted_mac,
			       ((struct fip_mac_desc *)desc)->fd_mac,
			       ETH_ALEN);
			if (!is_valid_ether_addr(granted_mac)) {
				LIBFCOE_FIP_DBG("Invalid MAC address "
						"in FIP ELS\n");
				goto drop;
			}
			break;
		case FIP_DT_FLOGI:
		case FIP_DT_FDISC:
		case FIP_DT_LOGO:
		case FIP_DT_ELP:
			if (fh)
				goto drop;
			if (dlen < sizeof(*els) + sizeof(*fh) + 1)
				goto len_err;
			els_len = dlen - sizeof(*els);
			els = (struct fip_encaps *)desc;
			fh = (struct fc_frame_header *)(els + 1);
			els_dtype = desc->fip_dtype;
			break;
		default:
			LIBFCOE_FIP_DBG("unexpected descriptor type %x "
					"in FIP adv\n", desc->fip_dtype);
			
			if (desc->fip_dtype < FIP_DT_VENDOR_BASE)
				goto drop;
			continue;
		}
		desc = (struct fip_desc *)((char *)desc + dlen);
		rlen -= dlen;
	}

	if (!fh)
		goto drop;
	els_op = *(u8 *)(fh + 1);

	if (els_dtype == FIP_DT_FLOGI && sub == FIP_SC_REP &&
	    fip->flogi_oxid == ntohs(fh->fh_ox_id) &&
	    els_op == ELS_LS_ACC && is_valid_ether_addr(granted_mac)) {
		fip->flogi_oxid = FC_XID_UNKNOWN;
		fip->update_mac(fip, fip->data_src_addr, granted_mac);
		memcpy(fip->data_src_addr, granted_mac, ETH_ALEN);
	}

	
	skb_pull(skb, (u8 *)fh - skb->data);
	skb_trim(skb, els_len);
	fp = (struct fc_frame *)skb;
	fc_frame_init(fp);
	fr_sof(fp) = FC_SOF_I3;
	fr_eof(fp) = FC_EOF_T;
	fr_dev(fp) = lp;

	stats = fc_lport_get_stats(lp);
	stats->RxFrames++;
	stats->RxWords += skb->len / FIP_BPW;

	fc_exch_recv(lp, fp);
	return;

len_err:
	LIBFCOE_FIP_DBG("FIP length error in descriptor type %x len %zu\n",
			desc->fip_dtype, dlen);
drop:
	kfree_skb(skb);
}


static void fcoe_ctlr_recv_clr_vlink(struct fcoe_ctlr *fip,
				      struct fip_header *fh)
{
	struct fip_desc *desc;
	struct fip_mac_desc *mp;
	struct fip_wwn_desc *wp;
	struct fip_vn_desc *vp;
	size_t rlen;
	size_t dlen;
	struct fcoe_fcf *fcf = fip->sel_fcf;
	struct fc_lport *lp = fip->lp;
	u32	desc_mask;

	LIBFCOE_FIP_DBG("Clear Virtual Link received\n");
	if (!fcf)
		return;
	if (!fcf || !fc_host_port_id(lp->host))
		return;

	
	desc_mask = BIT(FIP_DT_MAC) | BIT(FIP_DT_NAME) | BIT(FIP_DT_VN_ID);

	rlen = ntohs(fh->fip_dl_len) * FIP_BPW;
	desc = (struct fip_desc *)(fh + 1);
	while (rlen >= sizeof(*desc)) {
		dlen = desc->fip_dlen * FIP_BPW;
		if (dlen > rlen)
			return;
		switch (desc->fip_dtype) {
		case FIP_DT_MAC:
			mp = (struct fip_mac_desc *)desc;
			if (dlen < sizeof(*mp))
				return;
			if (compare_ether_addr(mp->fd_mac, fcf->fcf_mac))
				return;
			desc_mask &= ~BIT(FIP_DT_MAC);
			break;
		case FIP_DT_NAME:
			wp = (struct fip_wwn_desc *)desc;
			if (dlen < sizeof(*wp))
				return;
			if (get_unaligned_be64(&wp->fd_wwn) != fcf->switch_name)
				return;
			desc_mask &= ~BIT(FIP_DT_NAME);
			break;
		case FIP_DT_VN_ID:
			vp = (struct fip_vn_desc *)desc;
			if (dlen < sizeof(*vp))
				return;
			if (compare_ether_addr(vp->fd_mac,
			    fip->data_src_addr) == 0 &&
			    get_unaligned_be64(&vp->fd_wwpn) == lp->wwpn &&
			    ntoh24(vp->fd_fc_id) == fc_host_port_id(lp->host))
				desc_mask &= ~BIT(FIP_DT_VN_ID);
			break;
		default:
			
			if (desc->fip_dtype < FIP_DT_VENDOR_BASE)
				return;
			break;
		}
		desc = (struct fip_desc *)((char *)desc + dlen);
		rlen -= dlen;
	}

	
	if (desc_mask) {
		LIBFCOE_FIP_DBG("missing descriptors mask %x\n", desc_mask);
	} else {
		LIBFCOE_FIP_DBG("performing Clear Virtual Link\n");
		fcoe_ctlr_reset(fip, FIP_ST_ENABLED);
	}
}


void fcoe_ctlr_recv(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	spin_lock_bh(&fip->fip_recv_list.lock);
	__skb_queue_tail(&fip->fip_recv_list, skb);
	spin_unlock_bh(&fip->fip_recv_list.lock);
	schedule_work(&fip->recv_work);
}
EXPORT_SYMBOL(fcoe_ctlr_recv);


static int fcoe_ctlr_recv_handler(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	struct fip_header *fiph;
	struct ethhdr *eh;
	enum fip_state state;
	u16 op;
	u8 sub;

	if (skb_linearize(skb))
		goto drop;
	if (skb->len < sizeof(*fiph))
		goto drop;
	eh = eth_hdr(skb);
	if (compare_ether_addr(eh->h_dest, fip->ctl_src_addr) &&
	    compare_ether_addr(eh->h_dest, FIP_ALL_ENODE_MACS))
		goto drop;
	fiph = (struct fip_header *)skb->data;
	op = ntohs(fiph->fip_op);
	sub = fiph->fip_subcode;

	if (FIP_VER_DECAPS(fiph->fip_ver) != FIP_VER)
		goto drop;
	if (ntohs(fiph->fip_dl_len) * FIP_BPW + sizeof(*fiph) > skb->len)
		goto drop;

	spin_lock_bh(&fip->lock);
	state = fip->state;
	if (state == FIP_ST_AUTO) {
		fip->map_dest = 0;
		fip->state = FIP_ST_ENABLED;
		state = FIP_ST_ENABLED;
		LIBFCOE_FIP_DBG("Using FIP mode\n");
	}
	spin_unlock_bh(&fip->lock);
	if (state != FIP_ST_ENABLED)
		goto drop;

	if (op == FIP_OP_LS) {
		fcoe_ctlr_recv_els(fip, skb);	
		return 0;
	}
	if (op == FIP_OP_DISC && sub == FIP_SC_ADV)
		fcoe_ctlr_recv_adv(fip, skb);
	else if (op == FIP_OP_CTRL && sub == FIP_SC_CLR_VLINK)
		fcoe_ctlr_recv_clr_vlink(fip, fiph);
	kfree_skb(skb);
	return 0;
drop:
	kfree_skb(skb);
	return -1;
}


static void fcoe_ctlr_select(struct fcoe_ctlr *fip)
{
	struct fcoe_fcf *fcf;
	struct fcoe_fcf *best = NULL;

	list_for_each_entry(fcf, &fip->fcfs, list) {
		LIBFCOE_FIP_DBG("consider FCF for fab %llx VFID %d map %x "
				"val %d\n", fcf->fabric_name, fcf->vfid,
				fcf->fc_map, fcoe_ctlr_mtu_valid(fcf));
		if (!fcoe_ctlr_fcf_usable(fcf)) {
			LIBFCOE_FIP_DBG("FCF for fab %llx map %x %svalid "
					"%savailable\n", fcf->fabric_name,
					fcf->fc_map, (fcf->flags & FIP_FL_SOL)
					? "" : "in", (fcf->flags & FIP_FL_AVAIL)
					? "" : "un");
			continue;
		}
		if (!best) {
			best = fcf;
			continue;
		}
		if (fcf->fabric_name != best->fabric_name ||
		    fcf->vfid != best->vfid ||
		    fcf->fc_map != best->fc_map) {
			LIBFCOE_FIP_DBG("Conflicting fabric, VFID, "
					"or FC-MAP\n");
			return;
		}
		if (fcf->pri < best->pri)
			best = fcf;
	}
	fip->sel_fcf = best;
}


static void fcoe_ctlr_timeout(unsigned long arg)
{
	struct fcoe_ctlr *fip = (struct fcoe_ctlr *)arg;
	struct fcoe_fcf *sel;
	struct fcoe_fcf *fcf;
	unsigned long next_timer = jiffies + msecs_to_jiffies(FIP_VN_KA_PERIOD);
	u8 send_ctlr_ka;
	u8 send_port_ka;

	spin_lock_bh(&fip->lock);
	if (fip->state == FIP_ST_DISABLED) {
		spin_unlock_bh(&fip->lock);
		return;
	}

	fcf = fip->sel_fcf;
	fcoe_ctlr_age_fcfs(fip);

	sel = fip->sel_fcf;
	if (!sel && fip->sel_time && time_after_eq(jiffies, fip->sel_time)) {
		fcoe_ctlr_select(fip);
		sel = fip->sel_fcf;
		fip->sel_time = 0;
	}

	if (sel != fcf) {
		fcf = sel;		
		if (sel) {
			printk(KERN_INFO "libfcoe: host%d: FIP selected "
			       "Fibre-Channel Forwarder MAC %pM\n",
			       fip->lp->host->host_no, sel->fcf_mac);
			memcpy(fip->dest_addr, sel->fcf_mac, ETH_ALEN);
			fip->port_ka_time = jiffies +
					    msecs_to_jiffies(FIP_VN_KA_PERIOD);
			fip->ctlr_ka_time = jiffies + sel->fka_period;
			fip->link = 1;
		} else {
			printk(KERN_NOTICE "libfcoe: host%d: "
			       "FIP Fibre-Channel Forwarder timed out.  "
			       "Starting FCF discovery.\n",
			       fip->lp->host->host_no);
			fip->link = 0;
		}
		schedule_work(&fip->link_work);
	}

	send_ctlr_ka = 0;
	send_port_ka = 0;
	if (sel) {
		if (time_after_eq(jiffies, fip->ctlr_ka_time)) {
			fip->ctlr_ka_time = jiffies + sel->fka_period;
			send_ctlr_ka = 1;
		}
		if (time_after(next_timer, fip->ctlr_ka_time))
			next_timer = fip->ctlr_ka_time;

		if (time_after_eq(jiffies, fip->port_ka_time)) {
			fip->port_ka_time += jiffies +
					msecs_to_jiffies(FIP_VN_KA_PERIOD);
			send_port_ka = 1;
		}
		if (time_after(next_timer, fip->port_ka_time))
			next_timer = fip->port_ka_time;
		mod_timer(&fip->timer, next_timer);
	} else if (fip->sel_time) {
		next_timer = fip->sel_time +
				msecs_to_jiffies(FCOE_CTLR_START_DELAY);
		mod_timer(&fip->timer, next_timer);
	}
	spin_unlock_bh(&fip->lock);

	if (send_ctlr_ka)
		fcoe_ctlr_send_keep_alive(fip, 0, fip->ctl_src_addr);
	if (send_port_ka)
		fcoe_ctlr_send_keep_alive(fip, 1, fip->data_src_addr);
}


static void fcoe_ctlr_link_work(struct work_struct *work)
{
	struct fcoe_ctlr *fip;
	int link;
	int last_link;

	fip = container_of(work, struct fcoe_ctlr, link_work);
	spin_lock_bh(&fip->lock);
	last_link = fip->last_link;
	link = fip->link;
	fip->last_link = link;
	spin_unlock_bh(&fip->lock);

	if (last_link != link) {
		if (link)
			fc_linkup(fip->lp);
		else
			fcoe_ctlr_reset(fip, FIP_ST_LINK_WAIT);
	}
}


static void fcoe_ctlr_recv_work(struct work_struct *recv_work)
{
	struct fcoe_ctlr *fip;
	struct sk_buff *skb;

	fip = container_of(recv_work, struct fcoe_ctlr, recv_work);
	spin_lock_bh(&fip->fip_recv_list.lock);
	while ((skb = __skb_dequeue(&fip->fip_recv_list))) {
		spin_unlock_bh(&fip->fip_recv_list.lock);
		fcoe_ctlr_recv_handler(fip, skb);
		spin_lock_bh(&fip->fip_recv_list.lock);
	}
	spin_unlock_bh(&fip->fip_recv_list.lock);
}


int fcoe_ctlr_recv_flogi(struct fcoe_ctlr *fip, struct fc_frame *fp, u8 *sa)
{
	struct fc_frame_header *fh;
	u8 op;
	u8 mac[ETH_ALEN];

	fh = fc_frame_header_get(fp);
	if (fh->fh_type != FC_TYPE_ELS)
		return 0;

	op = fc_frame_payload_op(fp);
	if (op == ELS_LS_ACC && fh->fh_r_ctl == FC_RCTL_ELS_REP &&
	    fip->flogi_oxid == ntohs(fh->fh_ox_id)) {

		spin_lock_bh(&fip->lock);
		if (fip->state != FIP_ST_AUTO && fip->state != FIP_ST_NON_FIP) {
			spin_unlock_bh(&fip->lock);
			return -EINVAL;
		}
		fip->state = FIP_ST_NON_FIP;
		LIBFCOE_FIP_DBG("received FLOGI LS_ACC using non-FIP mode\n");

		
		if (!compare_ether_addr(sa, (u8[6])FC_FCOE_FLOGI_MAC)) {
			fip->map_dest = 1;
		} else {
			memcpy(fip->dest_addr, sa, ETH_ALEN);
			fip->map_dest = 0;
		}
		fip->flogi_oxid = FC_XID_UNKNOWN;
		memcpy(mac, fip->data_src_addr, ETH_ALEN);
		fc_fcoe_set_mac(fip->data_src_addr, fh->fh_d_id);
		spin_unlock_bh(&fip->lock);

		fip->update_mac(fip, mac, fip->data_src_addr);
	} else if (op == ELS_FLOGI && fh->fh_r_ctl == FC_RCTL_ELS_REQ && sa) {
		
		spin_lock_bh(&fip->lock);
		if (fip->state == FIP_ST_AUTO || fip->state == FIP_ST_NON_FIP) {
			memcpy(fip->dest_addr, sa, ETH_ALEN);
			fip->map_dest = 0;
			if (fip->state == FIP_ST_NON_FIP)
				LIBFCOE_FIP_DBG("received FLOGI REQ, "
						"using non-FIP mode\n");
			fip->state = FIP_ST_NON_FIP;
		}
		spin_unlock_bh(&fip->lock);
	}
	return 0;
}
EXPORT_SYMBOL(fcoe_ctlr_recv_flogi);


u64 fcoe_wwn_from_mac(unsigned char mac[MAX_ADDR_LEN],
		      unsigned int scheme, unsigned int port)
{
	u64 wwn;
	u64 host_mac;

	
	host_mac = ((u64) mac[0] << 40) |
		((u64) mac[1] << 32) |
		((u64) mac[2] << 24) |
		((u64) mac[3] << 16) |
		((u64) mac[4] << 8) |
		(u64) mac[5];

	WARN_ON(host_mac >= (1ULL << 48));
	wwn = host_mac | ((u64) scheme << 60);
	switch (scheme) {
	case 1:
		WARN_ON(port != 0);
		break;
	case 2:
		WARN_ON(port >= 0xfff);
		wwn |= (u64) port << 48;
		break;
	default:
		WARN_ON(1);
		break;
	}

	return wwn;
}
EXPORT_SYMBOL_GPL(fcoe_wwn_from_mac);


int fcoe_libfc_config(struct fc_lport *lp, struct libfc_function_template *tt)
{
	
	memcpy(&lp->tt, tt, sizeof(*tt));
	if (fc_fcp_init(lp))
		return -ENOMEM;
	fc_exch_init(lp);
	fc_elsct_init(lp);
	fc_lport_init(lp);
	fc_rport_init(lp);
	fc_disc_init(lp);

	return 0;
}
EXPORT_SYMBOL_GPL(fcoe_libfc_config);
