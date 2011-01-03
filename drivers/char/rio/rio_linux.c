


#include <linux/module.h>
#include <linux/kdev_t.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/mm.h>
#include <linux/serial.h>
#include <linux/fcntl.h>
#include <linux/major.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/miscdevice.h>
#include <linux/init.h>

#include <linux/generic_serial.h>
#include <asm/uaccess.h>

#include "linux_compat.h"
#include "pkt.h"
#include "daemon.h"
#include "rio.h"
#include "riospace.h"
#include "cmdpkt.h"
#include "map.h"
#include "rup.h"
#include "port.h"
#include "riodrvr.h"
#include "rioinfo.h"
#include "func.h"
#include "errors.h"
#include "pci.h"

#include "parmmap.h"
#include "unixrup.h"
#include "board.h"
#include "host.h"
#include "phb.h"
#include "link.h"
#include "cmdblk.h"
#include "route.h"
#include "cirrus.h"
#include "rioioctl.h"
#include "param.h"
#include "protsts.h"
#include "rioboard.h"


#include "rio_linux.h"



#ifndef RIO_NORMAL_MAJOR0

#define RIO_NORMAL_MAJOR0  154
#define RIO_NORMAL_MAJOR1  156
#endif

#ifndef PCI_DEVICE_ID_SPECIALIX_SX_XIO_IO8
#define PCI_DEVICE_ID_SPECIALIX_SX_XIO_IO8 0x2000
#endif

#ifndef RIO_WINDOW_LEN
#define RIO_WINDOW_LEN 0x10000
#endif





#undef RIO_PARANOIA_CHECK



#define IRQ_RATE_LIMIT 200



static struct Conf
 RIOConf = {
	 "RIO Config here",
					 HZ * 2,
					
				 0,
				
				 1,
				
				 25,
				
				 10,
				
	 0x7000,
	 0x7C00,
				 5,
				
				 120,
				
				 "\033d#",
				
				 "\024",
				
				 2000,
				
				 10,
				
				 1,
				
					 0x0A0000,
					
					 0xFF0000,
					
				 1024,
				
				 256,
				
				 80,
				
				 HZ,
				
};






static void rio_disable_tx_interrupts(void *ptr);
static void rio_enable_tx_interrupts(void *ptr);
static void rio_disable_rx_interrupts(void *ptr);
static void rio_enable_rx_interrupts(void *ptr);
static int rio_carrier_raised(struct tty_port *port);
static void rio_shutdown_port(void *ptr);
static int rio_set_real_termios(void *ptr);
static void rio_hungup(void *ptr);
static void rio_close(void *ptr);
static int rio_chars_in_buffer(void *ptr);
static long rio_fw_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int rio_init_drivers(void);

static void my_hd(void *addr, int len);

static struct tty_driver *rio_driver, *rio_driver2;


struct rio_info *p;

int rio_debug;



static int rio_poll = 1;



static int rio_probe_addrs[] = { 0xc0000, 0xd0000, 0xe0000 };

#define NR_RIO_ADDRS ARRAY_SIZE(rio_probe_addrs)



static long rio_irqmask = -1;

MODULE_AUTHOR("Rogier Wolff <R.E.Wolff@bitwizard.nl>, Patrick van de Lageweg <patrick@bitwizard.nl>");
MODULE_DESCRIPTION("RIO driver");
MODULE_LICENSE("GPL");
module_param(rio_poll, int, 0);
module_param(rio_debug, int, 0644);
module_param(rio_irqmask, long, 0);

static struct real_driver rio_real_driver = {
	rio_disable_tx_interrupts,
	rio_enable_tx_interrupts,
	rio_disable_rx_interrupts,
	rio_enable_rx_interrupts,
	rio_shutdown_port,
	rio_set_real_termios,
	rio_chars_in_buffer,
	rio_close,
	rio_hungup,
	NULL
};



static const struct file_operations rio_fw_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rio_fw_ioctl,
};

static struct miscdevice rio_fw_device = {
	RIOCTL_MISC_MINOR, "rioctl", &rio_fw_fops
};





#ifdef RIO_PARANOIA_CHECK



