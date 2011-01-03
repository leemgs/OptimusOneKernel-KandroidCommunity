






#include <linux/module.h>
#include <linux/types.h>		
#include <linux/netdevice.h>		
#include <linux/proc_fs.h>
#include <linux/rtnetlink.h>		
#include <linux/seq_file.h>
#include <linux/init.h>			
#include <linux/if_arp.h>		
#include <linux/etherdevice.h>		
#include <linux/interrupt.h>
#include <net/net_namespace.h>

#include <linux/wireless.h>		
#include <net/iw_handler.h>		
#include <net/netlink.h>
#include <net/wext.h>

#include <asm/uaccess.h>		




static const struct iw_ioctl_description standard_ioctl[] = {
	[SIOCSIWCOMMIT	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWNAME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_CHAR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_range),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWPRIV	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWPRIV	- SIOCIWFIRST] = { 
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_priv_args),
		.max_tokens	= 16,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWSTATS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWSTATS	- SIOCIWFIRST] = { 
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_statistics),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCGIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCSIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCGIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCSIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[SIOCGIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMLME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_mlme),
		.max_tokens	= sizeof(struct iw_mlme),
	},
	[SIOCGIWAPLIST	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_AP,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= 0,
		.max_tokens	= sizeof(struct iw_scan_req),
	},
	[SIOCGIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_SCAN_MAX_DATA,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
	},
	[SIOCGIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
	},
	[SIOCSIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCGIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCSIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCGIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCSIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCGIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCSIWPMKSA - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_pmksa),
		.max_tokens	= sizeof(struct iw_pmksa),
	},
};
static const unsigned standard_ioctl_num = ARRAY_SIZE(standard_ioctl);


static const struct iw_ioctl_description standard_event[] = {
	[IWEVTXDROP	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVQUAL	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_QUAL,
	},
	[IWEVCUSTOM	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_CUSTOM_MAX,
	},
	[IWEVREGISTERED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVEXPIRED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVGENIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVMICHAELMICFAILURE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_michaelmicfailure),
	},
	[IWEVASSOCREQIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVASSOCRESPIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVPMKIDCAND	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_pmkid_cand),
	},
};
static const unsigned standard_event_num = ARRAY_SIZE(standard_event);


static const char iw_priv_type_size[] = {
	0,				
	1,				
	1,				
	0,				
	sizeof(__u32),			
	sizeof(struct iw_freq),		
	sizeof(struct sockaddr),	
	0,				
};


static const int event_type_size[] = {
	IW_EV_LCP_LEN,			
	0,
	IW_EV_CHAR_LEN,			
	0,
	IW_EV_UINT_LEN,			
	IW_EV_FREQ_LEN,			
	IW_EV_ADDR_LEN,			
	0,
	IW_EV_POINT_LEN,		
	IW_EV_PARAM_LEN,		
	IW_EV_QUAL_LEN,			
};

#ifdef CONFIG_COMPAT
static const int compat_event_type_size[] = {
	IW_EV_COMPAT_LCP_LEN,		
	0,
	IW_EV_COMPAT_CHAR_LEN,		
	0,
	IW_EV_COMPAT_UINT_LEN,		
	IW_EV_COMPAT_FREQ_LEN,		
	IW_EV_COMPAT_ADDR_LEN,		
	0,
	IW_EV_COMPAT_POINT_LEN,		
	IW_EV_COMPAT_PARAM_LEN,		
	IW_EV_COMPAT_QUAL_LEN,		
};
#endif






static iw_handler get_handler(struct net_device *dev, unsigned int cmd)
{
	
	unsigned int	index;		

	
	if (dev->wireless_handlers == NULL)
		return NULL;

	
	index = cmd - SIOCIWFIRST;
	if (index < dev->wireless_handlers->num_standard)
		return dev->wireless_handlers->standard[index];

	
	index = cmd - SIOCIWFIRSTPRIV;
	if (index < dev->wireless_handlers->num_private)
		return dev->wireless_handlers->private[index];

	
	return NULL;
}



struct iw_statistics *get_wireless_stats(struct net_device *dev)
{
	
	if ((dev->wireless_handlers != NULL) &&
	   (dev->wireless_handlers->get_wireless_stats != NULL))
		return dev->wireless_handlers->get_wireless_stats(dev);

	
	return NULL;
}



