

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

#include <asm/cputype.h>

#include <mach/common.h>
#include <mach/control.h>
#include <mach/cpu.h>

static struct omap_chip_id omap_chip;
static unsigned int omap_revision;


unsigned int omap_rev(void)
{
	return omap_revision;
}
EXPORT_SYMBOL(omap_rev);


int omap_chip_is(struct omap_chip_id oci)
{
	return (oci.oc & omap_chip.oc) ? 1 : 0;
}
EXPORT_SYMBOL(omap_chip_is);

int omap_type(void)
{
	u32 val = 0;

	if (cpu_is_omap24xx())
		val = omap_ctrl_readl(OMAP24XX_CONTROL_STATUS);
	else if (cpu_is_omap34xx())
		val = omap_ctrl_readl(OMAP343X_CONTROL_STATUS);
	else {
		pr_err("Cannot detect omap type!\n");
		goto out;
	}

	val &= OMAP2_DEVICETYPE_MASK;
	val >>= 8;

out:
	return val;
}
EXPORT_SYMBOL(omap_type);




#define OMAP_TAP_IDCODE		0x0204
#define OMAP_TAP_DIE_ID_0	0x0218
#define OMAP_TAP_DIE_ID_1	0x021C
#define OMAP_TAP_DIE_ID_2	0x0220
#define OMAP_TAP_DIE_ID_3	0x0224

#define read_tap_reg(reg)	__raw_readl(tap_base  + (reg))

struct omap_id {
	u16	hawkeye;	
	u8	dev;		
	u32	type;		
};


static struct omap_id omap_ids[] __initdata = {
	{ .hawkeye = 0xb5d9, .dev = 0x0, .type = 0x24200024 },
	{ .hawkeye = 0xb5d9, .dev = 0x1, .type = 0x24201024 },
	{ .hawkeye = 0xb5d9, .dev = 0x2, .type = 0x24202024 },
	{ .hawkeye = 0xb5d9, .dev = 0x4, .type = 0x24220024 },
	{ .hawkeye = 0xb5d9, .dev = 0x8, .type = 0x24230024 },
	{ .hawkeye = 0xb68a, .dev = 0x0, .type = 0x24300024 },
};

static void __iomem *tap_base;
static u16 tap_prod_id;

void __init omap24xx_check_revision(void)
{
	int i, j;
	u32 idcode, prod_id;
	u16 hawkeye;
	u8  dev_type, rev;

	idcode = read_tap_reg(OMAP_TAP_IDCODE);
	prod_id = read_tap_reg(tap_prod_id);
	hawkeye = (idcode >> 12) & 0xffff;
	rev = (idcode >> 28) & 0x0f;
	dev_type = (prod_id >> 16) & 0x0f;

	pr_debug("OMAP_TAP_IDCODE 0x%08x REV %i HAWKEYE 0x%04x MANF %03x\n",
		 idcode, rev, hawkeye, (idcode >> 1) & 0x7ff);
	pr_debug("OMAP_TAP_DIE_ID_0: 0x%08x\n",
		 read_tap_reg(OMAP_TAP_DIE_ID_0));
	pr_debug("OMAP_TAP_DIE_ID_1: 0x%08x DEV_REV: %i\n",
		 read_tap_reg(OMAP_TAP_DIE_ID_1),
		 (read_tap_reg(OMAP_TAP_DIE_ID_1) >> 28) & 0xf);
	pr_debug("OMAP_TAP_DIE_ID_2: 0x%08x\n",
		 read_tap_reg(OMAP_TAP_DIE_ID_2));
	pr_debug("OMAP_TAP_DIE_ID_3: 0x%08x\n",
		 read_tap_reg(OMAP_TAP_DIE_ID_3));
	pr_debug("OMAP_TAP_PROD_ID_0: 0x%08x DEV_TYPE: %i\n",
		 prod_id, dev_type);

	
	for (i = 0; i < ARRAY_SIZE(omap_ids); i++) {
		if (hawkeye == omap_ids[i].hawkeye)
			break;
	}

	if (i == ARRAY_SIZE(omap_ids)) {
		printk(KERN_ERR "Unknown OMAP CPU id\n");
		return;
	}

	for (j = i; j < ARRAY_SIZE(omap_ids); j++) {
		if (dev_type == omap_ids[j].dev)
			break;
	}

	if (j == ARRAY_SIZE(omap_ids)) {
		printk(KERN_ERR "Unknown OMAP device type. "
				"Handling it as OMAP%04x\n",
				omap_ids[i].type >> 16);
		j = i;
	}

	pr_info("OMAP%04x", omap_rev() >> 16);
	if ((omap_rev() >> 8) & 0x0f)
		pr_info("ES%x", (omap_rev() >> 12) & 0xf);
	pr_info("\n");
}

