

#define DRV_NAME	"fmvj18x_cs"
#define DRV_VERSION	"2.9"

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
#include <linux/crc32.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ciscode.h>
#include <pcmcia/ds.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>





MODULE_DESCRIPTION("fmvj18x and compatible PCMCIA ethernet driver");
MODULE_LICENSE("GPL");

#define INT_MODULE_PARM(n, v) static int n = v; module_param(n, int, 0)



INT_MODULE_PARM(sram_config, 0);

#ifdef PCMCIA_DEBUG
INT_MODULE_PARM(pc_debug, PCMCIA_DEBUG);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version = DRV_NAME ".c " DRV_VERSION " 2002/03/23";
#else
#define DEBUG(n, args...)
#endif



static int fmvj18x_config(struct pcmcia_device *link);
static int fmvj18x_get_hwinfo(struct pcmcia_device *link, u_char *node_id);
static int fmvj18x_setup_mfc(struct pcmcia_device *link);
static void fmvj18x_release(struct pcmcia_device *link);
static void fmvj18x_detach(struct pcmcia_device *p_dev);


static int fjn_config(struct net_device *dev, struct ifmap *map);
static int fjn_open(struct net_device *dev);
static int fjn_close(struct net_device *dev);
static netdev_tx_t fjn_start_xmit(struct sk_buff *skb,
					struct net_device *dev);
static irqreturn_t fjn_interrupt(int irq, void *dev_id);
static void fjn_rx(struct net_device *dev);
static void fjn_reset(struct net_device *dev);
static void set_rx_mode(struct net_device *dev);
static void fjn_tx_timeout(struct net_device *dev);
static const struct ethtool_ops netdev_ethtool_ops;


typedef enum { MBH10302, MBH10304, TDK, CONTEC, LA501, UNGERMANN, 
	       XXX10304, NEC, KME
} cardtype_t;


typedef struct local_info_t {
	struct pcmcia_device	*p_dev;
    dev_node_t node;
    long open_time;
    uint tx_started:1;
    uint tx_queue;
    u_short tx_queue_len;
    cardtype_t cardtype;
    u_short sent;
    u_char __iomem *base;
} local_info_t;

#define MC_FILTERBREAK 64



#define TX_STATUS               0 
#define RX_STATUS               1 
#define TX_INTR                 2 
#define RX_INTR                 3 
#define TX_MODE                 4 
#define RX_MODE                 5 
#define CONFIG_0                6 
#define CONFIG_1                7 

#define NODE_ID                 8 
#define MAR_ADR                 8 

#define DATAPORT                8 
#define TX_START               10 
#define COL_CTRL               11 
#define BMPR12                 12 
#define BMPR13                 13 
#define RX_SKIP                14 

#define LAN_CTRL               16 

#define MAC_ID               0x1a 
#define UNGERMANN_MAC_ID     0x18 


#define ENA_TMT_OK           0x80
#define ENA_TMT_REC          0x20
#define ENA_COL              0x04
#define ENA_16_COL           0x02
#define ENA_TBUS_ERR         0x01

#define ENA_PKT_RDY          0x80
#define ENA_BUS_ERR          0x40
#define ENA_LEN_ERR          0x08
#define ENA_ALG_ERR          0x04
#define ENA_CRC_ERR          0x02
#define ENA_OVR_FLO          0x01


#define F_TMT_RDY            0x80 
#define F_NET_BSY            0x40 
#define F_TMT_OK             0x20 
#define F_SRT_PKT            0x10 
#define F_COL_ERR            0x04 
#define F_16_COL             0x02 
#define F_TBUS_ERR           0x01 

#define F_PKT_RDY            0x80 
#define F_BUS_ERR            0x40 
#define F_LEN_ERR            0x08 
#define F_ALG_ERR            0x04 
#define F_CRC_ERR            0x02 
#define F_OVR_FLO            0x01 

#define F_BUF_EMP            0x40 

#define F_SKP_PKT            0x05 


#define D_TX_INTR  ( ENA_TMT_OK )
#define D_RX_INTR  ( ENA_PKT_RDY | ENA_LEN_ERR \
		   | ENA_ALG_ERR | ENA_CRC_ERR | ENA_OVR_FLO )
#define TX_STAT_M  ( F_TMT_RDY )
#define RX_STAT_M  ( F_PKT_RDY | F_LEN_ERR \
                   | F_ALG_ERR | F_CRC_ERR | F_OVR_FLO )


