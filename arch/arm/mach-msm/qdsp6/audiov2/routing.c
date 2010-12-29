

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <mach/msm_qdsp6_audiov2.h>

static int q6_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t q6_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	char cmd[32];

	if (count >= sizeof(cmd))
		return -EINVAL;
	if (copy_from_user(cmd, buf, count))
		return -EFAULT;
	cmd[count] = 0;

	if ((count > 1) && (cmd[count-1] == '\n'))
		cmd[count-1] = 0;

	q6audio_set_route(cmd);

	return count;
}

static int q6_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations q6_fops = {
	.owner		= THIS_MODULE,
	.open		= q6_open,
	.write		= q6_write,
	.release	= q6_release,
};

static struct miscdevice q6_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "msm_audio_route",
	.fops	= &q6_fops,
};


static int __init q6_init(void)
{
	return misc_register(&q6_misc);
}

device_initcall(q6_init);