static inline int rio_paranoia_check(struct rio_port const *port, char *name, const char *routine)
{

	static const char *badmagic = KERN_ERR "rio: Warning: bad rio port magic number for device %s in %s\n";
	static const char *badinfo = KERN_ERR "rio: Warning: null rio port for device %s in %s\n";

	if (!port) {
		printk(badinfo, name, routine);
		return 1;
	}
	if (port->magic != RIO_MAGIC) {
		printk(badmagic, name, routine);
		return 1;
	}

	return 0;
}
#else
#define rio_paranoia_check(a,b,c) 0
#endif


#ifdef DEBUG
static void my_hd(void *ad, int len)
{
	int i, j, ch;
	unsigned char *addr = ad;

	for (i = 0; i < len; i += 16) {
		rio_dprintk(RIO_DEBUG_PARAM, "%08lx ", (unsigned long) addr + i);
		for (j = 0; j < 16; j++) {
			rio_dprintk(RIO_DEBUG_PARAM, "%02x %s", addr[j + i], (j == 7) ? " " : "");
		}
		for (j = 0; j < 16; j++) {
			ch = addr[j + i];
			rio_dprintk(RIO_DEBUG_PARAM, "%c", (ch < 0x20) ? '.' : ((ch > 0x7f) ? '.' : ch));
		}
		rio_dprintk(RIO_DEBUG_PARAM, "\n");
	}
}
#else
#define my_hd(ad,len) do{ } while (0)
#endif



int RIODelay(struct Port *PortP, int njiffies)
{
	func_enter();

	rio_dprintk(RIO_DEBUG_DELAY, "delaying %d jiffies\n", njiffies);
	msleep_interruptible(jiffies_to_msecs(njiffies));
	func_exit();

	if (signal_pending(current))
		return RIO_FAIL;
	else
		return !RIO_FAIL;
}



int RIODelay_ni(struct Port *PortP, int njiffies)
{
	func_enter();

	rio_dprintk(RIO_DEBUG_DELAY, "delaying %d jiffies (ni)\n", njiffies);
	msleep(jiffies_to_msecs(njiffies));
	func_exit();
	return !RIO_FAIL;
}

void rio_copy_to_card(void *from, void __iomem *to, int len)
{
	rio_copy_toio(to, from, len);
}

int rio_minor(struct tty_struct *tty)
{
	return tty->index + ((tty->driver == rio_driver) ? 0 : 256);
}

static int rio_set_real_termios(void *ptr)
{
	return RIOParam((struct Port *) ptr, RIOC_CONFIG, 1, 1);
}


static void rio_reset_interrupt(struct Host *HostP)
{
	func_enter();

	switch (HostP->Type) {
	case RIO_AT:
	case RIO_MCA:
	case RIO_PCI:
		writeb(0xFF, &HostP->ResetInt);
	}

	func_exit();
}


static irqreturn_t rio_interrupt(int irq, void *ptr)
{
	struct Host *HostP;
	func_enter();

	HostP = ptr;			
	rio_dprintk(RIO_DEBUG_IFLOW, "rio: enter rio_interrupt (%d/%d)\n", irq, HostP->Ivec);

	

	rio_dprintk(RIO_DEBUG_IFLOW, "rio: We've have noticed the interrupt\n");
	if (HostP->Ivec == irq) {
		
		rio_reset_interrupt(HostP);
	}

	if ((HostP->Flags & RUN_STATE) != RC_RUNNING)
		return IRQ_HANDLED;

	if (test_and_set_bit(RIO_BOARD_INTR_LOCK, &HostP->locks)) {
		printk(KERN_ERR "Recursive interrupt! (host %p/irq%d)\n", ptr, HostP->Ivec);
		return IRQ_HANDLED;
	}

	RIOServiceHost(p, HostP);

	rio_dprintk(RIO_DEBUG_IFLOW, "riointr() doing host %p type %d\n", ptr, HostP->Type);

	clear_bit(RIO_BOARD_INTR_LOCK, &HostP->locks);
	rio_dprintk(RIO_DEBUG_IFLOW, "rio: exit rio_interrupt (%d/%d)\n", irq, HostP->Ivec);
	func_exit();
	return IRQ_HANDLED;
}


