

#ifndef __MTD_TRANS_H__
#define __MTD_TRANS_H__

#include <linux/mutex.h>

struct hd_geometry;
struct mtd_info;
struct mtd_blktrans_ops;
struct file;
struct inode;

struct mtd_blktrans_dev {
	struct mtd_blktrans_ops *tr;
	struct list_head list;
	struct mtd_info *mtd;
	struct mutex lock;
	int devnum;
	unsigned long size;
	int readonly;
	void *blkcore_priv; 
};

struct blkcore_priv; 

struct mtd_blktrans_ops {
	char *name;
	int major;
	int part_bits;
	int blksize;
	int blkshift;

	
	int (*readsect)(struct mtd_blktrans_dev *dev,
		    unsigned long block, char *buffer);
	int (*writesect)(struct mtd_blktrans_dev *dev,
		     unsigned long block, char *buffer);
	int (*discard)(struct mtd_blktrans_dev *dev,
		       unsigned long block, unsigned nr_blocks);

	
	int (*getgeo)(struct mtd_blktrans_dev *dev, struct hd_geometry *geo);
	int (*flush)(struct mtd_blktrans_dev *dev);

	
	int (*open)(struct mtd_blktrans_dev *dev);
	int (*release)(struct mtd_blktrans_dev *dev);

	
	void (*add_mtd)(struct mtd_blktrans_ops *tr, struct mtd_info *mtd);
	void (*remove_dev)(struct mtd_blktrans_dev *dev);

	struct list_head devs;
	struct list_head list;
	struct module *owner;

	struct mtd_blkcore_priv *blkcore_priv;
};

extern int register_mtd_blktrans(struct mtd_blktrans_ops *tr);
extern int deregister_mtd_blktrans(struct mtd_blktrans_ops *tr);
extern int add_mtd_blktrans_dev(struct mtd_blktrans_dev *dev);
extern int del_mtd_blktrans_dev(struct mtd_blktrans_dev *dev);


#endif 
