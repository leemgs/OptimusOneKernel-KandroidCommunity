


#include <linux/pci.h>
#include <linux/delay.h>

#include "ipath_kernel.h"
#include "ipath_registers.h"
#include "ipath_7220.h"


#define KR_IBSerDesMappTable (0x94000 / (sizeof(uint64_t)))


#define PCIE_SERDES0 0
#define PCIE_SERDES1 1


#define EPB_ADDR_SHF 8
#define EPB_LOC(chn, elt, reg) \
	(((elt & 0xf) | ((chn & 7) << 4) | ((reg & 0x3f) << 9)) << \
	 EPB_ADDR_SHF)
#define EPB_IB_QUAD0_CS_SHF (25)
#define EPB_IB_QUAD0_CS (1U <<  EPB_IB_QUAD0_CS_SHF)
#define EPB_IB_UC_CS_SHF (26)
#define EPB_PCIE_UC_CS_SHF (27)
#define EPB_GLOBAL_WR (1U << (EPB_ADDR_SHF + 8))


static int ipath_sd7220_reg_mod(struct ipath_devdata *dd, int sdnum, u32 loc,
				u32 data, u32 mask);
static int ibsd_mod_allchnls(struct ipath_devdata *dd, int loc, int val,
			     int mask);
static int ipath_sd_trimdone_poll(struct ipath_devdata *dd);
static void ipath_sd_trimdone_monitor(struct ipath_devdata *dd,
				      const char *where);
static int ipath_sd_setvals(struct ipath_devdata *dd);
static int ipath_sd_early(struct ipath_devdata *dd);
static int ipath_sd_dactrim(struct ipath_devdata *dd);

int ipath_sd7220_presets(struct ipath_devdata *dd);
static int ipath_internal_presets(struct ipath_devdata *dd);

static int ipath_sd_trimself(struct ipath_devdata *dd, int val);
static int epb_access(struct ipath_devdata *dd, int sdnum, int claim);

void ipath_set_relock_poll(struct ipath_devdata *dd, int ibup);


static int ipath_ibsd_ucode_loaded(struct ipath_devdata *dd)
{
	if (!dd->serdes_first_init_done && (ipath_sd7220_ib_vfy(dd) > 0))
		dd->serdes_first_init_done = 1;
	return dd->serdes_first_init_done;
}


#define INFINIPATH_HWE_IB_UC_MEMORYPARITYERR      0x0000004000000000ULL
#define IB_MPREG5 (EPB_LOC(6, 0, 0xE) | (1L << EPB_IB_UC_CS_SHF))
#define IB_MPREG6 (EPB_LOC(6, 0, 0xF) | (1U << EPB_IB_UC_CS_SHF))
#define UC_PAR_CLR_D 8
#define UC_PAR_CLR_M 0xC
#define IB_CTRL2(chn) (EPB_LOC(chn, 7, 3) | EPB_IB_QUAD0_CS)
#define START_EQ1(chan) EPB_LOC(chan, 7, 0x27)

void ipath_sd7220_clr_ibpar(struct ipath_devdata *dd)
{
	int ret;

	
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6,
		UC_PAR_CLR_D, UC_PAR_CLR_M);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed clearing IBSerDes Parity err\n");
		goto bail;
	}
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0,
		UC_PAR_CLR_M);

	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	udelay(4);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrclear,
		INFINIPATH_HWE_IB_UC_MEMORYPARITYERR);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
bail:
	return;
}


#define IBSD_RESYNC_TRIES 3
#define IB_PGUDP(chn) (EPB_LOC((chn), 2, 1) | EPB_IB_QUAD0_CS)
#define IB_CMUDONE(chn) (EPB_LOC((chn), 7, 0xF) | EPB_IB_QUAD0_CS)

static int ipath_resync_ibepb(struct ipath_devdata *dd)
{
	int ret, pat, tries, chn;
	u32 loc;

	ret = -1;
	chn = 0;
	for (tries = 0; tries < (4 * IBSD_RESYNC_TRIES); ++tries) {
		loc = IB_PGUDP(chn);
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, 0, 0);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed read in resync\n");
			continue;
		}
		if (ret != 0xF0 && ret != 0x55 && tries == 0)
			ipath_dev_err(dd, "unexpected pattern in resync\n");
		pat = ret ^ 0xA5; 
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, pat, 0xFF);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed write in resync\n");
			continue;
		}
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, 0, 0);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed re-read in resync\n");
			continue;
		}
		if (ret != pat) {
			ipath_dev_err(dd, "Failed compare1 in resync\n");
			continue;
		}
		loc = IB_CMUDONE(chn);
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, 0, 0);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed CMUDONE rd in resync\n");
			continue;
		}
		if ((ret & 0x70) != ((chn << 4) | 0x40)) {
			ipath_dev_err(dd, "Bad CMUDONE value %02X, chn %d\n",
				ret, chn);
			continue;
		}
		if (++chn == 4)
			break;  
	}
	ipath_cdbg(VERBOSE, "Resync in %d tries\n", tries);
	return (ret > 0) ? 0 : ret;
}


