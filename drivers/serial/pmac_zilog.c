

#undef DEBUG
#undef DEBUG_HARD
#undef USE_CTRL_O_SYSRQ

#include <linux/module.h>
#include <linux/tty.h>

#include <linux/tty_flip.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/slab.h>
#include <linux/adb.h>
#include <linux/pmu.h>
#include <linux/bitops.h>
#include <linux/sysrq.h>
#include <linux/mutex.h>
#include <asm/sections.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/prom.h>
#include <asm/machdep.h>
#include <asm/pmac_feature.h>
#include <asm/dbdma.h>
#include <asm/macio.h>

#if defined (CONFIG_SERIAL_PMACZILOG_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/serial.h>
#include <linux/serial_core.h>

#include "pmac_zilog.h"


#undef HAS_DBDMA

static char version[] __initdata = "pmac_zilog: 0.6 (Benjamin Herrenschmidt <benh@kernel.crashing.org>)";
MODULE_AUTHOR("Benjamin Herrenschmidt <benh@kernel.crashing.org>");
MODULE_DESCRIPTION("Driver for the PowerMac serial ports.");
MODULE_LICENSE("GPL");

#define PWRDBG(fmt, arg...)	printk(KERN_DEBUG fmt , ## arg)

#ifdef CONFIG_SERIAL_PMACZILOG_TTYS
#define PMACZILOG_MAJOR		TTY_MAJOR
#define PMACZILOG_MINOR		64
#define PMACZILOG_NAME		"ttyS"
#else
#define PMACZILOG_MAJOR		204
#define PMACZILOG_MINOR		192
#define PMACZILOG_NAME		"ttyPZ"
#endif



static struct uart_pmac_port	pmz_ports[MAX_ZS_PORTS];
static int			pmz_ports_count;
static DEFINE_MUTEX(pmz_irq_mutex);

static struct uart_driver pmz_uart_reg = {
	.owner		=	THIS_MODULE,
	.driver_name	=	PMACZILOG_NAME,
	.dev_name	=	PMACZILOG_NAME,
	.major		=	PMACZILOG_MAJOR,
	.minor		=	PMACZILOG_MINOR,
};



static void pmz_load_zsregs(struct uart_pmac_port *uap, u8 *regs)
{
	int i;

	if (ZS_IS_ASLEEP(uap))
		return;

	
	for (i = 0; i < 1000; i++) {
		unsigned char stat = read_zsreg(uap, R1);
		if (stat & ALL_SNT)
			break;
		udelay(100);
	}

	ZS_CLEARERR(uap);
	zssync(uap);
	ZS_CLEARFIFO(uap);
	zssync(uap);
	ZS_CLEARERR(uap);

	
	write_zsreg(uap, R1,
		    regs[R1] & ~(RxINT_MASK | TxINT_ENAB | EXT_INT_ENAB));

	
	write_zsreg(uap, R4, regs[R4]);

	
	write_zsreg(uap, R10, regs[R10]);

	
       	write_zsreg(uap, R3, regs[R3] & ~RxENABLE);
       	write_zsreg(uap, R5, regs[R5] & ~TxENABLE);

	
	write_zsreg(uap, R15, regs[R15] | EN85C30);
	write_zsreg(uap, R7, regs[R7P]);

	
	write_zsreg(uap, R15, regs[R15] & ~EN85C30);

	
	write_zsreg(uap, R6, regs[R6]);
	write_zsreg(uap, R7, regs[R7]);

	
	write_zsreg(uap, R14, regs[R14] & ~BRENAB);

	
	write_zsreg(uap, R11, regs[R11]);

	
	write_zsreg(uap, R12, regs[R12]);
	write_zsreg(uap, R13, regs[R13]);
	
	
	write_zsreg(uap, R14, regs[R14]);

	
	write_zsreg(uap, R0, RES_EXT_INT);
	write_zsreg(uap, R0, RES_EXT_INT);

	
	write_zsreg(uap, R3, regs[R3]);
	write_zsreg(uap, R5, regs[R5]);

	
	write_zsreg(uap, R1, regs[R1]);

	
	write_zsreg(uap, R9, regs[R9]);
}


static void pmz_maybe_update_regs(struct uart_pmac_port *uap)
{
       	if (!ZS_REGS_HELD(uap)) {
		if (ZS_TX_ACTIVE(uap)) {
			uap->flags |= PMACZILOG_FLAG_REGS_HELD;
		} else {
			pmz_debug("pmz: maybe_update_regs: updating\n");
			pmz_load_zsregs(uap, uap->curregs);
		}
	}
}

static struct tty_struct *pmz_receive_chars(struct uart_pmac_port *uap)
{
	struct tty_struct *tty = NULL;
	unsigned char ch, r1, drop, error, flag;
	int loops = 0;

	
	if (!ZS_IS_OPEN(uap)) {
		pmz_debug("pmz: draining input\n");
		
		for (;;) {
			if ((++loops) > 1000)
				goto flood;
			(void)read_zsreg(uap, R1);
			write_zsreg(uap, R0, ERR_RES);
			(void)read_zsdata(uap);
			ch = read_zsreg(uap, R0);
			if (!(ch & Rx_CH_AV))
				break;
		}
		return NULL;
	}

	
	if (uap->port.state == NULL || uap->port.state->port.tty == NULL) {
		WARN_ON(1);
		(void)read_zsdata(uap);
		return NULL;
	}
	tty = uap->port.state->port.tty;

	while (1) {
		error = 0;
		drop = 0;

		r1 = read_zsreg(uap, R1);
		ch = read_zsdata(uap);

		if (r1 & (PAR_ERR | Rx_OVR | CRC_ERR)) {
			write_zsreg(uap, R0, ERR_RES);
			zssync(uap);
		}

		ch &= uap->parity_mask;
		if (ch == 0 && uap->flags & PMACZILOG_FLAG_BREAK) {
			uap->flags &= ~PMACZILOG_FLAG_BREAK;
		}

#if defined(CONFIG_MAGIC_SYSRQ) && defined(CONFIG_SERIAL_CORE_CONSOLE)
#ifdef USE_CTRL_O_SYSRQ
		
		if (ch == '\x0f') {
			uap->port.sysrq = jiffies + HZ*5;
			goto next_char;
		}
#endif 
		if (uap->port.sysrq) {
			int swallow;
			spin_unlock(&uap->port.lock);
			swallow = uart_handle_sysrq_char(&uap->port, ch);
			spin_lock(&uap->port.lock);
			if (swallow)
				goto next_char;
 		}
#endif 

		
		if (drop)
			goto next_char;

		flag = TTY_NORMAL;
		uap->port.icount.rx++;

		if (r1 & (PAR_ERR | Rx_OVR | CRC_ERR | BRK_ABRT)) {
			error = 1;
			if (r1 & BRK_ABRT) {
				pmz_debug("pmz: got break !\n");
				r1 &= ~(PAR_ERR | CRC_ERR);
				uap->port.icount.brk++;
				if (uart_handle_break(&uap->port))
					goto next_char;
			}
			else if (r1 & PAR_ERR)
				uap->port.icount.parity++;
			else if (r1 & CRC_ERR)
				uap->port.icount.frame++;
			if (r1 & Rx_OVR)
				uap->port.icount.overrun++;
			r1 &= uap->port.read_status_mask;
			if (r1 & BRK_ABRT)
				flag = TTY_BREAK;
			else if (r1 & PAR_ERR)
				flag = TTY_PARITY;
			else if (r1 & CRC_ERR)
				flag = TTY_FRAME;
		}

		if (uap->port.ignore_status_mask == 0xff ||
		    (r1 & uap->port.ignore_status_mask) == 0) {
		    	tty_insert_flip_char(tty, ch, flag);
		}
		if (r1 & Rx_OVR)
			tty_insert_flip_char(tty, 0, TTY_OVERRUN);
	next_char:
		
		if ((++loops) > 1000)
			goto flood;
		ch = read_zsreg(uap, R0);
		if (!(ch & Rx_CH_AV))
			break;
	}

