

#include <linux/module.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/mtio.h>
#include <linux/smp_lock.h>

#include <asm/uaccess.h>

#define TAPE_DBF_AREA	tape_core_dbf

#include "tape.h"
#include "tape_std.h"
#include "tape_class.h"

#define TAPECHAR_MAJOR		0	


static ssize_t tapechar_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t tapechar_write(struct file *, const char __user *, size_t, loff_t *);
static int tapechar_open(struct inode *,struct file *);
static int tapechar_release(struct inode *,struct file *);
static int tapechar_ioctl(struct inode *, struct file *, unsigned int,
			  unsigned long);
static long tapechar_compat_ioctl(struct file *, unsigned int,
			  unsigned long);

static const struct file_operations tape_fops =
{
	.owner = THIS_MODULE,
	.read = tapechar_read,
	.write = tapechar_write,
	.ioctl = tapechar_ioctl,
	.compat_ioctl = tapechar_compat_ioctl,
	.open = tapechar_open,
	.release = tapechar_release,
};

static int tapechar_major = TAPECHAR_MAJOR;


int
tapechar_setup_device(struct tape_device * device)
{
	char	device_name[20];

	sprintf(device_name, "ntibm%i", device->first_minor / 2);
	device->nt = register_tape_dev(
		&device->cdev->dev,
		MKDEV(tapechar_major, device->first_minor),
		&tape_fops,
		device_name,
		"non-rewinding"
	);
	device_name[0] = 'r';
	device->rt = register_tape_dev(
		&device->cdev->dev,
		MKDEV(tapechar_major, device->first_minor + 1),
		&tape_fops,
		device_name,
		"rewinding"
	);

	return 0;
}

void
tapechar_cleanup_device(struct tape_device *device)
{
	unregister_tape_dev(&device->cdev->dev, device->rt);
	device->rt = NULL;
	unregister_tape_dev(&device->cdev->dev, device->nt);
	device->nt = NULL;
}

static int
tapechar_check_idalbuffer(struct tape_device *device, size_t block_size)
{
	struct idal_buffer *new;

	if (device->char_data.idal_buf != NULL &&
	    device->char_data.idal_buf->size == block_size)
		return 0;

	if (block_size > MAX_BLOCKSIZE) {
		DBF_EVENT(3, "Invalid blocksize (%zd > %d)\n",
			block_size, MAX_BLOCKSIZE);
		return -EINVAL;
	}

	
	new = idal_buffer_alloc(block_size, 0);
	if (IS_ERR(new))
		return -ENOMEM;

	if (device->char_data.idal_buf != NULL)
		idal_buffer_free(device->char_data.idal_buf);

	device->char_data.idal_buf = new;

	return 0;
}


static ssize_t
tapechar_read(struct file *filp, char __user *data, size_t count, loff_t *ppos)
{
	struct tape_device *device;
	struct tape_request *request;
	size_t block_size;
	int rc;

	DBF_EVENT(6, "TCHAR:read\n");
	device = (struct tape_device *) filp->private_data;

	
	if(device->required_tapemarks) {
		return tape_std_terminate_write(device);
	}

	
	if (device->char_data.block_size != 0) {
		if (count < device->char_data.block_size) {
			DBF_EVENT(3, "TCHAR:read smaller than block "
				  "size was requested\n");
			return -EINVAL;
		}
		block_size = device->char_data.block_size;
	} else {
		block_size = count;
	}

	rc = tapechar_check_idalbuffer(device, block_size);
	if (rc)
		return rc;

#ifdef CONFIG_S390_TAPE_BLOCK
	
	device->blk_data.medium_changed = 1;
#endif

	DBF_EVENT(6, "TCHAR:nbytes: %lx\n", block_size);
	
	request = device->discipline->read_block(device, block_size);
	if (IS_ERR(request))
		return PTR_ERR(request);
	
	rc = tape_do_io(device, request);
	if (rc == 0) {
		rc = block_size - request->rescnt;
		DBF_EVENT(6, "TCHAR:rbytes:  %x\n", rc);
		filp->f_pos += rc;
		
		if (idal_buffer_to_user(device->char_data.idal_buf,
					data, rc) != 0)
			rc = -EFAULT;
	}
	tape_free_request(request);
	return rc;
}


