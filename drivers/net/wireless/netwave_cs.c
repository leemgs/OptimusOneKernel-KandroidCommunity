


#undef NETWAVE_STATS

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/bitops.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>
#include <pcmcia/mem_op.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/dma.h>

#define NETWAVE_REGOFF         0x8000

#define NETWAVE_REG_COR        0x0
#define NETWAVE_REG_CCSR       0x2
#define NETWAVE_REG_ASR        0x4
#define NETWAVE_REG_IMR        0xa
#define NETWAVE_REG_PMR        0xc
#define NETWAVE_REG_IOLOW      0x6
#define NETWAVE_REG_IOHI       0x7
#define NETWAVE_REG_IOCONTROL  0x8
#define NETWAVE_REG_DATA       0xf

#define NETWAVE_EREG_ASCC      0x114
#define NETWAVE_EREG_RSER      0x120
#define NETWAVE_EREG_RSERW     0x124
#define NETWAVE_EREG_TSER      0x130
#define NETWAVE_EREG_TSERW     0x134
#define NETWAVE_EREG_CB        0x100
#define NETWAVE_EREG_SPCQ      0x154
#define NETWAVE_EREG_SPU       0x155
#define NETWAVE_EREG_LIF       0x14e
#define NETWAVE_EREG_ISPLQ     0x156
#define NETWAVE_EREG_HHC       0x158
#define NETWAVE_EREG_NI        0x16e
#define NETWAVE_EREG_MHS       0x16b
#define NETWAVE_EREG_TDP       0x140
#define NETWAVE_EREG_RDP       0x150
#define NETWAVE_EREG_PA        0x160
#define NETWAVE_EREG_EC        0x180
#define NETWAVE_EREG_CRBP      0x17a
#define NETWAVE_EREG_ARW       0x166


#define NETWAVE_CMD_NOP        0x00
#define NETWAVE_CMD_SRC        0x01
#define NETWAVE_CMD_STC        0x02
#define NETWAVE_CMD_AMA        0x03
#define NETWAVE_CMD_DMA        0x04
#define NETWAVE_CMD_SAMA       0x05
#define NETWAVE_CMD_ER         0x06
#define NETWAVE_CMD_DR         0x07
#define NETWAVE_CMD_TL         0x08
#define NETWAVE_CMD_SRP        0x09
#define NETWAVE_CMD_SSK        0x0a
#define NETWAVE_CMD_SMD        0x0b
#define NETWAVE_CMD_SAPD       0x0c
#define NETWAVE_CMD_SSS        0x11

#define NETWAVE_CMD_EOC        0x00


#define NETWAVE_ASR_RXRDY   0x80
#define NETWAVE_ASR_TXBA    0x01

#define TX_TIMEOUT		((32*HZ)/100)

static const unsigned int imrConfRFU1 = 0x10; 
static const unsigned int imrConfIENA = 0x02; 

static const unsigned int corConfIENA   = 0x01; 
static const unsigned int corConfLVLREQ = 0x40; 

static const unsigned int rxConfRxEna  = 0x80; 
static const unsigned int rxConfMAC    = 0x20;  
static const unsigned int rxConfPro    = 0x10; 
static const unsigned int rxConfAMP    = 0x08; 
static const unsigned int rxConfBcast  = 0x04; 

static const unsigned int txConfTxEna  = 0x80; 
static const unsigned int txConfMAC    = 0x20; 
static const unsigned int txConfEUD    = 0x10; 
static const unsigned int txConfKey    = 0x02; 
static const unsigned int txConfLoop   = 0x01; 



#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version =
"netwave_cs.c 0.3.0 Thu Jul 17 14:36:02 1997 (John Markus Bjørndalen)\n";
#else
#define DEBUG(n, args...)
#endif






static u_int  domain = 0x100;


static u_int  scramble_key = 0x0;


static int mem_speed;

module_param(domain, int, 0);
module_param(scramble_key, int, 0);
module_param(mem_speed, int, 0);




static void netwave_release(struct pcmcia_device *link);     
static int netwave_pcmcia_config(struct pcmcia_device *arg); 
static void netwave_detach(struct pcmcia_device *p_dev);    


