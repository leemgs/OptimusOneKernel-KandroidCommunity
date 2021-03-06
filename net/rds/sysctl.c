
#include <linux/kernel.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>

#include "rds.h"

static struct ctl_table_header *rds_sysctl_reg_table;

static unsigned long rds_sysctl_reconnect_min = 1;
static unsigned long rds_sysctl_reconnect_max = ~0UL;

unsigned long rds_sysctl_reconnect_min_jiffies;
unsigned long rds_sysctl_reconnect_max_jiffies = HZ;

unsigned int  rds_sysctl_max_unacked_packets = 8;
unsigned int  rds_sysctl_max_unacked_bytes = (16 << 20);

unsigned int rds_sysctl_ping_enable = 1;

static ctl_table rds_sysctl_rds_table[] = {
	{
		.ctl_name       = CTL_UNNUMBERED,
		.procname       = "reconnect_min_delay_ms",
		.data		= &rds_sysctl_reconnect_min_jiffies,
		.maxlen         = sizeof(unsigned long),
		.mode           = 0644,
		.proc_handler   = &proc_doulongvec_ms_jiffies_minmax,
		.extra1		= &rds_sysctl_reconnect_min,
		.extra2		= &rds_sysctl_reconnect_max_jiffies,
	},
	{
		.ctl_name       = CTL_UNNUMBERED,
		.procname       = "reconnect_max_delay_ms",
		.data		= &rds_sysctl_reconnect_max_jiffies,
		.maxlen         = sizeof(unsigned long),
		.mode           = 0644,
		.proc_handler   = &proc_doulongvec_ms_jiffies_minmax,
		.extra1		= &rds_sysctl_reconnect_min_jiffies,
		.extra2		= &rds_sysctl_reconnect_max,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "max_unacked_packets",
		.data		= &rds_sysctl_max_unacked_packets,
		.maxlen         = sizeof(unsigned long),
		.mode           = 0644,
		.proc_handler   = &proc_dointvec,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "max_unacked_bytes",
		.data		= &rds_sysctl_max_unacked_bytes,
		.maxlen         = sizeof(unsigned long),
		.mode           = 0644,
		.proc_handler   = &proc_dointvec,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "ping_enable",
		.data		= &rds_sysctl_ping_enable,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &proc_dointvec,
	},
	{ .ctl_name = 0}
};

static struct ctl_path rds_sysctl_path[] = {
	{ .procname = "net", .ctl_name = CTL_NET, },
	{ .procname = "rds", .ctl_name = CTL_UNNUMBERED, },
	{ }
};


void rds_sysctl_exit(void)
{
	if (rds_sysctl_reg_table)
		unregister_sysctl_table(rds_sysctl_reg_table);
}

int __init rds_sysctl_init(void)
{
	rds_sysctl_reconnect_min = msecs_to_jiffies(1);
	rds_sysctl_reconnect_min_jiffies = rds_sysctl_reconnect_min;

	rds_sysctl_reg_table = register_sysctl_paths(rds_sysctl_path, rds_sysctl_rds_table);
	if (rds_sysctl_reg_table == NULL)
		return -ENOMEM;
	return 0;
}
