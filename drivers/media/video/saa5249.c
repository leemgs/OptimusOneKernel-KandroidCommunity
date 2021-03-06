

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/videotext.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-i2c-drv.h>

MODULE_AUTHOR("Michael Geng <linux@MichaelGeng.de>");
MODULE_DESCRIPTION("Philips SAA5249 Teletext decoder driver");
MODULE_LICENSE("GPL");


#define VTX_VER_MAJ 1
#define VTX_VER_MIN 8


#define NUM_DAUS 4
#define NUM_BUFS 8

static const int disp_modes[8][3] =
{
	{ 0x46, 0x03, 0x03 },	
	{ 0x46, 0xcc, 0xcc },	
	{ 0x44, 0x0f, 0x0f },	
	{ 0x46, 0xcc, 0x46 },	
	{ 0x44, 0x03, 0x03 },	
	{ 0x44, 0xcc, 0xcc },	
	{ 0x44, 0x0f, 0x0f },	
	{ 0x44, 0xcc, 0x46 }	
};



#define PAGE_WAIT    msecs_to_jiffies(300)	
						
#define PGBUF_EXPIRE msecs_to_jiffies(15000)	
						
typedef struct {
	u8 pgbuf[VTX_VIRTUALSIZE];		
	u8 laststat[10];			
	u8 sregs[7];				
	unsigned long expire;			
	unsigned clrfound : 1;			
	unsigned stopped : 1;			
} vdau_t;

struct saa5249_device
{
	struct v4l2_subdev sd;
	struct video_device *vdev;
	vdau_t vdau[NUM_DAUS];			
						
	int vtx_use_count;
	int is_searching[NUM_DAUS];
	int disp_mode;
	int virtual_mode;
	unsigned long in_use;
	struct mutex lock;
};

static inline struct saa5249_device *to_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct saa5249_device, sd);
}


#define CCTWR 34		
#define CCTRD 35
#define NOACK_REPEAT 10		
#define CLEAR_DELAY   msecs_to_jiffies(50)	
#define READY_TIMEOUT msecs_to_jiffies(30)	
#define INIT_DELAY 500		
#define START_DELAY 10		

#define VTX_DEV_MINOR 0

static struct video_device saa_template;	



static void jdelay(unsigned long delay)
{
	sigset_t oldblocked = current->blocked;

	spin_lock_irq(&current->sighand->siglock);
	sigfillset(&current->blocked);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	msleep_interruptible(jiffies_to_msecs(delay));

	spin_lock_irq(&current->sighand->siglock);
	current->blocked = oldblocked;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
}




static int i2c_sendbuf(struct saa5249_device *t, int reg, int count, u8 *data)
{
	struct i2c_client *client = v4l2_get_subdevdata(&t->sd);
	char buf[64];

	buf[0] = reg;
	memcpy(buf+1, data, count);

	if (i2c_master_send(client, buf, count + 1) == count + 1)
		return 0;
	return -1;
}

static int i2c_senddata(struct saa5249_device *t, ...)
{
	unsigned char buf[64];
	int v;
	int ct = 0;
	va_list argp;
	va_start(argp,t);

	while ((v = va_arg(argp, int)) != -1)
		buf[ct++] = v;

	va_end(argp);
	return i2c_sendbuf(t, buf[0], ct-1, buf+1);
}



static int i2c_getdata(struct saa5249_device *t, int count, u8 *buf)
{
	struct i2c_client *client = v4l2_get_subdevdata(&t->sd);

	if (i2c_master_recv(client, buf, count) != count)
		return -1;
	return 0;
}




