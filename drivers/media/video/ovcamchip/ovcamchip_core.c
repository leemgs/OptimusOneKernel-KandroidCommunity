

#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <media/v4l2-device.h>
#include <media/v4l2-i2c-drv.h>
#include "ovcamchip_priv.h"

#define DRIVER_VERSION "v2.27 for Linux 2.6"
#define DRIVER_AUTHOR "Mark McClelland <mark@alpha.dyndns.org>"
#define DRIVER_DESC "OV camera chip I2C driver"

#define PINFO(fmt, args...) printk(KERN_INFO "ovcamchip: " fmt "\n" , ## args);
#define PERROR(fmt, args...) printk(KERN_ERR "ovcamchip: " fmt "\n" , ## args);

#ifdef DEBUG
int ovcamchip_debug = 0;
static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug,
  "Debug level: 0=none, 1=inits, 2=warning, 3=config, 4=functions, 5=all");
#endif


static int mono = -1;
module_param(mono, int, 0);
MODULE_PARM_DESC(mono,
  "1=chips are monochrome (OVx1xx), 0=force color, -1=autodetect (default)");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");



#define GENERIC_REG_ID_HIGH       0x1C	
#define GENERIC_REG_ID_LOW        0x1D	
#define GENERIC_REG_COM_I         0x29	

static char *chip_names[NUM_CC_TYPES] = {
	[CC_UNKNOWN]	= "Unknown chip",
	[CC_OV76BE]	= "OV76BE",
	[CC_OV7610]	= "OV7610",
	[CC_OV7620]	= "OV7620",
	[CC_OV7620AE]	= "OV7620AE",
	[CC_OV6620]	= "OV6620",
	[CC_OV6630]	= "OV6630",
	[CC_OV6630AE]	= "OV6630AE",
	[CC_OV6630AF]	= "OV6630AF",
};



int ov_write_regvals(struct i2c_client *c, struct ovcamchip_regvals *rvals)
{
	int rc;

	while (rvals->reg != 0xff) {
		rc = ov_write(c, rvals->reg, rvals->val);
		if (rc < 0)
			return rc;
		rvals++;
	}

	return 0;
}


int ov_write_mask(struct i2c_client *c,
		  unsigned char reg,
		  unsigned char value,
		  unsigned char mask)
{
	int rc;
	unsigned char oldval, newval;

	if (mask == 0xff) {
		newval = value;
	} else {
		rc = ov_read(c, reg, &oldval);
		if (rc < 0)
			return rc;

		oldval &= (~mask);		
		value &= mask;			
		newval = oldval | value;	
	}

	return ov_write(c, reg, newval);
}




static int init_camchip(struct i2c_client *c)
{
	int i, success;
	unsigned char high, low;

	
	ov_write(c, 0x12, 0x80);

	
	msleep(150);

	for (i = 0, success = 0; i < I2C_DETECT_RETRIES && !success; i++) {
		if (ov_read(c, GENERIC_REG_ID_HIGH, &high) >= 0) {
			if (ov_read(c, GENERIC_REG_ID_LOW, &low) >= 0) {
				if (high == 0x7F && low == 0xA2) {
					success = 1;
					continue;
				}
			}
		}

		
		ov_write(c, 0x12, 0x80);

		
		msleep(150);

		
		ov_read(c, 0x00, &low);
	}

	if (!success)
		return -EIO;

	PDEBUG(1, "I2C synced in %d attempt(s)", i);

	return 0;
}


static int ov7xx0_detect(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	int rc;
	unsigned char val;

	PDEBUG(4, "");

	
	rc = ov_read(c, GENERIC_REG_COM_I, &val);
	if (rc < 0) {
		PERROR("Error detecting ov7xx0 type");
		return rc;
	}

	if ((val & 3) == 3) {
		PINFO("Camera chip is an OV7610");
		ov->subtype = CC_OV7610;
	} else if ((val & 3) == 1) {
		rc = ov_read(c, 0x15, &val);
		if (rc < 0) {
			PERROR("Error detecting ov7xx0 type");
			return rc;
		}

		if (val & 1) {
			PINFO("Camera chip is an OV7620AE");
			
			ov->subtype = CC_OV7620;
		} else {
			PINFO("Camera chip is an OV76BE");
			ov->subtype = CC_OV76BE;
		}
	} else if ((val & 3) == 0) {
		PINFO("Camera chip is an OV7620");
		ov->subtype = CC_OV7620;
	} else {
		PERROR("Unknown camera chip version: %d", val & 3);
		return -ENOSYS;
	}

	if (ov->subtype == CC_OV76BE)
		ov->sops = &ov76be_ops;
	else if (ov->subtype == CC_OV7620)
		ov->sops = &ov7x20_ops;
	else
		ov->sops = &ov7x10_ops;

	return 0;
}


