

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/i2c/twl4030.h>




struct twlreg_info {
	
	u8			base;

	
	u8			id;

	
	u8			table_len;
	const u16		*table;

	
	u16			min_mV;

	
	struct regulator_desc	desc;
};



#define VREG_GRP		0
#define VREG_TYPE		1
#define VREG_REMAP		2
#define VREG_DEDICATED		3	


static inline int
twl4030reg_read(struct twlreg_info *info, unsigned offset)
{
	u8 value;
	int status;

	status = twl4030_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER,
			&value, info->base + offset);
	return (status < 0) ? status : value;
}

static inline int
twl4030reg_write(struct twlreg_info *info, unsigned offset, u8 value)
{
	return twl4030_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
			value, info->base + offset);
}





static int twl4030reg_grp(struct regulator_dev *rdev)
{
	return twl4030reg_read(rdev_get_drvdata(rdev), VREG_GRP);
}



#define P3_GRP		BIT(7)		
#define P2_GRP		BIT(6)		
#define P1_GRP		BIT(5)		

static int twl4030reg_is_enabled(struct regulator_dev *rdev)
{
	int	state = twl4030reg_grp(rdev);

	if (state < 0)
		return state;

	return (state & P1_GRP) != 0;
}

static int twl4030reg_enable(struct regulator_dev *rdev)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	int			grp;

	grp = twl4030reg_read(info, VREG_GRP);
	if (grp < 0)
		return grp;

	grp |= P1_GRP;
	return twl4030reg_write(info, VREG_GRP, grp);
}

static int twl4030reg_disable(struct regulator_dev *rdev)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	int			grp;

	grp = twl4030reg_read(info, VREG_GRP);
	if (grp < 0)
		return grp;

	grp &= ~P1_GRP;
	return twl4030reg_write(info, VREG_GRP, grp);
}

static int twl4030reg_get_status(struct regulator_dev *rdev)
{
	int	state = twl4030reg_grp(rdev);

	if (state < 0)
		return state;
	state &= 0x0f;

	
	if (!state)
		return REGULATOR_STATUS_OFF;
	return (state & BIT(3))
		? REGULATOR_STATUS_NORMAL
		: REGULATOR_STATUS_STANDBY;
}

static int twl4030reg_set_mode(struct regulator_dev *rdev, unsigned mode)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	unsigned		message;
	int			status;

	
	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		message = MSG_SINGULAR(DEV_GRP_P1, info->id, RES_STATE_ACTIVE);
		break;
	case REGULATOR_MODE_STANDBY:
		message = MSG_SINGULAR(DEV_GRP_P1, info->id, RES_STATE_SLEEP);
		break;
	default:
		return -EINVAL;
	}

	
	status = twl4030reg_grp(rdev);
	if (status < 0)
		return status;
	if (!(status & (P3_GRP | P2_GRP | P1_GRP)))
		return -EACCES;

	status = twl4030_i2c_write_u8(TWL4030_MODULE_PM_MASTER,
			message >> 8, 0x15  );
	if (status >= 0)
		return status;

	return twl4030_i2c_write_u8(TWL4030_MODULE_PM_MASTER,
			message, 0x16  );
}




#ifdef CONFIG_TWL4030_ALLOW_UNSUPPORTED
#define UNSUP_MASK	0x0000
#else
#define UNSUP_MASK	0x8000
#endif

#define UNSUP(x)	(UNSUP_MASK | (x))
#define IS_UNSUP(x)	(UNSUP_MASK & (x))
#define LDO_MV(x)	(~UNSUP_MASK & (x))


