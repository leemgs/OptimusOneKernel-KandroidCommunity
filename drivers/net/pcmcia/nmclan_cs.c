

#define DRV_NAME	"nmclan_cs"
#define DRV_VERSION	"0.16"




#define MULTI_TX			0
#define RESET_ON_TIMEOUT		1
#define TX_INTERRUPTABLE		1
#define RESET_XILINX			0



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>
#include <linux/bitops.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>



#define ETHER_ADDR_LEN			ETH_ALEN
					
#define MACE_LADRF_LEN			8
					


#define MACE_MAX_IR_ITERATIONS		10
#define MACE_MAX_RX_ITERATIONS		12
	


#define AM2150_MAX_TX_FRAMES		4
#define AM2150_MAX_RX_FRAMES		12


#define AM2150_RCV			0x00
#define AM2150_XMT			0x04
#define AM2150_XMT_SKIP			0x09
#define AM2150_RCV_NEXT			0x0A
#define AM2150_RCV_FRAME_COUNT		0x0B
#define AM2150_MACE_BANK		0x0C
#define AM2150_MACE_BASE		0x10


#define MACE_RCVFIFO			0
#define MACE_XMTFIFO			1
#define MACE_XMTFC			2
#define MACE_XMTFS			3
#define MACE_XMTRC			4
#define MACE_RCVFC			5
#define MACE_RCVFS			6
#define MACE_FIFOFC			7
#define MACE_IR				8
#define MACE_IMR			9
#define MACE_PR				10
#define MACE_BIUCC			11
#define MACE_FIFOCC			12
#define MACE_MACCC			13
#define MACE_PLSCC			14
#define MACE_PHYCC			15
#define MACE_CHIPIDL			16
#define MACE_CHIPIDH			17
#define MACE_IAC			18

#define MACE_LADRF			20
#define MACE_PADR			21


#define MACE_MPC			24

#define MACE_RNTPC			26
#define MACE_RCVCC			27

#define MACE_UTR			29
#define MACE_RTR1			30
#define MACE_RTR2			31


#define MACE_XMTRC_EXDEF		0x80
#define MACE_XMTRC_XMTRC		0x0F

#define MACE_XMTFS_XMTSV		0x80
#define MACE_XMTFS_UFLO			0x40
#define MACE_XMTFS_LCOL			0x20
#define MACE_XMTFS_MORE			0x10
#define MACE_XMTFS_ONE			0x08
#define MACE_XMTFS_DEFER		0x04
#define MACE_XMTFS_LCAR			0x02
#define MACE_XMTFS_RTRY			0x01

#define MACE_RCVFS_RCVSTS		0xF000
#define MACE_RCVFS_OFLO			0x8000
#define MACE_RCVFS_CLSN			0x4000
#define MACE_RCVFS_FRAM			0x2000
#define MACE_RCVFS_FCS			0x1000

#define MACE_FIFOFC_RCVFC		0xF0
#define MACE_FIFOFC_XMTFC		0x0F

#define MACE_IR_JAB			0x80
#define MACE_IR_BABL			0x40
#define MACE_IR_CERR			0x20
#define MACE_IR_RCVCCO			0x10
#define MACE_IR_RNTPCO			0x08
#define MACE_IR_MPCO			0x04
#define MACE_IR_RCVINT			0x02
#define MACE_IR_XMTINT			0x01

#define MACE_MACCC_PROM			0x80
#define MACE_MACCC_DXMT2PD		0x40
#define MACE_MACCC_EMBA			0x20
#define MACE_MACCC_RESERVED		0x10
#define MACE_MACCC_DRCVPA		0x08
#define MACE_MACCC_DRCVBC		0x04
#define MACE_MACCC_ENXMT		0x02
#define MACE_MACCC_ENRCV		0x01

#define MACE_PHYCC_LNKFL		0x80
#define MACE_PHYCC_DLNKTST		0x40
#define MACE_PHYCC_REVPOL		0x20
#define MACE_PHYCC_DAPC			0x10
#define MACE_PHYCC_LRT			0x08
#define MACE_PHYCC_ASEL			0x04
#define MACE_PHYCC_RWAKE		0x02
#define MACE_PHYCC_AWAKE		0x01

#define MACE_IAC_ADDRCHG		0x80
#define MACE_IAC_PHYADDR		0x04
#define MACE_IAC_LOGADDR		0x02

#define MACE_UTR_RTRE			0x80
#define MACE_UTR_RTRD			0x40
#define MACE_UTR_RPA			0x20
#define MACE_UTR_FCOLL			0x10
#define MACE_UTR_RCVFCSE		0x08
#define MACE_UTR_LOOP_INCL_MENDEC	0x06
#define MACE_UTR_LOOP_NO_MENDEC		0x04
#define MACE_UTR_LOOP_EXTERNAL		0x02
#define MACE_UTR_LOOP_NONE		0x00
#define MACE_UTR_RESERVED		0x01


#define MACEBANK(win_num) outb((win_num), ioaddr + AM2150_MACE_BANK)

