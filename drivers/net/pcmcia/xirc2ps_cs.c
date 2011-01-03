

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
#include <linux/mii.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ciscode.h>

#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#ifndef MANFID_COMPAQ
  #define MANFID_COMPAQ 	   0x0138
  #define MANFID_COMPAQ2	   0x0183  
#endif

#include <pcmcia/ds.h>


#define TX_TIMEOUT	((400*HZ)/1000)




#define XIRCREG_CR  0	
enum xirc_cr {
    TransmitPacket = 0x01,
    SoftReset = 0x02,
    EnableIntr = 0x04,
    ForceIntr  = 0x08,
    ClearTxFIFO = 0x10,
    ClearRxOvrun = 0x20,
    RestartTx	 = 0x40
};
#define XIRCREG_ESR 0	
enum xirc_esr {
    FullPktRcvd = 0x01, 
    PktRejected = 0x04, 
    TxPktPend = 0x08,	
    IncorPolarity = 0x10,
    MediaSelect = 0x20	
};
#define XIRCREG_PR  1	
#define XIRCREG_EDP 4	
#define XIRCREG_ISR 6	
enum xirc_isr {
    TxBufOvr = 0x01,	
    PktTxed  = 0x02,	
    MACIntr  = 0x04,	
    TxResGrant = 0x08,	
    RxFullPkt = 0x20,	
    RxPktRej  = 0x40,	
    ForcedIntr= 0x80	
};
#define XIRCREG1_IMR0 12 
#define XIRCREG1_IMR1 13
#define XIRCREG0_TSO  8  
#define XIRCREG0_TRS  10 
#define XIRCREG0_DO   12 
#define XIRCREG0_RSR  12 
enum xirc_rsr {
    PhyPkt = 0x01,	
    BrdcstPkt = 0x02,	
    PktTooLong = 0x04,	
    AlignErr = 0x10,	
    CRCErr = 0x20,	
    PktRxOk = 0x80	
};
#define XIRCREG0_PTR 13 
#define XIRCREG0_RBC 14 
#define XIRCREG1_ECR 14 
enum xirc_ecr {
    FullDuplex = 0x04,	
    LongTPMode = 0x08,	
    DisablePolCor = 0x10,
    DisableLinkPulse = 0x20, 
    DisableAutoTx = 0x40, 
};
#define XIRCREG2_RBS 8	
#define XIRCREG2_LED 10 

#define XIRCREG2_MSR 12 

#define XIRCREG4_GPR0 8 
#define XIRCREG4_GPR1 9 
#define XIRCREG2_GPR2 13 
#define XIRCREG4_BOV 10 
#define XIRCREG4_LMA 12 
#define XIRCREG4_LMD 14 

#define XIRCREG40_CMD0 8    
enum xirc_cmd { 	    
    Transmit = 0x01,
    EnableRecv = 0x04,
    DisableRecv = 0x08,
    Abort = 0x10,
    Online = 0x20,
    IntrAck = 0x40,
    Offline = 0x80
};
#define XIRCREG5_RHSA0	10  
#define XIRCREG40_RXST0 9   
#define XIRCREG40_TXST0 11  
#define XIRCREG40_TXST1 12  
#define XIRCREG40_RMASK0 13  
#define XIRCREG40_TMASK0 14  
#define XIRCREG40_TMASK1 15  
#define XIRCREG42_SWC0	8   
#define XIRCREG42_SWC1	9   
#define XIRCREG42_BOC	10  
#define XIRCREG44_TDR0	8   
#define XIRCREG44_TDR1	9   
#define XIRCREG44_RXBC_LO 10 
#define XIRCREG44_RXBC_HI 11 
#define XIRCREG45_REV	 15 
#define XIRCREG50_IA	8   

static const char *if_names[] = { "Auto", "10BaseT", "10Base2", "AUI", "100BaseT" };


#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KDBG_XIRC args)
#else
#define DEBUG(n, args...)
#endif

#define KDBG_XIRC KERN_DEBUG   "xirc2ps_cs: "
#define KERR_XIRC KERN_ERR     "xirc2ps_cs: "
#define KWRN_XIRC KERN_WARNING "xirc2ps_cs: "
#define KNOT_XIRC KERN_NOTICE  "xirc2ps_cs: "
#define KINF_XIRC KERN_INFO    "xirc2ps_cs: "


#define XIR_UNKNOWN  0	
#define XIR_CE	     1	
#define XIR_CE2      2	
#define XIR_CE3      3	
#define XIR_CEM      4	
#define XIR_CEM2     5	
#define XIR_CEM3     6	
#define XIR_CEM33    7	
#define XIR_CEM56M   8	
#define XIR_CEM56    9	
#define XIR_CM28    10	
#define XIR_CM33    11	
#define XIR_CM56    12	
#define XIR_CG	    13	
#define XIR_CBE     14	




MODULE_DESCRIPTION("Xircom PCMCIA ethernet driver");
MODULE_LICENSE("Dual MPL/GPL");

#define INT_MODULE_PARM(n, v) static int n = v; module_param(n, int, 0)

INT_MODULE_PARM(if_port,	0);
INT_MODULE_PARM(full_duplex,	0);
INT_MODULE_PARM(do_sound, 	1);
INT_MODULE_PARM(lockup_hack,	0);  




static unsigned maxrx_bytes = 22000;


static void mii_idle(unsigned int ioaddr);
static void mii_putbit(unsigned int ioaddr, unsigned data);
static int  mii_getbit(unsigned int ioaddr);
static void mii_wbits(unsigned int ioaddr, unsigned data, int len);
static unsigned mii_rd(unsigned int ioaddr, u_char phyaddr, u_char phyreg);
static void mii_wr(unsigned int ioaddr, u_char phyaddr, u_char phyreg,
		   unsigned data, int len);



static int has_ce2_string(struct pcmcia_device * link);
static int xirc2ps_config(struct pcmcia_device * link);
static void xirc2ps_release(struct pcmcia_device * link);



static void xirc2ps_detach(struct pcmcia_device *p_dev);



static irqreturn_t xirc2ps_interrupt(int irq, void *dev_id);





typedef struct local_info_t {
	struct net_device	*dev;
	struct pcmcia_device	*p_dev;
    dev_node_t node;

    int card_type;
    int probe_port;
    int silicon; 
    int mohawk;  
    int dingo;	 
    int new_mii; 
    int modem;	 
    void __iomem *dingo_ccr; 
    unsigned last_ptr_value; 
    const char *manf_str;
    struct work_struct tx_timeout_task;
} local_info_t;


static netdev_tx_t do_start_xmit(struct sk_buff *skb,
				       struct net_device *dev);