static void netwave_doreset(unsigned int iobase, u_char __iomem *ramBase);
static void netwave_reset(struct net_device *dev);


static int netwave_open(struct net_device *dev);  
static int netwave_close(struct net_device *dev); 


static netdev_tx_t netwave_start_xmit( struct sk_buff *skb,
					     struct net_device *dev);
static int netwave_rx( struct net_device *dev);


static irqreturn_t netwave_interrupt(int irq, void *dev_id);
static void netwave_watchdog(struct net_device *);


static struct iw_statistics* netwave_get_wireless_stats(struct net_device *dev);

static void set_multicast_list(struct net_device *dev);



static const struct iw_handler_def	netwave_handler_def;

#define SIOCGIPSNAP	SIOCIWFIRSTPRIV	+ 1	

#define MAX_ESA 10

typedef struct net_addr {
    u_char addr48[6];
} net_addr;

struct site_survey {
    u_short length;
    u_char  struct_revision;
    u_char  roaming_state;
	
    u_char  sp_existsFlag;
    u_char  sp_link_quality;
    u_char  sp_max_link_quality;
    u_char  linkQualityGoodFairBoundary;
    u_char  linkQualityFairPoorBoundary;
    u_char  sp_utilization;
    u_char  sp_goodness;
    u_char  sp_hotheadcount;
    u_char  roaming_condition;
	
    net_addr sp;
    u_char   numAPs;
    net_addr nearByAccessPoints[MAX_ESA];
};	
   
typedef struct netwave_private {
	struct pcmcia_device	*p_dev;
    spinlock_t	spinlock;	
    dev_node_t node;
    u_char     __iomem *ramBase;
    int        timeoutCounter;
    int        lastExec;
    struct timer_list      watchdog;	
    struct site_survey     nss;
    struct iw_statistics   iw_stats;    
} netwave_private;


static inline unsigned short get_uint16(u_char __iomem *staddr) 
{
    return readw(staddr); 
}

static inline short get_int16(u_char __iomem * staddr)
{
    return readw(staddr);
}


static inline void wait_WOC(unsigned int iobase)
{
    
    while ((inb(iobase + NETWAVE_REG_ASR) & 0x8) != 0x8) ; 
}

static void netwave_snapshot(netwave_private *priv, u_char __iomem *ramBase, 
			     unsigned int iobase) {
    u_short resultBuffer;

    
    if ( jiffies - priv->lastExec > 100) { 
	 
	
	wait_WOC(iobase); 
	writeb(NETWAVE_CMD_SSS, ramBase + NETWAVE_EREG_CB + 0); 
	writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 1); 
	wait_WOC(iobase); 

	 
	resultBuffer = readw(ramBase + NETWAVE_EREG_CRBP); 
	copy_from_pc( &priv->nss, ramBase+resultBuffer, 
		      sizeof(struct site_survey)); 
    } 
}


static struct iw_statistics *netwave_get_wireless_stats(struct net_device *dev)
{	
    unsigned long flags;
    unsigned int iobase = dev->base_addr;
    netwave_private *priv = netdev_priv(dev);
    u_char __iomem *ramBase = priv->ramBase;
    struct iw_statistics* wstats;
	
    wstats = &priv->iw_stats;

    spin_lock_irqsave(&priv->spinlock, flags);
	
    netwave_snapshot( priv, ramBase, iobase);

    wstats->status = priv->nss.roaming_state;
    wstats->qual.qual = readb( ramBase + NETWAVE_EREG_SPCQ); 
    wstats->qual.level = readb( ramBase + NETWAVE_EREG_ISPLQ);
    wstats->qual.noise = readb( ramBase + NETWAVE_EREG_SPU) & 0x3f;
    wstats->discard.nwid = 0L;
    wstats->discard.code = 0L;
    wstats->discard.misc = 0L;

    spin_unlock_irqrestore(&priv->spinlock, flags);
    
    return &priv->iw_stats;
}