#define MACE_IMR_DEFAULT \
  (0xFF - \
    ( \
      MACE_IR_CERR | \
      MACE_IR_RCVCCO | \
      MACE_IR_RNTPCO | \
      MACE_IR_MPCO | \
      MACE_IR_RCVINT | \
      MACE_IR_XMTINT \
    ) \
  )
#undef MACE_IMR_DEFAULT
#define MACE_IMR_DEFAULT 0x00 

#define TX_TIMEOUT		((400*HZ)/1000)



typedef struct _mace_statistics {
    
    int xmtsv;
    int uflo;
    int lcol;
    int more;
    int one;
    int defer;
    int lcar;
    int rtry;

    
    int exdef;
    int xmtrc;

    
    int oflo;
    int clsn;
    int fram;
    int fcs;

    
    int rfs_rntpc;

    
    int rfs_rcvcc;

    
    int jab;
    int babl;
    int cerr;
    int rcvcco;
    int rntpco;
    int mpco;

    
    int mpc;

    
    int rntpc;

    
    int rcvcc;
} mace_statistics;

typedef struct _mace_private {
	struct pcmcia_device	*p_dev;
    dev_node_t node;
    struct net_device_stats linux_stats; 
    mace_statistics mace_stats; 

    
    int multicast_ladrf[MACE_LADRF_LEN]; 
    int multicast_num_addrs;

    char tx_free_frames; 
    char tx_irq_disabled; 
    
    spinlock_t bank_lock; 
} mace_private;



#ifdef PCMCIA_DEBUG
static char rcsid[] =
"nmclan_cs.c,v 0.16 1995/07/01 06:42:17 rpao Exp rpao";
static char *version =
DRV_NAME " " DRV_VERSION " (Roger C. Pao)";
#endif

static const char *if_names[]={
    "Auto", "10baseT", "BNC",
};



MODULE_DESCRIPTION("New Media PCMCIA ethernet driver");
MODULE_LICENSE("GPL");

#define INT_MODULE_PARM(n, v) static int n = v; module_param(n, int, 0)


INT_MODULE_PARM(if_port, 0);

#ifdef PCMCIA_DEBUG
INT_MODULE_PARM(pc_debug, PCMCIA_DEBUG);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
#else
#define DEBUG(n, args...)
#endif



static int nmclan_config(struct pcmcia_device *link);
static void nmclan_release(struct pcmcia_device *link);

static void nmclan_reset(struct net_device *dev);
static int mace_config(struct net_device *dev, struct ifmap *map);
static int mace_open(struct net_device *dev);
static int mace_close(struct net_device *dev);
static netdev_tx_t mace_start_xmit(struct sk_buff *skb,
					 struct net_device *dev);
static void mace_tx_timeout(struct net_device *dev);
static irqreturn_t mace_interrupt(int irq, void *dev_id);
static struct net_device_stats *mace_get_stats(struct net_device *dev);
static int mace_rx(struct net_device *dev, unsigned char RxCnt);
static void restore_multicast_list(struct net_device *dev);
static void set_multicast_list(struct net_device *dev);
static const struct ethtool_ops netdev_ethtool_ops;


static void nmclan_detach(struct pcmcia_device *p_dev);

static const struct net_device_ops mace_netdev_ops = {
	.ndo_open		= mace_open,
	.ndo_stop		= mace_close,
	.ndo_start_xmit		= mace_start_xmit,
	.ndo_tx_timeout		= mace_tx_timeout,
	.ndo_set_config		= mace_config,
	.ndo_get_stats		= mace_get_stats,
	.ndo_set_multicast_list	= set_multicast_list,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};



static int nmclan_probe(struct pcmcia_device *link)
{
    mace_private *lp;
    struct net_device *dev;

    DEBUG(0, "nmclan_attach()\n");
    DEBUG(1, "%s\n", rcsid);

    
    dev = alloc_etherdev(sizeof(mace_private));
    if (!dev)
	    return -ENOMEM;
    lp = netdev_priv(dev);
    lp->p_dev = link;
    link->priv = dev;
    
    spin_lock_init(&lp->bank_lock);
    link->io.NumPorts1 = 32;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
    link->io.IOAddrLines = 5;
    link->irq.Attributes = IRQ_TYPE_EXCLUSIVE | IRQ_HANDLE_PRESENT;
    link->irq.IRQInfo1 = IRQ_LEVEL_ID;
    link->irq.Handler = &mace_interrupt;
    link->irq.Instance = dev;
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.ConfigIndex = 1;
    link->conf.Present = PRESENT_OPTION;

    lp->tx_free_frames=AM2150_MAX_TX_FRAMES;

    dev->netdev_ops = &mace_netdev_ops;
    SET_ETHTOOL_OPS(dev, &netdev_ethtool_ops);
    dev->watchdog_timeo = TX_TIMEOUT;

    return nmclan_config(link);
} 



static void nmclan_detach(struct pcmcia_device *link)
{
    struct net_device *dev = link->priv;

    DEBUG(0, "nmclan_detach(0x%p)\n", link);

    if (link->dev_node)
	unregister_netdev(dev);

    nmclan_release(link);

    free_netdev(dev);
} 


