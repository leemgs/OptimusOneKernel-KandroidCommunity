









#ifndef _LINUX_I2C_H
#define _LINUX_I2C_H

#include <linux/types.h>
#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/i2c-id.h>
#include <linux/mod_devicetable.h>
#include <linux/device.h>	
#include <linux/sched.h>	
#include <linux/mutex.h>

extern struct bus_type i2c_bus_type;



struct i2c_msg;
struct i2c_algorithm;
struct i2c_adapter;
struct i2c_client;
struct i2c_driver;
union i2c_smbus_data;
struct i2c_board_info;

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)

extern int i2c_master_send(struct i2c_client *client, const char *buf,
			   int count);
extern int i2c_master_recv(struct i2c_client *client, char *buf, int count);


extern int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			int num);


extern s32 i2c_smbus_xfer(struct i2c_adapter *adapter, u16 addr,
			  unsigned short flags, char read_write, u8 command,
			  int size, union i2c_smbus_data *data);



extern s32 i2c_smbus_read_byte(struct i2c_client *client);
extern s32 i2c_smbus_write_byte(struct i2c_client *client, u8 value);
extern s32 i2c_smbus_read_byte_data(struct i2c_client *client, u8 command);
extern s32 i2c_smbus_write_byte_data(struct i2c_client *client,
				     u8 command, u8 value);
extern s32 i2c_smbus_read_word_data(struct i2c_client *client, u8 command);
extern s32 i2c_smbus_write_word_data(struct i2c_client *client,
				     u8 command, u16 value);

extern s32 i2c_smbus_read_block_data(struct i2c_client *client,
				     u8 command, u8 *values);
extern s32 i2c_smbus_write_block_data(struct i2c_client *client,
				      u8 command, u8 length, const u8 *values);

extern s32 i2c_smbus_read_i2c_block_data(struct i2c_client *client,
					 u8 command, u8 length, u8 *values);
extern s32 i2c_smbus_write_i2c_block_data(struct i2c_client *client,
					  u8 command, u8 length,
					  const u8 *values);
#endif 


struct i2c_driver {
	unsigned int class;

	
	int (*attach_adapter)(struct i2c_adapter *);
	int (*detach_adapter)(struct i2c_adapter *);

	
	int (*probe)(struct i2c_client *, const struct i2c_device_id *);
	int (*remove)(struct i2c_client *);

	
	void (*shutdown)(struct i2c_client *);
	int (*suspend)(struct i2c_client *, pm_message_t mesg);
	int (*resume)(struct i2c_client *);

	
	int (*command)(struct i2c_client *client, unsigned int cmd, void *arg);

	struct device_driver driver;
	const struct i2c_device_id *id_table;

	
	int (*detect)(struct i2c_client *, int kind, struct i2c_board_info *);
	const struct i2c_client_address_data *address_data;
	struct list_head clients;
};
#define to_i2c_driver(d) container_of(d, struct i2c_driver, driver)


struct i2c_client {
	unsigned short flags;		
	unsigned short addr;		
					
					
	char name[I2C_NAME_SIZE];
	struct i2c_adapter *adapter;	
	struct i2c_driver *driver;	
	struct device dev;		
	int irq;			
	struct list_head detected;
};
#define to_i2c_client(d) container_of(d, struct i2c_client, dev)

extern struct i2c_client *i2c_verify_client(struct device *dev);

static inline struct i2c_client *kobj_to_i2c_client(struct kobject *kobj)
{
	struct device * const dev = container_of(kobj, struct device, kobj);
	return to_i2c_client(dev);
}

static inline void *i2c_get_clientdata(const struct i2c_client *dev)
{
	return dev_get_drvdata(&dev->dev);
}

static inline void i2c_set_clientdata(struct i2c_client *dev, void *data)
{
	dev_set_drvdata(&dev->dev, data);
}


struct i2c_board_info {
	char		type[I2C_NAME_SIZE];
	unsigned short	flags;
	unsigned short	addr;
	void		*platform_data;
	struct dev_archdata	*archdata;
	int		irq;
};


#define I2C_BOARD_INFO(dev_type, dev_addr) \
	.type = dev_type, .addr = (dev_addr)


#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)

extern struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info);


extern struct i2c_client *
i2c_new_probed_device(struct i2c_adapter *adap,
		      struct i2c_board_info *info,
		      unsigned short const *addr_list);


extern struct i2c_client *
i2c_new_dummy(struct i2c_adapter *adap, u16 address);

extern void i2c_unregister_device(struct i2c_client *);
#endif 


#ifdef CONFIG_I2C_BOARDINFO
extern int
i2c_register_board_info(int busnum, struct i2c_board_info const *info,
			unsigned n);
#else
static inline int
i2c_register_board_info(int busnum, struct i2c_board_info const *info,
			unsigned n)
{
	return 0;
}
#endif 


struct i2c_algorithm {
	
	
	int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
			   int num);
	int (*smbus_xfer) (struct i2c_adapter *adap, u16 addr,
			   unsigned short flags, char read_write,
			   u8 command, int size, union i2c_smbus_data *data);

	
	u32 (*functionality) (struct i2c_adapter *);
};