static const struct net_device_ops netwave_netdev_ops = {
	.ndo_open	 	= netwave_open,
	.ndo_stop		= netwave_close,
	.ndo_start_xmit		= netwave_start_xmit,
	.ndo_set_multicast_list = set_multicast_list,
	.ndo_tx_timeout		= netwave_watchdog,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};


static int netwave_probe(struct pcmcia_device *link)
{
    struct net_device *dev;
    netwave_private *priv;

    DEBUG(0, "netwave_attach()\n");

    
    dev = alloc_etherdev(sizeof(netwave_private));
    if (!dev)
	return -ENOMEM;
    priv = netdev_priv(dev);
    priv->p_dev = link;
    link->priv = dev;

    
    link->io.NumPorts1 = 16;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
    
    link->io.IOAddrLines = 5;
    
    
    link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING | IRQ_HANDLE_PRESENT;
    link->irq.IRQInfo1 = IRQ_LEVEL_ID;
    link->irq.Handler = &netwave_interrupt;
    
    
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.ConfigIndex = 1;

    
    spin_lock_init(&priv->spinlock);

    
    dev->netdev_ops = &netwave_netdev_ops;
    
    dev->wireless_handlers = &netwave_handler_def;

    dev->watchdog_timeo = TX_TIMEOUT;

    link->irq.Instance = dev;

    return netwave_pcmcia_config( link);
} 


static void netwave_detach(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	DEBUG(0, "netwave_detach(0x%p)\n", link);

	netwave_release(link);

	if (link->dev_node)
		unregister_netdev(dev);

	free_netdev(dev);
} 


static int netwave_get_name(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	strcpy(wrqu->name, "Netwave");
	return 0;
}


static int netwave_set_nwid(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long flags;
	unsigned int iobase = dev->base_addr;
	netwave_private *priv = netdev_priv(dev);
	u_char __iomem *ramBase = priv->ramBase;

	
	spin_lock_irqsave(&priv->spinlock, flags);

	if(!wrqu->nwid.disabled) {
	    domain = wrqu->nwid.value;
	    printk( KERN_DEBUG "Setting domain to 0x%x%02x\n", 
		    (domain >> 8) & 0x01, domain & 0xff);
	    wait_WOC(iobase);
	    writeb(NETWAVE_CMD_SMD, ramBase + NETWAVE_EREG_CB + 0);
	    writeb( domain & 0xff, ramBase + NETWAVE_EREG_CB + 1);
	    writeb((domain >>8 ) & 0x01,ramBase + NETWAVE_EREG_CB+2);
	    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 3);
	}

	
	spin_unlock_irqrestore(&priv->spinlock, flags);
    
	return 0;
}


static int netwave_get_nwid(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	wrqu->nwid.value = domain;
	wrqu->nwid.disabled = 0;
	wrqu->nwid.fixed = 1;
	return 0;
}


static int netwave_set_scramble(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu,
				char *key)
{
	unsigned long flags;
	unsigned int iobase = dev->base_addr;
	netwave_private *priv = netdev_priv(dev);
	u_char __iomem *ramBase = priv->ramBase;

	
	spin_lock_irqsave(&priv->spinlock, flags);

	scramble_key = (key[0] << 8) | key[1];
	wait_WOC(iobase);
	writeb(NETWAVE_CMD_SSK, ramBase + NETWAVE_EREG_CB + 0);
	writeb(scramble_key & 0xff, ramBase + NETWAVE_EREG_CB + 1);
	writeb((scramble_key>>8) & 0xff, ramBase + NETWAVE_EREG_CB + 2);
	writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 3);

	
	spin_unlock_irqrestore(&priv->spinlock, flags);
    
	return 0;
}


static int netwave_get_scramble(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu,
				char *key)
{
	key[1] = scramble_key & 0xff;
	key[0] = (scramble_key>>8) & 0xff;
	wrqu->encoding.flags = IW_ENCODE_ENABLED;
	wrqu->encoding.length = 2;
	return 0;
}


static int netwave_get_mode(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	if(domain & 0x100)
		wrqu->mode = IW_MODE_INFRA;
	else
		wrqu->mode = IW_MODE_ADHOC;

	return 0;
}