static void rio_pollfunc(unsigned long data)
{
	func_enter();

	rio_interrupt(0, &p->RIOHosts[data]);
	mod_timer(&p->RIOHosts[data].timer, jiffies + rio_poll);

	func_exit();
}






static void rio_disable_tx_interrupts(void *ptr)
{
	func_enter();

	

	func_exit();
}


static void rio_enable_tx_interrupts(void *ptr)
{
	struct Port *PortP = ptr;
	

	func_enter();

	

	RIOTxEnable((char *) PortP);

	
	PortP->gs.port.flags &= ~GS_TX_INTEN;

	func_exit();
}


static void rio_disable_rx_interrupts(void *ptr)
{
	func_enter();
	func_exit();
}

static void rio_enable_rx_interrupts(void *ptr)
{
	
	func_enter();
	func_exit();
}



static int rio_carrier_raised(struct tty_port *port)
{
	struct Port *PortP = container_of(port, struct Port, gs.port);
	int rv;

	func_enter();
	rv = (PortP->ModemState & RIOC_MSVR1_CD) != 0;

	rio_dprintk(RIO_DEBUG_INIT, "Getting CD status: %d\n", rv);

	func_exit();
	return rv;
}



static int rio_chars_in_buffer(void *ptr)
{
	func_enter();

	func_exit();
	return 0;
}



static void rio_shutdown_port(void *ptr)
{
	struct Port *PortP;

	func_enter();

	PortP = (struct Port *) ptr;
	PortP->gs.port.tty = NULL;
	func_exit();
}



static void rio_hungup(void *ptr)
{
	struct Port *PortP;

	func_enter();

	PortP = (struct Port *) ptr;
	PortP->gs.port.tty = NULL;

	func_exit();
}



static void rio_close(void *ptr)
{
	struct Port *PortP;

	func_enter();

	PortP = (struct Port *) ptr;

	riotclose(ptr);

	if (PortP->gs.port.count) {
		printk(KERN_ERR "WARNING port count:%d\n", PortP->gs.port.count);
		PortP->gs.port.count = 0;
	}

	PortP->gs.port.tty = NULL;
	func_exit();
}



static long rio_fw_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	func_enter();

	
	lock_kernel();
	rc = riocontrol(p, 0, cmd, arg, capable(CAP_SYS_ADMIN));
	unlock_kernel();

	func_exit();
	return rc;
}

extern int RIOShortCommand(struct rio_info *p, struct Port *PortP, int command, int len, int arg);

static int rio_ioctl(struct tty_struct *tty, struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int rc;
	struct Port *PortP;
	int ival;

	func_enter();

	PortP = (struct Port *) tty->driver_data;

	rc = 0;
	switch (cmd) {
	case TIOCSSOFTCAR:
		if ((rc = get_user(ival, (unsigned __user *) argp)) == 0) {
			tty->termios->c_cflag = (tty->termios->c_cflag & ~CLOCAL) | (ival ? CLOCAL : 0);
		}
		break;
	case TIOCGSERIAL:
		rc = -EFAULT;
		if (access_ok(VERIFY_WRITE, argp, sizeof(struct serial_struct)))
			rc = gs_getserial(&PortP->gs, argp);
		break;
	case TCSBRK:
		if (PortP->State & RIO_DELETED) {
			rio_dprintk(RIO_DEBUG_TTY, "BREAK on deleted RTA\n");
			rc = -EIO;
		} else {
			if (RIOShortCommand(p, PortP, RIOC_SBREAK, 2, 250) ==
					RIO_FAIL) {
				rio_dprintk(RIO_DEBUG_INTR, "SBREAK RIOShortCommand failed\n");
				rc = -EIO;
			}
		}
		break;
	case TCSBRKP:
		if (PortP->State & RIO_DELETED) {
			rio_dprintk(RIO_DEBUG_TTY, "BREAK on deleted RTA\n");
			rc = -EIO;
		} else {
			int l;
			l = arg ? arg * 100 : 250;
			if (l > 255)
				l = 255;
			if (RIOShortCommand(p, PortP, RIOC_SBREAK, 2,
					arg ? arg * 100 : 250) == RIO_FAIL) {
				rio_dprintk(RIO_DEBUG_INTR, "SBREAK RIOShortCommand failed\n");
				rc = -EIO;
			}
		}
		break;
	case TIOCSSERIAL:
		rc = -EFAULT;
		if (access_ok(VERIFY_READ, argp, sizeof(struct serial_struct)))
			rc = gs_setserial(&PortP->gs, argp);
		break;
	default:
		rc = -ENOIOCTLCMD;
		break;
	}
	func_exit();
	return rc;
}




