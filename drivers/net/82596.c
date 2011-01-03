


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/bitops.h>

#include <asm/io.h>
#include <asm/dma.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>

static char version[] __initdata =
	"82596.c $Revision: 1.5 $\n";

#define DRV_NAME	"82596"



#define DEB_INIT	0x0001
#define DEB_PROBE	0x0002
#define DEB_SERIOUS	0x0004
#define DEB_ERRORS	0x0008
#define DEB_MULTI	0x0010
#define DEB_TDR		0x0020
#define DEB_OPEN	0x0040
#define DEB_RESET	0x0080
#define DEB_ADDCMD	0x0100
#define DEB_STATUS	0x0200
#define DEB_STARTTX	0x0400
#define DEB_RXADDR	0x0800
#define DEB_TXADDR	0x1000
#define DEB_RXFRAME	0x2000
#define DEB_INTS	0x4000
#define DEB_STRUCT	0x8000
#define DEB_ANY		0xffff


#define DEB(x,y)	if (i596_debug & (x)) y


#if defined(CONFIG_MVME16x_NET) || defined(CONFIG_MVME16x_NET_MODULE)
#define ENABLE_MVME16x_NET
#endif
#if defined(CONFIG_BVME6000_NET) || defined(CONFIG_BVME6000_NET_MODULE)
#define ENABLE_BVME6000_NET
#endif
#if defined(CONFIG_APRICOT) || defined(CONFIG_APRICOT_MODULE)
#define ENABLE_APRICOT
#endif

#ifdef ENABLE_MVME16x_NET
#include <asm/mvme16xhw.h>
#endif
#ifdef ENABLE_BVME6000_NET
#include <asm/bvme6000hw.h>
#endif



#ifdef __mc68000__
#define WSWAPrfd(x)  ((struct i596_rfd *) (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPrbd(x)  ((struct i596_rbd *) (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPiscp(x) ((struct i596_iscp *)(((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPscb(x)  ((struct i596_scb *) (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPcmd(x)  ((struct i596_cmd *) (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPtbd(x)  ((struct i596_tbd *) (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define WSWAPchar(x) ((char *)            (((u32)(x)<<16) | ((((u32)(x)))>>16)))
#define ISCP_BUSY	0x00010000
#define MACH_IS_APRICOT	0
#else
#define WSWAPrfd(x)     ((struct i596_rfd *)((long)x))
#define WSWAPrbd(x)     ((struct i596_rbd *)((long)x))
#define WSWAPiscp(x)    ((struct i596_iscp *)((long)x))
#define WSWAPscb(x)     ((struct i596_scb *)((long)x))
#define WSWAPcmd(x)     ((struct i596_cmd *)((long)x))
#define WSWAPtbd(x)     ((struct i596_tbd *)((long)x))
#define WSWAPchar(x)    ((char *)((long)x))
#define ISCP_BUSY	0x0001
#define MACH_IS_APRICOT	1
#endif


#define PORT_RESET		0x00	
#define PORT_SELFTEST		0x01	
#define PORT_ALTSCP		0x02	
#define PORT_ALTDUMP		0x03	

static int i596_debug = (DEB_SERIOUS|DEB_PROBE);

MODULE_AUTHOR("Richard Hirst");
MODULE_DESCRIPTION("i82596 driver");
MODULE_LICENSE("GPL");

module_param(i596_debug, int, 0);
MODULE_PARM_DESC(i596_debug, "i82596 debug mask");



static int rx_copybreak = 100;

#define PKT_BUF_SZ	1536
#define MAX_MC_CNT	64

#define I596_TOTAL_SIZE 17

#define I596_NULL ((void *)0xffffffff)

#define CMD_EOL		0x8000	
#define CMD_SUSP	0x4000	
#define CMD_INTR	0x2000	

#define CMD_FLEX	0x0008	

enum commands {
	CmdNOp = 0, CmdSASetup = 1, CmdConfigure = 2, CmdMulticastList = 3,
	CmdTx = 4, CmdTDR = 5, CmdDump = 6, CmdDiagnose = 7
};

#define STAT_C		0x8000	
#define STAT_B		0x4000	
#define STAT_OK		0x2000	
#define STAT_A		0x1000	

#define	 CUC_START	0x0100
#define	 CUC_RESUME	0x0200
#define	 CUC_SUSPEND    0x0300
#define	 CUC_ABORT	0x0400
#define	 RX_START	0x0010
#define	 RX_RESUME	0x0020
#define	 RX_SUSPEND	0x0030
#define	 RX_ABORT	0x0040

#define TX_TIMEOUT	5


struct i596_reg {
	unsigned short porthi;
	unsigned short portlo;
	unsigned long ca;
};

#define EOF		0x8000
#define SIZE_MASK	0x3fff

struct i596_tbd {
	unsigned short size;
	unsigned short pad;
	struct i596_tbd *next;
	char *data;
};



struct i596_cmd {
	struct i596_cmd *v_next;	
	unsigned short status;
	unsigned short command;
	struct i596_cmd *b_next;	
};

struct tx_cmd {
	struct i596_cmd cmd;
	struct i596_tbd *tbd;
	unsigned short size;
	unsigned short pad;
	struct sk_buff *skb;	
};

struct tdr_cmd {
	struct i596_cmd cmd;
	unsigned short status;
	unsigned short pad;
};

struct mc_cmd {
	struct i596_cmd cmd;
	short mc_cnt;
	char mc_addrs[MAX_MC_CNT*6];
};

struct sa_cmd {
	struct i596_cmd cmd;
	char eth_addr[8];
};

struct cf_cmd {
	struct i596_cmd cmd;
	char i596_config[16];
};

struct i596_rfd {
	unsigned short stat;
	unsigned short cmd;
	struct i596_rfd *b_next;	
	struct i596_rbd *rbd;
	unsigned short count;
	unsigned short size;
	struct i596_rfd *v_next;	
	struct i596_rfd *v_prev;
};

