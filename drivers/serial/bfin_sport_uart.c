









#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>

#include <asm/delay.h>
#include <asm/portmux.h>

#include "bfin_sport_uart.h"

unsigned short bfin_uart_pin_req_sport0[] =
	{P_SPORT0_TFS, P_SPORT0_DTPRI, P_SPORT0_TSCLK, P_SPORT0_RFS, \
	 P_SPORT0_DRPRI, P_SPORT0_RSCLK, P_SPORT0_DRSEC, P_SPORT0_DTSEC, 0};

unsigned short bfin_uart_pin_req_sport1[] =
	{P_SPORT1_TFS, P_SPORT1_DTPRI, P_SPORT1_TSCLK, P_SPORT1_RFS, \
	P_SPORT1_DRPRI, P_SPORT1_RSCLK, P_SPORT1_DRSEC, P_SPORT1_DTSEC, 0};

#define DRV_NAME "bfin-sport-uart"

struct sport_uart_port {
	struct uart_port	port;
	char			*name;

	int			tx_irq;
	int			rx_irq;
	int			err_irq;
};

static void sport_uart_tx_chars(struct sport_uart_port *up);
static void sport_stop_tx(struct uart_port *port);

static inline void tx_one_byte(struct sport_uart_port *up, unsigned int value)
{
	pr_debug("%s value:%x\n", __func__, value);
	
	__asm__ __volatile__ (
		"R2 = b#01111111100;"
		"R3 = b#10000000001;"
		"%0 <<= 2;"
		"%0 = %0 & R2;"
		"%0 = %0 | R3;"
		: "=d"(value)
		: "d"(value)
		: "ASTAT", "R2", "R3"
	);
	pr_debug("%s value:%x\n", __func__, value);

	SPORT_PUT_TX(up, value);
}

static inline unsigned int rx_one_byte(struct sport_uart_port *up)
{
	unsigned int value, extract;
	u32 tmp_mask1, tmp_mask2, tmp_shift, tmp;

	value = SPORT_GET_RX32(up);
	pr_debug("%s value:%x\n", __func__, value);

	
	__asm__ __volatile__ (
		"%[extr] = 0;"
		"%[mask1] = 0x1801(Z);"
		"%[mask2] = 0x0300(Z);"
		"%[shift] = 0;"
		"LSETUP(.Lloop_s, .Lloop_e) LC0 = %[lc];"
		".Lloop_s:"
		"%[tmp] = extract(%[val], %[mask1].L)(Z);"
		"%[tmp] <<= %[shift];"
		"%[extr] = %[extr] | %[tmp];"
		"%[mask1] = %[mask1] - %[mask2];"
		".Lloop_e:"
		"%[shift] += 1;"
		: [val]"=d"(value), [extr]"=d"(extract), [shift]"=d"(tmp_shift), [tmp]"=d"(tmp),
		  [mask1]"=d"(tmp_mask1), [mask2]"=d"(tmp_mask2)
		: "d"(value), [lc]"a"(8)
		: "ASTAT", "LB0", "LC0", "LT0"
	);

	pr_debug("	extract:%x\n", extract);
	return extract;
}

static int sport_uart_setup(struct sport_uart_port *up, int sclk, int baud_rate)
{
	int tclkdiv, tfsdiv, rclkdiv;

	
	SPORT_PUT_TCR1(up, (LATFS | ITFS | TFSR | TLSBIT | ITCLK));
	SPORT_PUT_TCR2(up, 10);
	pr_debug("%s TCR1:%x, TCR2:%x\n", __func__, SPORT_GET_TCR1(up), SPORT_GET_TCR2(up));

	
	SPORT_PUT_RCR1(up, (RCKFE | LARFS | LRFS | RFSR | IRCLK));
	SPORT_PUT_RCR2(up, 28);
	pr_debug("%s RCR1:%x, RCR2:%x\n", __func__, SPORT_GET_RCR1(up), SPORT_GET_RCR2(up));

	tclkdiv = sclk/(2 * baud_rate) - 1;
	tfsdiv = 12;
	rclkdiv = sclk/(2 * baud_rate * 3) - 1;
	SPORT_PUT_TCLKDIV(up, tclkdiv);
	SPORT_PUT_TFSDIV(up, tfsdiv);
	SPORT_PUT_RCLKDIV(up, rclkdiv);
	SSYNC();
	pr_debug("%s sclk:%d, baud_rate:%d, tclkdiv:%d, tfsdiv:%d, rclkdiv:%d\n",
			__func__, sclk, baud_rate, tclkdiv, tfsdiv, rclkdiv);

	return 0;
}