static void rio_throttle(struct tty_struct *tty)
{
	struct Port *port = (struct Port *) tty->driver_data;

	func_enter();
	

	if ((tty->termios->c_cflag & CRTSCTS) || (I_IXOFF(tty))) {
		port->State |= RIO_THROTTLE_RX;
	}

	func_exit();
}


static void rio_unthrottle(struct tty_struct *tty)
{
	struct Port *port = (struct Port *) tty->driver_data;

	func_enter();
	

	port->State &= ~RIO_THROTTLE_RX;

	func_exit();
	return;
}








static struct vpd_prom *get_VPD_PROM(struct Host *hp)
{
	static struct vpd_prom vpdp;
	char *p;
	int i;

	func_enter();
	rio_dprintk(RIO_DEBUG_PROBE, "Going to verify vpd prom at %p.\n", hp->Caddr + RIO_VPD_ROM);

	p = (char *) &vpdp;
	for (i = 0; i < sizeof(struct vpd_prom); i++)
		*p++ = readb(hp->Caddr + RIO_VPD_ROM + i * 2);
	

	
	*p++ = 0;

	if (rio_debug & RIO_DEBUG_PROBE)
		my_hd((char *) &vpdp, 0x20);

	func_exit();

	return &vpdp;
}

static const struct tty_operations rio_ops = {
	.open = riotopen,
	.close = gs_close,
	.write = gs_write,
	.put_char = gs_put_char,
	.flush_chars = gs_flush_chars,
	.write_room = gs_write_room,
	.chars_in_buffer = gs_chars_in_buffer,
	.flush_buffer = gs_flush_buffer,
	.ioctl = rio_ioctl,
	.throttle = rio_throttle,
	.unthrottle = rio_unthrottle,
	.set_termios = gs_set_termios,
	.stop = gs_stop,
	.start = gs_start,
	.hangup = gs_hangup,
};

static int rio_init_drivers(void)
{
	int error = -ENOMEM;

	rio_driver = alloc_tty_driver(256);
	if (!rio_driver)
		goto out;
	rio_driver2 = alloc_tty_driver(256);
	if (!rio_driver2)
		goto out1;

	func_enter();

	rio_driver->owner = THIS_MODULE;
	rio_driver->driver_name = "specialix_rio";
	rio_driver->name = "ttySR";
	rio_driver->major = RIO_NORMAL_MAJOR0;
	rio_driver->type = TTY_DRIVER_TYPE_SERIAL;
	rio_driver->subtype = SERIAL_TYPE_NORMAL;
	rio_driver->init_termios = tty_std_termios;
	rio_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	rio_driver->flags = TTY_DRIVER_REAL_RAW;
	tty_set_operations(rio_driver, &rio_ops);

	rio_driver2->owner = THIS_MODULE;
	rio_driver2->driver_name = "specialix_rio";
	rio_driver2->name = "ttySR";
	rio_driver2->major = RIO_NORMAL_MAJOR1;
	rio_driver2->type = TTY_DRIVER_TYPE_SERIAL;
	rio_driver2->subtype = SERIAL_TYPE_NORMAL;
	rio_driver2->init_termios = tty_std_termios;
	rio_driver2->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	rio_driver2->flags = TTY_DRIVER_REAL_RAW;
	tty_set_operations(rio_driver2, &rio_ops);

	rio_dprintk(RIO_DEBUG_INIT, "set_termios = %p\n", gs_set_termios);

	if ((error = tty_register_driver(rio_driver)))
		goto out2;
	if ((error = tty_register_driver(rio_driver2)))
		goto out3;
	func_exit();
	return 0;
      out3:
	tty_unregister_driver(rio_driver);
      out2:
	put_tty_driver(rio_driver2);
      out1:
	put_tty_driver(rio_driver);
      out:
	printk(KERN_ERR "rio: Couldn't register a rio driver, error = %d\n", error);
	return 1;
}