static int mace_read(mace_private *lp, unsigned int ioaddr, int reg)
{
  int data = 0xFF;
  unsigned long flags;

  switch (reg >> 4) {
    case 0: 
      data = inb(ioaddr + AM2150_MACE_BASE + reg);
      break;
    case 1: 
      spin_lock_irqsave(&lp->bank_lock, flags);
      MACEBANK(1);
      data = inb(ioaddr + AM2150_MACE_BASE + (reg & 0x0F));
      MACEBANK(0);
      spin_unlock_irqrestore(&lp->bank_lock, flags);
      break;
  }
  return (data & 0xFF);
} 


static void mace_write(mace_private *lp, unsigned int ioaddr, int reg,
		       int data)
{
  unsigned long flags;

  switch (reg >> 4) {
    case 0: 
      outb(data & 0xFF, ioaddr + AM2150_MACE_BASE + reg);
      break;
    case 1: 
      spin_lock_irqsave(&lp->bank_lock, flags);
      MACEBANK(1);
      outb(data & 0xFF, ioaddr + AM2150_MACE_BASE + (reg & 0x0F));
      MACEBANK(0);
      spin_unlock_irqrestore(&lp->bank_lock, flags);
      break;
  }
} 


static int mace_init(mace_private *lp, unsigned int ioaddr, char *enet_addr)
{
  int i;
  int ct = 0;

  
  mace_write(lp, ioaddr, MACE_BIUCC, 1);
  while (mace_read(lp, ioaddr, MACE_BIUCC) & 0x01) {
    ;
    if(++ct > 500)
    {
    	printk(KERN_ERR "mace: reset failed, card removed ?\n");
    	return -1;
    }
    udelay(1);
  }
  mace_write(lp, ioaddr, MACE_BIUCC, 0);

  
  mace_write(lp, ioaddr, MACE_FIFOCC, 0x0F);

  mace_write(lp,ioaddr, MACE_RCVFC, 0); 
  mace_write(lp, ioaddr, MACE_IMR, 0xFF); 

  
  switch (if_port) {
    case 1:
      mace_write(lp, ioaddr, MACE_PLSCC, 0x02);
      break;
    case 2:
      mace_write(lp, ioaddr, MACE_PLSCC, 0x00);
      break;
    default:
      mace_write(lp, ioaddr, MACE_PHYCC,  4);
      
      break;
  }

  mace_write(lp, ioaddr, MACE_IAC, MACE_IAC_ADDRCHG | MACE_IAC_PHYADDR);
  
  ct = 0;
  while (mace_read(lp, ioaddr, MACE_IAC) & MACE_IAC_ADDRCHG)
  {
  	if(++ ct > 500)
  	{
  		printk(KERN_ERR "mace: ADDRCHG timeout, card removed ?\n");
  		return -1;
  	}
  }
  
  for (i = 0; i < ETHER_ADDR_LEN; i++)
    mace_write(lp, ioaddr, MACE_PADR, enet_addr[i]);

  
  
  
  mace_write(lp, ioaddr, MACE_MACCC, 0x00);
  return 0;
} 



#define CS_CHECK(fn, ret) \
  do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int nmclan_config(struct pcmcia_device *link)
{
  struct net_device *dev = link->priv;
  mace_private *lp = netdev_priv(dev);
  tuple_t tuple;
  u_char buf[64];
  int i, last_ret, last_fn;
  unsigned int ioaddr;

  DEBUG(0, "nmclan_config(0x%p)\n", link);

  CS_CHECK(RequestIO, pcmcia_request_io(link, &link->io));
  CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));
  CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));
  dev->irq = link->irq.AssignedIRQ;
  dev->base_addr = link->io.BasePort1;

  ioaddr = dev->base_addr;

  
  tuple.DesiredTuple = 0x80 ;
  tuple.TupleData = buf;
  tuple.TupleDataMax = 64;
  tuple.TupleOffset = 0;
  tuple.Attributes = 0;
  CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link, &tuple));
  CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link, &tuple));
  memcpy(dev->dev_addr, tuple.TupleData, ETHER_ADDR_LEN);

  
  {
    char sig[2];

    sig[0] = mace_read(lp, ioaddr, MACE_CHIPIDL);
    sig[1] = mace_read(lp, ioaddr, MACE_CHIPIDH);
    if ((sig[0] == 0x40) && ((sig[1] & 0x0F) == 0x09)) {
      DEBUG(0, "nmclan_cs configured: mace id=%x %x\n",
	    sig[0], sig[1]);
    } else {
      printk(KERN_NOTICE "nmclan_cs: mace id not found: %x %x should"
	     " be 0x40 0x?9\n", sig[0], sig[1]);
      return -ENODEV;
    }
  }

  if(mace_init(lp, ioaddr, dev->dev_addr) == -1)
  	goto failed;

  
  if (if_port <= 2)
    dev->if_port = if_port;
  else
    printk(KERN_NOTICE "nmclan_cs: invalid if_port requested\n");

  link->dev_node = &lp->node;
  SET_NETDEV_DEV(dev, &handle_to_dev(link));

  i = register_netdev(dev);
  if (i != 0) {
    printk(KERN_NOTICE "nmclan_cs: register_netdev() failed\n");
    link->dev_node = NULL;
    goto failed;
  }

  strcpy(lp->node.dev_name, dev->name);

  printk(KERN_INFO "%s: nmclan: port %#3lx, irq %d, %s port,"
	 " hw_addr %pM\n",
	 dev->name, dev->base_addr, dev->irq, if_names[dev->if_port],
	 dev->dev_addr);
  return 0;

