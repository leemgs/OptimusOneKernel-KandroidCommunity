





#undef DEBUG

#include <linux/device.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#include <asm/mpc52xx.h>
#include <asm/mpc52xx_psc.h>

#if defined(CONFIG_SERIAL_MPC52xx_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/serial_core.h>



#define SERIAL_PSC_MAJOR	204
#define SERIAL_PSC_MINOR	148


#define ISR_PASS_LIMIT 256	


static struct uart_port mpc52xx_uart_ports[MPC52xx_PSC_MAXNUM];
	


static struct device_node *mpc52xx_uart_nodes[MPC52xx_PSC_MAXNUM];

static void mpc52xx_uart_of_enumerate(void);


#define PSC(port) ((struct mpc52xx_psc __iomem *)((port)->membase))



static irqreturn_t mpc52xx_uart_int(int irq, void *dev_id);



#ifdef CONFIG_SERIAL_CORE_CONSOLE
#define uart_console(port) \
	((port)->cons && (port)->cons->index == (port)->line)
#else
#define uart_console(port)	(0)
#endif





struct psc_ops {
	void		(*fifo_init)(struct uart_port *port);
	int		(*raw_rx_rdy)(struct uart_port *port);
	int		(*raw_tx_rdy)(struct uart_port *port);
	int		(*rx_rdy)(struct uart_port *port);
	int		(*tx_rdy)(struct uart_port *port);
	int		(*tx_empty)(struct uart_port *port);
	void		(*stop_rx)(struct uart_port *port);
	void		(*start_tx)(struct uart_port *port);
	void		(*stop_tx)(struct uart_port *port);
	void		(*rx_clr_irq)(struct uart_port *port);
	void		(*tx_clr_irq)(struct uart_port *port);
	void		(*write_char)(struct uart_port *port, unsigned char c);
	unsigned char	(*read_char)(struct uart_port *port);
	void		(*cw_disable_ints)(struct uart_port *port);
	void		(*cw_restore_ints)(struct uart_port *port);
	unsigned long	(*getuartclk)(void *p);
};

#ifdef CONFIG_PPC_MPC52xx
#define FIFO_52xx(port) ((struct mpc52xx_psc_fifo __iomem *)(PSC(port)+1))
static void mpc52xx_psc_fifo_init(struct uart_port *port)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);
	struct mpc52xx_psc_fifo __iomem *fifo = FIFO_52xx(port);

	
	out_be16(&psc->mpc52xx_psc_clock_select, 0xdd00);

	out_8(&fifo->rfcntl, 0x00);
	out_be16(&fifo->rfalarm, 0x1ff);
	out_8(&fifo->tfcntl, 0x07);
	out_be16(&fifo->tfalarm, 0x80);

	port->read_status_mask |= MPC52xx_PSC_IMR_RXRDY | MPC52xx_PSC_IMR_TXRDY;
	out_be16(&psc->mpc52xx_psc_imr, port->read_status_mask);
}

static int mpc52xx_psc_raw_rx_rdy(struct uart_port *port)
{
	return in_be16(&PSC(port)->mpc52xx_psc_status)
	    & MPC52xx_PSC_SR_RXRDY;
}

static int mpc52xx_psc_raw_tx_rdy(struct uart_port *port)
{
	return in_be16(&PSC(port)->mpc52xx_psc_status)
	    & MPC52xx_PSC_SR_TXRDY;
}


static int mpc52xx_psc_rx_rdy(struct uart_port *port)
{
	return in_be16(&PSC(port)->mpc52xx_psc_isr)
	    & port->read_status_mask
	    & MPC52xx_PSC_IMR_RXRDY;
}

static int mpc52xx_psc_tx_rdy(struct uart_port *port)
{
	return in_be16(&PSC(port)->mpc52xx_psc_isr)
	    & port->read_status_mask
	    & MPC52xx_PSC_IMR_TXRDY;
}

static int mpc52xx_psc_tx_empty(struct uart_port *port)
{
	return in_be16(&PSC(port)->mpc52xx_psc_status)
	    & MPC52xx_PSC_SR_TXEMP;
}

static void mpc52xx_psc_start_tx(struct uart_port *port)
{
	port->read_status_mask |= MPC52xx_PSC_IMR_TXRDY;
	out_be16(&PSC(port)->mpc52xx_psc_imr, port->read_status_mask);
}

static void mpc52xx_psc_stop_tx(struct uart_port *port)
{
	port->read_status_mask &= ~MPC52xx_PSC_IMR_TXRDY;
	out_be16(&PSC(port)->mpc52xx_psc_imr, port->read_status_mask);
}

static void mpc52xx_psc_stop_rx(struct uart_port *port)
{
	port->read_status_mask &= ~MPC52xx_PSC_IMR_RXRDY;
	out_be16(&PSC(port)->mpc52xx_psc_imr, port->read_status_mask);
}