	return tty;
 flood:
	uap->curregs[R1] &= ~(EXT_INT_ENAB | TxINT_ENAB | RxINT_MASK);
	write_zsreg(uap, R1, uap->curregs[R1]);
	zssync(uap);
	dev_err(&uap->dev->ofdev.dev, "pmz: rx irq flood !\n");
	return tty;
}

static void pmz_status_handle(struct uart_pmac_port *uap)
{
	unsigned char status;

	status = read_zsreg(uap, R0);
	write_zsreg(uap, R0, RES_EXT_INT);
	zssync(uap);

	if (ZS_IS_OPEN(uap) && ZS_WANTS_MODEM_STATUS(uap)) {
		if (status & SYNC_HUNT)
			uap->port.icount.dsr++;

		
		if ((status ^ uap->prev_status) & DCD)
			uart_handle_dcd_change(&uap->port,
					       (status & DCD));
		if ((status ^ uap->prev_status) & CTS)
			uart_handle_cts_change(&uap->port,
					       !(status & CTS));

		wake_up_interruptible(&uap->port.state->port.delta_msr_wait);
	}

	if (status & BRK_ABRT)
		uap->flags |= PMACZILOG_FLAG_BREAK;

	uap->prev_status = status;
}

static void pmz_transmit_chars(struct uart_pmac_port *uap)
{
	struct circ_buf *xmit;

	if (ZS_IS_ASLEEP(uap))
		return;
	if (ZS_IS_CONS(uap)) {
		unsigned char status = read_zsreg(uap, R0);

		
		if (!(status & Tx_BUF_EMP))
			return;
	}

	uap->flags &= ~PMACZILOG_FLAG_TX_ACTIVE;

	if (ZS_REGS_HELD(uap)) {
		pmz_load_zsregs(uap, uap->curregs);
		uap->flags &= ~PMACZILOG_FLAG_REGS_HELD;
	}

	if (ZS_TX_STOPPED(uap)) {
		uap->flags &= ~PMACZILOG_FLAG_TX_STOPPED;
		goto ack_tx_int;
	}

	if (uap->port.x_char) {
		uap->flags |= PMACZILOG_FLAG_TX_ACTIVE;
		write_zsdata(uap, uap->port.x_char);
		zssync(uap);
		uap->port.icount.tx++;
		uap->port.x_char = 0;
		return;
	}

	if (uap->port.state == NULL)
		goto ack_tx_int;
	xmit = &uap->port.state->xmit;
	if (uart_circ_empty(xmit)) {
		uart_write_wakeup(&uap->port);
		goto ack_tx_int;
	}
	if (uart_tx_stopped(&uap->port))
		goto ack_tx_int;

	uap->flags |= PMACZILOG_FLAG_TX_ACTIVE;
	write_zsdata(uap, xmit->buf[xmit->tail]);
	zssync(uap);

	xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	uap->port.icount.tx++;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&uap->port);

	return;

ack_tx_int:
	write_zsreg(uap, R0, RES_Tx_P);
	zssync(uap);
}


static irqreturn_t pmz_interrupt(int irq, void *dev_id)
{
	struct uart_pmac_port *uap = dev_id;
	struct uart_pmac_port *uap_a;
	struct uart_pmac_port *uap_b;
	int rc = IRQ_NONE;
	struct tty_struct *tty;
	u8 r3;

	uap_a = pmz_get_port_A(uap);
	uap_b = uap_a->mate;
       
       	spin_lock(&uap_a->port.lock);
	r3 = read_zsreg(uap_a, R3);

#ifdef DEBUG_HARD
	pmz_debug("irq, r3: %x\n", r3);
#endif
       	
	tty = NULL;
       	if (r3 & (CHAEXT | CHATxIP | CHARxIP)) {
		write_zsreg(uap_a, R0, RES_H_IUS);
		zssync(uap_a);		
       		if (r3 & CHAEXT)
       			pmz_status_handle(uap_a);
		if (r3 & CHARxIP)
			tty = pmz_receive_chars(uap_a);
       		if (r3 & CHATxIP)
       			pmz_transmit_chars(uap_a);
	        rc = IRQ_HANDLED;
       	}
       	spin_unlock(&uap_a->port.lock);
	if (tty != NULL)
		tty_flip_buffer_push(tty);

	if (uap_b->node == NULL)
		goto out;

       	spin_lock(&uap_b->port.lock);
	tty = NULL;
	if (r3 & (CHBEXT | CHBTxIP | CHBRxIP)) {
		write_zsreg(uap_b, R0, RES_H_IUS);
		zssync(uap_b);
       		if (r3 & CHBEXT)
       			pmz_status_handle(uap_b);
       	       	if (r3 & CHBRxIP)
       			tty = pmz_receive_chars(uap_b);
       		if (r3 & CHBTxIP)
       			pmz_transmit_chars(uap_b);
	       	rc = IRQ_HANDLED;
       	}
       	spin_unlock(&uap_b->port.lock);
	if (tty != NULL)
		tty_flip_buffer_push(tty);

 out:
#ifdef DEBUG_HARD
	pmz_debug("irq done.\n");
#endif
	return rc;
}


static inline u8 pmz_peek_status(struct uart_pmac_port *uap)
{
	unsigned long flags;
	u8 status;
	
	spin_lock_irqsave(&uap->port.lock, flags);
	status = read_zsreg(uap, R0);
	spin_unlock_irqrestore(&uap->port.lock, flags);

	return status;
}


static unsigned int pmz_tx_empty(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char status;

	if (ZS_IS_ASLEEP(uap) || uap->node == NULL)
		return TIOCSER_TEMT;

	status = pmz_peek_status(to_pmz(port));
	if (status & Tx_BUF_EMP)
		return TIOCSER_TEMT;
	return 0;
}