#define D_TX_MODE            0x06 
#define ID_MATCHED           0x02 
#define RECV_ALL             0x03 
#define CONFIG0_DFL          0x5a 
#define CONFIG0_DFL_1        0x5e 
#define CONFIG0_RST          0xda 
#define CONFIG0_RST_1        0xde 
#define BANK_0               0xa0 
#define BANK_1               0xa4 
#define BANK_2               0xa8 
#define CHIP_OFF             0x80 
#define DO_TX                0x80 
#define SEND_PKT             0x81 
#define AUTO_MODE            0x07 
#define MANU_MODE            0x03 
#define TDK_AUTO_MODE        0x47 
#define TDK_MANU_MODE        0x43 
#define INTR_OFF             0x0d 
#define INTR_ON              0x1d 

#define TX_TIMEOUT		((400*HZ)/1000)

#define BANK_0U              0x20 
#define BANK_1U              0x24 
#define BANK_2U              0x28 

static const struct net_device_ops fjn_netdev_ops = {
	.ndo_open 		= fjn_open,
	.ndo_stop		= fjn_close,
	.ndo_start_xmit 	= fjn_start_xmit,
	.ndo_tx_timeout 	= fjn_tx_timeout,
	.ndo_set_config 	= fjn_config,
	.ndo_set_multicast_list = set_rx_mode,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static int fmvj18x_probe(struct pcmcia_device *link)
{
    local_info_t *lp;
    struct net_device *dev;

    DEBUG(0, "fmvj18x_attach()\n");

    
    dev = alloc_etherdev(sizeof(local_info_t));
    if (!dev)
	return -ENOMEM;
    lp = netdev_priv(dev);
    link->priv = dev;
    lp->p_dev = link;
    lp->base = NULL;

    
    link->io.NumPorts1 = 32;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
    link->io.IOAddrLines = 5;

    
    link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING|IRQ_HANDLE_PRESENT;
    link->irq.IRQInfo1 = IRQ_LEVEL_ID;
    link->irq.Handler = &fjn_interrupt;
    link->irq.Instance = dev;

    
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;

    dev->netdev_ops = &fjn_netdev_ops;
    dev->watchdog_timeo = TX_TIMEOUT;

    SET_ETHTOOL_OPS(dev, &netdev_ethtool_ops);

    return fmvj18x_config(link);
} 



static void fmvj18x_detach(struct pcmcia_device *link)
{
    struct net_device *dev = link->priv;

    DEBUG(0, "fmvj18x_detach(0x%p)\n", link);

    if (link->dev_node)
	unregister_netdev(dev);

    fmvj18x_release(link);

    free_netdev(dev);
} 



#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int mfc_try_io_port(struct pcmcia_device *link)
{
    int i, ret;
    static const unsigned int serial_base[5] =
	{ 0x3f8, 0x2f8, 0x3e8, 0x2e8, 0x0 };

    for (i = 0; i < 5; i++) {
	link->io.BasePort2 = serial_base[i];
	link->io.Attributes2 = IO_DATA_PATH_WIDTH_8;
	if (link->io.BasePort2 == 0) {
	    link->io.NumPorts2 = 0;
	    printk(KERN_NOTICE "fmvj18x_cs: out of resource for serial\n");
	}
	ret = pcmcia_request_io(link, &link->io);
	if (ret == 0)
		return ret;
    }
    return ret;
}

static int ungermann_try_io_port(struct pcmcia_device *link)
{
    int ret;
    unsigned int ioaddr;
    
    for (ioaddr = 0x300; ioaddr < 0x3e0; ioaddr += 0x20) {
	link->io.BasePort1 = ioaddr;
	ret = pcmcia_request_io(link, &link->io);
	if (ret == 0) {
	    
	    link->conf.ConfigIndex = 
		((link->io.BasePort1 & 0x0f0) >> 3) | 0x22;
	    return ret;
	}
    }
    return ret;	
}

static int fmvj18x_config(struct pcmcia_device *link)
{
    struct net_device *dev = link->priv;
    local_info_t *lp = netdev_priv(dev);
    tuple_t tuple;
    cisparse_t parse;
    u_short buf[32];
    int i, last_fn = 0, last_ret = 0, ret;
    unsigned int ioaddr;
    cardtype_t cardtype;
    char *card_name = "unknown";
    u_char *node_id;

    DEBUG(0, "fmvj18x_config(0x%p)\n", link);

    tuple.TupleData = (u_char *)buf;
    tuple.TupleDataMax = 64;
    tuple.TupleOffset = 0;
    tuple.DesiredTuple = CISTPL_FUNCE;
    tuple.TupleOffset = 0;
    if (pcmcia_get_first_tuple(link, &tuple) == 0) {
	
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link, &tuple));
	CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link, &tuple));
	CS_CHECK(ParseTuple, pcmcia_parse_tuple(&tuple, &parse));
	link->conf.ConfigIndex = parse.cftable_entry.index;
	switch (link->manf_id) {
	case MANFID_TDK:
	    cardtype = TDK;
	    if (link->card_id == PRODID_TDK_GN3410
			|| link->card_id == PRODID_TDK_NP9610
			|| link->card_id == PRODID_TDK_MN3200) {
		
		link->conf.ConfigBase = 0x800;
		link->conf.ConfigIndex = 0x47;
		link->io.NumPorts2 = 8;
	    }
	    break;
	case MANFID_NEC:
	    cardtype = NEC; 
	    link->conf.ConfigBase = 0x800;
	    link->conf.ConfigIndex = 0x47;
	    link->io.NumPorts2 = 8;
	    break;
	case MANFID_KME:
	    cardtype = KME; 
	    link->conf.ConfigBase = 0x800;
	    link->conf.ConfigIndex = 0x47;
	    link->io.NumPorts2 = 8;
	    break;
	case MANFID_CONTEC:
	    cardtype = CONTEC;
	    break;
	case MANFID_FUJITSU:
	    if (link->conf.ConfigBase == 0x0fe0)
		cardtype = MBH10302;
	    else if (link->card_id == PRODID_FUJITSU_MBH10302) 
                 
		cardtype = MBH10304;
	    else if (link->card_id == PRODID_FUJITSU_MBH10304)
		cardtype = MBH10304;
	    else
		cardtype = LA501;
	    break;
	default:
	    cardtype = MBH10304;
	}
    } else {
	
	switch (link->manf_id) {
	case MANFID_FUJITSU:
	    if (link->card_id == PRODID_FUJITSU_MBH10304) {
		cardtype = XXX10304;    
	        link->conf.ConfigIndex = 0x20;
	    } else {
		cardtype = MBH10302;    
		link->conf.ConfigIndex = 1;
	    }
	    break;
	case MANFID_UNGERMANN:
	    cardtype = UNGERMANN;
	    break;
	default:
	    cardtype = MBH10302;
	    link->conf.ConfigIndex = 1;
	}
    }

    if (link->io.NumPorts2 != 0) {
    	link->irq.Attributes =
		IRQ_TYPE_DYNAMIC_SHARING|IRQ_FIRST_SHARED|IRQ_HANDLE_PRESENT;
	ret = mfc_try_io_port(link);
	if (ret != 0) goto cs_failed;
    } else if (cardtype == UNGERMANN) {
	ret = ungermann_try_io_port(link);
	if (ret != 0) goto cs_failed;
    } else { 
	CS_CHECK(RequestIO, pcmcia_request_io(link, &link->io));
    }
    CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));
    CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));
    dev->irq = link->irq.AssignedIRQ;
    dev->base_addr = link->io.BasePort1;

    if (link->io.BasePort2 != 0) {
	ret = fmvj18x_setup_mfc(link);
	if (ret != 0) goto failed;
    }

    ioaddr = dev->base_addr;

    
    if (sram_config == 0) 
	outb(CONFIG0_RST, ioaddr + CONFIG_0);
    else
	outb(CONFIG0_RST_1, ioaddr + CONFIG_0);

    
    if (cardtype == MBH10302)
	outb(BANK_0, ioaddr + CONFIG_1);
    else
	outb(BANK_0U, ioaddr + CONFIG_1);
    
    
    switch (cardtype) {
    case MBH10304:
    case TDK:
    case LA501:
    case CONTEC:
    case NEC:
    case KME:
	tuple.DesiredTuple = CISTPL_FUNCE;
	tuple.TupleOffset = 0;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link, &tuple));
	tuple.TupleOffset = 0;
	CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link, &tuple));
	if (cardtype == MBH10304) {
	    
	    node_id = &(tuple.TupleData[5]);
	    card_name = "FMV-J182";
	} else {
	    while (tuple.TupleData[0] != CISTPL_FUNCE_LAN_NODE_ID ) {
		CS_CHECK(GetNextTuple, pcmcia_get_next_tuple(link, &tuple));
		CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link, &tuple));
	    }
	    node_id = &(tuple.TupleData[2]);
	    if( cardtype == TDK ) {
		card_name = "TDK LAK-CD021";
	    } else if( cardtype == LA501 ) {
		card_name = "LA501";
	    } else if( cardtype == NEC ) {
		card_name = "PK-UG-J001";
	    } else if( cardtype == KME ) {
		card_name = "Panasonic";
	    } else {
		card_name = "C-NET(PC)C";
	    }
	}
	
	for (i = 0; i < 6; i++)
	    dev->dev_addr[i] = node_id[i];
	break;
    case UNGERMANN:
	
	for (i = 0; i < 6; i++) 
	    dev->dev_addr[i] = inb(ioaddr + UNGERMANN_MAC_ID + i);
	card_name = "Access/CARD";
	break;
    case XXX10304:
	
	if (fmvj18x_get_hwinfo(link, tuple.TupleData) == -1) {
	    printk(KERN_NOTICE "fmvj18x_cs: unable to read hardware net address.\n");
	    goto failed;
	}
	for (i = 0 ; i < 6; i++) {
	    dev->dev_addr[i] = tuple.TupleData[i];
	}
	card_name = "FMV-J182";
	break;
    case MBH10302:
    default:
	
	for (i = 0; i < 6; i++) 
	    dev->dev_addr[i] = inb(ioaddr + MAC_ID + i);
	card_name = "FMV-J181";
	break;
    }

    lp->cardtype = cardtype;
    link->dev_node = &lp->node;
    SET_NETDEV_DEV(dev, &handle_to_dev(link));

    if (register_netdev(dev) != 0) {
	printk(KERN_NOTICE "fmvj18x_cs: register_netdev() failed\n");
	link->dev_node = NULL;
	goto failed;
    }

    strcpy(lp->node.dev_name, dev->name);

    
    printk(KERN_INFO "%s: %s, sram %s, port %#3lx, irq %d, "
	   "hw_addr %pM\n",
	   dev->name, card_name, sram_config == 0 ? "4K TX*2" : "8K TX*2", 
	   dev->base_addr, dev->irq, dev->dev_addr);

    return 0;
    