static void mpc52xx_psc_rx_clr_irq(struct uart_port *port)
{
}

static void mpc52xx_psc_tx_clr_irq(struct uart_port *port)
{
}

static void mpc52xx_psc_write_char(struct uart_port *port, unsigned char c)
{
	out_8(&PSC(port)->mpc52xx_psc_buffer_8, c);
}

static unsigned char mpc52xx_psc_read_char(struct uart_port *port)
{
	return in_8(&PSC(port)->mpc52xx_psc_buffer_8);
}

static void mpc52xx_psc_cw_disable_ints(struct uart_port *port)
{
	out_be16(&PSC(port)->mpc52xx_psc_imr, 0);
}

static void mpc52xx_psc_cw_restore_ints(struct uart_port *port)
{
	out_be16(&PSC(port)->mpc52xx_psc_imr, port->read_status_mask);
}


static unsigned long mpc52xx_getuartclk(void *p)
{
	
	return mpc5xxx_get_bus_frequency(p) / 2;
}

static struct psc_ops mpc52xx_psc_ops = {
	.fifo_init = mpc52xx_psc_fifo_init,
	.raw_rx_rdy = mpc52xx_psc_raw_rx_rdy,
	.raw_tx_rdy = mpc52xx_psc_raw_tx_rdy,
	.rx_rdy = mpc52xx_psc_rx_rdy,
	.tx_rdy = mpc52xx_psc_tx_rdy,
	.tx_empty = mpc52xx_psc_tx_empty,
	.stop_rx = mpc52xx_psc_stop_rx,
	.start_tx = mpc52xx_psc_start_tx,
	.stop_tx = mpc52xx_psc_stop_tx,
	.rx_clr_irq = mpc52xx_psc_rx_clr_irq,
	.tx_clr_irq = mpc52xx_psc_tx_clr_irq,
	.write_char = mpc52xx_psc_write_char,
	.read_char = mpc52xx_psc_read_char,
	.cw_disable_ints = mpc52xx_psc_cw_disable_ints,
	.cw_restore_ints = mpc52xx_psc_cw_restore_ints,
	.getuartclk = mpc52xx_getuartclk,
};

#endif 

#ifdef CONFIG_PPC_MPC512x
#define FIFO_512x(port) ((struct mpc512x_psc_fifo __iomem *)(PSC(port)+1))
static void mpc512x_psc_fifo_init(struct uart_port *port)
{
	out_be32(&FIFO_512x(port)->txcmd, MPC512x_PSC_FIFO_RESET_SLICE);
	out_be32(&FIFO_512x(port)->txcmd, MPC512x_PSC_FIFO_ENABLE_SLICE);
	out_be32(&FIFO_512x(port)->txalarm, 1);
	out_be32(&FIFO_512x(port)->tximr, 0);

	out_be32(&FIFO_512x(port)->rxcmd, MPC512x_PSC_FIFO_RESET_SLICE);
	out_be32(&FIFO_512x(port)->rxcmd, MPC512x_PSC_FIFO_ENABLE_SLICE);
	out_be32(&FIFO_512x(port)->rxalarm, 1);
	out_be32(&FIFO_512x(port)->rximr, 0);

	out_be32(&FIFO_512x(port)->tximr, MPC512x_PSC_FIFO_ALARM);
	out_be32(&FIFO_512x(port)->rximr, MPC512x_PSC_FIFO_ALARM);
}

static int mpc512x_psc_raw_rx_rdy(struct uart_port *port)
{
	return !(in_be32(&FIFO_512x(port)->rxsr) & MPC512x_PSC_FIFO_EMPTY);
}

static int mpc512x_psc_raw_tx_rdy(struct uart_port *port)
{
	return !(in_be32(&FIFO_512x(port)->txsr) & MPC512x_PSC_FIFO_FULL);
}

static int mpc512x_psc_rx_rdy(struct uart_port *port)
{
	return in_be32(&FIFO_512x(port)->rxsr)
	    & in_be32(&FIFO_512x(port)->rximr)
	    & MPC512x_PSC_FIFO_ALARM;
}

static int mpc512x_psc_tx_rdy(struct uart_port *port)
{
	return in_be32(&FIFO_512x(port)->txsr)
	    & in_be32(&FIFO_512x(port)->tximr)
	    & MPC512x_PSC_FIFO_ALARM;
}

static int mpc512x_psc_tx_empty(struct uart_port *port)
{
	return in_be32(&FIFO_512x(port)->txsr)
	    & MPC512x_PSC_FIFO_EMPTY;
}