struct i596_rbd {
    unsigned short count;
    unsigned short zero1;
    struct i596_rbd *b_next;
    unsigned char *b_data;		
    unsigned short size;
    unsigned short zero2;
    struct sk_buff *skb;
    struct i596_rbd *v_next;
    struct i596_rbd *b_addr;		
    unsigned char *v_data;		
};

#define TX_RING_SIZE 64
#define RX_RING_SIZE 16

struct i596_scb {
	unsigned short status;
	unsigned short command;
	struct i596_cmd *cmd;
	struct i596_rfd *rfd;
	unsigned long crc_err;
	unsigned long align_err;
	unsigned long resource_err;
	unsigned long over_err;
	unsigned long rcvdt_err;
	unsigned long short_err;
	unsigned short t_on;
	unsigned short t_off;
};

struct i596_iscp {
	unsigned long stat;
	struct i596_scb *scb;
};

struct i596_scp {
	unsigned long sysbus;
	unsigned long pad;
	struct i596_iscp *iscp;
};

struct i596_private {
	volatile struct i596_scp scp;
	volatile struct i596_iscp iscp;
	volatile struct i596_scb scb;
	struct sa_cmd sa_cmd;
	struct cf_cmd cf_cmd;
	struct tdr_cmd tdr_cmd;
	struct mc_cmd mc_cmd;
	unsigned long stat;
	int last_restart __attribute__((aligned(4)));
	struct i596_rfd *rfd_head;
	struct i596_rbd *rbd_head;
	struct i596_cmd *cmd_tail;
	struct i596_cmd *cmd_head;
	int cmd_backlog;
	unsigned long last_cmd;
	struct i596_rfd rfds[RX_RING_SIZE];
	struct i596_rbd rbds[RX_RING_SIZE];
	struct tx_cmd tx_cmds[TX_RING_SIZE];
	struct i596_tbd tbds[TX_RING_SIZE];
	int next_tx_cmd;
	spinlock_t lock;
};

static char init_setup[] =
{
	0x8E,			
	0xC8,			
#ifdef CONFIG_VME
	0xc0,			
#else
	0x80,			
#endif
	0x2E,			
	0x00,			
	0x60,			
	0x00,			
	0xf2,			
	0x00,			
	0x00,			
	0x40,			
	0xff,
	0x00,
	0x7f  };

static int i596_open(struct net_device *dev);
static netdev_tx_t i596_start_xmit(struct sk_buff *skb, struct net_device *dev);
static irqreturn_t i596_interrupt(int irq, void *dev_id);
static int i596_close(struct net_device *dev);
static void i596_add_cmd(struct net_device *dev, struct i596_cmd *cmd);
static void i596_tx_timeout (struct net_device *dev);
static void print_eth(unsigned char *buf, char *str);
static void set_multicast_list(struct net_device *dev);

static int rx_ring_size = RX_RING_SIZE;
static int ticks_limit = 25;
static int max_cmd_backlog = TX_RING_SIZE-1;


static inline void CA(struct net_device *dev)
{
#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		((struct i596_reg *) dev->base_addr)->ca = 1;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile u32 i;

		i = *(volatile u32 *) (dev->base_addr);
	}
#endif
#ifdef ENABLE_APRICOT
	if (MACH_IS_APRICOT) {
		outw(0, (short) (dev->base_addr) + 4);
	}
#endif
}


static inline void MPU_PORT(struct net_device *dev, int c, volatile void *x)
{
#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		struct i596_reg *p = (struct i596_reg *) (dev->base_addr);
		p->porthi = ((c) | (u32) (x)) & 0xffff;
		p->portlo = ((c) | (u32) (x)) >> 16;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		u32 v = (u32) (c) | (u32) (x);
		v = ((u32) (v) << 16) | ((u32) (v) >> 16);
		*(volatile u32 *) dev->base_addr = v;
		udelay(1);
		*(volatile u32 *) dev->base_addr = v;
	}
#endif
}


static inline int wait_istat(struct net_device *dev, struct i596_private *lp, int delcnt, char *str)
{
	while (--delcnt && lp->iscp.stat)
		udelay(10);
	if (!delcnt) {
		printk(KERN_ERR "%s: %s, status %4.4x, cmd %4.4x.\n",
		     dev->name, str, lp->scb.status, lp->scb.command);
		return -1;
	}
	else
		return 0;
}


static inline int wait_cmd(struct net_device *dev, struct i596_private *lp, int delcnt, char *str)
{
	while (--delcnt && lp->scb.command)
		udelay(10);
	if (!delcnt) {
		printk(KERN_ERR "%s: %s, status %4.4x, cmd %4.4x.\n",
		     dev->name, str, lp->scb.status, lp->scb.command);
		return -1;
	}
	else
		return 0;
}


static inline int wait_cfg(struct net_device *dev, struct i596_cmd *cmd, int delcnt, char *str)
{
	volatile struct i596_cmd *c = cmd;

	while (--delcnt && c->command)
		udelay(10);
	if (!delcnt) {
		printk(KERN_ERR "%s: %s.\n", dev->name, str);
		return -1;
	}
	else
		return 0;
}


