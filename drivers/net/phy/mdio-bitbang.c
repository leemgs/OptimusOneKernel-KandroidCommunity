

#include <linux/module.h>
#include <linux/mdio-bitbang.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>

#define MDIO_READ 1
#define MDIO_WRITE 0

#define MDIO_SETUP_TIME 10
#define MDIO_HOLD_TIME 10


#define MDIO_DELAY 250


#define MDIO_READ_DELAY 350


static void mdiobb_send_bit(struct mdiobb_ctrl *ctrl, int val)
{
	const struct mdiobb_ops *ops = ctrl->ops;

	ops->set_mdio_data(ctrl, val);
	ndelay(MDIO_DELAY);
	ops->set_mdc(ctrl, 1);
	ndelay(MDIO_DELAY);
	ops->set_mdc(ctrl, 0);
}


static int mdiobb_get_bit(struct mdiobb_ctrl *ctrl)
{
	const struct mdiobb_ops *ops = ctrl->ops;

	ndelay(MDIO_DELAY);
	ops->set_mdc(ctrl, 1);
	ndelay(MDIO_READ_DELAY);
	ops->set_mdc(ctrl, 0);

	return ops->get_mdio_data(ctrl);
}


static void mdiobb_send_num(struct mdiobb_ctrl *ctrl, u16 val, int bits)
{
	int i;

	for (i = bits - 1; i >= 0; i--)
		mdiobb_send_bit(ctrl, (val >> i) & 1);
}


static u16 mdiobb_get_num(struct mdiobb_ctrl *ctrl, int bits)
{
	int i;
	u16 ret = 0;

	for (i = bits - 1; i >= 0; i--) {
		ret <<= 1;
		ret |= mdiobb_get_bit(ctrl);
	}

	return ret;
}


static void mdiobb_cmd(struct mdiobb_ctrl *ctrl, int read, u8 phy, u8 reg)
{
	const struct mdiobb_ops *ops = ctrl->ops;
	int i;

	ops->set_mdio_dir(ctrl, 1);

	

	for (i = 0; i < 32; i++)
		mdiobb_send_bit(ctrl, 1);

	
	mdiobb_send_bit(ctrl, 0);
	mdiobb_send_bit(ctrl, 1);
	mdiobb_send_bit(ctrl, read);
	mdiobb_send_bit(ctrl, !read);

	mdiobb_send_num(ctrl, phy, 5);
	mdiobb_send_num(ctrl, reg, 5);
}


static int mdiobb_read(struct mii_bus *bus, int phy, int reg)
{
	struct mdiobb_ctrl *ctrl = bus->priv;
	int ret, i;

	mdiobb_cmd(ctrl, MDIO_READ, phy, reg);
	ctrl->ops->set_mdio_dir(ctrl, 0);

	
	if (mdiobb_get_bit(ctrl) != 0) {
		
		for (i = 0; i < 32; i++)
			mdiobb_get_bit(ctrl);

		return 0xffff;
	}

	ret = mdiobb_get_num(ctrl, 16);
	mdiobb_get_bit(ctrl);
	return ret;
}

static int mdiobb_write(struct mii_bus *bus, int phy, int reg, u16 val)
{
	struct mdiobb_ctrl *ctrl = bus->priv;

	mdiobb_cmd(ctrl, MDIO_WRITE, phy, reg);

	
	mdiobb_send_bit(ctrl, 1);
	mdiobb_send_bit(ctrl, 0);

	mdiobb_send_num(ctrl, val, 16);

	ctrl->ops->set_mdio_dir(ctrl, 0);
	mdiobb_get_bit(ctrl);
	return 0;
}

struct mii_bus *alloc_mdio_bitbang(struct mdiobb_ctrl *ctrl)
{
	struct mii_bus *bus;

	bus = mdiobus_alloc();
	if (!bus)
		return NULL;

	__module_get(ctrl->ops->owner);

	bus->read = mdiobb_read;
	bus->write = mdiobb_write;
	bus->priv = ctrl;

	return bus;
}
EXPORT_SYMBOL(alloc_mdio_bitbang);

void free_mdio_bitbang(struct mii_bus *bus)
{
	struct mdiobb_ctrl *ctrl = bus->priv;

	module_put(ctrl->ops->owner);
	mdiobus_free(bus);
}
EXPORT_SYMBOL(free_mdio_bitbang);

MODULE_LICENSE("GPL");
