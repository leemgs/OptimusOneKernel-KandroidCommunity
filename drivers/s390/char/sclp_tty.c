

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>

#include "ctrlchar.h"
#include "sclp.h"
#include "sclp_rw.h"
#include "sclp_tty.h"


#define SCLP_TTY_BUF_SIZE 512




static spinlock_t sclp_tty_lock;

static struct list_head sclp_tty_pages;

static struct list_head sclp_tty_outqueue;

static int sclp_tty_buffer_count;

static struct sclp_buffer *sclp_ttybuf;

static struct timer_list sclp_tty_timer;

static struct tty_struct *sclp_tty;
static unsigned char sclp_tty_chars[SCLP_TTY_BUF_SIZE];
static unsigned short int sclp_tty_chars_count;

struct tty_driver *sclp_tty_driver;

static int sclp_tty_tolower;
static int sclp_tty_columns = 80;

#define SPACES_PER_TAB 8
#define CASE_DELIMITER 0x6c 


static int
sclp_tty_open(struct tty_struct *tty, struct file *filp)
{
	sclp_tty = tty;
	tty->driver_data = NULL;
	tty->low_latency = 0;
	return 0;
}


static void
sclp_tty_close(struct tty_struct *tty, struct file *filp)
{
	if (tty->count > 1)
		return;
	sclp_tty = NULL;
}


static int
sclp_tty_write_room (struct tty_struct *tty)
{
	unsigned long flags;
	struct list_head *l;
	int count;

	spin_lock_irqsave(&sclp_tty_lock, flags);
	count = 0;
	if (sclp_ttybuf != NULL)
		count = sclp_buffer_space(sclp_ttybuf) / sizeof(struct mto);
	list_for_each(l, &sclp_tty_pages)
		count += NR_EMPTY_MTO_PER_SCCB;
	spin_unlock_irqrestore(&sclp_tty_lock, flags);
	return count;
}

static void
sclp_ttybuf_callback(struct sclp_buffer *buffer, int rc)
{
	unsigned long flags;
	void *page;

	do {
		page = sclp_unmake_buffer(buffer);
		spin_lock_irqsave(&sclp_tty_lock, flags);
		
		list_del(&buffer->list);
		sclp_tty_buffer_count--;
		list_add_tail((struct list_head *) page, &sclp_tty_pages);
		
		buffer = NULL;
		if (!list_empty(&sclp_tty_outqueue))
			buffer = list_entry(sclp_tty_outqueue.next,
					    struct sclp_buffer, list);
		spin_unlock_irqrestore(&sclp_tty_lock, flags);
	} while (buffer && sclp_emit_buffer(buffer, sclp_ttybuf_callback));
	
	if (sclp_tty != NULL) {
		tty_wakeup(sclp_tty);
	}
}

static inline void
__sclp_ttybuf_emit(struct sclp_buffer *buffer)
{
	unsigned long flags;
	int count;
	int rc;

	spin_lock_irqsave(&sclp_tty_lock, flags);
	list_add_tail(&buffer->list, &sclp_tty_outqueue);
	count = sclp_tty_buffer_count++;
	spin_unlock_irqrestore(&sclp_tty_lock, flags);
	if (count)
		return;
	rc = sclp_emit_buffer(buffer, sclp_ttybuf_callback);
	if (rc)
		sclp_ttybuf_callback(buffer, rc);
}


static void
sclp_tty_timeout(unsigned long data)
{
	unsigned long flags;
	struct sclp_buffer *buf;

	spin_lock_irqsave(&sclp_tty_lock, flags);
	buf = sclp_ttybuf;
	sclp_ttybuf = NULL;
	spin_unlock_irqrestore(&sclp_tty_lock, flags);

	if (buf != NULL) {
		__sclp_ttybuf_emit(buf);
	}
}