static void i596_display_data(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	struct i596_cmd *cmd;
	struct i596_rfd *rfd;
	struct i596_rbd *rbd;

	printk(KERN_ERR "lp and scp at %p, .sysbus = %08lx, .iscp = %p\n",
	       &lp->scp, lp->scp.sysbus, lp->scp.iscp);
	printk(KERN_ERR "iscp at %p, iscp.stat = %08lx, .scb = %p\n",
	       &lp->iscp, lp->iscp.stat, lp->iscp.scb);
	printk(KERN_ERR "scb at %p, scb.status = %04x, .command = %04x,"
		" .cmd = %p, .rfd = %p\n",
	       &lp->scb, lp->scb.status, lp->scb.command,
		lp->scb.cmd, lp->scb.rfd);
	printk(KERN_ERR "   errors: crc %lx, align %lx, resource %lx,"
               " over %lx, rcvdt %lx, short %lx\n",
		lp->scb.crc_err, lp->scb.align_err, lp->scb.resource_err,
		lp->scb.over_err, lp->scb.rcvdt_err, lp->scb.short_err);
	cmd = lp->cmd_head;
	while (cmd != I596_NULL) {
		printk(KERN_ERR "cmd at %p, .status = %04x, .command = %04x, .b_next = %p\n",
		  cmd, cmd->status, cmd->command, cmd->b_next);
		cmd = cmd->v_next;
	}
	rfd = lp->rfd_head;
	printk(KERN_ERR "rfd_head = %p\n", rfd);
	do {
		printk(KERN_ERR "   %p .stat %04x, .cmd %04x, b_next %p, rbd %p,"
                        " count %04x\n",
			rfd, rfd->stat, rfd->cmd, rfd->b_next, rfd->rbd,
			rfd->count);
		rfd = rfd->v_next;
	} while (rfd != lp->rfd_head);
	rbd = lp->rbd_head;
	printk(KERN_ERR "rbd_head = %p\n", rbd);
	do {
		printk(KERN_ERR "   %p .count %04x, b_next %p, b_data %p, size %04x\n",
			rbd, rbd->count, rbd->b_next, rbd->b_data, rbd->size);
		rbd = rbd->v_next;
	} while (rbd != lp->rbd_head);
}


#if defined(ENABLE_MVME16x_NET) || defined(ENABLE_BVME6000_NET)
static irqreturn_t i596_error(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		volatile unsigned char *pcc2 = (unsigned char *) 0xfff42000;

		pcc2[0x28] = 1;
		pcc2[0x2b] = 0x1d;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *ethirq = (unsigned char *) BVME_ETHIRQ_REG;

		*ethirq = 1;
		*ethirq = 3;
	}
#endif
	printk(KERN_ERR "%s: Error interrupt\n", dev->name);
	i596_display_data(dev);
	return IRQ_HANDLED;
}
#endif

static inline void init_rx_bufs(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	int i;
	struct i596_rfd *rfd;
	struct i596_rbd *rbd;

	

	for (i = 0, rbd = lp->rbds; i < rx_ring_size; i++, rbd++) {
		struct sk_buff *skb = dev_alloc_skb(PKT_BUF_SZ);

		if (skb == NULL)
			panic("82596: alloc_skb() failed");
		skb->dev = dev;
		rbd->v_next = rbd+1;
		rbd->b_next = WSWAPrbd(virt_to_bus(rbd+1));
		rbd->b_addr = WSWAPrbd(virt_to_bus(rbd));
		rbd->skb = skb;
		rbd->v_data = skb->data;
		rbd->b_data = WSWAPchar(virt_to_bus(skb->data));
		rbd->size = PKT_BUF_SZ;
#ifdef __mc68000__
		cache_clear(virt_to_phys(skb->data), PKT_BUF_SZ);
#endif
	}
	lp->rbd_head = lp->rbds;
	rbd = lp->rbds + rx_ring_size - 1;
	rbd->v_next = lp->rbds;
	rbd->b_next = WSWAPrbd(virt_to_bus(lp->rbds));

	

	for (i = 0, rfd = lp->rfds; i < rx_ring_size; i++, rfd++) {
		rfd->rbd = I596_NULL;
		rfd->v_next = rfd+1;
		rfd->v_prev = rfd-1;
		rfd->b_next = WSWAPrfd(virt_to_bus(rfd+1));
		rfd->cmd = CMD_FLEX;
	}
	lp->rfd_head = lp->rfds;
	lp->scb.rfd = WSWAPrfd(virt_to_bus(lp->rfds));
	rfd = lp->rfds;
	rfd->rbd = lp->rbd_head;
	rfd->v_prev = lp->rfds + rx_ring_size - 1;
	rfd = lp->rfds + rx_ring_size - 1;
	rfd->v_next = lp->rfds;
	rfd->b_next = WSWAPrfd(virt_to_bus(lp->rfds));
	rfd->cmd = CMD_EOL|CMD_FLEX;
}

static inline void remove_rx_bufs(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	struct i596_rbd *rbd;
	int i;

	for (i = 0, rbd = lp->rbds; i < rx_ring_size; i++, rbd++) {
		if (rbd->skb == NULL)
			break;
		dev_kfree_skb(rbd->skb);
	}
}


static void rebuild_rx_bufs(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	int i;

	

	for (i = 0; i < rx_ring_size; i++) {
		lp->rfds[i].rbd = I596_NULL;
		lp->rfds[i].cmd = CMD_FLEX;
	}
	lp->rfds[rx_ring_size-1].cmd = CMD_EOL|CMD_FLEX;
	lp->rfd_head = lp->rfds;
	lp->scb.rfd = WSWAPrfd(virt_to_bus(lp->rfds));
	lp->rbd_head = lp->rbds;
	lp->rfds[0].rbd = WSWAPrbd(virt_to_bus(lp->rbds));
}


static int init_i596_mem(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
#if !defined(ENABLE_MVME16x_NET) && !defined(ENABLE_BVME6000_NET) || defined(ENABLE_APRICOT)
	short ioaddr = dev->base_addr;
#endif
	unsigned long flags;

	MPU_PORT(dev, PORT_RESET, NULL);

	udelay(100);		

#if defined(ENABLE_MVME16x_NET) || defined(ENABLE_BVME6000_NET)
#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		volatile unsigned char *pcc2 = (unsigned char *) 0xfff42000;

		
		pcc2[0x28] = 1;
		pcc2[0x2a] = 0x48;
		
		pcc2[0x2b] = 0x08;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *ethirq = (unsigned char *) BVME_ETHIRQ_REG;

		*ethirq = 1;
	}
#endif

	

	MPU_PORT(dev, PORT_ALTSCP, (void *)virt_to_bus((void *)&lp->scp));

#elif defined(ENABLE_APRICOT)

	{
		u32 scp = virt_to_bus(&lp->scp);

		
		outw(0, ioaddr);
		outw(0, ioaddr);
		outb(4, ioaddr + 0xf);
		outw(scp | 2, ioaddr);
		outw(scp >> 16, ioaddr);
	}