static int ipath_ibsd_reset(struct ipath_devdata *dd, int assert_rst)
{
	u64 rst_val;
	int ret = 0;
	unsigned long flags;

	rst_val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibserdesctrl);
	if (assert_rst) {
		
		spin_lock_irqsave(&dd->ipath_sdepb_lock, flags);
		epb_access(dd, IB_7220_SERDES, 1);
		rst_val |= 1ULL;
		
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
			dd->ipath_hwerrmask &
			~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_ibserdesctrl, rst_val);
		
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		udelay(2);
		
		epb_access(dd, IB_7220_SERDES, -1);
		spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
	} else {
		
		u64 val;
		rst_val &= ~(1ULL);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
			dd->ipath_hwerrmask &
			~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR);

		ret = ipath_resync_ibepb(dd);
		if (ret < 0)
			ipath_dev_err(dd, "unable to re-sync IB EPB\n");

		
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG5, 1, 1);
		if (ret < 0)
			goto bail;
		
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0x80,
			0x80);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed to set WDOG disable\n");
			goto bail;
		}
		ipath_write_kreg(dd, dd->ipath_kregs->kr_ibserdesctrl, rst_val);
		
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		udelay(1);
		
		ipath_sd7220_clr_ibpar(dd);
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_hwerrstatus);
		if (val & INFINIPATH_HWE_IB_UC_MEMORYPARITYERR) {
			ipath_dev_err(dd, "IBUC Parity still set after RST\n");
			dd->ipath_hwerrmask &=
				~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR;
		}
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
			dd->ipath_hwerrmask);
	}

bail:
	return ret;
}

static void ipath_sd_trimdone_monitor(struct ipath_devdata *dd,
       const char *where)
{
	int ret, chn, baduns;
	u64 val;

	if (!where)
		where = "?";

	
	udelay(2);

	ret = ipath_resync_ibepb(dd);
	if (ret < 0)
		ipath_dev_err(dd, "not able to re-sync IB EPB (%s)\n", where);

	
	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_CTRL2(0), 0, 0);
	if (ret < 0)
		ipath_dev_err(dd, "Failed TRIMDONE 1st read, (%s)\n", where);

	
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
	if (val & (1ULL << 11))
		ipath_cdbg(VERBOSE, "IBCS TRIMDONE set (%s)\n", where);
	else
		ipath_dev_err(dd, "IBCS TRIMDONE clear (%s)\n", where);

	udelay(2);

	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, IB_MPREG6, 0x80, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "Failed Dummy RMW, (%s)\n", where);
	udelay(10);

	baduns = 0;

	for (chn = 3; chn >= 0; --chn) {
		
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
			IB_CTRL2(chn), 0, 0);
		if (ret < 0)
			ipath_dev_err(dd, "Failed checking TRIMDONE, chn %d"
				" (%s)\n", chn, where);

		if (!(ret & 0x10)) {
			int probe;
			baduns |= (1 << chn);
			ipath_dev_err(dd, "TRIMDONE cleared on chn %d (%02X)."
				" (%s)\n", chn, ret, where);
			probe = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_PGUDP(0), 0, 0);
			ipath_dev_err(dd, "probe is %d (%02X)\n",
				probe, probe);
			probe = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0, 0);
			ipath_dev_err(dd, "re-read: %d (%02X)\n",
				probe, probe);
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0x10, 0x10);
			if (ret < 0)
				ipath_dev_err(dd,
					"Err on TRIMDONE rewrite1\n");
		}
	}
	for (chn = 3; chn >= 0; --chn) {
		
		if (baduns & (1 << chn)) {
			ipath_dev_err(dd,
				"Reseting TRIMDONE on chn %d (%s)\n",
				chn, where);
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				IB_CTRL2(chn), 0x10, 0x10);
			if (ret < 0)
				ipath_dev_err(dd, "Failed re-setting "
					"TRIMDONE, chn %d (%s)\n",
					chn, where);
		}
	}
}


