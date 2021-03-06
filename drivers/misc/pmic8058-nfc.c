


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mfd/pmic8058.h>
#include <linux/pmic8058-nfc.h>
#include <linux/debugfs.h>


#define	SSBI_REG_NFC_CTRL	0x14D
#define	SSBI_REG_NFC_TEST	0x14E


#define	PM8058_NFC_SUPPORT_EN		0x80
#define	PM8058_NFC_LDO_EN		0x40
#define	PM8058_NFC_EN			0x20
#define	PM8058_NFC_EXT_VDDLDO_EN	0x10
#define	PM8058_NFC_VPH_PWR_EN		0x08
#define	PM8058_NFC_RESERVED		0x04
#define	PM8058_NFC_VDDLDO_LEVEL		0x03


#define	PM8058_NFC_VDDLDO_MON_EN	0x80
#define	PM8058_NFC_ATEST_EN		0x40
#define	PM8058_NFC_DTEST1_EN		0x20
#define	PM8058_NFC_RESERVED2		0x18
#define	PM8058_NFC_VDDLDO_OK_S		0x04
#define	PM8058_NFC_MBG_EN_S		0x02
#define	PM8058_NFC_EXT_EN_S		0x01

struct pm8058_nfc_device {
	struct mutex		nfc_mutex;
	struct pm8058_chip	*pm_chip;
#if defined(CONFIG_DEBUG_FS)
	struct dentry 		*dent;
#endif
};
static struct pm8058_nfc_device	*nfc_dev;



struct pm8058_nfc_device *pm8058_nfc_request(void)
{
	return nfc_dev;
}
EXPORT_SYMBOL(pm8058_nfc_request);


int pm8058_nfc_config(struct pm8058_nfc_device *nfcdev, u32 mask, u32 flags)
{
	u8	nfc_ctrl, nfc_test, m, f;
	int	rc;

	if (nfcdev == NULL || IS_ERR(nfcdev) || !mask)
		return -EINVAL;
	if (nfcdev->pm_chip == NULL)
		return -ENODEV;

	mutex_lock(&nfcdev->nfc_mutex);

	if (!(mask & PM_NFC_CTRL_REQ))
		goto config_test;

	rc = pm8058_read(nfcdev->pm_chip, SSBI_REG_NFC_CTRL, &nfc_ctrl, 1);
	if (rc) {
		pr_err("%s: FAIL pm8058_read(): rc=%d (nfc_ctrl=0x%x)\n",
		       __func__, rc, nfc_ctrl);
		goto config_done;
	}

	m = mask & 0x00ff;
	f = flags & 0x00ff;
	nfc_ctrl &= ~m;
	nfc_ctrl |= m & f;

	rc = pm8058_write(nfcdev->pm_chip, SSBI_REG_NFC_CTRL, &nfc_ctrl, 1);
	if (rc) {
		pr_err("%s: FAIL pm8058_write(): rc=%d (nfc_ctrl=0x%x)\n",
		       __func__, rc, nfc_ctrl);
		goto config_done;
	}

config_test:
	if (!(mask & PM_NFC_TEST_REQ))
		goto config_done;

	rc = pm8058_read(nfcdev->pm_chip, SSBI_REG_NFC_TEST, &nfc_test, 1);
	if (rc) {
		pr_err("%s: FAIL pm8058_read(): rc=%d (nfc_test=0x%x)\n",
		       __func__, rc, nfc_test);
		goto config_done;
	}

	m = (mask >> 8) & 0x00ff;
	f = (flags >> 8) & 0x00ff;
	nfc_test &= ~m;
	nfc_test |= m & f;

	rc = pm8058_write(nfcdev->pm_chip, SSBI_REG_NFC_TEST, &nfc_test, 1);
	if (rc) {
		pr_err("%s: FAIL pm8058_write(): rc=%d (nfc_test=0x%x)\n",
		       __func__, rc, nfc_test);
		goto config_done;
	}

config_done:
	mutex_unlock(&nfcdev->nfc_mutex);
	return 0;
}
EXPORT_SYMBOL(pm8058_nfc_config);