static irqreturn_t sport_uart_rx_irq(int irq, void *dev_id)
{
	struct sport_uart_port *up = dev_id;
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int ch;

	do {
		ch = rx_one_byte(up);
		up->port.icount.rx++;

		if (uart_handle_sysrq_char(&up->port, ch))
			;
		else
			tty_insert_flip_char(tty, ch, TTY_NORMAL);
	} while (SPORT_GET_STAT(up) & RXNE);
	tty_flip_buffer_push(tty);

	return IRQ_HANDLED;
}

static irqreturn_t sport_uart_tx_irq(int irq, void *dev_id)
{
	sport_uart_tx_chars(dev_id);

	return IRQ_HANDLED;
}

static irqreturn_t sport_uart_err_irq(int irq, void *dev_id)
{
	struct sport_uart_port *up = dev_id;
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int stat = SPORT_GET_STAT(up);

	
	if (stat & ROVF) {
		up->port.icount.overrun++;
		tty_insert_flip_char(tty, 0, TTY_OVERRUN);
		SPORT_PUT_STAT(up, ROVF); 
	}
	
	if (stat & (TOVF | TUVF | RUVF)) {
		printk(KERN_ERR "SPORT Error:%s %s %s\n",
				(stat & TOVF)?"TX overflow":"",
				(stat & TUVF)?"TX underflow":"",
				(stat & RUVF)?"RX underflow":"");
		SPORT_PUT_TCR1(up, SPORT_GET_TCR1(up) & ~TSPEN);
		SPORT_PUT_RCR1(up, SPORT_GET_RCR1(up) & ~RSPEN);
	}
	SSYNC();

	return IRQ_HANDLED;
}


static int sport_startup(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;
	char buffer[20];
	int retval;

	pr_debug("%s enter\n", __func__);
	snprintf(buffer, 20, "%s rx", up->name);
	retval = request_irq(up->rx_irq, sport_uart_rx_irq, IRQF_SAMPLE_RANDOM, buffer, up);
	if (retval) {
		printk(KERN_ERR "Unable to request interrupt %s\n", buffer);
		return retval;
	}

	snprintf(buffer, 20, "%s tx", up->name);
	retval = request_irq(up->tx_irq, sport_uart_tx_irq, IRQF_SAMPLE_RANDOM, buffer, up);
	if (retval) {
		printk(KERN_ERR "Unable to request interrupt %s\n", buffer);
		goto fail1;
	}

	snprintf(buffer, 20, "%s err", up->name);
	retval = request_irq(up->err_irq, sport_uart_err_irq, IRQF_SAMPLE_RANDOM, buffer, up);
	if (retval) {
		printk(KERN_ERR "Unable to request interrupt %s\n", buffer);
		goto fail2;
	}

	if (port->line) {
		if (peripheral_request_list(bfin_uart_pin_req_sport1, DRV_NAME))
			goto fail3;
	} else {
		if (peripheral_request_list(bfin_uart_pin_req_sport0, DRV_NAME))
			goto fail3;
	}

	sport_uart_setup(up, get_sclk(), port->uartclk);

	
	SPORT_PUT_RCR1(up, (SPORT_GET_RCR1(up) | RSPEN));
	SSYNC();

	return 0;


fail3:
	printk(KERN_ERR DRV_NAME
		": Requesting Peripherals failed\n");

	free_irq(up->err_irq, up);
fail2:
	free_irq(up->tx_irq, up);
fail1:
	free_irq(up->rx_irq, up);

	return retval;

}

