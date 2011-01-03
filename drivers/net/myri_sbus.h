

#ifndef _MYRI_SBUS_H
#define _MYRI_SBUS_H


#define LANAI_IPF0	0x00UL		
#define LANAI_CUR0	0x04UL
#define LANAI_PREV0	0x08UL
#define LANAI_DATA0	0x0cUL
#define LANAI_DPF0	0x10UL
#define LANAI_IPF1	0x14UL		
#define LANAI_CUR1	0x18UL
#define LANAI_PREV1	0x1cUL
#define LANAI_DATA1	0x20UL
#define LANAI_DPF1	0x24UL
#define LANAI_ISTAT	0x28UL		
#define LANAI_EIMASK	0x2cUL		
#define LANAI_ITIMER	0x30UL		
#define LANAI_RTC	0x34UL		
#define LANAI_CSUM	0x38UL		
#define LANAI_DMAXADDR	0x3cUL		
#define LANAI_DMALADDR	0x40UL		
#define LANAI_DMACTR	0x44UL		
#define LANAI_RXDMAPTR	0x48UL		
#define LANAI_RXDMALIM	0x4cUL		
#define LANAI_TXDMAPTR	0x50UL		
#define LANAI_TXDMALIM	0x54UL		
#define LANAI_TXDMALIMT	0x58UL		
	
#define LANAI_RBYTE	0x60UL		
	
#define LANAI_RHALF	0x70UL		
	
#define LANAI_RWORD	0x74UL		
#define LANAI_SALIGN	0x78UL		
#define LANAI_SBYTE	0x7cUL		
#define LANAI_SHALF	0x80UL		
#define LANAI_SWORD	0x84UL		
#define LANAI_SSENDT	0x88UL		
#define LANAI_DMADIR	0x8cUL		
#define LANAI_DMASTAT	0x90UL		
#define LANAI_TIMEO	0x94UL		
#define LANAI_MYRINET	0x98UL		
#define LANAI_HWDEBUG	0x9cUL		
#define LANAI_LEDS	0xa0UL		
#define LANAI_VERS	0xa4UL		
#define LANAI_LINKON	0xa8UL		
	
#define LANAI_CVAL	0x108UL		
#define LANAI_REG_SIZE	0x10cUL


#define ISTAT_DEBUG	0x80000000
#define ISTAT_HOST	0x40000000
#define ISTAT_LAN7	0x00800000
#define ISTAT_LAN6	0x00400000
#define ISTAT_LAN5	0x00200000
#define ISTAT_LAN4	0x00100000
#define ISTAT_LAN3	0x00080000
#define ISTAT_LAN2	0x00040000
#define ISTAT_LAN1	0x00020000
#define ISTAT_LAN0	0x00010000
#define ISTAT_WRDY	0x00008000
#define ISTAT_HRDY	0x00004000
#define ISTAT_SRDY	0x00002000
#define ISTAT_LINK	0x00001000
#define ISTAT_FRES	0x00000800
#define ISTAT_NRES	0x00000800
#define ISTAT_WAKE	0x00000400
#define ISTAT_OB2	0x00000200
#define ISTAT_OB1	0x00000100
#define ISTAT_TAIL	0x00000080
#define ISTAT_WDOG	0x00000040
#define ISTAT_TIME	0x00000020
#define ISTAT_DMA	0x00000010
#define ISTAT_SEND	0x00000008
#define ISTAT_BUF	0x00000004
#define ISTAT_RECV	0x00000002
#define ISTAT_BRDY	0x00000001


#define MYRI_RESETOFF	0x00UL
#define MYRI_RESETON	0x04UL
#define MYRI_IRQOFF	0x08UL
#define MYRI_IRQON	0x0cUL
#define MYRI_WAKEUPOFF	0x10UL
#define MYRI_WAKEUPON	0x14UL
#define MYRI_IRQREAD	0x18UL
	
#define MYRI_LOCALMEM	0x4000UL
#define MYRI_REG_SIZE	0x25000UL


#define SHMEM_IMASK_RX		0x00000002
#define SHMEM_IMASK_TX		0x00000001


#define KERNEL_CHANNEL		0


struct myri_eeprom {
	unsigned int		cval;
	unsigned short		cpuvers;
	unsigned char		id[6];
	unsigned int		ramsz;
	unsigned char		fvers[32];
	unsigned char		mvers[16];
	unsigned short		dlval;
	unsigned short		brd_type;
	unsigned short		bus_type;
	unsigned short		prod_code;
	unsigned int		serial_num;
	unsigned short		_reserved[24];
	unsigned int		_unused[2];
};


#define BUS_TYPE_SBUS		1


#define CPUVERS_2_3		0x0203
#define CPUVERS_3_0		0x0300
#define CPUVERS_3_1		0x0301
#define CPUVERS_3_2		0x0302
#define CPUVERS_4_0		0x0400
#define CPUVERS_4_1		0x0401
#define CPUVERS_4_2		0x0402
#define CPUVERS_5_0		0x0500


