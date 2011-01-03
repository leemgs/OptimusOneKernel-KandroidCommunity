

#ifndef AF_CAN_H
#define AF_CAN_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/can.h>



struct receiver {
	struct hlist_node list;
	struct rcu_head rcu;
	canid_t can_id;
	canid_t mask;
	unsigned long matches;
	void (*func)(struct sk_buff *, void *);
	void *data;
	char *ident;
};

enum { RX_ERR, RX_ALL, RX_FIL, RX_INV, RX_EFF, RX_MAX };

struct dev_rcv_lists {
	struct hlist_node list;
	struct rcu_head rcu;
	struct net_device *dev;
	struct hlist_head rx[RX_MAX];
	struct hlist_head rx_sff[0x800];
	int remove_on_zero_entries;
	int entries;
};




struct s_stats {
	unsigned long jiffies_init;

	unsigned long rx_frames;
	unsigned long tx_frames;
	unsigned long matches;

	unsigned long total_rx_rate;
	unsigned long total_tx_rate;
	unsigned long total_rx_match_ratio;

	unsigned long current_rx_rate;
	unsigned long current_tx_rate;
	unsigned long current_rx_match_ratio;

	unsigned long max_rx_rate;
	unsigned long max_tx_rate;
	unsigned long max_rx_match_ratio;

	unsigned long rx_frames_delta;
	unsigned long tx_frames_delta;
	unsigned long matches_delta;
};


struct s_pstats {
	unsigned long stats_reset;
	unsigned long user_reset;
	unsigned long rcv_entries;
	unsigned long rcv_entries_max;
};


extern void can_init_proc(void);
extern void can_remove_proc(void);
extern void can_stat_update(unsigned long data);


extern struct timer_list can_stattimer;    
extern struct s_stats    can_stats;        
extern struct s_pstats   can_pstats;       
extern struct hlist_head can_rx_dev_list;  

#endif 