static ssize_t
tapechar_write(struct file *filp, const char __user *data, size_t count, loff_t *ppos)
{
	struct tape_device *device;
	struct tape_request *request;
	size_t block_size;
	size_t written;
	int nblocks;
	int i, rc;

	DBF_EVENT(6, "TCHAR:write\n");
	device = (struct tape_device *) filp->private_data;
	
	if (device->char_data.block_size != 0) {
		if (count < device->char_data.block_size) {
			DBF_EVENT(3, "TCHAR:write smaller than block "
				  "size was requested\n");
			return -EINVAL;
		}
		block_size = device->char_data.block_size;
		nblocks = count / block_size;
	} else {
		block_size = count;
		nblocks = 1;
	}

	rc = tapechar_check_idalbuffer(device, block_size);
	if (rc)
		return rc;

#ifdef CONFIG_S390_TAPE_BLOCK
	
	device->blk_data.medium_changed = 1;
#endif

	DBF_EVENT(6,"TCHAR:nbytes: %lx\n", block_size);
	DBF_EVENT(6, "TCHAR:nblocks: %x\n", nblocks);
	
	request = device->discipline->write_block(device, block_size);
	if (IS_ERR(request))
		return PTR_ERR(request);
	rc = 0;
	written = 0;
	for (i = 0; i < nblocks; i++) {
		
		if (idal_buffer_from_user(device->char_data.idal_buf,
					  data, block_size)) {
			rc = -EFAULT;
			break;
		}
		rc = tape_do_io(device, request);
		if (rc)
			break;
		DBF_EVENT(6, "TCHAR:wbytes: %lx\n",
			  block_size - request->rescnt);
		filp->f_pos += block_size - request->rescnt;
		written += block_size - request->rescnt;
		if (request->rescnt != 0)
			break;
		data += block_size;
	}
	tape_free_request(request);
	if (rc == -ENOSPC) {
		
		if (device->discipline->process_eov)
			device->discipline->process_eov(device);
		if (written > 0)
			rc = 0;

	}

	
	if (!rc)
		device->required_tapemarks = 2;

	return rc ? rc : written;
}


static int
tapechar_open (struct inode *inode, struct file *filp)
{
	struct tape_device *device;
	int minor, rc;

	DBF_EVENT(6, "TCHAR:open: %i:%i\n",
		imajor(filp->f_path.dentry->d_inode),
		iminor(filp->f_path.dentry->d_inode));

	if (imajor(filp->f_path.dentry->d_inode) != tapechar_major)
		return -ENODEV;

	lock_kernel();
	minor = iminor(filp->f_path.dentry->d_inode);
	device = tape_get_device(minor / TAPE_MINORS_PER_DEV);
	if (IS_ERR(device)) {
		DBF_EVENT(3, "TCHAR:open: tape_get_device() failed\n");
		rc = PTR_ERR(device);
		goto out;
	}


	rc = tape_open(device);
	if (rc == 0) {
		filp->private_data = device;
		rc = nonseekable_open(inode, filp);
	}
	else
		tape_put_device(device);

out:
	unlock_kernel();
	return rc;
}



