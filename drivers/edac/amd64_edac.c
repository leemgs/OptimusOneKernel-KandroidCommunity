#include "amd64_edac.h"
#include <asm/k8.h>

static struct edac_pci_ctl_info *amd64_ctl_pci;

static int report_gart_errors;
module_param(report_gart_errors, int, 0644);


static int ecc_enable_override;
module_param(ecc_enable_override, int, 0644);

static struct msr *msrs;


struct amd64_pvt;
static struct mem_ctl_info *mci_lookup[EDAC_MAX_NUMNODES];
static struct amd64_pvt *pvt_lookup[EDAC_MAX_NUMNODES];


u32 revf_quad_ddr2_shift[] = {
	0,	
	28,	
	29,	
	29,	
	29,	
	30,	
	30,	
	31,	
	31,	
	32,	
	32,	
	33,	
	0,	
	0,	
	0,	
	0	
};



struct scrubrate scrubrates[] = {
	{ 0x01, 1600000000UL},
	{ 0x02, 800000000UL},
	{ 0x03, 400000000UL},
	{ 0x04, 200000000UL},
	{ 0x05, 100000000UL},
	{ 0x06, 50000000UL},
	{ 0x07, 25000000UL},
	{ 0x08, 12284069UL},
	{ 0x09, 6274509UL},
	{ 0x0A, 3121951UL},
	{ 0x0B, 1560975UL},
	{ 0x0C, 781440UL},
	{ 0x0D, 390720UL},
	{ 0x0E, 195300UL},
	{ 0x0F, 97650UL},
	{ 0x10, 48854UL},
	{ 0x11, 24427UL},
	{ 0x12, 12213UL},
	{ 0x13, 6101UL},
	{ 0x14, 3051UL},
	{ 0x15, 1523UL},
	{ 0x16, 761UL},
	{ 0x00, 0UL},        
};




static int amd64_search_set_scrub_rate(struct pci_dev *ctl, u32 new_bw,
				       u32 min_scrubrate)
{
	u32 scrubval;
	int i;

	
	for (i = 0; i < ARRAY_SIZE(scrubrates); i++) {
		
		if (scrubrates[i].scrubval < min_scrubrate)
			continue;

		if (scrubrates[i].bandwidth <= new_bw)
			break;

		
	}

	scrubval = scrubrates[i].scrubval;
	if (scrubval)
		edac_printk(KERN_DEBUG, EDAC_MC,
			    "Setting scrub rate bandwidth: %u\n",
			    scrubrates[i].bandwidth);
	else
		edac_printk(KERN_DEBUG, EDAC_MC, "Turning scrubbing off.\n");

	pci_write_bits32(ctl, K8_SCRCTRL, scrubval, 0x001F);

	return 0;
}

static int amd64_set_scrub_rate(struct mem_ctl_info *mci, u32 *bandwidth)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u32 min_scrubrate = 0x0;

	switch (boot_cpu_data.x86) {
	case 0xf:
		min_scrubrate = K8_MIN_SCRUB_RATE_BITS;
		break;
	case 0x10:
		min_scrubrate = F10_MIN_SCRUB_RATE_BITS;
		break;
	case 0x11:
		min_scrubrate = F11_MIN_SCRUB_RATE_BITS;
		break;

	default:
		amd64_printk(KERN_ERR, "Unsupported family!\n");
		break;
	}
	return amd64_search_set_scrub_rate(pvt->misc_f3_ctl, *bandwidth,
			min_scrubrate);
}

static int amd64_get_scrub_rate(struct mem_ctl_info *mci, u32 *bw)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u32 scrubval = 0;
	int status = -1, i, ret = 0;

	ret = pci_read_config_dword(pvt->misc_f3_ctl, K8_SCRCTRL, &scrubval);
	if (ret)
		debugf0("Reading K8_SCRCTRL failed\n");

	scrubval = scrubval & 0x001F;

	edac_printk(KERN_DEBUG, EDAC_MC,
		    "pci-read, sdram scrub control value: %d \n", scrubval);

	for (i = 0; ARRAY_SIZE(scrubrates); i++) {
		if (scrubrates[i].scrubval == scrubval) {
			*bw = scrubrates[i].bandwidth;
			status = 0;
			break;
		}
	}

	return status;
}


static inline u32 amd64_map_to_dcs_mask(struct amd64_pvt *pvt, int csrow)
{
	if (boot_cpu_data.x86 == 0xf && pvt->ext_model < OPTERON_CPU_REV_F)
		return csrow;
	else
		return csrow >> 1;
}


static u32 amd64_get_dct_base(struct amd64_pvt *pvt, int dct, int csrow)
{
	if (dct == 0)
		return pvt->dcsb0[csrow];
	else
		return pvt->dcsb1[csrow];
}


static u32 amd64_get_dct_mask(struct amd64_pvt *pvt, int dct, int csrow)
{
	if (dct == 0)
		return pvt->dcsm0[amd64_map_to_dcs_mask(pvt, csrow)];
	else
		return pvt->dcsm1[amd64_map_to_dcs_mask(pvt, csrow)];
}



static void amd64_get_base_and_limit(struct amd64_pvt *pvt, int node_id,
			       u64 *base, u64 *limit)
{
	*base = pvt->dram_base[node_id];
	*limit = pvt->dram_limit[node_id];
}


static int amd64_base_limit_match(struct amd64_pvt *pvt,
					u64 sys_addr, int node_id)
{
	u64 base, limit, addr;

	amd64_get_base_and_limit(pvt, node_id, &base, &limit);

	
	addr = sys_addr & 0x000000ffffffffffull;

	return (addr >= base) && (addr <= limit);
}


static struct mem_ctl_info *find_mc_by_sys_addr(struct mem_ctl_info *mci,
						u64 sys_addr)
{
	struct amd64_pvt *pvt;
	int node_id;
	u32 intlv_en, bits;

	
	pvt = mci->pvt_info;

	
	intlv_en = pvt->dram_IntlvEn[0];

	if (intlv_en == 0) {
		for (node_id = 0; node_id < DRAM_REG_COUNT; node_id++) {
			if (amd64_base_limit_match(pvt, sys_addr, node_id))
				goto found;
		}
		goto err_no_match;
	}

	if (unlikely((intlv_en != 0x01) &&
		     (intlv_en != 0x03) &&
		     (intlv_en != 0x07))) {
		amd64_printk(KERN_WARNING, "junk value of 0x%x extracted from "
			     "IntlvEn field of DRAM Base Register for node 0: "
			     "this probably indicates a BIOS bug.\n", intlv_en);
		return NULL;
	}

	bits = (((u32) sys_addr) >> 12) & intlv_en;

	for (node_id = 0; ; ) {
		if ((pvt->dram_IntlvSel[node_id] & intlv_en) == bits)
			break;	

		if (++node_id >= DRAM_REG_COUNT)
			goto err_no_match;
	}

	
	if (unlikely(!amd64_base_limit_match(pvt, sys_addr, node_id))) {
		amd64_printk(KERN_WARNING,
			     "%s(): sys_addr 0x%llx falls outside base/limit "
			     "address range for node %d with node interleaving "
			     "enabled.\n",
			     __func__, sys_addr, node_id);
		return NULL;
	}

found:
	return edac_mc_find(node_id);

err_no_match:
	debugf2("sys_addr 0x%lx doesn't match any node\n",
		(unsigned long)sys_addr);

	return NULL;
}


static u64 base_from_dct_base(struct amd64_pvt *pvt, int csrow)
{
	return ((u64) (amd64_get_dct_base(pvt, 0, csrow) & pvt->dcsb_base)) <<
				pvt->dcs_shift;
}


static u64 mask_from_dct_mask(struct amd64_pvt *pvt, int csrow)
{
	u64 dcsm_bits, other_bits;
	u64 mask;

	
	dcsm_bits = amd64_get_dct_mask(pvt, 0, csrow) & pvt->dcsm_mask;

	other_bits = pvt->dcsm_mask;
	other_bits = ~(other_bits << pvt->dcs_shift);

	
	mask = (dcsm_bits << pvt->dcs_shift) | other_bits;

	return mask;
}


static int input_addr_to_csrow(struct mem_ctl_info *mci, u64 input_addr)
{
	struct amd64_pvt *pvt;
	int csrow;
	u64 base, mask;

	pvt = mci->pvt_info;

	
	for (csrow = 0; csrow < pvt->cs_count; csrow++) {

		
		if ((pvt->dcsb0[csrow] & K8_DCSB_CS_ENABLE) == 0)
			continue;

		base = base_from_dct_base(pvt, csrow);
		mask = ~mask_from_dct_mask(pvt, csrow);

		if ((input_addr & mask) == (base & mask)) {
			debugf2("InputAddr 0x%lx matches csrow %d (node %d)\n",
				(unsigned long)input_addr, csrow,
				pvt->mc_node_id);

			return csrow;
		}
	}

	debugf2("no matching csrow for InputAddr 0x%lx (MC node %d)\n",
		(unsigned long)input_addr, pvt->mc_node_id);

	return -1;
}


static inline u64 get_dram_base(struct mem_ctl_info *mci)
{
	struct amd64_pvt *pvt = mci->pvt_info;

	return pvt->dram_base[pvt->mc_node_id];
}


int amd64_get_dram_hole_info(struct mem_ctl_info *mci, u64 *hole_base,
			     u64 *hole_offset, u64 *hole_size)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u64 base;

	
	if (boot_cpu_data.x86 == 0xf && pvt->ext_model < OPTERON_CPU_REV_E) {
		debugf1("  revision %d for node %d does not support DHAR\n",
			pvt->ext_model, pvt->mc_node_id);
		return 1;
	}

	
	if (boot_cpu_data.x86 == 0x10 &&
	    (pvt->dhar & F10_DRAM_MEM_HOIST_VALID) == 0) {
		debugf1("  Dram Memory Hoisting is DISABLED on this system\n");
		return 1;
	}

	if ((pvt->dhar & DHAR_VALID) == 0) {
		debugf1("  Dram Memory Hoisting is DISABLED on this node %d\n",
			pvt->mc_node_id);
		return 1;
	}

	

	

	base = dhar_base(pvt->dhar);

	*hole_base = base;
	*hole_size = (0x1ull << 32) - base;

	if (boot_cpu_data.x86 > 0xf)
		*hole_offset = f10_dhar_offset(pvt->dhar);
	else
		*hole_offset = k8_dhar_offset(pvt->dhar);

	debugf1("  DHAR info for node %d base 0x%lx offset 0x%lx size 0x%lx\n",
		pvt->mc_node_id, (unsigned long)*hole_base,
		(unsigned long)*hole_offset, (unsigned long)*hole_size);

	return 0;
}
EXPORT_SYMBOL_GPL(amd64_get_dram_hole_info);


static u64 sys_addr_to_dram_addr(struct mem_ctl_info *mci, u64 sys_addr)
{
	u64 dram_base, hole_base, hole_offset, hole_size, dram_addr;
	int ret = 0;

	dram_base = get_dram_base(mci);

	ret = amd64_get_dram_hole_info(mci, &hole_base, &hole_offset,
				      &hole_size);
	if (!ret) {
		if ((sys_addr >= (1ull << 32)) &&
		    (sys_addr < ((1ull << 32) + hole_size))) {
			
			dram_addr = sys_addr - hole_offset;

			debugf2("using DHAR to translate SysAddr 0x%lx to "
				"DramAddr 0x%lx\n",
				(unsigned long)sys_addr,
				(unsigned long)dram_addr);

			return dram_addr;
		}
	}

	
	dram_addr = (sys_addr & 0xffffffffffull) - dram_base;

	debugf2("using DRAM Base register to translate SysAddr 0x%lx to "
		"DramAddr 0x%lx\n", (unsigned long)sys_addr,
		(unsigned long)dram_addr);
	return dram_addr;
}