static int call_commit_handler(struct net_device *dev)
{
	if ((netif_running(dev)) &&
	   (dev->wireless_handlers->standard[0] != NULL))
		
		return dev->wireless_handlers->standard[0](dev, NULL,
							   NULL, NULL);
	else
		return 0;		
}



static int get_priv_size(__u16 args)
{
	int	num = args & IW_PRIV_SIZE_MASK;
	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return num * iw_priv_type_size[type];
}



static int adjust_priv_size(__u16 args, struct iw_point *iwp)
{
	int	num = iwp->length;
	int	max = args & IW_PRIV_SIZE_MASK;
	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;

	
	if (max < num)
		num = max;

	return num * iw_priv_type_size[type];
}



static int iw_handler_get_iwstats(struct net_device *		dev,
				  struct iw_request_info *	info,
				  union iwreq_data *		wrqu,
				  char *			extra)
{
	
	struct iw_statistics *stats;

	stats = get_wireless_stats(dev);
	if (stats) {
		
		memcpy(extra, stats, sizeof(struct iw_statistics));
		wrqu->data.length = sizeof(struct iw_statistics);

		
		if (wrqu->data.flags != 0)
			stats->qual.updated &= ~IW_QUAL_ALL_UPDATED;
		return 0;
	} else
		return -EOPNOTSUPP;
}



static int iw_handler_get_private(struct net_device *		dev,
				  struct iw_request_info *	info,
				  union iwreq_data *		wrqu,
				  char *			extra)
{
	
	if ((dev->wireless_handlers->num_private_args == 0) ||
	   (dev->wireless_handlers->private_args == NULL))
		return -EOPNOTSUPP;

	
	if (wrqu->data.length < dev->wireless_handlers->num_private_args) {
		
		wrqu->data.length = dev->wireless_handlers->num_private_args;
		return -E2BIG;
	}

	
	wrqu->data.length = dev->wireless_handlers->num_private_args;

	
	memcpy(extra, dev->wireless_handlers->private_args,
	       sizeof(struct iw_priv_args) * wrqu->data.length);

	return 0;
}





#ifdef CONFIG_PROC_FS



static void wireless_seq_printf_stats(struct seq_file *seq,
				      struct net_device *dev)
{
	
	struct iw_statistics *stats = get_wireless_stats(dev);
	static struct iw_statistics nullstats = {};

	
	if (!stats && dev->wireless_handlers)
		stats = &nullstats;

	if (stats) {
		seq_printf(seq, "%6s: %04x  %3d%c  %3d%c  %3d%c  %6d %6d %6d "
				"%6d %6d   %6d\n",
			   dev->name, stats->status, stats->qual.qual,
			   stats->qual.updated & IW_QUAL_QUAL_UPDATED
			   ? '.' : ' ',
			   ((__s32) stats->qual.level) -
			   ((stats->qual.updated & IW_QUAL_DBM) ? 0x100 : 0),
			   stats->qual.updated & IW_QUAL_LEVEL_UPDATED
			   ? '.' : ' ',
			   ((__s32) stats->qual.noise) -
			   ((stats->qual.updated & IW_QUAL_DBM) ? 0x100 : 0),
			   stats->qual.updated & IW_QUAL_NOISE_UPDATED
			   ? '.' : ' ',
			   stats->discard.nwid, stats->discard.code,
			   stats->discard.fragment, stats->discard.retries,
			   stats->discard.misc, stats->miss.beacon);

		if (stats != &nullstats)
			stats->qual.updated &= ~IW_QUAL_ALL_UPDATED;
	}
}



static int wireless_dev_seq_show(struct seq_file *seq, void *v)
{
	might_sleep();

	if (v == SEQ_START_TOKEN)
		seq_printf(seq, "Inter-| sta-|   Quality        |   Discarded "
				"packets               | Missed | WE\n"
				" face | tus | link level noise |  nwid  "
				"crypt   frag  retry   misc | beacon | %d\n",
			   WIRELESS_EXT);
	else
		wireless_seq_printf_stats(seq, v);
	return 0;
}

static void *wireless_dev_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net *net = seq_file_net(seq);
	loff_t off;
	struct net_device *dev;

	rtnl_lock();
	if (!*pos)
		return SEQ_START_TOKEN;

	off = 1;
	for_each_netdev(net, dev)
		if (off++ == *pos)
			return dev;
	return NULL;
}

static void *wireless_dev_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net *net = seq_file_net(seq);

	++*pos;

	return v == SEQ_START_TOKEN ?
		first_net_device(net) : next_net_device(v);
}