cs_failed:
    
    cs_error(link, last_fn, last_ret);
failed:
    fmvj18x_release(link);
    return -ENODEV;
} 


static int fmvj18x_get_hwinfo(struct pcmcia_device *link, u_char *node_id)
{
    win_req_t req;
    memreq_t mem;
    u_char __iomem *base;
    int i, j;

    
    req.Attributes = WIN_DATA_WIDTH_8|WIN_MEMORY_TYPE_AM|WIN_ENABLE;
    req.Base = 0; req.Size = 0;
    req.AccessSpeed = 0;
    i = pcmcia_request_window(&link, &req, &link->win);
    if (i != 0) {
	cs_error(link, RequestWindow, i);
	return -1;
    }

    base = ioremap(req.Base, req.Size);
    mem.Page = 0;
    mem.CardOffset = 0;
    pcmcia_map_mem_page(link->win, &mem);

     
    for (i = 0; i < 0x200; i++) {
	if (readb(base+i*2) == 0x22) {	
	    if (readb(base+(i-1)*2) == 0xff
	     && readb(base+(i+5)*2) == 0x04
	     && readb(base+(i+6)*2) == 0x06
	     && readb(base+(i+13)*2) == 0xff) 
		break;
	}
    }

    if (i != 0x200) {
	for (j = 0 ; j < 6; j++,i++) {
	    node_id[j] = readb(base+(i+7)*2);
	}
    }

    iounmap(base);
    j = pcmcia_release_window(link->win);
    if (j != 0)
	cs_error(link, ReleaseWindow, j);
    return (i != 0x200) ? 0 : -1;

} 