static int num_node_interleave_bits(unsigned intlv_en)
{
	static const int intlv_shift_table[] = { 0, 1, 0, 2, 0, 0, 0, 3 };
	int n;

	BUG_ON(intlv_en > 7);
	n = intlv_shift_table[intlv_en];
	return n;
}


static u64 dram_addr_to_input_addr(struct mem_ctl_info *mci, u64 dram_addr)
{
	struct amd64_pvt *pvt;
	int intlv_shift;
	u64 input_addr;

	pvt = mci->pvt_info;

	
	intlv_shift = num_node_interleave_bits(pvt->dram_IntlvEn[0]);
	input_addr = ((dram_addr >> intlv_shift) & 0xffffff000ull) +
	    (dram_addr & 0xfff);

	debugf2("  Intlv Shift=%d DramAddr=0x%lx maps to InputAddr=0x%lx\n",
		intlv_shift, (unsigned long)dram_addr,
		(unsigned long)input_addr);

	return input_addr;
}


static u64 sys_addr_to_input_addr(struct mem_ctl_info *mci, u64 sys_addr)
{
	u64 input_addr;

	input_addr =
	    dram_addr_to_input_addr(mci, sys_addr_to_dram_addr(mci, sys_addr));

	debugf2("SysAdddr 0x%lx translates to InputAddr 0x%lx\n",
		(unsigned long)sys_addr, (unsigned long)input_addr);

	return input_addr;
}



static u64 input_addr_to_dram_addr(struct mem_ctl_info *mci, u64 input_addr)
{
	struct amd64_pvt *pvt;
	int node_id, intlv_shift;
	u64 bits, dram_addr;
	u32 intlv_sel;

	
	pvt = mci->pvt_info;
	node_id = pvt->mc_node_id;
	BUG_ON((node_id < 0) || (node_id > 7));

	intlv_shift = num_node_interleave_bits(pvt->dram_IntlvEn[0]);

	if (intlv_shift == 0) {
		debugf1("    InputAddr 0x%lx translates to DramAddr of "
			"same value\n",	(unsigned long)input_addr);

		return input_addr;
	}

	bits = ((input_addr & 0xffffff000ull) << intlv_shift) +
	    (input_addr & 0xfff);

	intlv_sel = pvt->dram_IntlvSel[node_id] & ((1 << intlv_shift) - 1);
	dram_addr = bits + (intlv_sel << 12);

	debugf1("InputAddr 0x%lx translates to DramAddr 0x%lx "
		"(%d node interleave bits)\n", (unsigned long)input_addr,
		(unsigned long)dram_addr, intlv_shift);

	return dram_addr;
}


static u64 dram_addr_to_sys_addr(struct mem_ctl_info *mci, u64 dram_addr)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u64 hole_base, hole_offset, hole_size, base, limit, sys_addr;
	int ret = 0;

	ret = amd64_get_dram_hole_info(mci, &hole_base, &hole_offset,
				      &hole_size);
	if (!ret) {
		if ((dram_addr >= hole_base) &&
		    (dram_addr < (hole_base + hole_size))) {
			sys_addr = dram_addr + hole_offset;

			debugf1("using DHAR to translate DramAddr 0x%lx to "
				"SysAddr 0x%lx\n", (unsigned long)dram_addr,
				(unsigned long)sys_addr);

			return sys_addr;
		}
	}

	amd64_get_base_and_limit(pvt, pvt->mc_node_id, &base, &limit);
	sys_addr = dram_addr + base;

	
	sys_addr |= ~((sys_addr & (1ull << 39)) - 1);

	debugf1("    Node %d, DramAddr 0x%lx to SysAddr 0x%lx\n",
		pvt->mc_node_id, (unsigned long)dram_addr,
		(unsigned long)sys_addr);

	return sys_addr;
}


static inline u64 input_addr_to_sys_addr(struct mem_ctl_info *mci,
					 u64 input_addr)
{
	return dram_addr_to_sys_addr(mci,
				     input_addr_to_dram_addr(mci, input_addr));
}


static void find_csrow_limits(struct mem_ctl_info *mci, int csrow,
			      u64 *input_addr_min, u64 *input_addr_max)
{
	struct amd64_pvt *pvt;
	u64 base, mask;

	pvt = mci->pvt_info;
	BUG_ON((csrow < 0) || (csrow >= pvt->cs_count));

	base = base_from_dct_base(pvt, csrow);
	mask = mask_from_dct_mask(pvt, csrow);

	*input_addr_min = base & ~mask;
	*input_addr_max = base | mask | pvt->dcs_mask_notused;
}


static u64 extract_error_address(struct mem_ctl_info *mci,
				 struct err_regs *info)
{
	struct amd64_pvt *pvt = mci->pvt_info;

	return pvt->ops->get_error_address(mci, info);
}



static inline void error_address_to_page_and_offset(u64 error_address,
						    u32 *page, u32 *offset)
{
	*page = (u32) (error_address >> PAGE_SHIFT);
	*offset = ((u32) error_address) & ~PAGE_MASK;
}


static int sys_addr_to_csrow(struct mem_ctl_info *mci, u64 sys_addr)
{
	int csrow;

	csrow = input_addr_to_csrow(mci, sys_addr_to_input_addr(mci, sys_addr));

	if (csrow == -1)
		amd64_mc_printk(mci, KERN_ERR,
			     "Failed to translate InputAddr to csrow for "
			     "address 0x%lx\n", (unsigned long)sys_addr);
	return csrow;
}

static int get_channel_from_ecc_syndrome(unsigned short syndrome);

static void amd64_cpu_display_info(struct amd64_pvt *pvt)
{
	if (boot_cpu_data.x86 == 0x11)
		edac_printk(KERN_DEBUG, EDAC_MC, "F11h CPU detected\n");
	else if (boot_cpu_data.x86 == 0x10)
		edac_printk(KERN_DEBUG, EDAC_MC, "F10h CPU detected\n");
	else if (boot_cpu_data.x86 == 0xf)
		edac_printk(KERN_DEBUG, EDAC_MC, "%s detected\n",
			(pvt->ext_model >= OPTERON_CPU_REV_F) ?
			"Rev F or later" : "Rev E or earlier");
	else
		
		edac_printk(KERN_ERR, EDAC_MC, "Unknown cpu!\n");
}


static enum edac_type amd64_determine_edac_cap(struct amd64_pvt *pvt)
{
	int bit;
	enum dev_type edac_cap = EDAC_FLAG_NONE;

	bit = (boot_cpu_data.x86 > 0xf || pvt->ext_model >= OPTERON_CPU_REV_F)
		? 19
		: 17;

	if (pvt->dclr0 & BIT(bit))
		edac_cap = EDAC_FLAG_SECDED;

	return edac_cap;
}


static void f10_debug_display_dimm_sizes(int ctrl, struct amd64_pvt *pvt,
					 int ganged);


static void amd64_dump_misc_regs(struct amd64_pvt *pvt)
{
	int ganged;

	debugf1("  nbcap:0x%8.08x DctDualCap=%s DualNode=%s 8-Node=%s\n",
		pvt->nbcap,
		(pvt->nbcap & K8_NBCAP_DCT_DUAL) ? "True" : "False",
		(pvt->nbcap & K8_NBCAP_DUAL_NODE) ? "True" : "False",
		(pvt->nbcap & K8_NBCAP_8_NODE) ? "True" : "False");
	debugf1("    ECC Capable=%s   ChipKill Capable=%s\n",
		(pvt->nbcap & K8_NBCAP_SECDED) ? "True" : "False",
		(pvt->nbcap & K8_NBCAP_CHIPKILL) ? "True" : "False");
	debugf1("  DramCfg0-low=0x%08x DIMM-ECC=%s Parity=%s Width=%s\n",
		pvt->dclr0,
		(pvt->dclr0 & BIT(19)) ?  "Enabled" : "Disabled",
		(pvt->dclr0 & BIT(8)) ?  "Enabled" : "Disabled",
		(pvt->dclr0 & BIT(11)) ?  "128b" : "64b");
	debugf1("    DIMM x4 Present: L0=%s L1=%s L2=%s L3=%s  DIMM Type=%s\n",
		(pvt->dclr0 & BIT(12)) ?  "Y" : "N",
		(pvt->dclr0 & BIT(13)) ?  "Y" : "N",
		(pvt->dclr0 & BIT(14)) ?  "Y" : "N",
		(pvt->dclr0 & BIT(15)) ?  "Y" : "N",
		(pvt->dclr0 & BIT(16)) ?  "UN-Buffered" : "Buffered");


	debugf1("  online-spare: 0x%8.08x\n", pvt->online_spare);

	if (boot_cpu_data.x86 == 0xf) {
		debugf1("  dhar: 0x%8.08x Base=0x%08x Offset=0x%08x\n",
			pvt->dhar, dhar_base(pvt->dhar),
			k8_dhar_offset(pvt->dhar));
		debugf1("      DramHoleValid=%s\n",
			(pvt->dhar & DHAR_VALID) ?  "True" : "False");

		debugf1("  dbam-dkt: 0x%8.08x\n", pvt->dbam0);

		
		return;

	} else {
		debugf1("  dhar: 0x%8.08x Base=0x%08x Offset=0x%08x\n",
			pvt->dhar, dhar_base(pvt->dhar),
			f10_dhar_offset(pvt->dhar));
		debugf1("    DramMemHoistValid=%s DramHoleValid=%s\n",
			(pvt->dhar & F10_DRAM_MEM_HOIST_VALID) ?
			"True" : "False",
			(pvt->dhar & DHAR_VALID) ?
			"True" : "False");
	}

	
	if (!dct_ganging_enabled(pvt)) {
		debugf1("  DramCfg1-low=0x%08x DIMM-ECC=%s Parity=%s "
			"Width=%s\n", pvt->dclr1,
			(pvt->dclr1 & BIT(19)) ?  "Enabled" : "Disabled",
			(pvt->dclr1 & BIT(8)) ?  "Enabled" : "Disabled",
			(pvt->dclr1 & BIT(11)) ?  "128b" : "64b");
		debugf1("    DIMM x4 Present: L0=%s L1=%s L2=%s L3=%s  "
			"DIMM Type=%s\n",
			(pvt->dclr1 & BIT(12)) ?  "Y" : "N",
			(pvt->dclr1 & BIT(13)) ?  "Y" : "N",
			(pvt->dclr1 & BIT(14)) ?  "Y" : "N",
			(pvt->dclr1 & BIT(15)) ?  "Y" : "N",
			(pvt->dclr1 & BIT(16)) ?  "UN-Buffered" : "Buffered");
	}

	
	ganged = dct_ganging_enabled(pvt);

	f10_debug_display_dimm_sizes(0, pvt, ganged);

	if (!ganged)
		f10_debug_display_dimm_sizes(1, pvt, ganged);
}


static void amd64_read_dbam_reg(struct amd64_pvt *pvt)
{
	int err = 0;
	unsigned int reg;

	reg = DBAM0;
	err = pci_read_config_dword(pvt->dram_f2_ctl, reg, &pvt->dbam0);
	if (err)
		goto err_reg;

	if (boot_cpu_data.x86 >= 0x10) {
		reg = DBAM1;
		err = pci_read_config_dword(pvt->dram_f2_ctl, reg, &pvt->dbam1);

		if (err)
			goto err_reg;
	}

	return;

err_reg:
	debugf0("Error reading F2x%03x.\n", reg);
}