static void pmz_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char set_bits, clear_bits;

        
	if (ZS_IS_IRDA(uap))
		return;
	
	if (ZS_IS_ASLEEP(uap) ||
	    !(ZS_IS_OPEN(uap) || ZS_IS_CONS(uap)))
		return;

	set_bits = clear_bits = 0;

	if (ZS_IS_INTMODEM(uap)) {
		if (mctrl & TIOCM_RTS)
			set_bits |= RTS;
		else
			clear_bits |= RTS;
	}
	if (mctrl & TIOCM_DTR)
		set_bits |= DTR;
	else
		clear_bits |= DTR;

	 
	uap->curregs[R5] |= set_bits;
	uap->curregs[R5] &= ~clear_bits;
	if (ZS_IS_ASLEEP(uap))
		return;
	write_zsreg(uap, R5, uap->curregs[R5]);
	pmz_debug("pmz_set_mctrl: set bits: %x, clear bits: %x -> %x\n",
		  set_bits, clear_bits, uap->curregs[R5]);
	zssync(uap);
}


static unsigned int pmz_get_mctrl(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char status;
	unsigned int ret;

	if (ZS_IS_ASLEEP(uap) || uap->node == NULL)
		return 0;

	status = read_zsreg(uap, R0);

	ret = 0;
	if (status & DCD)
		ret |= TIOCM_CAR;
	if (status & SYNC_HUNT)
		ret |= TIOCM_DSR;
	if (!(status & CTS))
		ret |= TIOCM_CTS;

	return ret;
}


static void pmz_stop_tx(struct uart_port *port)
{
	to_pmz(port)->flags |= PMACZILOG_FLAG_TX_STOPPED;
}


static void pmz_start_tx(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char status;

	pmz_debug("pmz: start_tx()\n");

	uap->flags |= PMACZILOG_FLAG_TX_ACTIVE;
	uap->flags &= ~PMACZILOG_FLAG_TX_STOPPED;

	if (ZS_IS_ASLEEP(uap) || uap->node == NULL)
		return;

	status = read_zsreg(uap, R0);

	
	if (!(status & Tx_BUF_EMP))
		return;

	
	if (port->x_char) {
		write_zsdata(uap, port->x_char);
		zssync(uap);
		port->icount.tx++;
		port->x_char = 0;
	} else {
		struct circ_buf *xmit = &port->state->xmit;

		write_zsdata(uap, xmit->buf[xmit->tail]);
		zssync(uap);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;

		if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
			uart_write_wakeup(&uap->port);
	}
	pmz_debug("pmz: start_tx() done.\n");
}


static void pmz_stop_rx(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);

	if (ZS_IS_ASLEEP(uap) || uap->node == NULL)
		return;

	pmz_debug("pmz: stop_rx()()\n");

	
	uap->curregs[R1] &= ~RxINT_MASK;
	pmz_maybe_update_regs(uap);

	pmz_debug("pmz: stop_rx() done.\n");
}


static void pmz_enable_ms(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char new_reg;

	if (ZS_IS_IRDA(uap) || uap->node == NULL)
		return;
	new_reg = uap->curregs[R15] | (DCDIE | SYNCIE | CTSIE);
	if (new_reg != uap->curregs[R15]) {
		uap->curregs[R15] = new_reg;

		if (ZS_IS_ASLEEP(uap))
			return;
		 
		write_zsreg(uap, R15, uap->curregs[R15]);
	}
}


static void pmz_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned char set_bits, clear_bits, new_reg;
	unsigned long flags;

	if (uap->node == NULL)
		return;
	set_bits = clear_bits = 0;

	if (break_state)
		set_bits |= SND_BRK;
	else
		clear_bits |= SND_BRK;

	spin_lock_irqsave(&port->lock, flags);

	new_reg = (uap->curregs[R5] | set_bits) & ~clear_bits;
	if (new_reg != uap->curregs[R5]) {
		uap->curregs[R5] = new_reg;

		 
		if (ZS_IS_ASLEEP(uap))
			return;
		write_zsreg(uap, R5, uap->curregs[R5]);
	}

	spin_unlock_irqrestore(&port->lock, flags);
}


static int pmz_set_scc_power(struct uart_pmac_port *uap, int state)
{
	int delay = 0;
	int rc;

	if (state) {
		rc = pmac_call_feature(
			PMAC_FTR_SCC_ENABLE, uap->node, uap->port_type, 1);
		pmz_debug("port power on result: %d\n", rc);
		if (ZS_IS_INTMODEM(uap)) {
			rc = pmac_call_feature(
				PMAC_FTR_MODEM_ENABLE, uap->node, 0, 1);
			delay = 2500;	
			pmz_debug("modem power result: %d\n", rc);
		}
	} else {
		
		if (ZS_IS_INTMODEM(uap)) {
			rc = pmac_call_feature(
				PMAC_FTR_MODEM_ENABLE, uap->node, 0, 0);
			pmz_debug("port power off result: %d\n", rc);
		}
		pmac_call_feature(PMAC_FTR_SCC_ENABLE, uap->node, uap->port_type, 0);
	}
	return delay;
}


static void pmz_fix_zero_bug_scc(struct uart_pmac_port *uap)
{
	write_zsreg(uap, 9, ZS_IS_CHANNEL_A(uap) ? CHRA : CHRB);
	zssync(uap);
	udelay(10);
	write_zsreg(uap, 9, (ZS_IS_CHANNEL_A(uap) ? CHRA : CHRB) | NV);
	zssync(uap);

	write_zsreg(uap, 4, X1CLK | MONSYNC);
	write_zsreg(uap, 3, Rx8);
	write_zsreg(uap, 5, Tx8 | RTS);
	write_zsreg(uap, 9, NV);	
	write_zsreg(uap, 11, RCBR | TCBR);
	write_zsreg(uap, 12, 0);
	write_zsreg(uap, 13, 0);
	write_zsreg(uap, 14, (LOOPBAK | BRSRC));
	write_zsreg(uap, 14, (LOOPBAK | BRSRC | BRENAB));
	write_zsreg(uap, 3, Rx8 | RxENABLE);
	write_zsreg(uap, 0, RES_EXT_INT);
	write_zsreg(uap, 0, RES_EXT_INT);
	write_zsreg(uap, 0, RES_EXT_INT);	

	
	write_zsreg(uap, 9, NV);
	write_zsreg(uap, 4, X16CLK | SB_MASK);
	write_zsreg(uap, 3, Rx8);

	while (read_zsreg(uap, 0) & Rx_CH_AV) {
		(void)read_zsreg(uap, 8);
		write_zsreg(uap, 0, RES_EXT_INT);
		write_zsreg(uap, 0, ERR_RES);
	}
}