static void sport_uart_tx_chars(struct sport_uart_port *up)
{
	struct circ_buf *xmit = &up->port.state->xmit;

	if (SPORT_GET_STAT(up) & TXF)
		return;

	if (up->port.x_char) {
		tx_one_byte(up, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char = 0;
		return;
	}

	if (uart_circ_empty(xmit) || uart_tx_stopped(&up->port)) {
		sport_stop_tx(&up->port);
		return;
	}

	while(!(SPORT_GET_STAT(up) & TXF) && !uart_circ_empty(xmit)) {
		tx_one_byte(up, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE -1);
		up->port.icount.tx++;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);
}

static unsigned int sport_tx_empty(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;
	unsigned int stat;

	stat = SPORT_GET_STAT(up);
	pr_debug("%s stat:%04x\n", __func__, stat);
	if (stat & TXHRE) {
		return TIOCSER_TEMT;
	} else
		return 0;
}

static unsigned int sport_get_mctrl(struct uart_port *port)
{
	pr_debug("%s enter\n", __func__);
	return (TIOCM_CTS | TIOCM_CD | TIOCM_DSR);
}

static void sport_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	pr_debug("%s enter\n", __func__);
}

static void sport_stop_tx(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;
	unsigned int stat;

	pr_debug("%s enter\n", __func__);

	stat = SPORT_GET_STAT(up);
	while(!(stat & TXHRE)) {
		udelay(1);
		stat = SPORT_GET_STAT(up);
	}
	
	udelay(500);

	SPORT_PUT_TCR1(up, (SPORT_GET_TCR1(up) & ~TSPEN));
	SSYNC();

	return;
}

static void sport_start_tx(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;

	pr_debug("%s enter\n", __func__);
	
	sport_uart_tx_chars(up);

	
	SPORT_PUT_TCR1(up, (SPORT_GET_TCR1(up) | TSPEN));
	SSYNC();
	pr_debug("%s exit\n", __func__);
}

static void sport_stop_rx(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;

	pr_debug("%s enter\n", __func__);
	
	SPORT_PUT_RCR1(up, (SPORT_GET_RCR1(up) & ~RSPEN));
	SSYNC();
}

static void sport_enable_ms(struct uart_port *port)
{
	pr_debug("%s enter\n", __func__);
}

static void sport_break_ctl(struct uart_port *port, int break_state)
{
	pr_debug("%s enter\n", __func__);
}

static void sport_shutdown(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;

	pr_debug("%s enter\n", __func__);

	
	SPORT_PUT_TCR1(up, (SPORT_GET_TCR1(up) & ~TSPEN));
	SPORT_PUT_RCR1(up, (SPORT_GET_RCR1(up) & ~RSPEN));
	SSYNC();

	if (port->line) {
		peripheral_free_list(bfin_uart_pin_req_sport1);
	} else {
		peripheral_free_list(bfin_uart_pin_req_sport0);
	}

	free_irq(up->rx_irq, up);
	free_irq(up->tx_irq, up);
	free_irq(up->err_irq, up);
}

static void sport_set_termios(struct uart_port *port,
		struct ktermios *termios, struct ktermios *old)
{
	pr_debug("%s enter, c_cflag:%08x\n", __func__, termios->c_cflag);
	uart_update_timeout(port, CS8 ,port->uartclk);
}

static const char *sport_type(struct uart_port *port)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;

	pr_debug("%s enter\n", __func__);
	return up->name;
}

static void sport_release_port(struct uart_port *port)
{
	pr_debug("%s enter\n", __func__);
}

static int sport_request_port(struct uart_port *port)
{
	pr_debug("%s enter\n", __func__);
	return 0;
}

static void sport_config_port(struct uart_port *port, int flags)
{
	struct sport_uart_port *up = (struct sport_uart_port *)port;

	pr_debug("%s enter\n", __func__);
	up->port.type = PORT_BFIN_SPORT;
}

static int sport_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	pr_debug("%s enter\n", __func__);
	return 0;
}

struct uart_ops sport_uart_ops = {
	.tx_empty	= sport_tx_empty,
	.set_mctrl	= sport_set_mctrl,
	.get_mctrl	= sport_get_mctrl,
	.stop_tx	= sport_stop_tx,
	.start_tx	= sport_start_tx,
	.stop_rx	= sport_stop_rx,
	.enable_ms	= sport_enable_ms,
	.break_ctl	= sport_break_ctl,
	.startup	= sport_startup,
	.shutdown	= sport_shutdown,
	.set_termios	= sport_set_termios,
	.type		= sport_type,
	.release_port	= sport_release_port,
	.request_port	= sport_request_port,
	.config_port	= sport_config_port,
	.verify_port	= sport_verify_port,
};