static void amd64_set_dct_base_and_mask(struct amd64_pvt *pvt)
{

	if (boot_cpu_data.x86 == 0xf && pvt->ext_model < OPTERON_CPU_REV_F) {
		pvt->dcsb_base		= REV_E_DCSB_BASE_BITS;
		pvt->dcsm_mask		= REV_E_DCSM_MASK_BITS;
		pvt->dcs_mask_notused	= REV_E_DCS_NOTUSED_BITS;
		pvt->dcs_shift		= REV_E_DCS_SHIFT;
		pvt->cs_count		= 8;
		pvt->num_dcsm		= 8;
	} else {
		pvt->dcsb_base		= REV_F_F1Xh_DCSB_BASE_BITS;
		pvt->dcsm_mask		= REV_F_F1Xh_DCSM_MASK_BITS;
		pvt->dcs_mask_notused	= REV_F_F1Xh_DCS_NOTUSED_BITS;
		pvt->dcs_shift		= REV_F_F1Xh_DCS_SHIFT;

		if (boot_cpu_data.x86 == 0x11) {
			pvt->cs_count = 4;
			pvt->num_dcsm = 2;
		} else {
			pvt->cs_count = 8;
			pvt->num_dcsm = 4;
		}
	}
}


static void amd64_read_dct_base_mask(struct amd64_pvt *pvt)
{
	int cs, reg, err = 0;

	amd64_set_dct_base_and_mask(pvt);

	for (cs = 0; cs < pvt->cs_count; cs++) {
		reg = K8_DCSB0 + (cs * 4);
		err = pci_read_config_dword(pvt->dram_f2_ctl, reg,
						&pvt->dcsb0[cs]);
		if (unlikely(err))
			debugf0("Reading K8_DCSB0[%d] failed\n", cs);
		else
			debugf0("  DCSB0[%d]=0x%08x reg: F2x%x\n",
				cs, pvt->dcsb0[cs], reg);

		
		if (boot_cpu_data.x86 >= 0x10 && !dct_ganging_enabled(pvt)) {
			reg = F10_DCSB1 + (cs * 4);
			err = pci_read_config_dword(pvt->dram_f2_ctl, reg,
							&pvt->dcsb1[cs]);
			if (unlikely(err))
				debugf0("Reading F10_DCSB1[%d] failed\n", cs);
			else
				debugf0("  DCSB1[%d]=0x%08x reg: F2x%x\n",
					cs, pvt->dcsb1[cs], reg);
		} else {
			pvt->dcsb1[cs] = 0;
		}
	}

	for (cs = 0; cs < pvt->num_dcsm; cs++) {
		reg = K8_DCSM0 + (cs * 4);
		err = pci_read_config_dword(pvt->dram_f2_ctl, reg,
					&pvt->dcsm0[cs]);
		if (unlikely(err))
			debugf0("Reading K8_DCSM0 failed\n");
		else
			debugf0("    DCSM0[%d]=0x%08x reg: F2x%x\n",
				cs, pvt->dcsm0[cs], reg);

		
		if (boot_cpu_data.x86 >= 0x10 && !dct_ganging_enabled(pvt)) {
			reg = F10_DCSM1 + (cs * 4);
			err = pci_read_config_dword(pvt->dram_f2_ctl, reg,
					&pvt->dcsm1[cs]);
			if (unlikely(err))
				debugf0("Reading F10_DCSM1[%d] failed\n", cs);
			else
				debugf0("    DCSM1[%d]=0x%08x reg: F2x%x\n",
					cs, pvt->dcsm1[cs], reg);
		} else
			pvt->dcsm1[cs] = 0;
	}
}

static enum mem_type amd64_determine_memory_type(struct amd64_pvt *pvt)
{
	enum mem_type type;

	if (boot_cpu_data.x86 >= 0x10 || pvt->ext_model >= OPTERON_CPU_REV_F) {
		
		type = (pvt->dclr0 & BIT(16)) ? MEM_DDR2 : MEM_RDDR2;
	} else {
		
		type = (pvt->dclr0 & BIT(18)) ? MEM_DDR : MEM_RDDR;
	}

	debugf1("  Memory type is: %s\n",
		(type == MEM_DDR2) ? "MEM_DDR2" :
		(type == MEM_RDDR2) ? "MEM_RDDR2" :
		(type == MEM_DDR) ? "MEM_DDR" : "MEM_RDDR");

	return type;
}


static int k8_early_channel_count(struct amd64_pvt *pvt)
{
	int flag, err = 0;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCLR_0, &pvt->dclr0);
	if (err)
		return err;

	if ((boot_cpu_data.x86_model >> 4) >= OPTERON_CPU_REV_F) {
		
		flag = pvt->dclr0 & F10_WIDTH_128;
	} else {
		
		flag = pvt->dclr0 & REVE_WIDTH_128;
	}

	
	pvt->dclr1 = 0;

	return (flag) ? 2 : 1;
}


static u64 k8_get_error_address(struct mem_ctl_info *mci,
				struct err_regs *info)
{
	return (((u64) (info->nbeah & 0xff)) << 32) +
			(info->nbeal & ~0x03);
}


static void k8_read_dram_base_limit(struct amd64_pvt *pvt, int dram)
{
	u32 low;
	u32 off = dram << 3;	
	int err;

	err = pci_read_config_dword(pvt->addr_f1_ctl,
				    K8_DRAM_BASE_LOW + off, &low);
	if (err)
		debugf0("Reading K8_DRAM_BASE_LOW failed\n");

	
	pvt->dram_base[dram] = ((u64) low & 0xFFFF0000) << 8;
	pvt->dram_IntlvEn[dram] = (low >> 8) & 0x7;
	pvt->dram_rw_en[dram] = (low & 0x3);

	err = pci_read_config_dword(pvt->addr_f1_ctl,
				    K8_DRAM_LIMIT_LOW + off, &low);
	if (err)
		debugf0("Reading K8_DRAM_LIMIT_LOW failed\n");

	
	pvt->dram_limit[dram] = (((u64) low & 0xFFFF0000) << 8) | 0x00FFFFFF;
	pvt->dram_IntlvSel[dram] = (low >> 8) & 0x7;
	pvt->dram_DstNode[dram] = (low & 0x7);
}

static void k8_map_sysaddr_to_csrow(struct mem_ctl_info *mci,
					struct err_regs *info,
					u64 SystemAddress)
{
	struct mem_ctl_info *src_mci;
	unsigned short syndrome;
	int channel, csrow;
	u32 page, offset;

	
	syndrome  = HIGH_SYNDROME(info->nbsl) << 8;
	syndrome |= LOW_SYNDROME(info->nbsh);

	
	if (info->nbcfg & K8_NBCFG_CHIPKILL) {
		channel = get_channel_from_ecc_syndrome(syndrome);
		if (channel < 0) {
			
			amd64_mc_printk(mci, KERN_WARNING,
				       "unknown syndrome 0x%x - possible error "
				       "reporting race\n", syndrome);
			edac_mc_handle_ce_no_info(mci, EDAC_MOD_STR);
			return;
		}
	} else {
		
		channel = ((SystemAddress & BIT(3)) != 0);
	}

	
	src_mci = find_mc_by_sys_addr(mci, SystemAddress);
	if (!src_mci) {
		amd64_mc_printk(mci, KERN_ERR,
			     "failed to map error address 0x%lx to a node\n",
			     (unsigned long)SystemAddress);
		edac_mc_handle_ce_no_info(mci, EDAC_MOD_STR);
		return;
	}

	
	csrow = sys_addr_to_csrow(src_mci, SystemAddress);
	if (csrow < 0) {
		edac_mc_handle_ce_no_info(src_mci, EDAC_MOD_STR);
	} else {
		error_address_to_page_and_offset(SystemAddress, &page, &offset);

		edac_mc_handle_ce(src_mci, page, offset, syndrome, csrow,
				  channel, EDAC_MOD_STR);
	}
}


static int k8_dbam_map_to_pages(struct amd64_pvt *pvt, int dram_map)
{
	int nr_pages;

	if (pvt->ext_model >= OPTERON_CPU_REV_F) {
		nr_pages = 1 << (revf_quad_ddr2_shift[dram_map] - PAGE_SHIFT);
	} else {
		
		dram_map -= (pvt->ext_model >= OPTERON_CPU_REV_D) ?
				(dram_map > 8 ? 4 : (dram_map > 5 ?
				3 : (dram_map > 2 ? 1 : 0))) : 0;

		
		nr_pages = 1 << (dram_map + 25 - PAGE_SHIFT);
	}

	return nr_pages;
}


static int f10_early_channel_count(struct amd64_pvt *pvt)
{
	int dbams[] = { DBAM0, DBAM1 };
	int err = 0, channels = 0;
	int i, j;
	u32 dbam;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCLR_0, &pvt->dclr0);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCLR_1, &pvt->dclr1);
	if (err)
		goto err_reg;

	
	if (pvt->dclr0 & F10_WIDTH_128) {
		debugf0("Data WIDTH is 128 bits - 2 channels\n");
		channels = 2;
		return channels;
	}

	
	debugf0("Data WIDTH is NOT 128 bits - need more decoding\n");

	
	for (i = 0; i < ARRAY_SIZE(dbams); i++) {
		err = pci_read_config_dword(pvt->dram_f2_ctl, dbams[i], &dbam);
		if (err)
			goto err_reg;

		for (j = 0; j < 4; j++) {
			if (DBAM_DIMM(j, dbam) > 0) {
				channels++;
				break;
			}
		}
	}

	debugf0("MCT channel count: %d\n", channels);

	return channels;

err_reg:
	return -1;

}

static int f10_dbam_map_to_pages(struct amd64_pvt *pvt, int dram_map)
{
	return 1 << (revf_quad_ddr2_shift[dram_map] - PAGE_SHIFT);
}


static void amd64_setup(struct amd64_pvt *pvt)
{
	u32 reg;

	pci_read_config_dword(pvt->misc_f3_ctl, F10_NB_CFG_HIGH, &reg);

	pvt->flags.cf8_extcfg = !!(reg & F10_NB_CFG_LOW_ENABLE_EXT_CFG);
	reg |= F10_NB_CFG_LOW_ENABLE_EXT_CFG;
	pci_write_config_dword(pvt->misc_f3_ctl, F10_NB_CFG_HIGH, reg);
}


static void amd64_teardown(struct amd64_pvt *pvt)
{
	u32 reg;

	pci_read_config_dword(pvt->misc_f3_ctl, F10_NB_CFG_HIGH, &reg);

	reg &= ~F10_NB_CFG_LOW_ENABLE_EXT_CFG;
	if (pvt->flags.cf8_extcfg)
		reg |= F10_NB_CFG_LOW_ENABLE_EXT_CFG;
	pci_write_config_dword(pvt->misc_f3_ctl, F10_NB_CFG_HIGH, reg);
}

static u64 f10_get_error_address(struct mem_ctl_info *mci,
			struct err_regs *info)
{
	return (((u64) (info->nbeah & 0xffff)) << 32) +
			(info->nbeal & ~0x01);
}


static void f10_read_dram_base_limit(struct amd64_pvt *pvt, int dram)
{
	u32 high_offset, low_offset, high_base, low_base, high_limit, low_limit;

	low_offset = K8_DRAM_BASE_LOW + (dram << 3);
	high_offset = F10_DRAM_BASE_HIGH + (dram << 3);

	
	pci_read_config_dword(pvt->addr_f1_ctl, low_offset, &low_base);

	
	pci_read_config_dword(pvt->addr_f1_ctl, high_offset, &high_base);

	
	pvt->dram_rw_en[dram] = (low_base & 0x3);

	if (pvt->dram_rw_en[dram] == 0)
		return;

	pvt->dram_IntlvEn[dram] = (low_base >> 8) & 0x7;

	pvt->dram_base[dram] = (((u64)high_base & 0x000000FF) << 40) |
			       (((u64)low_base  & 0xFFFF0000) << 8);

	low_offset = K8_DRAM_LIMIT_LOW + (dram << 3);
	high_offset = F10_DRAM_LIMIT_HIGH + (dram << 3);

	
	pci_read_config_dword(pvt->addr_f1_ctl, low_offset, &low_limit);

	
	pci_read_config_dword(pvt->addr_f1_ctl, high_offset, &high_limit);

	debugf0("  HW Regs: BASE=0x%08x-%08x      LIMIT=  0x%08x-%08x\n",
		high_base, low_base, high_limit, low_limit);

	pvt->dram_DstNode[dram] = (low_limit & 0x7);
	pvt->dram_IntlvSel[dram] = (low_limit >> 8) & 0x7;

	
	pvt->dram_limit[dram] = (((u64)high_limit & 0x000000FF) << 40) |
				(((u64) low_limit & 0xFFFF0000) << 8) |
				0x00FFFFFF;
}