int ipath_sd7220_init(struct ipath_devdata *dd, int was_reset)
{
	int ret = 1; 
	int first_reset;
	int val_stat;

	if (!was_reset) {
		
		ipath_ibsd_reset(dd, 1);
		ipath_sd_trimdone_monitor(dd, "Driver-reload");
	}

	
	ret = ipath_ibsd_ucode_loaded(dd);
	if (ret < 0) {
		ret = 1;
		goto done;
	}
	first_reset = !ret; 

	
	ret = ipath_sd_early(dd);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES early defaults\n");
		ret = 1;
		goto done;
	}

	
	if (first_reset) {
		ret = ipath_sd_dactrim(dd);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed IB SERDES DAC trim\n");
			ret = 1;
			goto done;
		}
	}

	
	ret = ipath_internal_presets(dd);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES presets\n");
		ret = 1;
		goto done;
	}
	ret = ipath_sd_trimself(dd, 0x80);
	if (ret < 0) {
		ipath_dev_err(dd, "Failed to set IB SERDES TRIMSELF\n");
		ret = 1;
		goto done;
	}

	
	ret = 0;	
	if (first_reset) {
		int vfy;
		int trim_done;
		ipath_dbg("SerDes uC was reset, reloading PRAM\n");
		ret = ipath_sd7220_ib_load(dd);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed to load IB SERDES image\n");
			ret = 1;
			goto done;
		}

		
		vfy = ipath_sd7220_ib_vfy(dd);
		if (vfy != ret) {
			ipath_dev_err(dd, "SERDES PRAM VFY failed\n");
			ret = 1;
			goto done;
		}
		
		ret = 0;

		
		ret = ibsd_mod_allchnls(dd, START_EQ1(0), 0, 0x38);
		if (ret < 0) {
			ipath_dev_err(dd, "Failed clearing START_EQ1\n");
			ret = 1;
			goto done;
		}

		ipath_ibsd_reset(dd, 0);
		
		trim_done = ipath_sd_trimdone_poll(dd);
		
		ipath_ibsd_reset(dd, 1);

		if (!trim_done) {
			ipath_dev_err(dd, "No TRIMDONE seen\n");
			ret = 1;
			goto done;
		}

		ipath_sd_trimdone_monitor(dd, "First-reset");
		
		dd->serdes_first_init_done = 1;
	}
	

	val_stat = ipath_sd_setvals(dd);
	if (val_stat < 0)
		ret = 1;
done:
	
	ipath_set_relock_poll(dd, -1);
	return ret;
}

#define EPB_ACC_REQ 1
#define EPB_ACC_GNT 0x100
#define EPB_DATA_MASK 0xFF
#define EPB_RD (1ULL << 24)
#define EPB_TRANS_RDY (1ULL << 31)
#define EPB_TRANS_ERR (1ULL << 30)
#define EPB_TRANS_TRIES 5


static int epb_access(struct ipath_devdata *dd, int sdnum, int claim)
{
	u16 acc;
	u64 accval;
	int owned = 0;
	u64 oct_sel = 0;

	switch (sdnum) {
	case IB_7220_SERDES :
		
		acc = dd->ipath_kregs->kr_ib_epbacc;
		break;
	case PCIE_SERDES0 :
	case PCIE_SERDES1 :
		
		acc = dd->ipath_kregs->kr_pcie_epbacc;
		oct_sel = (2 << (sdnum - PCIE_SERDES0));
		break;
	default :
		return 0;
	}

	
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	udelay(15);

	accval = ipath_read_kreg32(dd, acc);

	owned = !!(accval & EPB_ACC_GNT);
	if (claim < 0) {
		
		u64 pollval;
		
		u64 newval = 0;
		ipath_write_kreg(dd, acc, newval);
		
		pollval = ipath_read_kreg32(dd, acc);
		udelay(5);
		pollval = ipath_read_kreg32(dd, acc);
		if (pollval & EPB_ACC_GNT)
			owned = -1;
	} else if (claim > 0) {
		
		u64 pollval;
		u64 newval = EPB_ACC_REQ | oct_sel;
		ipath_write_kreg(dd, acc, newval);
		
		pollval = ipath_read_kreg32(dd, acc);
		udelay(5);
		pollval = ipath_read_kreg32(dd, acc);
		if (!(pollval & EPB_ACC_GNT))
			owned = -1;
	}
	return owned;
}


static int epb_trans(struct ipath_devdata *dd, u16 reg, u64 i_val, u64 *o_vp)
{
	int tries;
	u64 transval;


	ipath_write_kreg(dd, reg, i_val);
	
	transval = ipath_read_kreg64(dd, reg);

	for (tries = EPB_TRANS_TRIES; tries; --tries) {
		transval = ipath_read_kreg32(dd, reg);
		if (transval & EPB_TRANS_RDY)
			break;
		udelay(5);
	}
	if (transval & EPB_TRANS_ERR)
		return -1;
	if (tries > 0 && o_vp)
		*o_vp = transval;
	return tries;
}


