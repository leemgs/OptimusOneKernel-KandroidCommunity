

#ifndef _LINUX_SMB329_H
#define _LINUX_SMB329_H

#ifdef __KERNEL__

enum {
	SMB329_DISABLE_CHG,
	SMB329_ENABLE_SLOW_CHG,
	SMB329_ENABLE_FAST_CHG,
};

extern int smb329_set_charger_ctrl(uint32_t ctl);

#endif 

#endif 