static void wireless_dev_seq_stop(struct seq_file *seq, void *v)
{
	rtnl_unlock();
}

static const struct seq_operations wireless_seq_ops = {
	.start = wireless_dev_seq_start,
	.next  = wireless_dev_seq_next,
	.stop  = wireless_dev_seq_stop,
	.show  = wireless_dev_seq_show,
};

static int seq_open_wireless(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &wireless_seq_ops,
			    sizeof(struct seq_net_private));
}

static const struct file_operations wireless_seq_fops = {
	.owner	 = THIS_MODULE,
	.open    = seq_open_wireless,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_net,
};

int wext_proc_init(struct net *net)
{
	
	if (!proc_net_fops_create(net, "wireless", S_IRUGO, &wireless_seq_fops))
		return -ENOMEM;

	return 0;
}

void wext_proc_exit(struct net *net)
{
	proc_net_remove(net, "wireless");
}
#endif	





static int ioctl_standard_iw_point(struct iw_point *iwp, unsigned int cmd,
				   const struct iw_ioctl_description *descr,
				   iw_handler handler, struct net_device *dev,
				   struct iw_request_info *info)
{
	int err, extra_size, user_length = 0, essid_compat = 0;
	char *extra;

	
	extra_size = descr->max_tokens * descr->token_size;

	
	switch (cmd) {
	case SIOCSIWESSID:
	case SIOCGIWESSID:
	case SIOCSIWNICKN:
	case SIOCGIWNICKN:
		if (iwp->length == descr->max_tokens + 1)
			essid_compat = 1;
		else if (IW_IS_SET(cmd) && (iwp->length != 0)) {
			char essid[IW_ESSID_MAX_SIZE + 1];
			unsigned int len;
			len = iwp->length * descr->token_size;

			if (len > IW_ESSID_MAX_SIZE)
				return -EFAULT;

			err = copy_from_user(essid, iwp->pointer, len);
			if (err)
				return -EFAULT;

			if (essid[iwp->length - 1] == '\0')
				essid_compat = 1;
		}
		break;
	default:
		break;
	}

	iwp->length -= essid_compat;

	
	if (IW_IS_SET(cmd)) {
		
		if (!iwp->pointer && iwp->length != 0)
			return -EFAULT;
		
		if (iwp->length > descr->max_tokens)
			return -E2BIG;
		if (iwp->length < descr->min_tokens)
			return -EINVAL;
	} else {
		
		if (!iwp->pointer)
			return -EFAULT;
		
		user_length = iwp->length;

		

		
		if ((descr->flags & IW_DESCR_FLAG_NOMAX) &&
		    (user_length > descr->max_tokens)) {
			
			extra_size = user_length * descr->token_size;

			
		}
	}

	
	extra = kzalloc(extra_size, GFP_KERNEL);
	if (!extra)
		return -ENOMEM;

	
	if (IW_IS_SET(cmd) && (iwp->length != 0)) {
		if (copy_from_user(extra, iwp->pointer,
				   iwp->length *
				   descr->token_size)) {
			err = -EFAULT;
			goto out;
		}

		if (cmd == SIOCSIWENCODEEXT) {
			struct iw_encode_ext *ee = (void *) extra;

			if (iwp->length < sizeof(*ee) + ee->key_len)
				return -EFAULT;
		}
	}

	err = handler(dev, info, (union iwreq_data *) iwp, extra);

	iwp->length += essid_compat;

	
	if (!err && IW_IS_GET(cmd)) {
		
		if (user_length < iwp->length) {
			err = -E2BIG;
			goto out;
		}

		if (copy_to_user(iwp->pointer, extra,
				 iwp->length *
				 descr->token_size)) {
			err = -EFAULT;
			goto out;
		}
	}

	
	if ((descr->flags & IW_DESCR_FLAG_EVENT) && err == -EIWCOMMIT) {
		union iwreq_data *data = (union iwreq_data *) iwp;

		if (descr->flags & IW_DESCR_FLAG_RESTRICT)
			
			wireless_send_event(dev, cmd, data, NULL);
		else
			wireless_send_event(dev, cmd, data, extra);
	}

out:
	kfree(extra);
	return err;
}