static int ov6xx0_detect(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	int rc;
	unsigned char val;

	PDEBUG(4, "");

	
	rc = ov_read(c, GENERIC_REG_COM_I, &val);
	if (rc < 0) {
		PERROR("Error detecting ov6xx0 type");
		return -1;
	}

	if ((val & 3) == 0) {
		ov->subtype = CC_OV6630;
		PINFO("Camera chip is an OV6630");
	} else if ((val & 3) == 1) {
		ov->subtype = CC_OV6620;
		PINFO("Camera chip is an OV6620");
	} else if ((val & 3) == 2) {
		ov->subtype = CC_OV6630;
		PINFO("Camera chip is an OV6630AE");
	} else if ((val & 3) == 3) {
		ov->subtype = CC_OV6630;
		PINFO("Camera chip is an OV6630AF");
	}

	if (ov->subtype == CC_OV6620)
		ov->sops = &ov6x20_ops;
	else
		ov->sops = &ov6x30_ops;

	return 0;
}

static int ovcamchip_detect(struct i2c_client *c)
{
	

	
	PDEBUG(3, "Testing for 0V7xx0");
	if (init_camchip(c) < 0)
		return -ENODEV;
	
	if (c->addr & 1) {
		if (ov7xx0_detect(c) < 0) {
			PERROR("Failed to init OV7xx0");
			return -EIO;
		}
		return 0;
	}
	
	PDEBUG(3, "Testing for 0V6xx0");
	if (ov6xx0_detect(c) < 0) {
		PERROR("Failed to init OV6xx0");
		return -EIO;
	}
	return 0;
}



static long ovcamchip_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct ovcamchip *ov = to_ovcamchip(sd);
	struct i2c_client *c = v4l2_get_subdevdata(sd);

	if (!ov->initialized &&
	    cmd != OVCAMCHIP_CMD_Q_SUBTYPE &&
	    cmd != OVCAMCHIP_CMD_INITIALIZE) {
		v4l2_err(sd, "Camera chip not initialized yet!\n");
		return -EPERM;
	}

	switch (cmd) {
	case OVCAMCHIP_CMD_Q_SUBTYPE:
	{
		*(int *)arg = ov->subtype;
		return 0;
	}
	case OVCAMCHIP_CMD_INITIALIZE:
	{
		int rc;

		if (mono == -1)
			ov->mono = *(int *)arg;
		else
			ov->mono = mono;

		if (ov->mono) {
			if (ov->subtype != CC_OV7620)
				v4l2_warn(sd, "Monochrome not "
					"implemented for this chip\n");
			else
				v4l2_info(sd, "Initializing chip as "
					"monochrome\n");
		}

		rc = ov->sops->init(c);
		if (rc < 0)
			return rc;

		ov->initialized = 1;
		return 0;
	}
	default:
		return ov->sops->command(c, cmd, arg);
	}
}



static const struct v4l2_subdev_core_ops ovcamchip_core_ops = {
	.ioctl = ovcamchip_ioctl,
};

static const struct v4l2_subdev_ops ovcamchip_ops = {
	.core = &ovcamchip_core_ops,
};

static int ovcamchip_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct ovcamchip *ov;
	struct v4l2_subdev *sd;
	int rc = 0;

	ov = kzalloc(sizeof *ov, GFP_KERNEL);
	if (!ov) {
		rc = -ENOMEM;
		goto no_ov;
	}
	sd = &ov->sd;
	v4l2_i2c_subdev_init(sd, client, &ovcamchip_ops);

	rc = ovcamchip_detect(client);
	if (rc < 0)
		goto error;

	v4l_info(client, "%s found @ 0x%02x (%s)\n",
			chip_names[ov->subtype], client->addr << 1, client->adapter->name);

	PDEBUG(1, "Camera chip detection complete");

	return rc;
error:
	kfree(ov);
no_ov:
	PDEBUG(1, "returning %d", rc);
	return rc;
}

static int ovcamchip_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ovcamchip *ov = to_ovcamchip(sd);
	int rc;

	v4l2_device_unregister_subdev(sd);
	rc = ov->sops->free(client);
	if (rc < 0)
		return rc;

	kfree(ov);
	return 0;
}



static const struct i2c_device_id ovcamchip_id[] = {
	{ "ovcamchip", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ovcamchip_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "ovcamchip",
	.probe = ovcamchip_probe,
	.remove = ovcamchip_remove,
	.id_table = ovcamchip_id,
};
