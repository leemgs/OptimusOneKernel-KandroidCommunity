

#include "cx23885.h"
#include "dvb_ca_en50221.h"


#define NETUP_DATA		0x000000ff
#define NETUP_WR		0x00008000
#define NETUP_RD		0x00004000
#define NETUP_ACK		0x00001000
#define NETUP_ADHI		0x00000800
#define NETUP_ADLO		0x00000400
#define NETUP_CS1		0x00000200
#define NETUP_CS0		0x00000100
#define NETUP_EN_ALL		0x00001000
#define NETUP_CTRL_OFF		(NETUP_CS1 | NETUP_CS0 | NETUP_WR | NETUP_RD)
#define NETUP_CI_CTL		0x04
#define NETUP_CI_RD		1


static unsigned int ci_dbg;
module_param(ci_dbg, int, 0644);
MODULE_PARM_DESC(ci_dbg, "Enable CI debugging");

#define ci_dbg_print(args...) \
	do { \
		if (ci_dbg) \
			printk(KERN_DEBUG args); \
	} while (0)


struct netup_ci_state {
	struct dvb_ca_en50221 ca;
	struct mutex ca_mutex;
	struct i2c_adapter *i2c_adap;
	u8 ci_i2c_addr;
	int status;
	struct work_struct work;
	void *priv;
};


int netup_read_i2c(struct i2c_adapter *i2c_adap, u8 addr, u8 reg,
						u8 *buf, int len)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr	= addr,
			.flags	= 0,
			.buf	= &reg,
			.len	= 1
		}, {
			.addr	= addr,
			.flags	= I2C_M_RD,
			.buf	= buf,
			.len	= len
		}
	};

	ret = i2c_transfer(i2c_adap, msg, 2);

	if (ret != 2) {
		ci_dbg_print("%s: i2c read error, Reg = 0x%02x, Status = %d\n",
						__func__, reg, ret);

		return -1;
	}

	ci_dbg_print("%s: i2c read Addr=0x%04x, Reg = 0x%02x, data = %02x\n",
						__func__, addr, reg, buf[0]);

	return 0;
}

int netup_write_i2c(struct i2c_adapter *i2c_adap, u8 addr, u8 reg,
						u8 *buf, int len)
{
	int ret;
	u8 buffer[len + 1];

	struct i2c_msg msg = {
		.addr	= addr,
		.flags	= 0,
		.buf	= &buffer[0],
		.len	= len + 1
	};

	buffer[0] = reg;
	memcpy(&buffer[1], buf, len);

	ret = i2c_transfer(i2c_adap, &msg, 1);

	if (ret != 1) {
		ci_dbg_print("%s: i2c write error, Reg=[0x%02x], Status=%d\n",
						__func__, reg, ret);
		return -1;
	}

	return 0;
}

int netup_ci_get_mem(struct cx23885_dev *dev)
{
	int mem;
	unsigned long timeout = jiffies + msecs_to_jiffies(1);

	for (;;) {
		mem = cx_read(MC417_RWD);
		if ((mem & NETUP_ACK) == 0)
			break;
		if (time_after(jiffies, timeout))
			break;
		udelay(1);
	}

	cx_set(MC417_RWD, NETUP_CTRL_OFF);

	return mem & 0xff;
}

int netup_ci_op_cam(struct dvb_ca_en50221 *en50221, int slot,
				u8 flag, u8 read, int addr, u8 data)
{
	struct netup_ci_state *state = en50221->data;
	struct cx23885_tsport *port = state->priv;
	struct cx23885_dev *dev = port->dev;

	u8 store;
	int mem;
	int ret;

	if (0 != slot)
		return -EINVAL;

	ret = netup_read_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &store, 1);
	if (ret != 0)
		return ret;

	store &= ~0x0c;
	store |= flag;

	ret = netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &store, 1);
	if (ret != 0)
		return ret;

	mutex_lock(&dev->gpio_lock);

	
	cx_write(MC417_OEN, NETUP_EN_ALL);
	msleep(2);
	cx_write(MC417_RWD, NETUP_CTRL_OFF |
				NETUP_ADLO | (0xff & addr));
	cx_clear(MC417_RWD, NETUP_ADLO);
	cx_write(MC417_RWD, NETUP_CTRL_OFF |
				NETUP_ADHI | (0xff & (addr >> 8)));
	cx_clear(MC417_RWD, NETUP_ADHI);

	if (read) { 
		cx_write(MC417_OEN, NETUP_EN_ALL | NETUP_DATA);
		msleep(2);
	} else 
		cx_write(MC417_RWD, NETUP_CTRL_OFF | data);

	
	cx_clear(MC417_RWD,
			(state->ci_i2c_addr == 0x40) ? NETUP_CS0 : NETUP_CS1);
	
	cx_clear(MC417_RWD, (read) ? NETUP_RD : NETUP_WR);
	mem = netup_ci_get_mem(dev);

	mutex_unlock(&dev->gpio_lock);

	if (!read)
		if (mem < 0)
			return -EREMOTEIO;

	ci_dbg_print("%s: %s: addr=[0x%02x], %s=%x\n", __func__,
			(read) ? "read" : "write", addr,
			(flag == NETUP_CI_CTL) ? "ctl" : "mem",
			(read) ? mem : data);

	if (read)
		return mem;

	return 0;
}

int netup_ci_read_attribute_mem(struct dvb_ca_en50221 *en50221,
						int slot, int addr)
{
	return netup_ci_op_cam(en50221, slot, 0, NETUP_CI_RD, addr, 0);
}

int netup_ci_write_attribute_mem(struct dvb_ca_en50221 *en50221,
						int slot, int addr, u8 data)
{
	return netup_ci_op_cam(en50221, slot, 0, 0, addr, data);
}