#endif

	lp->last_cmd = jiffies;

#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x)
		lp->scp.sysbus = 0x00000054;
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000)
		lp->scp.sysbus = 0x0000004c;
#endif
#ifdef ENABLE_APRICOT
	if (MACH_IS_APRICOT)
		lp->scp.sysbus = 0x00440000;
#endif

	lp->scp.iscp = WSWAPiscp(virt_to_bus((void *)&lp->iscp));
	lp->iscp.scb = WSWAPscb(virt_to_bus((void *)&lp->scb));
	lp->iscp.stat = ISCP_BUSY;
	lp->cmd_backlog = 0;

	lp->cmd_head = lp->scb.cmd = I596_NULL;

#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		lp->scb.t_on  = 7 * 25;
		lp->scb.t_off = 1 * 25;
	}
#endif

	DEB(DEB_INIT,printk(KERN_DEBUG "%s: starting i82596.\n", dev->name));

#if defined(ENABLE_APRICOT)
	(void) inb(ioaddr + 0x10);
	outb(4, ioaddr + 0xf);
#endif
	CA(dev);

	if (wait_istat(dev,lp,1000,"initialization timed out"))
		goto failed;
	DEB(DEB_INIT,printk(KERN_DEBUG "%s: i82596 initialization successful\n", dev->name));

	
	rebuild_rx_bufs(dev);
	lp->scb.command = 0;

#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		volatile unsigned char *pcc2 = (unsigned char *) 0xfff42000;

		
		pcc2[0x2a] = 0x55;	
		pcc2[0x2b] = 0x15;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *ethirq = (unsigned char *) BVME_ETHIRQ_REG;

		*ethirq = 3;
	}
#endif


	DEB(DEB_INIT,printk(KERN_DEBUG "%s: queuing CmdConfigure\n", dev->name));
	memcpy(lp->cf_cmd.i596_config, init_setup, 14);
	lp->cf_cmd.cmd.command = CmdConfigure;
	i596_add_cmd(dev, &lp->cf_cmd.cmd);

	DEB(DEB_INIT,printk(KERN_DEBUG "%s: queuing CmdSASetup\n", dev->name));
	memcpy(lp->sa_cmd.eth_addr, dev->dev_addr, 6);
	lp->sa_cmd.cmd.command = CmdSASetup;
	i596_add_cmd(dev, &lp->sa_cmd.cmd);

	DEB(DEB_INIT,printk(KERN_DEBUG "%s: queuing CmdTDR\n", dev->name));
	lp->tdr_cmd.cmd.command = CmdTDR;
	i596_add_cmd(dev, &lp->tdr_cmd.cmd);

	spin_lock_irqsave (&lp->lock, flags);

	if (wait_cmd(dev,lp,1000,"timed out waiting to issue RX_START")) {
		spin_unlock_irqrestore (&lp->lock, flags);
		goto failed;
	}
	DEB(DEB_INIT,printk(KERN_DEBUG "%s: Issuing RX_START\n", dev->name));
	lp->scb.command = RX_START;
	CA(dev);

	spin_unlock_irqrestore (&lp->lock, flags);

	if (wait_cmd(dev,lp,1000,"RX_START not processed"))
		goto failed;
	DEB(DEB_INIT,printk(KERN_DEBUG "%s: Receive unit started OK\n", dev->name));
	return 0;

failed:
	printk(KERN_CRIT "%s: Failed to initialise 82596\n", dev->name);
	MPU_PORT(dev, PORT_RESET, NULL);
	return -1;
}

static inline int i596_rx(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	struct i596_rfd *rfd;
	struct i596_rbd *rbd;
	int frames = 0;

	DEB(DEB_RXFRAME,printk(KERN_DEBUG "i596_rx(), rfd_head %p, rbd_head %p\n",
			lp->rfd_head, lp->rbd_head));

	rfd = lp->rfd_head;		

	while ((rfd->stat) & STAT_C) {	
		if (rfd->rbd == I596_NULL)
			rbd = I596_NULL;
		else if (rfd->rbd == lp->rbd_head->b_addr)
			rbd = lp->rbd_head;
		else {
			printk(KERN_CRIT "%s: rbd chain broken!\n", dev->name);
			
			rbd = I596_NULL;
		}
		DEB(DEB_RXFRAME, printk(KERN_DEBUG "  rfd %p, rfd.rbd %p, rfd.stat %04x\n",
			rfd, rfd->rbd, rfd->stat));

		if (rbd != I596_NULL && ((rfd->stat) & STAT_OK)) {
			
			int pkt_len = rbd->count & 0x3fff;
			struct sk_buff *skb = rbd->skb;
			int rx_in_place = 0;

			DEB(DEB_RXADDR,print_eth(rbd->v_data, "received"));
			frames++;

			

			if (pkt_len > rx_copybreak) {
				struct sk_buff *newskb;

				
				newskb = dev_alloc_skb(PKT_BUF_SZ);
				if (newskb == NULL) {
					skb = NULL;	
					goto memory_squeeze;
				}
				
				skb_put(skb, pkt_len);
				rx_in_place = 1;
				rbd->skb = newskb;
				newskb->dev = dev;
				rbd->v_data = newskb->data;
				rbd->b_data = WSWAPchar(virt_to_bus(newskb->data));
#ifdef __mc68000__
				cache_clear(virt_to_phys(newskb->data), PKT_BUF_SZ);
#endif
			}
			else
				skb = dev_alloc_skb(pkt_len + 2);
memory_squeeze:
			if (skb == NULL) {
				
				printk(KERN_WARNING "%s: i596_rx Memory squeeze, dropping packet.\n", dev->name);
				dev->stats.rx_dropped++;
			}
			else {
				if (!rx_in_place) {
					
					skb_reserve(skb, 2);
					memcpy(skb_put(skb,pkt_len), rbd->v_data, pkt_len);
				}
				skb->protocol=eth_type_trans(skb,dev);
				skb->len = pkt_len;
#ifdef __mc68000__
				cache_clear(virt_to_phys(rbd->skb->data),
						pkt_len);
#endif
				netif_rx(skb);
				dev->stats.rx_packets++;
				dev->stats.rx_bytes+=pkt_len;
			}
		}
		else {
			DEB(DEB_ERRORS, printk(KERN_DEBUG "%s: Error, rfd.stat = 0x%04x\n",
					dev->name, rfd->stat));
			dev->stats.rx_errors++;
			if ((rfd->stat) & 0x0001)
				dev->stats.collisions++;
			if ((rfd->stat) & 0x0080)
				dev->stats.rx_length_errors++;
			if ((rfd->stat) & 0x0100)
				dev->stats.rx_over_errors++;
			if ((rfd->stat) & 0x0200)
				dev->stats.rx_fifo_errors++;
			if ((rfd->stat) & 0x0400)
				dev->stats.rx_frame_errors++;
			if ((rfd->stat) & 0x0800)
				dev->stats.rx_crc_errors++;
			if ((rfd->stat) & 0x1000)
				dev->stats.rx_length_errors++;
		}

		

		if (rbd != I596_NULL && (rbd->count & 0x4000)) {
			rbd->count = 0;
			lp->rbd_head = rbd->v_next;
		}

		

		rfd->rbd = I596_NULL;
		rfd->stat = 0;
		rfd->cmd = CMD_EOL|CMD_FLEX;
		rfd->count = 0;

		

		rfd->v_prev->cmd = CMD_FLEX;

		

		lp->scb.rfd = rfd->b_next;
		lp->rfd_head = rfd->v_next;
		rfd = lp->rfd_head;
	}

	DEB(DEB_RXFRAME,printk(KERN_DEBUG "frames %d\n", frames));

	return 0;
}