static void xirc_tx_timeout(struct net_device *dev);
static void xirc2ps_tx_timeout_task(struct work_struct *work);
static void set_addresses(struct net_device *dev);
static void set_multicast_list(struct net_device *dev);
static int set_card_type(struct pcmcia_device *link, const void *s);
static int do_config(struct net_device *dev, struct ifmap *map);
static int do_open(struct net_device *dev);
static int do_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static const struct ethtool_ops netdev_ethtool_ops;
static void hardreset(struct net_device *dev);
static void do_reset(struct net_device *dev, int full);
static int init_mii(struct net_device *dev);
static void do_powerdown(struct net_device *dev);
static int do_stop(struct net_device *dev);


static int
first_tuple(struct pcmcia_device *handle, tuple_t *tuple, cisparse_t *parse)
{
	int err;

	if ((err = pcmcia_get_first_tuple(handle, tuple)) == 0 &&
			(err = pcmcia_get_tuple_data(handle, tuple)) == 0)
		err = pcmcia_parse_tuple(tuple, parse);
	return err;
}

static int
next_tuple(struct pcmcia_device *handle, tuple_t *tuple, cisparse_t *parse)
{
	int err;

	if ((err = pcmcia_get_next_tuple(handle, tuple)) == 0 &&
			(err = pcmcia_get_tuple_data(handle, tuple)) == 0)
		err = pcmcia_parse_tuple(tuple, parse);
	return err;
}

#define SelectPage(pgnr)   outb((pgnr), ioaddr + XIRCREG_PR)
#define GetByte(reg)	   ((unsigned)inb(ioaddr + (reg)))
#define GetWord(reg)	   ((unsigned)inw(ioaddr + (reg)))
#define PutByte(reg,value) outb((value), ioaddr+(reg))
#define PutWord(reg,value) outw((value), ioaddr+(reg))


#if defined(PCMCIA_DEBUG) && 0 
static void
PrintRegisters(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;

    if (pc_debug > 1) {
	int i, page;

	printk(KDBG_XIRC "Register  common: ");
	for (i = 0; i < 8; i++)
	    printk(" %2.2x", GetByte(i));
	printk("\n");
	for (page = 0; page <= 8; page++) {
	    printk(KDBG_XIRC "Register page %2x: ", page);
	    SelectPage(page);
	    for (i = 8; i < 16; i++)
		printk(" %2.2x", GetByte(i));
	    printk("\n");
	}
	for (page=0x40 ; page <= 0x5f; page++) {
	    if (page == 0x43 || (page >= 0x46 && page <= 0x4f)
		|| (page >= 0x51 && page <=0x5e))
		continue;
	    printk(KDBG_XIRC "Register page %2x: ", page);
	    SelectPage(page);
	    for (i = 8; i < 16; i++)
		printk(" %2.2x", GetByte(i));
	    printk("\n");
	}
    }
}
#endif 




static void
mii_idle(unsigned int ioaddr)
{
    PutByte(XIRCREG2_GPR2, 0x04|0); 
    udelay(1);
    PutByte(XIRCREG2_GPR2, 0x04|1); 
    udelay(1);
}


static void
mii_putbit(unsigned int ioaddr, unsigned data)
{
  #if 1
    if (data) {
	PutByte(XIRCREG2_GPR2, 0x0c|2|0); 
	udelay(1);
	PutByte(XIRCREG2_GPR2, 0x0c|2|1); 
	udelay(1);
    } else {
	PutByte(XIRCREG2_GPR2, 0x0c|0|0); 
	udelay(1);
	PutByte(XIRCREG2_GPR2, 0x0c|0|1); 
	udelay(1);
    }
  #else
    if (data) {
	PutWord(XIRCREG2_GPR2-1, 0x0e0e);
	udelay(1);
	PutWord(XIRCREG2_GPR2-1, 0x0f0f);
	udelay(1);
    } else {
	PutWord(XIRCREG2_GPR2-1, 0x0c0c);
	udelay(1);
	PutWord(XIRCREG2_GPR2-1, 0x0d0d);
	udelay(1);
    }
  #endif
}


static int
mii_getbit(unsigned int ioaddr)
{
    unsigned d;

    PutByte(XIRCREG2_GPR2, 4|0); 
    udelay(1);
    d = GetByte(XIRCREG2_GPR2); 
    PutByte(XIRCREG2_GPR2, 4|1); 
    udelay(1);
    return d & 0x20; 
}

static void
mii_wbits(unsigned int ioaddr, unsigned data, int len)
{
    unsigned m = 1 << (len-1);
    for (; m; m >>= 1)
	mii_putbit(ioaddr, data & m);
}

static unsigned
mii_rd(unsigned int ioaddr,	u_char phyaddr, u_char phyreg)
{
    int i;
    unsigned data=0, m;

    SelectPage(2);
    for (i=0; i < 32; i++)		
	mii_putbit(ioaddr, 1);
    mii_wbits(ioaddr, 0x06, 4); 	
    mii_wbits(ioaddr, phyaddr, 5);	
    mii_wbits(ioaddr, phyreg, 5);	
    mii_idle(ioaddr);			
    mii_getbit(ioaddr);

    for (m = 1<<15; m; m >>= 1)
	if (mii_getbit(ioaddr))
	    data |= m;
    mii_idle(ioaddr);
    return data;
}

static void
mii_wr(unsigned int ioaddr, u_char phyaddr, u_char phyreg, unsigned data,
       int len)
{
    int i;

    SelectPage(2);
    for (i=0; i < 32; i++)		
	mii_putbit(ioaddr, 1);
    mii_wbits(ioaddr, 0x05, 4); 	
    mii_wbits(ioaddr, phyaddr, 5);	
    mii_wbits(ioaddr, phyreg, 5);	
    mii_putbit(ioaddr, 1);		
    mii_putbit(ioaddr, 0);
    mii_wbits(ioaddr, data, len);	
    mii_idle(ioaddr);
}



static const struct net_device_ops netdev_ops = {
	.ndo_open		= do_open,
	.ndo_stop		= do_stop,
	.ndo_start_xmit		= do_start_xmit,
	.ndo_tx_timeout 	= xirc_tx_timeout,
	.ndo_set_config		= do_config,
	.ndo_do_ioctl		= do_ioctl,
	.ndo_set_multicast_list	= set_multicast_list,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};



static int
xirc2ps_probe(struct pcmcia_device *link)
{
    struct net_device *dev;
    local_info_t *local;

    DEBUG(0, "attach()\n");

    
    dev = alloc_etherdev(sizeof(local_info_t));
    if (!dev)
	    return -ENOMEM;
    local = netdev_priv(dev);
    local->dev = dev;
    local->p_dev = link;
    link->priv = dev;

    
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.ConfigIndex = 1;
    link->irq.Handler = xirc2ps_interrupt;
    link->irq.Instance = dev;

    
    dev->netdev_ops = &netdev_ops;
    dev->ethtool_ops = &netdev_ethtool_ops;
    dev->watchdog_timeo = TX_TIMEOUT;
    INIT_WORK(&local->tx_timeout_task, xirc2ps_tx_timeout_task);

    return xirc2ps_config(link);
} 