static int netwave_get_range(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	struct iw_range *range = (struct iw_range *) extra;
	int ret = 0;

	
	wrqu->data.length = sizeof(struct iw_range);

	
	memset(range, 0, sizeof(struct iw_range));

	
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 9;	
		   
	
	range->throughput = 450 * 1000;	
	range->min_nwid = 0x0000;
	range->max_nwid = 0x01FF;

	range->num_channels = range->num_frequency = 0;
		   
	range->sensitivity = 0x3F;
	range->max_qual.qual = 255;
	range->max_qual.level = 255;
	range->max_qual.noise = 0;
		   
	range->num_bitrates = 1;
	range->bitrate[0] = 1000000;	

	range->encoding_size[0] = 2;		
	range->num_encoding_sizes = 1;
	range->max_encoding_tokens = 1;	

	return ret;
}


static int netwave_get_snap(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long flags;
	unsigned int iobase = dev->base_addr;
	netwave_private *priv = netdev_priv(dev);
	u_char __iomem *ramBase = priv->ramBase;

	
	spin_lock_irqsave(&priv->spinlock, flags);

	
	netwave_snapshot( priv, ramBase, iobase);
	wrqu->data.length = priv->nss.length;
	memcpy(extra, (u_char *) &priv->nss, sizeof( struct site_survey));

	priv->lastExec = jiffies;

	
	spin_unlock_irqrestore(&priv->spinlock, flags);
    
	return(0);
}



static const struct iw_priv_args netwave_private_args[] = {

  { SIOCGIPSNAP, 0, 
    IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | sizeof(struct site_survey), 
    "getsitesurvey" },
};

static const iw_handler		netwave_handler[] =
{
	NULL,				
	netwave_get_name,		
	netwave_set_nwid,		
	netwave_get_nwid,		
	NULL,				
	NULL,				
	NULL,				
	netwave_get_mode,		
	NULL,				
	NULL,				
	NULL,				
	netwave_get_range,		
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	netwave_set_scramble,		
	netwave_get_scramble,		
};

static const iw_handler		netwave_private_handler[] =
{
	NULL,				
	netwave_get_snap,		
};

static const struct iw_handler_def	netwave_handler_def =
{
	.num_standard	= ARRAY_SIZE(netwave_handler),
	.num_private	= ARRAY_SIZE(netwave_private_handler),
	.num_private_args = ARRAY_SIZE(netwave_private_args),
	.standard	= (iw_handler *) netwave_handler,
	.private	= (iw_handler *) netwave_private_handler,
	.private_args	= (struct iw_priv_args *) netwave_private_args,
	.get_wireless_stats = netwave_get_wireless_stats,
};



#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int netwave_pcmcia_config(struct pcmcia_device *link) {
    struct net_device *dev = link->priv;
    netwave_private *priv = netdev_priv(dev);
    int i, j, last_ret, last_fn;
    win_req_t req;
    memreq_t mem;
    u_char __iomem *ramBase = NULL;

    DEBUG(0, "netwave_pcmcia_config(0x%p)\n", link);

    
    for (i = j = 0x0; j < 0x400; j += 0x20) {
	link->io.BasePort1 = j ^ 0x300;
	i = pcmcia_request_io(link, &link->io);
	if (i == 0)
		break;
    }
    if (i != 0) {
	cs_error(link, RequestIO, i);
	goto failed;
    }

    
    CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));

    
    CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));

    
    DEBUG(1, "Setting mem speed of %d\n", mem_speed);

    req.Attributes = WIN_DATA_WIDTH_8|WIN_MEMORY_TYPE_CM|WIN_ENABLE;
    req.Base = 0; req.Size = 0x8000;
    req.AccessSpeed = mem_speed;
    CS_CHECK(RequestWindow, pcmcia_request_window(&link, &req, &link->win));
    mem.CardOffset = 0x20000; mem.Page = 0; 
    CS_CHECK(MapMemPage, pcmcia_map_mem_page(link->win, &mem));

    
    ramBase = ioremap(req.Base, 0x8000);
    priv->ramBase = ramBase;

    dev->irq = link->irq.AssignedIRQ;
    dev->base_addr = link->io.BasePort1;
    SET_NETDEV_DEV(dev, &handle_to_dev(link));

    if (register_netdev(dev) != 0) {
	printk(KERN_DEBUG "netwave_cs: register_netdev() failed\n");
	goto failed;
    }

    strcpy(priv->node.dev_name, dev->name);
    link->dev_node = &priv->node;

    
    netwave_doreset(dev->base_addr, ramBase);

    
    for (i = 0; i < 6; i++) 
	dev->dev_addr[i] = readb(ramBase + NETWAVE_EREG_PA + i);

    printk(KERN_INFO "%s: Netwave: port %#3lx, irq %d, mem %lx, "
	   "id %c%c, hw_addr %pM\n",
	   dev->name, dev->base_addr, dev->irq,
	   (u_long) ramBase,
	   (int) readb(ramBase+NETWAVE_EREG_NI),
	   (int) readb(ramBase+NETWAVE_EREG_NI+1),
	   dev->dev_addr);

    
    printk(KERN_DEBUG "Netwave_reset: revision %04x %04x\n", 
	   get_uint16(ramBase + NETWAVE_EREG_ARW),
	   get_uint16(ramBase + NETWAVE_EREG_ARW+2));
    return 0;

