

#ifndef WAVELAN_P_H
#define WAVELAN_P_H




























#include	<linux/module.h>

#include	<linux/kernel.h>
#include	<linux/sched.h>
#include	<linux/types.h>
#include	<linux/fcntl.h>
#include	<linux/interrupt.h>
#include	<linux/stat.h>
#include	<linux/ptrace.h>
#include	<linux/ioport.h>
#include	<linux/in.h>
#include	<linux/string.h>
#include	<linux/delay.h>
#include	<linux/bitops.h>
#include	<asm/system.h>
#include	<asm/io.h>
#include	<asm/dma.h>
#include	<asm/uaccess.h>
#include	<linux/errno.h>
#include	<linux/netdevice.h>
#include	<linux/etherdevice.h>
#include	<linux/skbuff.h>
#include	<linux/slab.h>
#include	<linux/timer.h>
#include	<linux/init.h>

#include <linux/wireless.h>		
#include <net/iw_handler.h>		


#include	"i82586.h"
#include	"wavelan.h"



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
#define DEBUG_CONFIG_ERROR	
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
#undef DEBUG_I82586_SHOW	
#undef DEBUG_DEVICE_SHOW	



#ifdef DEBUG_VERSION_SHOW
static const char	*version	= "wavelan.c : v24 (SMP + wireless extensions) 11/12/01\n";
#endif


#define	WATCHDOG_JIFFIES	(512*HZ/100)



#define SIOCSIPQTHR	SIOCIWFIRSTPRIV		
#define SIOCGIPQTHR	SIOCIWFIRSTPRIV + 1	

#define SIOCSIPHISTO	SIOCIWFIRSTPRIV + 2	
#define SIOCGIPHISTO	SIOCIWFIRSTPRIV + 3	




typedef struct iw_statistics	iw_stats;
typedef struct iw_quality	iw_qual;
typedef struct iw_freq		iw_freq;typedef struct net_local	net_local;
typedef struct timer_list	timer_list;


typedef u_char		mac_addr[WAVELAN_ADDR_SIZE];	


struct net_local
{
  net_local *	next;		
  struct net_device *	dev;		
  spinlock_t	spinlock;	
  int		nresets;	
  u_char	reconfig_82586;	
  u_char	promiscuous;	
  int		mc_count;	
  u_short	hacr;		

  int		tx_n_in_use;
  u_short	rx_head;
  u_short	rx_last;
  u_short	tx_first_free;
  u_short	tx_first_in_use;

  iw_stats	wstats;		

  struct iw_spy_data	spy_data;
  struct iw_public_data	wireless_data;

#ifdef HISTOGRAM
  int		his_number;		
  u_char	his_range[16];		
  u_long	his_sum[16];		
#endif	
};




static u_char
	wv_irq_to_psa(int);
static int
	wv_psa_to_irq(u_char);

static inline u_short		
	hasr_read(u_long);	
static inline void
	hacr_write(u_long,	
		   u_short),	
	hacr_write_slow(u_long,
		   u_short),
	set_chan_attn(u_long,	
		      u_short),	
	wv_hacr_reset(u_long),	
	wv_16_off(u_long,	
		  u_short),	
	wv_16_on(u_long,	
		 u_short),	
	wv_ints_off(struct net_device *),
	wv_ints_on(struct net_device *);

static void
	psa_read(u_long,	
		 u_short,	
		 int,		
		 u_char *,	
		 int),		
	psa_write(u_long, 	
		  u_short,	
		  int,		
		  u_char *,	
		  int);		
static inline void
	mmc_out(u_long,		
		u_short,
		u_char),
	mmc_write(u_long,	
		  u_char,
		  u_char *,
		  int);
static inline u_char		
	mmc_in(u_long,
	       u_short);
static inline void
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

static  void
	obram_read(u_long,	
		   u_short,	
		   u_char *,	
		   int);	
static inline void
	obram_write(u_long,	
		    u_short,	
		    u_char *,	
		    int);	
static void
	wv_ack(struct net_device *);
static inline int
	wv_synchronous_cmd(struct net_device *,
			   const char *),
	wv_config_complete(struct net_device *,
			   u_long,
			   net_local *);
static int
	wv_complete(struct net_device *,
		    u_long,
		    net_local *);
static inline void
	wv_82586_reconfig(struct net_device *);

#ifdef DEBUG_I82586_SHOW
static void
	wv_scb_show(unsigned short);
#endif
static inline void
	wv_init_info(struct net_device *);	

static iw_stats *
	wavelan_get_wireless_stats(struct net_device *);
static void
	wavelan_set_multicast_list(struct net_device *);

static inline void
	wv_packet_read(struct net_device *,	
		       u_short,
		       int),
	wv_receive(struct net_device *);	

static inline int
	wv_packet_write(struct net_device *,	
			void *,
			short);
static netdev_tx_t
	wavelan_packet_xmit(struct sk_buff *,	
			    struct net_device *);

static inline int
	wv_mmc_init(struct net_device *),	
	wv_ru_start(struct net_device *),	
	wv_cu_start(struct net_device *),	
	wv_82586_start(struct net_device *);	
static void
	wv_82586_config(struct net_device *);	
static inline void
	wv_82586_stop(struct net_device *);
static int
	wv_hw_reset(struct net_device *),	
	wv_check_ioaddr(u_long,		
			u_char *);	

static irqreturn_t
	wavelan_interrupt(int,		
			  void *);
static void
	wavelan_watchdog(struct net_device *);	

static int
	wavelan_open(struct net_device *),	
	wavelan_close(struct net_device *),	
	wavelan_config(struct net_device *, unsigned short);
extern struct net_device *wavelan_probe(int unit);	




static net_local *	wavelan_list	= (net_local *) NULL;


static u_char	irqvals[]	=
{
	   0,    0,    0, 0x01,
	0x02, 0x04,    0, 0x08,
	   0,    0, 0x10, 0x20,
	0x40,    0,    0, 0x80,
};


static unsigned short	iobase[]	=
{
#if	0
  
  0x300, 0x390, 0x3E0, 0x3C0
#endif	
  0x390, 0x3E0
};

#ifdef	MODULE

static int	io[4];
static int	irq[4];
static char	*name[4];
module_param_array(io, int, NULL, 0);
module_param_array(irq, int, NULL, 0);
module_param_array(name, charp, NULL, 0);

MODULE_PARM_DESC(io, "WaveLAN I/O base address(es),required");
MODULE_PARM_DESC(irq, "WaveLAN IRQ number(s)");
MODULE_PARM_DESC(name, "WaveLAN interface neme(s)");
#endif	

#endif	