static const struct tty_port_operations rio_port_ops = {
	.carrier_raised = rio_carrier_raised,
};

static int rio_init_datastructures(void)
{
	int i;
	struct Port *port;
	func_enter();

	
	

#define RI_SZ   sizeof(struct rio_info)
#define HOST_SZ sizeof(struct Host)
#define PORT_SZ sizeof(struct Port *)
#define TMIO_SZ sizeof(struct termios *)
	rio_dprintk(RIO_DEBUG_INIT, "getting : %Zd %Zd %Zd %Zd %Zd bytes\n", RI_SZ, RIO_HOSTS * HOST_SZ, RIO_PORTS * PORT_SZ, RIO_PORTS * TMIO_SZ, RIO_PORTS * TMIO_SZ);

	if (!(p = kzalloc(RI_SZ, GFP_KERNEL)))
		goto free0;
	if (!(p->RIOHosts = kzalloc(RIO_HOSTS * HOST_SZ, GFP_KERNEL)))
		goto free1;
	if (!(p->RIOPortp = kzalloc(RIO_PORTS * PORT_SZ, GFP_KERNEL)))
		goto free2;
	p->RIOConf = RIOConf;
	rio_dprintk(RIO_DEBUG_INIT, "Got : %p %p %p\n", p, p->RIOHosts, p->RIOPortp);

#if 1
	for (i = 0; i < RIO_PORTS; i++) {
		port = p->RIOPortp[i] = kzalloc(sizeof(struct Port), GFP_KERNEL);
		if (!port) {
			goto free6;
		}
		rio_dprintk(RIO_DEBUG_INIT, "initing port %d (%d)\n", i, port->Mapped);
		tty_port_init(&port->gs.port);
		port->gs.port.ops = &rio_port_ops;
		port->PortNum = i;
		port->gs.magic = RIO_MAGIC;
		port->gs.close_delay = HZ / 2;
		port->gs.closing_wait = 30 * HZ;
		port->gs.rd = &rio_real_driver;
		spin_lock_init(&port->portSem);
	}
#else
	
#endif



	if (rio_debug & RIO_DEBUG_INIT) {
		my_hd(&rio_real_driver, sizeof(rio_real_driver));
	}


	func_exit();
	return 0;

      free6:for (i--; i >= 0; i--)
		kfree(p->RIOPortp[i]);
 kfree(p->RIOPortp);
      free2:kfree(p->RIOHosts);
      free1:
	rio_dprintk(RIO_DEBUG_INIT, "Not enough memory! %p %p %p\n", p, p->RIOHosts, p->RIOPortp);
	kfree(p);
      free0:
	return -ENOMEM;
}

static void __exit rio_release_drivers(void)
{
	func_enter();
	tty_unregister_driver(rio_driver2);
	tty_unregister_driver(rio_driver);
	put_tty_driver(rio_driver2);
	put_tty_driver(rio_driver);
	func_exit();
}


#ifdef CONFIG_PCI
 

 



static void fix_rio_pci(struct pci_dev *pdev)
{
	unsigned long hwbase;
	unsigned char __iomem *rebase;
	unsigned int t;

#define CNTRL_REG_OFFSET        0x50
#define CNTRL_REG_GOODVALUE     0x18260000

	hwbase = pci_resource_start(pdev, 0);
	rebase = ioremap(hwbase, 0x80);
	t = readl(rebase + CNTRL_REG_OFFSET);
	if (t != CNTRL_REG_GOODVALUE) {
		printk(KERN_DEBUG "rio: performing cntrl reg fix: %08x -> %08x\n", t, CNTRL_REG_GOODVALUE);
		writel(CNTRL_REG_GOODVALUE, rebase + CNTRL_REG_OFFSET);
	}
	iounmap(rebase);
}
#endif


