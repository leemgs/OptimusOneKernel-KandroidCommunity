#ifndef S390_IO_SCH_H
#define S390_IO_SCH_H

#include <asm/schid.h>


struct cmd_orb {
	u32 intparm;	
	u32 key  : 4;	
	u32 spnd : 1;	
	u32 res1 : 1;	
	u32 mod  : 1;	
	u32 sync : 1;	
	u32 fmt  : 1;	
	u32 pfch : 1;	
	u32 isic : 1;	
	u32 alcc : 1;	
	u32 ssic : 1;	
	u32 res2 : 1;	
	u32 c64  : 1;	
	u32 i2k  : 1;	
	u32 lpm  : 8;	
	u32 ils  : 1;	
	u32 zero : 6;	
	u32 orbx : 1;	
	u32 cpa;	
}  __attribute__ ((packed, aligned(4)));


struct tm_orb {
	u32 intparm;
	u32 key:4;
	u32 :9;
	u32 b:1;
	u32 :2;
	u32 lpm:8;
	u32 :7;
	u32 x:1;
	u32 tcw;
	u32 prio:8;
	u32 :8;
	u32 rsvpgm:8;
	u32 :8;
	u32 :32;
	u32 :32;
	u32 :32;
	u32 :32;
}  __attribute__ ((packed, aligned(4)));

union orb {
	struct cmd_orb cmd;
	struct tm_orb tm;
}  __attribute__ ((packed, aligned(4)));

struct io_subchannel_private {
	union orb orb;		
	struct ccw1 sense_ccw;	
} __attribute__ ((aligned(8)));

#define to_io_private(n) ((struct io_subchannel_private *)n->private)
#define sch_get_cdev(n) (dev_get_drvdata(&n->dev))
#define sch_set_cdev(n, c) (dev_set_drvdata(&n->dev, c))

#define MAX_CIWS 8


struct senseid {
	
	u8  reserved;	
	u16 cu_type;	
	u8  cu_model;	
	u16 dev_type;	
	u8  dev_model;	
	u8  unused;	
	
	struct ciw ciw[MAX_CIWS];	
}  __attribute__ ((packed, aligned(4)));

struct ccw_device_private {
	struct ccw_device *cdev;
	struct subchannel *sch;
	int state;		
	atomic_t onoff;
	unsigned long registered;
	struct ccw_dev_id dev_id;	
	struct subchannel_id schid;	
	u8 imask;		
	int iretry;		
	struct {
		unsigned int fast:1;	
		unsigned int repall:1;	
		unsigned int pgroup:1;	
		unsigned int force:1;	
	} __attribute__ ((packed)) options;
	struct {
		unsigned int pgid_single:1; 
		unsigned int esid:1;	    
		unsigned int dosense:1;	    
		unsigned int doverify:1;    
		unsigned int donotify:1;    
		unsigned int recog_done:1;  
		unsigned int fake_irb:1;    
		unsigned int intretry:1;    
		unsigned int resuming:1;    
	} __attribute__((packed)) flags;
	unsigned long intparm;	
	struct qdio_irq *qdio_data;
	struct irb irb;		
	struct senseid senseid;	
	struct pgid pgid[8];	
	struct ccw1 iccws[2];	
	struct work_struct kick_work;
	wait_queue_head_t wait_q;
	struct timer_list timer;
	void *cmb;			
	struct list_head cmb_list;	
	u64 cmb_start_time;		
	void *cmb_wait;			
};

static inline int ssch(struct subchannel_id schid, union orb *addr)
{
	register struct subchannel_id reg1 asm("1") = schid;
	int ccode = -EIO;

	asm volatile(
		"	ssch	0(%2)\n"
		"0:	ipm	%0\n"
		"	srl	%0,28\n"
		"1:\n"
		EX_TABLE(0b, 1b)
		: "+d" (ccode)
		: "d" (reg1), "a" (addr), "m" (*addr)
		: "cc", "memory");
	return ccode;
}

static inline int rsch(struct subchannel_id schid)
{
	register struct subchannel_id reg1 asm("1") = schid;
	int ccode;

	asm volatile(
		"	rsch\n"
		"	ipm	%0\n"
		"	srl	%0,28"
		: "=d" (ccode)
		: "d" (reg1)
		: "cc", "memory");
	return ccode;
}

static inline int csch(struct subchannel_id schid)
{
	register struct subchannel_id reg1 asm("1") = schid;
	int ccode;

	asm volatile(
		"	csch\n"
		"	ipm	%0\n"
		"	srl	%0,28"
		: "=d" (ccode)
		: "d" (reg1)
		: "cc");
	return ccode;
}

static inline int hsch(struct subchannel_id schid)
{
	register struct subchannel_id reg1 asm("1") = schid;
	int ccode;

	asm volatile(
		"	hsch\n"
		"	ipm	%0\n"
		"	srl	%0,28"
		: "=d" (ccode)
		: "d" (reg1)
		: "cc");
	return ccode;
}

static inline int xsch(struct subchannel_id schid)
{
	register struct subchannel_id reg1 asm("1") = schid;
	int ccode;

	asm volatile(
		"	.insn	rre,0xb2760000,%1,0\n"
		"	ipm	%0\n"
		"	srl	%0,28"
		: "=d" (ccode)
		: "d" (reg1)
		: "cc");
	return ccode;
}

#endif