static void
xirc2ps_detach(struct pcmcia_device *link)
{
    struct net_device *dev = link->priv;

    DEBUG(0, "detach(0x%p)\n", link);

    if (link->dev_node)
	unregister_netdev(dev);

    xirc2ps_release(link);

    free_netdev(dev);
} 


static int
set_card_type(struct pcmcia_device *link, const void *s)
{
    struct net_device *dev = link->priv;
    local_info_t *local = netdev_priv(dev);
  #ifdef PCMCIA_DEBUG
    unsigned cisrev = ((const unsigned char *)s)[2];
  #endif
    unsigned mediaid= ((const unsigned char *)s)[3];
    unsigned prodid = ((const unsigned char *)s)[4];

    DEBUG(0, "cisrev=%02x mediaid=%02x prodid=%02x\n",
	  cisrev, mediaid, prodid);

    local->mohawk = 0;
    local->dingo = 0;
    local->modem = 0;
    local->card_type = XIR_UNKNOWN;
    if (!(prodid & 0x40)) {
	printk(KNOT_XIRC "Ooops: Not a creditcard\n");
	return 0;
    }
    if (!(mediaid & 0x01)) {
	printk(KNOT_XIRC "Not an Ethernet card\n");
	return 0;
    }
    if (mediaid & 0x10) {
	local->modem = 1;
	switch(prodid & 15) {
	  case 1: local->card_type = XIR_CEM   ; break;
	  case 2: local->card_type = XIR_CEM2  ; break;
	  case 3: local->card_type = XIR_CEM3  ; break;
	  case 4: local->card_type = XIR_CEM33 ; break;
	  case 5: local->card_type = XIR_CEM56M;
		  local->mohawk = 1;
		  break;
	  case 6:
	  case 7: 
		  local->card_type = XIR_CEM56 ;
		  local->mohawk = 1;
		  local->dingo = 1;
		  break;
	}
    } else {
	switch(prodid & 15) {
	  case 1: local->card_type = has_ce2_string(link)? XIR_CE2 : XIR_CE ;
		  break;
	  case 2: local->card_type = XIR_CE2; break;
	  case 3: local->card_type = XIR_CE3;
		  local->mohawk = 1;
		  break;
	}
    }
    if (local->card_type == XIR_CE || local->card_type == XIR_CEM) {
	printk(KNOT_XIRC "Sorry, this is an old CE card\n");
	return 0;
    }
    if (local->card_type == XIR_UNKNOWN)
	printk(KNOT_XIRC "unknown card (mediaid=%02x prodid=%02x)\n",
	       mediaid, prodid);

    return 1;
}


static int
has_ce2_string(struct pcmcia_device * p_dev)
{
	if (p_dev->prod_id[2] && strstr(p_dev->prod_id[2], "CE2"))
		return 1;
	return 0;
}

static int
xirc2ps_config_modem(struct pcmcia_device *p_dev,
		     cistpl_cftable_entry_t *cf,
		     cistpl_cftable_entry_t *dflt,
		     unsigned int vcc,
		     void *priv_data)
{
	unsigned int ioaddr;

	if (cf->io.nwin > 0  &&  (cf->io.win[0].base & 0xf) == 8) {
		for (ioaddr = 0x300; ioaddr < 0x400; ioaddr += 0x10) {
			p_dev->io.BasePort2 = cf->io.win[0].base;
			p_dev->io.BasePort1 = ioaddr;
			if (!pcmcia_request_io(p_dev, &p_dev->io))
				return 0;
		}
	}
	return -ENODEV;
}

static int
xirc2ps_config_check(struct pcmcia_device *p_dev,
		     cistpl_cftable_entry_t *cf,
		     cistpl_cftable_entry_t *dflt,
		     unsigned int vcc,
		     void *priv_data)
{
	int *pass = priv_data;

	if (cf->io.nwin > 0 && (cf->io.win[0].base & 0xf) == 8) {
		p_dev->io.BasePort2 = cf->io.win[0].base;
		p_dev->io.BasePort1 = p_dev->io.BasePort2
			+ (*pass ? (cf->index & 0x20 ? -24:8)
			   : (cf->index & 0x20 ?   8:-24));
		if (!pcmcia_request_io(p_dev, &p_dev->io))
			return 0;
	}
	return -ENODEV;

}


