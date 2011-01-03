

#ifndef _EDAC_CORE_H_
#define _EDAC_CORE_H_

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/pci.h>
#include <linux/time.h>
#include <linux/nmi.h>
#include <linux/rcupdate.h>
#include <linux/completion.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/workqueue.h>

#define EDAC_MC_LABEL_LEN	31
#define EDAC_DEVICE_NAME_LEN	31
#define EDAC_ATTRIB_VALUE_LEN	15
#define MC_PROC_NAME_MAX_LEN	7

#if PAGE_SHIFT < 20
#define PAGES_TO_MiB( pages )	( ( pages ) >> ( 20 - PAGE_SHIFT ) )
#else				
#define PAGES_TO_MiB( pages )	( ( pages ) << ( PAGE_SHIFT - 20 ) )
#endif

#define edac_printk(level, prefix, fmt, arg...) \
	printk(level "EDAC " prefix ": " fmt, ##arg)

#define edac_printk_verbose(level, prefix, fmt, arg...) \
	printk(level "EDAC " prefix ": " "in %s, line at %d: " fmt,	\
	       __FILE__, __LINE__, ##arg)

#define edac_mc_printk(mci, level, fmt, arg...) \
	printk(level "EDAC MC%d: " fmt, mci->mc_idx, ##arg)

#define edac_mc_chipset_printk(mci, level, prefix, fmt, arg...) \
	printk(level "EDAC " prefix " MC%d: " fmt, mci->mc_idx, ##arg)


#define edac_device_printk(ctl, level, fmt, arg...) \
	printk(level "EDAC DEVICE%d: " fmt, ctl->dev_idx, ##arg)


#define edac_pci_printk(ctl, level, fmt, arg...) \
	printk(level "EDAC PCI%d: " fmt, ctl->pci_idx, ##arg)


#define EDAC_MC "MC"
#define EDAC_PCI "PCI"
#define EDAC_DEBUG "DEBUG"

#ifdef CONFIG_EDAC_DEBUG
extern int edac_debug_level;

#ifndef CONFIG_EDAC_DEBUG_VERBOSE
#define edac_debug_printk(level, fmt, arg...)                           \
	do {                                                            \
		if (level <= edac_debug_level)                          \
			edac_printk(KERN_DEBUG, EDAC_DEBUG,		\
				    "%s: " fmt, __func__, ##arg);	\
	} while (0)
#else  
#define edac_debug_printk(level, fmt, arg...)                            \
	do {                                                             \
		if (level <= edac_debug_level)                           \
			edac_printk_verbose(KERN_DEBUG, EDAC_DEBUG, fmt, \
					    ##arg);			\
	} while (0)
#endif

#define debugf0( ... ) edac_debug_printk(0, __VA_ARGS__ )
#define debugf1( ... ) edac_debug_printk(1, __VA_ARGS__ )
#define debugf2( ... ) edac_debug_printk(2, __VA_ARGS__ )
#define debugf3( ... ) edac_debug_printk(3, __VA_ARGS__ )
#define debugf4( ... ) edac_debug_printk(4, __VA_ARGS__ )

#else				

#define debugf0( ... )
#define debugf1( ... )
#define debugf2( ... )
#define debugf3( ... )
#define debugf4( ... )

#endif				

#define PCI_VEND_DEV(vend, dev) PCI_VENDOR_ID_ ## vend, \
	PCI_DEVICE_ID_ ## vend ## _ ## dev

#define edac_dev_name(dev) (dev)->dev_name


enum dev_type {
	DEV_UNKNOWN = 0,
	DEV_X1,
	DEV_X2,
	DEV_X4,
	DEV_X8,
	DEV_X16,
	DEV_X32,		
	DEV_X64			
};

#define DEV_FLAG_UNKNOWN	BIT(DEV_UNKNOWN)
#define DEV_FLAG_X1		BIT(DEV_X1)
#define DEV_FLAG_X2		BIT(DEV_X2)
#define DEV_FLAG_X4		BIT(DEV_X4)
#define DEV_FLAG_X8		BIT(DEV_X8)
#define DEV_FLAG_X16		BIT(DEV_X16)
#define DEV_FLAG_X32		BIT(DEV_X32)
#define DEV_FLAG_X64		BIT(DEV_X64)


enum mem_type {
	MEM_EMPTY = 0,		
	MEM_RESERVED,		
	MEM_UNKNOWN,		
	MEM_FPM,		
	MEM_EDO,		
	MEM_BEDO,		
	MEM_SDR,		
	MEM_RDR,		
	MEM_DDR,		
	MEM_RDDR,		
	MEM_RMBS,		
	MEM_DDR2,		
	MEM_FB_DDR2,		
	MEM_RDDR2,		
	MEM_XDR,		
	MEM_DDR3,		
	MEM_RDDR3,		
};

#define MEM_FLAG_EMPTY		BIT(MEM_EMPTY)
#define MEM_FLAG_RESERVED	BIT(MEM_RESERVED)
#define MEM_FLAG_UNKNOWN	BIT(MEM_UNKNOWN)
#define MEM_FLAG_FPM		BIT(MEM_FPM)
#define MEM_FLAG_EDO		BIT(MEM_EDO)
#define MEM_FLAG_BEDO		BIT(MEM_BEDO)
#define MEM_FLAG_SDR		BIT(MEM_SDR)
#define MEM_FLAG_RDR		BIT(MEM_RDR)
#define MEM_FLAG_DDR		BIT(MEM_DDR)
#define MEM_FLAG_RDDR		BIT(MEM_RDDR)
#define MEM_FLAG_RMBS		BIT(MEM_RMBS)
#define MEM_FLAG_DDR2           BIT(MEM_DDR2)
#define MEM_FLAG_FB_DDR2        BIT(MEM_FB_DDR2)
#define MEM_FLAG_RDDR2          BIT(MEM_RDDR2)
#define MEM_FLAG_XDR            BIT(MEM_XDR)
#define MEM_FLAG_DDR3		 BIT(MEM_DDR3)
#define MEM_FLAG_RDDR3		 BIT(MEM_RDDR3)


enum edac_type {
	EDAC_UNKNOWN = 0,	
	EDAC_NONE,		
	EDAC_RESERVED,		
	EDAC_PARITY,		
	EDAC_EC,		
	EDAC_SECDED,		
	EDAC_S2ECD2ED,		
	EDAC_S4ECD4ED,		
	EDAC_S8ECD8ED,		
	EDAC_S16ECD16ED,	
};

#define EDAC_FLAG_UNKNOWN	BIT(EDAC_UNKNOWN)
#define EDAC_FLAG_NONE		BIT(EDAC_NONE)
#define EDAC_FLAG_PARITY	BIT(EDAC_PARITY)
#define EDAC_FLAG_EC		BIT(EDAC_EC)
#define EDAC_FLAG_SECDED	BIT(EDAC_SECDED)
#define EDAC_FLAG_S2ECD2ED	BIT(EDAC_S2ECD2ED)
#define EDAC_FLAG_S4ECD4ED	BIT(EDAC_S4ECD4ED)
#define EDAC_FLAG_S8ECD8ED	BIT(EDAC_S8ECD8ED)
#define EDAC_FLAG_S16ECD16ED	BIT(EDAC_S16ECD16ED)


enum scrub_type {
	SCRUB_UNKNOWN = 0,	
	SCRUB_NONE,		
	SCRUB_SW_PROG,		
	SCRUB_SW_SRC,		
	SCRUB_SW_PROG_SRC,	
	SCRUB_SW_TUNABLE,	
	SCRUB_HW_PROG,		
	SCRUB_HW_SRC,		
	SCRUB_HW_PROG_SRC,	
	SCRUB_HW_TUNABLE	
};

#define SCRUB_FLAG_SW_PROG	BIT(SCRUB_SW_PROG)
#define SCRUB_FLAG_SW_SRC	BIT(SCRUB_SW_SRC)
#define SCRUB_FLAG_SW_PROG_SRC	BIT(SCRUB_SW_PROG_SRC)
#define SCRUB_FLAG_SW_TUN	BIT(SCRUB_SW_SCRUB_TUNABLE)
#define SCRUB_FLAG_HW_PROG	BIT(SCRUB_HW_PROG)
#define SCRUB_FLAG_HW_SRC	BIT(SCRUB_HW_SRC)
#define SCRUB_FLAG_HW_PROG_SRC	BIT(SCRUB_HW_PROG_SRC)
#define SCRUB_FLAG_HW_TUN	BIT(SCRUB_HW_TUNABLE)




#define	OP_ALLOC		0x100
#define OP_RUNNING_POLL		0x201
#define OP_RUNNING_INTERRUPT	0x202
#define OP_RUNNING_POLL_INTR	0x203
#define OP_OFFLINE		0x300



struct channel_info {
	int chan_idx;		
	u32 ce_count;		
	char label[EDAC_MC_LABEL_LEN + 1];	
	struct csrow_info *csrow;	
};

struct csrow_info {
	unsigned long first_page;	
	unsigned long last_page;	
	unsigned long page_mask;	
	u32 nr_pages;		
	u32 grain;		
	int csrow_idx;		
	enum dev_type dtype;	
	u32 ue_count;		
	u32 ce_count;		
	enum mem_type mtype;	
	enum edac_type edac_mode;	
	struct mem_ctl_info *mci;	

	struct kobject kobj;	

	
	u32 nr_channels;
	struct channel_info *channels;
};


struct mcidev_sysfs_attribute {
        struct attribute attr;
        ssize_t (*show)(struct mem_ctl_info *,char *);
        ssize_t (*store)(struct mem_ctl_info *, const char *,size_t);
};


struct mem_ctl_info {
	struct list_head link;	

	struct module *owner;	

	unsigned long mtype_cap;	
	unsigned long edac_ctl_cap;	
	unsigned long edac_cap;	
	unsigned long scrub_cap;	
	enum scrub_type scrub_mode;	

	
	int (*set_sdram_scrub_rate) (struct mem_ctl_info * mci, u32 * bw);

	
	int (*get_sdram_scrub_rate) (struct mem_ctl_info * mci, u32 * bw);


	
	void (*edac_check) (struct mem_ctl_info * mci);

	
	
	unsigned long (*ctl_page_to_phys) (struct mem_ctl_info * mci,
					   unsigned long page);
	int mc_idx;
	int nr_csrows;
	struct csrow_info *csrows;
	
	struct device *dev;
	const char *mod_name;
	const char *mod_ver;
	const char *ctl_name;
	const char *dev_name;
	char proc_name[MC_PROC_NAME_MAX_LEN + 1];
	void *pvt_info;
	u32 ue_noinfo_count;	
	u32 ce_noinfo_count;	
	u32 ue_count;		
	u32 ce_count;		
	unsigned long start_time;	

	
	struct rcu_head rcu;
	struct completion complete;

	
	struct kobject edac_mci_kobj;

	
	struct mcidev_sysfs_attribute *mc_driver_sysfs_attributes;

	
	struct delayed_work work;

	
	int op_state;
};



struct edac_device_counter {
	u32 ue_count;
	u32 ce_count;
};


struct edac_device_ctl_info;
struct edac_device_block;


struct edac_dev_sysfs_attribute {
	struct attribute attr;
	ssize_t (*show)(struct edac_device_ctl_info *, char *);
	ssize_t (*store)(struct edac_device_ctl_info *, const char *, size_t);
};


struct edac_dev_sysfs_block_attribute {
	struct attribute attr;
	ssize_t (*show)(struct kobject *, struct attribute *, char *);
	ssize_t (*store)(struct kobject *, struct attribute *,
			const char *, size_t);
	struct edac_device_block *block;

	unsigned int value;
};


struct edac_device_block {
	struct edac_device_instance *instance;	
	char name[EDAC_DEVICE_NAME_LEN + 1];

	struct edac_device_counter counters;	

	int nr_attribs;		

	
	struct edac_dev_sysfs_block_attribute *block_attributes;

	
	struct kobject kobj;
};


struct edac_device_instance {
	struct edac_device_ctl_info *ctl;	
	char name[EDAC_DEVICE_NAME_LEN + 4];

	struct edac_device_counter counters;	

	u32 nr_blocks;		
	struct edac_device_block *blocks;	

	
	struct kobject kobj;
};



struct edac_device_ctl_info {
	
	struct list_head link;

	struct module *owner;	

	int dev_idx;

	
	int log_ue;		
	int log_ce;		
	int panic_on_ue;	
	unsigned poll_msec;	
	unsigned long delay;	

	
	struct edac_dev_sysfs_attribute *sysfs_attributes;

	
	struct sysdev_class *edac_class;

	
	int op_state;
	
	struct delayed_work work;

	
	void (*edac_check) (struct edac_device_ctl_info * edac_dev);

	struct device *dev;	

	const char *mod_name;	
	const char *ctl_name;	
	const char *dev_name;	

	void *pvt_info;		

	unsigned long start_time;	

	
	struct rcu_head rcu;
	struct completion removal_complete;

	
	char name[EDAC_DEVICE_NAME_LEN + 1];

	
	u32 nr_instances;
	struct edac_device_instance *instances;

	
	struct edac_device_counter counters;

	
	struct kobject kobj;
};


#define to_edac_mem_ctl_work(w) \
		container_of(w, struct mem_ctl_info, work)

#define to_edac_device_ctl_work(w) \
		container_of(w,struct edac_device_ctl_info,work)


extern struct edac_device_ctl_info *edac_device_alloc_ctl_info(
		unsigned sizeof_private,
		char *edac_device_name, unsigned nr_instances,
		char *edac_block_name, unsigned nr_blocks,
		unsigned offset_value,
		struct edac_dev_sysfs_block_attribute *block_attributes,
		unsigned nr_attribs,
		int device_index);


#define	BLOCK_OFFSET_VALUE_OFF	((unsigned) -1)

extern void edac_device_free_ctl_info(struct edac_device_ctl_info *ctl_info);

#ifdef CONFIG_PCI

struct edac_pci_counter {
	atomic_t pe_count;
	atomic_t npe_count;
};


struct edac_pci_ctl_info {
	
	struct list_head link;

	int pci_idx;

	struct sysdev_class *edac_class;	

	
	int op_state;
	
	struct delayed_work work;

	
	void (*edac_check) (struct edac_pci_ctl_info * edac_dev);

	struct device *dev;	

	const char *mod_name;	
	const char *ctl_name;	
	const char *dev_name;	

	void *pvt_info;		

	unsigned long start_time;	

	
	struct rcu_head rcu;
	struct completion complete;

	
	char name[EDAC_DEVICE_NAME_LEN + 1];

	
	struct edac_pci_counter counters;

	
	struct kobject kobj;
	struct completion kobj_complete;
};

#define to_edac_pci_ctl_work(w) \
		container_of(w, struct edac_pci_ctl_info,work)


static inline void pci_write_bits8(struct pci_dev *pdev, int offset, u8 value,
				   u8 mask)
{
	if (mask != 0xff) {
		u8 buf;

		pci_read_config_byte(pdev, offset, &buf);
		value &= mask;
		buf &= ~mask;
		value |= buf;
	}

	pci_write_config_byte(pdev, offset, value);
}


static inline void pci_write_bits16(struct pci_dev *pdev, int offset,
				    u16 value, u16 mask)
{
	if (mask != 0xffff) {
		u16 buf;

		pci_read_config_word(pdev, offset, &buf);
		value &= mask;
		buf &= ~mask;
		value |= buf;
	}

	pci_write_config_word(pdev, offset, value);
}


static inline void pci_write_bits32(struct pci_dev *pdev, int offset,
				    u32 value, u32 mask)
{
	if (mask != 0xffffffff) {
		u32 buf;

		pci_read_config_dword(pdev, offset, &buf);
		value &= mask;
		buf &= ~mask;
		value |= buf;
	}

	pci_write_config_dword(pdev, offset, value);
}

#endif				

extern struct mem_ctl_info *edac_mc_alloc(unsigned sz_pvt, unsigned nr_csrows,
					  unsigned nr_chans, int edac_index);
extern int edac_mc_add_mc(struct mem_ctl_info *mci);
extern void edac_mc_free(struct mem_ctl_info *mci);
extern struct mem_ctl_info *edac_mc_find(int idx);
extern struct mem_ctl_info *edac_mc_del_mc(struct device *dev);
extern int edac_mc_find_csrow_by_page(struct mem_ctl_info *mci,
				      unsigned long page);


extern void edac_mc_handle_ce(struct mem_ctl_info *mci,
			      unsigned long page_frame_number,
			      unsigned long offset_in_page,
			      unsigned long syndrome, int row, int channel,
			      const char *msg);
extern void edac_mc_handle_ce_no_info(struct mem_ctl_info *mci,
				      const char *msg);
extern void edac_mc_handle_ue(struct mem_ctl_info *mci,
			      unsigned long page_frame_number,
			      unsigned long offset_in_page, int row,
			      const char *msg);
extern void edac_mc_handle_ue_no_info(struct mem_ctl_info *mci,
				      const char *msg);
extern void edac_mc_handle_fbd_ue(struct mem_ctl_info *mci, unsigned int csrow,
				  unsigned int channel0, unsigned int channel1,
				  char *msg);
extern void edac_mc_handle_fbd_ce(struct mem_ctl_info *mci, unsigned int csrow,
				  unsigned int channel, char *msg);


extern int edac_device_add_device(struct edac_device_ctl_info *edac_dev);
extern struct edac_device_ctl_info *edac_device_del_device(struct device *dev);
extern void edac_device_handle_ue(struct edac_device_ctl_info *edac_dev,
				int inst_nr, int block_nr, const char *msg);
extern void edac_device_handle_ce(struct edac_device_ctl_info *edac_dev,
				int inst_nr, int block_nr, const char *msg);
extern int edac_device_alloc_index(void);


extern struct edac_pci_ctl_info *edac_pci_alloc_ctl_info(unsigned int sz_pvt,
				const char *edac_pci_name);

extern void edac_pci_free_ctl_info(struct edac_pci_ctl_info *pci);

extern void edac_pci_reset_delay_period(struct edac_pci_ctl_info *pci,
				unsigned long value);

extern int edac_pci_alloc_index(void);
extern int edac_pci_add_device(struct edac_pci_ctl_info *pci, int edac_idx);
extern struct edac_pci_ctl_info *edac_pci_del_device(struct device *dev);

extern struct edac_pci_ctl_info *edac_pci_create_generic_ctl(
				struct device *dev,
				const char *mod_name);

extern void edac_pci_release_generic_ctl(struct edac_pci_ctl_info *pci);
extern int edac_pci_create_sysfs(struct edac_pci_ctl_info *pci);
extern void edac_pci_remove_sysfs(struct edac_pci_ctl_info *pci);


extern char *edac_op_state_to_string(int op_state);

#endif				