static void i596_cleanup_cmd(struct net_device *dev, struct i596_private *lp)
{
	struct i596_cmd *ptr;

	while (lp->cmd_head != I596_NULL) {
		ptr = lp->cmd_head;
		lp->cmd_head = ptr->v_next;
		lp->cmd_backlog--;

		switch ((ptr->command) & 0x7) {
		case CmdTx:
			{
				struct tx_cmd *tx_cmd = (struct tx_cmd *) ptr;
				struct sk_buff *skb = tx_cmd->skb;

				dev_kfree_skb(skb);

				dev->stats.tx_errors++;
				dev->stats.tx_aborted_errors++;

				ptr->v_next = ptr->b_next = I596_NULL;
				tx_cmd->cmd.command = 0;  
				break;
			}
		default:
			ptr->v_next = ptr->b_next = I596_NULL;
		}
	}

	wait_cmd(dev,lp,100,"i596_cleanup_cmd timed out");
	lp->scb.cmd = I596_NULL;
}

static void i596_reset(struct net_device *dev, struct i596_private *lp,
			int ioaddr)
{
	unsigned long flags;

	DEB(DEB_RESET,printk(KERN_DEBUG "i596_reset\n"));

	spin_lock_irqsave (&lp->lock, flags);

	wait_cmd(dev,lp,100,"i596_reset timed out");

	netif_stop_queue(dev);

	lp->scb.command = CUC_ABORT | RX_ABORT;
	CA(dev);

	
	wait_cmd(dev,lp,1000,"i596_reset 2 timed out");
	spin_unlock_irqrestore (&lp->lock, flags);

	i596_cleanup_cmd(dev,lp);
	i596_rx(dev);

	netif_start_queue(dev);
	init_i596_mem(dev);
}

static void i596_add_cmd(struct net_device *dev, struct i596_cmd *cmd)
{
	struct i596_private *lp = dev->ml_priv;
	int ioaddr = dev->base_addr;
	unsigned long flags;

	DEB(DEB_ADDCMD,printk(KERN_DEBUG "i596_add_cmd\n"));

	cmd->status = 0;
	cmd->command |= (CMD_EOL | CMD_INTR);
	cmd->v_next = cmd->b_next = I596_NULL;

	spin_lock_irqsave (&lp->lock, flags);

	if (lp->cmd_head != I596_NULL) {
		lp->cmd_tail->v_next = cmd;
		lp->cmd_tail->b_next = WSWAPcmd(virt_to_bus(&cmd->status));
	} else {
		lp->cmd_head = cmd;
		wait_cmd(dev,lp,100,"i596_add_cmd timed out");
		lp->scb.cmd = WSWAPcmd(virt_to_bus(&cmd->status));
		lp->scb.command = CUC_START;
		CA(dev);
	}
	lp->cmd_tail = cmd;
	lp->cmd_backlog++;

	spin_unlock_irqrestore (&lp->lock, flags);

	if (lp->cmd_backlog > max_cmd_backlog) {
		unsigned long tickssofar = jiffies - lp->last_cmd;

		if (tickssofar < ticks_limit)
			return;

		printk(KERN_NOTICE "%s: command unit timed out, status resetting.\n", dev->name);

		i596_reset(dev, lp, ioaddr);
	}
}

static int i596_open(struct net_device *dev)
{
	int res = 0;

	DEB(DEB_OPEN,printk(KERN_DEBUG "%s: i596_open() irq %d.\n", dev->name, dev->irq));

	if (request_irq(dev->irq, i596_interrupt, 0, "i82596", dev)) {
		printk(KERN_ERR "%s: IRQ %d not free\n", dev->name, dev->irq);
		return -EAGAIN;
	}
#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		if (request_irq(0x56, i596_error, 0, "i82596_error", dev))
			return -EAGAIN;
	}
#endif
	init_rx_bufs(dev);

	netif_start_queue(dev);

	
	if (init_i596_mem(dev)) {
		res = -EAGAIN;
		free_irq(dev->irq, dev);
	}

	return res;
}