static void mpc512x_psc_stop_rx(struct uart_port *port)
{
	unsigned long rx_fifo_imr;

	rx_fifo_imr = in_be32(&FIFO_512x(port)->rximr);
	rx_fifo_imr &= ~MPC512x_PSC_FIFO_ALARM;
	out_be32(&FIFO_512x(port)->rximr, rx_fifo_imr);
}

static void mpc512x_psc_start_tx(struct uart_port *port)
{
	unsigned long tx_fifo_imr;

	tx_fifo_imr = in_be32(&FIFO_512x(port)->tximr);
	tx_fifo_imr |= MPC512x_PSC_FIFO_ALARM;
	out_be32(&FIFO_512x(port)->tximr, tx_fifo_imr);
}

static void mpc512x_psc_stop_tx(struct uart_port *port)
{
	unsigned long tx_fifo_imr;

	tx_fifo_imr = in_be32(&FIFO_512x(port)->tximr);
	tx_fifo_imr &= ~MPC512x_PSC_FIFO_ALARM;
	out_be32(&FIFO_512x(port)->tximr, tx_fifo_imr);
}

static void mpc512x_psc_rx_clr_irq(struct uart_port *port)
{
	out_be32(&FIFO_512x(port)->rxisr, in_be32(&FIFO_512x(port)->rxisr));
}

static void mpc512x_psc_tx_clr_irq(struct uart_port *port)
{
	out_be32(&FIFO_512x(port)->txisr, in_be32(&FIFO_512x(port)->txisr));
}

static void mpc512x_psc_write_char(struct uart_port *port, unsigned char c)
{
	out_8(&FIFO_512x(port)->txdata_8, c);
}

static unsigned char mpc512x_psc_read_char(struct uart_port *port)
{
	return in_8(&FIFO_512x(port)->rxdata_8);
}

static void mpc512x_psc_cw_disable_ints(struct uart_port *port)
{
	port->read_status_mask =
		in_be32(&FIFO_512x(port)->tximr) << 16 |
		in_be32(&FIFO_512x(port)->rximr);
	out_be32(&FIFO_512x(port)->tximr, 0);
	out_be32(&FIFO_512x(port)->rximr, 0);
}

static void mpc512x_psc_cw_restore_ints(struct uart_port *port)
{
	out_be32(&FIFO_512x(port)->tximr,
		(port->read_status_mask >> 16) & 0x7f);
	out_be32(&FIFO_512x(port)->rximr, port->read_status_mask & 0x7f);
}

static unsigned long mpc512x_getuartclk(void *p)
{
	return mpc5xxx_get_bus_frequency(p);
}

static struct psc_ops mpc512x_psc_ops = {
	.fifo_init = mpc512x_psc_fifo_init,
	.raw_rx_rdy = mpc512x_psc_raw_rx_rdy,
	.raw_tx_rdy = mpc512x_psc_raw_tx_rdy,
	.rx_rdy = mpc512x_psc_rx_rdy,
	.tx_rdy = mpc512x_psc_tx_rdy,
	.tx_empty = mpc512x_psc_tx_empty,
	.stop_rx = mpc512x_psc_stop_rx,
	.start_tx = mpc512x_psc_start_tx,
	.stop_tx = mpc512x_psc_stop_tx,
	.rx_clr_irq = mpc512x_psc_rx_clr_irq,
	.tx_clr_irq = mpc512x_psc_tx_clr_irq,
	.write_char = mpc512x_psc_write_char,
	.read_char = mpc512x_psc_read_char,
	.cw_disable_ints = mpc512x_psc_cw_disable_ints,
	.cw_restore_ints = mpc512x_psc_cw_restore_ints,
	.getuartclk = mpc512x_getuartclk,
};
#endif

static struct psc_ops *psc_ops;





static unsigned int
mpc52xx_uart_tx_empty(struct uart_port *port)
{
	return psc_ops->tx_empty(port) ? TIOCSER_TEMT : 0;
}

static void
mpc52xx_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	if (mctrl & TIOCM_RTS)
		out_8(&PSC(port)->op1, MPC52xx_PSC_OP_RTS);
	else
		out_8(&PSC(port)->op0, MPC52xx_PSC_OP_RTS);
}

static unsigned int
mpc52xx_uart_get_mctrl(struct uart_port *port)
{
	unsigned int ret = TIOCM_DSR;
	u8 status = in_8(&PSC(port)->mpc52xx_psc_ipcr);

	if (!(status & MPC52xx_PSC_CTS))
		ret |= TIOCM_CTS;
	if (!(status & MPC52xx_PSC_DCD))
		ret |= TIOCM_CAR;

	return ret;
}

static void
mpc52xx_uart_stop_tx(struct uart_port *port)
{
	
	psc_ops->stop_tx(port);
}

static void
mpc52xx_uart_start_tx(struct uart_port *port)
{
	
	psc_ops->start_tx(port);
}

