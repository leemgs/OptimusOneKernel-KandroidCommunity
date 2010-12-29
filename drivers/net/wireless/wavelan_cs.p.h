

#ifndef WAVELAN_CS_P_H
#define WAVELAN_CS_P_H



























#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/ethtool.h>
#include <linux/wireless.h>		
#include <net/iw_handler.h>		


#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>


#include "i82593.h"	

#include "wavelan_cs.h"	



#define WAVELAN_ROAMING		
#undef WAVELAN_ROAMING_EXT	
#undef SET_PSA_CRC		
#define USE_PSA_CONFIG		
#undef EEPROM_IS_PROTECTED	
#define MULTICAST_AVOID		
#undef SET_MAC_ADDRESS		


#define WIRELESS_SPY		
#undef HISTOGRAM		



#undef DEBUG_MODULE_TRACE	
#undef DEBUG_CALLBACK_TRACE	
#undef DEBUG_INTERRUPT_TRACE	
#undef DEBUG_INTERRUPT_INFO	
#define DEBUG_INTERRUPT_ERROR	
#undef DEBUG_CONFIG_TRACE	
#undef DEBUG_CONFIG_INFO	
#define DEBUG_CONFIG_ERRORS	
#undef DEBUG_TX_TRACE		
#undef DEBUG_TX_INFO		
#undef DEBUG_TX_FAIL		
#define DEBUG_TX_ERROR		
#undef DEBUG_RX_TRACE		
#undef DEBUG_RX_INFO		
#undef DEBUG_RX_FAIL		
#define DEBUG_RX_ERROR		
#undef DEBUG_PACKET_DUMP	
#undef DEBUG_IOCTL_TRACE	
#undef DEBUG_IOCTL_INFO		
#define DEBUG_IOCTL_ERROR	
#define DEBUG_BASIC_SHOW	
#undef DEBUG_VERSION_SHOW	
#undef DEBUG_PSA_SHOW		
#undef DEBUG_MMC_SHOW		
#undef DEBUG_SHOW_UNUSED	
#undef DEBUG_I82593_SHOW	
#undef DEBUG_DEVICE_SHOW	



#ifdef DEBUG_VERSION_SHOW
static const char *version = "wavelan_cs.c : v24 (SMP + wireless extensions) 11/1/02\n";
#endif


#define	WATCHDOG_JIFFIES	(256*HZ/100)


#ifndef IW_ESSID_MAX_SIZE
#define IW_ESSID_MAX_SIZE	32
#endif



#define SIOCSIPQTHR	SIOCIWFIRSTPRIV		
#define SIOCGIPQTHR	SIOCIWFIRSTPRIV + 1	
#define SIOCSIPROAM     SIOCIWFIRSTPRIV + 2	
#define SIOCGIPROAM     SIOCIWFIRSTPRIV + 3	

#define SIOCSIPHISTO	SIOCIWFIRSTPRIV + 4	
#define SIOCGIPHISTO	SIOCIWFIRSTPRIV + 5	


#ifdef WAVELAN_ROAMING		

#define WAVELAN_ROAMING_DEBUG	 0	
					
#define MAX_WAVEPOINTS		7	
#define WAVEPOINT_HISTORY	5	
#define WAVEPOINT_FAST_HISTORY	2	
#define SEARCH_THRESH_LOW	10	
#define SEARCH_THRESH_HIGH	13	
#define WAVELAN_ROAMING_DELTA	1	
#define CELL_TIMEOUT		2*HZ	

#define FAST_CELL_SEARCH	1	
#define NWID_PROMISC		1	

typedef struct wavepoint_beacon
{
  unsigned char		dsap,		
			ssap,		
			ctrl,		
			O,U,I,		
			spec_id1,	
			spec_id2,	
			pdu_type,	
			seq;		
  __be16		domain_id,	
			nwid;		
} wavepoint_beacon;

typedef struct wavepoint_history
{
  unsigned short	nwid;		
  int			average_slow;	
  int			average_fast;	
  unsigned char	  sigqual[WAVEPOINT_HISTORY]; 
  unsigned char		qualptr;	
  unsigned char		last_seq;	
  struct wavepoint_history *next;	
  struct wavepoint_history *prev;	
  unsigned long		last_seen;	
} wavepoint_history;