static void i596_tx_timeout (struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	int ioaddr = dev->base_addr;

	
	DEB(DEB_ERRORS,printk(KERN_ERR "%s: transmit timed out, status resetting.\n",
			dev->name));

	dev->stats.tx_errors++;

	
	if (lp->last_restart == dev->stats.tx_packets) {
		DEB(DEB_ERRORS,printk(KERN_ERR "Resetting board.\n"));
		
		i596_reset (dev, lp, ioaddr);
	} else {
		
		DEB(DEB_ERRORS,printk(KERN_ERR "Kicking board.\n"));
		lp->scb.command = CUC_START | RX_START;
		CA (dev);
		lp->last_restart = dev->stats.tx_packets;
	}

	dev->trans_start = jiffies;
	netif_wake_queue (dev);
}

static netdev_tx_t i596_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	struct tx_cmd *tx_cmd;
	struct i596_tbd *tbd;
	short length = skb->len;
	dev->trans_start = jiffies;

	DEB(DEB_STARTTX,printk(KERN_DEBUG "%s: i596_start_xmit(%x,%p) called\n",
				dev->name, skb->len, skb->data));

	if (skb->len < ETH_ZLEN) {
		if (skb_padto(skb, ETH_ZLEN))
			return NETDEV_TX_OK;
		length = ETH_ZLEN;
	}
	netif_stop_queue(dev);

	tx_cmd = lp->tx_cmds + lp->next_tx_cmd;
	tbd = lp->tbds + lp->next_tx_cmd;

	if (tx_cmd->cmd.command) {
		printk(KERN_NOTICE "%s: xmit ring full, dropping packet.\n",
				dev->name);
		dev->stats.tx_dropped++;

		dev_kfree_skb(skb);
	} else {
		if (++lp->next_tx_cmd == TX_RING_SIZE)
			lp->next_tx_cmd = 0;
		tx_cmd->tbd = WSWAPtbd(virt_to_bus(tbd));
		tbd->next = I596_NULL;

		tx_cmd->cmd.command = CMD_FLEX | CmdTx;
		tx_cmd->skb = skb;

		tx_cmd->pad = 0;
		tx_cmd->size = 0;
		tbd->pad = 0;
		tbd->size = EOF | length;

		tbd->data = WSWAPchar(virt_to_bus(skb->data));

#ifdef __mc68000__
		cache_push(virt_to_phys(skb->data), length);
#endif
		DEB(DEB_TXADDR,print_eth(skb->data, "tx-queued"));
		i596_add_cmd(dev, &tx_cmd->cmd);

		dev->stats.tx_packets++;
		dev->stats.tx_bytes += length;
	}

	netif_start_queue(dev);

	return NETDEV_TX_OK;
}

static void print_eth(unsigned char *add, char *str)
{
	printk(KERN_DEBUG "i596 0x%p, %pM --> %pM %02X%02X, %s\n",
	       add, add + 6, add, add[12], add[13], str);
}

static int io = 0x300;
static int irq = 10;

static const struct net_device_ops i596_netdev_ops = {
	.ndo_open 		= i596_open,
	.ndo_stop		= i596_close,
	.ndo_start_xmit		= i596_start_xmit,
	.ndo_set_multicast_list = set_multicast_list,
	.ndo_tx_timeout		= i596_tx_timeout,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

struct net_device * __init i82596_probe(int unit)
{
	struct net_device *dev;
	int i;
	struct i596_private *lp;
	char eth_addr[8];
	static int probed;
	int err;

	if (probed)
		return ERR_PTR(-ENODEV);
	probed++;

	dev = alloc_etherdev(0);
	if (!dev)
		return ERR_PTR(-ENOMEM);

	if (unit >= 0) {
		sprintf(dev->name, "eth%d", unit);
		netdev_boot_setup_check(dev);
	} else {
		dev->base_addr = io;
		dev->irq = irq;
	}

#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		if (mvme16x_config & MVME16x_CONFIG_NO_ETHERNET) {
			printk(KERN_NOTICE "Ethernet probe disabled - chip not present\n");
			err = -ENODEV;
			goto out;
		}
		memcpy(eth_addr, (void *) 0xfffc1f2c, 6);	
		dev->base_addr = MVME_I596_BASE;
		dev->irq = (unsigned) MVME16x_IRQ_I596;
		goto found;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *rtc = (unsigned char *) BVME_RTC_BASE;
		unsigned char msr = rtc[3];
		int i;

		rtc[3] |= 0x80;
		for (i = 0; i < 6; i++)
			eth_addr[i] = rtc[i * 4 + 7];	
		rtc[3] = msr;
		dev->base_addr = BVME_I596_BASE;
		dev->irq = (unsigned) BVME_IRQ_I596;
		goto found;
	}
#endif
#ifdef ENABLE_APRICOT
	{
		int checksum = 0;
		int ioaddr = 0x300;

		
		

		if (!request_region(ioaddr, I596_TOTAL_SIZE, DRV_NAME)) {
			printk(KERN_ERR "82596: IO address 0x%04x in use\n", ioaddr);
			err = -EBUSY;
			goto out;
		}

		dev->base_addr = ioaddr;

		for (i = 0; i < 8; i++) {
			eth_addr[i] = inb(ioaddr + 8 + i);
			checksum += eth_addr[i];
		}

		

		if ((checksum % 0x100) ||
		    (memcmp(eth_addr, "\x00\x00\x49", 3) != 0)) {
			err = -ENODEV;
			goto out1;
		}

		dev->irq = 10;
		goto found;
	}
#endif
	err = -ENODEV;
	goto out;

found:
	dev->mem_start = (int)__get_free_pages(GFP_ATOMIC, 0);
	if (!dev->mem_start) {
		err = -ENOMEM;
		goto out1;
	}

	DEB(DEB_PROBE,printk(KERN_INFO "%s: 82596 at %#3lx,", dev->name, dev->base_addr));

	for (i = 0; i < 6; i++)
		DEB(DEB_PROBE,printk(" %2.2X", dev->dev_addr[i] = eth_addr[i]));

	DEB(DEB_PROBE,printk(" IRQ %d.\n", dev->irq));

	DEB(DEB_PROBE,printk(KERN_INFO "%s", version));

	
	dev->netdev_ops = &i596_netdev_ops;
	dev->watchdog_timeo = TX_TIMEOUT;

	dev->ml_priv = (void *)(dev->mem_start);

	lp = dev->ml_priv;
	DEB(DEB_INIT,printk(KERN_DEBUG "%s: lp at 0x%08lx (%zd bytes), "
			"lp->scb at 0x%08lx\n",
			dev->name, (unsigned long)lp,
			sizeof(struct i596_private), (unsigned long)&lp->scb));
	memset((void *) lp, 0, sizeof(struct i596_private));

