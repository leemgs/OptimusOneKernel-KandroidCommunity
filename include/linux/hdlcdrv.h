

#ifndef _HDLCDRV_H
#define _HDLCDRV_H




struct hdlcdrv_params {
	int iobase;
	int irq;
	int dma;
	int dma2;
	int seriobase;
	int pariobase;
	int midiiobase;
};	

struct hdlcdrv_channel_params {
	int tx_delay;  
	int tx_tail;   
	int slottime;  
	int ppersist;  
	int fulldup;   
	               
};	

struct hdlcdrv_old_channel_state {
  	int ptt;
  	int dcd;
  	int ptt_keyed;
};

struct hdlcdrv_channel_state {
 	int ptt;
 	int dcd;
 	int ptt_keyed;
 	unsigned long tx_packets;
 	unsigned long tx_errors;
 	unsigned long rx_packets;
 	unsigned long rx_errors;
};

struct hdlcdrv_ioctl {
	int cmd;
	union {
		struct hdlcdrv_params mp;
		struct hdlcdrv_channel_params cp;
		struct hdlcdrv_channel_state cs;
		struct hdlcdrv_old_channel_state ocs;
		unsigned int calibrate;
		unsigned char bits;
		char modename[128];
		char drivername[32];
	} data;
};




#define HDLCDRVCTL_GETMODEMPAR       0
#define HDLCDRVCTL_SETMODEMPAR       1
#define HDLCDRVCTL_MODEMPARMASK      2  
#define HDLCDRVCTL_GETCHANNELPAR    10
#define HDLCDRVCTL_SETCHANNELPAR    11
#define HDLCDRVCTL_OLDGETSTAT       20
#define HDLCDRVCTL_CALIBRATE        21
#define HDLCDRVCTL_GETSTAT          22


#define HDLCDRVCTL_GETSAMPLES       30
#define HDLCDRVCTL_GETBITS          31


#define HDLCDRVCTL_GETMODE          40
#define HDLCDRVCTL_SETMODE          41
#define HDLCDRVCTL_MODELIST         42
#define HDLCDRVCTL_DRIVERNAME       43


#define HDLCDRV_PARMASK_IOBASE      (1<<0)
#define HDLCDRV_PARMASK_IRQ         (1<<1)
#define HDLCDRV_PARMASK_DMA         (1<<2)
#define HDLCDRV_PARMASK_DMA2        (1<<3)
#define HDLCDRV_PARMASK_SERIOBASE   (1<<4)
#define HDLCDRV_PARMASK_PARIOBASE   (1<<5)
#define HDLCDRV_PARMASK_MIDIIOBASE  (1<<6)



#ifdef __KERNEL__

#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/spinlock.h>

#define HDLCDRV_MAGIC      0x5ac6e778
#define HDLCDRV_HDLCBUFFER  32 
#define HDLCDRV_BITBUFFER  256 
#undef HDLCDRV_LOOPBACK  
#define HDLCDRV_DEBUG


#define HDLCDRV_MAXFLEN             400	


struct hdlcdrv_hdlcbuffer {
	spinlock_t lock;
	unsigned rd, wr;
	unsigned short buf[HDLCDRV_HDLCBUFFER];
};

#ifdef HDLCDRV_DEBUG
struct hdlcdrv_bitbuffer {
	unsigned int rd;
	unsigned int wr;
	unsigned int shreg;
	unsigned char buffer[HDLCDRV_BITBUFFER];
};

static inline void hdlcdrv_add_bitbuffer(struct hdlcdrv_bitbuffer *buf, 
					 unsigned int bit)
{
	unsigned char new;

	new = buf->shreg & 1;
	buf->shreg >>= 1;
	buf->shreg |= (!!bit) << 7;
	if (new) {
		buf->buffer[buf->wr] = buf->shreg;
		buf->wr = (buf->wr+1) % sizeof(buf->buffer);
		buf->shreg = 0x80;
	}
}

static inline void hdlcdrv_add_bitbuffer_word(struct hdlcdrv_bitbuffer *buf, 
					      unsigned int bits)
{
	buf->buffer[buf->wr] = bits & 0xff;
	buf->wr = (buf->wr+1) % sizeof(buf->buffer);
	buf->buffer[buf->wr] = (bits >> 8) & 0xff;
	buf->wr = (buf->wr+1) % sizeof(buf->buffer);

}
#endif 




struct hdlcdrv_ops {
	
	const char *drvname;
	const char *drvinfo;
	
	int (*open)(struct net_device *);
	int (*close)(struct net_device *);
	int (*ioctl)(struct net_device *, struct ifreq *, 
		     struct hdlcdrv_ioctl *, int);
};

struct hdlcdrv_state {
	int magic;
	int opened;

	const struct hdlcdrv_ops *ops;

	struct {
		int bitrate;
	} par;

	struct hdlcdrv_pttoutput {
		int dma2;
		int seriobase;
		int pariobase;
		int midiiobase;
		unsigned int flags;
	} ptt_out;

	struct hdlcdrv_channel_params ch_params;