static int ioctl_standard_call(struct net_device *	dev,
			       struct iwreq		*iwr,
			       unsigned int		cmd,
			       struct iw_request_info	*info,
			       iw_handler		handler)
{
	const struct iw_ioctl_description *	descr;
	int					ret = -EINVAL;

	
	if ((cmd - SIOCIWFIRST) >= standard_ioctl_num)
		return -EOPNOTSUPP;
	descr = &(standard_ioctl[cmd - SIOCIWFIRST]);

	
	if (descr->header_type != IW_HEADER_TYPE_POINT) {

		
		ret = handler(dev, info, &(iwr->u), NULL);

		
		if ((descr->flags & IW_DESCR_FLAG_EVENT) &&
		   ((ret == 0) || (ret == -EIWCOMMIT)))
			wireless_send_event(dev, cmd, &(iwr->u), NULL);
	} else {
		ret = ioctl_standard_iw_point(&iwr->u.data, cmd, descr,
					      handler, dev, info);
	}

	
	if (ret == -EIWCOMMIT)
		ret = call_commit_handler(dev);

	

	return ret;
}



static int get_priv_descr_and_size(struct net_device *dev, unsigned int cmd,
				   const struct iw_priv_args **descrp)
{
	const struct iw_priv_args *descr;
	int i, extra_size;

	descr = NULL;
	for (i = 0; i < dev->wireless_handlers->num_private_args; i++) {
		if (cmd == dev->wireless_handlers->private_args[i].cmd) {
			descr = &dev->wireless_handlers->private_args[i];
			break;
		}
	}

	extra_size = 0;
	if (descr) {
		if (IW_IS_SET(cmd)) {
			int	offset = 0;	
			
			if (descr->name[0] == '\0')
				
				offset = sizeof(__u32);

			
			extra_size = get_priv_size(descr->set_args);

			
			if ((descr->set_args & IW_PRIV_SIZE_FIXED) &&
			   ((extra_size + offset) <= IFNAMSIZ))
				extra_size = 0;
		} else {
			
			extra_size = get_priv_size(descr->get_args);

			
			if ((descr->get_args & IW_PRIV_SIZE_FIXED) &&
			   (extra_size <= IFNAMSIZ))
				extra_size = 0;
		}
	}
	*descrp = descr;
	return extra_size;
}

static int ioctl_private_iw_point(struct iw_point *iwp, unsigned int cmd,
				  const struct iw_priv_args *descr,
				  iw_handler handler, struct net_device *dev,
				  struct iw_request_info *info, int extra_size)
{
	char *extra;
	int err;

	
	if (IW_IS_SET(cmd)) {
		if (!iwp->pointer && iwp->length != 0)
			return -EFAULT;

		if (iwp->length > (descr->set_args & IW_PRIV_SIZE_MASK))
			return -E2BIG;
	} else if (!iwp->pointer)
		return -EFAULT;

	extra = kmalloc(extra_size, GFP_KERNEL);
	if (!extra)
		return -ENOMEM;

	
	if (IW_IS_SET(cmd) && (iwp->length != 0)) {
		if (copy_from_user(extra, iwp->pointer, extra_size)) {
			err = -EFAULT;
			goto out;
		}
	}

	
	err = handler(dev, info, (union iwreq_data *) iwp, extra);

	
	if (!err && IW_IS_GET(cmd)) {
		
		if (!(descr->get_args & IW_PRIV_SIZE_FIXED))
			extra_size = adjust_priv_size(descr->get_args, iwp);

		if (copy_to_user(iwp->pointer, extra, extra_size))
			err =  -EFAULT;
	}

out:
	kfree(extra);
	return err;
}

static int ioctl_private_call(struct net_device *dev, struct iwreq *iwr,
			      unsigned int cmd, struct iw_request_info *info,
			      iw_handler handler)
{
	int extra_size = 0, ret = -EINVAL;
	const struct iw_priv_args *descr;

	extra_size = get_priv_descr_and_size(dev, cmd, &descr);

	
	if (extra_size == 0) {
		
		ret = handler(dev, info, &(iwr->u), (char *) &(iwr->u));
	} else {
		ret = ioctl_private_iw_point(&iwr->u.data, cmd, descr,
					     handler, dev, info, extra_size);
	}

	
	if (ret == -EIWCOMMIT)
		ret = call_commit_handler(dev);

	return ret;
}


typedef int (*wext_ioctl_func)(struct net_device *, struct iwreq *,
			       unsigned int, struct iw_request_info *,
			       iw_handler);