static int fmvj18x_setup_mfc(struct pcmcia_device *link)
{
    win_req_t req;
    memreq_t mem;
    int i;
    struct net_device *dev = link->priv;
    unsigned int ioaddr;
    local_info_t *lp = netdev_priv(dev);

    
    req.Attributes = WIN_DATA_WIDTH_8|WIN_MEMORY_TYPE_AM|WIN_ENABLE;
    req.Base = 0; req.Size = 0;
    req.AccessSpeed = 0;
    i = pcmcia_request_window(&link, &req, &link->win);
    if (i != 0) {
	cs_error(link, RequestWindow, i);
	return -1;
    }

    lp->base = ioremap(req.Base, req.Size);
    if (lp->base == NULL) {
	printk(KERN_NOTICE "fmvj18x_cs: ioremap failed\n");
	return -1;
    }

    mem.Page = 0;
    mem.CardOffset = 0;
    i = pcmcia_map_mem_page(link->win, &mem);
    if (i != 0) {
	iounmap(lp->base);
	lp->base = NULL;
	cs_error(link, MapMemPage, i);
	return -1;
    }
    
    ioaddr = dev->base_addr;
    writeb(0x47, lp->base+0x800);	
    writeb(0x0,  lp->base+0x802);	

    writeb(ioaddr & 0xff, lp->base+0x80a);	  
    writeb((ioaddr >> 8) & 0xff, lp->base+0x80c); 
   
    writeb(0x45, lp->base+0x820);	
    writeb(0x8,  lp->base+0x822);	

    return 0;

}


