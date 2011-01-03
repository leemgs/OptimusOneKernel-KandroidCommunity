

#include <linux/namei.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/idr.h>
#include <linux/major.h>
#include <linux/file.h>

#include <scsi/scsi.h>
#include <scsi/scsi_driver.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_ioctl.h>

#include <scsi/osd_initiator.h>
#include <scsi/osd_sec.h>

#include "osd_debug.h"

#ifndef TYPE_OSD
#  define TYPE_OSD 0x11
#endif

#ifndef SCSI_OSD_MAJOR
#  define SCSI_OSD_MAJOR 260
#endif
#define SCSI_OSD_MAX_MINOR 64

static const char osd_name[] = "osd";
static const char *osd_version_string = "open-osd 0.1.0";
const char osd_symlink[] = "scsi_osd";

MODULE_AUTHOR("Boaz Harrosh <bharrosh@panasas.com>");
MODULE_DESCRIPTION("open-osd Upper-Layer-Driver osd.ko");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV_MAJOR(SCSI_OSD_MAJOR);
MODULE_ALIAS_SCSI_DEVICE(TYPE_OSD);

struct osd_uld_device {
	int minor;
	struct kref kref;
	struct cdev cdev;
	struct osd_dev od;
	struct gendisk *disk;
	struct device *class_member;
};

static void __uld_get(struct osd_uld_device *oud);
static void __uld_put(struct osd_uld_device *oud);



static int osd_uld_open(struct inode *inode, struct file *file)
{
	struct osd_uld_device *oud = container_of(inode->i_cdev,
					struct osd_uld_device, cdev);

	__uld_get(oud);
	
	file->private_data = oud;
	OSD_DEBUG("osd_uld_open %p\n", oud);
	return 0;
}

static int osd_uld_release(struct inode *inode, struct file *file)
{
	struct osd_uld_device *oud = file->private_data;

	OSD_DEBUG("osd_uld_release %p\n", file->private_data);
	file->private_data = NULL;
	__uld_put(oud);
	return 0;
}


unsigned g_test_ioctl;
do_test_fn *g_do_test;

int osduld_register_test(unsigned ioctl, do_test_fn *do_test)
{
	if (g_test_ioctl)
		return -EINVAL;

	g_test_ioctl = ioctl;
	g_do_test = do_test;
	return 0;
}
EXPORT_SYMBOL(osduld_register_test);

void osduld_unregister_test(unsigned ioctl)
{
	if (ioctl == g_test_ioctl) {
		g_test_ioctl = 0;
		g_do_test = NULL;
	}
}
EXPORT_SYMBOL(osduld_unregister_test);

static do_test_fn *_find_ioctl(unsigned cmd)
{
	if (g_test_ioctl == cmd)
		return g_do_test;
	else
		return NULL;
}

static long osd_uld_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	struct osd_uld_device *oud = file->private_data;
	int ret;
	do_test_fn *do_test;

	do_test = _find_ioctl(cmd);
	if (do_test)
		ret = do_test(&oud->od, cmd, arg);
	else {
		OSD_ERR("Unknown ioctl %d: osd_uld_device=%p\n", cmd, oud);
		ret = -ENOIOCTLCMD;
	}
	return ret;
}

static const struct file_operations osd_fops = {
	.owner          = THIS_MODULE,
	.open           = osd_uld_open,
	.release        = osd_uld_release,
	.unlocked_ioctl = osd_uld_ioctl,
};

struct osd_dev *osduld_path_lookup(const char *name)
{
	struct osd_uld_device *oud;
	struct osd_dev *od;
	struct file *file;
	int error;

	if (!name || !*name) {
		OSD_ERR("Mount with !path || !*path\n");
		return ERR_PTR(-EINVAL);
	}

	od = kzalloc(sizeof(*od), GFP_KERNEL);
	if (!od)
		return ERR_PTR(-ENOMEM);

