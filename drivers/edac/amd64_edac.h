

#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/slab.h>
#include <linux/mmzone.h>
#include <linux/edac.h>
#include <asm/msr.h>
#include "edac_core.h"
#include "edac_mce_amd.h"

#define amd64_printk(level, fmt, arg...) \
	edac_printk(level, "amd64", fmt, ##arg)

#define amd64_mc_printk(mci, level, fmt, arg...) \
	edac_mc_chipset_printk(mci, level, "amd64", fmt, ##arg)



#define EDAC_AMD64_VERSION		" Ver: 3.2.0 " __DATE__
#define EDAC_MOD_STR			"amd64_edac"

#define EDAC_MAX_NUMNODES		8


#define OPTERON_CPU_LE_REV_C		0
#define OPTERON_CPU_REV_D		1
#define OPTERON_CPU_REV_E		2


#define OPTERON_CPU_REV_F		4
#define OPTERON_CPU_REV_FA		5


#define MAX_CS_COUNT			8
#define DRAM_REG_COUNT			8

#define ON true
#define OFF false





#define K8_DRAM_BASE_LOW		0x40
#define K8_DRAM_LIMIT_LOW		0x44
#define K8_DHAR				0xf0

#define DHAR_VALID			BIT(0)
#define F10_DRAM_MEM_HOIST_VALID	BIT(1)

#define DHAR_BASE_MASK			0xff000000
#define dhar_base(dhar)			(dhar & DHAR_BASE_MASK)

#define K8_DHAR_OFFSET_MASK		0x0000ff00
#define k8_dhar_offset(dhar)		((dhar & K8_DHAR_OFFSET_MASK) << 16)

#define F10_DHAR_OFFSET_MASK		0x0000ff80
					
#define f10_dhar_offset(dhar)		((dhar & F10_DHAR_OFFSET_MASK) << 16)



#define F10_DRAM_BASE_HIGH		0x140
#define F10_DRAM_LIMIT_HIGH		0x144



#define K8_DCSB0			0x40
#define F10_DCSB1			0x140

#define K8_DCSB_CS_ENABLE		BIT(0)
#define K8_DCSB_NPT_SPARE		BIT(1)
#define K8_DCSB_NPT_TESTFAIL		BIT(2)


#define REV_E_DCSB_BASE_BITS		(0xFFE0FE00ULL)
#define REV_E_DCS_SHIFT			4

#define REV_F_F1Xh_DCSB_BASE_BITS	(0x1FF83FE0ULL)
#define REV_F_F1Xh_DCS_SHIFT		8


#define REV_F_DCSB_BASE_BITS		(0x1FF83FE0ULL)
#define REV_F_DCS_SHIFT			8


#define K8_DCSM0			0x60
#define F10_DCSM1			0x160


#define REV_E_DCSM_MASK_BITS		0x3FE0FE00


#define REV_E_DCS_NOTUSED_BITS		0x01F01FFF


#define REV_F_F1Xh_DCSM_MASK_BITS	0x1FF83FE0


#define REV_F_F1Xh_DCS_NOTUSED_BITS	0x07C01FFF

#define DBAM0				0x80
#define DBAM1				0x180


#define DBAM_DIMM(i, reg)		((((reg) >> (4*i))) & 0xF)

#define DBAM_MAX_VALUE			11


#define F10_DCLR_0			0x90
#define F10_DCLR_1			0x190
#define REVE_WIDTH_128			BIT(16)
#define F10_WIDTH_128			BIT(11)


#define F10_DCHR_0			0x94
#define F10_DCHR_1			0x194

#define F10_DCHR_FOUR_RANK_DIMM		BIT(18)
#define F10_DCHR_Ddr3Mode		BIT(8)
#define F10_DCHR_MblMode		BIT(6)


#define F10_DCTL_SEL_LOW		0x110

#define dct_sel_baseaddr(pvt)    \
	((pvt->dram_ctl_select_low) & 0xFFFFF800)

#define dct_sel_interleave_addr(pvt)    \
	(((pvt->dram_ctl_select_low) >> 6) & 0x3)

enum {
	F10_DCTL_SEL_LOW_DctSelHiRngEn	= BIT(0),
	F10_DCTL_SEL_LOW_DctSelIntLvEn	= BIT(2),
	F10_DCTL_SEL_LOW_DctGangEn	= BIT(4),
	F10_DCTL_SEL_LOW_DctDatIntLv	= BIT(5),
	F10_DCTL_SEL_LOW_DramEnable	= BIT(8),
	F10_DCTL_SEL_LOW_MemCleared	= BIT(10),
};

#define    dct_high_range_enabled(pvt)    \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_DctSelHiRngEn)

#define dct_interleave_enabled(pvt)	   \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_DctSelIntLvEn)

#define dct_ganging_enabled(pvt)        \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_DctGangEn)

#define dct_data_intlv_enabled(pvt)    \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_DctDatIntLv)

#define dct_dram_enabled(pvt)    \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_DramEnable)

#define dct_memory_cleared(pvt)    \
	(pvt->dram_ctl_select_low & F10_DCTL_SEL_LOW_MemCleared)


#define F10_DCTL_SEL_HIGH		0x114



#define K8_NBCTL			0x40


#define K8_NBCTL_CECCEn			BIT(0)


#define K8_NBCTL_UECCEn			BIT(1)

#define K8_NBCFG			0x44
#define K8_NBCFG_CHIPKILL		BIT(23)
#define K8_NBCFG_ECC_ENABLE		BIT(22)

#define K8_NBSL				0x48



#define F10_NBSL_EXT_ERR_RES		0x0
#define F10_NBSL_EXT_ERR_ECC		0x8


#define F10_NBSL_EXT_ERR_LINK_PROTO	0xB
#define F10_NBSL_EXT_ERR_L3_PROTO	0xB

#define F10_NBSL_EXT_ERR_NB_ARRAY	0xC
#define F10_NBSL_EXT_ERR_DRAM_PARITY	0xD
#define F10_NBSL_EXT_ERR_LINK_RETRY	0xE


#define F10_NBSL_EXT_ERR_GART_WALK	0xF
#define F10_NBSL_EXT_ERR_DEV_WALK	0xF


#define F10_NBSL_EXT_ERR_L3_DATA	0x1C
#define F10_NBSL_EXT_ERR_L3_TAG		0x1D
#define F10_NBSL_EXT_ERR_L3_LRU		0x1E


#define K8_NBSL_EXT_ERR_ECC		0x0
#define K8_NBSL_EXT_ERR_CRC		0x1
#define K8_NBSL_EXT_ERR_SYNC		0x2
#define K8_NBSL_EXT_ERR_MST		0x3
#define K8_NBSL_EXT_ERR_TGT		0x4
#define K8_NBSL_EXT_ERR_GART		0x5
#define K8_NBSL_EXT_ERR_RMW		0x6
#define K8_NBSL_EXT_ERR_WDT		0x7
#define K8_NBSL_EXT_ERR_CHIPKILL_ECC	0x8
#define K8_NBSL_EXT_ERR_DRAM_PARITY	0xD


#define K8_NBSL_PP_SRC			0x0
#define K8_NBSL_PP_RES			0x1
#define K8_NBSL_PP_OBS			0x2
#define K8_NBSL_PP_GENERIC		0x3

#define EXTRACT_ERR_CPU_MAP(x)		((x) & 0xF)

#define K8_NBEAL			0x50
#define K8_NBEAH			0x54
#define K8_SCRCTRL			0x58

#define F10_NB_CFG_LOW			0x88
#define	F10_NB_CFG_LOW_ENABLE_EXT_CFG	BIT(14)

#define F10_NB_CFG_HIGH			0x8C

#define F10_ONLINE_SPARE		0xB0
#define F10_ONLINE_SPARE_SWAPDONE0(x)	((x) & BIT(1))
#define F10_ONLINE_SPARE_SWAPDONE1(x)	((x) & BIT(3))
#define F10_ONLINE_SPARE_BADDRAM_CS0(x) (((x) >> 4) & 0x00000007)
#define F10_ONLINE_SPARE_BADDRAM_CS1(x) (((x) >> 8) & 0x00000007)

#define F10_NB_ARRAY_ADDR		0xB8

#define F10_NB_ARRAY_DRAM_ECC		0x80000000


#define SET_NB_ARRAY_ADDRESS(section)	(((section) & 0x3) << 1)

#define F10_NB_ARRAY_DATA		0xBC

#define SET_NB_DRAM_INJECTION_WRITE(word, bits)  \
					(BIT(((word) & 0xF) + 20) | \
					BIT(17) | bits)

#define SET_NB_DRAM_INJECTION_READ(word, bits)  \
					(BIT(((word) & 0xF) + 20) | \
					BIT(16) |  bits)

#define K8_NBCAP			0xE8
#define K8_NBCAP_CORES			(BIT(12)|BIT(13))
#define K8_NBCAP_CHIPKILL		BIT(4)
#define K8_NBCAP_SECDED			BIT(3)
#define K8_NBCAP_8_NODE			BIT(2)
#define K8_NBCAP_DUAL_NODE		BIT(1)
#define K8_NBCAP_DCT_DUAL		BIT(0)


#define K8_MSR_MCGCTL_NBE		BIT(4)

#define K8_MSR_MC4CTL			0x0410
#define K8_MSR_MC4STAT			0x0411
#define K8_MSR_MC4ADDR			0x0412


static inline int get_node_id(struct pci_dev *pdev)
{
	return PCI_SLOT(pdev->devfn) - 0x18;
}

enum amd64_chipset_families {
	K8_CPUS = 0,
	F10_CPUS,
	F11_CPUS,
};


struct error_injection {
	u32	section;
	u32	word;
	u32	bit_map;
};

struct amd64_pvt {
	
	struct pci_dev *addr_f1_ctl;
	struct pci_dev *dram_f2_ctl;
	struct pci_dev *misc_f3_ctl;

	int mc_node_id;		
	int ext_model;		

	struct low_ops *ops;	

	int channel_count;

	
	u32 dclr0;		
	u32 dclr1;		
	u32 dchr0;		
	u32 dchr1;		
	u32 nbcap;		
	u32 nbcfg;		
	u32 ext_nbcfg;		
	u32 dhar;		
	u32 dbam0;		
	u32 dbam1;		

	
	u32 dcsb0[MAX_CS_COUNT];
	u32 dcsb1[MAX_CS_COUNT];

	
	u32 dcsm0[MAX_CS_COUNT];
	u32 dcsm1[MAX_CS_COUNT];

	
	u64 dram_base[DRAM_REG_COUNT];
	u64 dram_limit[DRAM_REG_COUNT];
	u8  dram_IntlvSel[DRAM_REG_COUNT];
	u8  dram_IntlvEn[DRAM_REG_COUNT];
	u8  dram_DstNode[DRAM_REG_COUNT];
	u8  dram_rw_en[DRAM_REG_COUNT];

	
	u32 dcsb_base;		
	u32 dcsm_mask;		
	u32 cs_count;		
	u32 num_dcsm;		
	u32 dcs_mask_notused;	
	u32 dcs_shift;		

	u64 top_mem;		
	u64 top_mem2;		

	u32 dram_ctl_select_low;	
	u32 dram_ctl_select_high;	
	u32 online_spare;               

	
	struct err_regs ctl_error_info;

	
	struct error_injection injection;

	
	u32 nbctl_mcgctl_saved;		
	u32 old_nbctl;

	
	u32 mc_type_index;

	
	struct flags {
		unsigned long cf8_extcfg:1;
		unsigned long ecc_report:1;
	} flags;
};

struct scrubrate {
       u32 scrubval;           
       u32 bandwidth;          
};

extern struct scrubrate scrubrates[23];
extern u32 revf_quad_ddr2_shift[16];
extern const char *tt_msgs[4];
extern const char *ll_msgs[4];
extern const char *rrrr_msgs[16];
extern const char *to_msgs[2];
extern const char *pp_msgs[4];
extern const char *ii_msgs[4];
extern const char *ext_msgs[32];
extern const char *htlink_msgs[8];

#ifdef CONFIG_EDAC_DEBUG
#define NUM_DBG_ATTRS 9
#else
#define NUM_DBG_ATTRS 0
#endif

#ifdef CONFIG_EDAC_AMD64_ERROR_INJECTION
#define NUM_INJ_ATTRS 5
#else
#define NUM_INJ_ATTRS 0
#endif

extern struct mcidev_sysfs_attribute amd64_dbg_attrs[NUM_DBG_ATTRS],
				     amd64_inj_attrs[NUM_INJ_ATTRS];


struct low_ops {
	int (*probe_valid_hardware)(struct amd64_pvt *pvt);
	int (*early_channel_count)(struct amd64_pvt *pvt);

	u64 (*get_error_address)(struct mem_ctl_info *mci,
			struct err_regs *info);
	void (*read_dram_base_limit)(struct amd64_pvt *pvt, int dram);
	void (*read_dram_ctl_register)(struct amd64_pvt *pvt);
	void (*map_sysaddr_to_csrow)(struct mem_ctl_info *mci,
					struct err_regs *info,
					u64 SystemAddr);
	int (*dbam_map_to_pages)(struct amd64_pvt *pvt, int dram_map);
};

struct amd64_family_type {
	const char *ctl_name;
	u16 addr_f1_ctl;
	u16 misc_f3_ctl;
	struct low_ops ops;
};

static struct amd64_family_type amd64_family_types[];

static inline const char *get_amd_family_name(int index)
{
	return amd64_family_types[index].ctl_name;
}

static inline struct low_ops *family_ops(int index)
{
	return &amd64_family_types[index].ops;
}


#define K8_MIN_SCRUB_RATE_BITS	0x0
#define F10_MIN_SCRUB_RATE_BITS	0x5
#define F11_MIN_SCRUB_RATE_BITS	0x6

int amd64_get_dram_hole_info(struct mem_ctl_info *mci, u64 *hole_base,
			     u64 *hole_offset, u64 *hole_size);