static void
mpc52xx_uart_send_xchar(struct uart_port *port, char ch)
{
	unsigned long flags;
	spin_lock_irqsave(&port->lock, flags);

	port->x_char = ch;
	if (ch) {
		
		
		psc_ops->start_tx(port);
	}

	spin_unlock_irqrestore(&port->lock, flags);
}

static void
mpc52xx_uart_stop_rx(struct uart_port *port)
{
	
	psc_ops->stop_rx(port);
}

static void
mpc52xx_uart_enable_ms(struct uart_port *port)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);

	
	in_8(&psc->mpc52xx_psc_ipcr);
	
	out_8(&psc->mpc52xx_psc_acr, MPC52xx_PSC_IEC_CTS | MPC52xx_PSC_IEC_DCD);

	port->read_status_mask |= MPC52xx_PSC_IMR_IPC;
	out_be16(&psc->mpc52xx_psc_imr, port->read_status_mask);
}

static void
mpc52xx_uart_break_ctl(struct uart_port *port, int ctl)
{
	unsigned long flags;
	spin_lock_irqsave(&port->lock, flags);

	if (ctl == -1)
		out_8(&PSC(port)->command, MPC52xx_PSC_START_BRK);
	else
		out_8(&PSC(port)->command, MPC52xx_PSC_STOP_BRK);

	spin_unlock_irqrestore(&port->lock, flags);
}

static int
mpc52xx_uart_startup(struct uart_port *port)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);
	int ret;

	
	ret = request_irq(port->irq, mpc52xx_uart_int,
		IRQF_DISABLED | IRQF_SAMPLE_RANDOM,
		"mpc52xx_psc_uart", port);
	if (ret)
		return ret;

	
	out_8(&psc->command, MPC52xx_PSC_RST_RX);
	out_8(&psc->command, MPC52xx_PSC_RST_TX);

	out_be32(&psc->sicr, 0);	

	psc_ops->fifo_init(port);

	out_8(&psc->command, MPC52xx_PSC_TX_ENABLE);
	out_8(&psc->command, MPC52xx_PSC_RX_ENABLE);

	return 0;
}

static void
mpc52xx_uart_shutdown(struct uart_port *port)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);

	
	out_8(&psc->command, MPC52xx_PSC_RST_RX);
	if (!uart_console(port))
		out_8(&psc->command, MPC52xx_PSC_RST_TX);

	port->read_status_mask = 0;
	out_be16(&psc->mpc52xx_psc_imr, port->read_status_mask);

	
	free_irq(port->irq, port);
}

static void
mpc52xx_uart_set_termios(struct uart_port *port, struct ktermios *new,
			 struct ktermios *old)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);
	unsigned long flags;
	unsigned char mr1, mr2;
	unsigned short ctr;
	unsigned int j, baud, quot;

	
	mr1 = 0;

	switch (new->c_cflag & CSIZE) {
	case CS5:	mr1 |= MPC52xx_PSC_MODE_5_BITS;
		break;
	case CS6:	mr1 |= MPC52xx_PSC_MODE_6_BITS;
		break;
	case CS7:	mr1 |= MPC52xx_PSC_MODE_7_BITS;
		break;
	case CS8:
	default:	mr1 |= MPC52xx_PSC_MODE_8_BITS;
	}

	if (new->c_cflag & PARENB) {
		mr1 |= (new->c_cflag & PARODD) ?
			MPC52xx_PSC_MODE_PARODD : MPC52xx_PSC_MODE_PAREVEN;
	} else
		mr1 |= MPC52xx_PSC_MODE_PARNONE;


	mr2 = 0;

	if (new->c_cflag & CSTOPB)
		mr2 |= MPC52xx_PSC_MODE_TWO_STOP;
	else
		mr2 |= ((new->c_cflag & CSIZE) == CS5) ?
			MPC52xx_PSC_MODE_ONE_STOP_5_BITS :
			MPC52xx_PSC_MODE_ONE_STOP;

	if (new->c_cflag & CRTSCTS) {
		mr1 |= MPC52xx_PSC_MODE_RXRTS;
		mr2 |= MPC52xx_PSC_MODE_TXCTS;
	}

	baud = uart_get_baud_rate(port, new, old, 0, port->uartclk/16);
	quot = uart_get_divisor(port, baud);
	ctr = quot & 0xffff;

	
	spin_lock_irqsave(&port->lock, flags);

	
	uart_update_timeout(port, new->c_cflag, baud);

	
	
	j = 5000000;	
	
	
	while (!mpc52xx_uart_tx_empty(port) && --j)
		udelay(1);

	if (!j)
		printk(KERN_ERR "mpc52xx_uart.c: "
			"Unable to flush RX & TX fifos in-time in set_termios."
			"Some chars may have been lost.\n");

	
	out_8(&psc->command, MPC52xx_PSC_RST_RX);
	out_8(&psc->command, MPC52xx_PSC_RST_TX);

	
	out_8(&psc->command, MPC52xx_PSC_SEL_MODE_REG_1);
	out_8(&psc->mode, mr1);
	out_8(&psc->mode, mr2);
	out_8(&psc->ctur, ctr >> 8);
	out_8(&psc->ctlr, ctr & 0xff);

	if (UART_ENABLE_MS(port, new->c_cflag))
		mpc52xx_uart_enable_ms(port);

	
	out_8(&psc->command, MPC52xx_PSC_TX_ENABLE);
	out_8(&psc->command, MPC52xx_PSC_RX_ENABLE);

	
	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *
mpc52xx_uart_type(struct uart_port *port)
{
	return port->type == PORT_MPC52xx ? "MPC52xx PSC" : NULL;
}

static void
mpc52xx_uart_release_port(struct uart_port *port)
{
	
	if (port->flags & UPF_IOREMAP) {
		iounmap(port->membase);
		port->membase = NULL;
	}

	release_mem_region(port->mapbase, sizeof(struct mpc52xx_psc));
}

static int
mpc52xx_uart_request_port(struct uart_port *port)
{
	int err;

	if (port->flags & UPF_IOREMAP) 
		port->membase = ioremap(port->mapbase,
					sizeof(struct mpc52xx_psc));

	if (!port->membase)
		return -EINVAL;

	err = request_mem_region(port->mapbase, sizeof(struct mpc52xx_psc),
			"mpc52xx_psc_uart") != NULL ? 0 : -EBUSY;

	if (err && (port->flags & UPF_IOREMAP)) {
		iounmap(port->membase);
		port->membase = NULL;
	}

	return err;
}

static void
mpc52xx_uart_config_port(struct uart_port *port, int flags)
{
	if ((flags & UART_CONFIG_TYPE)
		&& (mpc52xx_uart_request_port(port) == 0))
		port->type = PORT_MPC52xx;
}

static int
mpc52xx_uart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if (ser->type != PORT_UNKNOWN && ser->type != PORT_MPC52xx)
		return -EINVAL;

	if ((ser->irq != port->irq) ||
	    (ser->io_type != UPIO_MEM) ||
	    (ser->baud_base != port->uartclk)  ||
	    (ser->iomem_base != (void *)port->mapbase) ||
	    (ser->hub6 != 0))
		return -EINVAL;

	return 0;
}


static struct uart_ops mpc52xx_uart_ops = {
	.tx_empty	= mpc52xx_uart_tx_empty,
	.set_mctrl	= mpc52xx_uart_set_mctrl,
	.get_mctrl	= mpc52xx_uart_get_mctrl,
	.stop_tx	= mpc52xx_uart_stop_tx,
	.start_tx	= mpc52xx_uart_start_tx,
	.send_xchar	= mpc52xx_uart_send_xchar,
	.stop_rx	= mpc52xx_uart_stop_rx,
	.enable_ms	= mpc52xx_uart_enable_ms,
	.break_ctl	= mpc52xx_uart_break_ctl,
	.startup	= mpc52xx_uart_startup,
	.shutdown	= mpc52xx_uart_shutdown,
	.set_termios	= mpc52xx_uart_set_termios,


	.type		= mpc52xx_uart_type,
	.release_port	= mpc52xx_uart_release_port,
	.request_port	= mpc52xx_uart_request_port,
	.config_port	= mpc52xx_uart_config_port,
	.verify_port	= mpc52xx_uart_verify_port
};






static inline int
mpc52xx_uart_int_rx_chars(struct uart_port *port)
{
	struct tty_struct *tty = port->state->port.tty;
	unsigned char ch, flag;
	unsigned short status;

	
	while (psc_ops->raw_rx_rdy(port)) {
		
		ch = psc_ops->read_char(port);

		
#ifdef SUPPORT_SYSRQ
		if (uart_handle_sysrq_char(port, ch)) {
			port->sysrq = 0;
			continue;
		}
#endif

		

		flag = TTY_NORMAL;
		port->icount.rx++;

		status = in_be16(&PSC(port)->mpc52xx_psc_status);

		if (status & (MPC52xx_PSC_SR_PE |
			      MPC52xx_PSC_SR_FE |
			      MPC52xx_PSC_SR_RB)) {

			if (status & MPC52xx_PSC_SR_RB) {
				flag = TTY_BREAK;
				uart_handle_break(port);
				port->icount.brk++;
			} else if (status & MPC52xx_PSC_SR_PE) {
				flag = TTY_PARITY;
				port->icount.parity++;
			}
			else if (status & MPC52xx_PSC_SR_FE) {
				flag = TTY_FRAME;
				port->icount.frame++;
			}

			
			out_8(&PSC(port)->command, MPC52xx_PSC_RST_ERR_STAT);

		}
		tty_insert_flip_char(tty, ch, flag);
		if (status & MPC52xx_PSC_SR_OE) {
			
			tty_insert_flip_char(tty, 0, TTY_OVERRUN);
			port->icount.overrun++;
		}
	}

	spin_unlock(&port->lock);
	tty_flip_buffer_push(tty);
	spin_lock(&port->lock);

	return psc_ops->raw_rx_rdy(port);
}