static int
xirc2ps_config(struct pcmcia_device * link)
{
    struct net_device *dev = link->priv;
    local_info_t *local = netdev_priv(dev);
    unsigned int ioaddr;
    tuple_t tuple;
    cisparse_t parse;
    int err, i;
    u_char buf[64];
    cistpl_lan_node_id_t *node_id = (cistpl_lan_node_id_t*)parse.funce.data;

    local->dingo_ccr = NULL;

    DEBUG(0, "config(0x%p)\n", link);

    
    tuple.Attributes = 0;
    tuple.TupleData = buf;
    tuple.TupleDataMax = 64;
    tuple.TupleOffset = 0;

    
    tuple.DesiredTuple = CISTPL_MANFID;
    if ((err=first_tuple(link, &tuple, &parse))) {
	printk(KNOT_XIRC "manfid not found in CIS\n");
	goto failure;
    }

    switch(parse.manfid.manf) {
      case MANFID_XIRCOM:
	local->manf_str = "Xircom";
	break;
      case MANFID_ACCTON:
	local->manf_str = "Accton";
	break;
      case MANFID_COMPAQ:
      case MANFID_COMPAQ2:
	local->manf_str = "Compaq";
	break;
      case MANFID_INTEL:
	local->manf_str = "Intel";
	break;
      case MANFID_TOSHIBA:
	local->manf_str = "Toshiba";
	break;
      default:
	printk(KNOT_XIRC "Unknown Card Manufacturer ID: 0x%04x\n",
	       (unsigned)parse.manfid.manf);
	goto failure;
    }
    DEBUG(0, "found %s card\n", local->manf_str);

    if (!set_card_type(link, buf)) {
	printk(KNOT_XIRC "this card is not supported\n");
	goto failure;
    }

    
    tuple.DesiredTuple = CISTPL_FUNCE;
    for (err = first_tuple(link, &tuple, &parse); !err;
			     err = next_tuple(link, &tuple, &parse)) {
	
	if (parse.funce.type == CISTPL_FUNCE_LAN_NODE_ID
	    && ((cistpl_lan_node_id_t *)parse.funce.data)->nb)
	    break;
    }
    if (err) { 
	tuple.DesiredTuple = 0x89;  
	if ((err = pcmcia_get_first_tuple(link, &tuple)) == 0 &&
		(err = pcmcia_get_tuple_data(link, &tuple)) == 0) {
	    if (tuple.TupleDataLen == 8 && *buf == CISTPL_FUNCE_LAN_NODE_ID)
		memcpy(&parse, buf, 8);
	    else
		err = -1;
	}
    }
    if (err) { 
	tuple.DesiredTuple = CISTPL_FUNCE;
	for (err = first_tuple(link, &tuple, &parse); !err;
				 err = next_tuple(link, &tuple, &parse)) {
	    if (parse.funce.type == 0x02 && parse.funce.data[0] == 1
		&& parse.funce.data[1] == 6 && tuple.TupleDataLen == 13) {
		buf[1] = 4;
		memcpy(&parse, buf+1, 8);
		break;
	    }
	}
    }
    if (err) {
	printk(KNOT_XIRC "node-id not found in CIS\n");
	goto failure;
    }
    node_id = (cistpl_lan_node_id_t *)parse.funce.data;
    if (node_id->nb != 6) {
	printk(KNOT_XIRC "malformed node-id in CIS\n");
	goto failure;
    }
    for (i=0; i < 6; i++)
	dev->dev_addr[i] = node_id->id[i];

    link->io.IOAddrLines =10;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
    link->irq.Attributes = IRQ_HANDLE_PRESENT;
    link->irq.IRQInfo1 = IRQ_LEVEL_ID;
    if (local->modem) {
	int pass;

	if (do_sound) {
	    link->conf.Attributes |= CONF_ENABLE_SPKR;
	    link->conf.Status |= CCSR_AUDIO_ENA;
	}
	link->irq.Attributes |= IRQ_TYPE_DYNAMIC_SHARING|IRQ_FIRST_SHARED ;
	link->io.NumPorts2 = 8;
	link->io.Attributes2 = IO_DATA_PATH_WIDTH_8;
	if (local->dingo) {
	    
	    link->io.NumPorts1 = 16; 
	    if (!pcmcia_loop_config(link, xirc2ps_config_modem, NULL))
		    goto port_found;
	} else {
	    link->io.NumPorts1 = 18;
	    
	    for (pass=0; pass < 2; pass++)
		    if (!pcmcia_loop_config(link, xirc2ps_config_check, &pass))
			    goto port_found;
	    
	}
	printk(KNOT_XIRC "no ports available\n");
    } else {
	link->irq.Attributes |= IRQ_TYPE_DYNAMIC_SHARING;
	link->io.NumPorts1 = 16;
	for (ioaddr = 0x300; ioaddr < 0x400; ioaddr += 0x10) {
	    link->io.BasePort1 = ioaddr;
	    if (!(err=pcmcia_request_io(link, &link->io)))
		goto port_found;
	}
	link->io.BasePort1 = 0; 
	if ((err=pcmcia_request_io(link, &link->io))) {
	    cs_error(link, RequestIO, err);
	    goto config_error;
	}
    }
  port_found:
    if (err)
	 goto config_error;

    
    if ((err=pcmcia_request_irq(link, &link->irq))) {
	cs_error(link, RequestIRQ, err);
	goto config_error;
    }

    
    if ((err=pcmcia_request_configuration(link, &link->conf))) {
	cs_error(link, RequestConfiguration, err);
	goto config_error;
    }

    if (local->dingo) {
	conf_reg_t reg;
	win_req_t req;
	memreq_t mem;

	
	reg.Action = CS_WRITE;
	reg.Offset = CISREG_IOBASE_0;
	reg.Value = link->io.BasePort2 & 0xff;
	if ((err = pcmcia_access_configuration_register(link, &reg))) {
	    cs_error(link, AccessConfigurationRegister, err);
	    goto config_error;
	}
	reg.Action = CS_WRITE;
	reg.Offset = CISREG_IOBASE_1;
	reg.Value = (link->io.BasePort2 >> 8) & 0xff;
	if ((err = pcmcia_access_configuration_register(link, &reg))) {
	    cs_error(link, AccessConfigurationRegister, err);
	    goto config_error;
	}

	
	req.Attributes = WIN_DATA_WIDTH_8|WIN_MEMORY_TYPE_AM|WIN_ENABLE;
	req.Base = req.Size = 0;
	req.AccessSpeed = 0;
	if ((err = pcmcia_request_window(&link, &req, &link->win))) {
	    cs_error(link, RequestWindow, err);
	    goto config_error;
	}
	local->dingo_ccr = ioremap(req.Base,0x1000) + 0x0800;
	mem.CardOffset = 0x0;
	mem.Page = 0;
	if ((err = pcmcia_map_mem_page(link->win, &mem))) {
	    cs_error(link, MapMemPage, err);
	    goto config_error;
	}

	
	writeb(0x47, local->dingo_ccr + CISREG_COR);
	ioaddr = link->io.BasePort1;
	writeb(ioaddr & 0xff	  , local->dingo_ccr + CISREG_IOBASE_0);
	writeb((ioaddr >> 8)&0xff , local->dingo_ccr + CISREG_IOBASE_1);

      #if 0
	{
	    u_char tmp;
	    printk(KERN_INFO "ECOR:");
	    for (i=0; i < 7; i++) {
		tmp = readb(local->dingo_ccr + i*2);
		printk(" %02x", tmp);
	    }
	    printk("\n");
	    printk(KERN_INFO "DCOR:");
	    for (i=0; i < 4; i++) {
		tmp = readb(local->dingo_ccr + 0x20 + i*2);
		printk(" %02x", tmp);
	    }
	    printk("\n");
	    printk(KERN_INFO "SCOR:");
	    for (i=0; i < 10; i++) {
		tmp = readb(local->dingo_ccr + 0x40 + i*2);
		printk(" %02x", tmp);
	    }
	    printk("\n");
	}
      #endif

	writeb(0x01, local->dingo_ccr + 0x20);
	writeb(0x0c, local->dingo_ccr + 0x22);
	writeb(0x00, local->dingo_ccr + 0x24);
	writeb(0x00, local->dingo_ccr + 0x26);
	writeb(0x00, local->dingo_ccr + 0x28);
    }

    
    local->probe_port=0;
    if (!if_port) {
	local->probe_port = dev->if_port = 1;
    } else if ((if_port >= 1 && if_port <= 2) ||
	       (local->mohawk && if_port==4))
	dev->if_port = if_port;
    else
	printk(KNOT_XIRC "invalid if_port requested\n");

    
    dev->irq = link->irq.AssignedIRQ;
    dev->base_addr = link->io.BasePort1;

    if (local->dingo)
	do_reset(dev, 1); 

    link->dev_node = &local->node;
    SET_NETDEV_DEV(dev, &handle_to_dev(link));

    if ((err=register_netdev(dev))) {
	printk(KNOT_XIRC "register_netdev() failed\n");
	link->dev_node = NULL;
	goto config_error;
    }

    strcpy(local->node.dev_name, dev->name);

    
    printk(KERN_INFO "%s: %s: port %#3lx, irq %d, hwaddr %pM\n",
	   dev->name, local->manf_str,(u_long)dev->base_addr, (int)dev->irq,
	   dev->dev_addr);

    return 0;

  config_error:
    xirc2ps_release(link);
    return -ENODEV;

  failure:
    return -ENODEV;
} 


