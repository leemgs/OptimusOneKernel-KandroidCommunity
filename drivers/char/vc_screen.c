

#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>
#include <linux/kbd_kern.h>
#include <linux/console.h>
#include <linux/device.h>
#include <linux/smp_lock.h>

#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>

#undef attr
#undef org
#undef addr
#define HEADER_SIZE	4

static int
vcs_size(struct inode *inode)
{
	int size;
	int minor = iminor(inode);
	int currcons = minor & 127;
	struct vc_data *vc;

	if (currcons == 0)
		currcons = fg_console;
	else
		currcons--;
	if (!vc_cons_allocated(currcons))
		return -ENXIO;
	vc = vc_cons[currcons].d;

	size = vc->vc_rows * vc->vc_cols;

	if (minor & 128)
		size = 2*size + HEADER_SIZE;
	return size;
}

static loff_t vcs_lseek(struct file *file, loff_t offset, int orig)
{
	int size;

	mutex_lock(&con_buf_mtx);
	size = vcs_size(file->f_path.dentry->d_inode);
	switch (orig) {
		default:
			mutex_unlock(&con_buf_mtx);
			return -EINVAL;
		case 2:
			offset += size;
			break;
		case 1:
			offset += file->f_pos;
		case 0:
			break;
	}
	if (offset < 0 || offset > size) {
		mutex_unlock(&con_buf_mtx);
		return -EINVAL;
	}
	file->f_pos = offset;
	mutex_unlock(&con_buf_mtx);
	return file->f_pos;
}


static ssize_t
vcs_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	unsigned int currcons = iminor(inode);
	struct vc_data *vc;
	long pos;
	long viewed, attr, read;
	int col, maxcol;
	unsigned short *org = NULL;
	ssize_t ret;

	mutex_lock(&con_buf_mtx);

	pos = *ppos;

	
	acquire_console_sem();

	attr = (currcons & 128);
	currcons = (currcons & 127);
	if (currcons == 0) {
		currcons = fg_console;
		viewed = 1;
	} else {
		currcons--;
		viewed = 0;
	}
	ret = -ENXIO;
	if (!vc_cons_allocated(currcons))
		goto unlock_out;
	vc = vc_cons[currcons].d;

	ret = -EINVAL;
	if (pos < 0)
		goto unlock_out;
	read = 0;
	ret = 0;
	while (count) {
		char *con_buf0, *con_buf_start;
		long this_round, size;
		ssize_t orig_count;
		long p = pos;

		
		size = vcs_size(inode);
		if (pos >= size)
			break;
		if (count > size - pos)
			count = size - pos;

		this_round = count;
		if (this_round > CON_BUF_SIZE)
			this_round = CON_BUF_SIZE;

		

		con_buf_start = con_buf0 = con_buf;
		orig_count = this_round;
		maxcol = vc->vc_cols;
		if (!attr) {
			org = screen_pos(vc, p, viewed);
			col = p % maxcol;
			p += maxcol - col;
			while (this_round-- > 0) {
				*con_buf0++ = (vcs_scr_readw(vc, org++) & 0xff);
				if (++col == maxcol) {
					org = screen_pos(vc, p, viewed);
					col = 0;
					p += maxcol;
				}
			}
		} else {
			if (p < HEADER_SIZE) {
				size_t tmp_count;

				con_buf0[0] = (char)vc->vc_rows;
				con_buf0[1] = (char)vc->vc_cols;
				getconsxy(vc, con_buf0 + 2);

				con_buf_start += p;
				this_round += p;
				if (this_round > CON_BUF_SIZE) {
					this_round = CON_BUF_SIZE;
					orig_count = this_round - p;
				}

				tmp_count = HEADER_SIZE;
				if (tmp_count > this_round)
					tmp_count = this_round;

				
				this_round -= tmp_count;
				p = HEADER_SIZE;
				con_buf0 = con_buf + HEADER_SIZE;
				
			} else if (p & 1) {
				
				con_buf_start++;
				if (this_round < CON_BUF_SIZE)
					this_round++;
				else
					orig_count--;
			}
			if (this_round > 0) {
				unsigned short *tmp_buf = (unsigned short *)con_buf0;

				p -= HEADER_SIZE;
				p /= 2;
				col = p % maxcol;

				org = screen_pos(vc, p, viewed);
				p += maxcol - col;

				
				this_round = (this_round + 1) >> 1;

				while (this_round) {
					*tmp_buf++ = vcs_scr_readw(vc, org++);
					this_round --;
					if (++col == maxcol) {
						org = screen_pos(vc, p, viewed);
						col = 0;
						p += maxcol;
					}
				}
			}
		}

		

		release_console_sem();
		ret = copy_to_user(buf, con_buf_start, orig_count);
		acquire_console_sem();

		if (ret) {
			read += (orig_count - ret);
			ret = -EFAULT;
			break;
		}
		buf += orig_count;
		pos += orig_count;
		read += orig_count;
		count -= orig_count;
	}
	*ppos += read;
	if (read)
		ret = read;
unlock_out:
	release_console_sem();
	mutex_unlock(&con_buf_mtx);
	return ret;
}