static inline int
mpc52xx_uart_int_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;

	
	if (port->x_char) {
		psc_ops->write_char(port, port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return 1;
	}

	
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		mpc52xx_uart_stop_tx(port);
		return 0;
	}

	
	while (psc_ops->raw_tx_rdy(port)) {
		psc_ops->write_char(port, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	}

	
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	
	if (uart_circ_empty(xmit)) {
		mpc52xx_uart_stop_tx(port);
		return 0;
	}

	return 1;
}

static irqreturn_t
mpc52xx_uart_int(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	unsigned long pass = ISR_PASS_LIMIT;
	unsigned int keepgoing;
	u8 status;

	spin_lock(&port->lock);

	
	do {
		
		keepgoing = 0;

		psc_ops->rx_clr_irq(port);
		if (psc_ops->rx_rdy(port))
			keepgoing |= mpc52xx_uart_int_rx_chars(port);

		psc_ops->tx_clr_irq(port);
		if (psc_ops->tx_rdy(port))
			keepgoing |= mpc52xx_uart_int_tx_chars(port);

		status = in_8(&PSC(port)->mpc52xx_psc_ipcr);
		if (status & MPC52xx_PSC_D_DCD)
			uart_handle_dcd_change(port, !(status & MPC52xx_PSC_DCD));

		if (status & MPC52xx_PSC_D_CTS)
			uart_handle_cts_change(port, !(status & MPC52xx_PSC_CTS));

		
		if (!(--pass))
			keepgoing = 0;

	} while (keepgoing);

	spin_unlock(&port->lock);

	return IRQ_HANDLED;
}






#ifdef CONFIG_SERIAL_MPC52xx_CONSOLE

static void __init
mpc52xx_console_get_options(struct uart_port *port,
			    int *baud, int *parity, int *bits, int *flow)
{
	struct mpc52xx_psc __iomem *psc = PSC(port);
	unsigned char mr1;

	pr_debug("mpc52xx_console_get_options(port=%p)\n", port);

	
	out_8(&psc->command, MPC52xx_PSC_SEL_MODE_REG_1);
	mr1 = in_8(&psc->mode);

	
	*baud = CONFIG_SERIAL_MPC52xx_CONSOLE_BAUD;

	
	switch (mr1 & MPC52xx_PSC_MODE_BITS_MASK) {
	case MPC52xx_PSC_MODE_5_BITS:
		*bits = 5;
		break;
	case MPC52xx_PSC_MODE_6_BITS:
		*bits = 6;
		break;
	case MPC52xx_PSC_MODE_7_BITS:
		*bits = 7;
		break;
	case MPC52xx_PSC_MODE_8_BITS:
	default:
		*bits = 8;
	}

	if (mr1 & MPC52xx_PSC_MODE_PARNONE)
		*parity = 'n';
	else
		*parity = mr1 & MPC52xx_PSC_MODE_PARODD ? 'o' : 'e';
}

static void
mpc52xx_console_write(struct console *co, const char *s, unsigned int count)
{
	struct uart_port *port = &mpc52xx_uart_ports[co->index];
	unsigned int i, j;

	
	psc_ops->cw_disable_ints(port);

	
	j = 5000000;	
	while (!mpc52xx_uart_tx_empty(port) && --j)
		udelay(1);

	
	for (i = 0; i < count; i++, s++) {
		
		if (*s == '\n')
			psc_ops->write_char(port, '\r');

		
		psc_ops->write_char(port, *s);

		
		j = 20000;	
		while (!mpc52xx_uart_tx_empty(port) && --j)
			udelay(1);
	}

	
	psc_ops->cw_restore_ints(port);
}


