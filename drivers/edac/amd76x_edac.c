

#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/slab.h>
#include <linux/edac.h>
#include "edac_core.h"

#define AMD76X_REVISION	" Ver: 2.0.2 "  __DATE__
#define EDAC_MOD_STR	"amd76x_edac"

#define amd76x_printk(level, fmt, arg...) \
	edac_printk(level, "amd76x", fmt, ##arg)

#define amd76x_mc_printk(mci, level, fmt, arg...) \
	edac_mc_chipset_printk(mci, level, "amd76x", fmt, ##arg)

#define AMD76X_NR_CSROWS 8
#define AMD76X_NR_CHANS  1
#define AMD76X_NR_DIMMS  4



#define AMD76X_ECC_MODE_STATUS	0x48	

#define AMD76X_DRAM_MODE_STATUS	0x58	

#define AMD76X_MEM_BASE_ADDR	0xC0	

struct amd76x_error_info {
	u32 ecc_mode_status;
};

enum amd76x_chips {
	AMD761 = 0,
	AMD762
};

struct amd76x_dev_info {
	const char *ctl_name;
};

static const struct amd76x_dev_info amd76x_devs[] = {
	[AMD761] = {
		.ctl_name = "AMD761"},
	[AMD762] = {
		.ctl_name = "AMD762"},
};

static struct edac_pci_ctl_info *amd76x_pci;


static void amd76x_get_error_info(struct mem_ctl_info *mci,
				struct amd76x_error_info *info)
{
	struct pci_dev *pdev;

	pdev = to_pci_dev(mci->dev);
	pci_read_config_dword(pdev, AMD76X_ECC_MODE_STATUS,
			&info->ecc_mode_status);

	if (info->ecc_mode_status & BIT(8))
		pci_write_bits32(pdev, AMD76X_ECC_MODE_STATUS,
				 (u32) BIT(8), (u32) BIT(8));

	if (info->ecc_mode_status & BIT(9))
		pci_write_bits32(pdev, AMD76X_ECC_MODE_STATUS,
				 (u32) BIT(9), (u32) BIT(9));
}


static int amd76x_process_error_info(struct mem_ctl_info *mci,
				struct amd76x_error_info *info,
				int handle_errors)
{
	int error_found;
	u32 row;

	error_found = 0;

	
	if (info->ecc_mode_status & BIT(8)) {
		error_found = 1;

		if (handle_errors) {
			row = (info->ecc_mode_status >> 4) & 0xf;
			edac_mc_handle_ue(mci, mci->csrows[row].first_page, 0,
					row, mci->ctl_name);
		}
	}

	
	if (info->ecc_mode_status & BIT(9)) {
		error_found = 1;

		if (handle_errors) {
			row = info->ecc_mode_status & 0xf;
			edac_mc_handle_ce(mci, mci->csrows[row].first_page, 0,
					0, row, 0, mci->ctl_name);
		}
	}

	return error_found;
}


static void amd76x_check(struct mem_ctl_info *mci)
{
	struct amd76x_error_info info;
	debugf3("%s()\n", __func__);
	amd76x_get_error_info(mci, &info);
	amd76x_process_error_info(mci, &info, 1);
}

static void amd76x_init_csrows(struct mem_ctl_info *mci, struct pci_dev *pdev,
			enum edac_type edac_mode)
{
	struct csrow_info *csrow;
	u32 mba, mba_base, mba_mask, dms;
	int index;

	for (index = 0; index < mci->nr_csrows; index++) {
		csrow = &mci->csrows[index];

		
		pci_read_config_dword(pdev,
				AMD76X_MEM_BASE_ADDR + (index * 4), &mba);

		if (!(mba & BIT(0)))
			continue;

		mba_base = mba & 0xff800000UL;
		mba_mask = ((mba & 0xff80) << 16) | 0x7fffffUL;
		pci_read_config_dword(pdev, AMD76X_DRAM_MODE_STATUS, &dms);
		csrow->first_page = mba_base >> PAGE_SHIFT;
		csrow->nr_pages = (mba_mask + 1) >> PAGE_SHIFT;
		csrow->last_page = csrow->first_page + csrow->nr_pages - 1;
		csrow->page_mask = mba_mask >> PAGE_SHIFT;
		csrow->grain = csrow->nr_pages << PAGE_SHIFT;
		csrow->mtype = MEM_RDDR;
		csrow->dtype = ((dms >> index) & 0x1) ? DEV_X4 : DEV_UNKNOWN;
		csrow->edac_mode = edac_mode;
	}
}


static int amd76x_probe1(struct pci_dev *pdev, int dev_idx)
{
	static const enum edac_type ems_modes[] = {
		EDAC_NONE,
		EDAC_EC,
		EDAC_SECDED,
		EDAC_SECDED
	};
	struct mem_ctl_info *mci = NULL;
	u32 ems;
	u32 ems_mode;
	struct amd76x_error_info discard;

	debugf0("%s()\n", __func__);
	pci_read_config_dword(pdev, AMD76X_ECC_MODE_STATUS, &ems);
	ems_mode = (ems >> 10) & 0x3;
	mci = edac_mc_alloc(0, AMD76X_NR_CSROWS, AMD76X_NR_CHANS, 0);

	if (mci == NULL) {
		return -ENOMEM;
	}

	debugf0("%s(): mci = %p\n", __func__, mci);
	mci->dev = &pdev->dev;
	mci->mtype_cap = MEM_FLAG_RDDR;
	mci->edac_ctl_cap = EDAC_FLAG_NONE | EDAC_FLAG_EC | EDAC_FLAG_SECDED;
	mci->edac_cap = ems_mode ?
		(EDAC_FLAG_EC | EDAC_FLAG_SECDED) : EDAC_FLAG_NONE;
	mci->mod_name = EDAC_MOD_STR;
	mci->mod_ver = AMD76X_REVISION;
	mci->ctl_name = amd76x_devs[dev_idx].ctl_name;
	mci->dev_name = pci_name(pdev);
	mci->edac_check = amd76x_check;
	mci->ctl_page_to_phys = NULL;

	amd76x_init_csrows(mci, pdev, ems_modes[ems_mode]);
	amd76x_get_error_info(mci, &discard);	

	
	if (edac_mc_add_mc(mci)) {
		debugf3("%s(): failed edac_mc_add_mc()\n", __func__);
		goto fail;
	}

	
	amd76x_pci = edac_pci_create_generic_ctl(&pdev->dev, EDAC_MOD_STR);
	if (!amd76x_pci) {
		printk(KERN_WARNING
			"%s(): Unable to create PCI control\n",
			__func__);
		printk(KERN_WARNING
			"%s(): PCI error report via EDAC not setup\n",
			__func__);
	}

	
	debugf3("%s(): success\n", __func__);
	return 0;

fail:
	edac_mc_free(mci);
	return -ENODEV;
}


static int __devinit amd76x_init_one(struct pci_dev *pdev,
				const struct pci_device_id *ent)
{
	debugf0("%s()\n", __func__);

	
	return amd76x_probe1(pdev, ent->driver_data);
}


static void __devexit amd76x_remove_one(struct pci_dev *pdev)
{
	struct mem_ctl_info *mci;

	debugf0("%s()\n", __func__);

	if (amd76x_pci)
		edac_pci_release_generic_ctl(amd76x_pci);

	if ((mci = edac_mc_del_mc(&pdev->dev)) == NULL)
		return;

	edac_mc_free(mci);
}

static const struct pci_device_id amd76x_pci_tbl[] __devinitdata = {
	{
	 PCI_VEND_DEV(AMD, FE_GATE_700C), PCI_ANY_ID, PCI_ANY_ID, 0, 0,
	 AMD762},
	{
	 PCI_VEND_DEV(AMD, FE_GATE_700E), PCI_ANY_ID, PCI_ANY_ID, 0, 0,
	 AMD761},
	{
	 0,
	 }			
};

MODULE_DEVICE_TABLE(pci, amd76x_pci_tbl);

static struct pci_driver amd76x_driver = {
	.name = EDAC_MOD_STR,
	.probe = amd76x_init_one,
	.remove = __devexit_p(amd76x_remove_one),
	.id_table = amd76x_pci_tbl,
};

static int __init amd76x_init(void)
{
       
       opstate_init();

	return pci_register_driver(&amd76x_driver);
}

static void __exit amd76x_exit(void)
{
	pci_unregister_driver(&amd76x_driver);
}

module_init(amd76x_init);
module_exit(amd76x_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Networx (http://lnxi.com) Thayne Harbaugh");
MODULE_DESCRIPTION("MC support for AMD 76x memory controllers");

module_param(edac_op_state, int, 0444);
MODULE_PARM_DESC(edac_op_state, "EDAC Error Reporting state: 0=Poll,1=NMI");