static int sclp_tty_write_string(const unsigned char *str, int count, int may_fail)
{
	unsigned long flags;
	void *page;
	int written;
	int overall_written;
	struct sclp_buffer *buf;

	if (count <= 0)
		return 0;
	overall_written = 0;
	spin_lock_irqsave(&sclp_tty_lock, flags);
	do {
		
		if (sclp_ttybuf == NULL) {
			while (list_empty(&sclp_tty_pages)) {
				spin_unlock_irqrestore(&sclp_tty_lock, flags);
				if (may_fail)
					goto out;
				else
					sclp_sync_wait();
				spin_lock_irqsave(&sclp_tty_lock, flags);
			}
			page = sclp_tty_pages.next;
			list_del((struct list_head *) page);
			sclp_ttybuf = sclp_make_buffer(page, sclp_tty_columns,
						       SPACES_PER_TAB);
		}
		
		written = sclp_write(sclp_ttybuf, str, count);
		overall_written += written;
		if (written == count)
			break;
		
		buf = sclp_ttybuf;
		sclp_ttybuf = NULL;
		spin_unlock_irqrestore(&sclp_tty_lock, flags);
		__sclp_ttybuf_emit(buf);
		spin_lock_irqsave(&sclp_tty_lock, flags);
		str += written;
		count -= written;
	} while (count > 0);
	
	if (sclp_ttybuf && sclp_chars_in_buffer(sclp_ttybuf) &&
	    !timer_pending(&sclp_tty_timer)) {
		init_timer(&sclp_tty_timer);
		sclp_tty_timer.function = sclp_tty_timeout;
		sclp_tty_timer.data = 0UL;
		sclp_tty_timer.expires = jiffies + HZ/10;
		add_timer(&sclp_tty_timer);
	}
	spin_unlock_irqrestore(&sclp_tty_lock, flags);
out:
	return overall_written;
}


static int
sclp_tty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	if (sclp_tty_chars_count > 0) {
		sclp_tty_write_string(sclp_tty_chars, sclp_tty_chars_count, 0);
		sclp_tty_chars_count = 0;
	}
	return sclp_tty_write_string(buf, count, 1);
}


static int
sclp_tty_put_char(struct tty_struct *tty, unsigned char ch)
{
	sclp_tty_chars[sclp_tty_chars_count++] = ch;
	if (ch == '\n' || sclp_tty_chars_count >= SCLP_TTY_BUF_SIZE) {
		sclp_tty_write_string(sclp_tty_chars, sclp_tty_chars_count, 0);
		sclp_tty_chars_count = 0;
	}
	return 1;
}


static void
sclp_tty_flush_chars(struct tty_struct *tty)
{
	if (sclp_tty_chars_count > 0) {
		sclp_tty_write_string(sclp_tty_chars, sclp_tty_chars_count, 0);
		sclp_tty_chars_count = 0;
	}
}


static int
sclp_tty_chars_in_buffer(struct tty_struct *tty)
{
	unsigned long flags;
	struct list_head *l;
	struct sclp_buffer *t;
	int count;

	spin_lock_irqsave(&sclp_tty_lock, flags);
	count = 0;
	if (sclp_ttybuf != NULL)
		count = sclp_chars_in_buffer(sclp_ttybuf);
	list_for_each(l, &sclp_tty_outqueue) {
		t = list_entry(l, struct sclp_buffer, list);
		count += sclp_chars_in_buffer(t);
	}
	spin_unlock_irqrestore(&sclp_tty_lock, flags);
	return count;
}


static void
sclp_tty_flush_buffer(struct tty_struct *tty)
{
	if (sclp_tty_chars_count > 0) {
		sclp_tty_write_string(sclp_tty_chars, sclp_tty_chars_count, 0);
		sclp_tty_chars_count = 0;
	}
}