static int wireless_process_ioctl(struct net *net, struct ifreq *ifr,
				  unsigned int cmd,
				  struct iw_request_info *info,
				  wext_ioctl_func standard,
				  wext_ioctl_func private)
{
	struct iwreq *iwr = (struct iwreq *) ifr;
	struct net_device *dev;
	iw_handler	handler;

	

	
	if ((dev = __dev_get_by_name(net, ifr->ifr_name)) == NULL)
		return -ENODEV;

	
	if (cmd == SIOCGIWSTATS)
		return standard(dev, iwr, cmd, info,
				&iw_handler_get_iwstats);

	if (cmd == SIOCGIWPRIV && dev->wireless_handlers)
		return standard(dev, iwr, cmd, info,
				&iw_handler_get_private);

	
	if (!netif_device_present(dev))
		return -ENODEV;

	
	handler = get_handler(dev, cmd);
	if (handler) {
		
		if (cmd < SIOCIWFIRSTPRIV)
			return standard(dev, iwr, cmd, info, handler);
		else
			return private(dev, iwr, cmd, info, handler);
	}
	
	if (dev->netdev_ops->ndo_do_ioctl)
		return dev->netdev_ops->ndo_do_ioctl(dev, ifr, cmd);
	return -EOPNOTSUPP;
}


static int wext_permission_check(unsigned int cmd)
{
	if ((IW_IS_SET(cmd) || cmd == SIOCGIWENCODE || cmd == SIOCGIWENCODEEXT)
	    && !capable(CAP_NET_ADMIN))
		return -EPERM;

	return 0;
}


static int wext_ioctl_dispatch(struct net *net, struct ifreq *ifr,
			       unsigned int cmd, struct iw_request_info *info,
			       wext_ioctl_func standard,
			       wext_ioctl_func private)
{
	int ret = wext_permission_check(cmd);

	if (ret)
		return ret;

	dev_load(net, ifr->ifr_name);
	rtnl_lock();
	ret = wireless_process_ioctl(net, ifr, cmd, info, standard, private);
	rtnl_unlock();

	return ret;
}

int wext_handle_ioctl(struct net *net, struct ifreq *ifr, unsigned int cmd,
		      void __user *arg)
{
	struct iw_request_info info = { .cmd = cmd, .flags = 0 };
	int ret;

	ret = wext_ioctl_dispatch(net, ifr, cmd, &info,
				  ioctl_standard_call,
				  ioctl_private_call);
	if (ret >= 0 &&
	    IW_IS_GET(cmd) &&
	    copy_to_user(arg, ifr, sizeof(struct iwreq)))
		return -EFAULT;

	return ret;
}

#ifdef CONFIG_COMPAT
static int compat_standard_call(struct net_device	*dev,
				struct iwreq		*iwr,
				unsigned int		cmd,
				struct iw_request_info	*info,
				iw_handler		handler)
{
	const struct iw_ioctl_description *descr;
	struct compat_iw_point *iwp_compat;
	struct iw_point iwp;
	int err;

	descr = standard_ioctl + (cmd - SIOCIWFIRST);

	if (descr->header_type != IW_HEADER_TYPE_POINT)
		return ioctl_standard_call(dev, iwr, cmd, info, handler);

	iwp_compat = (struct compat_iw_point *) &iwr->u.data;
	iwp.pointer = compat_ptr(iwp_compat->pointer);
	iwp.length = iwp_compat->length;
	iwp.flags = iwp_compat->flags;

	err = ioctl_standard_iw_point(&iwp, cmd, descr, handler, dev, info);

	iwp_compat->pointer = ptr_to_compat(iwp.pointer);
	iwp_compat->length = iwp.length;
	iwp_compat->flags = iwp.flags;

	return err;
}

static int compat_private_call(struct net_device *dev, struct iwreq *iwr,
			       unsigned int cmd, struct iw_request_info *info,
			       iw_handler handler)
{
	const struct iw_priv_args *descr;
	int ret, extra_size;

	extra_size = get_priv_descr_and_size(dev, cmd, &descr);

	
	if (extra_size == 0) {
		
		ret = handler(dev, info, &(iwr->u), (char *) &(iwr->u));
	} else {
		struct compat_iw_point *iwp_compat;
		struct iw_point iwp;

		iwp_compat = (struct compat_iw_point *) &iwr->u.data;
		iwp.pointer = compat_ptr(iwp_compat->pointer);
		iwp.length = iwp_compat->length;
		iwp.flags = iwp_compat->flags;

		ret = ioctl_private_iw_point(&iwp, cmd, descr,
					     handler, dev, info, extra_size);

		iwp_compat->pointer = ptr_to_compat(iwp.pointer);
		iwp_compat->length = iwp.length;
		iwp_compat->flags = iwp.flags;
	}

	
	if (ret == -EIWCOMMIT)
		ret = call_commit_handler(dev);

	return ret;
}