static int ipath_sd7220_reg_mod(struct ipath_devdata *dd, int sdnum, u32 loc,
				u32 wd, u32 mask)
{
	u16 trans;
	u64 transval;
	int owned;
	int tries, ret;
	unsigned long flags;

	switch (sdnum) {
	case IB_7220_SERDES :
		trans = dd->ipath_kregs->kr_ib_epbtrans;
		break;
	case PCIE_SERDES0 :
	case PCIE_SERDES1 :
		trans = dd->ipath_kregs->kr_pcie_epbtrans;
		break;
	default :
		return -1;
	}

	
	spin_lock_irqsave(&dd->ipath_sdepb_lock, flags);

	owned = epb_access(dd, sdnum, 1);
	if (owned < 0) {
		spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
		return -1;
	}
	ret = 0;
	for (tries = EPB_TRANS_TRIES; tries; --tries) {
		transval = ipath_read_kreg32(dd, trans);
		if (transval & EPB_TRANS_RDY)
			break;
		udelay(5);
	}

	if (tries > 0) {
		tries = 1;	
		if (mask != 0xFF) {
			
			transval = loc | EPB_RD;
			tries = epb_trans(dd, trans, transval, &transval);
		}
		if (tries > 0 && mask != 0) {
			
			wd = (wd & mask) | (transval & ~mask);
			transval = loc | (wd & EPB_DATA_MASK);
			tries = epb_trans(dd, trans, transval, &transval);
		}
	}
	

	
	if (epb_access(dd, sdnum, -1) < 0)
		ret = -1;
	else
		ret = transval & EPB_DATA_MASK;

	spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
	if (tries <= 0)
		ret = -1;
	return ret;
}

#define EPB_ROM_R (2)
#define EPB_ROM_W (1)

#define EPB_UC_CTL EPB_LOC(6, 0, 0)
#define EPB_MADDRL EPB_LOC(6, 0, 2)
#define EPB_MADDRH EPB_LOC(6, 0, 3)
#define EPB_ROMDATA EPB_LOC(6, 0, 4)
#define EPB_RAMDATA EPB_LOC(6, 0, 5)


static int ipath_sd7220_ram_xfer(struct ipath_devdata *dd, int sdnum, u32 loc,
			       u8 *buf, int cnt, int rd_notwr)
{
	u16 trans;
	u64 transval;
	u64 csbit;
	int owned;
	int tries;
	int sofar;
	int addr;
	int ret;
	unsigned long flags;
	const char *op;

	
	switch (sdnum) {
	case IB_7220_SERDES :
		csbit = 1ULL << EPB_IB_UC_CS_SHF;
		trans = dd->ipath_kregs->kr_ib_epbtrans;
		break;
	case PCIE_SERDES0 :
	case PCIE_SERDES1 :
		
		csbit = 1ULL << EPB_PCIE_UC_CS_SHF;
		trans = dd->ipath_kregs->kr_pcie_epbtrans;
		break;
	default :
		return -1;
	}

	op = rd_notwr ? "Rd" : "Wr";
	spin_lock_irqsave(&dd->ipath_sdepb_lock, flags);

	owned = epb_access(dd, sdnum, 1);
	if (owned < 0) {
		spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
		ipath_dbg("Could not get %s access to %s EPB: %X, loc %X\n",
			op, (sdnum == IB_7220_SERDES) ? "IB" : "PCIe",
			owned, loc);
		return -1;
	}

	
	addr = loc & 0x1FFF;
	for (tries = EPB_TRANS_TRIES; tries; --tries) {
		transval = ipath_read_kreg32(dd, trans);
		if (transval & EPB_TRANS_RDY)
			break;
		udelay(5);
	}

	sofar = 0;
	if (tries <= 0)
		ipath_dbg("No initial RDY on EPB access request\n");
	else {
		

		
		transval = csbit | EPB_UC_CTL |
			(rd_notwr ? EPB_ROM_R : EPB_ROM_W);
		tries = epb_trans(dd, trans, transval, &transval);
		if (tries <= 0)
			ipath_dbg("No EPB response to uC %s cmd\n", op);
		while (tries > 0 && sofar < cnt) {
			if (!sofar) {
				
				int addrbyte = (addr + sofar) >> 8;
				transval = csbit | EPB_MADDRH | addrbyte;
				tries = epb_trans(dd, trans, transval,
						  &transval);
				if (tries <= 0) {
					ipath_dbg("No EPB response ADDRH\n");
					break;
				}
				addrbyte = (addr + sofar) & 0xFF;
				transval = csbit | EPB_MADDRL | addrbyte;
				tries = epb_trans(dd, trans, transval,
						 &transval);
				if (tries <= 0) {
					ipath_dbg("No EPB response ADDRL\n");
					break;
				}
			}

			if (rd_notwr)
				transval = csbit | EPB_ROMDATA | EPB_RD;
			else
				transval = csbit | EPB_ROMDATA | buf[sofar];
			tries = epb_trans(dd, trans, transval, &transval);
			if (tries <= 0) {
				ipath_dbg("No EPB response DATA\n");
				break;
			}
			if (rd_notwr)
				buf[sofar] = transval & EPB_DATA_MASK;
			++sofar;
		}
		
		transval = csbit | EPB_UC_CTL;
		tries = epb_trans(dd, trans, transval, &transval);
		if (tries <= 0)
			ipath_dbg("No EPB response to drop of uC %s cmd\n", op);
	}

	ret = sofar;
	
	if (epb_access(dd, sdnum, -1) < 0)
		ret = -1;

	spin_unlock_irqrestore(&dd->ipath_sdepb_lock, flags);
	if (tries <= 0) {
		ipath_dbg("SERDES PRAM %s failed after %d bytes\n", op, sofar);
		ret = -1;
	}
	return ret;
}