static void
xirc2ps_release(struct pcmcia_device *link)
{
	DEBUG(0, "release(0x%p)\n", link);

	if (link->win) {
		struct net_device *dev = link->priv;
		local_info_t *local = netdev_priv(dev);
		if (local->dingo)
			iounmap(local->dingo_ccr - 0x0800);
	}
	pcmcia_disable_device(link);
} 




static int xirc2ps_suspend(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open) {
		netif_device_detach(dev);
		do_powerdown(dev);
	}

	return 0;
}

static int xirc2ps_resume(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open) {
		do_reset(dev,1);
		netif_device_attach(dev);
	}

	return 0;
}





static irqreturn_t
xirc2ps_interrupt(int irq, void *dev_id)
{
    struct net_device *dev = (struct net_device *)dev_id;
    local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr;
    u_char saved_page;
    unsigned bytes_rcvd;
    unsigned int_status, eth_status, rx_status, tx_status;
    unsigned rsr, pktlen;
    ulong start_ticks = jiffies; 

    if (!netif_device_present(dev))
	return IRQ_HANDLED;

    ioaddr = dev->base_addr;
    if (lp->mohawk) { 
	PutByte(XIRCREG_CR, 0);
    }

    DEBUG(6, "%s: interrupt %d at %#x.\n", dev->name, irq, ioaddr);

    saved_page = GetByte(XIRCREG_PR);
    
    int_status = GetByte(XIRCREG_ISR);
    bytes_rcvd = 0;
  loop_entry:
    if (int_status == 0xff) { 
	DEBUG(3, "%s: interrupt %d for dead card\n", dev->name, irq);
	goto leave;
    }
    eth_status = GetByte(XIRCREG_ESR);

    SelectPage(0x40);
    rx_status  = GetByte(XIRCREG40_RXST0);
    PutByte(XIRCREG40_RXST0, (~rx_status & 0xff));
    tx_status = GetByte(XIRCREG40_TXST0);
    tx_status |= GetByte(XIRCREG40_TXST1) << 8;
    PutByte(XIRCREG40_TXST0, 0);
    PutByte(XIRCREG40_TXST1, 0);

    DEBUG(3, "%s: ISR=%#2.2x ESR=%#2.2x RSR=%#2.2x TSR=%#4.4x\n",
	  dev->name, int_status, eth_status, rx_status, tx_status);

    
    SelectPage(0);
    while (eth_status & FullPktRcvd) {
	rsr = GetByte(XIRCREG0_RSR);
	if (bytes_rcvd > maxrx_bytes && (rsr & PktRxOk)) {
	    
	    dev->stats.rx_dropped++;
	    DEBUG(2, "%s: RX drop, too much done\n", dev->name);
	} else if (rsr & PktRxOk) {
	    struct sk_buff *skb;

	    pktlen = GetWord(XIRCREG0_RBC);
	    bytes_rcvd += pktlen;

	    DEBUG(5, "rsr=%#02x packet_length=%u\n", rsr, pktlen);

	    skb = dev_alloc_skb(pktlen+3); 
	    if (!skb) {
		printk(KNOT_XIRC "low memory, packet dropped (size=%u)\n",
		       pktlen);
		dev->stats.rx_dropped++;
	    } else { 
		skb_reserve(skb, 2);
		if (lp->silicon == 0 ) { 
		    unsigned rhsa; 

		    SelectPage(5);
		    rhsa = GetWord(XIRCREG5_RHSA0);
		    SelectPage(0);
		    rhsa += 3; 
		    if (rhsa >= 0x8000)
			rhsa = 0;
		    if (rhsa + pktlen > 0x8000) {
			unsigned i;
			u_char *buf = skb_put(skb, pktlen);
			for (i=0; i < pktlen ; i++, rhsa++) {
			    buf[i] = GetByte(XIRCREG_EDP);
			    if (rhsa == 0x8000) {
				rhsa = 0;
				i--;
			    }
			}
		    } else {
			insw(ioaddr+XIRCREG_EDP,
				skb_put(skb, pktlen), (pktlen+1)>>1);
		    }
		}
	      #if 0
		else if (lp->mohawk) {
		    
		    unsigned i;
		    u_long *p = skb_put(skb, pktlen);
		    register u_long a;
		    unsigned int edpreg = ioaddr+XIRCREG_EDP-2;
		    for (i=0; i < len ; i += 4, p++) {
			a = inl(edpreg);
			__asm__("rorl $16,%0\n\t"
				:"=q" (a)
				: "0" (a));
			*p = a;
		    }
		}
	      #endif
		else {
		    insw(ioaddr+XIRCREG_EDP, skb_put(skb, pktlen),
			    (pktlen+1)>>1);
		}
		skb->protocol = eth_type_trans(skb, dev);
		netif_rx(skb);
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pktlen;
		if (!(rsr & PhyPkt))
		    dev->stats.multicast++;
	    }
	} else { 
	    DEBUG(5, "rsr=%#02x\n", rsr);
	}
	if (rsr & PktTooLong) {
	    dev->stats.rx_frame_errors++;
	    DEBUG(3, "%s: Packet too long\n", dev->name);
	}
	if (rsr & CRCErr) {
	    dev->stats.rx_crc_errors++;
	    DEBUG(3, "%s: CRC error\n", dev->name);
	}
	if (rsr & AlignErr) {
	    dev->stats.rx_fifo_errors++; 
	    DEBUG(3, "%s: Alignment error\n", dev->name);
	}

	
	PutWord(XIRCREG0_DO, 0x8000); 

	
	eth_status = GetByte(XIRCREG_ESR);
    }
    if (rx_status & 0x10) { 
	dev->stats.rx_over_errors++;
	PutByte(XIRCREG_CR, ClearRxOvrun);
	DEBUG(3, "receive overrun cleared\n");
    }

    
    if (int_status & PktTxed) {
	unsigned n, nn;

	n = lp->last_ptr_value;
	nn = GetByte(XIRCREG0_PTR);
	lp->last_ptr_value = nn;
	if (nn < n) 
	    dev->stats.tx_packets += 256 - n;
	else if (n == nn) { 
	    DEBUG(0, "PTR not changed?\n");
	} else
	    dev->stats.tx_packets += lp->last_ptr_value - n;
	netif_wake_queue(dev);
    }
    if (tx_status & 0x0002) {	
	DEBUG(0, "tx restarted due to execssive collissions\n");
	PutByte(XIRCREG_CR, RestartTx);  
    }
    if (tx_status & 0x0040)
	dev->stats.tx_aborted_errors++;

    
    if (bytes_rcvd > 1000) {
	u_long duration = jiffies - start_ticks;

	if (duration >= HZ/10) { 
	    maxrx_bytes = (bytes_rcvd * (HZ/10)) / duration;
	    if (maxrx_bytes < 2000)
		maxrx_bytes = 2000;
	    else if (maxrx_bytes > 22000)
		maxrx_bytes = 22000;
	    DEBUG(1, "set maxrx=%u (rcvd=%u ticks=%lu)\n",
		  maxrx_bytes, bytes_rcvd, duration);
	} else if (!duration && maxrx_bytes < 22000) {
	    
	    maxrx_bytes += 2000;
	    if (maxrx_bytes > 22000)
		maxrx_bytes = 22000;
	    DEBUG(1, "set maxrx=%u\n", maxrx_bytes);
	}
    }

  leave:
    if (lockup_hack) {
	if (int_status != 0xff && (int_status = GetByte(XIRCREG_ISR)) != 0)
	    goto loop_entry;
    }
    SelectPage(saved_page);
    PutByte(XIRCREG_CR, EnableIntr);  
    
    return IRQ_HANDLED;
} 



