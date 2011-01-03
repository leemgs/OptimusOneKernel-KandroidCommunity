

#include <net/sctp/structs.h>
#include <net/sctp/sctp.h>
#include <linux/sysctl.h>

static int zero = 0;
static int one = 1;
static int timer_max = 86400000; 
static int int_max = INT_MAX;
static int sack_timer_min = 1;
static int sack_timer_max = 500;
static int addr_scope_max = 3; 

extern int sysctl_sctp_mem[3];
extern int sysctl_sctp_rmem[3];
extern int sysctl_sctp_wmem[3];

static ctl_table sctp_table[] = {
	{
		.ctl_name	= NET_SCTP_RTO_INITIAL,
		.procname	= "rto_initial",
		.data		= &sctp_rto_initial,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &one,
		.extra2         = &timer_max
	},
	{
		.ctl_name	= NET_SCTP_RTO_MIN,
		.procname	= "rto_min",
		.data		= &sctp_rto_min,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &one,
		.extra2         = &timer_max
	},
	{
		.ctl_name	= NET_SCTP_RTO_MAX,
		.procname	= "rto_max",
		.data		= &sctp_rto_max,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &one,
		.extra2         = &timer_max
	},
	{
		.ctl_name	= NET_SCTP_VALID_COOKIE_LIFE,
		.procname	= "valid_cookie_life",
		.data		= &sctp_valid_cookie_life,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &one,
		.extra2         = &timer_max
	},
	{
		.ctl_name	= NET_SCTP_MAX_BURST,
		.procname	= "max_burst",
		.data		= &sctp_max_burst,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1		= &zero,
		.extra2		= &int_max
	},
	{
		.ctl_name	= NET_SCTP_ASSOCIATION_MAX_RETRANS,
		.procname	= "association_max_retrans",
		.data		= &sctp_max_retrans_association,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1		= &one,
		.extra2		= &int_max
	},
	{
		.ctl_name	= NET_SCTP_SNDBUF_POLICY,
		.procname	= "sndbuf_policy",
		.data		= &sctp_sndbuf_policy,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_RCVBUF_POLICY,
		.procname	= "rcvbuf_policy",
		.data		= &sctp_rcvbuf_policy,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_PATH_MAX_RETRANS,
		.procname	= "path_max_retrans",
		.data		= &sctp_max_retrans_path,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1		= &one,
		.extra2		= &int_max
	},
	{
		.ctl_name	= NET_SCTP_MAX_INIT_RETRANSMITS,
		.procname	= "max_init_retransmits",
		.data		= &sctp_max_retrans_init,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1		= &one,
		.extra2		= &int_max
	},
	{
		.ctl_name	= NET_SCTP_HB_INTERVAL,
		.procname	= "hb_interval",
		.data		= &sctp_hb_interval,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &one,
		.extra2         = &timer_max
	},
	{
		.ctl_name	= NET_SCTP_PRESERVE_ENABLE,
		.procname	= "cookie_preserve_enable",
		.data		= &sctp_cookie_preserve_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_RTO_ALPHA,
		.procname	= "rto_alpha_exp_divisor",
		.data		= &sctp_rto_alpha,
		.maxlen		= sizeof(int),
		.mode		= 0444,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_RTO_BETA,
		.procname	= "rto_beta_exp_divisor",
		.data		= &sctp_rto_beta,
		.maxlen		= sizeof(int),
		.mode		= 0444,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_ADDIP_ENABLE,
		.procname	= "addip_enable",
		.data		= &sctp_addip_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_PRSCTP_ENABLE,
		.procname	= "prsctp_enable",
		.data		= &sctp_prsctp_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= NET_SCTP_SACK_TIMEOUT,
		.procname	= "sack_timeout",
		.data		= &sctp_sack_timeout,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.strategy	= sysctl_intvec,
		.extra1         = &sack_timer_min,
		.extra2         = &sack_timer_max,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "sctp_mem",
		.data		= &sysctl_sctp_mem,
		.maxlen		= sizeof(sysctl_sctp_mem),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "sctp_rmem",
		.data		= &sysctl_sctp_rmem,
		.maxlen		= sizeof(sysctl_sctp_rmem),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "sctp_wmem",
		.data		= &sysctl_sctp_wmem,
		.maxlen		= sizeof(sysctl_sctp_wmem),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "auth_enable",
		.data		= &sctp_auth_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "addip_noauth_enable",
		.data		= &sctp_addip_noauth,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
		.strategy	= sysctl_intvec
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "addr_scope_policy",
		.data		= &sctp_scope_policy,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_minmax,
		.strategy	= &sysctl_intvec,
		.extra1		= &zero,
		.extra2		= &addr_scope_max,
	},
	{ .ctl_name = 0 }
};

static struct ctl_path sctp_path[] = {
	{ .procname = "net", .ctl_name = CTL_NET, },
	{ .procname = "sctp", .ctl_name = NET_SCTP, },
	{ }
};

static struct ctl_table_header * sctp_sysctl_header;


void sctp_sysctl_register(void)
{
	sctp_sysctl_header = register_sysctl_paths(sctp_path, sctp_table);
}


void sctp_sysctl_unregister(void)
{
	unregister_sysctl_table(sctp_sysctl_header);
}