static struct sport_uart_port sport_uart_ports[] = {
	{ 
		.name	= "SPORT0",
		.tx_irq = IRQ_SPORT0_TX,
		.rx_irq = IRQ_SPORT0_RX,
		.err_irq= IRQ_SPORT0_ERROR,
		.port	= {
			.type		= PORT_BFIN_SPORT,
			.iotype		= UPIO_MEM,
			.membase	= (void __iomem *)SPORT0_TCR1,
			.mapbase	= SPORT0_TCR1,
			.irq		= IRQ_SPORT0_RX,
			.uartclk	= CONFIG_SPORT_BAUD_RATE,
			.fifosize	= 8,
			.ops		= &sport_uart_ops,
			.line		= 0,
		},
	}, { 
		.name	= "SPORT1",
		.tx_irq = IRQ_SPORT1_TX,
		.rx_irq = IRQ_SPORT1_RX,
		.err_irq= IRQ_SPORT1_ERROR,
		.port	= {
			.type		= PORT_BFIN_SPORT,
			.iotype		= UPIO_MEM,
			.membase	= (void __iomem *)SPORT1_TCR1,
			.mapbase	= SPORT1_TCR1,
			.irq		= IRQ_SPORT1_RX,
			.uartclk	= CONFIG_SPORT_BAUD_RATE,
			.fifosize	= 8,
			.ops		= &sport_uart_ops,
			.line		= 1,
		},
	}
};

static struct uart_driver sport_uart_reg = {
	.owner		= THIS_MODULE,
	.driver_name	= "SPORT-UART",
	.dev_name	= "ttySS",
	.major		= 204,
	.minor		= 84,
	.nr		= ARRAY_SIZE(sport_uart_ports),
	.cons		= NULL,
};

static int sport_uart_suspend(struct platform_device *dev, pm_message_t state)
{
	struct sport_uart_port *sport = platform_get_drvdata(dev);

	pr_debug("%s enter\n", __func__);
	if (sport)
		uart_suspend_port(&sport_uart_reg, &sport->port);

	return 0;
}

static int sport_uart_resume(struct platform_device *dev)
{
	struct sport_uart_port *sport = platform_get_drvdata(dev);

	pr_debug("%s enter\n", __func__);
	if (sport)
		uart_resume_port(&sport_uart_reg, &sport->port);

	return 0;
}

static int sport_uart_probe(struct platform_device *dev)
{
	pr_debug("%s enter\n", __func__);
	sport_uart_ports[dev->id].port.dev = &dev->dev;
	uart_add_one_port(&sport_uart_reg, &sport_uart_ports[dev->id].port);
	platform_set_drvdata(dev, &sport_uart_ports[dev->id]);

	return 0;
}

static int sport_uart_remove(struct platform_device *dev)
{
	struct sport_uart_port *sport = platform_get_drvdata(dev);

	pr_debug("%s enter\n", __func__);
	platform_set_drvdata(dev, NULL);

	if (sport)
		uart_remove_one_port(&sport_uart_reg, &sport->port);

	return 0;
}

static struct platform_driver sport_uart_driver = {
	.probe		= sport_uart_probe,
	.remove		= sport_uart_remove,
	.suspend	= sport_uart_suspend,
	.resume		= sport_uart_resume,
	.driver		= {
		.name	= DRV_NAME,
	},
};

static int __init sport_uart_init(void)
{
	int ret;

	pr_debug("%s enter\n", __func__);
	ret = uart_register_driver(&sport_uart_reg);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register %s:%d\n",
				sport_uart_reg.driver_name, ret);
		return ret;
	}

	ret = platform_driver_register(&sport_uart_driver);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register sport uart driver:%d\n", ret);
		uart_unregister_driver(&sport_uart_reg);
	}


	pr_debug("%s exit\n", __func__);
	return ret;
}

static void __exit sport_uart_exit(void)
{
	pr_debug("%s enter\n", __func__);
	platform_driver_unregister(&sport_uart_driver);
	uart_unregister_driver(&sport_uart_reg);
}

module_init(sport_uart_init);
module_exit(sport_uart_exit);

MODULE_LICENSE("GPL");