static const u16 VAUX1_VSEL_table[] = {
	UNSUP(1500), UNSUP(1800), 2500, 2800,
	3000, 3000, 3000, 3000,
};
static const u16 VAUX2_4030_VSEL_table[] = {
	UNSUP(1000), UNSUP(1000), UNSUP(1200), 1300,
	1500, 1800, UNSUP(1850), 2500,
	UNSUP(2600), 2800, UNSUP(2850), UNSUP(3000),
	UNSUP(3150), UNSUP(3150), UNSUP(3150), UNSUP(3150),
};
static const u16 VAUX2_VSEL_table[] = {
	1700, 1700, 1900, 1300,
	1500, 1800, 2000, 2500,
	2100, 2800, 2200, 2300,
	2400, 2400, 2400, 2400,
};
static const u16 VAUX3_VSEL_table[] = {
	1500, 1800, 2500, 2800,
	3000, 3000, 3000, 3000,
};
static const u16 VAUX4_VSEL_table[] = {
	700, 1000, 1200, UNSUP(1300),
	1500, 1800, UNSUP(1850), 2500,
	UNSUP(2600), 2800, UNSUP(2850), UNSUP(3000),
	UNSUP(3150), UNSUP(3150), UNSUP(3150), UNSUP(3150),
};
static const u16 VMMC1_VSEL_table[] = {
	1850, 2850, 3000, 3150,
};
static const u16 VMMC2_VSEL_table[] = {
	UNSUP(1000), UNSUP(1000), UNSUP(1200), UNSUP(1300),
	UNSUP(1500), UNSUP(1800), 1850, UNSUP(2500),
	2600, 2800, 2850, 3000,
	3150, 3150, 3150, 3150,
};
static const u16 VPLL1_VSEL_table[] = {
	1000, 1200, 1300, 1800,
	UNSUP(2800), UNSUP(3000), UNSUP(3000), UNSUP(3000),
};
static const u16 VPLL2_VSEL_table[] = {
	700, 1000, 1200, 1300,
	UNSUP(1500), 1800, UNSUP(1850), UNSUP(2500),
	UNSUP(2600), UNSUP(2800), UNSUP(2850), UNSUP(3000),
	UNSUP(3150), UNSUP(3150), UNSUP(3150), UNSUP(3150),
};
static const u16 VSIM_VSEL_table[] = {
	UNSUP(1000), UNSUP(1200), UNSUP(1300), 1800,
	2800, 3000, 3000, 3000,
};
static const u16 VDAC_VSEL_table[] = {
	1200, 1300, 1800, 1800,
};


static int twl4030ldo_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	int			mV = info->table[index];

	return IS_UNSUP(mV) ? 0 : (LDO_MV(mV) * 1000);
}

static int
twl4030ldo_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	int			vsel;

	for (vsel = 0; vsel < info->table_len; vsel++) {
		int mV = info->table[vsel];
		int uV;

		if (IS_UNSUP(mV))
			continue;
		uV = LDO_MV(mV) * 1000;

		

		
		if (min_uV <= uV && uV <= max_uV)
			return twl4030reg_write(info, VREG_DEDICATED, vsel);
	}

	return -EDOM;
}

static int twl4030ldo_get_voltage(struct regulator_dev *rdev)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);
	int			vsel = twl4030reg_read(info, VREG_DEDICATED);

	if (vsel < 0)
		return vsel;

	vsel &= info->table_len - 1;
	return LDO_MV(info->table[vsel]) * 1000;
}

static struct regulator_ops twl4030ldo_ops = {
	.list_voltage	= twl4030ldo_list_voltage,

	.set_voltage	= twl4030ldo_set_voltage,
	.get_voltage	= twl4030ldo_get_voltage,

	.enable		= twl4030reg_enable,
	.disable	= twl4030reg_disable,
	.is_enabled	= twl4030reg_is_enabled,

	.set_mode	= twl4030reg_set_mode,

	.get_status	= twl4030reg_get_status,
};




static int twl4030fixed_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);

	return info->min_mV * 1000;
}

static int twl4030fixed_get_voltage(struct regulator_dev *rdev)
{
	struct twlreg_info	*info = rdev_get_drvdata(rdev);

	return info->min_mV * 1000;
}

static struct regulator_ops twl4030fixed_ops = {
	.list_voltage	= twl4030fixed_list_voltage,

	.get_voltage	= twl4030fixed_get_voltage,

	.enable		= twl4030reg_enable,
	.disable	= twl4030reg_disable,
	.is_enabled	= twl4030reg_is_enabled,

	.set_mode	= twl4030reg_set_mode,

	.get_status	= twl4030reg_get_status,
};