static void fmvj18x_release(struct pcmcia_device *link)
{

    struct net_device *dev = link->priv;
    local_info_t *lp = netdev_priv(dev);
    u_char __iomem *tmp;
    int j;

    DEBUG(0, "fmvj18x_release(0x%p)\n", link);

    if (lp->base != NULL) {
	tmp = lp->base;
	lp->base = NULL;    
	iounmap(tmp);
	j = pcmcia_release_window(link->win);
	if (j != 0)
	    cs_error(link, ReleaseWindow, j);
    }

    pcmcia_disable_device(link);

}

static int fmvj18x_suspend(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open)
		netif_device_detach(dev);

	return 0;
}

static int fmvj18x_resume(struct pcmcia_device *link)
{
	struct net_device *dev = link->priv;

	if (link->open) {
		fjn_reset(dev);
		netif_device_attach(dev);
	}

	return 0;
}



static struct pcmcia_device_id fmvj18x_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x0004, 0x0004),
	PCMCIA_DEVICE_PROD_ID12("EAGLE Technology", "NE200 ETHERNET LAN MBH10302 04", 0x528c88c4, 0x74f91e59),
	PCMCIA_DEVICE_PROD_ID12("Eiger Labs,Inc", "EPX-10BT PC Card Ethernet 10BT", 0x53af556e, 0x877f9922),
	PCMCIA_DEVICE_PROD_ID12("Eiger labs,Inc.", "EPX-10BT PC Card Ethernet 10BT", 0xf47e6c66, 0x877f9922),
	PCMCIA_DEVICE_PROD_ID12("FUJITSU", "LAN Card(FMV-J182)", 0x6ee5a3d8, 0x5baf31db),
	PCMCIA_DEVICE_PROD_ID12("FUJITSU", "MBH10308", 0x6ee5a3d8, 0x3f04875e),
	PCMCIA_DEVICE_PROD_ID12("FUJITSU TOWA", "LA501", 0xb8451188, 0x12939ba2),
	PCMCIA_DEVICE_PROD_ID12("HITACHI", "HT-4840-11", 0xf4f43949, 0x773910f4),
	PCMCIA_DEVICE_PROD_ID12("NextComK.K.", "NC5310B Ver1.0       ", 0x8cef4d3a, 0x075fc7b6),
	PCMCIA_DEVICE_PROD_ID12("NextComK.K.", "NC5310 Ver1.0        ", 0x8cef4d3a, 0xbccf43e6),
	PCMCIA_DEVICE_PROD_ID12("RATOC System Inc.", "10BASE_T CARD R280", 0x85c10e17, 0xd9413666),
	PCMCIA_DEVICE_PROD_ID12("TDK", "LAC-CD02x", 0x1eae9475, 0x8fa0ee70),
	PCMCIA_DEVICE_PROD_ID12("TDK", "LAC-CF010", 0x1eae9475, 0x7683bc9a),
	PCMCIA_DEVICE_PROD_ID1("CONTEC Co.,Ltd.", 0x58d8fee2),
	PCMCIA_DEVICE_PROD_ID1("PCMCIA LAN MBH10304  ES", 0x2599f454),
	PCMCIA_DEVICE_PROD_ID1("PCMCIA MBH10302", 0x8f4005da),
	PCMCIA_DEVICE_PROD_ID1("UBKK,V2.0", 0x90888080),
	PCMCIA_PFC_DEVICE_PROD_ID12(0, "TDK", "GlobalNetworker 3410/3412", 0x1eae9475, 0xd9a93bed),
	PCMCIA_PFC_DEVICE_PROD_ID12(0, "NEC", "PK-UG-J001" ,0x18df0ba0 ,0x831b1064),
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0105, 0x0d0a),
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0105, 0x0e0a),
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0032, 0x0a05),
	PCMCIA_PFC_DEVICE_MANF_CARD(0, 0x0032, 0x1101),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, fmvj18x_ids);

static struct pcmcia_driver fmvj18x_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "fmvj18x_cs",
	},
	.probe		= fmvj18x_probe,
	.remove		= fmvj18x_detach,
	.id_table       = fmvj18x_ids,
	.suspend	= fmvj18x_suspend,
	.resume		= fmvj18x_resume,
};

static int __init init_fmvj18x_cs(void)
{
	return pcmcia_register_driver(&fmvj18x_cs_driver);
}