static void
xirc2ps_tx_timeout_task(struct work_struct *work)
{
	local_info_t *local =
		container_of(work, local_info_t, tx_timeout_task);
	struct net_device *dev = local->dev;
    
    do_reset(dev,1);
    dev->trans_start = jiffies;
    netif_wake_queue(dev);
}

static void
xirc_tx_timeout(struct net_device *dev)
{
    local_info_t *lp = netdev_priv(dev);
    dev->stats.tx_errors++;
    printk(KERN_NOTICE "%s: transmit timed out\n", dev->name);
    schedule_work(&lp->tx_timeout_task);
}

static netdev_tx_t
do_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    int okay;
    unsigned freespace;
    unsigned pktlen = skb->len;

    DEBUG(1, "do_start_xmit(skb=%p, dev=%p) len=%u\n",
	  skb, dev, pktlen);


    
    if (pktlen < ETH_ZLEN)
    {
        if (skb_padto(skb, ETH_ZLEN))
        	return NETDEV_TX_OK;
	pktlen = ETH_ZLEN;
    }

    netif_stop_queue(dev);
    SelectPage(0);
    PutWord(XIRCREG0_TRS, (u_short)pktlen+2);
    freespace = GetWord(XIRCREG0_TSO);
    okay = freespace & 0x8000;
    freespace &= 0x7fff;
    
    okay = pktlen +2 < freespace;
    DEBUG(2 + (okay ? 2 : 0), "%s: avail. tx space=%u%s\n",
	  dev->name, freespace, okay ? " (okay)":" (not enough)");
    if (!okay) { 
	return NETDEV_TX_BUSY;  
    }
    
    PutWord(XIRCREG_EDP, (u_short)pktlen);
    outsw(ioaddr+XIRCREG_EDP, skb->data, pktlen>>1);
    if (pktlen & 1)
	PutByte(XIRCREG_EDP, skb->data[pktlen-1]);

    if (lp->mohawk)
	PutByte(XIRCREG_CR, TransmitPacket|EnableIntr);

    dev_kfree_skb (skb);
    dev->trans_start = jiffies;
    dev->stats.tx_bytes += pktlen;
    netif_start_queue(dev);
    return NETDEV_TX_OK;
}


static void
set_addresses(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;
    local_info_t *lp = netdev_priv(dev);
    struct dev_mc_list *dmi = dev->mc_list;
    unsigned char *addr;
    int i,j,k,n;

    SelectPage(k=0x50);
    for (i=0,j=8,n=0; ; i++, j++) {
	if (i > 5) {
	    if (++n > 9)
		break;
	    i = 0;
	    if (n > 1 && n <= dev->mc_count && dmi) {
	   	 dmi = dmi->next;
	    }
	}
	if (j > 15) {
	    j = 8;
	    k++;
	    SelectPage(k);
	}

	if (n && n <= dev->mc_count && dmi)
	    addr = dmi->dmi_addr;
	else
	    addr = dev->dev_addr;

	if (lp->mohawk)
	    PutByte(j, addr[5-i]);
	else
	    PutByte(j, addr[i]);
    }
    SelectPage(0);
}



static void
set_multicast_list(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;
    unsigned value;

    SelectPage(0x42);
    value = GetByte(XIRCREG42_SWC1) & 0xC0;

    if (dev->flags & IFF_PROMISC) { 
	PutByte(XIRCREG42_SWC1, value | 0x06); 
    } else if (dev->mc_count > 9 || (dev->flags & IFF_ALLMULTI)) {
	PutByte(XIRCREG42_SWC1, value | 0x02); 
    } else if (dev->mc_count) {
	
	PutByte(XIRCREG42_SWC1, value | 0x01);
	SelectPage(0x40);
	PutByte(XIRCREG40_CMD0, Offline);
	set_addresses(dev);
	SelectPage(0x40);
	PutByte(XIRCREG40_CMD0, EnableRecv | Online);
    } else { 
	PutByte(XIRCREG42_SWC1, value | 0x00);
    }
    SelectPage(0);
}

static int
do_config(struct net_device *dev, struct ifmap *map)
{
    local_info_t *local = netdev_priv(dev);

    DEBUG(0, "do_config(%p)\n", dev);
    if (map->port != 255 && map->port != dev->if_port) {
	if (map->port > 4)
	    return -EINVAL;
	if (!map->port) {
	    local->probe_port = 1;
	    dev->if_port = 1;
	} else {
	    local->probe_port = 0;
	    dev->if_port = map->port;
	}
	printk(KERN_INFO "%s: switching to %s port\n",
	       dev->name, if_names[dev->if_port]);
	do_reset(dev,1);  
    }
    return 0;
}


static int
do_open(struct net_device *dev)
{
    local_info_t *lp = netdev_priv(dev);
    struct pcmcia_device *link = lp->p_dev;

    DEBUG(0, "do_open(%p)\n", dev);

    
    
    if (!pcmcia_dev_present(link))
	return -ENODEV;

    
    link->open++;

    netif_start_queue(dev);
    do_reset(dev,1);

    return 0;
}