static int __pmz_startup(struct uart_pmac_port *uap)
{
	int pwr_delay = 0;

	memset(&uap->curregs, 0, sizeof(uap->curregs));

	
	pwr_delay = pmz_set_scc_power(uap, 1);

	
	pmz_fix_zero_bug_scc(uap);

	
	uap->curregs[R9] = 0;
	write_zsreg(uap, 9, ZS_IS_CHANNEL_A(uap) ? CHRA : CHRB);
	zssync(uap);
	udelay(10);
	write_zsreg(uap, 9, 0);
	zssync(uap);

	
	write_zsreg(uap, R1, 0);
	write_zsreg(uap, R0, ERR_RES);
	write_zsreg(uap, R0, ERR_RES);
	write_zsreg(uap, R0, RES_H_IUS);
	write_zsreg(uap, R0, RES_H_IUS);

	
	uap->curregs[R4] = X16CLK | SB1;
	uap->curregs[R3] = Rx8;
	uap->curregs[R5] = Tx8 | RTS;
	if (!ZS_IS_IRDA(uap))
		uap->curregs[R5] |= DTR;
	uap->curregs[R12] = 0;
	uap->curregs[R13] = 0;
	uap->curregs[R14] = BRENAB;

	
	uap->curregs[R15] = BRKIE;

	
	uap->curregs[R9] |= NV | MIE;

	pmz_load_zsregs(uap, uap->curregs);

	
	write_zsreg(uap, R3, uap->curregs[R3] |= RxENABLE);
	write_zsreg(uap, R5, uap->curregs[R5] |= TxENABLE);

	
	uap->prev_status = read_zsreg(uap, R0);


	return pwr_delay;
}

static void pmz_irda_reset(struct uart_pmac_port *uap)
{
	uap->curregs[R5] |= DTR;
	write_zsreg(uap, R5, uap->curregs[R5]);
	zssync(uap);
	mdelay(110);
	uap->curregs[R5] &= ~DTR;
	write_zsreg(uap, R5, uap->curregs[R5]);
	zssync(uap);
	mdelay(10);
}


static int pmz_startup(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned long flags;
	int pwr_delay = 0;

	pmz_debug("pmz: startup()\n");

	if (ZS_IS_ASLEEP(uap))
		return -EAGAIN;
	if (uap->node == NULL)
		return -ENODEV;

	mutex_lock(&pmz_irq_mutex);

	uap->flags |= PMACZILOG_FLAG_IS_OPEN;

	
	if (!ZS_IS_CONS(uap)) {
		spin_lock_irqsave(&port->lock, flags);
		pwr_delay = __pmz_startup(uap);
		spin_unlock_irqrestore(&port->lock, flags);
	}	

	pmz_get_port_A(uap)->flags |= PMACZILOG_FLAG_IS_IRQ_ON;
	if (request_irq(uap->port.irq, pmz_interrupt, IRQF_SHARED, "PowerMac Zilog", uap)) {
		dev_err(&uap->dev->ofdev.dev,
			"Unable to register zs interrupt handler.\n");
		pmz_set_scc_power(uap, 0);
		mutex_unlock(&pmz_irq_mutex);
		return -ENXIO;
	}

	mutex_unlock(&pmz_irq_mutex);

	
	if (pwr_delay != 0) {
		pmz_debug("pmz: delaying %d ms\n", pwr_delay);
		msleep(pwr_delay);
	}

	
	if (ZS_IS_IRDA(uap))
		pmz_irda_reset(uap);

	
	spin_lock_irqsave(&port->lock, flags);
	uap->curregs[R1] |= INT_ALL_Rx | TxINT_ENAB;
	if (!ZS_IS_EXTCLK(uap))
		uap->curregs[R1] |= EXT_INT_ENAB;
	write_zsreg(uap, R1, uap->curregs[R1]);
       	spin_unlock_irqrestore(&port->lock, flags);

	pmz_debug("pmz: startup() done.\n");

	return 0;
}

static void pmz_shutdown(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned long flags;

	pmz_debug("pmz: shutdown()\n");

	if (uap->node == NULL)
		return;

	mutex_lock(&pmz_irq_mutex);

	
       	free_irq(uap->port.irq, uap);

	spin_lock_irqsave(&port->lock, flags);

	uap->flags &= ~PMACZILOG_FLAG_IS_OPEN;

	if (!ZS_IS_OPEN(uap->mate))
		pmz_get_port_A(uap)->flags &= ~PMACZILOG_FLAG_IS_IRQ_ON;

	
	if (!ZS_IS_ASLEEP(uap)) {
		uap->curregs[R1] &= ~(EXT_INT_ENAB | TxINT_ENAB | RxINT_MASK);
		write_zsreg(uap, R1, uap->curregs[R1]);
		zssync(uap);
	}

	if (ZS_IS_CONS(uap) || ZS_IS_ASLEEP(uap)) {
		spin_unlock_irqrestore(&port->lock, flags);
		mutex_unlock(&pmz_irq_mutex);
		return;
	}

	
	uap->curregs[R3] &= ~RxENABLE;
	uap->curregs[R5] &= ~TxENABLE;

	
	uap->curregs[R5] &= ~SND_BRK;
	pmz_maybe_update_regs(uap);

	
	pmz_set_scc_power(uap, 0);

	spin_unlock_irqrestore(&port->lock, flags);

	mutex_unlock(&pmz_irq_mutex);

	pmz_debug("pmz: shutdown() done.\n");
}