#define PROG_CHUNK 64

int ipath_sd7220_prog_ld(struct ipath_devdata *dd, int sdnum,
	u8 *img, int len, int offset)
{
	int cnt, sofar, req;

	sofar = 0;
	while (sofar < len) {
		req = len - sofar;
		if (req > PROG_CHUNK)
			req = PROG_CHUNK;
		cnt = ipath_sd7220_ram_xfer(dd, sdnum, offset + sofar,
					  img + sofar, req, 0);
		if (cnt < req) {
			sofar = -1;
			break;
		}
		sofar += req;
	}
	return sofar;
}

#define VFY_CHUNK 64
#define SD_PRAM_ERROR_LIMIT 42

int ipath_sd7220_prog_vfy(struct ipath_devdata *dd, int sdnum,
	const u8 *img, int len, int offset)
{
	int cnt, sofar, req, idx, errors;
	unsigned char readback[VFY_CHUNK];

	errors = 0;
	sofar = 0;
	while (sofar < len) {
		req = len - sofar;
		if (req > VFY_CHUNK)
			req = VFY_CHUNK;
		cnt = ipath_sd7220_ram_xfer(dd, sdnum, sofar + offset,
					  readback, req, 1);
		if (cnt < req) {
			
			sofar = -1;
			break;
		}
		for (idx = 0; idx < cnt; ++idx) {
			if (readback[idx] != img[idx+sofar])
				++errors;
		}
		sofar += cnt;
	}
	return errors ? -errors : sofar;
}


#define IB_SERDES_TRIM_DONE (1ULL << 11)
#define TRIM_TMO (30)

static int ipath_sd_trimdone_poll(struct ipath_devdata *dd)
{
	int trim_tmo, ret;
	uint64_t val;

	
	ret = 0;
	for (trim_tmo = 0; trim_tmo < TRIM_TMO; ++trim_tmo) {
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
		if (val & IB_SERDES_TRIM_DONE) {
			ipath_cdbg(VERBOSE, "TRIMDONE after %d\n", trim_tmo);
			ret = 1;
			break;
		}
		msleep(10);
	}
	if (trim_tmo >= TRIM_TMO) {
		ipath_dev_err(dd, "No TRIMDONE in %d tries\n", trim_tmo);
		ret = 0;
	}
	return ret;
}

#define TX_FAST_ELT (9)



#define NUM_DDS_REGS 6
#define DDS_REG_MAP 0x76A910 

#define DDS_VAL(amp_d, main_d, ipst_d, ipre_d, amp_s, main_s, ipst_s, ipre_s) \
	{ { ((amp_d & 0x1F) << 1) | 1, ((amp_s & 0x1F) << 1) | 1, \
	  (main_d << 3) | 4 | (ipre_d >> 2), \
	  (main_s << 3) | 4 | (ipre_s >> 2), \
	  ((ipst_d & 0xF) << 1) | ((ipre_d & 3) << 6) | 0x21, \
	  ((ipst_s & 0xF) << 1) | ((ipre_s & 3) << 6) | 0x21 } }

static struct dds_init {
	uint8_t reg_vals[NUM_DDS_REGS];
} dds_init_vals[] = {
	
	
#define DDS_3M 0
	DDS_VAL(31, 19, 12, 0, 29, 22,  9, 0),
	DDS_VAL(31, 12, 15, 4, 31, 15, 15, 1),
	DDS_VAL(31, 13, 15, 3, 31, 16, 15, 0),
	DDS_VAL(31, 14, 15, 2, 31, 17, 14, 0),
	DDS_VAL(31, 15, 15, 1, 31, 18, 13, 0),
	DDS_VAL(31, 16, 15, 0, 31, 19, 12, 0),
	DDS_VAL(31, 17, 14, 0, 31, 20, 11, 0),
	DDS_VAL(31, 18, 13, 0, 30, 21, 10, 0),
	DDS_VAL(31, 20, 11, 0, 28, 23,  8, 0),
	DDS_VAL(31, 21, 10, 0, 27, 24,  7, 0),
	DDS_VAL(31, 22,  9, 0, 26, 25,  6, 0),
	DDS_VAL(30, 23,  8, 0, 25, 26,  5, 0),
	DDS_VAL(29, 24,  7, 0, 23, 27,  4, 0),
	
#define DDS_1M 13
	DDS_VAL(28, 25,  6, 0, 21, 28,  3, 0),
	DDS_VAL(27, 26,  5, 0, 19, 29,  2, 0),
	DDS_VAL(25, 27,  4, 0, 17, 30,  1, 0)
};



#define RXEQ_INIT_RDESC(elt, addr) (((elt) & 0xF) | ((addr) << 4))
#define RXEQ_VAL(elt, adr, val0, val1, val2, val3) \
	{RXEQ_INIT_RDESC((elt), (adr)), {(val0), (val1), (val2), (val3)} }