static long do_saa5249_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	static int virtual_mode = false;
	struct saa5249_device *t = video_drvdata(file);

	switch (cmd) {
	case VTXIOCGETINFO:
	{
		vtx_info_t *info = arg;
		info->version_major = VTX_VER_MAJ;
		info->version_minor = VTX_VER_MIN;
		info->numpages = NUM_DAUS;
		
		return 0;
	}

	case VTXIOCCLRPAGE:
	{
		vtx_pagereq_t *req = arg;

		if (req->pgbuf < 0 || req->pgbuf >= NUM_DAUS)
			return -EINVAL;
		memset(t->vdau[req->pgbuf].pgbuf, ' ', sizeof(t->vdau[0].pgbuf));
		t->vdau[req->pgbuf].clrfound = true;
		return 0;
	}

	case VTXIOCCLRFOUND:
	{
		vtx_pagereq_t *req = arg;

		if (req->pgbuf < 0 || req->pgbuf >= NUM_DAUS)
			return -EINVAL;
		t->vdau[req->pgbuf].clrfound = true;
		return 0;
	}

	case VTXIOCPAGEREQ:
	{
		vtx_pagereq_t *req = arg;
		if (!(req->pagemask & PGMASK_PAGE))
			req->page = 0;
		if (!(req->pagemask & PGMASK_HOUR))
			req->hour = 0;
		if (!(req->pagemask & PGMASK_MINUTE))
			req->minute = 0;
		if (req->page < 0 || req->page > 0x8ff) 
			return -EINVAL;
		req->page &= 0x7ff;
		if (req->hour < 0 || req->hour > 0x3f || req->minute < 0 || req->minute > 0x7f ||
			req->pagemask < 0 || req->pagemask >= PGMASK_MAX || req->pgbuf < 0 || req->pgbuf >= NUM_DAUS)
			return -EINVAL;
		t->vdau[req->pgbuf].sregs[0] = (req->pagemask & PG_HUND ? 0x10 : 0) | (req->page / 0x100);
		t->vdau[req->pgbuf].sregs[1] = (req->pagemask & PG_TEN ? 0x10 : 0) | ((req->page / 0x10) & 0xf);
		t->vdau[req->pgbuf].sregs[2] = (req->pagemask & PG_UNIT ? 0x10 : 0) | (req->page & 0xf);
		t->vdau[req->pgbuf].sregs[3] = (req->pagemask & HR_TEN ? 0x10 : 0) | (req->hour / 0x10);
		t->vdau[req->pgbuf].sregs[4] = (req->pagemask & HR_UNIT ? 0x10 : 0) | (req->hour & 0xf);
		t->vdau[req->pgbuf].sregs[5] = (req->pagemask & MIN_TEN ? 0x10 : 0) | (req->minute / 0x10);
		t->vdau[req->pgbuf].sregs[6] = (req->pagemask & MIN_UNIT ? 0x10 : 0) | (req->minute & 0xf);
		t->vdau[req->pgbuf].stopped = false;
		t->vdau[req->pgbuf].clrfound = true;
		t->is_searching[req->pgbuf] = true;
		return 0;
	}

	case VTXIOCGETSTAT:
	{
		vtx_pagereq_t *req = arg;
		u8 infobits[10];
		vtx_pageinfo_t info;
		int a;

		if (req->pgbuf < 0 || req->pgbuf >= NUM_DAUS)
			return -EINVAL;
		if (!t->vdau[req->pgbuf].stopped) {
			if (i2c_senddata(t, 2, 0, -1) ||
				i2c_sendbuf(t, 3, sizeof(t->vdau[0].sregs), t->vdau[req->pgbuf].sregs) ||
				i2c_senddata(t, 8, 0, 25, 0, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', -1) ||
				i2c_senddata(t, 2, 0, t->vdau[req->pgbuf].sregs[0] | 8, -1) ||
				i2c_senddata(t, 8, 0, 25, 0, -1))
				return -EIO;
			jdelay(PAGE_WAIT);
			if (i2c_getdata(t, 10, infobits))
				return -EIO;

			if (!(infobits[8] & 0x10) && !(infobits[7] & 0xf0) &&	
				(memcmp(infobits, t->vdau[req->pgbuf].laststat, sizeof(infobits)) ||
				time_after_eq(jiffies, t->vdau[req->pgbuf].expire)))
			{		
				if (i2c_senddata(t, 8, 0, 0, 0, -1) ||
					i2c_getdata(t, VTX_PAGESIZE, t->vdau[req->pgbuf].pgbuf))
					return -EIO;
				t->vdau[req->pgbuf].expire = jiffies + PGBUF_EXPIRE;
				memset(t->vdau[req->pgbuf].pgbuf + VTX_PAGESIZE, ' ', VTX_VIRTUALSIZE - VTX_PAGESIZE);
				if (t->virtual_mode) {
					
					if (i2c_senddata(t, 8, 0, 0x20, 0, -1) ||
						i2c_getdata(t, 40, t->vdau[req->pgbuf].pgbuf + VTX_PAGESIZE + 20 * 40))
						return -EIO;
					
					if (i2c_senddata(t, 8, 0, 0x21, 0, -1) ||
						i2c_getdata(t, 40, t->vdau[req->pgbuf].pgbuf + VTX_PAGESIZE + 16 * 40))
						return -EIO;
					
					if (i2c_senddata(t, 8, 0, 0x22, 0, -1) ||
						i2c_getdata(t, 40, t->vdau[req->pgbuf].pgbuf + VTX_PAGESIZE + 23 * 40))
						return -EIO;
				}
				t->vdau[req->pgbuf].clrfound = false;
				memcpy(t->vdau[req->pgbuf].laststat, infobits, sizeof(infobits));
			} else {
				memcpy(infobits, t->vdau[req->pgbuf].laststat, sizeof(infobits));
			}
		} else {
			memcpy(infobits, t->vdau[req->pgbuf].laststat, sizeof(infobits));
		}

		info.pagenum = ((infobits[8] << 8) & 0x700) | ((infobits[1] << 4) & 0xf0) | (infobits[0] & 0x0f);
		if (info.pagenum < 0x100)
			info.pagenum += 0x800;
		info.hour = ((infobits[5] << 4) & 0x30) | (infobits[4] & 0x0f);
		info.minute = ((infobits[3] << 4) & 0x70) | (infobits[2] & 0x0f);
		info.charset = ((infobits[7] >> 1) & 7);
		info.delete = !!(infobits[3] & 8);
		info.headline = !!(infobits[5] & 4);
		info.subtitle = !!(infobits[5] & 8);
		info.supp_header = !!(infobits[6] & 1);
		info.update = !!(infobits[6] & 2);
		info.inter_seq = !!(infobits[6] & 4);
		info.dis_disp = !!(infobits[6] & 8);
		info.serial = !!(infobits[7] & 1);
		info.notfound = !!(infobits[8] & 0x10);
		info.pblf = !!(infobits[9] & 0x20);
		info.hamming = 0;
		for (a = 0; a <= 7; a++) {
			if (infobits[a] & 0xf0) {
				info.hamming = 1;
				break;
			}
		}
		if (t->vdau[req->pgbuf].clrfound)
			info.notfound = 1;
		if (copy_to_user(req->buffer, &info, sizeof(vtx_pageinfo_t)))
			return -EFAULT;
		if (!info.hamming && !info.notfound)
			t->is_searching[req->pgbuf] = false;
		return 0;
	}

	case VTXIOCGETPAGE:
	{
		vtx_pagereq_t *req = arg;
		int start, end;

		if (req->pgbuf < 0 || req->pgbuf >= NUM_DAUS || req->start < 0 ||
			req->start > req->end || req->end >= (virtual_mode ? VTX_VIRTUALSIZE : VTX_PAGESIZE))
			return -EINVAL;
		if (copy_to_user(req->buffer, &t->vdau[req->pgbuf].pgbuf[req->start], req->end - req->start + 1))
			return -EFAULT;

		 

		if (req->start <= 39 && req->end >= 32) {
			int len;
			char buf[16];
			start = max(req->start, 32);
			end = min(req->end, 39);
			len = end - start + 1;
			if (i2c_senddata(t, 8, 0, 0, start, -1) ||
				i2c_getdata(t, len, buf))
				return -EIO;
			if (copy_to_user(req->buffer + start - req->start, buf, len))
				return -EFAULT;
		}
		
		if (req->start <= 31 && req->end >= 7 && t->is_searching[req->pgbuf]) {
			char buf[32];
			int len;

			start = max(req->start, 7);
			end = min(req->end, 31);
			len = end - start + 1;
			if (i2c_senddata(t, 8, 0, 0, start, -1) ||
				i2c_getdata(t, len, buf))
				return -EIO;
			if (copy_to_user(req->buffer + start - req->start, buf, len))
				return -EFAULT;
		}
		return 0;
	}

	case VTXIOCSTOPDAU:
	{
		vtx_pagereq_t *req = arg;

		if (req->pgbuf < 0 || req->pgbuf >= NUM_DAUS)
			return -EINVAL;
		t->vdau[req->pgbuf].stopped = true;
		t->is_searching[req->pgbuf] = false;
		return 0;
	}

	case VTXIOCPUTPAGE:
	case VTXIOCSETDISP:
	case VTXIOCPUTSTAT:
		return 0;

	case VTXIOCCLRCACHE:
	{
		if (i2c_senddata(t, 0, NUM_DAUS, 0, 8, -1) || i2c_senddata(t, 11,
			' ', ' ', ' ', ' ', ' ', ' ',
			' ', ' ', ' ', ' ', ' ', ' ',
			' ', ' ', ' ', ' ', ' ', ' ',
			' ', ' ', ' ', ' ', ' ', ' ',
			-1))
			return -EIO;
		if (i2c_senddata(t, 3, 0x20, -1))
			return -EIO;
		jdelay(10 * CLEAR_DELAY);			
		return 0;
	}

	case VTXIOCSETVIRT:
	{
		
		t->virtual_mode = (int)(long)arg;
		return 0;
	}
	}
	return -EINVAL;
}