cs_failed:
	cs_error(link, last_fn, last_ret);
failed:
	nmclan_release(link);
	return -ENODEV;
} 


static void nmclan_release(struct pcmcia_device *link)
{
	DEBUG(0, "nmclan_release(0x%p)\n", link);
	pcmcia_disable_device(link);
}

static int nmclan_suspend(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open)
		netif_device_detach(dev);

	return 0;
}

static int nmclan_resume(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open) {
		nmclan_reset(dev);
		netif_device_attach(dev);
	}

	return 0;
}



static void nmclan_reset(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);

#if RESET_XILINX
  struct pcmcia_device *link = &lp->link;
  conf_reg_t reg;
  u_long OrigCorValue; 

  
  reg.Function = 0;
  reg.Action = CS_READ;
  reg.Offset = CISREG_COR;
  reg.Value = 0;
  pcmcia_access_configuration_register(link, &reg);
  OrigCorValue = reg.Value;

  
  reg.Action = CS_WRITE;
  reg.Offset = CISREG_COR;
  DEBUG(1, "nmclan_reset: OrigCorValue=0x%lX, resetting...\n",
	OrigCorValue);
  reg.Value = COR_SOFT_RESET;
  pcmcia_access_configuration_register(link, &reg);
  

  
  reg.Value = COR_LEVEL_REQ | (OrigCorValue & COR_CONFIG_MASK);
  pcmcia_access_configuration_register(link, &reg);
  
  lp->tx_free_frames=AM2150_MAX_TX_FRAMES;

#endif 

  
  lp->tx_free_frames=AM2150_MAX_TX_FRAMES;

  
  mace_init(lp, dev->base_addr, dev->dev_addr);
  mace_write(lp, dev->base_addr, MACE_IMR, MACE_IMR_DEFAULT);

  
  restore_multicast_list(dev);
} 


static int mace_config(struct net_device *dev, struct ifmap *map)
{
  if ((map->port != (u_char)(-1)) && (map->port != dev->if_port)) {
    if (map->port <= 2) {
      dev->if_port = map->port;
      printk(KERN_INFO "%s: switched to %s port\n", dev->name,
	     if_names[dev->if_port]);
    } else
      return -EINVAL;
  }
  return 0;
} 


static int mace_open(struct net_device *dev)
{
  unsigned int ioaddr = dev->base_addr;
  mace_private *lp = netdev_priv(dev);
  struct pcmcia_device *link = lp->p_dev;

  if (!pcmcia_dev_present(link))
    return -ENODEV;

  link->open++;

  MACEBANK(0);

  netif_start_queue(dev);
  nmclan_reset(dev);

  return 0; 
} 


static int mace_close(struct net_device *dev)
{
  unsigned int ioaddr = dev->base_addr;
  mace_private *lp = netdev_priv(dev);
  struct pcmcia_device *link = lp->p_dev;

  DEBUG(2, "%s: shutting down ethercard.\n", dev->name);

  
  outb(0xFF, ioaddr + AM2150_MACE_BASE + MACE_IMR);

  link->open--;
  netif_stop_queue(dev);

  return 0;
} 

static void netdev_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	sprintf(info->bus_info, "PCMCIA 0x%lx", dev->base_addr);
}

#ifdef PCMCIA_DEBUG
static u32 netdev_get_msglevel(struct net_device *dev)
{
	return pc_debug;
}

static void netdev_set_msglevel(struct net_device *dev, u32 level)
{
	pc_debug = level;
}
#endif 

static const struct ethtool_ops netdev_ethtool_ops = {
	.get_drvinfo		= netdev_get_drvinfo,
#ifdef PCMCIA_DEBUG
	.get_msglevel		= netdev_get_msglevel,
	.set_msglevel		= netdev_set_msglevel,
#endif 
};



static void mace_tx_timeout(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);
  struct pcmcia_device *link = lp->p_dev;

  printk(KERN_NOTICE "%s: transmit timed out -- ", dev->name);
#if RESET_ON_TIMEOUT
  printk("resetting card\n");
  pcmcia_reset_card(link->socket);
#else 
  printk("NOT resetting card\n");
#endif 
  dev->trans_start = jiffies;
  netif_wake_queue(dev);
}

static netdev_tx_t mace_start_xmit(struct sk_buff *skb,
					 struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);
  unsigned int ioaddr = dev->base_addr;

  netif_stop_queue(dev);

  DEBUG(3, "%s: mace_start_xmit(length = %ld) called.\n",
	dev->name, (long)skb->len);