#define RXEQ_VAL_ALL(elt, adr, val)  \
	{RXEQ_INIT_RDESC((elt), (adr)), {(val), (val), (val), (val)} }

#define RXEQ_SDR_DFELTH 0
#define RXEQ_SDR_TLTH 0
#define RXEQ_SDR_G1CNT_Z1CNT 0x11
#define RXEQ_SDR_ZCNT 23

static struct rxeq_init {
	u16 rdesc;	
	u8  rdata[4];
} rxeq_init_vals[] = {
	
	RXEQ_VAL_ALL(7, 0x27, 0x10),
	
	RXEQ_VAL(7, 8,    0, 0, 0, 0), 
	RXEQ_VAL(7, 0x21, 0, 0, 0, 0), 
	
	RXEQ_VAL(7, 9,    2, 2, 2, 2), 
	RXEQ_VAL(7, 0x23, 2, 2, 2, 2), 
	
	RXEQ_VAL(7, 0x1B, 12, 12, 12, 12), 
	RXEQ_VAL(7, 0x1C, 12, 12, 12, 12), 
	
	RXEQ_VAL(7, 0x1E, 0x10, 0x10, 0x10, 0x10), 
	RXEQ_VAL(7, 0x1F, 0x10, 0x10, 0x10, 0x10), 
	
	RXEQ_VAL_ALL(6, 6, 0x20), 
	RXEQ_VAL_ALL(6, 6, 0), 
};


#define DDS_ROWS (16)
#define RXEQ_ROWS ARRAY_SIZE(rxeq_init_vals)

static int ipath_sd_setvals(struct ipath_devdata *dd)
{
	int idx, midx;
	int min_idx;	 
	uint32_t dds_reg_map;
	u64 __iomem *taddr, *iaddr;
	uint64_t data;
	uint64_t sdctl;

	taddr = dd->ipath_kregbase + KR_IBSerDesMappTable;
	iaddr = dd->ipath_kregbase + dd->ipath_kregs->kr_ib_ddsrxeq;

	
	sdctl = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibserdesctrl);
	sdctl = (sdctl & ~(0x1f << 8)) | (NUM_DDS_REGS << 8);
	sdctl = (sdctl & ~(0x1f << 13)) | (RXEQ_ROWS << 13);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_ibserdesctrl, sdctl);

	
	dds_reg_map = DDS_REG_MAP;
	for (idx = 0; idx < NUM_DDS_REGS; ++idx) {
		data = ((dds_reg_map & 0xF) << 4) | TX_FAST_ELT;
		writeq(data, iaddr + idx);
		mmiowb();
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		dds_reg_map >>= 4;
		for (midx = 0; midx < DDS_ROWS; ++midx) {
			u64 __iomem *daddr = taddr + ((midx << 4) + idx);
			data = dds_init_vals[midx].reg_vals[idx];
			writeq(data, daddr);
			mmiowb();
			ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		} 
	} 

	
	min_idx = idx; 
	taddr += 0x100; 
	
	for (idx = 0; idx < RXEQ_ROWS; ++idx) {
		int didx; 
		int vidx;

		
		didx = idx + min_idx;
		
		writeq(rxeq_init_vals[idx].rdesc, iaddr + didx);
		mmiowb();
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		
		for (vidx = 0; vidx < 4; vidx++) {
			data = rxeq_init_vals[idx].rdata[vidx];
			writeq(data, taddr + (vidx << 6) + idx);
			mmiowb();
			ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		}
	} 
	return 0;
}

#define CMUCTRL5 EPB_LOC(7, 0, 0x15)
#define RXHSCTRL0(chan) EPB_LOC(chan, 6, 0)
#define VCDL_DAC2(chan) EPB_LOC(chan, 6, 5)
#define VCDL_CTRL0(chan) EPB_LOC(chan, 6, 6)
#define VCDL_CTRL2(chan) EPB_LOC(chan, 6, 8)
#define START_EQ2(chan) EPB_LOC(chan, 7, 0x28)

static int ibsd_sto_noisy(struct ipath_devdata *dd, int loc, int val, int mask)
{
	int ret = -1;
	int sloc; 

	loc |= (1U << EPB_IB_QUAD0_CS_SHF);
	sloc = loc >> EPB_ADDR_SHF;

	ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, val, mask);
	if (ret < 0)
		ipath_dev_err(dd, "Write failed: elt %d,"
			" addr 0x%X, chnl %d, val 0x%02X, mask 0x%02X\n",
			(sloc & 0xF), (sloc >> 9) & 0x3f, (sloc >> 4) & 7,
			val & 0xFF, mask & 0xFF);
	return ret;
}