static int
tapechar_release(struct inode *inode, struct file *filp)
{
	struct tape_device *device;

	DBF_EVENT(6, "TCHAR:release: %x\n", iminor(inode));
	device = (struct tape_device *) filp->private_data;

	
	if ((iminor(inode) & 1) != 0) {
		if (device->required_tapemarks)
			tape_std_terminate_write(device);
		tape_mtop(device, MTREW, 1);
	} else {
		if (device->required_tapemarks > 1) {
			if (tape_mtop(device, MTWEOF, 1) == 0)
				device->required_tapemarks--;
		}
	}

	if (device->char_data.idal_buf != NULL) {
		idal_buffer_free(device->char_data.idal_buf);
		device->char_data.idal_buf = NULL;
	}
	tape_release(device);
	filp->private_data = tape_put_device(device);

	return 0;
}


static int
tapechar_ioctl(struct inode *inp, struct file *filp,
	       unsigned int no, unsigned long data)
{
	struct tape_device *device;
	int rc;

	DBF_EVENT(6, "TCHAR:ioct\n");

	device = (struct tape_device *) filp->private_data;

	if (no == MTIOCTOP) {
		struct mtop op;

		if (copy_from_user(&op, (char __user *) data, sizeof(op)) != 0)
			return -EFAULT;
		if (op.mt_count < 0)
			return -EINVAL;

		
		switch (op.mt_op) {
			case MTFSF:
			case MTBSF:
			case MTFSR:
			case MTBSR:
			case MTREW:
			case MTOFFL:
			case MTEOM:
			case MTRETEN:
			case MTBSFM:
			case MTFSFM:
			case MTSEEK:
#ifdef CONFIG_S390_TAPE_BLOCK
				device->blk_data.medium_changed = 1;
#endif
				if (device->required_tapemarks)
					tape_std_terminate_write(device);
			default:
				;
		}
		rc = tape_mtop(device, op.mt_op, op.mt_count);

		if (op.mt_op == MTWEOF && rc == 0) {
			if (op.mt_count > device->required_tapemarks)
				device->required_tapemarks = 0;
			else
				device->required_tapemarks -= op.mt_count;
		}
		return rc;
	}
	if (no == MTIOCPOS) {
		
		struct mtpos pos;

		rc = tape_mtop(device, MTTELL, 1);
		if (rc < 0)
			return rc;
		pos.mt_blkno = rc;
		if (copy_to_user((char __user *) data, &pos, sizeof(pos)) != 0)
			return -EFAULT;
		return 0;
	}
	if (no == MTIOCGET) {
		
		struct mtget get;

		memset(&get, 0, sizeof(get));
		get.mt_type = MT_ISUNKNOWN;
		get.mt_resid = 0 ;
		get.mt_dsreg = device->tape_state;
		
		get.mt_gstat = 0;
		get.mt_erreg = 0;
		get.mt_fileno = 0;
		get.mt_gstat  = device->tape_generic_status;

		if (device->medium_state == MS_LOADED) {
			rc = tape_mtop(device, MTTELL, 1);

			if (rc < 0)
				return rc;

			if (rc == 0)
				get.mt_gstat |= GMT_BOT(~0);

			get.mt_blkno = rc;
		}

		if (copy_to_user((char __user *) data, &get, sizeof(get)) != 0)
			return -EFAULT;

		return 0;
	}
	
	if (device->discipline->ioctl_fn == NULL)
		return -EINVAL;
	return device->discipline->ioctl_fn(device, no, data);
}

static long
tapechar_compat_ioctl(struct file *filp, unsigned int no, unsigned long data)
{
	struct tape_device *device = filp->private_data;
	int rval = -ENOIOCTLCMD;

	if (device->discipline->ioctl_fn) {
		lock_kernel();
		rval = device->discipline->ioctl_fn(device, no, data);
		unlock_kernel();
		if (rval == -EINVAL)
			rval = -ENOIOCTLCMD;
	}

	return rval;
}


int
tapechar_init (void)
{
	dev_t	dev;

	if (alloc_chrdev_region(&dev, 0, 256, "tape") != 0)
		return -1;

	tapechar_major = MAJOR(dev);

	return 0;
}


void
tapechar_exit(void)
{
	unregister_chrdev_region(MKDEV(tapechar_major, 0), 256);
}
