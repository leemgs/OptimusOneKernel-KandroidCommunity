

#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_vlan.h>

static int debug;
#define MODULE_VERS "0.6"

module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debug=1 is turn on debug messages");
MODULE_AUTHOR("Nick Fedchik <nick@fedchik.org.ua>");
MODULE_DESCRIPTION("Ebtables: 802.1Q VLAN tag match");
MODULE_LICENSE("GPL");


#define DEBUG_MSG(args...) if (debug) printk (KERN_DEBUG "ebt_vlan: " args)
#define GET_BITMASK(_BIT_MASK_) info->bitmask & _BIT_MASK_
#define EXIT_ON_MISMATCH(_MATCH_,_MASK_) {if (!((info->_MATCH_ == _MATCH_)^!!(info->invflags & _MASK_))) return false; }

static bool
ebt_vlan_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct ebt_vlan_info *info = par->matchinfo;
	const struct vlan_hdr *fp;
	struct vlan_hdr _frame;

	unsigned short TCI;	
	unsigned short id;	
	unsigned char prio;	
	
	__be16 encap;

	fp = skb_header_pointer(skb, 0, sizeof(_frame), &_frame);
	if (fp == NULL)
		return false;

	
	TCI = ntohs(fp->h_vlan_TCI);
	id = TCI & VLAN_VID_MASK;
	prio = (TCI >> 13) & 0x7;
	encap = fp->h_vlan_encapsulated_proto;

	
	if (GET_BITMASK(EBT_VLAN_ID))
		EXIT_ON_MISMATCH(id, EBT_VLAN_ID);

	
	if (GET_BITMASK(EBT_VLAN_PRIO))
		EXIT_ON_MISMATCH(prio, EBT_VLAN_PRIO);

	
	if (GET_BITMASK(EBT_VLAN_ENCAP))
		EXIT_ON_MISMATCH(encap, EBT_VLAN_ENCAP);

	return true;
}

static bool ebt_vlan_mt_check(const struct xt_mtchk_param *par)
{
	struct ebt_vlan_info *info = par->matchinfo;
	const struct ebt_entry *e = par->entryinfo;

	
	if (e->ethproto != htons(ETH_P_8021Q)) {
		DEBUG_MSG
		    ("passed entry proto %2.4X is not 802.1Q (8100)\n",
		     (unsigned short) ntohs(e->ethproto));
		return false;
	}

	
	if (info->bitmask & ~EBT_VLAN_MASK) {
		DEBUG_MSG("bitmask %2X is out of mask (%2X)\n",
			  info->bitmask, EBT_VLAN_MASK);
		return false;
	}

	
	if (info->invflags & ~EBT_VLAN_MASK) {
		DEBUG_MSG("inversion flags %2X is out of mask (%2X)\n",
			  info->invflags, EBT_VLAN_MASK);
		return false;
	}

	
	if (GET_BITMASK(EBT_VLAN_ID)) {
		if (!!info->id) { 
			if (info->id > VLAN_GROUP_ARRAY_LEN) {
				DEBUG_MSG
				    ("id %d is out of range (1-4096)\n",
				     info->id);
				return false;
			}
			
			info->bitmask &= ~EBT_VLAN_PRIO;
		}
		
	}

	if (GET_BITMASK(EBT_VLAN_PRIO)) {
		if ((unsigned char) info->prio > 7) {
			DEBUG_MSG("prio %d is out of range (0-7)\n",
			     info->prio);
			return false;
		}
	}
	
	if (GET_BITMASK(EBT_VLAN_ENCAP)) {
		if ((unsigned short) ntohs(info->encap) < ETH_ZLEN) {
			DEBUG_MSG
			    ("encap frame length %d is less than minimal\n",
			     ntohs(info->encap));
			return false;
		}
	}

	return true;
}

static struct xt_match ebt_vlan_mt_reg __read_mostly = {
	.name		= "vlan",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_vlan_mt,
	.checkentry	= ebt_vlan_mt_check,
	.matchsize	= XT_ALIGN(sizeof(struct ebt_vlan_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_vlan_init(void)
{
	DEBUG_MSG("ebtables 802.1Q extension module v"
		  MODULE_VERS "\n");
	DEBUG_MSG("module debug=%d\n", !!debug);
	return xt_register_match(&ebt_vlan_mt_reg);
}

static void __exit ebt_vlan_fini(void)
{
	xt_unregister_match(&ebt_vlan_mt_reg);
}

module_init(ebt_vlan_init);
module_exit(ebt_vlan_fini);
