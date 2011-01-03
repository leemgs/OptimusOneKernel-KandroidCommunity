
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/gpio.h>

#include <mach/hardware.h>
#include <asm/mach/map.h>
#include <mach/iomux-v3.h>

static void __iomem *base;

static unsigned long iomux_v3_pad_alloc_map[0x200 / BITS_PER_LONG];


int mxc_iomux_v3_setup_pad(struct pad_desc *pad)
{
	unsigned int pad_ofs = pad->pad_ctrl_ofs;

	if (test_and_set_bit(pad_ofs >> 2, iomux_v3_pad_alloc_map))
		return -EBUSY;
	if (pad->mux_ctrl_ofs)
		__raw_writel(pad->mux_mode, base + pad->mux_ctrl_ofs);

	if (pad->select_input_ofs)
		__raw_writel(pad->select_input,
				base + pad->select_input_ofs);

	if (!(pad->pad_ctrl & NO_PAD_CTRL) && pad->pad_ctrl_ofs)
		__raw_writel(pad->pad_ctrl, base + pad->pad_ctrl_ofs);
	return 0;
}
EXPORT_SYMBOL(mxc_iomux_v3_setup_pad);

int mxc_iomux_v3_setup_multiple_pads(struct pad_desc *pad_list, unsigned count)
{
	struct pad_desc *p = pad_list;
	int i;
	int ret;

	for (i = 0; i < count; i++) {
		ret = mxc_iomux_v3_setup_pad(p);
		if (ret)
			goto setup_error;
		p++;
	}
	return 0;

setup_error:
	mxc_iomux_v3_release_multiple_pads(pad_list, i);
	return ret;
}
EXPORT_SYMBOL(mxc_iomux_v3_setup_multiple_pads);

void mxc_iomux_v3_release_pad(struct pad_desc *pad)
{
	unsigned int pad_ofs = pad->pad_ctrl_ofs;

	clear_bit(pad_ofs >> 2, iomux_v3_pad_alloc_map);
}
EXPORT_SYMBOL(mxc_iomux_v3_release_pad);

void mxc_iomux_v3_release_multiple_pads(struct pad_desc *pad_list, int count)
{
	struct pad_desc *p = pad_list;
	int i;

	for (i = 0; i < count; i++) {
		mxc_iomux_v3_release_pad(p);
		p++;
	}
}
EXPORT_SYMBOL(mxc_iomux_v3_release_multiple_pads);

void mxc_iomux_v3_init(void __iomem *iomux_v3_base)
{
	base = iomux_v3_base;
}
