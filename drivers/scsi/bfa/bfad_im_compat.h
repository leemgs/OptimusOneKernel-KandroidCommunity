

#ifndef __BFAD_IM_COMPAT_H__
#define __BFAD_IM_COMPAT_H__

extern u32 *bfi_image_buf;
extern u32 bfi_image_size;

extern struct device_attribute *bfad_im_host_attrs[];
extern struct device_attribute *bfad_im_vport_attrs[];

u32 *bfad_get_firmware_buf(struct pci_dev *pdev);
u32 *bfad_read_firmware(struct pci_dev *pdev, u32 **bfi_image,
			u32 *bfi_image_size, char *fw_name);

static inline u32 *
bfad_load_fwimg(struct pci_dev *pdev)
{
	return(bfad_get_firmware_buf(pdev));
}

static inline void
bfad_free_fwimg(void)
{
	if (bfi_image_ct_size && bfi_image_ct)
		vfree(bfi_image_ct);
	if (bfi_image_cb_size && bfi_image_cb)
		vfree(bfi_image_cb);
}

#endif