static inline unsigned int vtx_fix_command(unsigned int cmd)
{
	switch (cmd) {
	case VTXIOCGETINFO_OLD:
		cmd = VTXIOCGETINFO;
		break;
	case VTXIOCCLRPAGE_OLD:
		cmd = VTXIOCCLRPAGE;
		break;
	case VTXIOCCLRFOUND_OLD:
		cmd = VTXIOCCLRFOUND;
		break;
	case VTXIOCPAGEREQ_OLD:
		cmd = VTXIOCPAGEREQ;
		break;
	case VTXIOCGETSTAT_OLD:
		cmd = VTXIOCGETSTAT;
		break;
	case VTXIOCGETPAGE_OLD:
		cmd = VTXIOCGETPAGE;
		break;
	case VTXIOCSTOPDAU_OLD:
		cmd = VTXIOCSTOPDAU;
		break;
	case VTXIOCPUTPAGE_OLD:
		cmd = VTXIOCPUTPAGE;
		break;
	case VTXIOCSETDISP_OLD:
		cmd = VTXIOCSETDISP;
		break;
	case VTXIOCPUTSTAT_OLD:
		cmd = VTXIOCPUTSTAT;
		break;
	case VTXIOCCLRCACHE_OLD:
		cmd = VTXIOCCLRCACHE;
		break;
	case VTXIOCSETVIRT_OLD:
		cmd = VTXIOCSETVIRT;
		break;
	}
	return cmd;
}