#define MYRICTRL_CTRL		0x00UL
#define MYRICTRL_IRQLVL		0x02UL
#define MYRICTRL_REG_SIZE	0x04UL


#define CONTROL_ROFF		0x8000	
#define CONTROL_RON		0x4000	
#define CONTROL_EIRQ		0x2000	
#define CONTROL_DIRQ		0x1000	
#define CONTROL_WON		0x0800	

#define MYRI_SCATTER_ENTRIES	8
#define MYRI_GATHER_ENTRIES	16

struct myri_sglist {
	u32 addr;
	u32 len;
};

struct myri_rxd {
	struct myri_sglist myri_scatters[MYRI_SCATTER_ENTRIES];	
	u32 csum;	
	u32 ctx;
	u32 num_sg;	
};

struct myri_txd {
	struct myri_sglist myri_gathers[MYRI_GATHER_ENTRIES]; 
	u32 num_sg;	
	u16 addr[4];	
	u32 chan;
	u32 len;	
	u32 csum_off;	
	u32 csum_field;	
};

#define MYRINET_MTU        8432
#define RX_ALLOC_SIZE      8448
#define MYRI_PAD_LEN       2
#define RX_COPY_THRESHOLD  256


#define TX_RING_MAXSIZE    16
#define RX_RING_MAXSIZE    16

#define TX_RING_SIZE       16
#define RX_RING_SIZE       16


static __inline__ int NEXT_RX(int num)
{
	
	if(++num > RX_RING_SIZE)
		num = 0;
	return num;
}

static __inline__ int PREV_RX(int num)
{
	if(--num < 0)
		num = RX_RING_SIZE;
	return num;
}

#define NEXT_TX(num)	(((num) + 1) & (TX_RING_SIZE - 1))
#define PREV_TX(num)	(((num) - 1) & (TX_RING_SIZE - 1))

#define TX_BUFFS_AVAIL(head, tail)		\
	((head) <= (tail) ?			\
	 (head) + (TX_RING_SIZE - 1) - (tail) :	\
	 (head) - (tail) - 1)

struct sendq {
	u32	tail;
	u32	head;
	u32	hdebug;
	u32	mdebug;
	struct myri_txd	myri_txd[TX_RING_MAXSIZE];
};

struct recvq {
	u32	head;
	u32	tail;
	u32	hdebug;
	u32	mdebug;
	struct myri_rxd	myri_rxd[RX_RING_MAXSIZE + 1];
};

#define MYRI_MLIST_SIZE 8

struct mclist {
	u32 maxlen;
	u32 len;
	u32 cache;
	struct pair {
		u8 addr[8];
		u32 val;
	} mc_pairs[MYRI_MLIST_SIZE];
	u8 bcast_addr[8];
};

struct myri_channel {
	u32		state;		
	u32		busy;		
	struct sendq	sendq;		
	struct recvq	recvq;		
	struct recvq	recvqa;		
	u32		rbytes;		
	u32		sbytes;		
	u32		rmsgs;		
	u32		smsgs;		
	struct mclist	mclist;		
};


#define STATE_WFH	0		
#define STATE_WFN	1		
#define STATE_READY	2		

struct myri_shmem {
	u8	addr[8];		
	u32	nchan;			
	u32	burst;			
	u32	shakedown;		
	u32	send;			
	u32	imask;			
	u32	mlevel;			
	u32	debug[4];		
	struct myri_channel channel;	
};

struct myri_eth {
	
	spinlock_t			irq_lock;
	struct myri_shmem __iomem	*shmem;		
	void __iomem			*cregs;		
	struct recvq __iomem		*rqack;		
	struct recvq __iomem		*rq;		
	struct sendq __iomem		*sq;		
	struct net_device		*dev;		
	int				tx_old;		
	void __iomem			*lregs;		
	struct sk_buff	       *rx_skbs[RX_RING_SIZE+1];
	struct sk_buff	       *tx_skbs[TX_RING_SIZE];  

	
	void __iomem			*regs;          
	void __iomem			*lanai;		
	unsigned int			myri_bursts;	
	struct myri_eeprom		eeprom;		
	unsigned int			reg_size;	
	unsigned int			shmem_base;	
	struct of_device		*myri_op;	
};


#define ALIGNED_RX_SKB_ADDR(addr) \
        ((((unsigned long)(addr) + (64 - 1)) & ~(64 - 1)) - (unsigned long)(addr))
static inline struct sk_buff *myri_alloc_skb(unsigned int length, gfp_t gfp_flags)
{
	struct sk_buff *skb;

	skb = alloc_skb(length + 64, gfp_flags);
	if(skb) {
		int offset = ALIGNED_RX_SKB_ADDR(skb->data);

		if(offset)
			skb_reserve(skb, offset);
	}
	return skb;
}

#endif 
