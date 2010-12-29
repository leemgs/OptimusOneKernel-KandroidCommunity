

#include <linux/kernel.h>
#include <linux/module.h>

#include "ath.h"

MODULE_AUTHOR("Atheros Communications");
MODULE_DESCRIPTION("Shared library for Atheros wireless LAN cards.");
MODULE_LICENSE("Dual BSD/GPL");

struct sk_buff *ath_rxbuf_alloc(struct ath_common *common,
				u32 len,
				gfp_t gfp_mask)
{
	struct sk_buff *skb;
	u32 off;

	

	
	skb = __dev_alloc_skb(len + common->cachelsz - 1, gfp_mask);
	if (skb != NULL) {
		off = ((unsigned long) skb->data) % common->cachelsz;
		if (off != 0)
			skb_reserve(skb, common->cachelsz - off);
	} else {
		printk(KERN_ERR "skbuff alloc of size %u failed\n", len);
		return NULL;
	}

	return skb;
}
EXPORT_SYMBOL(ath_rxbuf_alloc);