struct i2c_adapter {
	struct module *owner;
	unsigned int id;
	unsigned int class;		  
	const struct i2c_algorithm *algo; 
	void *algo_data;

	
	u8 level; 			
	struct mutex bus_lock;

	int timeout;			
	int retries;
	struct device dev;		

	int nr;
	char name[48];
	struct completion dev_released;
};
#define to_i2c_adapter(d) container_of(d, struct i2c_adapter, dev)

static inline void *i2c_get_adapdata(const struct i2c_adapter *dev)
{
	return dev_get_drvdata(&dev->dev);
}

static inline void i2c_set_adapdata(struct i2c_adapter *dev, void *data)
{
	dev_set_drvdata(&dev->dev, data);
}


static inline void i2c_lock_adapter(struct i2c_adapter *adapter)
{
	mutex_lock(&adapter->bus_lock);
}


static inline void i2c_unlock_adapter(struct i2c_adapter *adapter)
{
	mutex_unlock(&adapter->bus_lock);
}


#define I2C_CLIENT_PEC	0x04		
#define I2C_CLIENT_TEN	0x10		
					
#define I2C_CLIENT_WAKE	0x80		


#define I2C_CLASS_HWMON		(1<<0)	
#define I2C_CLASS_TV_ANALOG	(1<<1)	
#define I2C_CLASS_TV_DIGITAL	(1<<2)	
#define I2C_CLASS_DDC		(1<<3)	
#define I2C_CLASS_SPD		(1<<7)	


struct i2c_client_address_data {
	const unsigned short *normal_i2c;
	const unsigned short *probe;
	const unsigned short *ignore;
	const unsigned short * const *forces;
};


#define I2C_CLIENT_END		0xfffeU


#define ANY_I2C_BUS		0xffff


#define I2C_ADDRS(addr, addrs...) \
	((const unsigned short []){ addr, ## addrs, I2C_CLIENT_END })





#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
extern int i2c_add_adapter(struct i2c_adapter *);
extern int i2c_del_adapter(struct i2c_adapter *);
extern int i2c_add_numbered_adapter(struct i2c_adapter *);

extern int i2c_register_driver(struct module *, struct i2c_driver *);
extern void i2c_del_driver(struct i2c_driver *);

static inline int i2c_add_driver(struct i2c_driver *driver)
{
	return i2c_register_driver(THIS_MODULE, driver);
}

extern struct i2c_client *i2c_use_client(struct i2c_client *client);
extern void i2c_release_client(struct i2c_client *client);


extern void i2c_clients_command(struct i2c_adapter *adap,
				unsigned int cmd, void *arg);

extern struct i2c_adapter *i2c_get_adapter(int id);
extern void i2c_put_adapter(struct i2c_adapter *adap);



static inline u32 i2c_get_functionality(struct i2c_adapter *adap)
{
	return adap->algo->functionality(adap);
}


static inline int i2c_check_functionality(struct i2c_adapter *adap, u32 func)
{
	return (func & i2c_get_functionality(adap)) == func;
}


static inline int i2c_adapter_id(struct i2c_adapter *adap)
{
	return adap->nr;
}
#endif 
#endif 


struct i2c_msg {
	__u16 addr;	
	__u16 flags;
#define I2C_M_TEN		0x0010	
#define I2C_M_RD		0x0001	
#define I2C_M_NOSTART		0x4000	
#define I2C_M_REV_DIR_ADDR	0x2000	
#define I2C_M_IGNORE_NAK	0x1000	
#define I2C_M_NO_RD_ACK		0x0800	
#define I2C_M_RECV_LEN		0x0400	
	__u16 len;		
	__u8 *buf;		
};



#define I2C_FUNC_I2C			0x00000001
#define I2C_FUNC_10BIT_ADDR		0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	0x00000004 
#define I2C_FUNC_SMBUS_PEC		0x00000008
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 
#define I2C_FUNC_SMBUS_QUICK		0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 

#define I2C_FUNC_SMBUS_BYTE		(I2C_FUNC_SMBUS_READ_BYTE | \
					 I2C_FUNC_SMBUS_WRITE_BYTE)
#define I2C_FUNC_SMBUS_BYTE_DATA	(I2C_FUNC_SMBUS_READ_BYTE_DATA | \
					 I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
#define I2C_FUNC_SMBUS_WORD_DATA	(I2C_FUNC_SMBUS_READ_WORD_DATA | \
					 I2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define I2C_FUNC_SMBUS_BLOCK_DATA	(I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
					 I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
#define I2C_FUNC_SMBUS_I2C_BLOCK	(I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
					 I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

#define I2C_FUNC_SMBUS_EMUL		(I2C_FUNC_SMBUS_QUICK | \
					 I2C_FUNC_SMBUS_BYTE | \
					 I2C_FUNC_SMBUS_BYTE_DATA | \
					 I2C_FUNC_SMBUS_WORD_DATA | \
					 I2C_FUNC_SMBUS_PROC_CALL | \
					 I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
					 I2C_FUNC_SMBUS_I2C_BLOCK | \
					 I2C_FUNC_SMBUS_PEC)


#define I2C_SMBUS_BLOCK_MAX	32	
union i2c_smbus_data {
	__u8 byte;
	__u16 word;
	__u8 block[I2C_SMBUS_BLOCK_MAX + 2]; 
			       
};


#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0


#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		
#define I2C_SMBUS_I2C_BLOCK_DATA    8


#ifdef __KERNEL__



#define I2C_CLIENT_MAX_OPTS 48


#define I2C_CLIENT_DEFAULTS {I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END, \
			     I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END}