static void
sclp_tty_input(unsigned char* buf, unsigned int count)
{
	unsigned int cchar;

	
	if (sclp_tty == NULL)
		return;
	cchar = ctrlchar_handle(buf, count, sclp_tty);
	switch (cchar & CTRLCHAR_MASK) {
	case CTRLCHAR_SYSRQ:
		break;
	case CTRLCHAR_CTRL:
		tty_insert_flip_char(sclp_tty, cchar, TTY_NORMAL);
		tty_flip_buffer_push(sclp_tty);
		break;
	case CTRLCHAR_NONE:
		
		if (count < 2 ||
		    (strncmp((const char *) buf + count - 2, "^n", 2) &&
		     strncmp((const char *) buf + count - 2, "\252n", 2))) {
			
			tty_insert_flip_string(sclp_tty, buf, count);
			tty_insert_flip_char(sclp_tty, '\n', TTY_NORMAL);
		} else
			tty_insert_flip_string(sclp_tty, buf, count - 2);
		tty_flip_buffer_push(sclp_tty);
		break;
	}
}


static int sclp_switch_cases(unsigned char *buf, int count)
{
	unsigned char *ip, *op;
	int toggle;

	
	toggle = 0;
	ip = op = buf;
	while (count-- > 0) {
		
		if (*ip == CASE_DELIMITER) {
			
			if (count && ip[1] == CASE_DELIMITER) {
				
				*op++ = *ip++;
				count--;
			} else
				
				toggle = ~toggle;
			
			ip++;
		} else
			
			if (toggle)
				
				if (sclp_tty_tolower)
					
					*op++ = _ebc_toupper[(int) *ip++];
				else
					
					*op++ = _ebc_tolower[(int) *ip++];
			else
				
				*op++ = *ip++;
	}
	
	return op - buf;
}

static void
sclp_get_input(unsigned char *start, unsigned char *end)
{
	int count;

	count = end - start;
	if (sclp_tty_tolower)
		EBC_TOLOWER(start, count);
	count = sclp_switch_cases(start, count);
	
	sclp_ebcasc_str(start, count);

	
	sclp_tty_input(start, count);
}

static inline struct gds_vector *
find_gds_vector(struct gds_vector *start, struct gds_vector *end, u16 id)
{
	struct gds_vector *vec;

	for (vec = start; vec < end; vec = (void *) vec + vec->length)
		if (vec->gds_id == id)
			return vec;
	return NULL;
}

static inline struct gds_subvector *
find_gds_subvector(struct gds_subvector *start,
		   struct gds_subvector *end, u8 key)
{
	struct gds_subvector *subvec;

	for (subvec = start; subvec < end;
	     subvec = (void *) subvec + subvec->length)
		if (subvec->key == key)
			return subvec;
	return NULL;
}

static inline void
sclp_eval_selfdeftextmsg(struct gds_subvector *start,
			 struct gds_subvector *end)
{
	struct gds_subvector *subvec;

	subvec = start;
	while (subvec < end) {
		subvec = find_gds_subvector(subvec, end, 0x30);
		if (!subvec)
			break;
		sclp_get_input((unsigned char *)(subvec + 1),
			       (unsigned char *) subvec + subvec->length);
		subvec = (void *) subvec + subvec->length;
	}
}

static inline void
sclp_eval_textcmd(struct gds_subvector *start,
		  struct gds_subvector *end)
{
	struct gds_subvector *subvec;

	subvec = start;
	while (subvec < end) {
		subvec = find_gds_subvector(subvec, end,
					    GDS_KEY_SELFDEFTEXTMSG);
		if (!subvec)
			break;
		sclp_eval_selfdeftextmsg((struct gds_subvector *)(subvec + 1),
					 (void *)subvec + subvec->length);
		subvec = (void *) subvec + subvec->length;
	}
}

static inline void
sclp_eval_cpmsu(struct gds_vector *start, struct gds_vector *end)
{
	struct gds_vector *vec;

	vec = start;
	while (vec < end) {
		vec = find_gds_vector(vec, end, GDS_ID_TEXTCMD);
		if (!vec)
			break;
		sclp_eval_textcmd((struct gds_subvector *)(vec + 1),
				  (void *) vec + vec->length);
		vec = (void *) vec + vec->length;
	}
}