static int __init
mpc52xx_console_setup(struct console *co, char *options)
{
	struct uart_port *port = &mpc52xx_uart_ports[co->index];
	struct device_node *np = mpc52xx_uart_nodes[co->index];
	unsigned int uartclk;
	struct resource res;
	int ret;

	int baud = CONFIG_SERIAL_MPC52xx_CONSOLE_BAUD;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	pr_debug("mpc52xx_console_setup co=%p, co->index=%i, options=%s\n",
		 co, co->index, options);

	if ((co->index < 0) || (co->index >= MPC52xx_PSC_MAXNUM)) {
		pr_debug("PSC%x out of range\n", co->index);
		return -EINVAL;
	}

	if (!np) {
		pr_debug("PSC%x not found in device tree\n", co->index);
		return -EINVAL;
	}

	pr_debug("Console on ttyPSC%x is %s\n",
		 co->index, mpc52xx_uart_nodes[co->index]->full_name);

	
	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		pr_debug("Could not get resources for PSC%x\n", co->index);
		return ret;
	}

	uartclk = psc_ops->getuartclk(np);
	if (uartclk == 0) {
		pr_debug("Could not find uart clock frequency!\n");
		return -EINVAL;
	}

	
	spin_lock_init(&port->lock);
	port->uartclk = uartclk;
	port->ops	= &mpc52xx_uart_ops;
	port->mapbase = res.start;
	port->membase = ioremap(res.start, sizeof(struct mpc52xx_psc));
	port->irq = irq_of_parse_and_map(np, 0);

	if (port->membase == NULL)
		return -EINVAL;

	pr_debug("mpc52xx-psc uart at %p, mapped to %p, irq=%x, freq=%i\n",
		 (void *)port->mapbase, port->membase,
		 port->irq, port->uartclk);

	
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		mpc52xx_console_get_options(port, &baud, &parity, &bits, &flow);

	pr_debug("Setting console parameters: %i %i%c1 flow=%c\n",
		 baud, bits, parity, flow);

	return uart_set_options(port, co, baud, parity, bits, flow);
}


static struct uart_driver mpc52xx_uart_driver;

static struct console mpc52xx_console = {
	.name	= "ttyPSC",
	.write	= mpc52xx_console_write,
	.device	= uart_console_device,
	.setup	= mpc52xx_console_setup,
	.flags	= CON_PRINTBUFFER,
	.index	= -1,	
	.data	= &mpc52xx_uart_driver,
};


static int __init
mpc52xx_console_init(void)
{
	mpc52xx_uart_of_enumerate();
	register_console(&mpc52xx_console);
	return 0;
}

console_initcall(mpc52xx_console_init);

#define MPC52xx_PSC_CONSOLE &mpc52xx_console
#else
#define MPC52xx_PSC_CONSOLE NULL
#endif






static struct uart_driver mpc52xx_uart_driver = {
	.driver_name	= "mpc52xx_psc_uart",
	.dev_name	= "ttyPSC",
	.major		= SERIAL_PSC_MAJOR,
	.minor		= SERIAL_PSC_MINOR,
	.nr		= MPC52xx_PSC_MAXNUM,
	.cons		= MPC52xx_PSC_CONSOLE,
};





static struct of_device_id mpc52xx_uart_of_match[] = {
#ifdef CONFIG_PPC_MPC52xx
	{ .compatible = "fsl,mpc5200-psc-uart", .data = &mpc52xx_psc_ops, },
	
	{ .compatible = "mpc5200-psc-uart", .data = &mpc52xx_psc_ops, },
	
	{ .compatible = "mpc5200-serial", .data = &mpc52xx_psc_ops, },
#endif
#ifdef CONFIG_PPC_MPC512x
	{ .compatible = "fsl,mpc5121-psc-uart", .data = &mpc512x_psc_ops, },
#endif
	{},
};

static int __devinit
mpc52xx_uart_of_probe(struct of_device *op, const struct of_device_id *match)
{
	int idx = -1;
	unsigned int uartclk;
	struct uart_port *port = NULL;
	struct resource res;
	int ret;

	dev_dbg(&op->dev, "mpc52xx_uart_probe(op=%p, match=%p)\n", op, match);

	
	for (idx = 0; idx < MPC52xx_PSC_MAXNUM; idx++)
		if (mpc52xx_uart_nodes[idx] == op->node)
			break;
	if (idx >= MPC52xx_PSC_MAXNUM)
		return -EINVAL;
	pr_debug("Found %s assigned to ttyPSC%x\n",
		 mpc52xx_uart_nodes[idx]->full_name, idx);

	uartclk = psc_ops->getuartclk(op->node);
	if (uartclk == 0) {
		dev_dbg(&op->dev, "Could not find uart clock frequency!\n");
		return -EINVAL;
	}

	
	port = &mpc52xx_uart_ports[idx];

	spin_lock_init(&port->lock);
	port->uartclk = uartclk;
	port->fifosize	= 512;
	port->iotype	= UPIO_MEM;
	port->flags	= UPF_BOOT_AUTOCONF |
			  (uart_console(port) ? 0 : UPF_IOREMAP);
	port->line	= idx;
	port->ops	= &mpc52xx_uart_ops;
	port->dev	= &op->dev;

	
	ret = of_address_to_resource(op->node, 0, &res);
	if (ret)
		return ret;

	port->mapbase = res.start;
	if (!port->mapbase) {
		dev_dbg(&op->dev, "Could not allocate resources for PSC\n");
		return -EINVAL;
	}

	port->irq = irq_of_parse_and_map(op->node, 0);
	if (port->irq == NO_IRQ) {
		dev_dbg(&op->dev, "Could not get irq\n");
		return -EINVAL;
	}

	dev_dbg(&op->dev, "mpc52xx-psc uart at %p, irq=%x, freq=%i\n",
		(void *)port->mapbase, port->irq, port->uartclk);

	
	ret = uart_add_one_port(&mpc52xx_uart_driver, port);
	if (ret) {
		irq_dispose_mapping(port->irq);
		return ret;
	}

	dev_set_drvdata(&op->dev, (void *)port);
	return 0;
}