static void netdev_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "xirc2ps_cs");
	sprintf(info->bus_info, "PCMCIA 0x%lx", dev->base_addr);
}

static const struct ethtool_ops netdev_ethtool_ops = {
	.get_drvinfo		= netdev_get_drvinfo,
};

static int
do_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    local_info_t *local = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    struct mii_ioctl_data *data = if_mii(rq);

    DEBUG(1, "%s: ioctl(%-.6s, %#04x) %04x %04x %04x %04x\n",
	  dev->name, rq->ifr_ifrn.ifrn_name, cmd,
	  data->phy_id, data->reg_num, data->val_in, data->val_out);

    if (!local->mohawk)
	return -EOPNOTSUPP;

    switch(cmd) {
      case SIOCGMIIPHY:		
	data->phy_id = 0;	
	
      case SIOCGMIIREG:		
	data->val_out = mii_rd(ioaddr, data->phy_id & 0x1f,
			       data->reg_num & 0x1f);
	break;
      case SIOCSMIIREG:		
	mii_wr(ioaddr, data->phy_id & 0x1f, data->reg_num & 0x1f, data->val_in,
	       16);
	break;
      default:
	return -EOPNOTSUPP;
    }
    return 0;
}

static void
hardreset(struct net_device *dev)
{
    local_info_t *local = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;

    SelectPage(4);
    udelay(1);
    PutByte(XIRCREG4_GPR1, 0);	     
    msleep(40);				     
    if (local->mohawk)
	PutByte(XIRCREG4_GPR1, 1);	 
    else
	PutByte(XIRCREG4_GPR1, 1 | 4);	 
    msleep(20);			     
}

static void
do_reset(struct net_device *dev, int full)
{
    local_info_t *local = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    unsigned value;

    DEBUG(0, "%s: do_reset(%p,%d)\n", dev? dev->name:"eth?", dev, full);

    hardreset(dev);
    PutByte(XIRCREG_CR, SoftReset); 
    msleep(20);			     
    PutByte(XIRCREG_CR, 0);	     
    msleep(40);			     
    if (local->mohawk) {
	SelectPage(4);
	
	PutByte(XIRCREG4_GPR0, 0x0e);
    }

    
    msleep(500);			

    local->last_ptr_value = 0;
    local->silicon = local->mohawk ? (GetByte(XIRCREG4_BOV) & 0x70) >> 4
				   : (GetByte(XIRCREG4_BOV) & 0x30) >> 4;

    if (local->probe_port) {
	if (!local->mohawk) {
	    SelectPage(4);
	    PutByte(XIRCREG4_GPR0, 4);
	    local->probe_port = 0;
	}
    } else if (dev->if_port == 2) { 
	SelectPage(0x42);
	PutByte(XIRCREG42_SWC1, 0xC0);
    } else { 
	SelectPage(0x42);
	PutByte(XIRCREG42_SWC1, 0x80);
    }
    msleep(40);			     

  #ifdef PCMCIA_DEBUG
    if (pc_debug) {
	SelectPage(0);
	value = GetByte(XIRCREG_ESR);	 
	printk(KERN_DEBUG "%s: ESR is: %#02x\n", dev->name, value);
    }
  #endif

    
    SelectPage(1);
    PutByte(XIRCREG1_IMR0, 0xff); 
    PutByte(XIRCREG1_IMR1, 1	); 
    value = GetByte(XIRCREG1_ECR);
  #if 0
    if (local->mohawk)
	value |= DisableLinkPulse;
    PutByte(XIRCREG1_ECR, value);
  #endif
    DEBUG(0, "%s: ECR is: %#02x\n", dev->name, value);

    SelectPage(0x42);
    PutByte(XIRCREG42_SWC0, 0x20); 

    if (local->silicon != 1) {
	
	SelectPage(2);
	PutWord(XIRCREG2_RBS, 0x2000);
    }

    if (full)
	set_addresses(dev);

    
    SelectPage(0);
    PutWord(XIRCREG0_DO, 0x2000); 

    
    SelectPage(0x40);		     
    PutByte(XIRCREG40_RMASK0, 0xff); 
    PutByte(XIRCREG40_TMASK0, 0xff); 
    PutByte(XIRCREG40_TMASK1, 0xb0); 
    PutByte(XIRCREG40_RXST0,  0x00); 
    PutByte(XIRCREG40_TXST0,  0x00); 
    PutByte(XIRCREG40_TXST1,  0x00); 

    if (full && local->mohawk && init_mii(dev)) {
	if (dev->if_port == 4 || local->dingo || local->new_mii) {
	    printk(KERN_INFO "%s: MII selected\n", dev->name);
	    SelectPage(2);
	    PutByte(XIRCREG2_MSR, GetByte(XIRCREG2_MSR) | 0x08);
	    msleep(20);
	} else {
	    printk(KERN_INFO "%s: MII detected; using 10mbs\n",
		   dev->name);
	    SelectPage(0x42);
	    if (dev->if_port == 2) 
		PutByte(XIRCREG42_SWC1, 0xC0);
	    else  
		PutByte(XIRCREG42_SWC1, 0x80);
	    msleep(40);			
	}
	if (full_duplex)
	    PutByte(XIRCREG1_ECR, GetByte(XIRCREG1_ECR | FullDuplex));
    } else {  
	SelectPage(0);
	value = GetByte(XIRCREG_ESR);	 
	dev->if_port = (value & MediaSelect) ? 1 : 2;
    }

    
    SelectPage(2);
    if (dev->if_port == 1 || dev->if_port == 4) 
	PutByte(XIRCREG2_LED, 0x3b);
    else			      
	PutByte(XIRCREG2_LED, 0x3a);

    if (local->dingo)
	PutByte(0x0b, 0x04); 

    
    if (full) {
	set_multicast_list(dev);
	SelectPage(0x40);
	PutByte(XIRCREG40_CMD0, EnableRecv | Online);
    }

    
    SelectPage(1);
    PutByte(XIRCREG1_IMR0, 0xff);
    udelay(1);
    SelectPage(0);
    PutByte(XIRCREG_CR, EnableIntr);
    if (local->modem && !local->dingo) { 
	if (!(GetByte(0x10) & 0x01))
	    PutByte(0x10, 0x11); 
    }

    if (full)
	printk(KERN_INFO "%s: media %s, silicon revision %d\n",
	       dev->name, if_names[dev->if_port], local->silicon);
    
    SelectPage(0);
}