static void f10_read_dram_ctl_register(struct amd64_pvt *pvt)
{
	int err = 0;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCTL_SEL_LOW,
				    &pvt->dram_ctl_select_low);
	if (err) {
		debugf0("Reading F10_DCTL_SEL_LOW failed\n");
	} else {
		debugf0("DRAM_DCTL_SEL_LOW=0x%x  DctSelBaseAddr=0x%x\n",
			pvt->dram_ctl_select_low, dct_sel_baseaddr(pvt));

		debugf0("  DRAM DCTs are=%s DRAM Is=%s DRAM-Ctl-"
				"sel-hi-range=%s\n",
			(dct_ganging_enabled(pvt) ? "GANGED" : "NOT GANGED"),
			(dct_dram_enabled(pvt) ? "Enabled"   : "Disabled"),
			(dct_high_range_enabled(pvt) ? "Enabled" : "Disabled"));

		debugf0("  DctDatIntLv=%s MemCleared=%s DctSelIntLvAddr=0x%x\n",
			(dct_data_intlv_enabled(pvt) ? "Enabled" : "Disabled"),
			(dct_memory_cleared(pvt) ? "True " : "False "),
			dct_sel_interleave_addr(pvt));
	}

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCTL_SEL_HIGH,
				    &pvt->dram_ctl_select_high);
	if (err)
		debugf0("Reading F10_DCTL_SEL_HIGH failed\n");
}


static u32 f10_determine_channel(struct amd64_pvt *pvt, u64 sys_addr,
				int hi_range_sel, u32 intlv_en)
{
	u32 cs, temp, dct_sel_high = (pvt->dram_ctl_select_low >> 1) & 1;

	if (dct_ganging_enabled(pvt))
		cs = 0;
	else if (hi_range_sel)
		cs = dct_sel_high;
	else if (dct_interleave_enabled(pvt)) {
		
		if (dct_sel_interleave_addr(pvt) == 0)
			cs = sys_addr >> 6 & 1;
		else if ((dct_sel_interleave_addr(pvt) >> 1) & 1) {
			temp = hweight_long((u32) ((sys_addr >> 16) & 0x1F)) % 2;

			if (dct_sel_interleave_addr(pvt) & 1)
				cs = (sys_addr >> 9 & 1) ^ temp;
			else
				cs = (sys_addr >> 6 & 1) ^ temp;
		} else if (intlv_en & 4)
			cs = sys_addr >> 15 & 1;
		else if (intlv_en & 2)
			cs = sys_addr >> 14 & 1;
		else if (intlv_en & 1)
			cs = sys_addr >> 13 & 1;
		else
			cs = sys_addr >> 12 & 1;
	} else if (dct_high_range_enabled(pvt) && !dct_ganging_enabled(pvt))
		cs = ~dct_sel_high & 1;
	else
		cs = 0;

	return cs;
}

static inline u32 f10_map_intlv_en_to_shift(u32 intlv_en)
{
	if (intlv_en == 1)
		return 1;
	else if (intlv_en == 3)
		return 2;
	else if (intlv_en == 7)
		return 3;

	return 0;
}


static inline u64 f10_get_base_addr_offset(u64 sys_addr, int hi_range_sel,
						 u32 dct_sel_base_addr,
						 u64 dct_sel_base_off,
						 u32 hole_valid, u32 hole_off,
						 u64 dram_base)
{
	u64 chan_off;

	if (hi_range_sel) {
		if (!(dct_sel_base_addr & 0xFFFFF800) &&
		   hole_valid && (sys_addr >= 0x100000000ULL))
			chan_off = hole_off << 16;
		else
			chan_off = dct_sel_base_off;
	} else {
		if (hole_valid && (sys_addr >= 0x100000000ULL))
			chan_off = hole_off << 16;
		else
			chan_off = dram_base & 0xFFFFF8000000ULL;
	}

	return (sys_addr & 0x0000FFFFFFFFFFC0ULL) -
			(chan_off & 0x0000FFFFFF800000ULL);
}


#define	CH0SPARE_RANK	0
#define	CH1SPARE_RANK	1


static inline int f10_process_possible_spare(int csrow,
				u32 cs, struct amd64_pvt *pvt)
{
	u32 swap_done;
	u32 bad_dram_cs;

	
	if (cs) {
		swap_done = F10_ONLINE_SPARE_SWAPDONE1(pvt->online_spare);
		bad_dram_cs = F10_ONLINE_SPARE_BADDRAM_CS1(pvt->online_spare);
		if (swap_done && (csrow == bad_dram_cs))
			csrow = CH1SPARE_RANK;
	} else {
		swap_done = F10_ONLINE_SPARE_SWAPDONE0(pvt->online_spare);
		bad_dram_cs = F10_ONLINE_SPARE_BADDRAM_CS0(pvt->online_spare);
		if (swap_done && (csrow == bad_dram_cs))
			csrow = CH0SPARE_RANK;
	}
	return csrow;
}


static int f10_lookup_addr_in_dct(u32 in_addr, u32 nid, u32 cs)
{
	struct mem_ctl_info *mci;
	struct amd64_pvt *pvt;
	u32 cs_base, cs_mask;
	int cs_found = -EINVAL;
	int csrow;

	mci = mci_lookup[nid];
	if (!mci)
		return cs_found;

	pvt = mci->pvt_info;

	debugf1("InputAddr=0x%x  channelselect=%d\n", in_addr, cs);

	for (csrow = 0; csrow < pvt->cs_count; csrow++) {

		cs_base = amd64_get_dct_base(pvt, cs, csrow);
		if (!(cs_base & K8_DCSB_CS_ENABLE))
			continue;

		
		cs_base &= REV_F_F1Xh_DCSB_BASE_BITS;

		
		cs_mask = amd64_get_dct_mask(pvt, cs, csrow);

		debugf1("    CSROW=%d CSBase=0x%x RAW CSMask=0x%x\n",
				csrow, cs_base, cs_mask);

		cs_mask = (cs_mask | 0x0007C01F) & 0x1FFFFFFF;

		debugf1("              Final CSMask=0x%x\n", cs_mask);
		debugf1("    (InputAddr & ~CSMask)=0x%x "
				"(CSBase & ~CSMask)=0x%x\n",
				(in_addr & ~cs_mask), (cs_base & ~cs_mask));

		if ((in_addr & ~cs_mask) == (cs_base & ~cs_mask)) {
			cs_found = f10_process_possible_spare(csrow, cs, pvt);

			debugf1(" MATCH csrow=%d\n", cs_found);
			break;
		}
	}
	return cs_found;
}


static int f10_match_to_this_node(struct amd64_pvt *pvt, int dram_range,
				  u64 sys_addr, int *nid, int *chan_sel)
{
	int node_id, cs_found = -EINVAL, high_range = 0;
	u32 intlv_en, intlv_sel, intlv_shift, hole_off;
	u32 hole_valid, tmp, dct_sel_base, channel;
	u64 dram_base, chan_addr, dct_sel_base_off;

	dram_base = pvt->dram_base[dram_range];
	intlv_en = pvt->dram_IntlvEn[dram_range];

	node_id = pvt->dram_DstNode[dram_range];
	intlv_sel = pvt->dram_IntlvSel[dram_range];

	debugf1("(dram=%d) Base=0x%llx SystemAddr= 0x%llx Limit=0x%llx\n",
		dram_range, dram_base, sys_addr, pvt->dram_limit[dram_range]);

	
	hole_off = (pvt->dhar & 0x0000FF80);
	hole_valid = (pvt->dhar & 0x1);
	dct_sel_base_off = (pvt->dram_ctl_select_high & 0xFFFFFC00) << 16;

	debugf1("   HoleOffset=0x%x  HoleValid=0x%x IntlvSel=0x%x\n",
			hole_off, hole_valid, intlv_sel);

	if (intlv_en ||
	    (intlv_sel != ((sys_addr >> 12) & intlv_en)))
		return -EINVAL;

	dct_sel_base = dct_sel_baseaddr(pvt);

	
	if (dct_high_range_enabled(pvt) &&
	   !dct_ganging_enabled(pvt) &&
	   ((sys_addr >> 27) >= (dct_sel_base >> 11)))
		high_range = 1;

	channel = f10_determine_channel(pvt, sys_addr, high_range, intlv_en);

	chan_addr = f10_get_base_addr_offset(sys_addr, high_range, dct_sel_base,
					     dct_sel_base_off, hole_valid,
					     hole_off, dram_base);

	intlv_shift = f10_map_intlv_en_to_shift(intlv_en);

	
	tmp = chan_addr & 0xFC0;

	chan_addr = ((chan_addr >> intlv_shift) & 0xFFFFFFFFF000ULL) | tmp;

	
	if (dct_interleave_enabled(pvt) &&
	   !dct_high_range_enabled(pvt) &&
	   !dct_ganging_enabled(pvt)) {
		if (dct_sel_interleave_addr(pvt) != 1)
			chan_addr = (chan_addr >> 1) & 0xFFFFFFFFFFFFFFC0ULL;
		else {
			tmp = chan_addr & 0xFC0;
			chan_addr = ((chan_addr & 0xFFFFFFFFFFFFC000ULL) >> 1)
					| tmp;
		}
	}

	debugf1("   (ChannelAddrLong=0x%llx) >> 8 becomes InputAddr=0x%x\n",
		chan_addr, (u32)(chan_addr >> 8));

	cs_found = f10_lookup_addr_in_dct(chan_addr >> 8, node_id, channel);

	if (cs_found >= 0) {
		*nid = node_id;
		*chan_sel = channel;
	}
	return cs_found;
}

static int f10_translate_sysaddr_to_cs(struct amd64_pvt *pvt, u64 sys_addr,
				       int *node, int *chan_sel)
{
	int dram_range, cs_found = -EINVAL;
	u64 dram_base, dram_limit;

	for (dram_range = 0; dram_range < DRAM_REG_COUNT; dram_range++) {

		if (!pvt->dram_rw_en[dram_range])
			continue;

		dram_base = pvt->dram_base[dram_range];
		dram_limit = pvt->dram_limit[dram_range];

		if ((dram_base <= sys_addr) && (sys_addr <= dram_limit)) {

			cs_found = f10_match_to_this_node(pvt, dram_range,
							  sys_addr, node,
							  chan_sel);
			if (cs_found >= 0)
				break;
		}
	}
	return cs_found;
}


static void f10_map_sysaddr_to_csrow(struct mem_ctl_info *mci,
				     struct err_regs *info,
				     u64 sys_addr)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u32 page, offset;
	unsigned short syndrome;
	int nid, csrow, chan = 0;

	csrow = f10_translate_sysaddr_to_cs(pvt, sys_addr, &nid, &chan);

	if (csrow >= 0) {
		error_address_to_page_and_offset(sys_addr, &page, &offset);

		syndrome  = HIGH_SYNDROME(info->nbsl) << 8;
		syndrome |= LOW_SYNDROME(info->nbsh);

		
		if (pvt->nbcfg & K8_NBCFG_CHIPKILL)
			chan = get_channel_from_ecc_syndrome(syndrome);

		if (chan >= 0) {
			edac_mc_handle_ce(mci, page, offset, syndrome,
					csrow, chan, EDAC_MOD_STR);
		} else {
			
			for (chan = 0; chan < mci->csrows[csrow].nr_channels;
								chan++) {
					edac_mc_handle_ce(mci, page, offset,
							syndrome,
							csrow, chan,
							EDAC_MOD_STR);
			}
		}

	} else {
		edac_mc_handle_ce_no_info(mci, EDAC_MOD_STR);
	}
}