cs_failed:
    cs_error(link, last_fn, last_ret);
failed:
    netwave_release(link);
    return -ENODEV;
} 


static void netwave_release(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;
	netwave_private *priv = netdev_priv(dev);

	DEBUG(0, "netwave_release(0x%p)\n", link);

	pcmcia_disable_device(link);
	if (link->win)
		iounmap(priv->ramBase);
}

static int netwave_suspend(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open)
		netif_device_detach(dev);

	return 0;
}

static int netwave_resume(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open) {
		netwave_reset(dev);
		netif_device_attach(dev);
	}

	return 0;
}



static void netwave_doreset(unsigned int ioBase, u_char __iomem *ramBase)
{
    
    wait_WOC(ioBase);
    outb(0x80, ioBase + NETWAVE_REG_PMR);
    writeb(0x08, ramBase + NETWAVE_EREG_ASCC); 
    outb(0x0, ioBase + NETWAVE_REG_PMR); 
}


static void netwave_reset(struct net_device *dev) {
    
    netwave_private *priv = netdev_priv(dev);
    u_char __iomem *ramBase = priv->ramBase;
    unsigned int iobase = dev->base_addr;

    DEBUG(0, "netwave_reset: Done with hardware reset\n");

    priv->timeoutCounter = 0;

    
    netwave_doreset(iobase, ramBase);
    printk(KERN_DEBUG "netwave_reset: Done with hardware reset\n");
	
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_NOP, ramBase + NETWAVE_EREG_CB + 0);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 1);
	
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_SRC, ramBase + NETWAVE_EREG_CB + 0);
    writeb(rxConfRxEna + rxConfBcast, ramBase + NETWAVE_EREG_CB + 1);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 2);
    
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_STC, ramBase + NETWAVE_EREG_CB + 0);
    writeb(txConfTxEna, ramBase + NETWAVE_EREG_CB + 1);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 2);
    
    
    printk(KERN_DEBUG "Setting domain to 0x%x%02x\n", (domain >> 8) & 0x01, domain & 0xff);
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_SMD, ramBase + NETWAVE_EREG_CB + 0);
    writeb(domain & 0xff, ramBase + NETWAVE_EREG_CB + 1);
    writeb((domain>>8) & 0x01, ramBase + NETWAVE_EREG_CB + 2);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 3);
	
    
    printk(KERN_DEBUG "Setting scramble key to 0x%x\n", scramble_key);
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_SSK, ramBase + NETWAVE_EREG_CB + 0);
    writeb(scramble_key & 0xff, ramBase + NETWAVE_EREG_CB + 1);
    writeb((scramble_key>>8) & 0xff, ramBase + NETWAVE_EREG_CB + 2);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 3);

    
    wait_WOC(iobase);
    outb(imrConfIENA+imrConfRFU1, iobase + NETWAVE_REG_IMR);

    
    
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_ER, ramBase + NETWAVE_EREG_CB + 0);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 1);
	
    
    wait_WOC(iobase);
    outb(corConfIENA + corConfLVLREQ, iobase + NETWAVE_REG_COR);
}