static void __exit exit_fmvj18x_cs(void)
{
	pcmcia_unregister_driver(&fmvj18x_cs_driver);
}

module_init(init_fmvj18x_cs);
module_exit(exit_fmvj18x_cs);



static irqreturn_t fjn_interrupt(int dummy, void *dev_id)
{
    struct net_device *dev = dev_id;
    local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr;
    unsigned short tx_stat, rx_stat;

    ioaddr = dev->base_addr;

    
    outw(0x0000, ioaddr + TX_INTR);

    
    udelay(1);

    
    tx_stat = inb(ioaddr + TX_STATUS);
    rx_stat = inb(ioaddr + RX_STATUS);

    
    outb(tx_stat, ioaddr + TX_STATUS);
    outb(rx_stat, ioaddr + RX_STATUS);
    
    DEBUG(4, "%s: interrupt, rx_status %02x.\n", dev->name, rx_stat);
    DEBUG(4, "               tx_status %02x.\n", tx_stat);
    
    if (rx_stat || (inb(ioaddr + RX_MODE) & F_BUF_EMP) == 0) {
	
	fjn_rx(dev);
    }
    if (tx_stat & F_TMT_RDY) {
	dev->stats.tx_packets += lp->sent ;
        lp->sent = 0 ;
	if (lp->tx_queue) {
	    outb(DO_TX | lp->tx_queue, ioaddr + TX_START);
	    lp->sent = lp->tx_queue ;
	    lp->tx_queue = 0;
	    lp->tx_queue_len = 0;
	    dev->trans_start = jiffies;
	} else {
	    lp->tx_started = 0;
	}
	netif_wake_queue(dev);
    }
    DEBUG(4, "%s: exiting interrupt,\n", dev->name);
    DEBUG(4, "    tx_status %02x, rx_status %02x.\n", tx_stat, rx_stat);

    outb(D_TX_INTR, ioaddr + TX_INTR);
    outb(D_RX_INTR, ioaddr + RX_INTR);

    if (lp->base != NULL) {
	
	writeb(0x01, lp->base+0x802);
	writeb(0x09, lp->base+0x822);
    }

    return IRQ_HANDLED;

} 



static void fjn_tx_timeout(struct net_device *dev)
{
    struct local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;

    printk(KERN_NOTICE "%s: transmit timed out with status %04x, %s?\n",
	   dev->name, htons(inw(ioaddr + TX_STATUS)),
	   inb(ioaddr + TX_STATUS) & F_TMT_RDY
	   ? "IRQ conflict" : "network cable problem");
    printk(KERN_NOTICE "%s: timeout registers: %04x %04x %04x "
	   "%04x %04x %04x %04x %04x.\n",
	   dev->name, htons(inw(ioaddr + 0)),
	   htons(inw(ioaddr + 2)), htons(inw(ioaddr + 4)),
	   htons(inw(ioaddr + 6)), htons(inw(ioaddr + 8)),
	   htons(inw(ioaddr +10)), htons(inw(ioaddr +12)),
	   htons(inw(ioaddr +14)));
    dev->stats.tx_errors++;
    
    local_irq_disable();
    fjn_reset(dev);

    lp->tx_started = 0;
    lp->tx_queue = 0;
    lp->tx_queue_len = 0;
    lp->sent = 0;
    lp->open_time = jiffies;
    local_irq_enable();
    netif_wake_queue(dev);
}

static netdev_tx_t fjn_start_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
    struct local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    short length = skb->len;
    
    if (length < ETH_ZLEN)
    {
    	if (skb_padto(skb, ETH_ZLEN))
    		return NETDEV_TX_OK;
    	length = ETH_ZLEN;
    }

    netif_stop_queue(dev);

    {
	unsigned char *buf = skb->data;

	if (length > ETH_FRAME_LEN) {
	    printk(KERN_NOTICE "%s: Attempting to send a large packet"
		   " (%d bytes).\n", dev->name, length);
	    return NETDEV_TX_BUSY;
	}

	DEBUG(4, "%s: Transmitting a packet of length %lu.\n",
	      dev->name, (unsigned long)skb->len);
	dev->stats.tx_bytes += skb->len;

	
	outw(0x0000, ioaddr + TX_INTR);

	
	udelay(1);

	outw(length, ioaddr + DATAPORT);
	outsw(ioaddr + DATAPORT, buf, (length + 1) >> 1);

	lp->tx_queue++;
	lp->tx_queue_len += ((length+3) & ~1);

	if (lp->tx_started == 0) {
	    
	    outb(DO_TX | lp->tx_queue, ioaddr + TX_START);
	    lp->sent = lp->tx_queue ;
	    lp->tx_queue = 0;
	    lp->tx_queue_len = 0;
	    dev->trans_start = jiffies;
	    lp->tx_started = 1;
	    netif_start_queue(dev);
	} else {
	    if( sram_config == 0 ) {
		if (lp->tx_queue_len < (4096 - (ETH_FRAME_LEN +2)) )
		    
		    netif_start_queue(dev);
	    } else {
		if (lp->tx_queue_len < (8192 - (ETH_FRAME_LEN +2)) && 
						lp->tx_queue < 127 )
		    
		    netif_start_queue(dev);
	    }
	}

	
	outb(D_TX_INTR, ioaddr + TX_INTR);
	outb(D_RX_INTR, ioaddr + RX_INTR);
    }
    dev_kfree_skb (skb);

    return NETDEV_TX_OK;
} 