static void pmz_convert_to_zs(struct uart_pmac_port *uap, unsigned int cflag,
			      unsigned int iflag, unsigned long baud)
{
	int brg;


	
	if (baud >= 115200 && ZS_IS_IRDA(uap)) {
		uap->curregs[R4] = X1CLK;
		uap->curregs[R11] = RCTRxCP | TCTRxCP;
		uap->curregs[R14] = 0; 
		uap->curregs[R12] = 0;
		uap->curregs[R13] = 0;
		uap->flags |= PMACZILOG_FLAG_IS_EXTCLK;
	} else {
		switch (baud) {
		case ZS_CLOCK/16:	
			uap->curregs[R4] = X16CLK;
			uap->curregs[R11] = 0;
			uap->curregs[R14] = 0;
			break;
		case ZS_CLOCK/32:	
			uap->curregs[R4] = X32CLK;
			uap->curregs[R11] = 0;
			uap->curregs[R14] = 0;
			break;
		default:
			uap->curregs[R4] = X16CLK;
			uap->curregs[R11] = TCBR | RCBR;
			brg = BPS_TO_BRG(baud, ZS_CLOCK / 16);
			uap->curregs[R12] = (brg & 255);
			uap->curregs[R13] = ((brg >> 8) & 255);
			uap->curregs[R14] = BRENAB;
		}
		uap->flags &= ~PMACZILOG_FLAG_IS_EXTCLK;
	}

	
	uap->curregs[3] &= ~RxN_MASK;
	uap->curregs[5] &= ~TxN_MASK;

	switch (cflag & CSIZE) {
	case CS5:
		uap->curregs[3] |= Rx5;
		uap->curregs[5] |= Tx5;
		uap->parity_mask = 0x1f;
		break;
	case CS6:
		uap->curregs[3] |= Rx6;
		uap->curregs[5] |= Tx6;
		uap->parity_mask = 0x3f;
		break;
	case CS7:
		uap->curregs[3] |= Rx7;
		uap->curregs[5] |= Tx7;
		uap->parity_mask = 0x7f;
		break;
	case CS8:
	default:
		uap->curregs[3] |= Rx8;
		uap->curregs[5] |= Tx8;
		uap->parity_mask = 0xff;
		break;
	};
	uap->curregs[4] &= ~(SB_MASK);
	if (cflag & CSTOPB)
		uap->curregs[4] |= SB2;
	else
		uap->curregs[4] |= SB1;
	if (cflag & PARENB)
		uap->curregs[4] |= PAR_ENAB;
	else
		uap->curregs[4] &= ~PAR_ENAB;
	if (!(cflag & PARODD))
		uap->curregs[4] |= PAR_EVEN;
	else
		uap->curregs[4] &= ~PAR_EVEN;

	uap->port.read_status_mask = Rx_OVR;
	if (iflag & INPCK)
		uap->port.read_status_mask |= CRC_ERR | PAR_ERR;
	if (iflag & (BRKINT | PARMRK))
		uap->port.read_status_mask |= BRK_ABRT;

	uap->port.ignore_status_mask = 0;
	if (iflag & IGNPAR)
		uap->port.ignore_status_mask |= CRC_ERR | PAR_ERR;
	if (iflag & IGNBRK) {
		uap->port.ignore_status_mask |= BRK_ABRT;
		if (iflag & IGNPAR)
			uap->port.ignore_status_mask |= Rx_OVR;
	}

	if ((cflag & CREAD) == 0)
		uap->port.ignore_status_mask = 0xff;
}



static void pmz_irda_setup(struct uart_pmac_port *uap, unsigned long *baud)
{
	u8 cmdbyte;
	int t, version;

	switch (*baud) {
	
	case 2400:
		cmdbyte = 0x53;
		break;
	case 4800:
		cmdbyte = 0x52;
		break;
	case 9600:
		cmdbyte = 0x51;
		break;
	case 19200:
		cmdbyte = 0x50;
		break;
	case 38400:
		cmdbyte = 0x4f;
		break;
	case 57600:
		cmdbyte = 0x4e;
		break;
	case 115200:
		cmdbyte = 0x4d;
		break;
	
	case 1152000:
		cmdbyte = 0;
		break;
	case 4000000:
		cmdbyte = 0;
		break;
	default: 
		cmdbyte = 0x51;
		*baud = 9600;
		break;
	}

	
	t = 10000;
	while ((read_zsreg(uap, R0) & Tx_BUF_EMP) == 0
	       || (read_zsreg(uap, R1) & ALL_SNT) == 0) {
		if (--t <= 0) {
			dev_err(&uap->dev->ofdev.dev, "transmitter didn't drain\n");
			return;
		}
		udelay(10);
	}

	
	t = 100;
	(void)read_zsdata(uap);
	(void)read_zsdata(uap);
	(void)read_zsdata(uap);
	mdelay(10);
	while (read_zsreg(uap, R0) & Rx_CH_AV) {
		read_zsdata(uap);
		mdelay(10);
		if (--t <= 0) {
			dev_err(&uap->dev->ofdev.dev, "receiver didn't drain\n");
			return;
		}
	}

	
	uap->curregs[R5] |= DTR;
	write_zsreg(uap, R5, uap->curregs[R5]);
	zssync(uap);
       	mdelay(1);

	
	pmz_convert_to_zs(uap, CS8, 0, 19200);		
	pmz_load_zsregs(uap, uap->curregs);
       	mdelay(1);

	
	write_zsdata(uap, 1);
	t = 5000;
	while ((read_zsreg(uap, R0) & Rx_CH_AV) == 0) {
		if (--t <= 0) {
			dev_err(&uap->dev->ofdev.dev,
				"irda_setup timed out on get_version byte\n");
			goto out;
		}
		udelay(10);
	}
	version = read_zsdata(uap);

	if (version < 4) {
		dev_info(&uap->dev->ofdev.dev, "IrDA: dongle version %d not supported\n",
			 version);
		goto out;
	}

	
	write_zsdata(uap, cmdbyte);
	t = 5000;
	while ((read_zsreg(uap, R0) & Rx_CH_AV) == 0) {
		if (--t <= 0) {
			dev_err(&uap->dev->ofdev.dev,
				"irda_setup timed out on speed mode byte\n");
			goto out;
		}
		udelay(10);
	}
	t = read_zsdata(uap);
	if (t != cmdbyte)
		dev_err(&uap->dev->ofdev.dev,
			"irda_setup speed mode byte = %x (%x)\n", t, cmdbyte);

	dev_info(&uap->dev->ofdev.dev, "IrDA setup for %ld bps, dongle version: %d\n",
		 *baud, version);

	(void)read_zsdata(uap);
	(void)read_zsdata(uap);
	(void)read_zsdata(uap);

 out:
	
	uap->curregs[R5] &= ~DTR;
	write_zsreg(uap, R5, uap->curregs[R5]);
	zssync(uap);

	(void)read_zsdata(uap);
	(void)read_zsdata(uap);
	(void)read_zsdata(uap);
}


static void __pmz_set_termios(struct uart_port *port, struct ktermios *termios,
			      struct ktermios *old)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned long baud;

	pmz_debug("pmz: set_termios()\n");

	if (ZS_IS_ASLEEP(uap))
		return;

	memcpy(&uap->termios_cache, termios, sizeof(struct ktermios));

	
	if (ZS_IS_IRDA(uap)) {
		
		baud = uart_get_baud_rate(port, termios, old, 1200, 4000000);
		pmz_debug("pmz: switch IRDA to %ld bauds\n", baud);
		
		pmz_irda_setup(uap, &baud);
		
		pmz_convert_to_zs(uap, termios->c_cflag, termios->c_iflag, baud);
		pmz_load_zsregs(uap, uap->curregs);
		zssync(uap);
	} else {
		baud = uart_get_baud_rate(port, termios, old, 1200, 230400);
		pmz_convert_to_zs(uap, termios->c_cflag, termios->c_iflag, baud);
		
		if (UART_ENABLE_MS(&uap->port, termios->c_cflag)) {
			uap->curregs[R15] |= DCDIE | SYNCIE | CTSIE;
			uap->flags |= PMACZILOG_FLAG_MODEM_STATUS;
		} else {
			uap->curregs[R15] &= ~(DCDIE | SYNCIE | CTSIE);
			uap->flags &= ~PMACZILOG_FLAG_MODEM_STATUS;
		}

		
		pmz_maybe_update_regs(uap);
	}
	uart_update_timeout(port, termios->c_cflag, baud);

	pmz_debug("pmz: set_termios() done.\n");
}


