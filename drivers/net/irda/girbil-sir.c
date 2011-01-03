

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>

#include <net/irda/irda.h>

#include "sir-dev.h"

static int girbil_reset(struct sir_dev *dev);
static int girbil_open(struct sir_dev *dev);
static int girbil_close(struct sir_dev *dev);
static int girbil_change_speed(struct sir_dev *dev, unsigned speed);


#define GIRBIL_TXEN    0x01 
#define GIRBIL_RXEN    0x02 
#define GIRBIL_ECAN    0x04 
#define GIRBIL_ECHO    0x08 


#define GIRBIL_HIGH    0x20
#define GIRBIL_MEDIUM  0x21
#define GIRBIL_LOW     0x22


#define GIRBIL_2400    0x30
#define GIRBIL_4800    0x31
#define GIRBIL_9600    0x32
#define GIRBIL_19200   0x33
#define GIRBIL_38400   0x34
#define GIRBIL_57600   0x35
#define GIRBIL_115200  0x36


#define GIRBIL_IRDA    0x40
#define GIRBIL_ASK     0x41


#define GIRBIL_LOAD    0x51 

static struct dongle_driver girbil = {
	.owner		= THIS_MODULE,
	.driver_name	= "Greenwich GIrBIL",
	.type		= IRDA_GIRBIL_DONGLE,
	.open		= girbil_open,
	.close		= girbil_close,
	.reset		= girbil_reset,
	.set_speed	= girbil_change_speed,
};

static int __init girbil_sir_init(void)
{
	return irda_register_dongle(&girbil);
}

static void __exit girbil_sir_cleanup(void)
{
	irda_unregister_dongle(&girbil);
}

static int girbil_open(struct sir_dev *dev)
{
	struct qos_info *qos = &dev->qos;

	IRDA_DEBUG(2, "%s()\n", __func__);

	
	sirdev_set_dtr_rts(dev, TRUE, TRUE);

	qos->baud_rate.bits &= IR_9600|IR_19200|IR_38400|IR_57600|IR_115200;
	qos->min_turn_time.bits = 0x03;
	irda_qos_bits_to_value(qos);

	

	return 0;
}

static int girbil_close(struct sir_dev *dev)
{
	IRDA_DEBUG(2, "%s()\n", __func__);

	
	sirdev_set_dtr_rts(dev, FALSE, FALSE);

	return 0;
}



#define GIRBIL_STATE_WAIT_SPEED	(SIRDEV_STATE_DONGLE_SPEED + 1)

static int girbil_change_speed(struct sir_dev *dev, unsigned speed)
{
	unsigned state = dev->fsm.substate;
	unsigned delay = 0;
	u8 control[2];
	static int ret = 0;

	IRDA_DEBUG(2, "%s()\n", __func__);

	

	switch(state) {

	case SIRDEV_STATE_DONGLE_SPEED:

		
		sirdev_set_dtr_rts(dev, FALSE, TRUE);

		udelay(25);		

		ret = 0;
		switch (speed) {
		default:
			ret = -EINVAL;
			
		case 9600:
			control[0] = GIRBIL_9600;
			break;
		case 19200:
			control[0] = GIRBIL_19200;
			break;
		case 34800:
			control[0] = GIRBIL_38400;
			break;
		case 57600:
			control[0] = GIRBIL_57600;
			break;
		case 115200:
			control[0] = GIRBIL_115200;
			break;
		}
		control[1] = GIRBIL_LOAD;
	
		
		sirdev_raw_write(dev, control, 2);

		dev->speed = speed;

		state = GIRBIL_STATE_WAIT_SPEED;
		delay = 100;
		break;

	case GIRBIL_STATE_WAIT_SPEED:
		
		sirdev_set_dtr_rts(dev, TRUE, TRUE);

		udelay(25);		
		break;

	default:
		IRDA_ERROR("%s - undefined state %d\n", __func__, state);
		ret = -EINVAL;
		break;
	}
	dev->fsm.substate = state;
	return (delay > 0) ? delay : ret;
}




#define GIRBIL_STATE_WAIT1_RESET	(SIRDEV_STATE_DONGLE_RESET + 1)
#define GIRBIL_STATE_WAIT2_RESET	(SIRDEV_STATE_DONGLE_RESET + 2)
#define GIRBIL_STATE_WAIT3_RESET	(SIRDEV_STATE_DONGLE_RESET + 3)

static int girbil_reset(struct sir_dev *dev)
{
	unsigned state = dev->fsm.substate;
	unsigned delay = 0;
	u8 control = GIRBIL_TXEN | GIRBIL_RXEN;
	int ret = 0;

	IRDA_DEBUG(2, "%s()\n", __func__);

	switch (state) {
	case SIRDEV_STATE_DONGLE_RESET:
		
		sirdev_set_dtr_rts(dev, TRUE, FALSE);
		
		delay = 20;
		state = GIRBIL_STATE_WAIT1_RESET;
		break;

	case GIRBIL_STATE_WAIT1_RESET:
		
		sirdev_set_dtr_rts(dev, FALSE, TRUE);
		delay = 20;
		state = GIRBIL_STATE_WAIT2_RESET;
		break;

	case GIRBIL_STATE_WAIT2_RESET:
		
		sirdev_raw_write(dev, &control, 1);
		delay = 20;
		state = GIRBIL_STATE_WAIT3_RESET;
		break;

	case GIRBIL_STATE_WAIT3_RESET:
		
		sirdev_set_dtr_rts(dev, TRUE, TRUE);
		dev->speed = 9600;
		break;

	default:
		IRDA_ERROR("%s(), undefined state %d\n", __func__, state);
		ret = -1;
		break;
	}
	dev->fsm.substate = state;
	return (delay > 0) ? delay : ret;
}

MODULE_AUTHOR("Dag Brattli <dagb@cs.uit.no>");
MODULE_DESCRIPTION("Greenwich GIrBIL dongle driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("irda-dongle-4"); 

module_init(girbil_sir_init);
module_exit(girbil_sir_cleanup);
