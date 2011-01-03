

#ifndef _LINUX_SMB_FS_I
#define _LINUX_SMB_FS_I

#include <linux/types.h>
#include <linux/fs.h>


struct smb_inode_info {

	
        unsigned int open;	
	__u16 fileid;		
	__u16 attr;		

	__u16 access;		
	__u16 flags;
	unsigned long oldmtime;	
	unsigned long closed;	
	unsigned openers;	

	struct inode vfs_inode;	
};

#endif
