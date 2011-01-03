#include <linux/sysdev.h>
#include <asm/mce.h>

enum severity_level {
	MCE_NO_SEVERITY,
	MCE_KEEP_SEVERITY,
	MCE_SOME_SEVERITY,
	MCE_AO_SEVERITY,
	MCE_UC_SEVERITY,
	MCE_AR_SEVERITY,
	MCE_PANIC_SEVERITY,
};

#define ATTR_LEN		16


struct mce_bank {
	u64			ctl;			
	unsigned char init;				
	struct sysdev_attribute attr;			
	char			attrname[ATTR_LEN];	
};

int mce_severity(struct mce *a, int tolerant, char **msg);
struct dentry *mce_get_debugfs_dir(void);

extern int mce_ser;

extern struct mce_bank *mce_banks;