	file = filp_open(name, O_RDWR, 0);
	if (IS_ERR(file)) {
		error = PTR_ERR(file);
		goto free_od;
	}

	if (file->f_op != &osd_fops){
		error = -EINVAL;
		goto close_file;
	}

	oud = file->private_data;

	*od = oud->od;
	od->file = file;

	return od;

close_file:
	fput(file);
free_od:
	kfree(od);
	return ERR_PTR(error);
}
EXPORT_SYMBOL(osduld_path_lookup);

void osduld_put_device(struct osd_dev *od)
{

	if (od && !IS_ERR(od)) {
		struct osd_uld_device *oud = od->file->private_data;

		BUG_ON(od->scsi_device != oud->od.scsi_device);

		fput(od->file);
		kfree(od);
	}
}
EXPORT_SYMBOL(osduld_put_device);



static int __detect_osd(struct osd_uld_device *oud)
{
	struct scsi_device *scsi_device = oud->od.scsi_device;
	char caps[OSD_CAP_LEN];
	int error;

	
	OSD_DEBUG("start scsi_test_unit_ready %p %p %p\n",
			oud, scsi_device, scsi_device->request_queue);
	error = scsi_test_unit_ready(scsi_device, 10*HZ, 5, NULL);
	if (error)
		OSD_ERR("warning: scsi_test_unit_ready failed\n");

	osd_sec_init_nosec_doall_caps(caps, &osd_root_object, false, true);
	if (osd_auto_detect_ver(&oud->od, caps))
		return -ENODEV;

	return 0;
}

static struct class *osd_sysfs_class;
static DEFINE_IDA(osd_minor_ida);

static int osd_probe(struct device *dev)
{
	struct scsi_device *scsi_device = to_scsi_device(dev);
	struct gendisk *disk;
	struct osd_uld_device *oud;
	int minor;
	int error;

	if (scsi_device->type != TYPE_OSD)
		return -ENODEV;

	do {
		if (!ida_pre_get(&osd_minor_ida, GFP_KERNEL))
			return -ENODEV;

		error = ida_get_new(&osd_minor_ida, &minor);
	} while (error == -EAGAIN);

	if (error)
		return error;
	if (minor >= SCSI_OSD_MAX_MINOR) {
		error = -EBUSY;
		goto err_retract_minor;
	}

	error = -ENOMEM;
	oud = kzalloc(sizeof(*oud), GFP_KERNEL);
	if (NULL == oud)
		goto err_retract_minor;

	kref_init(&oud->kref);
	dev_set_drvdata(dev, oud);
	oud->minor = minor;

	
	
	disk = alloc_disk(1);
	if (!disk) {
		OSD_ERR("alloc_disk failed\n");
		goto err_free_osd;
	}
	disk->major = SCSI_OSD_MAJOR;
	disk->first_minor = oud->minor;
	sprintf(disk->disk_name, "osd%d", oud->minor);
	oud->disk = disk;

	
	scsi_device_get(scsi_device);
	osd_dev_init(&oud->od, scsi_device);

	
	error = __detect_osd(oud);
	if (error) {
		OSD_ERR("osd detection failed, non-compatible OSD device\n");
		goto err_put_disk;
	}

	
	cdev_init(&oud->cdev, &osd_fops);
	oud->cdev.owner = THIS_MODULE;
	error = cdev_add(&oud->cdev,
			 MKDEV(SCSI_OSD_MAJOR, oud->minor), 1);
	if (error) {
		OSD_ERR("cdev_add failed\n");
		goto err_put_disk;
	}
	kobject_get(&oud->cdev.kobj); 

	
	oud->class_member = device_create(osd_sysfs_class, dev,
		MKDEV(SCSI_OSD_MAJOR, oud->minor), "%s", disk->disk_name);
	if (IS_ERR(oud->class_member)) {
		OSD_ERR("class_device_create failed\n");
		error = PTR_ERR(oud->class_member);
		goto err_put_cdev;
	}

	dev_set_drvdata(oud->class_member, oud);

	OSD_INFO("osd_probe %s\n", disk->disk_name);
	return 0;

err_put_cdev:
	cdev_del(&oud->cdev);
err_put_disk:
	scsi_device_put(scsi_device);
	put_disk(disk);
err_free_osd:
	dev_set_drvdata(dev, NULL);
	kfree(oud);
err_retract_minor:
	ida_remove(&osd_minor_ida, minor);
	return error;
}

