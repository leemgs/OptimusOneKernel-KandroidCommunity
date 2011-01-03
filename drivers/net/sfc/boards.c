

#include "net_driver.h"
#include "phy.h"
#include "boards.h"
#include "efx.h"
#include "workarounds.h"



#define BOARD_TYPE(_rev) (_rev >> 8)
#define BOARD_MAJOR(_rev) ((_rev >> 4) & 0xf)
#define BOARD_MINOR(_rev) (_rev & 0xf)


#define BLINK_INTERVAL (HZ/2)

static void blink_led_timer(unsigned long context)
{
	struct efx_nic *efx = (struct efx_nic *)context;
	struct efx_blinker *bl = &efx->board_info.blinker;
	efx->board_info.set_id_led(efx, bl->state);
	bl->state = !bl->state;
	if (bl->resubmit)
		mod_timer(&bl->timer, jiffies + BLINK_INTERVAL);
}

static void board_blink(struct efx_nic *efx, bool blink)
{
	struct efx_blinker *blinker = &efx->board_info.blinker;

	
	if (blink) {
		blinker->resubmit = true;
		blinker->state = false;
		setup_timer(&blinker->timer, blink_led_timer,
			    (unsigned long)efx);
		mod_timer(&blinker->timer, jiffies + BLINK_INTERVAL);
	} else {
		blinker->resubmit = false;
		if (blinker->timer.function)
			del_timer_sync(&blinker->timer);
		efx->board_info.init_leds(efx);
	}
}


#define LM87_REG_ALARMS1		0x41
#define LM87_REG_ALARMS2		0x42
#define LM87_IN_LIMITS(nr, _min, _max)			\
	0x2B + (nr) * 2, _max, 0x2C + (nr) * 2, _min
#define LM87_AIN_LIMITS(nr, _min, _max)			\
	0x3B + (nr), _max, 0x1A + (nr), _min
#define LM87_TEMP_INT_LIMITS(_min, _max)		\
	0x39, _max, 0x3A, _min
#define LM87_TEMP_EXT1_LIMITS(_min, _max)		\
	0x37, _max, 0x38, _min

#define LM87_ALARM_TEMP_INT		0x10
#define LM87_ALARM_TEMP_EXT1		0x20

#if defined(CONFIG_SENSORS_LM87) || defined(CONFIG_SENSORS_LM87_MODULE)

static int efx_init_lm87(struct efx_nic *efx, struct i2c_board_info *info,
			 const u8 *reg_values)
{
	struct i2c_client *client = i2c_new_device(&efx->i2c_adap, info);
	int rc;

	if (!client)
		return -EIO;

	while (*reg_values) {
		u8 reg = *reg_values++;
		u8 value = *reg_values++;
		rc = i2c_smbus_write_byte_data(client, reg, value);
		if (rc)
			goto err;
	}

	efx->board_info.hwmon_client = client;
	return 0;

err:
	i2c_unregister_device(client);
	return rc;
}

static void efx_fini_lm87(struct efx_nic *efx)
{
	i2c_unregister_device(efx->board_info.hwmon_client);
}

static int efx_check_lm87(struct efx_nic *efx, unsigned mask)
{
	struct i2c_client *client = efx->board_info.hwmon_client;
	s32 alarms1, alarms2;

	
	if (EFX_WORKAROUND_7884(efx) && efx->link_up)
		return 0;

	alarms1 = i2c_smbus_read_byte_data(client, LM87_REG_ALARMS1);
	alarms2 = i2c_smbus_read_byte_data(client, LM87_REG_ALARMS2);
	if (alarms1 < 0)
		return alarms1;
	if (alarms2 < 0)
		return alarms2;
	alarms1 &= mask;
	alarms2 &= mask >> 8;
	if (alarms1 || alarms2) {
		EFX_ERR(efx,
			"LM87 detected a hardware failure (status %02x:%02x)"
			"%s%s\n",
			alarms1, alarms2,
			(alarms1 & LM87_ALARM_TEMP_INT) ? " INTERNAL" : "",
			(alarms1 & LM87_ALARM_TEMP_EXT1) ? " EXTERNAL" : "");
		return -ERANGE;
	}

	return 0;
}

#else 

static inline int
efx_init_lm87(struct efx_nic *efx, struct i2c_board_info *info,
	      const u8 *reg_values)
{
	return 0;
}
static inline void efx_fini_lm87(struct efx_nic *efx)
{
}
static inline int efx_check_lm87(struct efx_nic *efx, unsigned mask)
{
	return 0;
}

#endif 


static u8 sfe4002_lm87_channel = 0x03; 

static const u8 sfe4002_lm87_regs[] = {
	LM87_IN_LIMITS(0, 0x83, 0x91),		
	LM87_IN_LIMITS(1, 0x51, 0x5a),		
	LM87_IN_LIMITS(2, 0xb6, 0xca),		
	LM87_IN_LIMITS(3, 0xb0, 0xc9),		
	LM87_IN_LIMITS(4, 0xb0, 0xe0),		
	LM87_IN_LIMITS(5, 0x44, 0x4b),		
	LM87_AIN_LIMITS(0, 0xa0, 0xb2),		
	LM87_AIN_LIMITS(1, 0x91, 0xa1),		
	LM87_TEMP_INT_LIMITS(10, 60),		
	LM87_TEMP_EXT1_LIMITS(10, 70),		
	0
};

static struct i2c_board_info sfe4002_hwmon_info = {
	I2C_BOARD_INFO("lm87", 0x2e),
	.platform_data	= &sfe4002_lm87_channel,
};