#if (!TX_INTERRUPTABLE)
  
  outb(MACE_IMR_DEFAULT | MACE_IR_XMTINT,
    ioaddr + AM2150_MACE_BASE + MACE_IMR);
  lp->tx_irq_disabled=1;
#endif 

  {
    

    lp->linux_stats.tx_bytes += skb->len;
    lp->tx_free_frames--;

    
    
    outw(skb->len, ioaddr + AM2150_XMT);
    
    outsw(ioaddr + AM2150_XMT, skb->data, skb->len >> 1);
    if (skb->len & 1) {
      
      outb(skb->data[skb->len-1], ioaddr + AM2150_XMT);
    }

    dev->trans_start = jiffies;

#if MULTI_TX
    if (lp->tx_free_frames > 0)
      netif_start_queue(dev);
#endif 
  }

#if (!TX_INTERRUPTABLE)
  
  lp->tx_irq_disabled=0;
  outb(MACE_IMR_DEFAULT, ioaddr + AM2150_MACE_BASE + MACE_IMR);
#endif 

  dev_kfree_skb(skb);

  return NETDEV_TX_OK;
} 


static irqreturn_t mace_interrupt(int irq, void *dev_id)
{
  struct net_device *dev = (struct net_device *) dev_id;
  mace_private *lp = netdev_priv(dev);
  unsigned int ioaddr;
  int status;
  int IntrCnt = MACE_MAX_IR_ITERATIONS;

  if (dev == NULL) {
    DEBUG(2, "mace_interrupt(): irq 0x%X for unknown device.\n",
	  irq);
    return IRQ_NONE;
  }

  ioaddr = dev->base_addr;

  if (lp->tx_irq_disabled) {
    printk(
      (lp->tx_irq_disabled?
       KERN_NOTICE "%s: Interrupt with tx_irq_disabled "
       "[isr=%02X, imr=%02X]\n": 
       KERN_NOTICE "%s: Re-entering the interrupt handler "
       "[isr=%02X, imr=%02X]\n"),
      dev->name,
      inb(ioaddr + AM2150_MACE_BASE + MACE_IR),
      inb(ioaddr + AM2150_MACE_BASE + MACE_IMR)
    );
    
    return IRQ_NONE;
  }

  if (!netif_device_present(dev)) {
    DEBUG(2, "%s: interrupt from dead card\n", dev->name);
    return IRQ_NONE;
  }

  do {
    
    status = inb(ioaddr + AM2150_MACE_BASE + MACE_IR);

    DEBUG(3, "mace_interrupt: irq 0x%X status 0x%X.\n", irq, status);

    if (status & MACE_IR_RCVINT) {
      mace_rx(dev, MACE_MAX_RX_ITERATIONS);
    }

    if (status & MACE_IR_XMTINT) {
      unsigned char fifofc;
      unsigned char xmtrc;
      unsigned char xmtfs;

      fifofc = inb(ioaddr + AM2150_MACE_BASE + MACE_FIFOFC);
      if ((fifofc & MACE_FIFOFC_XMTFC)==0) {
	lp->linux_stats.tx_errors++;
	outb(0xFF, ioaddr + AM2150_XMT_SKIP);
      }

      
      xmtrc = inb(ioaddr + AM2150_MACE_BASE + MACE_XMTRC);
      if (xmtrc & MACE_XMTRC_EXDEF) lp->mace_stats.exdef++;
      lp->mace_stats.xmtrc += (xmtrc & MACE_XMTRC_XMTRC);

      if (
        (xmtfs = inb(ioaddr + AM2150_MACE_BASE + MACE_XMTFS)) &
        MACE_XMTFS_XMTSV 
      ) {
	lp->mace_stats.xmtsv++;

	if (xmtfs & ~MACE_XMTFS_XMTSV) {
	  if (xmtfs & MACE_XMTFS_UFLO) {
	    
	    lp->mace_stats.uflo++;
	  }
	  if (xmtfs & MACE_XMTFS_LCOL) {
	    
	    lp->mace_stats.lcol++;
	  }
	  if (xmtfs & MACE_XMTFS_MORE) {
	    
	    lp->mace_stats.more++;
	  }
	  if (xmtfs & MACE_XMTFS_ONE) {
	    
	    lp->mace_stats.one++;
	  }
	  if (xmtfs & MACE_XMTFS_DEFER) {
	    
	    lp->mace_stats.defer++;
	  }
	  if (xmtfs & MACE_XMTFS_LCAR) {
	    
	    lp->mace_stats.lcar++;
	  }
	  if (xmtfs & MACE_XMTFS_RTRY) {
	    
	    lp->mace_stats.rtry++;
	  }
        } 

      } 

      lp->linux_stats.tx_packets++;
      lp->tx_free_frames++;
      netif_wake_queue(dev);
    } 

    if (status & ~MACE_IMR_DEFAULT & ~MACE_IR_RCVINT & ~MACE_IR_XMTINT) {
      if (status & MACE_IR_JAB) {
        
        lp->mace_stats.jab++;
      }
      if (status & MACE_IR_BABL) {
        
        lp->mace_stats.babl++;
      }
      if (status & MACE_IR_CERR) {
	
        lp->mace_stats.cerr++;
      }
      if (status & MACE_IR_RCVCCO) {
        
        lp->mace_stats.rcvcco++;
      }
      if (status & MACE_IR_RNTPCO) {
        
        lp->mace_stats.rntpco++;
      }
      if (status & MACE_IR_MPCO) {
        
        lp->mace_stats.mpco++;
      }
    } 

  } while ((status & ~MACE_IMR_DEFAULT) && (--IntrCnt));

  return IRQ_HANDLED;
} 