int netup_ci_read_cam_ctl(struct dvb_ca_en50221 *en50221, int slot, u8 addr)
{
	return netup_ci_op_cam(en50221, slot, NETUP_CI_CTL,
							NETUP_CI_RD, addr, 0);
}

int netup_ci_write_cam_ctl(struct dvb_ca_en50221 *en50221, int slot,
							u8 addr, u8 data)
{
	return netup_ci_op_cam(en50221, slot, NETUP_CI_CTL, 0, addr, data);
}

int netup_ci_slot_reset(struct dvb_ca_en50221 *en50221, int slot)
{
	struct netup_ci_state *state = en50221->data;
	u8 buf =  0x80;
	int ret;

	if (0 != slot)
		return -EINVAL;

	udelay(500);
	ret = netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &buf, 1);

	if (ret != 0)
		return ret;

	udelay(500);

	buf = 0x00;
	ret = netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &buf, 1);

	msleep(1000);
	dvb_ca_en50221_camready_irq(&state->ca, 0);

	return 0;

}

int netup_ci_slot_shutdown(struct dvb_ca_en50221 *en50221, int slot)
{
	
	return 0;
}

int netup_ci_slot_ts_ctl(struct dvb_ca_en50221 *en50221, int slot)
{
	struct netup_ci_state *state = en50221->data;
	u8 buf = 0x60;

	if (0 != slot)
		return -EINVAL;

	return netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &buf, 1);
}


static void netup_read_ci_status(struct work_struct *work)
{
	struct netup_ci_state *state =
			container_of(work, struct netup_ci_state, work);
	u8 buf[33];
	int ret;

	ret = netup_read_i2c(state->i2c_adap, state->ci_i2c_addr,
							0, &buf[0], 33);

	if (ret != 0)
		return;

	ci_dbg_print("%s: Slot Status Addr=[0x%04x], Reg=[0x%02x], data=%02x, "
		"TS config = %02x\n", __func__, state->ci_i2c_addr, 0, buf[0],
		buf[32]);

	if (buf[0] & 1)
		state->status = DVB_CA_EN50221_POLL_CAM_PRESENT |
			DVB_CA_EN50221_POLL_CAM_READY;
	else
		state->status = 0;
}


int netup_ci_slot_status(struct cx23885_dev *dev, u32 pci_status)
{
	struct cx23885_tsport *port = NULL;
	struct netup_ci_state *state = NULL;

	if (pci_status & PCI_MSK_GPIO0)
		port = &dev->ts1;
	else if (pci_status & PCI_MSK_GPIO1)
		port = &dev->ts2;
	else 
		return 0;

	state = port->port_priv;

	schedule_work(&state->work);

	return 1;
}

int netup_poll_ci_slot_status(struct dvb_ca_en50221 *en50221, int slot, int open)
{
	struct netup_ci_state *state = en50221->data;

	if (0 != slot)
		return -EINVAL;

	return state->status;
}

int netup_ci_init(struct cx23885_tsport *port)
{
	struct netup_ci_state *state;
	u8 cimax_init[34] = {
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x44, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x44, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x00, 
		0x02, 
		0x01, 
		0x00, 
		0x00, 
		0x01, 
		0x04, 
		0x00, 
		0x04, 
		0x00, 
		0x33, 
		0x31, 
	};
	int ret;

	ci_dbg_print("%s\n", __func__);
	state = kzalloc(sizeof(struct netup_ci_state), GFP_KERNEL);
	if (!state) {
		ci_dbg_print("%s: Unable create CI structure!\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	port->port_priv = state;

	switch (port->nr) {
	case 1:
		state->ci_i2c_addr = 0x40;
		break;
	case 2:
		state->ci_i2c_addr = 0x41;
		break;
	}

	state->i2c_adap = &port->dev->i2c_bus[0].i2c_adap;
	state->ca.owner = THIS_MODULE;
	state->ca.read_attribute_mem = netup_ci_read_attribute_mem;
	state->ca.write_attribute_mem = netup_ci_write_attribute_mem;
	state->ca.read_cam_control = netup_ci_read_cam_ctl;
	state->ca.write_cam_control = netup_ci_write_cam_ctl;
	state->ca.slot_reset = netup_ci_slot_reset;
	state->ca.slot_shutdown = netup_ci_slot_shutdown;
	state->ca.slot_ts_enable = netup_ci_slot_ts_ctl;
	state->ca.poll_slot_status = netup_poll_ci_slot_status;
	state->ca.data = state;
	state->priv = port;

	ret = netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
						0, &cimax_init[0], 34);
	
	ret |= netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
						0x1f, &cimax_init[0x18], 1);
	
	ret |= netup_write_i2c(state->i2c_adap, state->ci_i2c_addr,
						0x18, &cimax_init[0x18], 1);

	if (0 != ret)
		goto err;

	ret = dvb_ca_en50221_init(&port->frontends.adapter,
				   &state->ca,
				    0,
				    1);
	if (0 != ret)
		goto err;

	INIT_WORK(&state->work, netup_read_ci_status);
	schedule_work(&state->work);

	ci_dbg_print("%s: CI initialized!\n", __func__);

	return 0;
err:
	ci_dbg_print("%s: Cannot initialize CI: Error %d.\n", __func__, ret);
	kfree(state);
	return ret;
}

void netup_ci_exit(struct cx23885_tsport *port)
{
	struct netup_ci_state *state;

	if (NULL == port)
		return;

	state = (struct netup_ci_state *)port->port_priv;
	if (NULL == state)
		return;

	if (NULL == state->ca.data)
		return;

	dvb_ca_en50221_release(&state->ca);
	kfree(state);
}
