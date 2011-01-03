

#ifndef _LINUX_DEVPTS_FS_H
#define _LINUX_DEVPTS_FS_H

#include <linux/errno.h>

#ifdef CONFIG_UNIX98_PTYS

int devpts_new_index(struct inode *ptmx_inode);
void devpts_kill_index(struct inode *ptmx_inode, int idx);

int devpts_pty_new(struct inode *ptmx_inode, struct tty_struct *tty);

struct tty_struct *devpts_get_tty(struct inode *pts_inode, int number);

void devpts_pty_kill(struct tty_struct *tty);

#else


static inline int devpts_new_index(struct inode *ptmx_inode) { return -EINVAL; }
static inline void devpts_kill_index(struct inode *ptmx_inode, int idx) { }
static inline int devpts_pty_new(struct inode *ptmx_inode,
				struct tty_struct *tty)
{
	return -EINVAL;
}
static inline struct tty_struct *devpts_get_tty(struct inode *pts_inode,
		int number)
{
	return NULL;
}
static inline void devpts_pty_kill(struct tty_struct *tty) { }

#endif


#endif 