#ifdef __mc68000__
	cache_push(virt_to_phys((void *)(dev->mem_start)), 4096);
	cache_clear(virt_to_phys((void *)(dev->mem_start)), 4096);
	kernel_set_cachemode((void *)(dev->mem_start), 4096, IOMAP_NOCACHE_SER);
#endif
	lp->scb.command = 0;
	lp->scb.cmd = I596_NULL;
	lp->scb.rfd = I596_NULL;
	spin_lock_init(&lp->lock);

	err = register_netdev(dev);
	if (err)
		goto out2;
	return dev;
out2:
#ifdef __mc68000__
	
	kernel_set_cachemode((void *)(dev->mem_start), 4096,
			IOMAP_FULL_CACHING);
#endif
	free_page ((u32)(dev->mem_start));
out1:
#ifdef ENABLE_APRICOT
	release_region(dev->base_addr, I596_TOTAL_SIZE);
#endif
out:
	free_netdev(dev);
	return ERR_PTR(err);
}

static irqreturn_t i596_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct i596_private *lp;
	short ioaddr;
	unsigned short status, ack_cmd = 0;
	int handled = 0;

#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		if (*(char *) BVME_LOCAL_IRQ_STAT & BVME_ETHERR) {
			i596_error(irq, dev_id);
			return IRQ_HANDLED;
		}
	}
#endif
	if (dev == NULL) {
		printk(KERN_ERR "i596_interrupt(): irq %d for unknown device.\n", irq);
		return IRQ_NONE;
	}

	ioaddr = dev->base_addr;
	lp = dev->ml_priv;

	spin_lock (&lp->lock);

	wait_cmd(dev,lp,100,"i596 interrupt, timeout");
	status = lp->scb.status;

	DEB(DEB_INTS,printk(KERN_DEBUG "%s: i596 interrupt, IRQ %d, status %4.4x.\n",
			dev->name, irq, status));

	ack_cmd = status & 0xf000;

	if ((status & 0x8000) || (status & 0x2000)) {
		struct i596_cmd *ptr;

		handled = 1;
		if ((status & 0x8000))
			DEB(DEB_INTS,printk(KERN_DEBUG "%s: i596 interrupt completed command.\n", dev->name));
		if ((status & 0x2000))
			DEB(DEB_INTS,printk(KERN_DEBUG "%s: i596 interrupt command unit inactive %x.\n", dev->name, status & 0x0700));

		while ((lp->cmd_head != I596_NULL) && (lp->cmd_head->status & STAT_C)) {
			ptr = lp->cmd_head;

			DEB(DEB_STATUS,printk(KERN_DEBUG "cmd_head->status = %04x, ->command = %04x\n",
				       lp->cmd_head->status, lp->cmd_head->command));
			lp->cmd_head = ptr->v_next;
			lp->cmd_backlog--;

			switch ((ptr->command) & 0x7) {
			case CmdTx:
			    {
				struct tx_cmd *tx_cmd = (struct tx_cmd *) ptr;
				struct sk_buff *skb = tx_cmd->skb;

				if ((ptr->status) & STAT_OK) {
					DEB(DEB_TXADDR,print_eth(skb->data, "tx-done"));
				} else {
					dev->stats.tx_errors++;
					if ((ptr->status) & 0x0020)
						dev->stats.collisions++;
					if (!((ptr->status) & 0x0040))
						dev->stats.tx_heartbeat_errors++;
					if ((ptr->status) & 0x0400)
						dev->stats.tx_carrier_errors++;
					if ((ptr->status) & 0x0800)
						dev->stats.collisions++;
					if ((ptr->status) & 0x1000)
						dev->stats.tx_aborted_errors++;
				}

				dev_kfree_skb_irq(skb);

				tx_cmd->cmd.command = 0; 
				break;
			    }
			case CmdTDR:
			    {
				unsigned short status = ((struct tdr_cmd *)ptr)->status;

				if (status & 0x8000) {
					DEB(DEB_TDR,printk(KERN_INFO "%s: link ok.\n", dev->name));
				} else {
					if (status & 0x4000)
						printk(KERN_ERR "%s: Transceiver problem.\n", dev->name);
					if (status & 0x2000)
						printk(KERN_ERR "%s: Termination problem.\n", dev->name);
					if (status & 0x1000)
						printk(KERN_ERR "%s: Short circuit.\n", dev->name);

					DEB(DEB_TDR,printk(KERN_INFO "%s: Time %d.\n", dev->name, status & 0x07ff));
				}
				break;
			    }
			case CmdConfigure:
			case CmdMulticastList:
				
				ptr->command = 0;
				break;
			}
			ptr->v_next = ptr->b_next = I596_NULL;
			lp->last_cmd = jiffies;
		}

		ptr = lp->cmd_head;
		while ((ptr != I596_NULL) && (ptr != lp->cmd_tail)) {
			ptr->command &= 0x1fff;
			ptr = ptr->v_next;
		}

		if ((lp->cmd_head != I596_NULL))
			ack_cmd |= CUC_START;
		lp->scb.cmd = WSWAPcmd(virt_to_bus(&lp->cmd_head->status));
	}
	if ((status & 0x1000) || (status & 0x4000)) {
		if ((status & 0x4000))
			DEB(DEB_INTS,printk(KERN_DEBUG "%s: i596 interrupt received a frame.\n", dev->name));
		i596_rx(dev);
		
		if (status & 0x1000) {
			if (netif_running(dev)) {
				DEB(DEB_ERRORS,printk(KERN_ERR "%s: i596 interrupt receive unit inactive, status 0x%x\n", dev->name, status));
				ack_cmd |= RX_START;
				dev->stats.rx_errors++;
				dev->stats.rx_fifo_errors++;
				rebuild_rx_bufs(dev);
			}
		}
	}
	wait_cmd(dev,lp,100,"i596 interrupt, timeout");
	lp->scb.command = ack_cmd;