static int
init_mii(struct net_device *dev)
{
    local_info_t *local = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    unsigned control, status, linkpartner;
    int i;

    if (if_port == 4 || if_port == 1) { 
	dev->if_port = if_port;
	local->probe_port = 0;
	return 1;
    }

    status = mii_rd(ioaddr,  0, 1);
    if ((status & 0xff00) != 0x7800)
	return 0; 

    local->new_mii = (mii_rd(ioaddr, 0, 2) != 0xffff);
    
    if (local->probe_port)
	control = 0x1000; 
    else if (dev->if_port == 4)
	control = 0x2000; 
    else
	control = 0x0000; 
    mii_wr(ioaddr,  0, 0, control, 16);
    udelay(100);
    control = mii_rd(ioaddr, 0, 0);

    if (control & 0x0400) {
	printk(KERN_NOTICE "%s can't take PHY out of isolation mode\n",
	       dev->name);
	local->probe_port = 0;
	return 0;
    }

    if (local->probe_port) {
	
	for (i=0; i < 35; i++) {
	    msleep(100);	 
	    status = mii_rd(ioaddr,  0, 1);
	    if ((status & 0x0020) && (status & 0x0004))
		break;
	}

	if (!(status & 0x0020)) {
	    printk(KERN_INFO "%s: autonegotiation failed;"
		   " using 10mbs\n", dev->name);
	    if (!local->new_mii) {
		control = 0x0000;
		mii_wr(ioaddr,  0, 0, control, 16);
		udelay(100);
		SelectPage(0);
		dev->if_port = (GetByte(XIRCREG_ESR) & MediaSelect) ? 1 : 2;
	    }
	} else {
	    linkpartner = mii_rd(ioaddr, 0, 5);
	    printk(KERN_INFO "%s: MII link partner: %04x\n",
		   dev->name, linkpartner);
	    if (linkpartner & 0x0080) {
		dev->if_port = 4;
	    } else
		dev->if_port = 1;
	}
    }

    return 1;
}

static void
do_powerdown(struct net_device *dev)
{

    unsigned int ioaddr = dev->base_addr;

    DEBUG(0, "do_powerdown(%p)\n", dev);

    SelectPage(4);
    PutByte(XIRCREG4_GPR1, 0);	     
    SelectPage(0);
}

static int
do_stop(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;
    local_info_t *lp = netdev_priv(dev);
    struct pcmcia_device *link = lp->p_dev;

    DEBUG(0, "do_stop(%p)\n", dev);

    if (!link)
	return -ENODEV;

    netif_stop_queue(dev);

    SelectPage(0);
    PutByte(XIRCREG_CR, 0);  
    SelectPage(0x01);
    PutByte(XIRCREG1_IMR0, 0x00); 
    SelectPage(4);
    PutByte(XIRCREG4_GPR1, 0);	
    SelectPage(0);

    link->open--;
    return 0;
}

static struct pcmcia_device_id xirc2ps_ids[] = {
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0089, 0x110a),
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0138, 0x110a),
	PCMCIA_PFC_DEVICE_PROD_ID13(0, "Xircom", "CEM28", 0x2e3ee845, 0x0ea978ea),
	PCMCIA_PFC_DEVICE_PROD_ID13(0, "Xircom", "CEM33", 0x2e3ee845, 0x80609023),
	PCMCIA_PFC_DEVICE_PROD_ID13(0, "Xircom", "CEM56", 0x2e3ee845, 0xa650c32a),
	PCMCIA_PFC_DEVICE_PROD_ID13(0, "Xircom", "REM10", 0x2e3ee845, 0x76df1d29),
	PCMCIA_PFC_DEVICE_PROD_ID13(0, "Xircom", "XEM5600", 0x2e3ee845, 0xf1403719),
	PCMCIA_PFC_DEVICE_PROD_ID12(0, "Xircom", "CreditCard Ethernet+Modem II", 0x2e3ee845, 0xeca401bf),
	PCMCIA_DEVICE_MANF_CARD(0x01bf, 0x010a),
	PCMCIA_DEVICE_PROD_ID13("Toshiba Information Systems", "TPCENET", 0x1b3b94fe, 0xf381c1a2),
	PCMCIA_DEVICE_PROD_ID13("Xircom", "CE3-10/100", 0x2e3ee845, 0x0ec0ac37),
	PCMCIA_DEVICE_PROD_ID13("Xircom", "PS-CE2-10", 0x2e3ee845, 0x947d9073),
	PCMCIA_DEVICE_PROD_ID13("Xircom", "R2E-100BTX", 0x2e3ee845, 0x2464a6e3),
	PCMCIA_DEVICE_PROD_ID13("Xircom", "RE-10", 0x2e3ee845, 0x3e08d609),
	PCMCIA_DEVICE_PROD_ID13("Xircom", "XE2000", 0x2e3ee845, 0xf7188e46),
	PCMCIA_DEVICE_PROD_ID12("Compaq", "Ethernet LAN Card", 0x54f7c49c, 0x9fd2f0a2),
	PCMCIA_DEVICE_PROD_ID12("Compaq", "Netelligent 10/100 PC Card", 0x54f7c49c, 0xefe96769),
	PCMCIA_DEVICE_PROD_ID12("Intel", "EtherExpress(TM) PRO/100 PC Card Mobile Adapter16", 0x816cc815, 0x174397db),
	PCMCIA_DEVICE_PROD_ID12("Toshiba", "10/100 Ethernet PC Card", 0x44a09d9c, 0xb44deecf),
	
	
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, xirc2ps_ids);


static struct pcmcia_driver xirc2ps_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "xirc2ps_cs",
	},
	.probe		= xirc2ps_probe,
	.remove		= xirc2ps_detach,
	.id_table       = xirc2ps_ids,
	.suspend	= xirc2ps_suspend,
	.resume		= xirc2ps_resume,
};

static int __init
init_xirc2ps_cs(void)
{
	return pcmcia_register_driver(&xirc2ps_cs_driver);
}

static void __exit
exit_xirc2ps_cs(void)
{
	pcmcia_unregister_driver(&xirc2ps_cs_driver);
}

module_init(init_xirc2ps_cs);
module_exit(exit_xirc2ps_cs);

#ifndef MODULE
static int __init setup_xirc2ps_cs(char *str)
{
	
	int ints[10] = { -1 };

	str = get_options(str, 9, ints);

#define MAYBE_SET(X,Y) if (ints[0] >= Y && ints[Y] != -1) { X = ints[Y]; }
	MAYBE_SET(if_port, 3);
	MAYBE_SET(full_duplex, 4);
	MAYBE_SET(do_sound, 5);
	MAYBE_SET(lockup_hack, 6);
#undef  MAYBE_SET

	return 1;
}

__setup("xirc2ps_cs=", setup_xirc2ps_cs);
#endif
