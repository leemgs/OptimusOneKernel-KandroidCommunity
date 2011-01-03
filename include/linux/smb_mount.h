

#ifndef _LINUX_SMB_MOUNT_H
#define _LINUX_SMB_MOUNT_H

#include <linux/types.h>

#define SMB_MOUNT_VERSION	6

struct smb_mount_data {
	int version;
	__kernel_uid_t mounted_uid; 
	__kernel_uid_t uid;
	__kernel_gid_t gid;
	__kernel_mode_t file_mode;
	__kernel_mode_t dir_mode;
};


#ifdef __KERNEL__


#define SMB_MOUNT_ASCII 0x76657273

#define SMB_MOUNT_OLDVERSION	6
#undef SMB_MOUNT_VERSION
#define SMB_MOUNT_VERSION	7


#define SMB_MOUNT_WIN95		0x0001	
#define SMB_MOUNT_OLDATTR	0x0002	
#define SMB_MOUNT_DIRATTR	0x0004	
#define SMB_MOUNT_CASE		0x0008	
#define SMB_MOUNT_UNICODE	0x0010	
#define SMB_MOUNT_UID		0x0020  
#define SMB_MOUNT_GID		0x0040  
#define SMB_MOUNT_FMODE		0x0080  
#define SMB_MOUNT_DMODE		0x0100  

struct smb_mount_data_kernel {
	int version;

	uid_t mounted_uid;	
	uid_t uid;
	gid_t gid;
	mode_t file_mode;
	mode_t dir_mode;

	u32 flags;

        
	int ttl;

	struct smb_nls_codepage codepage;
};

#endif

#endif