static int mace_rx(struct net_device *dev, unsigned char RxCnt)
{
  mace_private *lp = netdev_priv(dev);
  unsigned int ioaddr = dev->base_addr;
  unsigned char rx_framecnt;
  unsigned short rx_status;

  while (
    ((rx_framecnt = inb(ioaddr + AM2150_RCV_FRAME_COUNT)) > 0) &&
    (rx_framecnt <= 12) && 
    (RxCnt--)
  ) {
    rx_status = inw(ioaddr + AM2150_RCV);

    DEBUG(3, "%s: in mace_rx(), framecnt 0x%X, rx_status"
	  " 0x%X.\n", dev->name, rx_framecnt, rx_status);

    if (rx_status & MACE_RCVFS_RCVSTS) { 
      lp->linux_stats.rx_errors++;
      if (rx_status & MACE_RCVFS_OFLO) {
        lp->mace_stats.oflo++;
      }
      if (rx_status & MACE_RCVFS_CLSN) {
        lp->mace_stats.clsn++;
      }
      if (rx_status & MACE_RCVFS_FRAM) {
	lp->mace_stats.fram++;
      }
      if (rx_status & MACE_RCVFS_FCS) {
        lp->mace_stats.fcs++;
      }
    } else {
      short pkt_len = (rx_status & ~MACE_RCVFS_RCVSTS) - 4;
        
      struct sk_buff *skb;

      lp->mace_stats.rfs_rntpc += inb(ioaddr + AM2150_RCV);
        
      lp->mace_stats.rfs_rcvcc += inb(ioaddr + AM2150_RCV);
        

      DEBUG(3, "    receiving packet size 0x%X rx_status"
	    " 0x%X.\n", pkt_len, rx_status);

      skb = dev_alloc_skb(pkt_len+2);

      if (skb != NULL) {
	skb_reserve(skb, 2);
	insw(ioaddr + AM2150_RCV, skb_put(skb, pkt_len), pkt_len>>1);
	if (pkt_len & 1)
	    *(skb_tail_pointer(skb) - 1) = inb(ioaddr + AM2150_RCV);
	skb->protocol = eth_type_trans(skb, dev);
	
	netif_rx(skb); 

	lp->linux_stats.rx_packets++;
	lp->linux_stats.rx_bytes += pkt_len;
	outb(0xFF, ioaddr + AM2150_RCV_NEXT); 
	continue;
      } else {
	DEBUG(1, "%s: couldn't allocate a sk_buff of size"
	      " %d.\n", dev->name, pkt_len);
	lp->linux_stats.rx_dropped++;
      }
    }
    outb(0xFF, ioaddr + AM2150_RCV_NEXT); 
  } 

  return 0;
} 


static void pr_linux_stats(struct net_device_stats *pstats)
{
  DEBUG(2, "pr_linux_stats\n");
  DEBUG(2, " rx_packets=%-7ld        tx_packets=%ld\n",
	(long)pstats->rx_packets, (long)pstats->tx_packets);
  DEBUG(2, " rx_errors=%-7ld         tx_errors=%ld\n",
	(long)pstats->rx_errors, (long)pstats->tx_errors);
  DEBUG(2, " rx_dropped=%-7ld        tx_dropped=%ld\n",
	(long)pstats->rx_dropped, (long)pstats->tx_dropped);
  DEBUG(2, " multicast=%-7ld         collisions=%ld\n",
	(long)pstats->multicast, (long)pstats->collisions);

  DEBUG(2, " rx_length_errors=%-7ld  rx_over_errors=%ld\n",
	(long)pstats->rx_length_errors, (long)pstats->rx_over_errors);
  DEBUG(2, " rx_crc_errors=%-7ld     rx_frame_errors=%ld\n",
	(long)pstats->rx_crc_errors, (long)pstats->rx_frame_errors);
  DEBUG(2, " rx_fifo_errors=%-7ld    rx_missed_errors=%ld\n",
	(long)pstats->rx_fifo_errors, (long)pstats->rx_missed_errors);

  DEBUG(2, " tx_aborted_errors=%-7ld tx_carrier_errors=%ld\n",
	(long)pstats->tx_aborted_errors, (long)pstats->tx_carrier_errors);
  DEBUG(2, " tx_fifo_errors=%-7ld    tx_heartbeat_errors=%ld\n",
	(long)pstats->tx_fifo_errors, (long)pstats->tx_heartbeat_errors);
  DEBUG(2, " tx_window_errors=%ld\n",
	(long)pstats->tx_window_errors);
} 