static void fjn_reset(struct net_device *dev)
{
    struct local_info_t *lp = netdev_priv(dev);
    unsigned int ioaddr = dev->base_addr;
    int i;

    DEBUG(4, "fjn_reset(%s) called.\n",dev->name);

    
    if( sram_config == 0 ) 
	outb(CONFIG0_RST, ioaddr + CONFIG_0);
    else
	outb(CONFIG0_RST_1, ioaddr + CONFIG_0);

    
    if (lp->cardtype == MBH10302)
	outb(BANK_0, ioaddr + CONFIG_1);
    else
	outb(BANK_0U, ioaddr + CONFIG_1);

    
    outb(D_TX_MODE, ioaddr + TX_MODE);
    
    outb(ID_MATCHED, ioaddr + RX_MODE);

    
    for (i = 0; i < 6; i++) 
        outb(dev->dev_addr[i], ioaddr + NODE_ID + i);

    
    set_rx_mode(dev);

    
    if (lp->cardtype == MBH10302)
	outb(BANK_2, ioaddr + CONFIG_1);
    else
	outb(BANK_2U, ioaddr + CONFIG_1);

    
    if( lp->cardtype == TDK || lp->cardtype == CONTEC) 
        outb(TDK_AUTO_MODE, ioaddr + COL_CTRL);
    else
        outb(AUTO_MODE, ioaddr + COL_CTRL);

    
    outb(0x00, ioaddr + BMPR12);
    outb(0x00, ioaddr + BMPR13);

    
    outb(0x01, ioaddr + RX_SKIP);

    
    if( sram_config == 0 )
	outb(CONFIG0_DFL, ioaddr + CONFIG_0);
    else
	outb(CONFIG0_DFL_1, ioaddr + CONFIG_0);

    
    inw(ioaddr + DATAPORT);
    inw(ioaddr + DATAPORT);

    
    outb(0xff, ioaddr + TX_STATUS);
    outb(0xff, ioaddr + RX_STATUS);

    if (lp->cardtype == MBH10302)
	outb(INTR_OFF, ioaddr + LAN_CTRL);

    
    outb(D_TX_INTR, ioaddr + TX_INTR);
    outb(D_RX_INTR, ioaddr + RX_INTR);

    
    if (lp->cardtype == MBH10302)
	outb(INTR_ON, ioaddr + LAN_CTRL);
} 



