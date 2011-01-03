#ifndef	_CYCLOMX_H
#define	_CYCLOMX_H


#include <linux/wanrouter.h>
#include <linux/spinlock.h>

#ifdef	__KERNEL__


#include <linux/cycx_drv.h>	
#include <linux/cycx_cfm.h>	
#ifdef CONFIG_CYCLOMX_X25
#include <linux/cycx_x25.h>
#endif


struct cycx_device {
	char devname[WAN_DRVNAME_SZ + 1];
	struct cycx_hw hw;		
	struct wan_device wandev;	
	u32 state_tick;			
	spinlock_t lock;
	char in_isr;			
	char buff_int_mode_unbusy;      
	wait_queue_head_t wait_stats;  
	void __iomem *mbox;			
	void (*isr)(struct cycx_device* card);	
	int (*exec)(struct cycx_device* card, void* u_cmd, void* u_data);
	union {
#ifdef CONFIG_CYCLOMX_X25
		struct { 
			u32 lo_pvc;
			u32 hi_pvc;
			u32 lo_svc;
			u32 hi_svc;
			struct cycx_x25_stats stats;
			spinlock_t lock;
			u32 connection_keys;
		} x;
#endif
	} u;
};


void cycx_set_state(struct cycx_device *card, int state);

#ifdef CONFIG_CYCLOMX_X25
int cycx_x25_wan_init(struct cycx_device *card, wandev_conf_t *conf);
#endif
#endif	
#endif	
