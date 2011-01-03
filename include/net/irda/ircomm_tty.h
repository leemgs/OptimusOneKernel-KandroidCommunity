

#ifndef IRCOMM_TTY_H
#define IRCOMM_TTY_H

#include <linux/serial.h>
#include <linux/termios.h>
#include <linux/timer.h>
#include <linux/tty.h>		

#include <net/irda/irias_object.h>
#include <net/irda/ircomm_core.h>
#include <net/irda/ircomm_param.h>

#define IRCOMM_TTY_PORTS 32
#define IRCOMM_TTY_MAGIC 0x3432
#define IRCOMM_TTY_MAJOR 161
#define IRCOMM_TTY_MINOR 0


#define IRCOMM_TTY_HDR_UNINITIALISED	16

#define IRCOMM_TTY_DATA_UNINITIALISED	(64 - IRCOMM_TTY_HDR_UNINITIALISED)


#define ASYNC_B_INITIALIZED	31	
#define ASYNC_B_NORMAL_ACTIVE	29	
#define ASYNC_B_CLOSING		27	


struct ircomm_tty_cb {
	irda_queue_t queue;            
	magic_t magic;

	int state;                

	struct tty_struct *tty;
	struct ircomm_cb *ircomm; 

	struct sk_buff *tx_skb;   
	struct sk_buff *ctrl_skb; 

	
	struct ircomm_params settings;

	__u8 service_type;        
	int client;               
	LOCAL_FLOW flow;          

	int line;
	unsigned long flags;

	__u8 dlsap_sel;
	__u8 slsap_sel;

	__u32 saddr;
	__u32 daddr;

	__u32 max_data_size;   
	__u32 max_header_size; 
	__u32 tx_data_size;	

	struct iriap_cb *iriap; 
	struct ias_object* obj;
	void *skey;
	void *ckey;

	wait_queue_head_t open_wait;
	wait_queue_head_t close_wait;
	struct timer_list watchdog_timer;
	struct work_struct  tqueue;

        unsigned short    close_delay;
        unsigned short    closing_wait; 

	int  open_count;
	int  blocked_open;	

	
	spinlock_t spinlock;
};

void ircomm_tty_start(struct tty_struct *tty);
void ircomm_tty_check_modem_status(struct ircomm_tty_cb *self);

extern int ircomm_tty_tiocmget(struct tty_struct *tty, struct file *file);
extern int ircomm_tty_tiocmset(struct tty_struct *tty, struct file *file,
			       unsigned int set, unsigned int clear);
extern int ircomm_tty_ioctl(struct tty_struct *tty, struct file *file, 
			    unsigned int cmd, unsigned long arg);
extern void ircomm_tty_set_termios(struct tty_struct *tty, 
				   struct ktermios *old_termios);

#endif







