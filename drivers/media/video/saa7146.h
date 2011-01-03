

#ifndef __SAA7146__
#define __SAA7146__

#define SAA7146_VERSION_CODE 0x000101

#include <linux/types.h>
#include <linux/wait.h>

#ifndef O_NONCAP
#define O_NONCAP	O_TRUNC
#endif

#define MAX_GBUFFERS	2
#define FBUF_SIZE	0x190000

#ifdef __KERNEL__

struct saa7146_window
{
	int x, y;
	ushort width, height;
	ushort bpp, bpl;
	ushort swidth, sheight;
	short cropx, cropy;
	ushort cropwidth, cropheight;
	unsigned long vidadr;
	int color_fmt;
	ushort depth;
};


struct device_open
{
	int	     isopen;
	int	     noncapturing;
	struct saa7146  *dev;
};
#define MAX_OPENS 3

struct saa7146
{
	struct video_device video_dev;
	struct video_picture picture;
	struct video_audio audio_dev;
	struct video_info vidinfo;
	int user;
	int cap;
	int capuser;
	int irqstate;		
	int writemode;
	int playmode;
	unsigned int nr;
	unsigned long irq;          
	unsigned short id;
	unsigned char revision;
	unsigned char boardcfg[64];	
	unsigned long saa7146_adr;   
	struct saa7146_window win;
	unsigned char __iomem *saa7146_mem; 
	struct device_open open_data[MAX_OPENS];
#define MAX_MARKS 16
	
	int endmark[MAX_MARKS], endmarkhead, endmarktail;
	u32 *dmaRPS1, *pageRPS1, *dmaRPS2, *pageRPS2, *dmavid1, *dmavid2,
		*dmavid3, *dmaa1in, *dmaa1out, *dmaa2in, *dmaa2out,
		*pagedebi, *pagevid1, *pagevid2, *pagevid3, *pagea1in,
		*pagea1out, *pagea2in, *pagea2out;
	wait_queue_head_t i2cq, debiq, audq, vidq;
	u8  *vidbuf, *audbuf, *osdbuf, *dmadebi;
	int audhead, vidhead, osdhead, audtail, vidtail, osdtail;
	spinlock_t lock;	
};
#endif

#ifdef _ALPHA_SAA7146
#define saawrite(dat,adr)    writel((dat), saa->saa7146_adr+(adr))
#define saaread(adr)         readl(saa->saa7146_adr+(adr))
#else
#define saawrite(dat,adr)    writel((dat), saa->saa7146_mem+(adr))
#define saaread(adr)         readl(saa->saa7146_mem+(adr))
#endif

#define saaand(dat,adr)      saawrite((dat) & saaread(adr), adr)
#define saaor(dat,adr)       saawrite((dat) | saaread(adr), adr)
#define saaaor(dat,mask,adr) saawrite((dat) | ((mask) & saaread(adr)), adr)


#define SAA7146_UNKNOWN		0x00000000
#define SAA7146_SAA7111		0x00000001
#define SAA7146_SAA7121		0x00000002
#define SAA7146_IBMMPEG		0x00000004

#endif