static int netwave_hw_xmit(unsigned char* data, int len,
			   struct net_device* dev) {
    unsigned long flags;
    unsigned int TxFreeList,
	         curBuff,
	         MaxData, 
                 DataOffset;
    int tmpcount; 
	
    netwave_private *priv = netdev_priv(dev);
    u_char __iomem * ramBase = priv->ramBase;
    unsigned int iobase = dev->base_addr;

    
    spin_lock_irqsave(&priv->spinlock, flags);

    
    wait_WOC(iobase);
    if ((inb(iobase+NETWAVE_REG_ASR) & NETWAVE_ASR_TXBA) == 0) {
	
	printk(KERN_DEBUG "netwave_hw_xmit: %s - no xmit buffers available.\n",
	       dev->name);
	spin_unlock_irqrestore(&priv->spinlock, flags);
	return 1;
    }

    dev->stats.tx_bytes += len;

    DEBUG(3, "Transmitting with SPCQ %x SPU %x LIF %x ISPLQ %x\n",
	  readb(ramBase + NETWAVE_EREG_SPCQ),
	  readb(ramBase + NETWAVE_EREG_SPU),
	  readb(ramBase + NETWAVE_EREG_LIF),
	  readb(ramBase + NETWAVE_EREG_ISPLQ));

    
    wait_WOC(iobase);
    TxFreeList = get_uint16(ramBase + NETWAVE_EREG_TDP);
    MaxData    = get_uint16(ramBase + NETWAVE_EREG_TDP+2);
    DataOffset = get_uint16(ramBase + NETWAVE_EREG_TDP+4);
	
    DEBUG(3, "TxFreeList %x, MaxData %x, DataOffset %x\n",
	  TxFreeList, MaxData, DataOffset);

    
    curBuff = TxFreeList; 
    tmpcount = 0; 
    while (tmpcount < len) {
	int tmplen = len - tmpcount; 
	copy_to_pc(ramBase + curBuff + DataOffset, data + tmpcount, 
		   (tmplen < MaxData) ? tmplen : MaxData);
	tmpcount += MaxData;
			
	
	curBuff = get_uint16(ramBase + curBuff);
    }
    
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_TL, ramBase + NETWAVE_EREG_CB + 0);
    writeb(len & 0xff, ramBase + NETWAVE_EREG_CB + 1);
    writeb((len>>8) & 0xff, ramBase + NETWAVE_EREG_CB + 2);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 3);

    spin_unlock_irqrestore(&priv->spinlock, flags);
    return 0;
}

static netdev_tx_t netwave_start_xmit(struct sk_buff *skb,
					    struct net_device *dev) {
	

    netif_stop_queue(dev);

    {
	short length = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
	unsigned char* buf = skb->data;
	
	if (netwave_hw_xmit( buf, length, dev) == 1) {
	    
	    netif_start_queue(dev);
	}
	dev->trans_start = jiffies;
    }
    dev_kfree_skb(skb);
    
    return NETDEV_TX_OK;
} 