static inline void
sclp_eval_mdsmu(struct gds_vector *start, void *end)
{
	struct gds_vector *vec;

	vec = find_gds_vector(start, end, GDS_ID_CPMSU);
	if (vec)
		sclp_eval_cpmsu(vec + 1, (void *) vec + vec->length);
}

static void
sclp_tty_receiver(struct evbuf_header *evbuf)
{
	struct gds_vector *start, *end, *vec;

	start = (struct gds_vector *)(evbuf + 1);
	end = (void *) evbuf + evbuf->length;
	vec = find_gds_vector(start, end, GDS_ID_MDSMU);
	if (vec)
		sclp_eval_mdsmu(vec + 1, (void *) vec + vec->length);
}

static void
sclp_tty_state_change(struct sclp_register *reg)
{
}

static struct sclp_register sclp_input_event =
{
	.receive_mask = EVTYP_OPCMD_MASK | EVTYP_PMSGCMD_MASK,
	.state_change_fn = sclp_tty_state_change,
	.receiver_fn = sclp_tty_receiver
};

static const struct tty_operations sclp_ops = {
	.open = sclp_tty_open,
	.close = sclp_tty_close,
	.write = sclp_tty_write,
	.put_char = sclp_tty_put_char,
	.flush_chars = sclp_tty_flush_chars,
	.write_room = sclp_tty_write_room,
	.chars_in_buffer = sclp_tty_chars_in_buffer,
	.flush_buffer = sclp_tty_flush_buffer,
};

static int __init
sclp_tty_init(void)
{
	struct tty_driver *driver;
	void *page;
	int i;
	int rc;

	if (!CONSOLE_IS_SCLP)
		return 0;
	driver = alloc_tty_driver(1);
	if (!driver)
		return -ENOMEM;

	rc = sclp_rw_init();
	if (rc) {
		put_tty_driver(driver);
		return rc;
	}
	
	INIT_LIST_HEAD(&sclp_tty_pages);
	for (i = 0; i < MAX_KMEM_PAGES; i++) {
		page = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
		if (page == NULL) {
			put_tty_driver(driver);
			return -ENOMEM;
		}
		list_add_tail((struct list_head *) page, &sclp_tty_pages);
	}
	INIT_LIST_HEAD(&sclp_tty_outqueue);
	spin_lock_init(&sclp_tty_lock);
	init_timer(&sclp_tty_timer);
	sclp_ttybuf = NULL;
	sclp_tty_buffer_count = 0;
	if (MACHINE_IS_VM) {
		
		sclp_tty_columns = 76;
		
		sclp_tty_tolower = 1;
	}
	sclp_tty_chars_count = 0;
	sclp_tty = NULL;

	rc = sclp_register(&sclp_input_event);
	if (rc) {
		put_tty_driver(driver);
		return rc;
	}

	driver->owner = THIS_MODULE;
	driver->driver_name = "sclp_line";
	driver->name = "sclp_line";
	driver->major = TTY_MAJOR;
	driver->minor_start = 64;
	driver->type = TTY_DRIVER_TYPE_SYSTEM;
	driver->subtype = SYSTEM_TYPE_TTY;
	driver->init_termios = tty_std_termios;
	driver->init_termios.c_iflag = IGNBRK | IGNPAR;
	driver->init_termios.c_oflag = ONLCR | XTABS;
	driver->init_termios.c_lflag = ISIG | ECHO;
	driver->flags = TTY_DRIVER_REAL_RAW;
	tty_set_operations(driver, &sclp_ops);
	rc = tty_register_driver(driver);
	if (rc) {
		put_tty_driver(driver);
		return rc;
	}
	sclp_tty_driver = driver;
	return 0;
}
module_init(sclp_tty_init);