static int ibsd_mod_allchnls(struct ipath_devdata *dd, int loc, int val,
	int mask)
{
	int ret = -1;
	int chnl;

	if (loc & EPB_GLOBAL_WR) {
		
		loc |= (1U << EPB_IB_QUAD0_CS_SHF);
		chnl = (loc >> (4 + EPB_ADDR_SHF)) & 7;
		if (mask != 0xFF) {
			ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES,
				loc & ~EPB_GLOBAL_WR, 0, 0);
			if (ret < 0) {
				int sloc = loc >> EPB_ADDR_SHF;
				ipath_dev_err(dd, "pre-read failed: elt %d,"
					" addr 0x%X, chnl %d\n", (sloc & 0xF),
					(sloc >> 9) & 0x3f, chnl);
				return ret;
			}
			val = (ret & ~mask) | (val & mask);
		}
		loc &=  ~(7 << (4+EPB_ADDR_SHF));
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, loc, val, 0xFF);
		if (ret < 0) {
			int sloc = loc >> EPB_ADDR_SHF;
			ipath_dev_err(dd, "Global WR failed: elt %d,"
				" addr 0x%X, val %02X\n",
				(sloc & 0xF), (sloc >> 9) & 0x3f, val);
		}
		return ret;
	}
	
	loc &=  ~(7 << (4+EPB_ADDR_SHF));
	loc |= (1U << EPB_IB_QUAD0_CS_SHF);
	for (chnl = 0; chnl < 4; ++chnl) {
		int cloc;
		cloc = loc | (chnl << (4+EPB_ADDR_SHF));
		ret = ipath_sd7220_reg_mod(dd, IB_7220_SERDES, cloc, val, mask);
		if (ret < 0) {
			int sloc = loc >> EPB_ADDR_SHF;
			ipath_dev_err(dd, "Write failed: elt %d,"
				" addr 0x%X, chnl %d, val 0x%02X,"
				" mask 0x%02X\n",
				(sloc & 0xF), (sloc >> 9) & 0x3f, chnl,
				val & 0xFF, mask & 0xFF);
			break;
		}
	}
	return ret;
}


static int set_dds_vals(struct ipath_devdata *dd, struct dds_init *ddi)
{
	int ret;
	int idx, reg, data;
	uint32_t regmap;

	regmap = DDS_REG_MAP;
	for (idx = 0; idx < NUM_DDS_REGS; ++idx) {
		reg = (regmap & 0xF);
		regmap >>= 4;
		data = ddi->reg_vals[idx];
		
		ret = ibsd_mod_allchnls(dd, EPB_LOC(0, 9, reg), data, 0xFF);
		if (ret < 0)
			break;
	}
	return ret;
}


static int set_rxeq_vals(struct ipath_devdata *dd, int vsel)
{
	int ret;
	int ridx;
	int cnt = ARRAY_SIZE(rxeq_init_vals);

	for (ridx = 0; ridx < cnt; ++ridx) {
		int elt, reg, val, loc;
		elt = rxeq_init_vals[ridx].rdesc & 0xF;
		reg = rxeq_init_vals[ridx].rdesc >> 4;
		loc = EPB_LOC(0, elt, reg);
		val = rxeq_init_vals[ridx].rdata[vsel];
		
		ret = ibsd_mod_allchnls(dd, loc, val, 0xFF);
		if (ret < 0)
			break;
	}
	return ret;
}


static unsigned ipath_rxeq_set = 2;
module_param_named(rxeq_default_set, ipath_rxeq_set, uint,
	S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(rxeq_default_set,
	"Which set [0..3] of Rx Equalization values is default");

static int ipath_internal_presets(struct ipath_devdata *dd)
{
	int ret = 0;

	ret = set_dds_vals(dd, dds_init_vals + DDS_3M);

	if (ret < 0)
		ipath_dev_err(dd, "Failed to set default DDS values\n");
	ret = set_rxeq_vals(dd, ipath_rxeq_set & 3);
	if (ret < 0)
		ipath_dev_err(dd, "Failed to set default RXEQ values\n");
	return ret;
}

int ipath_sd7220_presets(struct ipath_devdata *dd)
{
	int ret = 0;

	if (!dd->ipath_presets_needed)
		return ret;
	dd->ipath_presets_needed = 0;
	
	ipath_ibsd_reset(dd, 1);
	udelay(2);
	ipath_sd_trimdone_monitor(dd, "link-down");

	ret = ipath_internal_presets(dd);
return ret;
}

static int ipath_sd_trimself(struct ipath_devdata *dd, int val)
{
	return ibsd_sto_noisy(dd, CMUCTRL5, val, 0xFF);
}

static int ipath_sd_early(struct ipath_devdata *dd)
{
	int ret = -1; 
	int chnl;

	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, RXHSCTRL0(chnl), 0xD4, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, VCDL_DAC2(chnl), 0x2D, 0xFF);
		if (ret < 0)
			goto bail;
	}
	
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, VCDL_CTRL2(chnl), 3, 0xF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, START_EQ1(chnl), 0x10, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, START_EQ2(chnl), 0x30, 0xFF);
		if (ret < 0)
			goto bail;
	}
bail:
	return ret;
}