static void pr_mace_stats(mace_statistics *pstats)
{
  DEBUG(2, "pr_mace_stats\n");

  DEBUG(2, " xmtsv=%-7d             uflo=%d\n",
	pstats->xmtsv, pstats->uflo);
  DEBUG(2, " lcol=%-7d              more=%d\n",
	pstats->lcol, pstats->more);
  DEBUG(2, " one=%-7d               defer=%d\n",
	pstats->one, pstats->defer);
  DEBUG(2, " lcar=%-7d              rtry=%d\n",
	pstats->lcar, pstats->rtry);

  
  DEBUG(2, " exdef=%-7d             xmtrc=%d\n",
	pstats->exdef, pstats->xmtrc);

  
  DEBUG(2, " oflo=%-7d              clsn=%d\n",
	pstats->oflo, pstats->clsn);
  DEBUG(2, " fram=%-7d              fcs=%d\n",
	pstats->fram, pstats->fcs);

  
  
  DEBUG(2, " rfs_rntpc=%-7d         rfs_rcvcc=%d\n",
	pstats->rfs_rntpc, pstats->rfs_rcvcc);

  
  DEBUG(2, " jab=%-7d               babl=%d\n",
	pstats->jab, pstats->babl);
  DEBUG(2, " cerr=%-7d              rcvcco=%d\n",
	pstats->cerr, pstats->rcvcco);
  DEBUG(2, " rntpco=%-7d            mpco=%d\n",
	pstats->rntpco, pstats->mpco);

  
  DEBUG(2, " mpc=%d\n", pstats->mpc);

  
  DEBUG(2, " rntpc=%d\n", pstats->rntpc);

  
  DEBUG(2, " rcvcc=%d\n", pstats->rcvcc);

} 


static void update_stats(unsigned int ioaddr, struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);

  lp->mace_stats.rcvcc += mace_read(lp, ioaddr, MACE_RCVCC);
  lp->mace_stats.rntpc += mace_read(lp, ioaddr, MACE_RNTPC);
  lp->mace_stats.mpc += mace_read(lp, ioaddr, MACE_MPC);
  

  

  
  lp->linux_stats.collisions = 
    lp->mace_stats.rcvcco * 256 + lp->mace_stats.rcvcc;
    

  
  lp->linux_stats.rx_length_errors = 
    lp->mace_stats.rntpco * 256 + lp->mace_stats.rntpc;
  
  lp->linux_stats.rx_crc_errors = lp->mace_stats.fcs;
  lp->linux_stats.rx_frame_errors = lp->mace_stats.fram;
  lp->linux_stats.rx_fifo_errors = lp->mace_stats.oflo;
  lp->linux_stats.rx_missed_errors = 
    lp->mace_stats.mpco * 256 + lp->mace_stats.mpc;

  
  lp->linux_stats.tx_aborted_errors = lp->mace_stats.rtry;
  lp->linux_stats.tx_carrier_errors = lp->mace_stats.lcar;
    
  lp->linux_stats.tx_fifo_errors = lp->mace_stats.uflo;
  lp->linux_stats.tx_heartbeat_errors = lp->mace_stats.cerr;
  

  return;
} 


static struct net_device_stats *mace_get_stats(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);

  update_stats(dev->base_addr, dev);

  DEBUG(1, "%s: updating the statistics.\n", dev->name);
  pr_linux_stats(&lp->linux_stats);
  pr_mace_stats(&lp->mace_stats);

  return &lp->linux_stats;
} 



#ifdef BROKEN_MULTICAST

static void updateCRC(int *CRC, int bit)
{
  int poly[]={
    1,1,1,0, 1,1,0,1,
    1,0,1,1, 1,0,0,0,
    1,0,0,0, 0,0,1,1,
    0,0,1,0, 0,0,0,0
  }; 

  int j;

  
  for (j = 32; j > 0; j--)
    CRC[j] = CRC[j-1];
  CRC[0] = 0;

  
  if (bit ^ CRC[32])
    for (j = 0; j < 32; j++)
      CRC[j] ^= poly[j];
} 


static void BuildLAF(int *ladrf, int *adr)
{
  int CRC[33]={1}; 

  int i, byte; 
  int hashcode; 

  CRC[32]=0;

  for (byte = 0; byte < 6; byte++)
    for (i = 0; i < 8; i++)
      updateCRC(CRC, (adr[byte] >> i) & 1);

  hashcode = 0;
  for (i = 0; i < 6; i++)
    hashcode = (hashcode << 1) + CRC[i];

  byte = hashcode >> 3;
  ladrf[byte] |= (1 << (hashcode & 7));

#ifdef PCMCIA_DEBUG
  if (pc_debug > 2)
    printk(KERN_DEBUG "    adr =%pM\n", adr);
  printk(KERN_DEBUG "    hashcode = %d(decimal), ladrf[0:63] =", hashcode);
  for (i = 0; i < 8; i++)
    printk(KERN_CONT " %02X", ladrf[i]);
  printk(KERN_CONT "\n");
  }
#endif
} 


