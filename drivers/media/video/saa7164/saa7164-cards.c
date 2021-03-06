

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>

#include "saa7164.h"


#define REGLEN_8bit	1
#define REGLEN_16bit	2

struct saa7164_board saa7164_boards[] = {
	[SAA7164_BOARD_UNKNOWN] = {
		
		.name		= "Unknown",
	},
	[SAA7164_BOARD_UNKNOWN_REV2] = {
		
		
		.name		= "Generic Rev2",
		.chiprev	= SAA7164_CHIP_REV2,
	},
	[SAA7164_BOARD_UNKNOWN_REV3] = {
		
		
		.name		= "Generic Rev3",
		.chiprev	= SAA7164_CHIP_REV3,
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2200] = {
		.name		= "Hauppauge WinTV-HVR2200",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV3,
		.unit		= {{
			.id		= 0x1d,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1b,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1e,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x10 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1f,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x12 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2200_2] = {
		.name		= "Hauppauge WinTV-HVR2200",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV2,
		.unit		= {{
			.id		= 0x06,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x05,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x10 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1e,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1f,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x12 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2200_3] = {
		.name		= "Hauppauge WinTV-HVR2200",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV2,
		.unit		= {{
			.id		= 0x1d,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x05,
			.type		= SAA7164_UNIT_ANALOG_DEMODULATOR,
			.name		= "TDA8290-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x84 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1b,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1c,
			.type		= SAA7164_UNIT_ANALOG_DEMODULATOR,
			.name		= "TDA8290-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x84 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1e,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x10 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1f,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "TDA10048-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x12 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2250] = {
		.name		= "Hauppauge WinTV-HVR2250",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV3,
		.unit		= {{
			.id		= 0x22,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x07,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x08,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x1e,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x20,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x23,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2250_2] = {
		.name		= "Hauppauge WinTV-HVR2250",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV3,
		.unit		= {{
			.id		= 0x28,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x07,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x08,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x24,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x26,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x29,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
	[SAA7164_BOARD_HAUPPAUGE_HVR2250_3] = {
		.name		= "Hauppauge WinTV-HVR2250",
		.porta		= SAA7164_MPEG_DVB,
		.portb		= SAA7164_MPEG_DVB,
		.chiprev	= SAA7164_CHIP_REV3,
		.unit		= {{
			.id		= 0x26,
			.type		= SAA7164_UNIT_EEPROM,
			.name		= "4K EEPROM",
			.i2c_bus_nr	= SAA7164_I2C_BUS_0,
			.i2c_bus_addr	= 0xa0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x04,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-1",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x07,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x08,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-1 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_1,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x22,
			.type		= SAA7164_UNIT_TUNER,
			.name		= "TDA18271-2",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0xc0 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x24,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (TOP)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x32 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		}, {
			.id		= 0x27,
			.type		= SAA7164_UNIT_DIGITAL_DEMODULATOR,
			.name		= "CX24228/S5H1411-2 (QAM)",
			.i2c_bus_nr	= SAA7164_I2C_BUS_2,
			.i2c_bus_addr	= 0x34 >> 1,
			.i2c_reg_len	= REGLEN_8bit,
		} },
	},
};
const unsigned int saa7164_bcount = ARRAY_SIZE(saa7164_boards);




struct saa7164_subid saa7164_subids[] = {
	{
		.subvendor = 0x0070,
		.subdevice = 0x8880,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2250,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8810,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2250,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8980,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2200,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8900,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2200_2,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8901,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2200_3,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x88A1,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2250_3,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8891,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2250_2,
	}, {
		.subvendor = 0x0070,
		.subdevice = 0x8851,
		.card      = SAA7164_BOARD_HAUPPAUGE_HVR2250_2,
	},
};
const unsigned int saa7164_idcount = ARRAY_SIZE(saa7164_subids);

void saa7164_card_list(struct saa7164_dev *dev)
{
	int i;

	if (0 == dev->pci->subsystem_vendor &&
	    0 == dev->pci->subsystem_device) {
		printk(KERN_ERR
			"%s: Board has no valid PCIe Subsystem ID and can't\n"
			"%s: be autodetected. Pass card=<n> insmod option to\n"
			"%s: workaround that. Send complaints to the vendor\n"
			"%s: of the TV card. Best regards,\n"
			"%s:         -- tux\n",
			dev->name, dev->name, dev->name, dev->name, dev->name);
	} else {
		printk(KERN_ERR
			"%s: Your board isn't known (yet) to the driver.\n"
			"%s: Try to pick one of the existing card configs via\n"
			"%s: card=<n> insmod option.  Updating to the latest\n"
			"%s: version might help as well.\n",
			dev->name, dev->name, dev->name, dev->name);
	}

	printk(KERN_ERR "%s: Here are valid choices for the card=<n> insmod "
		"option:\n", dev->name);

	for (i = 0; i < saa7164_bcount; i++)
		printk(KERN_ERR "%s:    card=%d -> %s\n",
		       dev->name, i, saa7164_boards[i].name);
}


#define PCIEBRIDGE_UNITID 2

void saa7164_gpio_setup(struct saa7164_dev *dev)
{


	switch (dev->board) {
	case SAA7164_BOARD_HAUPPAUGE_HVR2200:
	case SAA7164_BOARD_HAUPPAUGE_HVR2200_2:
	case SAA7164_BOARD_HAUPPAUGE_HVR2200_3:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250_2:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250_3:
		

		
		saa7164_api_clear_gpiobit(dev, PCIEBRIDGE_UNITID, 2);
		saa7164_api_clear_gpiobit(dev, PCIEBRIDGE_UNITID, 3);

		msleep(10);

		saa7164_api_set_gpiobit(dev, PCIEBRIDGE_UNITID, 2);
		saa7164_api_set_gpiobit(dev, PCIEBRIDGE_UNITID, 3);
		break;
	}

}

static void hauppauge_eeprom(struct saa7164_dev *dev, u8 *eeprom_data)
{
	struct tveeprom tv;

	
	tveeprom_hauppauge_analog(&dev->i2c_bus[0].i2c_client, &tv,
		eeprom_data);

	
	switch (tv.model) {
	case 88001:
		
		
	case 88021:
		
		break;
	case 88041:
		
		break;
	case 88061:
		
		break;
	case 89519:
	case 89609:
		
		break;
	case 89619:
		
		break;
	default:
		printk(KERN_ERR "%s: Warning: Unknown Hauppauge model #%d\n",
			dev->name, tv.model);
		break;
	}

	printk(KERN_INFO "%s: Hauppauge eeprom: model=%d\n", dev->name,
		tv.model);
}

void saa7164_card_setup(struct saa7164_dev *dev)
{
	static u8 eeprom[256];

	if (dev->i2c_bus[0].i2c_rc == 0) {
		if (saa7164_api_read_eeprom(dev, &eeprom[0],
			sizeof(eeprom)) < 0)
			return;
	}

	switch (dev->board) {
	case SAA7164_BOARD_HAUPPAUGE_HVR2200:
	case SAA7164_BOARD_HAUPPAUGE_HVR2200_2:
	case SAA7164_BOARD_HAUPPAUGE_HVR2200_3:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250_2:
	case SAA7164_BOARD_HAUPPAUGE_HVR2250_3:
		hauppauge_eeprom(dev, &eeprom[0]);
		break;
	}
}


int saa7164_i2caddr_to_unitid(struct saa7164_i2c *bus, int addr)
{
	

	struct saa7164_dev *dev = bus->dev;
	struct saa7164_unit *unit;
	int i;

	for (i = 0; i < SAA7164_MAX_UNITS; i++) {
		unit = &saa7164_boards[dev->board].unit[i];

		if (unit->type == SAA7164_UNIT_UNDEFINED)
			continue;
		if ((bus->nr == unit->i2c_bus_nr) &&
			(addr == unit->i2c_bus_addr))
			return unit->id;
	}

	return -1;
}


int saa7164_i2caddr_to_reglen(struct saa7164_i2c *bus, int addr)
{
	

	struct saa7164_dev *dev = bus->dev;
	struct saa7164_unit *unit;
	int i;

	for (i = 0; i < SAA7164_MAX_UNITS; i++) {
		unit = &saa7164_boards[dev->board].unit[i];

		if (unit->type == SAA7164_UNIT_UNDEFINED)
			continue;

		if ((bus->nr == unit->i2c_bus_nr) &&
			(addr == unit->i2c_bus_addr))
			return unit->i2c_reg_len;
	}

	return -1;
}



char *saa7164_unitid_name(struct saa7164_dev *dev, u8 unitid)
{
	char *undefed = "UNDEFINED";
	char *bridge = "BRIDGE";
	struct saa7164_unit *unit;
	int i;

	if (unitid == 0)
		return bridge;

	for (i = 0; i < SAA7164_MAX_UNITS; i++) {
		unit = &saa7164_boards[dev->board].unit[i];

		if (unit->type == SAA7164_UNIT_UNDEFINED)
			continue;

		if (unitid == unit->id)
				return unit->name;
	}

	return undefed;
}