static long saa5249_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	struct saa5249_device *t = video_drvdata(file);
	long err;

	cmd = vtx_fix_command(cmd);
	mutex_lock(&t->lock);
	err = video_usercopy(file, cmd, arg, do_saa5249_ioctl);
	mutex_unlock(&t->lock);
	return err;
}

static int saa5249_open(struct file *file)
{
	struct saa5249_device *t = video_drvdata(file);
	int pgbuf;

	if (test_and_set_bit(0, &t->in_use))
		return -EBUSY;

	if (i2c_senddata(t, 0, 0, -1) || 
		
		i2c_senddata(t, 1, disp_modes[t->disp_mode][0], 0, -1) ||
		
		i2c_senddata(t, 4, NUM_DAUS, disp_modes[t->disp_mode][1], disp_modes[t->disp_mode][2], 7, -1))
		
	{
		clear_bit(0, &t->in_use);
		return -EIO;
	}

	for (pgbuf = 0; pgbuf < NUM_DAUS; pgbuf++) {
		memset(t->vdau[pgbuf].pgbuf, ' ', sizeof(t->vdau[0].pgbuf));
		memset(t->vdau[pgbuf].sregs, 0, sizeof(t->vdau[0].sregs));
		memset(t->vdau[pgbuf].laststat, 0, sizeof(t->vdau[0].laststat));
		t->vdau[pgbuf].expire = 0;
		t->vdau[pgbuf].clrfound = true;
		t->vdau[pgbuf].stopped = true;
		t->is_searching[pgbuf] = false;
	}
	t->virtual_mode = false;
	return 0;
}