#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		

		volatile unsigned char *pcc2 = (unsigned char *) 0xfff42000;

		pcc2[0x2a] |= 0x08;
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *ethirq = (unsigned char *) BVME_ETHIRQ_REG;

		*ethirq = 1;
		*ethirq = 3;
	}
#endif
#ifdef ENABLE_APRICOT
	(void) inb(ioaddr + 0x10);
	outb(4, ioaddr + 0xf);
#endif
	CA(dev);

	DEB(DEB_INTS,printk(KERN_DEBUG "%s: exiting interrupt.\n", dev->name));

	spin_unlock (&lp->lock);
	return IRQ_RETVAL(handled);
}

static int i596_close(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	unsigned long flags;

	netif_stop_queue(dev);

	DEB(DEB_INIT,printk(KERN_DEBUG "%s: Shutting down ethercard, status was %4.4x.\n",
		       dev->name, lp->scb.status));

	spin_lock_irqsave(&lp->lock, flags);

	wait_cmd(dev,lp,100,"close1 timed out");
	lp->scb.command = CUC_ABORT | RX_ABORT;
	CA(dev);

	wait_cmd(dev,lp,100,"close2 timed out");

	spin_unlock_irqrestore(&lp->lock, flags);
	DEB(DEB_STRUCT,i596_display_data(dev));
	i596_cleanup_cmd(dev,lp);

#ifdef ENABLE_MVME16x_NET
	if (MACH_IS_MVME16x) {
		volatile unsigned char *pcc2 = (unsigned char *) 0xfff42000;

		
		pcc2[0x28] = 1;
		pcc2[0x2a] = 0x40;
		pcc2[0x2b] = 0x40;	
	}
#endif
#ifdef ENABLE_BVME6000_NET
	if (MACH_IS_BVME6000) {
		volatile unsigned char *ethirq = (unsigned char *) BVME_ETHIRQ_REG;

		*ethirq = 1;
	}
#endif

	free_irq(dev->irq, dev);
	remove_rx_bufs(dev);

	return 0;
}



static void set_multicast_list(struct net_device *dev)
{
	struct i596_private *lp = dev->ml_priv;
	int config = 0, cnt;

	DEB(DEB_MULTI,printk(KERN_DEBUG "%s: set multicast list, %d entries, promisc %s, allmulti %s\n",
		dev->name, dev->mc_count,
		dev->flags & IFF_PROMISC  ? "ON" : "OFF",
		dev->flags & IFF_ALLMULTI ? "ON" : "OFF"));

	if (wait_cfg(dev, &lp->cf_cmd.cmd, 1000, "config change request timed out"))
		return;

	if ((dev->flags & IFF_PROMISC) && !(lp->cf_cmd.i596_config[8] & 0x01)) {
		lp->cf_cmd.i596_config[8] |= 0x01;
		config = 1;
	}
	if (!(dev->flags & IFF_PROMISC) && (lp->cf_cmd.i596_config[8] & 0x01)) {
		lp->cf_cmd.i596_config[8] &= ~0x01;
		config = 1;
	}
	if ((dev->flags & IFF_ALLMULTI) && (lp->cf_cmd.i596_config[11] & 0x20)) {
		lp->cf_cmd.i596_config[11] &= ~0x20;
		config = 1;
	}
	if (!(dev->flags & IFF_ALLMULTI) && !(lp->cf_cmd.i596_config[11] & 0x20)) {
		lp->cf_cmd.i596_config[11] |= 0x20;
		config = 1;
	}
	if (config) {
		lp->cf_cmd.cmd.command = CmdConfigure;
		i596_add_cmd(dev, &lp->cf_cmd.cmd);
	}

	cnt = dev->mc_count;
	if (cnt > MAX_MC_CNT)
	{
		cnt = MAX_MC_CNT;
		printk(KERN_ERR "%s: Only %d multicast addresses supported",
			dev->name, cnt);
	}

	if (dev->mc_count > 0) {
		struct dev_mc_list *dmi;
		unsigned char *cp;
		struct mc_cmd *cmd;

		if (wait_cfg(dev, &lp->mc_cmd.cmd, 1000, "multicast list change request timed out"))
			return;
		cmd = &lp->mc_cmd;
		cmd->cmd.command = CmdMulticastList;
		cmd->mc_cnt = dev->mc_count * 6;
		cp = cmd->mc_addrs;
		for (dmi = dev->mc_list; cnt && dmi != NULL; dmi = dmi->next, cnt--, cp += 6) {
			memcpy(cp, dmi->dmi_addr, 6);
			if (i596_debug > 1)
				DEB(DEB_MULTI,printk(KERN_INFO "%s: Adding address %pM\n",
						dev->name, cp));
		}
		i596_add_cmd(dev, &cmd->cmd);
	}
}

#ifdef MODULE
static struct net_device *dev_82596;

#ifdef ENABLE_APRICOT
module_param(irq, int, 0);
MODULE_PARM_DESC(irq, "Apricot IRQ number");
#endif

static int debug = -1;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "i82596 debug mask");

int __init init_module(void)
{
	if (debug >= 0)
		i596_debug = debug;
	dev_82596 = i82596_probe(-1);
	if (IS_ERR(dev_82596))
		return PTR_ERR(dev_82596);
	return 0;
}

void __exit cleanup_module(void)
{
	unregister_netdev(dev_82596);
#ifdef __mc68000__
	

	kernel_set_cachemode((void *)(dev_82596->mem_start), 4096,
			IOMAP_FULL_CACHING);
#endif
	free_page ((u32)(dev_82596->mem_start));
#ifdef ENABLE_APRICOT
	
	release_region(dev_82596->base_addr, I596_TOTAL_SIZE);
#endif
	free_netdev(dev_82596);
}

#endif				