#define BACTRL(chnl) EPB_LOC(chnl, 6, 0x0E)
#define LDOUTCTRL1(chnl) EPB_LOC(chnl, 7, 6)
#define RXHSSTATUS(chnl) EPB_LOC(chnl, 6, 0xF)

static int ipath_sd_dactrim(struct ipath_devdata *dd)
{
	int ret = -1; 
	int chnl;

	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, BACTRL(chnl), 0x40, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, LDOUTCTRL1(chnl), 0x04, 0xFF);
		if (ret < 0)
			goto bail;
	}
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, RXHSSTATUS(chnl), 0x04, 0xFF);
		if (ret < 0)
			goto bail;
	}
	
	udelay(415);
	for (chnl = 0; chnl < 4; ++chnl) {
		ret = ibsd_sto_noisy(dd, LDOUTCTRL1(chnl), 0x00, 0xFF);
		if (ret < 0)
			goto bail;
	}
bail:
	return ret;
}

#define RELOCK_FIRST_MS 3
#define RXLSPPM(chan) EPB_LOC(chan, 0, 2)
void ipath_toggle_rclkrls(struct ipath_devdata *dd)
{
	int loc = RXLSPPM(0) | EPB_GLOBAL_WR;
	int ret;

	ret = ibsd_mod_allchnls(dd, loc, 0, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "RCLKRLS failed to clear D7\n");
	else {
		udelay(1);
		ibsd_mod_allchnls(dd, loc, 0x80, 0x80);
	}
	
	udelay(1);
	ret = ibsd_mod_allchnls(dd, loc, 0, 0x80);
	if (ret < 0)
		ipath_dev_err(dd, "RCLKRLS failed to clear D7\n");
	else {
		udelay(1);
		ibsd_mod_allchnls(dd, loc, 0x80, 0x80);
	}
	
	dd->ipath_f_xgxs_reset(dd);
}


void ipath_shutdown_relock_poll(struct ipath_devdata *dd)
{
	struct ipath_relock *irp = &dd->ipath_relock_singleton;
	if (atomic_read(&irp->ipath_relock_timer_active)) {
		del_timer_sync(&irp->ipath_relock_timer);
		atomic_set(&irp->ipath_relock_timer_active, 0);
	}
}

static unsigned ipath_relock_by_timer = 1;
module_param_named(relock_by_timer, ipath_relock_by_timer, uint,
	S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(relock_by_timer, "Allow relock attempt if link not up");

static void ipath_run_relock(unsigned long opaque)
{
	struct ipath_devdata *dd = (struct ipath_devdata *)opaque;
	struct ipath_relock *irp = &dd->ipath_relock_singleton;
	u64 val, ltstate;

	if (!(dd->ipath_flags & IPATH_INITTED)) {
		
		irp->ipath_relock_interval = HZ;
		mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
		return;
	}

	
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcstatus);
	ltstate = ipath_ib_linktrstate(dd, val);

	if (ltstate <= INFINIPATH_IBCS_LT_STATE_CFGWAITRMT
		&& ltstate != INFINIPATH_IBCS_LT_STATE_LINKUP) {
		int timeoff;
		
		if (ipath_relock_by_timer) {
			if (dd->ipath_flags & IPATH_IB_AUTONEG_INPROG)
				ipath_cdbg(VERBOSE, "Skip RELOCK in AUTONEG\n");
			else if (!(dd->ipath_flags & IPATH_IB_LINK_DISABLED)) {
				ipath_cdbg(VERBOSE, "RELOCK\n");
				ipath_toggle_rclkrls(dd);
			}
		}
		
		timeoff = irp->ipath_relock_interval << 1;
		if (timeoff > HZ)
			timeoff = HZ;
		irp->ipath_relock_interval = timeoff;

		mod_timer(&irp->ipath_relock_timer, jiffies + timeoff);
	} else {
		
		mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
	}
}

void ipath_set_relock_poll(struct ipath_devdata *dd, int ibup)
{
	struct ipath_relock *irp = &dd->ipath_relock_singleton;

	if (ibup > 0) {
		
		if (atomic_read(&irp->ipath_relock_timer_active))
			mod_timer(&irp->ipath_relock_timer, jiffies + HZ);
	} else {
		
		int timeout;
		timeout = (HZ * ((ibup == -1) ? 1000 : RELOCK_FIRST_MS))/1000;
		if (timeout == 0)
			timeout = 1;
		
		if (atomic_inc_return(&irp->ipath_relock_timer_active) == 1) {
			init_timer(&irp->ipath_relock_timer);
			irp->ipath_relock_timer.function = ipath_run_relock;
			irp->ipath_relock_timer.data = (unsigned long) dd;
			irp->ipath_relock_interval = timeout;
			irp->ipath_relock_timer.expires = jiffies + timeout;
			add_timer(&irp->ipath_relock_timer);
		} else {
			irp->ipath_relock_interval = timeout;
			mod_timer(&irp->ipath_relock_timer, jiffies + timeout);
			atomic_dec(&irp->ipath_relock_timer_active);
		}
	}
}