static int map_dbam_to_csrow_size(int index)
{
	int mega_bytes = 0;

	if (index > 0 && index <= DBAM_MAX_VALUE)
		mega_bytes = ((128 << (revf_quad_ddr2_shift[index]-27)));

	return mega_bytes;
}


static void f10_debug_display_dimm_sizes(int ctrl, struct amd64_pvt *pvt,
					 int ganged)
{
	int dimm, size0, size1;
	u32 dbam;
	u32 *dcsb;

	debugf1("  dbam%d: 0x%8.08x  CSROW is %s\n", ctrl,
			ctrl ? pvt->dbam1 : pvt->dbam0,
			ganged ? "GANGED - dbam1 not used" : "NON-GANGED");

	dbam = ctrl ? pvt->dbam1 : pvt->dbam0;
	dcsb = ctrl ? pvt->dcsb1 : pvt->dcsb0;

	
	for (dimm = 0; dimm < 4; dimm++) {

		size0 = 0;
		if (dcsb[dimm*2] & K8_DCSB_CS_ENABLE)
			size0 = map_dbam_to_csrow_size(DBAM_DIMM(dimm, dbam));

		size1 = 0;
		if (dcsb[dimm*2 + 1] & K8_DCSB_CS_ENABLE)
			size1 = map_dbam_to_csrow_size(DBAM_DIMM(dimm, dbam));

		debugf1("     CTRL-%d DIMM-%d=%5dMB   CSROW-%d=%5dMB "
				"CSROW-%d=%5dMB\n",
				ctrl,
				dimm,
				size0 + size1,
				dimm * 2,
				size0,
				dimm * 2 + 1,
				size1);
	}
}


static int f10_probe_valid_hardware(struct amd64_pvt *pvt)
{
	int ret = 0;

	
	if ((pvt->dchr0 & F10_DCHR_Ddr3Mode) ||
	    (pvt->dchr1 & F10_DCHR_Ddr3Mode)) {

		amd64_printk(KERN_WARNING,
			"%s() This machine is running with DDR3 memory. "
			"This is not currently supported. "
			"DCHR0=0x%x DCHR1=0x%x\n",
			__func__, pvt->dchr0, pvt->dchr1);

		amd64_printk(KERN_WARNING,
			"   Contact '%s' module MAINTAINER to help add"
			" support.\n",
			EDAC_MOD_STR);

		ret = 1;

	}
	return ret;
}


static struct amd64_family_type amd64_family_types[] = {
	[K8_CPUS] = {
		.ctl_name = "RevF",
		.addr_f1_ctl = PCI_DEVICE_ID_AMD_K8_NB_ADDRMAP,
		.misc_f3_ctl = PCI_DEVICE_ID_AMD_K8_NB_MISC,
		.ops = {
			.early_channel_count = k8_early_channel_count,
			.get_error_address = k8_get_error_address,
			.read_dram_base_limit = k8_read_dram_base_limit,
			.map_sysaddr_to_csrow = k8_map_sysaddr_to_csrow,
			.dbam_map_to_pages = k8_dbam_map_to_pages,
		}
	},
	[F10_CPUS] = {
		.ctl_name = "Family 10h",
		.addr_f1_ctl = PCI_DEVICE_ID_AMD_10H_NB_MAP,
		.misc_f3_ctl = PCI_DEVICE_ID_AMD_10H_NB_MISC,
		.ops = {
			.probe_valid_hardware = f10_probe_valid_hardware,
			.early_channel_count = f10_early_channel_count,
			.get_error_address = f10_get_error_address,
			.read_dram_base_limit = f10_read_dram_base_limit,
			.read_dram_ctl_register = f10_read_dram_ctl_register,
			.map_sysaddr_to_csrow = f10_map_sysaddr_to_csrow,
			.dbam_map_to_pages = f10_dbam_map_to_pages,
		}
	},
	[F11_CPUS] = {
		.ctl_name = "Family 11h",
		.addr_f1_ctl = PCI_DEVICE_ID_AMD_11H_NB_MAP,
		.misc_f3_ctl = PCI_DEVICE_ID_AMD_11H_NB_MISC,
		.ops = {
			.probe_valid_hardware = f10_probe_valid_hardware,
			.early_channel_count = f10_early_channel_count,
			.get_error_address = f10_get_error_address,
			.read_dram_base_limit = f10_read_dram_base_limit,
			.read_dram_ctl_register = f10_read_dram_ctl_register,
			.map_sysaddr_to_csrow = f10_map_sysaddr_to_csrow,
			.dbam_map_to_pages = f10_dbam_map_to_pages,
		}
	},
};

static struct pci_dev *pci_get_related_function(unsigned int vendor,
						unsigned int device,
						struct pci_dev *related)
{
	struct pci_dev *dev = NULL;

	dev = pci_get_device(vendor, device, dev);
	while (dev) {
		if ((dev->bus->number == related->bus->number) &&
		    (PCI_SLOT(dev->devfn) == PCI_SLOT(related->devfn)))
			break;
		dev = pci_get_device(vendor, device, dev);
	}

	return dev;
}


#define NUMBER_ECC_ROWS  36
static const unsigned short ecc_chipkill_syndromes[NUMBER_ECC_ROWS][16] = {
	
	{  0, 0xe821, 0x7c32, 0x9413, 0xbb44, 0x5365, 0xc776, 0x2f57,
	   0xdd88, 0x35a9, 0xa1ba, 0x499b, 0x66cc, 0x8eed, 0x1afe, 0xf2df },
	{  0, 0x5d31, 0xa612, 0xfb23, 0x9584, 0xc8b5, 0x3396, 0x6ea7,
	   0xeac8, 0xb7f9, 0x4cda, 0x11eb, 0x7f4c, 0x227d, 0xd95e, 0x846f },
	{  0, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	   0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f },
	{  0, 0x2021, 0x3032, 0x1013, 0x4044, 0x6065, 0x7076, 0x5057,
	   0x8088, 0xa0a9, 0xb0ba, 0x909b, 0xc0cc, 0xe0ed, 0xf0fe, 0xd0df },
	{  0, 0x5041, 0xa082, 0xf0c3, 0x9054, 0xc015, 0x30d6, 0x6097,
	   0xe0a8, 0xb0e9, 0x402a, 0x106b, 0x70fc, 0x20bd, 0xd07e, 0x803f },
	{  0, 0xbe21, 0xd732, 0x6913, 0x2144, 0x9f65, 0xf676, 0x4857,
	   0x3288, 0x8ca9, 0xe5ba, 0x5b9b, 0x13cc, 0xaded, 0xc4fe, 0x7adf },
	{  0, 0x4951, 0x8ea2, 0xc7f3, 0x5394, 0x1ac5, 0xdd36, 0x9467,
	   0xa1e8, 0xe8b9, 0x2f4a, 0x661b, 0xf27c, 0xbb2d, 0x7cde, 0x358f },
	{  0, 0x74e1, 0x9872, 0xec93, 0xd6b4, 0xa255, 0x4ec6, 0x3a27,
	   0x6bd8, 0x1f39, 0xf3aa, 0x874b, 0xbd6c, 0xc98d, 0x251e, 0x51ff },
	{  0, 0x15c1, 0x2a42, 0x3f83, 0xcef4, 0xdb35, 0xe4b6, 0xf177,
	   0x4758, 0x5299, 0x6d1a, 0x78db, 0x89ac, 0x9c6d, 0xa3ee, 0xb62f },
	{  0, 0x3d01, 0x1602, 0x2b03, 0x8504, 0xb805, 0x9306, 0xae07,
	   0xca08, 0xf709, 0xdc0a, 0xe10b, 0x4f0c, 0x720d, 0x590e, 0x640f },
	{  0, 0x9801, 0xec02, 0x7403, 0x6b04, 0xf305, 0x8706, 0x1f07,
	   0xbd08, 0x2509, 0x510a, 0xc90b, 0xd60c, 0x4e0d, 0x3a0e, 0xa20f },
	{  0, 0xd131, 0x6212, 0xb323, 0x3884, 0xe9b5, 0x5a96, 0x8ba7,
	   0x1cc8, 0xcdf9, 0x7eda, 0xafeb, 0x244c, 0xf57d, 0x465e, 0x976f },
	{  0, 0xe1d1, 0x7262, 0x93b3, 0xb834, 0x59e5, 0xca56, 0x2b87,
	   0xdc18, 0x3dc9, 0xae7a, 0x4fab, 0x542c, 0x85fd, 0x164e, 0xf79f },
	{  0, 0x6051, 0xb0a2, 0xd0f3, 0x1094, 0x70c5, 0xa036, 0xc067,
	   0x20e8, 0x40b9, 0x904a, 0x601b, 0x307c, 0x502d, 0x80de, 0xe08f },
	{  0, 0xa4c1, 0xf842, 0x5c83, 0xe6f4, 0x4235, 0x1eb6, 0xba77,
	   0x7b58, 0xdf99, 0x831a, 0x27db, 0x9dac, 0x396d, 0x65ee, 0xc12f },
	{  0, 0x11c1, 0x2242, 0x3383, 0xc8f4, 0xd935, 0xeab6, 0xfb77,
	   0x4c58, 0x5d99, 0x6e1a, 0x7fdb, 0x84ac, 0x956d, 0xa6ee, 0xb72f },

	
	{ 1, 0x45d1, 0x8a62, 0xcfb3, 0x5e34, 0x1be5, 0xd456, 0x9187,
	   0xa718, 0xe2c9, 0x2d7a, 0x68ab, 0xf92c, 0xbcfd, 0x734e, 0x369f },
	{ 1, 0x63e1, 0xb172, 0xd293, 0x14b4, 0x7755, 0xa5c6, 0xc627,
	   0x28d8, 0x4b39, 0x99aa, 0xfa4b, 0x3c6c, 0x5f8d, 0x8d1e, 0xeeff },
	{ 1, 0xb741, 0xd982, 0x6ec3, 0x2254, 0x9515, 0xfbd6, 0x4c97,
	   0x33a8, 0x84e9, 0xea2a, 0x5d6b, 0x11fc, 0xa6bd, 0xc87e, 0x7f3f },
	{ 1, 0xdd41, 0x6682, 0xbbc3, 0x3554, 0xe815, 0x53d6, 0xce97,
	   0x1aa8, 0xc7e9, 0x7c2a, 0xa1fb, 0x2ffc, 0xf2bd, 0x497e, 0x943f },
	{ 1, 0x2bd1, 0x3d62, 0x16b3, 0x4f34, 0x64e5, 0x7256, 0x5987,
	   0x8518, 0xaec9, 0xb87a, 0x93ab, 0xca2c, 0xe1fd, 0xf74e, 0xdc9f },
	{ 1, 0x83c1, 0xc142, 0x4283, 0xa4f4, 0x2735, 0x65b6, 0xe677,
	   0xf858, 0x7b99, 0x391a, 0xbadb, 0x5cac, 0xdf6d, 0x9dee, 0x1e2f },
	{ 1, 0x8fd1, 0xc562, 0x4ab3, 0xa934, 0x26e5, 0x6c56, 0xe387,
	   0xfe18, 0x71c9, 0x3b7a, 0xb4ab, 0x572c, 0xd8fd, 0x924e, 0x1d9f },
	{ 1, 0x4791, 0x89e2, 0xce73, 0x5264, 0x15f5, 0xdb86, 0x9c17,
	   0xa3b8, 0xe429, 0x2a5a, 0x6dcb, 0xf1dc, 0xb64d, 0x783e, 0x3faf },
	{ 1, 0x5781, 0xa9c2, 0xfe43, 0x92a4, 0xc525, 0x3b66, 0x6ce7,
	   0xe3f8, 0xb479, 0x4a3a, 0x1dbb, 0x715c, 0x26dd, 0xd89e, 0x8f1f },
	{ 1, 0xbf41, 0xd582, 0x6ac3, 0x2954, 0x9615, 0xfcd6, 0x4397,
	   0x3ea8, 0x81e9, 0xeb2a, 0x546b, 0x17fc, 0xa8bd, 0xc27e, 0x7d3f },
	{ 1, 0x9891, 0xe1e2, 0x7273, 0x6464, 0xf7f5, 0x8586, 0x1617,
	   0xb8b8, 0x2b29, 0x595a, 0xcacb, 0xdcdc, 0x4f4d, 0x3d3e, 0xaeaf },
	{ 1, 0xcce1, 0x4472, 0x8893, 0xfdb4, 0x3f55, 0xb9c6, 0x7527,
	   0x56d8, 0x9a39, 0x12aa, 0xde4b, 0xab6c, 0x678d, 0xef1e, 0x23ff },
	{ 1, 0xa761, 0xf9b2, 0x5ed3, 0xe214, 0x4575, 0x1ba6, 0xbcc7,
	   0x7328, 0xd449, 0x8a9a, 0x2dfb, 0x913c, 0x365d, 0x688e, 0xcfef },
	{ 1, 0xff61, 0x55b2, 0xaad3, 0x7914, 0x8675, 0x2ca6, 0xd3c7,
	   0x9e28, 0x6149, 0xcb9a, 0x34fb, 0xe73c, 0x185d, 0xb28e, 0x4def },
	{ 1, 0x5451, 0xa8a2, 0xfcf3, 0x9694, 0xc2c5, 0x3e36, 0x6a67,
	   0xebe8, 0xbfb9, 0x434a, 0x171b, 0x7d7c, 0x292d, 0xd5de, 0x818f },
	{ 1, 0x6fc1, 0xb542, 0xda83, 0x19f4, 0x7635, 0xacb6, 0xc377,
	   0x2e58, 0x4199, 0x9b1a, 0xf4db, 0x37ac, 0x586d, 0x82ee, 0xed2f },

	
	{ 0, 0xbe01, 0xd702, 0x6903, 0x2104, 0x9f05, 0xf606, 0x4807,
	   0x3208, 0x8c09, 0xe50a, 0x5b0b, 0x130c, 0xad0d, 0xc40e, 0x7a0f },
	{ 0, 0x4101, 0x8202, 0xc303, 0x5804, 0x1905, 0xda06, 0x9b07,
	   0xac08, 0xed09, 0x2e0a, 0x6f0b, 0x640c, 0xb50d, 0x760e, 0x370f },
	{ 1, 0xc441, 0x4882, 0x8cc3, 0xf654, 0x3215, 0xbed6, 0x7a97,
	   0x5ba8, 0x9fe9, 0x132a, 0xd76b, 0xadfc, 0x69bd, 0xe57e, 0x213f },
	{ 1, 0x7621, 0x9b32, 0xed13, 0xda44, 0xac65, 0x4176, 0x3757,
	   0x6f88, 0x19a9, 0xf4ba, 0x829b, 0xb5cc, 0xc3ed, 0x2efe, 0x58df }
};