static int saa5249_release(struct file *file)
{
	struct saa5249_device *t = video_drvdata(file);

	i2c_senddata(t, 1, 0x20, -1);		
	i2c_senddata(t, 5, 3, 3, -1);		
	clear_bit(0, &t->in_use);
	return 0;
}

static const struct v4l2_file_operations saa_fops = {
	.owner		= THIS_MODULE,
	.open		= saa5249_open,
	.release       	= saa5249_release,
	.ioctl          = saa5249_ioctl,
};

static struct video_device saa_template =
{
	.name		= "saa5249",
	.fops           = &saa_fops,
	.release 	= video_device_release,
};

static int saa5249_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_SAA5249, 0);
}

static const struct v4l2_subdev_core_ops saa5249_core_ops = {
	.g_chip_ident = saa5249_g_chip_ident,
};

static const struct v4l2_subdev_ops saa5249_ops = {
	.core = &saa5249_core_ops,
};

static int saa5249_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int pgbuf;
	int err;
	struct saa5249_device *t;
	struct v4l2_subdev *sd;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);
	v4l_info(client, "VideoText version %d.%d\n",
			VTX_VER_MAJ, VTX_VER_MIN);
	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (t == NULL)
		return -ENOMEM;
	sd = &t->sd;
	v4l2_i2c_subdev_init(sd, client, &saa5249_ops);
	mutex_init(&t->lock);

	
	t->vdev = video_device_alloc();
	if (t->vdev == NULL) {
		kfree(t);
		kfree(client);
		return -ENOMEM;
	}
	memcpy(t->vdev, &saa_template, sizeof(*t->vdev));

	for (pgbuf = 0; pgbuf < NUM_DAUS; pgbuf++) {
		memset(t->vdau[pgbuf].pgbuf, ' ', sizeof(t->vdau[0].pgbuf));
		memset(t->vdau[pgbuf].sregs, 0, sizeof(t->vdau[0].sregs));
		memset(t->vdau[pgbuf].laststat, 0, sizeof(t->vdau[0].laststat));
		t->vdau[pgbuf].expire = 0;
		t->vdau[pgbuf].clrfound = true;
		t->vdau[pgbuf].stopped = true;
		t->is_searching[pgbuf] = false;
	}
	video_set_drvdata(t->vdev, t);

	
	err = video_register_device(t->vdev, VFL_TYPE_VTX, -1);
	if (err < 0) {
		video_device_release(t->vdev);
		kfree(t);
		return err;
	}
	return 0;
}

static int saa5249_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct saa5249_device *t = to_dev(sd);

	video_unregister_device(t->vdev);
	v4l2_device_unregister_subdev(sd);
	kfree(t);
	return 0;
}

static const struct i2c_device_id saa5249_id[] = {
	{ "saa5249", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, saa5249_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "saa5249",
	.probe = saa5249_probe,
	.remove = saa5249_remove,
	.id_table = saa5249_id,
};