static void pmz_set_termios(struct uart_port *port, struct ktermios *termios,
			    struct ktermios *old)
{
	struct uart_pmac_port *uap = to_pmz(port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);	

	
	uap->curregs[R1] &= ~(EXT_INT_ENAB | TxINT_ENAB | RxINT_MASK);
	write_zsreg(uap, R1, uap->curregs[R1]);

	
	__pmz_set_termios(port, termios, old);

	
	if (ZS_IS_OPEN(uap)) {
		uap->curregs[R1] |= INT_ALL_Rx | TxINT_ENAB;
		if (!ZS_IS_EXTCLK(uap))
			uap->curregs[R1] |= EXT_INT_ENAB;
		write_zsreg(uap, R1, uap->curregs[R1]);
	}
	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *pmz_type(struct uart_port *port)
{
	struct uart_pmac_port *uap = to_pmz(port);

	if (ZS_IS_IRDA(uap))
		return "Z85c30 ESCC - Infrared port";
	else if (ZS_IS_INTMODEM(uap))
		return "Z85c30 ESCC - Internal modem";
	return "Z85c30 ESCC - Serial port";
}


static void pmz_release_port(struct uart_port *port)
{
}

static int pmz_request_port(struct uart_port *port)
{
	return 0;
}


static void pmz_config_port(struct uart_port *port, int flags)
{
}


static int pmz_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return -EINVAL;
}

#ifdef CONFIG_CONSOLE_POLL

static int pmz_poll_get_char(struct uart_port *port)
{
	struct uart_pmac_port *uap = (struct uart_pmac_port *)port;

	while ((read_zsreg(uap, R0) & Rx_CH_AV) == 0)
		udelay(5);
	return read_zsdata(uap);
}

static void pmz_poll_put_char(struct uart_port *port, unsigned char c)
{
	struct uart_pmac_port *uap = (struct uart_pmac_port *)port;

	
	while ((read_zsreg(uap, R0) & Tx_BUF_EMP) == 0)
		udelay(5);
	write_zsdata(uap, c);
}

#endif

static struct uart_ops pmz_pops = {
	.tx_empty	=	pmz_tx_empty,
	.set_mctrl	=	pmz_set_mctrl,
	.get_mctrl	=	pmz_get_mctrl,
	.stop_tx	=	pmz_stop_tx,
	.start_tx	=	pmz_start_tx,
	.stop_rx	=	pmz_stop_rx,
	.enable_ms	=	pmz_enable_ms,
	.break_ctl	=	pmz_break_ctl,
	.startup	=	pmz_startup,
	.shutdown	=	pmz_shutdown,
	.set_termios	=	pmz_set_termios,
	.type		=	pmz_type,
	.release_port	=	pmz_release_port,
	.request_port	=	pmz_request_port,
	.config_port	=	pmz_config_port,
	.verify_port	=	pmz_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char	=	pmz_poll_get_char,
	.poll_put_char	=	pmz_poll_put_char,
#endif
};


static int __init pmz_init_port(struct uart_pmac_port *uap)
{
	struct device_node *np = uap->node;
	const char *conn;
	const struct slot_names_prop {
		int	count;
		char	name[1];
	} *slots;
	int len;
	struct resource r_ports, r_rxdma, r_txdma;

	
	if (of_address_to_resource(np, 0, &r_ports))
		return -ENODEV;
	uap->port.mapbase = r_ports.start;
	uap->port.membase = ioremap(uap->port.mapbase, 0x1000);
      
	uap->control_reg = uap->port.membase;
	uap->data_reg = uap->control_reg + 0x10;
	
	
#ifdef HAS_DBDMA
	if (of_address_to_resource(np, 1, &r_txdma) == 0 &&
	    of_address_to_resource(np, 2, &r_rxdma) == 0)
		uap->flags |= PMACZILOG_FLAG_HAS_DMA;
#else
	memset(&r_txdma, 0, sizeof(struct resource));
	memset(&r_rxdma, 0, sizeof(struct resource));
#endif	
	if (ZS_HAS_DMA(uap)) {
		uap->tx_dma_regs = ioremap(r_txdma.start, 0x100);
		if (uap->tx_dma_regs == NULL) {	
			uap->flags &= ~PMACZILOG_FLAG_HAS_DMA;
			goto no_dma;
		}
		uap->rx_dma_regs = ioremap(r_rxdma.start, 0x100);
		if (uap->rx_dma_regs == NULL) {	
			iounmap(uap->tx_dma_regs);
			uap->tx_dma_regs = NULL;
			uap->flags &= ~PMACZILOG_FLAG_HAS_DMA;
			goto no_dma;
		}
		uap->tx_dma_irq = irq_of_parse_and_map(np, 1);
		uap->rx_dma_irq = irq_of_parse_and_map(np, 2);
	}
no_dma:

	
	if (of_device_is_compatible(np, "cobalt"))
		uap->flags |= PMACZILOG_FLAG_IS_INTMODEM;
	conn = of_get_property(np, "AAPL,connector", &len);
	if (conn && (strcmp(conn, "infrared") == 0))
		uap->flags |= PMACZILOG_FLAG_IS_IRDA;
	uap->port_type = PMAC_SCC_ASYNC;
	
	slots = of_get_property(np, "slot-names", &len);
	if (slots && slots->count > 0) {
		if (strcmp(slots->name, "IrDA") == 0)
			uap->flags |= PMACZILOG_FLAG_IS_IRDA;
		else if (strcmp(slots->name, "Modem") == 0)
			uap->flags |= PMACZILOG_FLAG_IS_INTMODEM;
	}
	if (ZS_IS_IRDA(uap))
		uap->port_type = PMAC_SCC_IRDA;
	if (ZS_IS_INTMODEM(uap)) {
		struct device_node* i2c_modem =
			of_find_node_by_name(NULL, "i2c-modem");
		if (i2c_modem) {
			const char* mid =
				of_get_property(i2c_modem, "modem-id", NULL);
			if (mid) switch(*mid) {
			case 0x04 :
			case 0x05 :
			case 0x07 :
			case 0x08 :
			case 0x0b :
			case 0x0c :
				uap->port_type = PMAC_SCC_I2S1;
			}
			printk(KERN_INFO "pmac_zilog: i2c-modem detected, id: %d\n",
				mid ? (*mid) : 0);
			of_node_put(i2c_modem);
		} else {
			printk(KERN_INFO "pmac_zilog: serial modem detected\n");
		}
	}

	
	uap->port.iotype = UPIO_MEM;
	uap->port.irq = irq_of_parse_and_map(np, 0);
	uap->port.uartclk = ZS_CLOCK;
	uap->port.fifosize = 1;
	uap->port.ops = &pmz_pops;
	uap->port.type = PORT_PMAC_ZILOG;
	uap->port.flags = 0;

	
	if (uap->port.irq == NO_IRQ &&
	    np->parent && np->parent->parent &&
	    of_device_is_compatible(np->parent->parent, "gatwick")) {
		
		uap->port.irq = irq_create_mapping(NULL, 64 + 15);
		uap->tx_dma_irq = irq_create_mapping(NULL, 64 + 4);
		uap->rx_dma_irq = irq_create_mapping(NULL, 64 + 5);
	}

	
	pmz_convert_to_zs(uap, CS8, 0, 9600);

	return 0;
}