struct wavepoint_table
{
  wavepoint_history	*head;		
  int			num_wavepoints;	
  unsigned char		locked;		
};

#endif	




typedef struct iw_statistics	iw_stats;
typedef struct iw_quality	iw_qual;
typedef struct iw_freq		iw_freq;
typedef struct net_local	net_local;
typedef struct timer_list	timer_list;


typedef u_char		mac_addr[WAVELAN_ADDR_SIZE];	


struct net_local
{
  dev_node_t 	node;		
  struct net_device *	dev;		
  spinlock_t	spinlock;	
  struct pcmcia_device *	link;		
  int		nresets;	
  u_char	configured;	
  u_char	reconfig_82593;	
  u_char	promiscuous;	
  u_char	allmulticast;	
  int		mc_count;	

  int   	stop;		
  int   	rfp;		
  int		overrunning;	

  iw_stats	wstats;		

  struct iw_spy_data	spy_data;
  struct iw_public_data	wireless_data;

#ifdef HISTOGRAM
  int		his_number;		
  u_char	his_range[16];		
  u_long	his_sum[16];		
#endif	
#ifdef WAVELAN_ROAMING
  u_long	domain_id;	
  int		filter_domains;	
 struct wavepoint_table	wavepoint_table;	
  wavepoint_history *	curr_point;		
  int			cell_search;		
  struct timer_list	cell_timer;		
#endif	
  void __iomem *mem;
};


static inline u_char		
	hasr_read(u_long);	
static void
	hacr_write(u_long,	
		   u_char),	
	hacr_write_slow(u_long,
		   u_char);
static void
	psa_read(struct net_device *,	
		 int,		
		 u_char *,	
		 int),		
	psa_write(struct net_device *,	
		  int,		
		  u_char *,	
		  int);		
static void
	mmc_out(u_long,		
		u_short,
		u_char),
	mmc_write(u_long,	
		  u_char,
		  u_char *,
		  int);
static u_char			
	mmc_in(u_long,
	       u_short);
static void
	mmc_read(u_long,	
		 u_char,
		 u_char *,
		 int),
	fee_wait(u_long,	
		 int,		
		 int);		
static void
	fee_read(u_long,	
		 u_short,	
		 u_short *,	
		 int);		

static int
	wv_82593_cmd(struct net_device *,	 
		     char *,
		     int,
		     int);
static inline int
	wv_diag(struct net_device *);	
static int
	read_ringbuf(struct net_device *,	
		     int,
		     char *,
		     int);
static void
	wv_82593_reconfig(struct net_device *);	

static void
	wv_init_info(struct net_device *);	

static iw_stats *
	wavelan_get_wireless_stats(struct net_device *);

static int
	wv_start_of_frame(struct net_device *,	
			  int,	
			  int);	
static void
	wv_packet_read(struct net_device *,	
		       int,
		       int),
	wv_packet_rcv(struct net_device *);	

static void
	wv_packet_write(struct net_device *,	
			void *,
			short);
static netdev_tx_t
	wavelan_packet_xmit(struct sk_buff *,	
			    struct net_device *);

static int
	wv_mmc_init(struct net_device *);	
static int
	wv_ru_stop(struct net_device *),	
	wv_ru_start(struct net_device *);	
static int
	wv_82593_config(struct net_device *);	
static int
	wv_pcmcia_reset(struct net_device *);	
static int
	wv_hw_config(struct net_device *);	
static void
	wv_hw_reset(struct net_device *);	
static int
	wv_pcmcia_config(struct pcmcia_device *);	
static void
	wv_pcmcia_release(struct pcmcia_device *);

static irqreturn_t
	wavelan_interrupt(int,	
			  void *);
static void
	wavelan_watchdog(struct net_device *);	

static int
	wavelan_open(struct net_device *),		
	wavelan_close(struct net_device *);	
static void
	wavelan_detach(struct pcmcia_device *p_dev);	






static int	mem_speed = 0;


module_param(mem_speed, int, 0);

#ifdef WAVELAN_ROAMING		

static int	do_roaming = 0;
module_param(do_roaming, bool, 0);
#endif	

MODULE_LICENSE("GPL");

#endif	