static int osd_remove(struct device *dev)
{
	struct scsi_device *scsi_device = to_scsi_device(dev);
	struct osd_uld_device *oud = dev_get_drvdata(dev);

	if (!oud || (oud->od.scsi_device != scsi_device)) {
		OSD_ERR("Half cooked osd-device %p,%p || %p!=%p",
			dev, oud, oud ? oud->od.scsi_device : NULL,
			scsi_device);
	}

	if (oud->class_member)
		device_destroy(osd_sysfs_class,
			       MKDEV(SCSI_OSD_MAJOR, oud->minor));

	
	if (oud->cdev.owner)
		cdev_del(&oud->cdev);

	__uld_put(oud);
	return 0;
}

static void __remove(struct kref *kref)
{
	struct osd_uld_device *oud = container_of(kref,
					struct osd_uld_device, kref);
	struct scsi_device *scsi_device = oud->od.scsi_device;

	
	kobject_put(&oud->cdev.kobj);

	osd_dev_fini(&oud->od);
	scsi_device_put(scsi_device);

	OSD_INFO("osd_remove %s\n",
		 oud->disk ? oud->disk->disk_name : NULL);

	if (oud->disk)
		put_disk(oud->disk);

	ida_remove(&osd_minor_ida, oud->minor);
	kfree(oud);
}

static void __uld_get(struct osd_uld_device *oud)
{
	kref_get(&oud->kref);
}

static void __uld_put(struct osd_uld_device *oud)
{
	kref_put(&oud->kref, __remove);
}



static struct scsi_driver osd_driver = {
	.owner			= THIS_MODULE,
	.gendrv = {
		.name		= osd_name,
		.probe		= osd_probe,
		.remove		= osd_remove,
	}
};

static int __init osd_uld_init(void)
{
	int err;

	osd_sysfs_class = class_create(THIS_MODULE, osd_symlink);
	if (IS_ERR(osd_sysfs_class)) {
		OSD_ERR("Unable to register sysfs class => %ld\n",
			PTR_ERR(osd_sysfs_class));
		return PTR_ERR(osd_sysfs_class);
	}

	err = register_chrdev_region(MKDEV(SCSI_OSD_MAJOR, 0),
				     SCSI_OSD_MAX_MINOR, osd_name);
	if (err) {
		OSD_ERR("Unable to register major %d for osd ULD => %d\n",
			SCSI_OSD_MAJOR, err);
		goto err_out;
	}

	err = scsi_register_driver(&osd_driver.gendrv);
	if (err) {
		OSD_ERR("scsi_register_driver failed => %d\n", err);
		goto err_out_chrdev;
	}

	OSD_INFO("LOADED %s\n", osd_version_string);
	return 0;

err_out_chrdev:
	unregister_chrdev_region(MKDEV(SCSI_OSD_MAJOR, 0), SCSI_OSD_MAX_MINOR);
err_out:
	class_destroy(osd_sysfs_class);
	return err;
}

static void __exit osd_uld_exit(void)
{
	scsi_unregister_driver(&osd_driver.gendrv);
	unregister_chrdev_region(MKDEV(SCSI_OSD_MAJOR, 0), SCSI_OSD_MAX_MINOR);
	class_destroy(osd_sysfs_class);
	OSD_INFO("UNLOADED %s\n", osd_version_string);
}

module_init(osd_uld_init);
module_exit(osd_uld_exit);