int compat_wext_handle_ioctl(struct net *net, unsigned int cmd,
			     unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct iw_request_info info;
	struct iwreq iwr;
	char *colon;
	int ret;

	if (copy_from_user(&iwr, argp, sizeof(struct iwreq)))
		return -EFAULT;

	iwr.ifr_name[IFNAMSIZ-1] = 0;
	colon = strchr(iwr.ifr_name, ':');
	if (colon)
		*colon = 0;

	info.cmd = cmd;
	info.flags = IW_REQUEST_FLAG_COMPAT;

	ret = wext_ioctl_dispatch(net, (struct ifreq *) &iwr, cmd, &info,
				  compat_standard_call,
				  compat_private_call);

	if (ret >= 0 &&
	    IW_IS_GET(cmd) &&
	    copy_to_user(argp, &iwr, sizeof(struct iwreq)))
		return -EFAULT;

	return ret;
}
#endif

static int __net_init wext_pernet_init(struct net *net)
{
	skb_queue_head_init(&net->wext_nlevents);
	return 0;
}

static void __net_exit wext_pernet_exit(struct net *net)
{
	skb_queue_purge(&net->wext_nlevents);
}

static struct pernet_operations wext_pernet_ops = {
	.init = wext_pernet_init,
	.exit = wext_pernet_exit,
};

static int __init wireless_nlevent_init(void)
{
	return register_pernet_subsys(&wext_pernet_ops);
}

subsys_initcall(wireless_nlevent_init);


static void wireless_nlevent_process(struct work_struct *work)
{
	struct sk_buff *skb;
	struct net *net;

	rtnl_lock();

	for_each_net(net) {
		while ((skb = skb_dequeue(&net->wext_nlevents)))
			rtnl_notify(skb, net, 0, RTNLGRP_LINK, NULL,
				    GFP_KERNEL);
	}

	rtnl_unlock();
}

static DECLARE_WORK(wireless_nlevent_work, wireless_nlevent_process);

static struct nlmsghdr *rtnetlink_ifinfo_prep(struct net_device *dev,
					      struct sk_buff *skb)
{
	struct ifinfomsg *r;
	struct nlmsghdr  *nlh;

	nlh = nlmsg_put(skb, 0, 0, RTM_NEWLINK, sizeof(*r), 0);
	if (!nlh)
		return NULL;

	r = nlmsg_data(nlh);
	r->ifi_family = AF_UNSPEC;
	r->__ifi_pad = 0;
	r->ifi_type = dev->type;
	r->ifi_index = dev->ifindex;
	r->ifi_flags = dev_get_flags(dev);
	r->ifi_change = 0;	

	NLA_PUT_STRING(skb, IFLA_IFNAME, dev->name);

	return nlh;
 nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return NULL;
}