static int get_channel_from_ecc_syndrome(unsigned short syndrome)
{
	int row;
	int column;

	
	column = syndrome & 0xF;

	
	for (row = 0; row < NUMBER_ECC_ROWS; row++) {
		if (ecc_chipkill_syndromes[row][column] == syndrome)
			return ecc_chipkill_syndromes[row][0];
	}

	debugf0("syndrome(%x) not found\n", syndrome);
	return -1;
}


static int amd64_get_error_info_regs(struct mem_ctl_info *mci,
				     struct err_regs *regs)
{
	struct amd64_pvt *pvt;
	struct pci_dev *misc_f3_ctl;
	int err = 0;

	pvt = mci->pvt_info;
	misc_f3_ctl = pvt->misc_f3_ctl;

	err = pci_read_config_dword(misc_f3_ctl, K8_NBSH, &regs->nbsh);
	if (err)
		goto err_reg;

	if (!(regs->nbsh & K8_NBSH_VALID_BIT))
		return 0;

	
	err = pci_read_config_dword(misc_f3_ctl, K8_NBSL, &regs->nbsl);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(misc_f3_ctl, K8_NBEAL, &regs->nbeal);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(misc_f3_ctl, K8_NBEAH, &regs->nbeah);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(misc_f3_ctl, K8_NBCFG, &regs->nbcfg);
	if (err)
		goto err_reg;

	return 1;

err_reg:
	debugf0("Reading error info register failed\n");
	return 0;
}


static int amd64_get_error_info(struct mem_ctl_info *mci,
				struct err_regs *info)
{
	struct amd64_pvt *pvt;
	struct err_regs regs;

	pvt = mci->pvt_info;

	if (!amd64_get_error_info_regs(mci, info))
		return 0;

	

	regs = *info;

	
	if (unlikely(!amd64_get_error_info_regs(mci, info)))
		return 0;

	
	pci_write_bits32(pvt->misc_f3_ctl, K8_NBSH, 0, K8_NBSH_VALID_BIT);

	
	if ((regs.nbsh != info->nbsh) ||
	     (regs.nbsl != info->nbsl) ||
	     (regs.nbeah != info->nbeah) ||
	     (regs.nbeal != info->nbeal)) {
		amd64_mc_printk(mci, KERN_WARNING,
				"hardware STATUS read access race condition "
				"detected!\n");
		return 0;
	}
	return 1;
}


static void amd64_handle_ce(struct mem_ctl_info *mci,
			    struct err_regs *info)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	u64 SystemAddress;

	
	if ((info->nbsh & K8_NBSH_VALID_ERROR_ADDR) == 0) {
		amd64_mc_printk(mci, KERN_ERR,
			"HW has no ERROR_ADDRESS available\n");
		edac_mc_handle_ce_no_info(mci, EDAC_MOD_STR);
		return;
	}

	SystemAddress = extract_error_address(mci, info);

	amd64_mc_printk(mci, KERN_ERR,
		"CE ERROR_ADDRESS= 0x%llx\n", SystemAddress);

	pvt->ops->map_sysaddr_to_csrow(mci, info, SystemAddress);
}


static void amd64_handle_ue(struct mem_ctl_info *mci,
			    struct err_regs *info)
{
	int csrow;
	u64 SystemAddress;
	u32 page, offset;
	struct mem_ctl_info *log_mci, *src_mci = NULL;

	log_mci = mci;

	if ((info->nbsh & K8_NBSH_VALID_ERROR_ADDR) == 0) {
		amd64_mc_printk(mci, KERN_CRIT,
			"HW has no ERROR_ADDRESS available\n");
		edac_mc_handle_ue_no_info(log_mci, EDAC_MOD_STR);
		return;
	}

	SystemAddress = extract_error_address(mci, info);

	
	src_mci = find_mc_by_sys_addr(mci, SystemAddress);
	if (!src_mci) {
		amd64_mc_printk(mci, KERN_CRIT,
			"ERROR ADDRESS (0x%lx) value NOT mapped to a MC\n",
			(unsigned long)SystemAddress);
		edac_mc_handle_ue_no_info(log_mci, EDAC_MOD_STR);
		return;
	}

	log_mci = src_mci;

	csrow = sys_addr_to_csrow(log_mci, SystemAddress);
	if (csrow < 0) {
		amd64_mc_printk(mci, KERN_CRIT,
			"ERROR_ADDRESS (0x%lx) value NOT mapped to 'csrow'\n",
			(unsigned long)SystemAddress);
		edac_mc_handle_ue_no_info(log_mci, EDAC_MOD_STR);
	} else {
		error_address_to_page_and_offset(SystemAddress, &page, &offset);
		edac_mc_handle_ue(log_mci, page, offset, csrow, EDAC_MOD_STR);
	}
}

static inline void __amd64_decode_bus_error(struct mem_ctl_info *mci,
					    struct err_regs *info)
{
	u32 ec  = ERROR_CODE(info->nbsl);
	u32 xec = EXT_ERROR_CODE(info->nbsl);
	int ecc_type = (info->nbsh >> 13) & 0x3;

	
	if (PP(ec) == K8_NBSL_PP_OBS)
		return;

	
	if (xec && xec != F10_NBSL_EXT_ERR_ECC)
		return;

	if (ecc_type == 2)
		amd64_handle_ce(mci, info);
	else if (ecc_type == 1)
		amd64_handle_ue(mci, info);

	
	if (info->nbsh & K8_NBSH_OVERFLOW)
		edac_mc_handle_ce_no_info(mci, EDAC_MOD_STR "Error Overflow");
}

void amd64_decode_bus_error(int node_id, struct err_regs *regs)
{
	struct mem_ctl_info *mci = mci_lookup[node_id];

	__amd64_decode_bus_error(mci, regs);

	
	if (regs->nbsh & K8_NBSH_UC_ERR && !report_gart_errors)
		edac_mc_handle_ue_no_info(mci, "UE bit is set");

}


static void amd64_check(struct mem_ctl_info *mci)
{
	struct err_regs regs;

	if (amd64_get_error_info(mci, &regs)) {
		struct amd64_pvt *pvt = mci->pvt_info;
		amd_decode_nb_mce(pvt->mc_node_id, &regs, 1);
	}
}


static int amd64_reserve_mc_sibling_devices(struct amd64_pvt *pvt, int mc_idx)
{
	const struct amd64_family_type *amd64_dev = &amd64_family_types[mc_idx];

	
	pvt->addr_f1_ctl = pci_get_related_function(pvt->dram_f2_ctl->vendor,
						    amd64_dev->addr_f1_ctl,
						    pvt->dram_f2_ctl);

	if (!pvt->addr_f1_ctl) {
		amd64_printk(KERN_ERR, "error address map device not found: "
			     "vendor %x device 0x%x (broken BIOS?)\n",
			     PCI_VENDOR_ID_AMD, amd64_dev->addr_f1_ctl);
		return 1;
	}

	
	pvt->misc_f3_ctl = pci_get_related_function(pvt->dram_f2_ctl->vendor,
						    amd64_dev->misc_f3_ctl,
						    pvt->dram_f2_ctl);

	if (!pvt->misc_f3_ctl) {
		pci_dev_put(pvt->addr_f1_ctl);
		pvt->addr_f1_ctl = NULL;

		amd64_printk(KERN_ERR, "error miscellaneous device not found: "
			     "vendor %x device 0x%x (broken BIOS?)\n",
			     PCI_VENDOR_ID_AMD, amd64_dev->misc_f3_ctl);
		return 1;
	}

	debugf1("    Addr Map device PCI Bus ID:\t%s\n",
		pci_name(pvt->addr_f1_ctl));
	debugf1("    DRAM MEM-CTL PCI Bus ID:\t%s\n",
		pci_name(pvt->dram_f2_ctl));
	debugf1("    Misc device PCI Bus ID:\t%s\n",
		pci_name(pvt->misc_f3_ctl));

	return 0;
}

static void amd64_free_mc_sibling_devices(struct amd64_pvt *pvt)
{
	pci_dev_put(pvt->addr_f1_ctl);
	pci_dev_put(pvt->misc_f3_ctl);
}