int pm8058_nfc_get_status(struct pm8058_nfc_device *nfcdev,
			  u32 mask, u32 *status)
{
	u8	nfc_ctrl, nfc_test;
	u32	st;
	int	rc;

	if (nfcdev == NULL || IS_ERR(nfcdev) || status == NULL)
		return -EINVAL;
	if (nfcdev->pm_chip == NULL)
		return -ENODEV;

	st = 0;
	mutex_lock(&nfcdev->nfc_mutex);

	if (!(mask & PM_NFC_CTRL_REQ))
		goto read_test;

	rc = pm8058_read(nfcdev->pm_chip, SSBI_REG_NFC_CTRL, &nfc_ctrl, 1);
	if (rc) {
		pr_err("%s: FAIL pm8058_read(): rc=%d (nfc_ctrl=0x%x)\n",
		       __func__, rc, nfc_ctrl);
		goto get_status_done;
	}

read_test:
	if (!(mask & (PM_NFC_TEST_REQ | PM_NFC_TEST_STATUS)))
		goto get_status_done;

	rc = pm8058_read(nfcdev->pm_chip, SSBI_REG_NFC_TEST, &nfc_test, 1);
	if (rc)
		pr_err("%s: FAIL pm8058_read(): rc=%d (nfc_test=0x%x)\n",
		       __func__, rc, nfc_test);

get_status_done:
	st = nfc_ctrl;
	st |= nfc_test << 8;
	*status = st;

	mutex_unlock(&nfcdev->nfc_mutex);
	return 0;
}
EXPORT_SYMBOL(pm8058_nfc_get_status);


void pm8058_nfc_free(struct pm8058_nfc_device *nfcdev)
{
	
	pm8058_nfc_config(nfcdev, PM_NFC_CTRL_REQ, 0);
}
EXPORT_SYMBOL(pm8058_nfc_free);

#if defined(CONFIG_DEBUG_FS)
static int pm8058_nfc_debug_set(void *data, u64 val)
{
	struct pm8058_nfc_device *nfcdev;
	u32	mask, control;
	int	rc;

	nfcdev = (struct pm8058_nfc_device *)data;
	control = (u32)val & 0xffff;
	mask = ((u32)val >> 16) & 0xffff;
	rc = pm8058_nfc_config(nfcdev, mask, control);
	if (rc)
		pr_err("%s: ERR pm8058_nfc_config: rc=%d, "
		       "[mask, control]=[0x%x, 0x%x]\n",
		       __func__, rc, mask, control);

	return 0;
}

static int pm8058_nfc_debug_get(void *data, u64 *val)
{
	struct pm8058_nfc_device *nfcdev;
	u32	status;
	int	rc;

	nfcdev = (struct pm8058_nfc_device *)data;
	rc = pm8058_nfc_get_status(nfcdev, (u32)-1, &status);
	if (rc)
		pr_err("%s: ERR pm8058_nfc_get_status: rc=%d, status=0x%x\n",
		       __func__, rc, status);

	if (val)
		*val = (u64)status;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pm8058_nfc_fops, pm8058_nfc_debug_get,
			pm8058_nfc_debug_set, "%llu\n");

static int pm8058_nfc_debug_init(struct pm8058_nfc_device *nfcdev)
{
	struct dentry *dent;

	dent = debugfs_create_file("pm8058-nfc", 0644, NULL,
				   (void *)nfcdev, &pm8058_nfc_fops);

	if (dent == NULL || IS_ERR(dent))
		pr_err("%s: ERR debugfs_create_file: dent=0x%x\n",
		       __func__, (unsigned)dent);

	nfcdev->dent = dent;
	return 0;
}
#endif

static int __devinit pmic8058_nfc_probe(struct platform_device *pdev)
{
	struct pm8058_chip		*pm_chip;
	struct pm8058_nfc_device	*nfcdev;

	pm_chip = platform_get_drvdata(pdev);
	if (pm_chip == NULL) {
		pr_err("%s: no parent data passed in.\n", __func__);
		return -EFAULT;
	}

	nfcdev = kzalloc(sizeof *nfcdev, GFP_KERNEL);
	if (nfcdev == NULL) {
		pr_err("%s: kzalloc() failed.\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&nfcdev->nfc_mutex);

	nfcdev->pm_chip = pm_chip;
	nfc_dev = nfcdev;
	platform_set_drvdata(pdev, nfcdev);

#if defined(CONFIG_DEBUG_FS)
	pm8058_nfc_debug_init(nfc_dev);
#endif

	pr_notice("%s: OK\n", __func__);
	return 0;
}

static int __devexit pmic8058_nfc_remove(struct platform_device *pdev)
{
	struct pm8058_nfc_device *nfcdev = platform_get_drvdata(pdev);

#if defined(CONFIG_DEBUG_FS)
	debugfs_remove(nfcdev->dent);
#endif

	platform_set_drvdata(pdev, nfcdev->pm_chip);
	kfree(nfcdev);
	return 0;
}

static struct platform_driver pmic8058_nfc_driver = {
	.probe		= pmic8058_nfc_probe,
	.remove		= __devexit_p(pmic8058_nfc_remove),
	.driver		= {
		.name = "pm8058-nfc",
		.owner = THIS_MODULE,
	},
};

static int __init pm8058_nfc_init(void)
{
	return platform_driver_register(&pmic8058_nfc_driver);
}

static void __exit pm8058_nfc_exit(void)
{
	platform_driver_unregister(&pmic8058_nfc_driver);
}

module_init(pm8058_nfc_init);
module_exit(pm8058_nfc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PMIC8058 NFC driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:pmic8058-nfc");