void wireless_send_event(struct net_device *	dev,
			 unsigned int		cmd,
			 union iwreq_data *	wrqu,
			 const char *		extra)
{
	const struct iw_ioctl_description *	descr = NULL;
	int extra_len = 0;
	struct iw_event  *event;		
	int event_len;				
	int hdr_len;				
	int wrqu_off = 0;			
	
	unsigned	cmd_index;		
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct nlattr *nla;
#ifdef CONFIG_COMPAT
	struct __compat_iw_event *compat_event;
	struct compat_iw_point compat_wrqu;
	struct sk_buff *compskb;
#endif

	

#if !defined (CONFIG_MACH_LGE)
	if (WARN_ON(cmd == SIOCGIWSCAN && extra))
		extra = NULL;
#else
	if ((cmd == SIOCGIWSCAN) && extra)
		extra = NULL;
#endif

	
	if (cmd <= SIOCIWLAST) {
		cmd_index = cmd - SIOCIWFIRST;
		if (cmd_index < standard_ioctl_num)
			descr = &(standard_ioctl[cmd_index]);
	} else {
		cmd_index = cmd - IWEVFIRST;
		if (cmd_index < standard_event_num)
			descr = &(standard_event[cmd_index]);
	}
	
	if (descr == NULL) {
		
		printk(KERN_ERR "%s (WE) : Invalid/Unknown Wireless Event (0x%04X)\n",
		       dev->name, cmd);
		return;
	}

	
	if (descr->header_type == IW_HEADER_TYPE_POINT) {
		
		if (wrqu->data.length > descr->max_tokens) {
			printk(KERN_ERR "%s (WE) : Wireless Event too big (%d)\n", dev->name, wrqu->data.length);
			return;
		}
		if (wrqu->data.length < descr->min_tokens) {
			printk(KERN_ERR "%s (WE) : Wireless Event too small (%d)\n", dev->name, wrqu->data.length);
			return;
		}
		
		if (extra != NULL)
			extra_len = wrqu->data.length * descr->token_size;
		
		wrqu_off = IW_EV_POINT_OFF;
	}

	
	hdr_len = event_type_size[descr->header_type];
	event_len = hdr_len + extra_len;

	

	skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!skb)
		return;

	
	nlh = rtnetlink_ifinfo_prep(dev, skb);
	if (WARN_ON(!nlh)) {
		kfree_skb(skb);
		return;
	}

	
	nla = nla_reserve(skb, IFLA_WIRELESS, event_len);
	if (!nla) {
		kfree_skb(skb);
		return;
	}
	event = nla_data(nla);

	
	memset(event, 0, hdr_len);
	event->len = event_len;
	event->cmd = cmd;
	memcpy(&event->u, ((char *) wrqu) + wrqu_off, hdr_len - IW_EV_LCP_LEN);
	if (extra_len)
		memcpy(((char *) event) + hdr_len, extra, extra_len);

	nlmsg_end(skb, nlh);
#ifdef CONFIG_COMPAT
	hdr_len = compat_event_type_size[descr->header_type];
	event_len = hdr_len + extra_len;

	compskb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!compskb) {
		kfree_skb(skb);
		return;
	}

	
	nlh = rtnetlink_ifinfo_prep(dev, compskb);
	if (WARN_ON(!nlh)) {
		kfree_skb(skb);
		kfree_skb(compskb);
		return;
	}

	
	nla = nla_reserve(compskb, IFLA_WIRELESS, event_len);
	if (!nla) {
		kfree_skb(skb);
		kfree_skb(compskb);
		return;
	}
	compat_event = nla_data(nla);

	compat_event->len = event_len;
	compat_event->cmd = cmd;
	if (descr->header_type == IW_HEADER_TYPE_POINT) {
		compat_wrqu.length = wrqu->data.length;
		compat_wrqu.flags = wrqu->data.flags;
		memcpy(&compat_event->pointer,
			((char *) &compat_wrqu) + IW_EV_COMPAT_POINT_OFF,
			hdr_len - IW_EV_COMPAT_LCP_LEN);
		if (extra_len)
			memcpy(((char *) compat_event) + hdr_len,
				extra, extra_len);
	} else {
		
		memcpy(&compat_event->pointer, wrqu,
			hdr_len - IW_EV_COMPAT_LCP_LEN);
	}

	nlmsg_end(compskb, nlh);

	skb_shinfo(skb)->frag_list = compskb;
#endif
	skb_queue_tail(&dev_net(dev)->wext_nlevents, skb);
	schedule_work(&wireless_nlevent_work);
}
EXPORT_SYMBOL(wireless_send_event);






static inline struct iw_spy_data *get_spydata(struct net_device *dev)
{
	
	if (dev->wireless_data)
		return dev->wireless_data->spy_data;
	return NULL;
}



int iw_handler_set_spy(struct net_device *	dev,
		       struct iw_request_info *	info,
		       union iwreq_data *	wrqu,
		       char *			extra)
{
	struct iw_spy_data *	spydata = get_spydata(dev);
	struct sockaddr *	address = (struct sockaddr *) extra;

	
	if (!spydata)
		return -EOPNOTSUPP;

	
	spydata->spy_number = 0;

	
	smp_wmb();

	
	if (wrqu->data.length > 0) {
		int i;

		
		for (i = 0; i < wrqu->data.length; i++)
			memcpy(spydata->spy_address[i], address[i].sa_data,
			       ETH_ALEN);
		
		memset(spydata->spy_stat, 0,
		       sizeof(struct iw_quality) * IW_MAX_SPY);
	}

	
	smp_wmb();

	
	spydata->spy_number = wrqu->data.length;

	return 0;
}
EXPORT_SYMBOL(iw_handler_set_spy);