static void pmz_dispose_port(struct uart_pmac_port *uap)
{
	struct device_node *np;

	np = uap->node;
	iounmap(uap->rx_dma_regs);
	iounmap(uap->tx_dma_regs);
	iounmap(uap->control_reg);
	uap->node = NULL;
	of_node_put(np);
	memset(uap, 0, sizeof(struct uart_pmac_port));
}


static int pmz_attach(struct macio_dev *mdev, const struct of_device_id *match)
{
	int i;
	
	
	for (i = 0; i < MAX_ZS_PORTS; i++)
		if (pmz_ports[i].node == mdev->ofdev.node) {
			struct uart_pmac_port *uap = &pmz_ports[i];

			uap->dev = mdev;
			dev_set_drvdata(&mdev->ofdev.dev, uap);
			if (macio_request_resources(uap->dev, "pmac_zilog"))
				printk(KERN_WARNING "%s: Failed to request resource"
				       ", port still active\n",
				       uap->node->name);
			else
				uap->flags |= PMACZILOG_FLAG_RSRC_REQUESTED;				
			return 0;
		}
	return -ENODEV;
}


static int pmz_detach(struct macio_dev *mdev)
{
	struct uart_pmac_port	*uap = dev_get_drvdata(&mdev->ofdev.dev);
	
	if (!uap)
		return -ENODEV;

	if (uap->flags & PMACZILOG_FLAG_RSRC_REQUESTED) {
		macio_release_resources(uap->dev);
		uap->flags &= ~PMACZILOG_FLAG_RSRC_REQUESTED;
	}
	dev_set_drvdata(&mdev->ofdev.dev, NULL);
	uap->dev = NULL;
	
	return 0;
}


static int pmz_suspend(struct macio_dev *mdev, pm_message_t pm_state)
{
	struct uart_pmac_port *uap = dev_get_drvdata(&mdev->ofdev.dev);
	struct uart_state *state;
	unsigned long flags;

	if (uap == NULL) {
		printk("HRM... pmz_suspend with NULL uap\n");
		return 0;
	}

	if (pm_state.event == mdev->ofdev.dev.power.power_state.event)
		return 0;

	pmz_debug("suspend, switching to state %d\n", pm_state.event);

	state = pmz_uart_reg.state + uap->port.line;

	mutex_lock(&pmz_irq_mutex);
	mutex_lock(&state->port.mutex);

	spin_lock_irqsave(&uap->port.lock, flags);

	if (ZS_IS_OPEN(uap) || ZS_IS_CONS(uap)) {
		
		uap->curregs[R3] &= ~RxENABLE;
		uap->curregs[R5] &= ~TxENABLE;

		
		uap->curregs[R1] &= ~(EXT_INT_ENAB | TxINT_ENAB | RxINT_MASK);
		uap->curregs[R5] &= ~SND_BRK;
		pmz_load_zsregs(uap, uap->curregs);
		uap->flags |= PMACZILOG_FLAG_IS_ASLEEP;
		mb();
	}

	spin_unlock_irqrestore(&uap->port.lock, flags);

	if (ZS_IS_OPEN(uap) || ZS_IS_OPEN(uap->mate))
		if (ZS_IS_ASLEEP(uap->mate) && ZS_IS_IRQ_ON(pmz_get_port_A(uap))) {
			pmz_get_port_A(uap)->flags &= ~PMACZILOG_FLAG_IS_IRQ_ON;
			disable_irq(uap->port.irq);
		}

	if (ZS_IS_CONS(uap))
		uap->port.cons->flags &= ~CON_ENABLED;

	
	pmz_set_scc_power(uap, 0);

	mutex_unlock(&state->port.mutex);
	mutex_unlock(&pmz_irq_mutex);

	pmz_debug("suspend, switching complete\n");

	mdev->ofdev.dev.power.power_state = pm_state;

	return 0;
}


static int pmz_resume(struct macio_dev *mdev)
{
	struct uart_pmac_port *uap = dev_get_drvdata(&mdev->ofdev.dev);
	struct uart_state *state;
	unsigned long flags;
	int pwr_delay = 0;

	if (uap == NULL)
		return 0;

	if (mdev->ofdev.dev.power.power_state.event == PM_EVENT_ON)
		return 0;
	
	pmz_debug("resume, switching to state 0\n");

	state = pmz_uart_reg.state + uap->port.line;

	mutex_lock(&pmz_irq_mutex);
	mutex_lock(&state->port.mutex);

	spin_lock_irqsave(&uap->port.lock, flags);
	if (!ZS_IS_OPEN(uap) && !ZS_IS_CONS(uap)) {
		spin_unlock_irqrestore(&uap->port.lock, flags);
		goto bail;
	}
	pwr_delay = __pmz_startup(uap);

	
	__pmz_set_termios(&uap->port, &uap->termios_cache, NULL);

	if (ZS_IS_OPEN(uap)) {
				
		uap->curregs[R1] |= INT_ALL_Rx | TxINT_ENAB;
		if (!ZS_IS_EXTCLK(uap))
			uap->curregs[R1] |= EXT_INT_ENAB;
		write_zsreg(uap, R1, uap->curregs[R1]);
	}

	spin_unlock_irqrestore(&uap->port.lock, flags);

	if (ZS_IS_CONS(uap))
		uap->port.cons->flags |= CON_ENABLED;

	
	if (ZS_IS_OPEN(uap) && !ZS_IS_IRQ_ON(pmz_get_port_A(uap))) {
		pmz_get_port_A(uap)->flags |= PMACZILOG_FLAG_IS_IRQ_ON;
		enable_irq(uap->port.irq);
	}

 bail:
	mutex_unlock(&state->port.mutex);
	mutex_unlock(&pmz_irq_mutex);

	
	if (pwr_delay != 0) {
		pmz_debug("pmz: delaying %d ms\n", pwr_delay);
		msleep(pwr_delay);
	}

	pmz_debug("resume, switching complete\n");

	mdev->ofdev.dev.power.power_state.event = PM_EVENT_ON;

	return 0;
}