#define TWL_ADJUSTABLE_LDO(label, offset, num) { \
	.base = offset, \
	.id = num, \
	.table_len = ARRAY_SIZE(label##_VSEL_table), \
	.table = label##_VSEL_table, \
	.desc = { \
		.name = #label, \
		.id = TWL4030_REG_##label, \
		.n_voltages = ARRAY_SIZE(label##_VSEL_table), \
		.ops = &twl4030ldo_ops, \
		.type = REGULATOR_VOLTAGE, \
		.owner = THIS_MODULE, \
		}, \
	}

#define TWL_FIXED_LDO(label, offset, mVolts, num) { \
	.base = offset, \
	.id = num, \
	.min_mV = mVolts, \
	.desc = { \
		.name = #label, \
		.id = TWL4030_REG_##label, \
		.n_voltages = 1, \
		.ops = &twl4030fixed_ops, \
		.type = REGULATOR_VOLTAGE, \
		.owner = THIS_MODULE, \
		}, \
	}


static struct twlreg_info twl4030_regs[] = {
	TWL_ADJUSTABLE_LDO(VAUX1, 0x17, 1),
	TWL_ADJUSTABLE_LDO(VAUX2_4030, 0x1b, 2),
	TWL_ADJUSTABLE_LDO(VAUX2, 0x1b, 2),
	TWL_ADJUSTABLE_LDO(VAUX3, 0x1f, 3),
	TWL_ADJUSTABLE_LDO(VAUX4, 0x23, 4),
	TWL_ADJUSTABLE_LDO(VMMC1, 0x27, 5),
	TWL_ADJUSTABLE_LDO(VMMC2, 0x2b, 6),
	
	TWL_ADJUSTABLE_LDO(VPLL2, 0x33, 8),
	TWL_ADJUSTABLE_LDO(VSIM, 0x37, 9),
	TWL_ADJUSTABLE_LDO(VDAC, 0x3b, 10),
	
	TWL_FIXED_LDO(VUSB1V5, 0x71, 1500, 17),
	TWL_FIXED_LDO(VUSB1V8, 0x74, 1800, 18),
	TWL_FIXED_LDO(VUSB3V1, 0x77, 3100, 19),
	
};

static int twl4030reg_probe(struct platform_device *pdev)
{
	int				i;
	struct twlreg_info		*info;
	struct regulator_init_data	*initdata;
	struct regulation_constraints	*c;
	struct regulator_dev		*rdev;

	for (i = 0, info = NULL; i < ARRAY_SIZE(twl4030_regs); i++) {
		if (twl4030_regs[i].desc.id != pdev->id)
			continue;
		info = twl4030_regs + i;
		break;
	}
	if (!info)
		return -ENODEV;

	initdata = pdev->dev.platform_data;
	if (!initdata)
		return -EINVAL;

	
	c = &initdata->constraints;
	c->valid_modes_mask &= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY;
	c->valid_ops_mask &= REGULATOR_CHANGE_VOLTAGE
				| REGULATOR_CHANGE_MODE
				| REGULATOR_CHANGE_STATUS;

	rdev = regulator_register(&info->desc, &pdev->dev, initdata, info);
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "can't register %s, %ld\n",
				info->desc.name, PTR_ERR(rdev));
		return PTR_ERR(rdev);
	}
	platform_set_drvdata(pdev, rdev);

	

	return 0;
}

static int __devexit twl4030reg_remove(struct platform_device *pdev)
{
	regulator_unregister(platform_get_drvdata(pdev));
	return 0;
}

MODULE_ALIAS("platform:twl4030_reg");

static struct platform_driver twl4030reg_driver = {
	.probe		= twl4030reg_probe,
	.remove		= __devexit_p(twl4030reg_remove),
	
	.driver.name	= "twl4030_reg",
	.driver.owner	= THIS_MODULE,
};

static int __init twl4030reg_init(void)
{
	return platform_driver_register(&twl4030reg_driver);
}
subsys_initcall(twl4030reg_init);

static void __exit twl4030reg_exit(void)
{
	platform_driver_unregister(&twl4030reg_driver);
}
module_exit(twl4030reg_exit)

MODULE_DESCRIPTION("TWL4030 regulator driver");
MODULE_LICENSE("GPL");