static void amd64_read_mc_registers(struct amd64_pvt *pvt)
{
	u64 msr_val;
	int dram, err = 0;

	
	rdmsrl(MSR_K8_TOP_MEM1, msr_val);
	pvt->top_mem = msr_val >> 23;
	debugf0("  TOP_MEM=0x%08llx\n", pvt->top_mem);

	
	rdmsrl(MSR_K8_SYSCFG, msr_val);
	if (msr_val & (1U << 21)) {
		rdmsrl(MSR_K8_TOP_MEM2, msr_val);
		pvt->top_mem2 = msr_val >> 23;
		debugf0("  TOP_MEM2=0x%08llx\n", pvt->top_mem2);
	} else
		debugf0("  TOP_MEM2 disabled.\n");

	amd64_cpu_display_info(pvt);

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCAP, &pvt->nbcap);
	if (err)
		goto err_reg;

	if (pvt->ops->read_dram_ctl_register)
		pvt->ops->read_dram_ctl_register(pvt);

	for (dram = 0; dram < DRAM_REG_COUNT; dram++) {
		
		pvt->ops->read_dram_base_limit(pvt, dram);

		
		if (pvt->dram_rw_en[dram] != 0) {
			debugf1("  DRAM_BASE[%d]: 0x%8.08x-%8.08x "
				"DRAM_LIMIT:  0x%8.08x-%8.08x\n",
				dram,
				(u32)(pvt->dram_base[dram] >> 32),
				(u32)(pvt->dram_base[dram] & 0xFFFFFFFF),
				(u32)(pvt->dram_limit[dram] >> 32),
				(u32)(pvt->dram_limit[dram] & 0xFFFFFFFF));
			debugf1("        IntlvEn=%s %s %s "
				"IntlvSel=%d DstNode=%d\n",
				pvt->dram_IntlvEn[dram] ?
					"Enabled" : "Disabled",
				(pvt->dram_rw_en[dram] & 0x2) ? "W" : "!W",
				(pvt->dram_rw_en[dram] & 0x1) ? "R" : "!R",
				pvt->dram_IntlvSel[dram],
				pvt->dram_DstNode[dram]);
		}
	}

	amd64_read_dct_base_mask(pvt);

	err = pci_read_config_dword(pvt->addr_f1_ctl, K8_DHAR, &pvt->dhar);
	if (err)
		goto err_reg;

	amd64_read_dbam_reg(pvt);

	err = pci_read_config_dword(pvt->misc_f3_ctl,
				F10_ONLINE_SPARE, &pvt->online_spare);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCLR_0, &pvt->dclr0);
	if (err)
		goto err_reg;

	err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCHR_0, &pvt->dchr0);
	if (err)
		goto err_reg;

	if (!dct_ganging_enabled(pvt)) {
		err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCLR_1,
						&pvt->dclr1);
		if (err)
			goto err_reg;

		err = pci_read_config_dword(pvt->dram_f2_ctl, F10_DCHR_1,
						&pvt->dchr1);
		if (err)
			goto err_reg;
	}

	amd64_dump_misc_regs(pvt);

	return;

err_reg:
	debugf0("Reading an MC register failed\n");

}


static u32 amd64_csrow_nr_pages(int csrow_nr, struct amd64_pvt *pvt)
{
	u32 dram_map, nr_pages;

	
	dram_map = (pvt->dbam0 >> ((csrow_nr / 2) * 4)) & 0xF;

	nr_pages = pvt->ops->dbam_map_to_pages(pvt, dram_map);

	
	nr_pages <<= (pvt->channel_count - 1);

	debugf0("  (csrow=%d) DBAM map index= %d\n", csrow_nr, dram_map);
	debugf0("    nr_pages= %u  channel-count = %d\n",
		nr_pages, pvt->channel_count);

	return nr_pages;
}


static int amd64_init_csrows(struct mem_ctl_info *mci)
{
	struct csrow_info *csrow;
	struct amd64_pvt *pvt;
	u64 input_addr_min, input_addr_max, sys_addr;
	int i, err = 0, empty = 1;

	pvt = mci->pvt_info;

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCFG, &pvt->nbcfg);
	if (err)
		debugf0("Reading K8_NBCFG failed\n");

	debugf0("NBCFG= 0x%x  CHIPKILL= %s DRAM ECC= %s\n", pvt->nbcfg,
		(pvt->nbcfg & K8_NBCFG_CHIPKILL) ? "Enabled" : "Disabled",
		(pvt->nbcfg & K8_NBCFG_ECC_ENABLE) ? "Enabled" : "Disabled"
		);

	for (i = 0; i < pvt->cs_count; i++) {
		csrow = &mci->csrows[i];

		if ((pvt->dcsb0[i] & K8_DCSB_CS_ENABLE) == 0) {
			debugf1("----CSROW %d EMPTY for node %d\n", i,
				pvt->mc_node_id);
			continue;
		}

		debugf1("----CSROW %d VALID for MC node %d\n",
			i, pvt->mc_node_id);

		empty = 0;
		csrow->nr_pages = amd64_csrow_nr_pages(i, pvt);
		find_csrow_limits(mci, i, &input_addr_min, &input_addr_max);
		sys_addr = input_addr_to_sys_addr(mci, input_addr_min);
		csrow->first_page = (u32) (sys_addr >> PAGE_SHIFT);
		sys_addr = input_addr_to_sys_addr(mci, input_addr_max);
		csrow->last_page = (u32) (sys_addr >> PAGE_SHIFT);
		csrow->page_mask = ~mask_from_dct_mask(pvt, i);
		

		csrow->mtype = amd64_determine_memory_type(pvt);

		debugf1("  for MC node %d csrow %d:\n", pvt->mc_node_id, i);
		debugf1("    input_addr_min: 0x%lx input_addr_max: 0x%lx\n",
			(unsigned long)input_addr_min,
			(unsigned long)input_addr_max);
		debugf1("    sys_addr: 0x%lx  page_mask: 0x%lx\n",
			(unsigned long)sys_addr, csrow->page_mask);
		debugf1("    nr_pages: %u  first_page: 0x%lx "
			"last_page: 0x%lx\n",
			(unsigned)csrow->nr_pages,
			csrow->first_page, csrow->last_page);

		
		if (pvt->nbcfg & K8_NBCFG_ECC_ENABLE)
			csrow->edac_mode =
			    (pvt->nbcfg & K8_NBCFG_CHIPKILL) ?
			    EDAC_S4ECD4ED : EDAC_SECDED;
		else
			csrow->edac_mode = EDAC_NONE;
	}

	return empty;
}


static void get_cpus_on_this_dct_cpumask(struct cpumask *mask, int nid)
{
	int cpu;

	for_each_online_cpu(cpu)
		if (amd_get_nb_id(cpu) == nid)
			cpumask_set_cpu(cpu, mask);
}


static bool amd64_nb_mce_bank_enabled_on_node(int nid)
{
	cpumask_var_t mask;
	int cpu, nbe;
	bool ret = false;

	if (!zalloc_cpumask_var(&mask, GFP_KERNEL)) {
		amd64_printk(KERN_WARNING, "%s: error allocating mask\n",
			     __func__);
		return false;
	}

	get_cpus_on_this_dct_cpumask(mask, nid);

	rdmsr_on_cpus(mask, MSR_IA32_MCG_CTL, msrs);

	for_each_cpu(cpu, mask) {
		struct msr *reg = per_cpu_ptr(msrs, cpu);
		nbe = reg->l & K8_MSR_MCGCTL_NBE;

		debugf0("core: %u, MCG_CTL: 0x%llx, NB MSR is %s\n",
			cpu, reg->q,
			(nbe ? "enabled" : "disabled"));

		if (!nbe)
			goto out;
	}
	ret = true;

out:
	free_cpumask_var(mask);
	return ret;
}

static int amd64_toggle_ecc_err_reporting(struct amd64_pvt *pvt, bool on)
{
	cpumask_var_t cmask;
	int cpu;

	if (!zalloc_cpumask_var(&cmask, GFP_KERNEL)) {
		amd64_printk(KERN_WARNING, "%s: error allocating mask\n",
			     __func__);
		return false;
	}

	get_cpus_on_this_dct_cpumask(cmask, pvt->mc_node_id);

	rdmsr_on_cpus(cmask, MSR_IA32_MCG_CTL, msrs);

	for_each_cpu(cpu, cmask) {

		struct msr *reg = per_cpu_ptr(msrs, cpu);

		if (on) {
			if (reg->l & K8_MSR_MCGCTL_NBE)
				pvt->flags.ecc_report = 1;

			reg->l |= K8_MSR_MCGCTL_NBE;
		} else {
			
			if (!pvt->flags.ecc_report)
				reg->l &= ~K8_MSR_MCGCTL_NBE;
		}
	}
	wrmsr_on_cpus(cmask, MSR_IA32_MCG_CTL, msrs);

	free_cpumask_var(cmask);

	return 0;
}


static void amd64_enable_ecc_error_reporting(struct mem_ctl_info *mci)
{
	struct amd64_pvt *pvt = mci->pvt_info;
	int err = 0;
	u32 value, mask = K8_NBCTL_CECCEn | K8_NBCTL_UECCEn;

	if (!ecc_enable_override)
		return;

	amd64_printk(KERN_WARNING,
		"'ecc_enable_override' parameter is active, "
		"Enabling AMD ECC hardware now: CAUTION\n");

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCTL, &value);
	if (err)
		debugf0("Reading K8_NBCTL failed\n");

	
	pvt->old_nbctl = value & mask;
	pvt->nbctl_mcgctl_saved = 1;

	value |= mask;
	pci_write_config_dword(pvt->misc_f3_ctl, K8_NBCTL, value);

	if (amd64_toggle_ecc_err_reporting(pvt, ON))
		amd64_printk(KERN_WARNING, "Error enabling ECC reporting over "
					   "MCGCTL!\n");

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCFG, &value);
	if (err)
		debugf0("Reading K8_NBCFG failed\n");

	debugf0("NBCFG(1)= 0x%x  CHIPKILL= %s ECC_ENABLE= %s\n", value,
		(value & K8_NBCFG_CHIPKILL) ? "Enabled" : "Disabled",
		(value & K8_NBCFG_ECC_ENABLE) ? "Enabled" : "Disabled");

	if (!(value & K8_NBCFG_ECC_ENABLE)) {
		amd64_printk(KERN_WARNING,
			"This node reports that DRAM ECC is "
			"currently Disabled; ENABLING now\n");

		
		value |= K8_NBCFG_ECC_ENABLE;
		pci_write_config_dword(pvt->misc_f3_ctl, K8_NBCFG, value);

		err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCFG, &value);
		if (err)
			debugf0("Reading K8_NBCFG failed\n");

		if (!(value & K8_NBCFG_ECC_ENABLE)) {
			amd64_printk(KERN_WARNING,
				"Hardware rejects Enabling DRAM ECC checking\n"
				"Check memory DIMM configuration\n");
		} else {
			amd64_printk(KERN_DEBUG,
				"Hardware accepted DRAM ECC Enable\n");
		}
	}
	debugf0("NBCFG(2)= 0x%x  CHIPKILL= %s ECC_ENABLE= %s\n", value,
		(value & K8_NBCFG_CHIPKILL) ? "Enabled" : "Disabled",
		(value & K8_NBCFG_ECC_ENABLE) ? "Enabled" : "Disabled");

	pvt->ctl_error_info.nbcfg = value;
}

static void amd64_restore_ecc_error_reporting(struct amd64_pvt *pvt)
{
	int err = 0;
	u32 value, mask = K8_NBCTL_CECCEn | K8_NBCTL_UECCEn;

	if (!pvt->nbctl_mcgctl_saved)
		return;

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCTL, &value);
	if (err)
		debugf0("Reading K8_NBCTL failed\n");
	value &= ~mask;
	value |= pvt->old_nbctl;

	
	pci_write_config_dword(pvt->misc_f3_ctl, K8_NBCTL, value);

	if (amd64_toggle_ecc_err_reporting(pvt, OFF))
		amd64_printk(KERN_WARNING, "Error restoring ECC reporting over "
					   "MCGCTL!\n");
}