static irqreturn_t netwave_interrupt(int irq, void* dev_id)
{
    unsigned int iobase;
    u_char __iomem *ramBase;
    struct net_device *dev = (struct net_device *)dev_id;
    struct netwave_private *priv = netdev_priv(dev);
    struct pcmcia_device *link = priv->p_dev;
    int i;
    
    if (!netif_device_present(dev))
	return IRQ_NONE;
    
    iobase = dev->base_addr;
    ramBase = priv->ramBase;
	
    
    for (i = 0; i < 10; i++) {
	u_char status;
		
	wait_WOC(iobase);	
	if (!(inb(iobase+NETWAVE_REG_CCSR) & 0x02))
	    break; 
	
        status = inb(iobase + NETWAVE_REG_ASR);
		
	if (!pcmcia_dev_present(link)) {
	    DEBUG(1, "netwave_interrupt: Interrupt with status 0x%x "
		  "from removed or suspended card!\n", status);
	    break;
	}
		
	
	if (status & 0x80) {
	    netwave_rx(dev);
	    
	    
	}
	
	if (status & 0x40) {
	    u_char rser;
			
	    rser = readb(ramBase + NETWAVE_EREG_RSER);			
	    
	    if (rser & 0x04) {
		++dev->stats.rx_dropped;
		++dev->stats.rx_crc_errors;
	    }
	    if (rser & 0x02)
		++dev->stats.rx_frame_errors;
			
	    
	    wait_WOC(iobase);
	    writeb(0x40 | (rser & 0x06), ramBase + NETWAVE_EREG_RSER + 4);

	    
	    wait_WOC(iobase);
	    writeb(0x40, ramBase + NETWAVE_EREG_ASCC);

	    
	    ++dev->stats.rx_errors;
	}
	
	if (status & 0x20) {
	    int txStatus;

	    txStatus = readb(ramBase + NETWAVE_EREG_TSER);
	    DEBUG(3, "Transmit done. TSER = %x id %x\n", 
		  txStatus, readb(ramBase + NETWAVE_EREG_TSER + 1));
	    
	    if (txStatus & 0x20) {
		
		wait_WOC(iobase);
		writeb(0x2f, ramBase + NETWAVE_EREG_TSER + 4);
		++dev->stats.tx_packets;
	    }
			
	    if (txStatus & 0xd0) {
		if (txStatus & 0x80) {
		    ++dev->stats.collisions; 
		    
		    
		}
		if (txStatus & 0x40) 
		    ++dev->stats.tx_carrier_errors;
		
		DEBUG(3, "netwave_interrupt: TxDN with error status %x\n", 
		      txStatus);
		
		
		wait_WOC(iobase);
		writeb(0xdf & txStatus, ramBase+NETWAVE_EREG_TSER+4);
		++dev->stats.tx_errors;
	    }
	    DEBUG(3, "New status is TSER %x ASR %x\n",
		  readb(ramBase + NETWAVE_EREG_TSER),
		  inb(iobase + NETWAVE_REG_ASR));

	    netif_wake_queue(dev);
	}
	
	
    }
    
    return IRQ_RETVAL(i);
} 


static void netwave_watchdog(struct net_device *dev) {

    DEBUG(1, "%s: netwave_watchdog: watchdog timer expired\n", dev->name);
    netwave_reset(dev);
    dev->trans_start = jiffies;
    netif_wake_queue(dev);
} 

static int netwave_rx(struct net_device *dev)
{
    netwave_private *priv = netdev_priv(dev);
    u_char __iomem *ramBase = priv->ramBase;
    unsigned int iobase = dev->base_addr;
    u_char rxStatus;
    struct sk_buff *skb = NULL;
    unsigned int curBuffer,
		rcvList;
    int rcvLen;
    int tmpcount = 0;
    int dataCount, dataOffset;
    int i;
    u_char *ptr;
	
    DEBUG(3, "xinw_rx: Receiving ... \n");

    
    for (i = 0; i < 10; i++) {
	
	wait_WOC(iobase);
	rxStatus = readb(ramBase + NETWAVE_EREG_RSER);		
	if ( !( rxStatus & 0x80)) 
	    break;
		
	
	
		
	
	wait_WOC(iobase);
	rcvLen  = get_int16( ramBase + NETWAVE_EREG_RDP);
	rcvList = get_uint16( ramBase + NETWAVE_EREG_RDP + 2);
		
	if (rcvLen < 0) {
	    printk(KERN_DEBUG "netwave_rx: Receive packet with len %d\n", 
		   rcvLen);
	    return 0;
	}
		
	skb = dev_alloc_skb(rcvLen+5);
	if (skb == NULL) {
	    DEBUG(1, "netwave_rx: Could not allocate an sk_buff of "
		  "length %d\n", rcvLen);
	    ++dev->stats.rx_dropped;
	    
	    wait_WOC(iobase);
	    writeb(NETWAVE_CMD_SRP, ramBase + NETWAVE_EREG_CB + 0);
	    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 1);
	    return 0;
	}

	skb_reserve( skb, 2);  
	skb_put( skb, rcvLen);

	
	ptr = (u_char*) skb->data;
	curBuffer = rcvList;
	tmpcount = 0; 
	while ( tmpcount < rcvLen) {
	    
	    dataCount  = get_uint16( ramBase+curBuffer+2);
	    dataOffset = get_uint16( ramBase+curBuffer+4);
		
	    copy_from_pc( ptr + tmpcount,
			  ramBase+curBuffer+dataOffset, dataCount);

	    tmpcount += dataCount;
		
	    
	    curBuffer = get_uint16(ramBase + curBuffer);
	}
	
	skb->protocol = eth_type_trans(skb,dev);
	
	netif_rx(skb);

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += rcvLen;

	
	wait_WOC(iobase);
	writeb(NETWAVE_CMD_SRP, ramBase + NETWAVE_EREG_CB + 0);
	writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 1);
	DEBUG(3, "Packet reception ok\n");
    }
    return 0;
}