	struct hdlcdrv_hdlcrx {
		struct hdlcdrv_hdlcbuffer hbuf;
		unsigned long in_hdlc_rx;
		
		int rx_state;	
		unsigned int bitstream;
		unsigned int bitbuf;
		int numbits;
		unsigned char dcd;
		
		int len;
		unsigned char *bp;
		unsigned char buffer[HDLCDRV_MAXFLEN+2];
	} hdlcrx;

	struct hdlcdrv_hdlctx {
		struct hdlcdrv_hdlcbuffer hbuf;
		unsigned long in_hdlc_tx;
		
		int tx_state;	
		int numflags;
		unsigned int bitstream;
		unsigned char ptt;
		int calibrate;
		int slotcnt;

		unsigned int bitbuf;
		int numbits;
		
		int len;
		unsigned char *bp;
		unsigned char buffer[HDLCDRV_MAXFLEN+2];
	} hdlctx;

#ifdef HDLCDRV_DEBUG
	struct hdlcdrv_bitbuffer bitbuf_channel;
	struct hdlcdrv_bitbuffer bitbuf_hdlc;
#endif 

	int ptt_keyed;

	
	struct sk_buff *skb;
};




static inline int hdlcdrv_hbuf_full(struct hdlcdrv_hdlcbuffer *hb) 
{
	unsigned long flags;
	int ret;
	
	spin_lock_irqsave(&hb->lock, flags);
	ret = !((HDLCDRV_HDLCBUFFER - 1 + hb->rd - hb->wr) % HDLCDRV_HDLCBUFFER);
	spin_unlock_irqrestore(&hb->lock, flags);
	return ret;
}



static inline int hdlcdrv_hbuf_empty(struct hdlcdrv_hdlcbuffer *hb)
{
	unsigned long flags;
	int ret;
	
	spin_lock_irqsave(&hb->lock, flags);
	ret = (hb->rd == hb->wr);
	spin_unlock_irqrestore(&hb->lock, flags);
	return ret;
}



static inline unsigned short hdlcdrv_hbuf_get(struct hdlcdrv_hdlcbuffer *hb)
{
	unsigned long flags;
	unsigned short val;
	unsigned newr;

	spin_lock_irqsave(&hb->lock, flags);
	if (hb->rd == hb->wr)
		val = 0;
	else {
		newr = (hb->rd+1) % HDLCDRV_HDLCBUFFER;
		val = hb->buf[hb->rd];
		hb->rd = newr;
	}
	spin_unlock_irqrestore(&hb->lock, flags);
	return val;
}



static inline void hdlcdrv_hbuf_put(struct hdlcdrv_hdlcbuffer *hb, 
				    unsigned short val)
{
	unsigned newp;
	unsigned long flags;
	
	spin_lock_irqsave(&hb->lock, flags);
	newp = (hb->wr+1) % HDLCDRV_HDLCBUFFER;
	if (newp != hb->rd) { 
		hb->buf[hb->wr] = val & 0xffff;
		hb->wr = newp;
	}
	spin_unlock_irqrestore(&hb->lock, flags);
}



static inline void hdlcdrv_putbits(struct hdlcdrv_state *s, unsigned int bits)
{
	hdlcdrv_hbuf_put(&s->hdlcrx.hbuf, bits);
}

static inline unsigned int hdlcdrv_getbits(struct hdlcdrv_state *s)
{
	unsigned int ret;

	if (hdlcdrv_hbuf_empty(&s->hdlctx.hbuf)) {
		if (s->hdlctx.calibrate > 0)
			s->hdlctx.calibrate--;
		else
			s->hdlctx.ptt = 0;
		ret = 0;
	} else 
		ret = hdlcdrv_hbuf_get(&s->hdlctx.hbuf);
#ifdef HDLCDRV_LOOPBACK
	hdlcdrv_hbuf_put(&s->hdlcrx.hbuf, ret);
#endif 
	return ret;
}

static inline void hdlcdrv_channelbit(struct hdlcdrv_state *s, unsigned int bit)
{
#ifdef HDLCDRV_DEBUG
	hdlcdrv_add_bitbuffer(&s->bitbuf_channel, bit);
#endif 
}

static inline void hdlcdrv_setdcd(struct hdlcdrv_state *s, int dcd)
{
	s->hdlcrx.dcd = !!dcd;
}

static inline int hdlcdrv_ptt(struct hdlcdrv_state *s)
{
	return s->hdlctx.ptt || (s->hdlctx.calibrate > 0);
}



void hdlcdrv_receiver(struct net_device *, struct hdlcdrv_state *);
void hdlcdrv_transmitter(struct net_device *, struct hdlcdrv_state *);
void hdlcdrv_arbitrate(struct net_device *, struct hdlcdrv_state *);
struct net_device *hdlcdrv_register(const struct hdlcdrv_ops *ops,
				    unsigned int privsize, const char *ifname,
				    unsigned int baseaddr, unsigned int irq, 
				    unsigned int dma);
void hdlcdrv_unregister(struct net_device *dev);





#endif 



#endif 


