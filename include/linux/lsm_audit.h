
#ifndef _LSM_COMMON_LOGGING_
#define _LSM_COMMON_LOGGING_

#include <linux/stddef.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/audit.h>
#include <linux/in6.h>
#include <linux/path.h>
#include <linux/key.h>
#include <linux/skbuff.h>
#include <asm/system.h>



struct common_audit_data {
	char    type;
#define LSM_AUDIT_DATA_FS      1
#define LSM_AUDIT_DATA_NET     2
#define LSM_AUDIT_DATA_CAP     3
#define LSM_AUDIT_DATA_IPC     4
#define LSM_AUDIT_DATA_TASK    5
#define LSM_AUDIT_DATA_KEY     6
#define LSM_AUDIT_NO_AUDIT     7
	struct task_struct *tsk;
	union 	{
		struct {
			struct path path;
			struct inode *inode;
		} fs;
		struct {
			int netif;
			struct sock *sk;
			u16 family;
			__be16 dport;
			__be16 sport;
			union {
				struct {
					__be32 daddr;
					__be32 saddr;
				} v4;
				struct {
					struct in6_addr daddr;
					struct in6_addr saddr;
				} v6;
			} fam;
		} net;
		int cap;
		int ipc_id;
		struct task_struct *tsk;
#ifdef CONFIG_KEYS
		struct {
			key_serial_t key;
			char *key_desc;
		} key_struct;
#endif
	} u;
	
	union {
#ifdef CONFIG_SECURITY_SMACK
		
		struct smack_audit_data {
			const char *function;
			char *subject;
			char *object;
			char *request;
			int result;
		} smack_audit_data;
#endif
#ifdef CONFIG_SECURITY_SELINUX
		
		struct {
			u32 ssid;
			u32 tsid;
			u16 tclass;
			u32 requested;
			u32 audited;
			u32 denied;
			struct av_decision *avd;
			int result;
		} selinux_audit_data;
#endif
	};
	
	void (*lsm_pre_audit)(struct audit_buffer *, void *);
	void (*lsm_post_audit)(struct audit_buffer *, void *);
};

#define v4info fam.v4
#define v6info fam.v6

int ipv4_skb_to_auditdata(struct sk_buff *skb,
		struct common_audit_data *ad, u8 *proto);

int ipv6_skb_to_auditdata(struct sk_buff *skb,
		struct common_audit_data *ad, u8 *proto);


#define COMMON_AUDIT_DATA_INIT(_d, _t) \
	{ memset((_d), 0, sizeof(struct common_audit_data)); \
	 (_d)->type = LSM_AUDIT_DATA_##_t; }

void common_lsm_audit(struct common_audit_data *a);

#endif