#define SFE4002_FAULT_LED (2)	
#define SFE4002_RX_LED    (0)	
#define SFE4002_TX_LED    (1)	

static void sfe4002_init_leds(struct efx_nic *efx)
{
	
	xfp_set_led(efx, SFE4002_TX_LED,
		    QUAKE_LED_TXLINK | QUAKE_LED_LINK_ACTSTAT);
	xfp_set_led(efx, SFE4002_RX_LED,
		    QUAKE_LED_RXLINK | QUAKE_LED_LINK_ACTSTAT);
	xfp_set_led(efx, SFE4002_FAULT_LED, QUAKE_LED_OFF);
}

static void sfe4002_set_id_led(struct efx_nic *efx, bool state)
{
	xfp_set_led(efx, SFE4002_FAULT_LED, state ? QUAKE_LED_ON :
			QUAKE_LED_OFF);
}

static int sfe4002_check_hw(struct efx_nic *efx)
{
	
	unsigned alarm_mask =
		(efx->board_info.major == 0 && efx->board_info.minor == 0) ?
		~LM87_ALARM_TEMP_EXT1 : ~0;

	return efx_check_lm87(efx, alarm_mask);
}

static int sfe4002_init(struct efx_nic *efx)
{
	int rc = efx_init_lm87(efx, &sfe4002_hwmon_info, sfe4002_lm87_regs);
	if (rc)
		return rc;
	efx->board_info.monitor = sfe4002_check_hw;
	efx->board_info.init_leds = sfe4002_init_leds;
	efx->board_info.set_id_led = sfe4002_set_id_led;
	efx->board_info.blink = board_blink;
	efx->board_info.fini = efx_fini_lm87;
	return 0;
}


static u8 sfn4112f_lm87_channel = 0x03; 

static const u8 sfn4112f_lm87_regs[] = {
	LM87_IN_LIMITS(0, 0x83, 0x91),		
	LM87_IN_LIMITS(1, 0x51, 0x5a),		
	LM87_IN_LIMITS(2, 0xb6, 0xca),		
	LM87_IN_LIMITS(4, 0xb0, 0xe0),		
	LM87_IN_LIMITS(5, 0x44, 0x4b),		
	LM87_AIN_LIMITS(1, 0x91, 0xa1),		
	LM87_TEMP_INT_LIMITS(10, 60),		
	LM87_TEMP_EXT1_LIMITS(10, 70),		
	0
};

static struct i2c_board_info sfn4112f_hwmon_info = {
	I2C_BOARD_INFO("lm87", 0x2e),
	.platform_data	= &sfn4112f_lm87_channel,
};

#define SFN4112F_ACT_LED	0
#define SFN4112F_LINK_LED	1

static void sfn4112f_init_leds(struct efx_nic *efx)
{
	xfp_set_led(efx, SFN4112F_ACT_LED,
		    QUAKE_LED_RXLINK | QUAKE_LED_LINK_ACT);
	xfp_set_led(efx, SFN4112F_LINK_LED,
		    QUAKE_LED_RXLINK | QUAKE_LED_LINK_STAT);
}

static void sfn4112f_set_id_led(struct efx_nic *efx, bool state)
{
	xfp_set_led(efx, SFN4112F_LINK_LED,
		    state ? QUAKE_LED_ON : QUAKE_LED_OFF);
}

static int sfn4112f_check_hw(struct efx_nic *efx)
{
	
	return efx_check_lm87(efx, ~0x48);
}

static int sfn4112f_init(struct efx_nic *efx)
{
	int rc = efx_init_lm87(efx, &sfn4112f_hwmon_info, sfn4112f_lm87_regs);
	if (rc)
		return rc;
	efx->board_info.monitor = sfn4112f_check_hw;
	efx->board_info.init_leds = sfn4112f_init_leds;
	efx->board_info.set_id_led = sfn4112f_set_id_led;
	efx->board_info.blink = board_blink;
	efx->board_info.fini = efx_fini_lm87;
	return 0;
}


struct efx_board_data {
	enum efx_board_type type;
	const char *ref_model;
	const char *gen_type;
	int (*init) (struct efx_nic *nic);
};


static struct efx_board_data board_data[] = {
	{ EFX_BOARD_SFE4001, "SFE4001", "10GBASE-T adapter", sfe4001_init },
	{ EFX_BOARD_SFE4002, "SFE4002", "XFP adapter", sfe4002_init },
	{ EFX_BOARD_SFN4111T, "SFN4111T", "100/1000/10GBASE-T adapter",
	  sfn4111t_init },
	{ EFX_BOARD_SFN4112F, "SFN4112F", "SFP+ adapter",
	  sfn4112f_init },
};

void efx_set_board_info(struct efx_nic *efx, u16 revision_info)
{
	struct efx_board_data *data = NULL;
	int i;

	efx->board_info.type = BOARD_TYPE(revision_info);
	efx->board_info.major = BOARD_MAJOR(revision_info);
	efx->board_info.minor = BOARD_MINOR(revision_info);

	for (i = 0; i < ARRAY_SIZE(board_data); i++)
		if (board_data[i].type == efx->board_info.type)
			data = &board_data[i];

	if (data) {
		EFX_INFO(efx, "board is %s rev %c%d\n",
			 (efx->pci_dev->subsystem_vendor == EFX_VENDID_SFC)
			 ? data->ref_model : data->gen_type,
			 'A' + efx->board_info.major, efx->board_info.minor);
		efx->board_info.init = data->init;
	} else {
		EFX_ERR(efx, "unknown board type %d\n", efx->board_info.type);
	}
}
