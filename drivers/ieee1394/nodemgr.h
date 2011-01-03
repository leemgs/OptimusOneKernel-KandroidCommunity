

#ifndef _IEEE1394_NODEMGR_H
#define _IEEE1394_NODEMGR_H

#include <linux/device.h>
#include <asm/system.h>
#include <asm/types.h>

#include "ieee1394_core.h"
#include "ieee1394_transactions.h"
#include "ieee1394_types.h"

struct csr1212_csr;
struct csr1212_keyval;
struct hpsb_host;
struct ieee1394_device_id;


struct bus_options {
	u8	irmc;		
	u8	cmc;		
	u8	isc;		
	u8	bmc;		
	u8	pmc;		
	u8	cyc_clk_acc;	
	u8	max_rom;	
	u8	generation;	
	u8	lnkspd;		
	u16	max_rec;	
};

#define UNIT_DIRECTORY_VENDOR_ID		0x01
#define UNIT_DIRECTORY_MODEL_ID			0x02
#define UNIT_DIRECTORY_SPECIFIER_ID		0x04
#define UNIT_DIRECTORY_VERSION			0x08
#define UNIT_DIRECTORY_HAS_LUN_DIRECTORY	0x10
#define UNIT_DIRECTORY_LUN_DIRECTORY		0x20
#define UNIT_DIRECTORY_HAS_LUN			0x40


struct unit_directory {
	struct node_entry *ne;	
	octlet_t address;	
	u8 flags;		

	quadlet_t vendor_id;
	struct csr1212_keyval *vendor_name_kv;

	quadlet_t model_id;
	struct csr1212_keyval *model_name_kv;
	quadlet_t specifier_id;
	quadlet_t version;
	quadlet_t directory_id;

	unsigned int id;

	int ignore_driver;

	int length;		

	struct device device;
	struct device unit_dev;

	struct csr1212_keyval *ud_kv;
	u32 lun;		
};

struct node_entry {
	u64 guid;			
	u32 guid_vendor_id;		

	struct hpsb_host *host;		
	nodeid_t nodeid;		
	struct bus_options busopt;	
	bool needs_probe;
	unsigned int generation;	

	
	u32 vendor_id;
	struct csr1212_keyval *vendor_name_kv;

	u32 capabilities;

	struct device device;
	struct device node_dev;

	
	bool in_limbo;

	struct csr1212_csr *csr;
};

struct hpsb_protocol_driver {
	
	const char *name;

	
	const struct ieee1394_device_id *id_table;

	
	int (*update)(struct unit_directory *ud);

	
	struct device_driver driver;
};

int __hpsb_register_protocol(struct hpsb_protocol_driver *, struct module *);
static inline int hpsb_register_protocol(struct hpsb_protocol_driver *driver)
{
	return __hpsb_register_protocol(driver, THIS_MODULE);
}

void hpsb_unregister_protocol(struct hpsb_protocol_driver *driver);

static inline int hpsb_node_entry_valid(struct node_entry *ne)
{
	return ne->generation == get_hpsb_generation(ne->host);
}
void hpsb_node_fill_packet(struct node_entry *ne, struct hpsb_packet *packet);
int hpsb_node_write(struct node_entry *ne, u64 addr,
		    quadlet_t *buffer, size_t length);
static inline int hpsb_node_read(struct node_entry *ne, u64 addr,
				 quadlet_t *buffer, size_t length)
{
	unsigned int g = ne->generation;

	smp_rmb();
	return hpsb_read(ne->host, ne->nodeid, g, addr, buffer, length);
}
static inline int hpsb_node_lock(struct node_entry *ne, u64 addr, int extcode,
				 quadlet_t *buffer, quadlet_t arg)
{
	unsigned int g = ne->generation;

	smp_rmb();
	return hpsb_lock(ne->host, ne->nodeid, g, addr, extcode, buffer, arg);
}
int nodemgr_for_each_host(void *data, int (*cb)(struct hpsb_host *, void *));

int init_ieee1394_nodemgr(void);
void cleanup_ieee1394_nodemgr(void);


extern struct device nodemgr_dev_template_host;


extern struct bus_attribute *const fw_bus_attrs[];

#endif 