static void fjn_rx(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;
    int boguscount = 10;	

    DEBUG(4, "%s: in rx_packet(), rx_status %02x.\n",
	  dev->name, inb(ioaddr + RX_STATUS));

    while ((inb(ioaddr + RX_MODE) & F_BUF_EMP) == 0) {
	u_short status = inw(ioaddr + DATAPORT);

	DEBUG(4, "%s: Rxing packet mode %02x status %04x.\n",
	      dev->name, inb(ioaddr + RX_MODE), status);
#ifndef final_version
	if (status == 0) {
	    outb(F_SKP_PKT, ioaddr + RX_SKIP);
	    break;
	}
#endif
	if ((status & 0xF0) != 0x20) {	
	    dev->stats.rx_errors++;
	    if (status & F_LEN_ERR) dev->stats.rx_length_errors++;
	    if (status & F_ALG_ERR) dev->stats.rx_frame_errors++;
	    if (status & F_CRC_ERR) dev->stats.rx_crc_errors++;
	    if (status & F_OVR_FLO) dev->stats.rx_over_errors++;
	} else {
	    u_short pkt_len = inw(ioaddr + DATAPORT);
	    
	    struct sk_buff *skb;

	    if (pkt_len > 1550) {
		printk(KERN_NOTICE "%s: The FMV-18x claimed a very "
		       "large packet, size %d.\n", dev->name, pkt_len);
		outb(F_SKP_PKT, ioaddr + RX_SKIP);
		dev->stats.rx_errors++;
		break;
	    }
	    skb = dev_alloc_skb(pkt_len+2);
	    if (skb == NULL) {
		printk(KERN_NOTICE "%s: Memory squeeze, dropping "
		       "packet (len %d).\n", dev->name, pkt_len);
		outb(F_SKP_PKT, ioaddr + RX_SKIP);
		dev->stats.rx_dropped++;
		break;
	    }

	    skb_reserve(skb, 2);
	    insw(ioaddr + DATAPORT, skb_put(skb, pkt_len),
		 (pkt_len + 1) >> 1);
	    skb->protocol = eth_type_trans(skb, dev);

#ifdef PCMCIA_DEBUG
	    if (pc_debug > 5) {
		int i;
		printk(KERN_DEBUG "%s: Rxed packet of length %d: ",
		       dev->name, pkt_len);
		for (i = 0; i < 14; i++)
		    printk(" %02x", skb->data[i]);
		printk(".\n");
	    }
#endif

	    netif_rx(skb);
	    dev->stats.rx_packets++;
	    dev->stats.rx_bytes += pkt_len;
	}
	if (--boguscount <= 0)
	    break;
    }

    


    return;
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

static int fjn_config(struct net_device *dev, struct ifmap *map){
    return 0;
}

static int fjn_open(struct net_device *dev)
{
    struct local_info_t *lp = netdev_priv(dev);
    struct pcmcia_device *link = lp->p_dev;

    DEBUG(4, "fjn_open('%s').\n", dev->name);

    if (!pcmcia_dev_present(link))
	return -ENODEV;
    
    link->open++;
    
    fjn_reset(dev);
    
    lp->tx_started = 0;
    lp->tx_queue = 0;
    lp->tx_queue_len = 0;
    lp->open_time = jiffies;
    netif_start_queue(dev);
    
    return 0;
} 



static int fjn_close(struct net_device *dev)
{
    struct local_info_t *lp = netdev_priv(dev);
    struct pcmcia_device *link = lp->p_dev;
    unsigned int ioaddr = dev->base_addr;

    DEBUG(4, "fjn_close('%s').\n", dev->name);

    lp->open_time = 0;
    netif_stop_queue(dev);

    
    if( sram_config == 0 ) 
	outb(CONFIG0_RST ,ioaddr + CONFIG_0);
    else
	outb(CONFIG0_RST_1 ,ioaddr + CONFIG_0);

    

    
    outb(CHIP_OFF ,ioaddr + CONFIG_1);

    
    if (lp->cardtype == MBH10302)
	outb(INTR_OFF, ioaddr + LAN_CTRL);

    link->open--;

    return 0;
} 





static void set_rx_mode(struct net_device *dev)
{
    unsigned int ioaddr = dev->base_addr;
    u_char mc_filter[8];		 
    u_long flags;
    int i;
    
    int saved_bank;
    int saved_config_0 = inb(ioaddr + CONFIG_0);
     
    local_irq_save(flags); 

    
    if (sram_config == 0) 
	outb(CONFIG0_RST, ioaddr + CONFIG_0);
    else
	outb(CONFIG0_RST_1, ioaddr + CONFIG_0);

    if (dev->flags & IFF_PROMISC) {
	memset(mc_filter, 0xff, sizeof(mc_filter));
	outb(3, ioaddr + RX_MODE);	
    } else if (dev->mc_count > MC_FILTERBREAK
	       ||  (dev->flags & IFF_ALLMULTI)) {
	
	memset(mc_filter, 0xff, sizeof(mc_filter));
	outb(2, ioaddr + RX_MODE);	
    } else if (dev->mc_count == 0) {
	memset(mc_filter, 0x00, sizeof(mc_filter));
	outb(1, ioaddr + RX_MODE);	
    } else {
	struct dev_mc_list *mclist;

	memset(mc_filter, 0, sizeof(mc_filter));
	for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
	     i++, mclist = mclist->next) {
	    unsigned int bit =
	    	ether_crc_le(ETH_ALEN, mclist->dmi_addr) >> 26;
	    mc_filter[bit >> 3] |= (1 << (bit & 7));
	}
	outb(2, ioaddr + RX_MODE);	
    }

    
    saved_bank = inb(ioaddr + CONFIG_1);
    outb(0xe4, ioaddr + CONFIG_1);

    for (i = 0; i < 8; i++)
	outb(mc_filter[i], ioaddr + MAR_ADR + i);
    outb(saved_bank, ioaddr + CONFIG_1);

    outb(saved_config_0, ioaddr + CONFIG_0);

    local_irq_restore(flags);
}