static void restore_multicast_list(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);
  int num_addrs = lp->multicast_num_addrs;
  int *ladrf = lp->multicast_ladrf;
  unsigned int ioaddr = dev->base_addr;
  int i;

  DEBUG(2, "%s: restoring Rx mode to %d addresses.\n",
	dev->name, num_addrs);

  if (num_addrs > 0) {

    DEBUG(1, "Attempt to restore multicast list detected.\n");

    mace_write(lp, ioaddr, MACE_IAC, MACE_IAC_ADDRCHG | MACE_IAC_LOGADDR);
    
    while (mace_read(lp, ioaddr, MACE_IAC) & MACE_IAC_ADDRCHG)
      ;
    
    for (i = 0; i < MACE_LADRF_LEN; i++)
      mace_write(lp, ioaddr, MACE_LADRF, ladrf[i]);

    mace_write(lp, ioaddr, MACE_UTR, MACE_UTR_RCVFCSE | MACE_UTR_LOOP_EXTERNAL);
    mace_write(lp, ioaddr, MACE_MACCC, MACE_MACCC_ENXMT | MACE_MACCC_ENRCV);

  } else if (num_addrs < 0) {

    
    mace_write(lp, ioaddr, MACE_UTR, MACE_UTR_LOOP_EXTERNAL);
    mace_write(lp, ioaddr, MACE_MACCC,
      MACE_MACCC_PROM | MACE_MACCC_ENXMT | MACE_MACCC_ENRCV
    );

  } else {

    
    mace_write(lp, ioaddr, MACE_UTR, MACE_UTR_LOOP_EXTERNAL);
    mace_write(lp, ioaddr, MACE_MACCC, MACE_MACCC_ENXMT | MACE_MACCC_ENRCV);

  }
} 



static void set_multicast_list(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);
  int adr[ETHER_ADDR_LEN] = {0}; 
  int i;
  struct dev_mc_list *dmi = dev->mc_list;

#ifdef PCMCIA_DEBUG
  if (pc_debug > 1) {
    static int old;
    if (dev->mc_count != old) {
      old = dev->mc_count;
      DEBUG(0, "%s: setting Rx mode to %d addresses.\n",
	    dev->name, old);
    }
  }
#endif

  
  lp->multicast_num_addrs = dev->mc_count;

  
  if (num_addrs > 0) {
    
    memset(lp->multicast_ladrf, 0, MACE_LADRF_LEN);
    for (i = 0; i < dev->mc_count; i++) {
      memcpy(adr, dmi->dmi_addr, ETHER_ADDR_LEN);
      dmi = dmi->next;
      BuildLAF(lp->multicast_ladrf, adr);
    }
  }

  restore_multicast_list(dev);

} 

#endif 

static void restore_multicast_list(struct net_device *dev)
{
  unsigned int ioaddr = dev->base_addr;
  mace_private *lp = netdev_priv(dev);

  DEBUG(2, "%s: restoring Rx mode to %d addresses.\n", dev->name,
	lp->multicast_num_addrs);

  if (dev->flags & IFF_PROMISC) {
    
    mace_write(lp,ioaddr, MACE_UTR, MACE_UTR_LOOP_EXTERNAL);
    mace_write(lp, ioaddr, MACE_MACCC,
      MACE_MACCC_PROM | MACE_MACCC_ENXMT | MACE_MACCC_ENRCV
    );
  } else {
    
    mace_write(lp, ioaddr, MACE_UTR, MACE_UTR_LOOP_EXTERNAL);
    mace_write(lp, ioaddr, MACE_MACCC, MACE_MACCC_ENXMT | MACE_MACCC_ENRCV);
  }
} 

static void set_multicast_list(struct net_device *dev)
{
  mace_private *lp = netdev_priv(dev);

#ifdef PCMCIA_DEBUG
  if (pc_debug > 1) {
    static int old;
    if (dev->mc_count != old) {
      old = dev->mc_count;
      DEBUG(0, "%s: setting Rx mode to %d addresses.\n",
	    dev->name, old);
    }
  }
#endif

  lp->multicast_num_addrs = dev->mc_count;
  restore_multicast_list(dev);

} 

static struct pcmcia_device_id nmclan_ids[] = {
	PCMCIA_DEVICE_PROD_ID12("New Media Corporation", "Ethernet", 0x085a850b, 0x00b2e941),
	PCMCIA_DEVICE_PROD_ID12("Portable Add-ons", "Ethernet+", 0xebf1d60, 0xad673aaf),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, nmclan_ids);

static struct pcmcia_driver nmclan_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "nmclan_cs",
	},
	.probe		= nmclan_probe,
	.remove		= nmclan_detach,
	.id_table       = nmclan_ids,
	.suspend	= nmclan_suspend,
	.resume		= nmclan_resume,
};

static int __init init_nmclan_cs(void)
{
	return pcmcia_register_driver(&nmclan_cs_driver);
}

static void __exit exit_nmclan_cs(void)
{
	pcmcia_unregister_driver(&nmclan_cs_driver);
}

module_init(init_nmclan_cs);
module_exit(exit_nmclan_cs);