#define I2C_CLIENT_MODULE_PARM(var,desc) \
  static unsigned short var[I2C_CLIENT_MAX_OPTS] = I2C_CLIENT_DEFAULTS; \
  static unsigned int var##_num; \
  module_param_array(var, short, &var##_num, 0); \
  MODULE_PARM_DESC(var, desc)

#define I2C_CLIENT_MODULE_PARM_FORCE(name)				\
I2C_CLIENT_MODULE_PARM(force_##name,					\
		       "List of adapter,address pairs which are "	\
		       "unquestionably assumed to contain a `"		\
		       # name "' chip")


#define I2C_CLIENT_INSMOD_COMMON					\
I2C_CLIENT_MODULE_PARM(probe, "List of adapter,address pairs to scan "	\
		       "additionally");					\
I2C_CLIENT_MODULE_PARM(ignore, "List of adapter,address pairs not to "	\
		       "scan");						\
static const struct i2c_client_address_data addr_data = {		\
	.normal_i2c	= normal_i2c,					\
	.probe		= probe,					\
	.ignore		= ignore,					\
	.forces		= forces,					\
}

#define I2C_CLIENT_FORCE_TEXT \
	"List of adapter,address pairs to boldly assume to be present"


#define I2C_CLIENT_INSMOD						\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
static const unsigned short * const forces[] = { force, NULL };		\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_1(chip1)					\
enum chips { any_chip, chip1 };						\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
static const unsigned short * const forces[] =	{ force,		\
	force_##chip1, NULL };						\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_2(chip1, chip2)				\
enum chips { any_chip, chip1, chip2 };					\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
static const unsigned short * const forces[] =	{ force,		\
	force_##chip1, force_##chip2, NULL };				\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_3(chip1, chip2, chip3)			\
enum chips { any_chip, chip1, chip2, chip3 };				\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
static const unsigned short * const forces[] =	{ force,		\
	force_##chip1, force_##chip2, force_##chip3, NULL };		\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_4(chip1, chip2, chip3, chip4)			\
enum chips { any_chip, chip1, chip2, chip3, chip4 };			\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip4);					\
static const unsigned short * const forces[] =	{ force,		\
	force_##chip1, force_##chip2, force_##chip3,			\
	force_##chip4, NULL};						\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_5(chip1, chip2, chip3, chip4, chip5)		\
enum chips { any_chip, chip1, chip2, chip3, chip4, chip5 };		\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip4);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip5);					\
static const unsigned short * const forces[] = { force,			\
	force_##chip1, force_##chip2, force_##chip3,			\
	force_##chip4, force_##chip5, NULL };				\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_6(chip1, chip2, chip3, chip4, chip5, chip6)	\
enum chips { any_chip, chip1, chip2, chip3, chip4, chip5, chip6 };	\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip4);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip5);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip6);					\
static const unsigned short * const forces[] = { force,			\
	force_##chip1, force_##chip2, force_##chip3,			\
	force_##chip4, force_##chip5, force_##chip6, NULL };		\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_7(chip1, chip2, chip3, chip4, chip5, chip6, chip7) \
enum chips { any_chip, chip1, chip2, chip3, chip4, chip5, chip6,	\
	     chip7 };							\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip4);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip5);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip6);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip7);					\
static const unsigned short * const forces[] = { force,			\
	force_##chip1, force_##chip2, force_##chip3,			\
	force_##chip4, force_##chip5, force_##chip6,			\
	force_##chip7, NULL };						\
I2C_CLIENT_INSMOD_COMMON

#define I2C_CLIENT_INSMOD_8(chip1, chip2, chip3, chip4, chip5, chip6, chip7, chip8) \
enum chips { any_chip, chip1, chip2, chip3, chip4, chip5, chip6,	\
	     chip7, chip8 };						\
I2C_CLIENT_MODULE_PARM(force, I2C_CLIENT_FORCE_TEXT);			\
I2C_CLIENT_MODULE_PARM_FORCE(chip1);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip2);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip3);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip4);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip5);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip6);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip7);					\
I2C_CLIENT_MODULE_PARM_FORCE(chip8);					\
static const unsigned short * const forces[] = { force,			\
	force_##chip1, force_##chip2, force_##chip3,			\
	force_##chip4, force_##chip5, force_##chip6,			\
	force_##chip7, force_##chip8, NULL };				\
I2C_CLIENT_INSMOD_COMMON
#endif 
#endif 