static const char *ecc_msg =
	"ECC disabled in the BIOS or no ECC capability, module will not load.\n"
	" Either enable ECC checking or force module loading by setting "
	"'ecc_enable_override'.\n"
	" (Note that use of the override may cause unknown side effects.)\n";

static int amd64_check_ecc_enabled(struct amd64_pvt *pvt)
{
	u32 value;
	int err = 0;
	u8 ecc_enabled = 0;
	bool nb_mce_en = false;

	err = pci_read_config_dword(pvt->misc_f3_ctl, K8_NBCFG, &value);
	if (err)
		debugf0("Reading K8_NBCTL failed\n");

	ecc_enabled = !!(value & K8_NBCFG_ECC_ENABLE);
	if (!ecc_enabled)
		amd64_printk(KERN_NOTICE, "This node reports that Memory ECC "
			     "is currently disabled, set F3x%x[22] (%s).\n",
			     K8_NBCFG, pci_name(pvt->misc_f3_ctl));
	else
		amd64_printk(KERN_INFO, "ECC is enabled by BIOS.\n");

	nb_mce_en = amd64_nb_mce_bank_enabled_on_node(pvt->mc_node_id);
	if (!nb_mce_en)
		amd64_printk(KERN_NOTICE, "NB MCE bank disabled, set MSR "
			     "0x%08x[4] on node %d to enable.\n",
			     MSR_IA32_MCG_CTL, pvt->mc_node_id);

	if (!ecc_enabled || !nb_mce_en) {
		if (!ecc_enable_override) {
			amd64_printk(KERN_NOTICE, "%s", ecc_msg);
			return -ENODEV;
		}
		ecc_enable_override = 0;
	}

	return 0;
}

struct mcidev_sysfs_attribute sysfs_attrs[ARRAY_SIZE(amd64_dbg_attrs) +
					  ARRAY_SIZE(amd64_inj_attrs) +
					  1];

struct mcidev_sysfs_attribute terminator = { .attr = { .name = NULL } };

static void amd64_set_mc_sysfs_attributes(struct mem_ctl_info *mci)
{
	unsigned int i = 0, j = 0;

	for (; i < ARRAY_SIZE(amd64_dbg_attrs); i++)
		sysfs_attrs[i] = amd64_dbg_attrs[i];

	for (j = 0; j < ARRAY_SIZE(amd64_inj_attrs); j++, i++)
		sysfs_attrs[i] = amd64_inj_attrs[j];

	sysfs_attrs[i] = terminator;

	mci->mc_driver_sysfs_attributes = sysfs_attrs;
}

static void amd64_setup_mci_misc_attributes(struct mem_ctl_info *mci)
{
	struct amd64_pvt *pvt = mci->pvt_info;

	mci->mtype_cap		= MEM_FLAG_DDR2 | MEM_FLAG_RDDR2;
	mci->edac_ctl_cap	= EDAC_FLAG_NONE;

	if (pvt->nbcap & K8_NBCAP_SECDED)
		mci->edac_ctl_cap |= EDAC_FLAG_SECDED;

	if (pvt->nbcap & K8_NBCAP_CHIPKILL)
		mci->edac_ctl_cap |= EDAC_FLAG_S4ECD4ED;

	mci->edac_cap		= amd64_determine_edac_cap(pvt);
	mci->mod_name		= EDAC_MOD_STR;
	mci->mod_ver		= EDAC_AMD64_VERSION;
	mci->ctl_name		= get_amd_family_name(pvt->mc_type_index);
	mci->dev_name		= pci_name(pvt->dram_f2_ctl);
	mci->ctl_page_to_phys	= NULL;

	
	mci->edac_check		= amd64_check;

	
	mci->set_sdram_scrub_rate = amd64_set_scrub_rate;
	mci->get_sdram_scrub_rate = amd64_get_scrub_rate;
}


static int amd64_probe_one_instance(struct pci_dev *dram_f2_ctl,
				    int mc_type_index)
{
	struct amd64_pvt *pvt = NULL;
	int err = 0, ret;

	ret = -ENOMEM;
	pvt = kzalloc(sizeof(struct amd64_pvt), GFP_KERNEL);
	if (!pvt)
		goto err_exit;

	pvt->mc_node_id = get_node_id(dram_f2_ctl);

	pvt->dram_f2_ctl	= dram_f2_ctl;
	pvt->ext_model		= boot_cpu_data.x86_model >> 4;
	pvt->mc_type_index	= mc_type_index;
	pvt->ops		= family_ops(mc_type_index);

	
	ret = -ENODEV;
	err = amd64_reserve_mc_sibling_devices(pvt, mc_type_index);
	if (err)
		goto err_free;

	ret = -EINVAL;
	err = amd64_check_ecc_enabled(pvt);
	if (err)
		goto err_put;

	
	if (boot_cpu_data.x86 >= 0x10)
		amd64_setup(pvt);

	
	pvt_lookup[pvt->mc_node_id] = pvt;

	return 0;

err_put:
	amd64_free_mc_sibling_devices(pvt);

err_free:
	kfree(pvt);

err_exit:
	return ret;
}


static int amd64_init_2nd_stage(struct amd64_pvt *pvt)
{
	int node_id = pvt->mc_node_id;
	struct mem_ctl_info *mci;
	int ret, err = 0;

	amd64_read_mc_registers(pvt);

	ret = -ENODEV;
	if (pvt->ops->probe_valid_hardware) {
		err = pvt->ops->probe_valid_hardware(pvt);
		if (err)
			goto err_exit;
	}

	
	pvt->channel_count = pvt->ops->early_channel_count(pvt);
	if (pvt->channel_count < 0)
		goto err_exit;

	ret = -ENOMEM;
	mci = edac_mc_alloc(0, pvt->cs_count, pvt->channel_count, node_id);
	if (!mci)
		goto err_exit;

	mci->pvt_info = pvt;

	mci->dev = &pvt->dram_f2_ctl->dev;
	amd64_setup_mci_misc_attributes(mci);

	if (amd64_init_csrows(mci))
		mci->edac_cap = EDAC_FLAG_NONE;

	amd64_enable_ecc_error_reporting(mci);
	amd64_set_mc_sysfs_attributes(mci);

	ret = -ENODEV;
	if (edac_mc_add_mc(mci)) {
		debugf1("failed edac_mc_add_mc()\n");
		goto err_add_mc;
	}

	mci_lookup[node_id] = mci;
	pvt_lookup[node_id] = NULL;

	
	if (report_gart_errors)
		amd_report_gart_errors(true);

	amd_register_ecc_decoder(amd64_decode_bus_error);

	return 0;

err_add_mc:
	edac_mc_free(mci);

err_exit:
	debugf0("failure to init 2nd stage: ret=%d\n", ret);

	amd64_restore_ecc_error_reporting(pvt);

	if (boot_cpu_data.x86 > 0xf)
		amd64_teardown(pvt);

	amd64_free_mc_sibling_devices(pvt);

	kfree(pvt_lookup[pvt->mc_node_id]);
	pvt_lookup[node_id] = NULL;

	return ret;
}


static int __devinit amd64_init_one_instance(struct pci_dev *pdev,
				 const struct pci_device_id *mc_type)
{
	int ret = 0;

	debugf0("(MC node=%d,mc_type='%s')\n", get_node_id(pdev),
		get_amd_family_name(mc_type->driver_data));

	ret = pci_enable_device(pdev);
	if (ret < 0)
		ret = -EIO;
	else
		ret = amd64_probe_one_instance(pdev, mc_type->driver_data);

	if (ret < 0)
		debugf0("ret=%d\n", ret);

	return ret;
}

static void __devexit amd64_remove_one_instance(struct pci_dev *pdev)
{
	struct mem_ctl_info *mci;
	struct amd64_pvt *pvt;

	
	mci = edac_mc_del_mc(&pdev->dev);
	if (!mci)
		return;

	pvt = mci->pvt_info;

	amd64_restore_ecc_error_reporting(pvt);

	if (boot_cpu_data.x86 > 0xf)
		amd64_teardown(pvt);

	amd64_free_mc_sibling_devices(pvt);

	
	amd_report_gart_errors(false);
	amd_unregister_ecc_decoder(amd64_decode_bus_error);

	
	mci->pvt_info = NULL;
	mci_lookup[pvt->mc_node_id] = NULL;

	kfree(pvt);
	edac_mc_free(mci);
}


static const struct pci_device_id amd64_pci_table[] __devinitdata = {
	{
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_K8_NB_MEMCTL,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.class		= 0,
		.class_mask	= 0,
		.driver_data	= K8_CPUS
	},
	{
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_10H_NB_DRAM,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.class		= 0,
		.class_mask	= 0,
		.driver_data	= F10_CPUS
	},
	{
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_11H_NB_DRAM,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.class		= 0,
		.class_mask	= 0,
		.driver_data	= F11_CPUS
	},
	{0, }
};
MODULE_DEVICE_TABLE(pci, amd64_pci_table);

static struct pci_driver amd64_pci_driver = {
	.name		= EDAC_MOD_STR,
	.probe		= amd64_init_one_instance,
	.remove		= __devexit_p(amd64_remove_one_instance),
	.id_table	= amd64_pci_table,
};

static void amd64_setup_pci_device(void)
{
	struct mem_ctl_info *mci;
	struct amd64_pvt *pvt;

	if (amd64_ctl_pci)
		return;

	mci = mci_lookup[0];
	if (mci) {

		pvt = mci->pvt_info;
		amd64_ctl_pci =
			edac_pci_create_generic_ctl(&pvt->dram_f2_ctl->dev,
						    EDAC_MOD_STR);

		if (!amd64_ctl_pci) {
			pr_warning("%s(): Unable to create PCI control\n",
				   __func__);

			pr_warning("%s(): PCI error report via EDAC not set\n",
				   __func__);
			}
	}
}

static int __init amd64_edac_init(void)
{
	int nb, err = -ENODEV;
	bool load_ok = false;

	edac_printk(KERN_INFO, EDAC_MOD_STR, EDAC_AMD64_VERSION "\n");

	opstate_init();

	if (cache_k8_northbridges() < 0)
		goto err_ret;

	msrs = msrs_alloc();
	if (!msrs)
		goto err_ret;

	err = pci_register_driver(&amd64_pci_driver);
	if (err)
		goto err_pci;

	
	err = -ENODEV;
	for (nb = 0; nb < num_k8_northbridges; nb++) {
		if (!pvt_lookup[nb])
			continue;

		err = amd64_init_2nd_stage(pvt_lookup[nb]);
		if (err)
			goto err_2nd_stage;

		load_ok = true;
	}

	if (load_ok) {
		amd64_setup_pci_device();
		return 0;
	}

err_2nd_stage:
	pci_unregister_driver(&amd64_pci_driver);
err_pci:
	msrs_free(msrs);
	msrs = NULL;
err_ret:
	return err;
}

static void __exit amd64_edac_exit(void)
{
	if (amd64_ctl_pci)
		edac_pci_release_generic_ctl(amd64_ctl_pci);

	pci_unregister_driver(&amd64_pci_driver);

	msrs_free(msrs);
	msrs = NULL;
}

module_init(amd64_edac_init);
module_exit(amd64_edac_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SoftwareBitMaker: Doug Thompson, "
		"Dave Peterson, Thayne Harbaugh");
MODULE_DESCRIPTION("MC support for AMD64 memory controllers - "
		EDAC_AMD64_VERSION);

module_param(edac_op_state, int, 0444);
MODULE_PARM_DESC(edac_op_state, "EDAC Error Reporting state: 0=Poll,1=NMI");