void __init omap34xx_check_revision(void)
{
	u32 cpuid, idcode;
	u16 hawkeye;
	u8 rev;
	char *rev_name = "ES1.0";

	
	cpuid = read_cpuid(CPUID_ID);
	if ((((cpuid >> 4) & 0xfff) == 0xc08) && ((cpuid & 0xf) == 0x0)) {
		omap_revision = OMAP3430_REV_ES1_0;
		goto out;
	}

	
	idcode = read_tap_reg(OMAP_TAP_IDCODE);
	hawkeye = (idcode >> 12) & 0xffff;
	rev = (idcode >> 28) & 0xff;

	if (hawkeye == 0xb7ae) {
		switch (rev) {
		case 0:
			omap_revision = OMAP3430_REV_ES2_0;
			rev_name = "ES2.0";
			break;
		case 2:
			omap_revision = OMAP3430_REV_ES2_1;
			rev_name = "ES2.1";
			break;
		case 3:
			omap_revision = OMAP3430_REV_ES3_0;
			rev_name = "ES3.0";
			break;
		case 4:
			omap_revision = OMAP3430_REV_ES3_1;
			rev_name = "ES3.1";
			break;
		default:
			
			omap_revision = OMAP3430_REV_ES3_1;
			rev_name = "Unknown revision\n";
		}
	}

out:
	pr_info("OMAP%04x %s\n", omap_rev() >> 16, rev_name);
}


void __init omap2_check_revision(void)
{
	
	if (cpu_is_omap24xx())
		omap24xx_check_revision();
	else if (cpu_is_omap34xx())
		omap34xx_check_revision();
	else if (cpu_is_omap44xx()) {
		printk(KERN_INFO "FIXME: CPU revision = OMAP4430\n");
		return;
	} else
		pr_err("OMAP revision unknown, please fix!\n");

	
	if (cpu_is_omap243x()) {
		
		omap_chip.oc |= CHIP_IS_OMAP2430;
	} else if (cpu_is_omap242x()) {
		
		omap_chip.oc |= CHIP_IS_OMAP2420;
	} else if (cpu_is_omap343x()) {
		omap_chip.oc = CHIP_IS_OMAP3430;
		if (omap_rev() == OMAP3430_REV_ES1_0)
			omap_chip.oc |= CHIP_IS_OMAP3430ES1;
		else if (omap_rev() >= OMAP3430_REV_ES2_0 &&
			 omap_rev() <= OMAP3430_REV_ES2_1)
			omap_chip.oc |= CHIP_IS_OMAP3430ES2;
		else if (omap_rev() == OMAP3430_REV_ES3_0)
			omap_chip.oc |= CHIP_IS_OMAP3430ES3_0;
		else if (omap_rev() == OMAP3430_REV_ES3_1)
			omap_chip.oc |= CHIP_IS_OMAP3430ES3_1;
	} else {
		pr_err("Uninitialized omap_chip, please fix!\n");
	}
}


void __init omap2_set_globals_tap(struct omap_globals *omap2_globals)
{
	omap_revision = omap2_globals->class;
	tap_base = omap2_globals->tap;

	if (cpu_is_omap34xx())
		tap_prod_id = 0x0210;
	else
		tap_prod_id = 0x0208;
}