static int __init rio_init(void)
{
	int found = 0;
	int i;
	struct Host *hp;
	int retval;
	struct vpd_prom *vpdp;
	int okboard;

#ifdef CONFIG_PCI
	struct pci_dev *pdev = NULL;
	unsigned short tshort;
#endif

	func_enter();
	rio_dprintk(RIO_DEBUG_INIT, "Initing rio module... (rio_debug=%d)\n", rio_debug);

	if (abs((long) (&rio_debug) - rio_debug) < 0x10000) {
		printk(KERN_WARNING "rio: rio_debug is an address, instead of a value. " "Assuming -1. Was %x/%p.\n", rio_debug, &rio_debug);
		rio_debug = -1;
	}

	if (misc_register(&rio_fw_device) < 0) {
		printk(KERN_ERR "RIO: Unable to register firmware loader driver.\n");
		return -EIO;
	}

	retval = rio_init_datastructures();
	if (retval < 0) {
		misc_deregister(&rio_fw_device);
		return retval;
	}
#ifdef CONFIG_PCI
	
	while ((pdev = pci_get_device(PCI_VENDOR_ID_SPECIALIX, PCI_DEVICE_ID_SPECIALIX_SX_XIO_IO8, pdev))) {
		u32 tint;

		if (pci_enable_device(pdev))
			continue;

		
		
		pci_read_config_dword(pdev, 0x2c, &tint);
		tshort = (tint >> 16) & 0xffff;
		rio_dprintk(RIO_DEBUG_PROBE, "Got a specialix card: %x.\n", tint);
		if (tshort != 0x0100) {
			rio_dprintk(RIO_DEBUG_PROBE, "But it's not a RIO card (%d)...\n", tshort);
			continue;
		}
		rio_dprintk(RIO_DEBUG_PROBE, "cp1\n");

		hp = &p->RIOHosts[p->RIONumHosts];
		hp->PaddrP = pci_resource_start(pdev, 2);
		hp->Ivec = pdev->irq;
		if (((1 << hp->Ivec) & rio_irqmask) == 0)
			hp->Ivec = 0;
		hp->Caddr = ioremap(p->RIOHosts[p->RIONumHosts].PaddrP, RIO_WINDOW_LEN);
		hp->CardP = (struct DpRam __iomem *) hp->Caddr;
		hp->Type = RIO_PCI;
		hp->Copy = rio_copy_to_card;
		hp->Mode = RIO_PCI_BOOT_FROM_RAM;
		spin_lock_init(&hp->HostLock);
		rio_reset_interrupt(hp);
		rio_start_card_running(hp);

		rio_dprintk(RIO_DEBUG_PROBE, "Going to test it (%p/%p).\n", (void *) p->RIOHosts[p->RIONumHosts].PaddrP, p->RIOHosts[p->RIONumHosts].Caddr);
		if (RIOBoardTest(p->RIOHosts[p->RIONumHosts].PaddrP, p->RIOHosts[p->RIONumHosts].Caddr, RIO_PCI, 0) == 0) {
			rio_dprintk(RIO_DEBUG_INIT, "Done RIOBoardTest\n");
			writeb(0xFF, &p->RIOHosts[p->RIONumHosts].ResetInt);
			p->RIOHosts[p->RIONumHosts].UniqueNum =
			    ((readb(&p->RIOHosts[p->RIONumHosts].Unique[0]) & 0xFF) << 0) |
			    ((readb(&p->RIOHosts[p->RIONumHosts].Unique[1]) & 0xFF) << 8) | ((readb(&p->RIOHosts[p->RIONumHosts].Unique[2]) & 0xFF) << 16) | ((readb(&p->RIOHosts[p->RIONumHosts].Unique[3]) & 0xFF) << 24);
			rio_dprintk(RIO_DEBUG_PROBE, "Hmm Tested ok, uniqid = %x.\n", p->RIOHosts[p->RIONumHosts].UniqueNum);

			fix_rio_pci(pdev);

			p->RIOHosts[p->RIONumHosts].pdev = pdev;
			pci_dev_get(pdev);

			p->RIOLastPCISearch = 0;
			p->RIONumHosts++;
			found++;
		} else {
			iounmap(p->RIOHosts[p->RIONumHosts].Caddr);
			p->RIOHosts[p->RIONumHosts].Caddr = NULL;
		}
	}

	

	

	
	while ((pdev = pci_get_device(PCI_VENDOR_ID_SPECIALIX, PCI_DEVICE_ID_SPECIALIX_RIO, pdev))) {
		if (pci_enable_device(pdev))
			continue;

#ifdef CONFIG_RIO_OLDPCI
		hp = &p->RIOHosts[p->RIONumHosts];
		hp->PaddrP = pci_resource_start(pdev, 0);
		hp->Ivec = pdev->irq;
		if (((1 << hp->Ivec) & rio_irqmask) == 0)
			hp->Ivec = 0;
		hp->Ivec |= 0x8000;	
		hp->Caddr = ioremap(p->RIOHosts[p->RIONumHosts].PaddrP, RIO_WINDOW_LEN);
		hp->CardP = (struct DpRam __iomem *) hp->Caddr;
		hp->Type = RIO_PCI;
		hp->Copy = rio_copy_to_card;
		hp->Mode = RIO_PCI_BOOT_FROM_RAM;
		spin_lock_init(&hp->HostLock);

		rio_dprintk(RIO_DEBUG_PROBE, "Ivec: %x\n", hp->Ivec);
		rio_dprintk(RIO_DEBUG_PROBE, "Mode: %x\n", hp->Mode);

		rio_reset_interrupt(hp);
		rio_start_card_running(hp);
		rio_dprintk(RIO_DEBUG_PROBE, "Going to test it (%p/%p).\n", (void *) p->RIOHosts[p->RIONumHosts].PaddrP, p->RIOHosts[p->RIONumHosts].Caddr);
		if (RIOBoardTest(p->RIOHosts[p->RIONumHosts].PaddrP, p->RIOHosts[p->RIONumHosts].Caddr, RIO_PCI, 0) == 0) {
			writeb(0xFF, &p->RIOHosts[p->RIONumHosts].ResetInt);
			p->RIOHosts[p->RIONumHosts].UniqueNum =
			    ((readb(&p->RIOHosts[p->RIONumHosts].Unique[0]) & 0xFF) << 0) |
			    ((readb(&p->RIOHosts[p->RIONumHosts].Unique[1]) & 0xFF) << 8) | ((readb(&p->RIOHosts[p->RIONumHosts].Unique[2]) & 0xFF) << 16) | ((readb(&p->RIOHosts[p->RIONumHosts].Unique[3]) & 0xFF) << 24);
			rio_dprintk(RIO_DEBUG_PROBE, "Hmm Tested ok, uniqid = %x.\n", p->RIOHosts[p->RIONumHosts].UniqueNum);

			p->RIOHosts[p->RIONumHosts].pdev = pdev;
			pci_dev_get(pdev);

			p->RIOLastPCISearch = 0;
			p->RIONumHosts++;
			found++;
		} else {
			iounmap(p->RIOHosts[p->RIONumHosts].Caddr);
			p->RIOHosts[p->RIONumHosts].Caddr = NULL;
		}
#else
		printk(KERN_ERR "Found an older RIO PCI card, but the driver is not " "compiled to support it.\n");
#endif
	}
#endif				

	
	for (i = 0; i < NR_RIO_ADDRS; i++) {
		hp = &p->RIOHosts[p->RIONumHosts];
		hp->PaddrP = rio_probe_addrs[i];
		
		hp->Ivec = 0;
		hp->Caddr = ioremap(p->RIOHosts[p->RIONumHosts].PaddrP, RIO_WINDOW_LEN);
		hp->CardP = (struct DpRam __iomem *) hp->Caddr;
		hp->Type = RIO_AT;
		hp->Copy = rio_copy_to_card;	
		hp->Mode = 0;
		spin_lock_init(&hp->HostLock);

		vpdp = get_VPD_PROM(hp);
		rio_dprintk(RIO_DEBUG_PROBE, "Got VPD ROM\n");
		okboard = 0;
		if ((strncmp(vpdp->identifier, RIO_ISA_IDENT, 16) == 0) || (strncmp(vpdp->identifier, RIO_ISA2_IDENT, 16) == 0) || (strncmp(vpdp->identifier, RIO_ISA3_IDENT, 16) == 0)) {
			
			if (RIOBoardTest(hp->PaddrP, hp->Caddr, RIO_AT, 0) == 0) {
				
				rio_dprintk(RIO_DEBUG_PROBE, "Hmm Tested ok, uniqid = %x.\n", p->RIOHosts[p->RIONumHosts].UniqueNum);
				if (RIOAssignAT(p, hp->PaddrP, hp->Caddr, 0)) {
					rio_dprintk(RIO_DEBUG_PROBE, "Hmm Tested ok, host%d uniqid = %x.\n", p->RIONumHosts, p->RIOHosts[p->RIONumHosts - 1].UniqueNum);
					okboard++;
					found++;
				}
			}

			if (!okboard) {
				iounmap(hp->Caddr);
				hp->Caddr = NULL;
			}
		}
	}


	for (i = 0; i < p->RIONumHosts; i++) {
		hp = &p->RIOHosts[i];
		if (hp->Ivec) {
			int mode = IRQF_SHARED;
			if (hp->Ivec & 0x8000) {
				mode = 0;
				hp->Ivec &= 0x7fff;
			}
			rio_dprintk(RIO_DEBUG_INIT, "Requesting interrupt hp: %p rio_interrupt: %d Mode: %x\n", hp, hp->Ivec, hp->Mode);
			retval = request_irq(hp->Ivec, rio_interrupt, mode, "rio", hp);
			rio_dprintk(RIO_DEBUG_INIT, "Return value from request_irq: %d\n", retval);
			if (retval) {
				printk(KERN_ERR "rio: Cannot allocate irq %d.\n", hp->Ivec);
				hp->Ivec = 0;
			}
			rio_dprintk(RIO_DEBUG_INIT, "Got irq %d.\n", hp->Ivec);
			if (hp->Ivec != 0) {
				rio_dprintk(RIO_DEBUG_INIT, "Enabling interrupts on rio card.\n");
				hp->Mode |= RIO_PCI_INT_ENABLE;
			} else
				hp->Mode &= ~RIO_PCI_INT_ENABLE;
			rio_dprintk(RIO_DEBUG_INIT, "New Mode: %x\n", hp->Mode);
			rio_start_card_running(hp);
		}
		

		setup_timer(&hp->timer, rio_pollfunc, i);
		if (!hp->Ivec) {
			rio_dprintk(RIO_DEBUG_INIT, "Starting polling at %dj intervals.\n", rio_poll);
			mod_timer(&hp->timer, jiffies + rio_poll);
		}
	}

	if (found) {
		rio_dprintk(RIO_DEBUG_INIT, "rio: total of %d boards detected.\n", found);
		rio_init_drivers();
	} else {
		
		misc_deregister(&rio_fw_device);
	}

	func_exit();
	return found ? 0 : -EIO;
}


static void __exit rio_exit(void)
{
	int i;
	struct Host *hp;

	func_enter();

	for (i = 0, hp = p->RIOHosts; i < p->RIONumHosts; i++, hp++) {
		RIOHostReset(hp->Type, hp->CardP, hp->Slot);
		if (hp->Ivec) {
			free_irq(hp->Ivec, hp);
			rio_dprintk(RIO_DEBUG_INIT, "freed irq %d.\n", hp->Ivec);
		}
		
		del_timer_sync(&hp->timer);
		if (hp->Caddr)
			iounmap(hp->Caddr);
		if (hp->Type == RIO_PCI)
			pci_dev_put(hp->pdev);
	}

	if (misc_deregister(&rio_fw_device) < 0) {
		printk(KERN_INFO "rio: couldn't deregister control-device\n");
	}


	rio_dprintk(RIO_DEBUG_CLEANUP, "Cleaning up drivers\n");

	rio_release_drivers();

	
	kfree(p->RIOPortp);
	kfree(p->RIOHosts);
	kfree(p);

	func_exit();
}

module_init(rio_init);
module_exit(rio_exit);