int iw_handler_get_spy(struct net_device *	dev,
		       struct iw_request_info *	info,
		       union iwreq_data *	wrqu,
		       char *			extra)
{
	struct iw_spy_data *	spydata = get_spydata(dev);
	struct sockaddr *	address = (struct sockaddr *) extra;
	int			i;

	
	if (!spydata)
		return -EOPNOTSUPP;

	wrqu->data.length = spydata->spy_number;

	
	for (i = 0; i < spydata->spy_number; i++) 	{
		memcpy(address[i].sa_data, spydata->spy_address[i], ETH_ALEN);
		address[i].sa_family = AF_UNIX;
	}
	
	if (spydata->spy_number > 0)
		memcpy(extra  + (sizeof(struct sockaddr) *spydata->spy_number),
		       spydata->spy_stat,
		       sizeof(struct iw_quality) * spydata->spy_number);
	
	for (i = 0; i < spydata->spy_number; i++)
		spydata->spy_stat[i].updated &= ~IW_QUAL_ALL_UPDATED;
	return 0;
}
EXPORT_SYMBOL(iw_handler_get_spy);



int iw_handler_set_thrspy(struct net_device *	dev,
			  struct iw_request_info *info,
			  union iwreq_data *	wrqu,
			  char *		extra)
{
	struct iw_spy_data *	spydata = get_spydata(dev);
	struct iw_thrspy *	threshold = (struct iw_thrspy *) extra;

	
	if (!spydata)
		return -EOPNOTSUPP;

	
	memcpy(&(spydata->spy_thr_low), &(threshold->low),
	       2 * sizeof(struct iw_quality));

	
	memset(spydata->spy_thr_under, '\0', sizeof(spydata->spy_thr_under));

	return 0;
}
EXPORT_SYMBOL(iw_handler_set_thrspy);



int iw_handler_get_thrspy(struct net_device *	dev,
			  struct iw_request_info *info,
			  union iwreq_data *	wrqu,
			  char *		extra)
{
	struct iw_spy_data *	spydata = get_spydata(dev);
	struct iw_thrspy *	threshold = (struct iw_thrspy *) extra;

	
	if (!spydata)
		return -EOPNOTSUPP;

	
	memcpy(&(threshold->low), &(spydata->spy_thr_low),
	       2 * sizeof(struct iw_quality));

	return 0;
}
EXPORT_SYMBOL(iw_handler_get_thrspy);



static void iw_send_thrspy_event(struct net_device *	dev,
				 struct iw_spy_data *	spydata,
				 unsigned char *	address,
				 struct iw_quality *	wstats)
{
	union iwreq_data	wrqu;
	struct iw_thrspy	threshold;

	
	wrqu.data.length = 1;
	wrqu.data.flags = 0;
	
	memcpy(threshold.addr.sa_data, address, ETH_ALEN);
	threshold.addr.sa_family = ARPHRD_ETHER;
	
	memcpy(&(threshold.qual), wstats, sizeof(struct iw_quality));
	
	memcpy(&(threshold.low), &(spydata->spy_thr_low),
	       2 * sizeof(struct iw_quality));

	
	wireless_send_event(dev, SIOCGIWTHRSPY, &wrqu, (char *) &threshold);
}



void wireless_spy_update(struct net_device *	dev,
			 unsigned char *	address,
			 struct iw_quality *	wstats)
{
	struct iw_spy_data *	spydata = get_spydata(dev);
	int			i;
	int			match = -1;

	
	if (!spydata)
		return;

	
	for (i = 0; i < spydata->spy_number; i++)
		if (!compare_ether_addr(address, spydata->spy_address[i])) {
			memcpy(&(spydata->spy_stat[i]), wstats,
			       sizeof(struct iw_quality));
			match = i;
		}

	
	if (match >= 0) {
		if (spydata->spy_thr_under[match]) {
			if (wstats->level > spydata->spy_thr_high.level) {
				spydata->spy_thr_under[match] = 0;
				iw_send_thrspy_event(dev, spydata,
						     address, wstats);
			}
		} else {
			if (wstats->level < spydata->spy_thr_low.level) {
				spydata->spy_thr_under[match] = 1;
				iw_send_thrspy_event(dev, spydata,
						     address, wstats);
			}
		}
	}
}
EXPORT_SYMBOL(wireless_spy_update);