static int netwave_open(struct net_device *dev) {
    netwave_private *priv = netdev_priv(dev);
    struct pcmcia_device *link = priv->p_dev;

    DEBUG(1, "netwave_open: starting.\n");
    
    if (!pcmcia_dev_present(link))
	return -ENODEV;

    link->open++;

    netif_start_queue(dev);
    netwave_reset(dev);
	
    return 0;
}

static int netwave_close(struct net_device *dev) {
    netwave_private *priv = netdev_priv(dev);
    struct pcmcia_device *link = priv->p_dev;

    DEBUG(1, "netwave_close: finishing.\n");

    link->open--;
    netif_stop_queue(dev);

    return 0;
}

static struct pcmcia_device_id netwave_ids[] = {
	PCMCIA_DEVICE_PROD_ID12("Xircom", "CreditCard Netwave", 0x2e3ee845, 0x54e28a28),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, netwave_ids);

static struct pcmcia_driver netwave_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "netwave_cs",
	},
	.probe		= netwave_probe,
	.remove		= netwave_detach,
	.id_table       = netwave_ids,
	.suspend	= netwave_suspend,
	.resume		= netwave_resume,
};

static int __init init_netwave_cs(void)
{
	return pcmcia_register_driver(&netwave_driver);
}

static void __exit exit_netwave_cs(void)
{
	pcmcia_unregister_driver(&netwave_driver);
}

module_init(init_netwave_cs);
module_exit(exit_netwave_cs);


static void set_multicast_list(struct net_device *dev)
{
    unsigned int iobase = dev->base_addr;
    netwave_private *priv = netdev_priv(dev);
    u_char __iomem * ramBase = priv->ramBase;
    u_char  rcvMode = 0;
   
#ifdef PCMCIA_DEBUG
    if (pc_debug > 2) {
	static int old;
	if (old != dev->mc_count) {
	    old = dev->mc_count;
	    DEBUG(0, "%s: setting Rx mode to %d addresses.\n",
		  dev->name, dev->mc_count);
	}
    }
#endif
	
    if (dev->mc_count || (dev->flags & IFF_ALLMULTI)) {
	
	rcvMode = rxConfRxEna + rxConfAMP + rxConfBcast;
    } else if (dev->flags & IFF_PROMISC) {
	
	rcvMode = rxConfRxEna + rxConfPro + rxConfAMP + rxConfBcast;
    } else {
	
	rcvMode = rxConfRxEna + rxConfBcast;
    }
	
    
    
    wait_WOC(iobase);
    writeb(NETWAVE_CMD_SRC, ramBase + NETWAVE_EREG_CB + 0);
    writeb(rcvMode, ramBase + NETWAVE_EREG_CB + 1);
    writeb(NETWAVE_CMD_EOC, ramBase + NETWAVE_EREG_CB + 2);
}
MODULE_LICENSE("GPL");
