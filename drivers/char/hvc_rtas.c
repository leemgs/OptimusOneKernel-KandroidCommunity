

#include <linux/console.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/types.h>

#include <asm/irq.h>
#include <asm/rtas.h>
#include "hvc_console.h"

#define hvc_rtas_cookie 0x67781e15
struct hvc_struct *hvc_rtas_dev;

static int rtascons_put_char_token = RTAS_UNKNOWN_SERVICE;
static int rtascons_get_char_token = RTAS_UNKNOWN_SERVICE;

static inline int hvc_rtas_write_console(uint32_t vtermno, const char *buf,
		int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (rtas_call(rtascons_put_char_token, 1, 1, NULL, buf[i]))
			break;
	}

	return i;
}

static int hvc_rtas_read_console(uint32_t vtermno, char *buf, int count)
{
	int i, c;

	for (i = 0; i < count; i++) {
		if (rtas_call(rtascons_get_char_token, 0, 2, &c))
			break;

		buf[i] = c;
	}

	return i;
}

static struct hv_ops hvc_rtas_get_put_ops = {
	.get_chars = hvc_rtas_read_console,
	.put_chars = hvc_rtas_write_console,
};

static int __init hvc_rtas_init(void)
{
	struct hvc_struct *hp;

	if (rtascons_put_char_token == RTAS_UNKNOWN_SERVICE)
		rtascons_put_char_token = rtas_token("put-term-char");
	if (rtascons_put_char_token == RTAS_UNKNOWN_SERVICE)
		return -EIO;

	if (rtascons_get_char_token == RTAS_UNKNOWN_SERVICE)
		rtascons_get_char_token = rtas_token("get-term-char");
	if (rtascons_get_char_token == RTAS_UNKNOWN_SERVICE)
		return -EIO;

	BUG_ON(hvc_rtas_dev);

	
	hp = hvc_alloc(hvc_rtas_cookie, NO_IRQ, &hvc_rtas_get_put_ops, 16);
	if (IS_ERR(hp))
		return PTR_ERR(hp);

	hvc_rtas_dev = hp;

	return 0;
}
module_init(hvc_rtas_init);


static void __exit hvc_rtas_exit(void)
{
	
	if (hvc_rtas_dev)
		hvc_remove(hvc_rtas_dev);
}
module_exit(hvc_rtas_exit);


static int __init hvc_rtas_console_init(void)
{
	rtascons_put_char_token = rtas_token("put-term-char");
	if (rtascons_put_char_token == RTAS_UNKNOWN_SERVICE)
		return -EIO;

	rtascons_get_char_token = rtas_token("get-term-char");
	if (rtascons_get_char_token == RTAS_UNKNOWN_SERVICE)
		return -EIO;

	hvc_instantiate(hvc_rtas_cookie, 0, &hvc_rtas_get_put_ops);
	add_preferred_console("hvc", 0, NULL);

	return 0;
}
console_initcall(hvc_rtas_console_init);