static int __init pmz_probe(void)
{
	struct device_node	*node_p, *node_a, *node_b, *np;
	int			count = 0;
	int			rc;

	
	node_p = of_find_node_by_name(NULL, "escc");
	while (node_p) {
		
		node_a = node_b = NULL;
		for (np = NULL; (np = of_get_next_child(node_p, np)) != NULL;) {
			if (strncmp(np->name, "ch-a", 4) == 0)
				node_a = of_node_get(np);
			else if (strncmp(np->name, "ch-b", 4) == 0)
				node_b = of_node_get(np);
		}
		if (!node_a && !node_b) {
			of_node_put(node_a);
			of_node_put(node_b);
			printk(KERN_ERR "pmac_zilog: missing node %c for escc %s\n",
				(!node_a) ? 'a' : 'b', node_p->full_name);
			goto next;
		}

		
		pmz_ports[count].mate		= &pmz_ports[count+1];
		pmz_ports[count+1].mate		= &pmz_ports[count];
		pmz_ports[count].flags		= PMACZILOG_FLAG_IS_CHANNEL_A;
		pmz_ports[count].node		= node_a;
		pmz_ports[count+1].node		= node_b;
		pmz_ports[count].port.line	= count;
		pmz_ports[count+1].port.line   	= count+1;

		
		rc = pmz_init_port(&pmz_ports[count]);
		if (rc == 0 && node_b != NULL)
			rc = pmz_init_port(&pmz_ports[count+1]);
		if (rc != 0) {
			of_node_put(node_a);
			of_node_put(node_b);
			memset(&pmz_ports[count], 0, sizeof(struct uart_pmac_port));
			memset(&pmz_ports[count+1], 0, sizeof(struct uart_pmac_port));
			goto next;
		}
		count += 2;
next:
		node_p = of_find_node_by_name(node_p, "escc");
	}
	pmz_ports_count = count;

	return 0;
}

#ifdef CONFIG_SERIAL_PMACZILOG_CONSOLE

static void pmz_console_write(struct console *con, const char *s, unsigned int count);
static int __init pmz_console_setup(struct console *co, char *options);

static struct console pmz_console = {
	.name	=	PMACZILOG_NAME,
	.write	=	pmz_console_write,
	.device	=	uart_console_device,
	.setup	=	pmz_console_setup,
	.flags	=	CON_PRINTBUFFER,
	.index	=	-1,
	.data   =	&pmz_uart_reg,
};

#define PMACZILOG_CONSOLE	&pmz_console
#else 
#define PMACZILOG_CONSOLE	(NULL)
#endif 


static int __init pmz_register(void)
{
	int i, rc;
	
	pmz_uart_reg.nr = pmz_ports_count;
	pmz_uart_reg.cons = PMACZILOG_CONSOLE;

	
	rc = uart_register_driver(&pmz_uart_reg);
	if (rc)
		return rc;

	
	for (i = 0; i < pmz_ports_count; i++) {
		struct uart_pmac_port *uport = &pmz_ports[i];
		
		if (uport->node != NULL)
			rc = uart_add_one_port(&pmz_uart_reg, &uport->port);
		if (rc)
			goto err_out;
	}

	return 0;
err_out:
	while (i-- > 0) {
		struct uart_pmac_port *uport = &pmz_ports[i];
		uart_remove_one_port(&pmz_uart_reg, &uport->port);
	}
	uart_unregister_driver(&pmz_uart_reg);
	return rc;
}

static struct of_device_id pmz_match[] = 
{
	{
	.name 		= "ch-a",
	},
	{
	.name 		= "ch-b",
	},
	{},
};
MODULE_DEVICE_TABLE (of, pmz_match);

static struct macio_driver pmz_driver = 
{
	.name 		= "pmac_zilog",
	.match_table	= pmz_match,
	.probe		= pmz_attach,
	.remove		= pmz_detach,
	.suspend	= pmz_suspend,
       	.resume		= pmz_resume,
};

static int __init init_pmz(void)
{
	int rc, i;
	printk(KERN_INFO "%s\n", version);

	
	if (pmz_ports_count == 0)
		pmz_probe();

	
	if (pmz_ports_count == 0)
		return -ENODEV;

	
	rc = pmz_register();
	if (rc) {
		printk(KERN_ERR 
			"pmac_zilog: Error registering serial device, disabling pmac_zilog.\n"
		 	"pmac_zilog: Did another serial driver already claim the minors?\n"); 
		
		for (i=0; i < pmz_ports_count; i++)
			pmz_dispose_port(&pmz_ports[i]);
		return rc;
	}
	
	
	return macio_register_driver(&pmz_driver);
}

static void __exit exit_pmz(void)
{
	int i;

	
	macio_unregister_driver(&pmz_driver);

	for (i = 0; i < pmz_ports_count; i++) {
		struct uart_pmac_port *uport = &pmz_ports[i];
		if (uport->node != NULL) {
			uart_remove_one_port(&pmz_uart_reg, &uport->port);
			pmz_dispose_port(uport);
		}
	}
	
	uart_unregister_driver(&pmz_uart_reg);
}

#ifdef CONFIG_SERIAL_PMACZILOG_CONSOLE

static void pmz_console_putchar(struct uart_port *port, int ch)
{
	struct uart_pmac_port *uap = (struct uart_pmac_port *)port;

	
	while ((read_zsreg(uap, R0) & Tx_BUF_EMP) == 0)
		udelay(5);
	write_zsdata(uap, ch);
}


static void pmz_console_write(struct console *con, const char *s, unsigned int count)
{
	struct uart_pmac_port *uap = &pmz_ports[con->index];
	unsigned long flags;

	if (ZS_IS_ASLEEP(uap))
		return;
	spin_lock_irqsave(&uap->port.lock, flags);

	
	write_zsreg(uap, R1, uap->curregs[1] & ~TxINT_ENAB);
	write_zsreg(uap, R5, uap->curregs[5] | TxENABLE | RTS | DTR);

	uart_console_write(&uap->port, s, count, pmz_console_putchar);

	
	write_zsreg(uap, R1, uap->curregs[1]);
	

	spin_unlock_irqrestore(&uap->port.lock, flags);
}


static int __init pmz_console_setup(struct console *co, char *options)
{
	struct uart_pmac_port *uap;
	struct uart_port *port;
	int baud = 38400;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	unsigned long pwr_delay;

	
	if (machine_is_compatible("RackMac1,1")
	    || machine_is_compatible("RackMac1,2")
	    || machine_is_compatible("MacRISC4"))
	 	baud = 57600;

	
	if (co->index >= pmz_ports_count)
		co->index = 0;
	uap = &pmz_ports[co->index];
	if (uap->node == NULL)
		return -ENODEV;
	port = &uap->port;

	
	uap->flags |= PMACZILOG_FLAG_IS_CONS;

	
	spin_lock_init(&port->lock);

	
	pwr_delay = __pmz_startup(uap);
	if (pwr_delay)
		mdelay(pwr_delay);
	
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static int __init pmz_console_init(void)
{
	
	pmz_probe();

	
	
	register_console(&pmz_console);

	return 0;

}
console_initcall(pmz_console_init);
#endif 

module_init(init_pmz);
module_exit(exit_pmz);
