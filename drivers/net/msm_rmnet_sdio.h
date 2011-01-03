

#include <linux/types.h>
#include <linux/skbuff.h>

#ifndef _MSM_RMNET_SDIO_H
#define _MSM_RMNET_SDIO_H

int msm_rmnet_sdio_open(uint32_t id, void *priv,
			void (*receive_cb)(void *, struct sk_buff *),
			void (*write_done)(void *, struct sk_buff *));

int msm_rmnet_sdio_close(uint32_t id);

int msm_rmnet_sdio_write(uint32_t id, struct sk_buff *skb);

#endif 
