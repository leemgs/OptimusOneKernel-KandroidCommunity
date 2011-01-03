

#ifndef LINUX_RIO_H
#define LINUX_RIO_H

#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/rio_regs.h>

#define RIO_NO_HOPCOUNT		-1
#define RIO_INVALID_DESTID	0xffff

#define RIO_MAX_MPORT_RESOURCES	16
#define RIO_MAX_DEV_RESOURCES	16

#define RIO_GLOBAL_TABLE	0xff	

#define RIO_INVALID_ROUTE	0xff	

#define RIO_MAX_ROUTE_ENTRIES(size)	(size ? (1 << 16) : (1 << 8))
#define RIO_ANY_DESTID(size)		(size ? 0xffff : 0xff)

#define RIO_MAX_MBOX		4
#define RIO_MAX_MSG_SIZE	0x1000


#define RIO_SUCCESSFUL			0x00
#define RIO_BAD_SIZE			0x81


#define RIO_DOORBELL_RESOURCE	0
#define RIO_INB_MBOX_RESOURCE	1
#define RIO_OUTB_MBOX_RESOURCE	2

extern struct bus_type rio_bus_type;
extern struct list_head rio_devices;	

struct rio_mport;


struct rio_dev {
	struct list_head global_list;	
	struct list_head net_list;	
	struct rio_net *net;	
	u16 did;
	u16 vid;
	u32 device_rev;
	u16 asm_did;
	u16 asm_vid;
	u16 asm_rev;
	u16 efptr;
	u32 pef;
	u32 swpinfo;		
	u32 src_ops;
	u32 dst_ops;
	u64 dma_mask;
	struct rio_switch *rswitch;	
	struct rio_driver *driver;	
	struct device dev;	
	struct resource riores[RIO_MAX_DEV_RESOURCES];
	u16 destid;
};

#define rio_dev_g(n) list_entry(n, struct rio_dev, global_list)
#define rio_dev_f(n) list_entry(n, struct rio_dev, net_list)
#define	to_rio_dev(n) container_of(n, struct rio_dev, dev)


struct rio_msg {
	struct resource *res;
	void (*mcback) (struct rio_mport * mport, void *dev_id, int mbox, int slot);
};


struct rio_dbell {
	struct list_head node;
	struct resource *res;
	void (*dinb) (struct rio_mport *mport, void *dev_id, u16 src, u16 dst, u16 info);
	void *dev_id;
};

enum rio_phy_type {
	RIO_PHY_PARALLEL,
	RIO_PHY_SERIAL,
};


struct rio_mport {
	struct list_head dbells;	
	struct list_head node;	
	struct list_head nnode;	
	struct resource iores;
	struct resource riores[RIO_MAX_MPORT_RESOURCES];
	struct rio_msg inb_msg[RIO_MAX_MBOX];
	struct rio_msg outb_msg[RIO_MAX_MBOX];
	int host_deviceid;	
	struct rio_ops *ops;	
	unsigned char id;	
	unsigned char index;	
	unsigned int sys_size;	
	enum rio_phy_type phy_type;	
	unsigned char name[40];
	void *priv;		
};


struct rio_net {
	struct list_head node;	
	struct list_head devices;	
	struct list_head mports;	
	struct rio_mport *hport;	
	unsigned char id;	
};


struct rio_switch {
	struct list_head node;
	u16 switchid;
	u16 hopcount;
	u16 destid;
	u8 *route_table;
	int (*add_entry) (struct rio_mport * mport, u16 destid, u8 hopcount,
			  u16 table, u16 route_destid, u8 route_port);
	int (*get_entry) (struct rio_mport * mport, u16 destid, u8 hopcount,
			  u16 table, u16 route_destid, u8 * route_port);
};




struct rio_ops {
	int (*lcread) (struct rio_mport *mport, int index, u32 offset, int len,
			u32 *data);
	int (*lcwrite) (struct rio_mport *mport, int index, u32 offset, int len,
			u32 data);
	int (*cread) (struct rio_mport *mport, int index, u16 destid,
			u8 hopcount, u32 offset, int len, u32 *data);
	int (*cwrite) (struct rio_mport *mport, int index, u16 destid,
			u8 hopcount, u32 offset, int len, u32 data);
	int (*dsend) (struct rio_mport *mport, int index, u16 destid, u16 data);
};

#define RIO_RESOURCE_MEM	0x00000100
#define RIO_RESOURCE_DOORBELL	0x00000200
#define RIO_RESOURCE_MAILBOX	0x00000400

#define RIO_RESOURCE_CACHEABLE	0x00010000
#define RIO_RESOURCE_PCI	0x00020000

#define RIO_RESOURCE_BUSY	0x80000000


struct rio_driver {
	struct list_head node;
	char *name;
	const struct rio_device_id *id_table;
	int (*probe) (struct rio_dev * dev, const struct rio_device_id * id);
	void (*remove) (struct rio_dev * dev);
	int (*suspend) (struct rio_dev * dev, u32 state);
	int (*resume) (struct rio_dev * dev);
	int (*enable_wake) (struct rio_dev * dev, u32 state, int enable);
	struct device_driver driver;
};

#define	to_rio_driver(drv) container_of(drv,struct rio_driver, driver)


struct rio_device_id {
	u16 did, vid;
	u16 asm_did, asm_vid;
};


struct rio_route_ops {
	u16 vid, did;
	int (*add_hook) (struct rio_mport * mport, u16 destid, u8 hopcount,
			 u16 table, u16 route_destid, u8 route_port);
	int (*get_hook) (struct rio_mport * mport, u16 destid, u8 hopcount,
			 u16 table, u16 route_destid, u8 * route_port);
};


extern int rio_init_mports(void);
extern void rio_register_mport(struct rio_mport *);
extern int rio_hw_add_outb_message(struct rio_mport *, struct rio_dev *, int,
				   void *, size_t);
extern int rio_hw_add_inb_buffer(struct rio_mport *, int, void *);
extern void *rio_hw_get_inb_message(struct rio_mport *, int);
extern int rio_open_inb_mbox(struct rio_mport *, void *, int, int);
extern void rio_close_inb_mbox(struct rio_mport *, int);
extern int rio_open_outb_mbox(struct rio_mport *, void *, int, int);
extern void rio_close_outb_mbox(struct rio_mport *, int);

#endif				