static ssize_t
vcs_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	unsigned int currcons = iminor(inode);
	struct vc_data *vc;
	long pos;
	long viewed, attr, size, written;
	char *con_buf0;
	int col, maxcol;
	u16 *org0 = NULL, *org = NULL;
	size_t ret;

	mutex_lock(&con_buf_mtx);

	pos = *ppos;

	
	acquire_console_sem();

	attr = (currcons & 128);
	currcons = (currcons & 127);

	if (currcons == 0) {
		currcons = fg_console;
		viewed = 1;
	} else {
		currcons--;
		viewed = 0;
	}
	ret = -ENXIO;
	if (!vc_cons_allocated(currcons))
		goto unlock_out;
	vc = vc_cons[currcons].d;

	size = vcs_size(inode);
	ret = -EINVAL;
	if (pos < 0 || pos > size)
		goto unlock_out;
	if (count > size - pos)
		count = size - pos;
	written = 0;
	while (count) {
		long this_round = count;
		size_t orig_count;
		long p;

		if (this_round > CON_BUF_SIZE)
			this_round = CON_BUF_SIZE;

		
		release_console_sem();
		ret = copy_from_user(con_buf, buf, this_round);
		acquire_console_sem();

		if (ret) {
			this_round -= ret;
			if (!this_round) {
				
				if (written)
					break;
				ret = -EFAULT;
				goto unlock_out;
			}
		}

		
		size = vcs_size(inode);
		if (pos >= size)
			break;
		if (this_round > size - pos)
			this_round = size - pos;

		

		con_buf0 = con_buf;
		orig_count = this_round;
		maxcol = vc->vc_cols;
		p = pos;
		if (!attr) {
			org0 = org = screen_pos(vc, p, viewed);
			col = p % maxcol;
			p += maxcol - col;

			while (this_round > 0) {
				unsigned char c = *con_buf0++;

				this_round--;
				vcs_scr_writew(vc,
					       (vcs_scr_readw(vc, org) & 0xff00) | c, org);
				org++;
				if (++col == maxcol) {
					org = screen_pos(vc, p, viewed);
					col = 0;
					p += maxcol;
				}
			}
		} else {
			if (p < HEADER_SIZE) {
				char header[HEADER_SIZE];

				getconsxy(vc, header + 2);
				while (p < HEADER_SIZE && this_round > 0) {
					this_round--;
					header[p++] = *con_buf0++;
				}
				if (!viewed)
					putconsxy(vc, header + 2);
			}
			p -= HEADER_SIZE;
			col = (p/2) % maxcol;
			if (this_round > 0) {
				org0 = org = screen_pos(vc, p/2, viewed);
				if ((p & 1) && this_round > 0) {
					char c;

					this_round--;
					c = *con_buf0++;
#ifdef __BIG_ENDIAN
					vcs_scr_writew(vc, c |
					     (vcs_scr_readw(vc, org) & 0xff00), org);
#else
					vcs_scr_writew(vc, (c << 8) |
					     (vcs_scr_readw(vc, org) & 0xff), org);
#endif
					org++;
					p++;
					if (++col == maxcol) {
						org = screen_pos(vc, p/2, viewed);
						col = 0;
					}
				}
				p /= 2;
				p += maxcol - col;
			}
			while (this_round > 1) {
				unsigned short w;

				w = get_unaligned(((unsigned short *)con_buf0));
				vcs_scr_writew(vc, w, org++);
				con_buf0 += 2;
				this_round -= 2;
				if (++col == maxcol) {
					org = screen_pos(vc, p, viewed);
					col = 0;
					p += maxcol;
				}
			}
			if (this_round > 0) {
				unsigned char c;

				c = *con_buf0++;
#ifdef __BIG_ENDIAN
				vcs_scr_writew(vc, (vcs_scr_readw(vc, org) & 0xff) | (c << 8), org);
#else
				vcs_scr_writew(vc, (vcs_scr_readw(vc, org) & 0xff00) | c, org);
#endif
			}
		}
		count -= orig_count;
		written += orig_count;
		buf += orig_count;
		pos += orig_count;
		if (org0)
			update_region(vc, (unsigned long)(org0), org - org0);
	}
	*ppos += written;
	ret = written;

unlock_out:
	release_console_sem();

	mutex_unlock(&con_buf_mtx);

	return ret;
}

static int
vcs_open(struct inode *inode, struct file *filp)
{
	unsigned int currcons = iminor(inode) & 127;
	int ret = 0;
	
	lock_kernel();
	if(currcons && !vc_cons_allocated(currcons-1))
		ret = -ENXIO;
	unlock_kernel();
	return ret;
}

static const struct file_operations vcs_fops = {
	.llseek		= vcs_lseek,
	.read		= vcs_read,
	.write		= vcs_write,
	.open		= vcs_open,
};

static struct class *vc_class;

void vcs_make_sysfs(int index)
{
	device_create(vc_class, NULL, MKDEV(VCS_MAJOR, index + 1), NULL,
		      "vcs%u", index + 1);
	device_create(vc_class, NULL, MKDEV(VCS_MAJOR, index + 129), NULL,
		      "vcsa%u", index + 1);
}

void vcs_remove_sysfs(int index)
{
	device_destroy(vc_class, MKDEV(VCS_MAJOR, index + 1));
	device_destroy(vc_class, MKDEV(VCS_MAJOR, index + 129));
}

int __init vcs_init(void)
{
	unsigned int i;

	if (register_chrdev(VCS_MAJOR, "vcs", &vcs_fops))
		panic("unable to get major %d for vcs device", VCS_MAJOR);
	vc_class = class_create(THIS_MODULE, "vc");

	device_create(vc_class, NULL, MKDEV(VCS_MAJOR, 0), NULL, "vcs");
	device_create(vc_class, NULL, MKDEV(VCS_MAJOR, 128), NULL, "vcsa");
	for (i = 0; i < MIN_NR_CONSOLES; i++)
		vcs_make_sysfs(i);
	return 0;
}