static int
mpc52xx_uart_of_remove(struct of_device *op)
{
	struct uart_port *port = dev_get_drvdata(&op->dev);
	dev_set_drvdata(&op->dev, NULL);

	if (port) {
		uart_remove_one_port(&mpc52xx_uart_driver, port);
		irq_dispose_mapping(port->irq);
	}

	return 0;
}

#ifdef CONFIG_PM
static int
mpc52xx_uart_of_suspend(struct of_device *op, pm_message_t state)
{
	struct uart_port *port = (struct uart_port *) dev_get_drvdata(&op->dev);

	if (port)
		uart_suspend_port(&mpc52xx_uart_driver, port);

	return 0;
}

static int
mpc52xx_uart_of_resume(struct of_device *op)
{
	struct uart_port *port = (struct uart_port *) dev_get_drvdata(&op->dev);

	if (port)
		uart_resume_port(&mpc52xx_uart_driver, port);

	return 0;
}
#endif

static void
mpc52xx_uart_of_assign(struct device_node *np)
{
	int i;

	
	for (i = 0; i < MPC52xx_PSC_MAXNUM; i++) {
		if (mpc52xx_uart_nodes[i] == NULL) {
			of_node_get(np);
			mpc52xx_uart_nodes[i] = np;
			return;
		}
	}
}

static void
mpc52xx_uart_of_enumerate(void)
{
	static int enum_done;
	struct device_node *np;
	const struct  of_device_id *match;
	int i;

	if (enum_done)
		return;

	
	for_each_matching_node(np, mpc52xx_uart_of_match) {
		match = of_match_node(mpc52xx_uart_of_match, np);
		psc_ops = match->data;
		mpc52xx_uart_of_assign(np);
	}

	enum_done = 1;

	for (i = 0; i < MPC52xx_PSC_MAXNUM; i++) {
		if (mpc52xx_uart_nodes[i])
			pr_debug("%s assigned to ttyPSC%x\n",
				 mpc52xx_uart_nodes[i]->full_name, i);
	}
}

MODULE_DEVICE_TABLE(of, mpc52xx_uart_of_match);

static struct of_platform_driver mpc52xx_uart_of_driver = {
	.match_table	= mpc52xx_uart_of_match,
	.probe		= mpc52xx_uart_of_probe,
	.remove		= mpc52xx_uart_of_remove,
#ifdef CONFIG_PM
	.suspend	= mpc52xx_uart_of_suspend,
	.resume		= mpc52xx_uart_of_resume,
#endif
	.driver		= {
		.name	= "mpc52xx-psc-uart",
	},
};






static int __init
mpc52xx_uart_init(void)
{
	int ret;

	printk(KERN_INFO "Serial: MPC52xx PSC UART driver\n");

	ret = uart_register_driver(&mpc52xx_uart_driver);
	if (ret) {
		printk(KERN_ERR "%s: uart_register_driver failed (%i)\n",
		       __FILE__, ret);
		return ret;
	}

	mpc52xx_uart_of_enumerate();

	ret = of_register_platform_driver(&mpc52xx_uart_of_driver);
	if (ret) {
		printk(KERN_ERR "%s: of_register_platform_driver failed (%i)\n",
		       __FILE__, ret);
		uart_unregister_driver(&mpc52xx_uart_driver);
		return ret;
	}

	return 0;
}

static void __exit
mpc52xx_uart_exit(void)
{
	of_unregister_platform_driver(&mpc52xx_uart_of_driver);
	uart_unregister_driver(&mpc52xx_uart_driver);
}


module_init(mpc52xx_uart_init);
module_exit(mpc52xx_uart_exit);

MODULE_AUTHOR("Sylvain Munaut <tnt@246tNt.com>");
MODULE_DESCRIPTION("Freescale MPC52xx PSC UART");
MODULE_LICENSE("GPL");
