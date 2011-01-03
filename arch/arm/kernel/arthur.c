



#include <linux/module.h>
#include <linux/personality.h>
#include <linux/stddef.h>
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/sched.h>

#include <asm/ptrace.h>



#define ARTHUR_SIGABRT		1
#define ARTHUR_SIGFPE		2
#define ARTHUR_SIGILL		3
#define ARTHUR_SIGINT		4
#define ARTHUR_SIGSEGV		5
#define ARTHUR_SIGTERM		6
#define ARTHUR_SIGSTAK		7
#define ARTHUR_SIGUSR1		8
#define ARTHUR_SIGUSR2		9
#define ARTHUR_SIGOSERROR	10

static unsigned long arthur_to_linux_signals[32] = {
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31
};

static unsigned long linux_to_arthur_signals[32] = {
	0,		-1,		ARTHUR_SIGINT,	-1,
       	ARTHUR_SIGILL,	5,		ARTHUR_SIGABRT,	7,
	ARTHUR_SIGFPE,	9,		ARTHUR_SIGUSR1,	ARTHUR_SIGSEGV,	
	ARTHUR_SIGUSR2,	13,		14,		ARTHUR_SIGTERM,
	16,		17,		18,		19,
	20,		21,		22,		23,
	24,		25,		26,		27,
	28,		29,		30,		31
};

static void arthur_lcall7(int nr, struct pt_regs *regs)
{
	struct siginfo info;
	info.si_signo = SIGSWI;
	info.si_errno = nr;
	
	send_sig_info(SIGSWI, &info, current);
}

static struct exec_domain arthur_exec_domain = {
	.name		= "Arthur",
	.handler	= arthur_lcall7,
	.pers_low	= PER_RISCOS,
	.pers_high	= PER_RISCOS,
	.signal_map	= arthur_to_linux_signals,
	.signal_invmap	= linux_to_arthur_signals,
	.module		= THIS_MODULE,
};



static int __init arthur_init(void)
{
	return register_exec_domain(&arthur_exec_domain);
}

static void __exit arthur_exit(void)
{
	unregister_exec_domain(&arthur_exec_domain);
}

module_init(arthur_init);
module_exit(arthur_exit);

MODULE_LICENSE("GPL");
