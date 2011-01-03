

#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/firewire.h>
#include <linux/firewire-constants.h>
#include <linux/idr.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/workqueue.h>

#include <asm/atomic.h>
#include <asm/byteorder.h>
#include <asm/system.h>

#include "core.h"

void fw_csr_iterator_init(struct fw_csr_iterator *ci, u32 * p)
{
	ci->p = p + 1;
	ci->end = ci->p + (p[0] >> 16);
}
EXPORT_SYMBOL(fw_csr_iterator_init);

int fw_csr_iterator_next(struct fw_csr_iterator *ci, int *key, int *value)
{
	*key = *ci->p >> 24;
	*value = *ci->p & 0xffffff;

	return ci->p++ < ci->end;
}
EXPORT_SYMBOL(fw_csr_iterator_next);

static bool is_fw_unit(struct device *dev);

static int match_unit_directory(u32 *directory, u32 match_flags,
				const struct ieee1394_device_id *id)
{
	struct fw_csr_iterator ci;
	int key, value, match;

	match = 0;
	fw_csr_iterator_init(&ci, directory);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		if (key == CSR_VENDOR && value == id->vendor_id)
			match |= IEEE1394_MATCH_VENDOR_ID;
		if (key == CSR_MODEL && value == id->model_id)
			match |= IEEE1394_MATCH_MODEL_ID;
		if (key == CSR_SPECIFIER_ID && value == id->specifier_id)
			match |= IEEE1394_MATCH_SPECIFIER_ID;
		if (key == CSR_VERSION && value == id->version)
			match |= IEEE1394_MATCH_VERSION;
	}

	return (match & match_flags) == match_flags;
}

static int fw_unit_match(struct device *dev, struct device_driver *drv)
{
	struct fw_unit *unit = fw_unit(dev);
	struct fw_device *device;
	const struct ieee1394_device_id *id;

	
	if (!is_fw_unit(dev))
		return 0;

	device = fw_parent_device(unit);
	id = container_of(drv, struct fw_driver, driver)->id_table;

	for (; id->match_flags != 0; id++) {
		if (match_unit_directory(unit->directory, id->match_flags, id))
			return 1;

		
		if ((id->match_flags & IEEE1394_MATCH_VENDOR_ID) &&
		    match_unit_directory(&device->config_rom[5],
				IEEE1394_MATCH_VENDOR_ID, id) &&
		    match_unit_directory(unit->directory, id->match_flags
				& ~IEEE1394_MATCH_VENDOR_ID, id))
			return 1;
	}

	return 0;
}

static int get_modalias(struct fw_unit *unit, char *buffer, size_t buffer_size)
{
	struct fw_device *device = fw_parent_device(unit);
	struct fw_csr_iterator ci;

	int key, value;
	int vendor = 0;
	int model = 0;
	int specifier_id = 0;
	int version = 0;

	fw_csr_iterator_init(&ci, &device->config_rom[5]);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		switch (key) {
		case CSR_VENDOR:
			vendor = value;
			break;
		case CSR_MODEL:
			model = value;
			break;
		}
	}

	fw_csr_iterator_init(&ci, unit->directory);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		switch (key) {
		case CSR_SPECIFIER_ID:
			specifier_id = value;
			break;
		case CSR_VERSION:
			version = value;
			break;
		}
	}

	return snprintf(buffer, buffer_size,
			"ieee1394:ven%08Xmo%08Xsp%08Xver%08X",
			vendor, model, specifier_id, version);
}

static int fw_unit_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct fw_unit *unit = fw_unit(dev);
	char modalias[64];

	get_modalias(unit, modalias, sizeof(modalias));

	if (add_uevent_var(env, "MODALIAS=%s", modalias))
		return -ENOMEM;

	return 0;
}

struct bus_type fw_bus_type = {
	.name = "firewire",
	.match = fw_unit_match,
};
EXPORT_SYMBOL(fw_bus_type);

int fw_device_enable_phys_dma(struct fw_device *device)
{
	int generation = device->generation;

	
	smp_rmb();

	return device->card->driver->enable_phys_dma(device->card,
						     device->node_id,
						     generation);
}
EXPORT_SYMBOL(fw_device_enable_phys_dma);

struct config_rom_attribute {
	struct device_attribute attr;
	u32 key;
};

static ssize_t show_immediate(struct device *dev,
			      struct device_attribute *dattr, char *buf)
{
	struct config_rom_attribute *attr =
		container_of(dattr, struct config_rom_attribute, attr);
	struct fw_csr_iterator ci;
	u32 *dir;
	int key, value, ret = -ENOENT;

	down_read(&fw_device_rwsem);

	if (is_fw_unit(dev))
		dir = fw_unit(dev)->directory;
	else
		dir = fw_device(dev)->config_rom + 5;

	fw_csr_iterator_init(&ci, dir);
	while (fw_csr_iterator_next(&ci, &key, &value))
		if (attr->key == key) {
			ret = snprintf(buf, buf ? PAGE_SIZE : 0,
				       "0x%06x\n", value);
			break;
		}

	up_read(&fw_device_rwsem);

	return ret;
}

#define IMMEDIATE_ATTR(name, key)				\
	{ __ATTR(name, S_IRUGO, show_immediate, NULL), key }

static ssize_t show_text_leaf(struct device *dev,
			      struct device_attribute *dattr, char *buf)
{
	struct config_rom_attribute *attr =
		container_of(dattr, struct config_rom_attribute, attr);
	struct fw_csr_iterator ci;
	u32 *dir, *block = NULL, *p, *end;
	int length, key, value, last_key = 0, ret = -ENOENT;
	char *b;

	down_read(&fw_device_rwsem);

	if (is_fw_unit(dev))
		dir = fw_unit(dev)->directory;
	else
		dir = fw_device(dev)->config_rom + 5;

	fw_csr_iterator_init(&ci, dir);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		if (attr->key == last_key &&
		    key == (CSR_DESCRIPTOR | CSR_LEAF))
			block = ci.p - 1 + value;
		last_key = key;
	}

	if (block == NULL)
		goto out;

	length = min(block[0] >> 16, 256U);
	if (length < 3)
		goto out;

	if (block[1] != 0 || block[2] != 0)
		
		goto out;

	if (buf == NULL) {
		ret = length * 4;
		goto out;
	}

	b = buf;
	end = &block[length + 1];
	for (p = &block[3]; p < end; p++, b += 4)
		* (u32 *) b = (__force u32) __cpu_to_be32(*p);

	
	while (b--, (isspace(*b) || *b == '\0') && b > buf);
	strcpy(b + 1, "\n");
	ret = b + 2 - buf;
 out:
	up_read(&fw_device_rwsem);

	return ret;
}

#define TEXT_LEAF_ATTR(name, key)				\
	{ __ATTR(name, S_IRUGO, show_text_leaf, NULL), key }

static struct config_rom_attribute config_rom_attributes[] = {
	IMMEDIATE_ATTR(vendor, CSR_VENDOR),
	IMMEDIATE_ATTR(hardware_version, CSR_HARDWARE_VERSION),
	IMMEDIATE_ATTR(specifier_id, CSR_SPECIFIER_ID),
	IMMEDIATE_ATTR(version, CSR_VERSION),
	IMMEDIATE_ATTR(model, CSR_MODEL),
	TEXT_LEAF_ATTR(vendor_name, CSR_VENDOR),
	TEXT_LEAF_ATTR(model_name, CSR_MODEL),
	TEXT_LEAF_ATTR(hardware_version_name, CSR_HARDWARE_VERSION),
};

static void init_fw_attribute_group(struct device *dev,
				    struct device_attribute *attrs,
				    struct fw_attribute_group *group)
{
	struct device_attribute *attr;
	int i, j;

	for (j = 0; attrs[j].attr.name != NULL; j++)
		group->attrs[j] = &attrs[j].attr;

	for (i = 0; i < ARRAY_SIZE(config_rom_attributes); i++) {
		attr = &config_rom_attributes[i].attr;
		if (attr->show(dev, attr, NULL) < 0)
			continue;
		group->attrs[j++] = &attr->attr;
	}

	group->attrs[j] = NULL;
	group->groups[0] = &group->group;
	group->groups[1] = NULL;
	group->group.attrs = group->attrs;
	dev->groups = (const struct attribute_group **) group->groups;
}

static ssize_t modalias_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct fw_unit *unit = fw_unit(dev);
	int length;

	length = get_modalias(unit, buf, PAGE_SIZE);
	strcpy(buf + length, "\n");

	return length + 1;
}

static ssize_t rom_index_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct fw_device *device = fw_device(dev->parent);
	struct fw_unit *unit = fw_unit(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(int)(unit->directory - device->config_rom));
}

static struct device_attribute fw_unit_attributes[] = {
	__ATTR_RO(modalias),
	__ATTR_RO(rom_index),
	__ATTR_NULL,
};

static ssize_t config_rom_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct fw_device *device = fw_device(dev);
	size_t length;

	down_read(&fw_device_rwsem);
	length = device->config_rom_length * 4;
	memcpy(buf, device->config_rom, length);
	up_read(&fw_device_rwsem);

	return length;
}

static ssize_t guid_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct fw_device *device = fw_device(dev);
	int ret;

	down_read(&fw_device_rwsem);
	ret = snprintf(buf, PAGE_SIZE, "0x%08x%08x\n",
		       device->config_rom[3], device->config_rom[4]);
	up_read(&fw_device_rwsem);

	return ret;
}

static int units_sprintf(char *buf, u32 *directory)
{
	struct fw_csr_iterator ci;
	int key, value;
	int specifier_id = 0;
	int version = 0;

	fw_csr_iterator_init(&ci, directory);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		switch (key) {
		case CSR_SPECIFIER_ID:
			specifier_id = value;
			break;
		case CSR_VERSION:
			version = value;
			break;
		}
	}

	return sprintf(buf, "0x%06x:0x%06x ", specifier_id, version);
}

static ssize_t units_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct fw_device *device = fw_device(dev);
	struct fw_csr_iterator ci;
	int key, value, i = 0;

	down_read(&fw_device_rwsem);
	fw_csr_iterator_init(&ci, &device->config_rom[5]);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		if (key != (CSR_UNIT | CSR_DIRECTORY))
			continue;
		i += units_sprintf(&buf[i], ci.p + value - 1);
		if (i >= PAGE_SIZE - (8 + 1 + 8 + 1))
			break;
	}
	up_read(&fw_device_rwsem);

	if (i)
		buf[i - 1] = '\n';

	return i;
}

static struct device_attribute fw_device_attributes[] = {
	__ATTR_RO(config_rom),
	__ATTR_RO(guid),
	__ATTR_RO(units),
	__ATTR_NULL,
};

static int read_rom(struct fw_device *device,
		    int generation, int index, u32 *data)
{
	int rcode;

	
	smp_rmb();

	rcode = fw_run_transaction(device->card, TCODE_READ_QUADLET_REQUEST,
			device->node_id, generation, device->max_speed,
			(CSR_REGISTER_BASE | CSR_CONFIG_ROM) + index * 4,
			data, 4);
	be32_to_cpus(data);

	return rcode;
}

#define READ_BIB_ROM_SIZE	256
#define READ_BIB_STACK_SIZE	16


static int read_bus_info_block(struct fw_device *device, int generation)
{
	u32 *rom, *stack, *old_rom, *new_rom;
	u32 sp, key;
	int i, end, length, ret = -1;

	rom = kmalloc(sizeof(*rom) * READ_BIB_ROM_SIZE +
		      sizeof(*stack) * READ_BIB_STACK_SIZE, GFP_KERNEL);
	if (rom == NULL)
		return -ENOMEM;

	stack = &rom[READ_BIB_ROM_SIZE];

	device->max_speed = SCODE_100;

	
	for (i = 0; i < 5; i++) {
		if (read_rom(device, generation, i, &rom[i]) != RCODE_COMPLETE)
			goto out;
		
		if (i == 0 && rom[i] == 0)
			goto out;
	}

	device->max_speed = device->node->max_speed;

	
	if ((rom[2] & 0x7) < device->max_speed ||
	    device->max_speed == SCODE_BETA ||
	    device->card->beta_repeaters_present) {
		u32 dummy;

		
		if (device->max_speed == SCODE_BETA)
			device->max_speed = device->card->link_speed;

		while (device->max_speed > SCODE_100) {
			if (read_rom(device, generation, 0, &dummy) ==
			    RCODE_COMPLETE)
				break;
			device->max_speed--;
		}
	}

	
	length = i;
	sp = 0;
	stack[sp++] = 0xc0000005;
	while (sp > 0) {
		
		key = stack[--sp];
		i = key & 0xffffff;
		if (i >= READ_BIB_ROM_SIZE)
			
			goto out;

		
		if (read_rom(device, generation, i, &rom[i]) != RCODE_COMPLETE)
			goto out;
		end = i + (rom[i] >> 16) + 1;
		i++;
		if (end > READ_BIB_ROM_SIZE)
			
			goto out;

		
		while (i < end) {
			if (read_rom(device, generation, i, &rom[i]) !=
			    RCODE_COMPLETE)
				goto out;
			if ((key >> 30) == 3 && (rom[i] >> 30) > 1 &&
			    sp < READ_BIB_STACK_SIZE)
				stack[sp++] = i + rom[i];
			i++;
		}
		if (length < i)
			length = i;
	}

	old_rom = device->config_rom;
	new_rom = kmemdup(rom, length * 4, GFP_KERNEL);
	if (new_rom == NULL)
		goto out;

	down_write(&fw_device_rwsem);
	device->config_rom = new_rom;
	device->config_rom_length = length;
	up_write(&fw_device_rwsem);

	kfree(old_rom);
	ret = 0;
	device->max_rec	= rom[2] >> 12 & 0xf;
	device->cmc	= rom[2] >> 30 & 1;
	device->irmc	= rom[2] >> 31 & 1;
 out:
	kfree(rom);

	return ret;
}

static void fw_unit_release(struct device *dev)
{
	struct fw_unit *unit = fw_unit(dev);

	kfree(unit);
}

static struct device_type fw_unit_type = {
	.uevent		= fw_unit_uevent,
	.release	= fw_unit_release,
};

static bool is_fw_unit(struct device *dev)
{
	return dev->type == &fw_unit_type;
}

static void create_units(struct fw_device *device)
{
	struct fw_csr_iterator ci;
	struct fw_unit *unit;
	int key, value, i;

	i = 0;
	fw_csr_iterator_init(&ci, &device->config_rom[5]);
	while (fw_csr_iterator_next(&ci, &key, &value)) {
		if (key != (CSR_UNIT | CSR_DIRECTORY))
			continue;

		
		unit = kzalloc(sizeof(*unit), GFP_KERNEL);
		if (unit == NULL) {
			fw_error("failed to allocate memory for unit\n");
			continue;
		}

		unit->directory = ci.p + value - 1;
		unit->device.bus = &fw_bus_type;
		unit->device.type = &fw_unit_type;
		unit->device.parent = &device->device;
		dev_set_name(&unit->device, "%s.%d", dev_name(&device->device), i++);

		BUILD_BUG_ON(ARRAY_SIZE(unit->attribute_group.attrs) <
				ARRAY_SIZE(fw_unit_attributes) +
				ARRAY_SIZE(config_rom_attributes));
		init_fw_attribute_group(&unit->device,
					fw_unit_attributes,
					&unit->attribute_group);

		if (device_register(&unit->device) < 0)
			goto skip_unit;

		continue;

	skip_unit:
		kfree(unit);
	}
}

static int shutdown_unit(struct device *device, void *data)
{
	device_unregister(device);

	return 0;
}


DECLARE_RWSEM(fw_device_rwsem);

DEFINE_IDR(fw_device_idr);
int fw_cdev_major;

struct fw_device *fw_device_get_by_devt(dev_t devt)
{
	struct fw_device *device;

	down_read(&fw_device_rwsem);
	device = idr_find(&fw_device_idr, MINOR(devt));
	if (device)
		fw_device_get(device);
	up_read(&fw_device_rwsem);

	return device;
}



#define MAX_RETRIES	10
#define RETRY_DELAY	(3 * HZ)
#define INITIAL_DELAY	(HZ / 2)
#define SHUTDOWN_DELAY	(2 * HZ)

static void fw_device_shutdown(struct work_struct *work)
{
	struct fw_device *device =
		container_of(work, struct fw_device, work.work);
	int minor = MINOR(device->device.devt);

	if (time_is_after_jiffies(device->card->reset_jiffies + SHUTDOWN_DELAY)
	    && !list_empty(&device->card->link)) {
		schedule_delayed_work(&device->work, SHUTDOWN_DELAY);
		return;
	}

	if (atomic_cmpxchg(&device->state,
			   FW_DEVICE_GONE,
			   FW_DEVICE_SHUTDOWN) != FW_DEVICE_GONE)
		return;

	fw_device_cdev_remove(device);
	device_for_each_child(&device->device, NULL, shutdown_unit);
	device_unregister(&device->device);

	down_write(&fw_device_rwsem);
	idr_remove(&fw_device_idr, minor);
	up_write(&fw_device_rwsem);

	fw_device_put(device);
}

static void fw_device_release(struct device *dev)
{
	struct fw_device *device = fw_device(dev);
	struct fw_card *card = device->card;
	unsigned long flags;

	
	spin_lock_irqsave(&card->lock, flags);
	device->node->data = NULL;
	spin_unlock_irqrestore(&card->lock, flags);

	fw_node_put(device->node);
	kfree(device->config_rom);
	kfree(device);
	fw_card_put(card);
}

static struct device_type fw_device_type = {
	.release = fw_device_release,
};

static bool is_fw_device(struct device *dev)
{
	return dev->type == &fw_device_type;
}

static int update_unit(struct device *dev, void *data)
{
	struct fw_unit *unit = fw_unit(dev);
	struct fw_driver *driver = (struct fw_driver *)dev->driver;

	if (is_fw_unit(dev) && driver != NULL && driver->update != NULL) {
		down(&dev->sem);
		driver->update(unit);
		up(&dev->sem);
	}

	return 0;
}

static void fw_device_update(struct work_struct *work)
{
	struct fw_device *device =
		container_of(work, struct fw_device, work.work);

	fw_device_cdev_update(device);
	device_for_each_child(&device->device, NULL, update_unit);
}


static int lookup_existing_device(struct device *dev, void *data)
{
	struct fw_device *old = fw_device(dev);
	struct fw_device *new = data;
	struct fw_card *card = new->card;
	int match = 0;

	if (!is_fw_device(dev))
		return 0;

	down_read(&fw_device_rwsem); 
	spin_lock_irq(&card->lock);  

	if (memcmp(old->config_rom, new->config_rom, 6 * 4) == 0 &&
	    atomic_cmpxchg(&old->state,
			   FW_DEVICE_GONE,
			   FW_DEVICE_RUNNING) == FW_DEVICE_GONE) {
		struct fw_node *current_node = new->node;
		struct fw_node *obsolete_node = old->node;

		new->node = obsolete_node;
		new->node->data = new;
		old->node = current_node;
		old->node->data = old;

		old->max_speed = new->max_speed;
		old->node_id = current_node->node_id;
		smp_wmb();  
		old->generation = card->generation;
		old->config_rom_retries = 0;
		fw_notify("rediscovered device %s\n", dev_name(dev));

		PREPARE_DELAYED_WORK(&old->work, fw_device_update);
		schedule_delayed_work(&old->work, 0);

		if (current_node == card->root_node)
			fw_schedule_bm_work(card, 0);

		match = 1;
	}

	spin_unlock_irq(&card->lock);
	up_read(&fw_device_rwsem);

	return match;
}

enum { BC_UNKNOWN = 0, BC_UNIMPLEMENTED, BC_IMPLEMENTED, };

static void set_broadcast_channel(struct fw_device *device, int generation)
{
	struct fw_card *card = device->card;
	__be32 data;
	int rcode;

	if (!card->broadcast_channel_allocated)
		return;

	
	if (!device->irmc || device->max_rec < 8)
		return;

	
	if (device->bc_implemented == BC_UNKNOWN) {
		rcode = fw_run_transaction(card, TCODE_READ_QUADLET_REQUEST,
				device->node_id, generation, device->max_speed,
				CSR_REGISTER_BASE + CSR_BROADCAST_CHANNEL,
				&data, 4);
		switch (rcode) {
		case RCODE_COMPLETE:
			if (data & cpu_to_be32(1 << 31)) {
				device->bc_implemented = BC_IMPLEMENTED;
				break;
			}
			
		case RCODE_ADDRESS_ERROR:
			device->bc_implemented = BC_UNIMPLEMENTED;
		}
	}

	if (device->bc_implemented == BC_IMPLEMENTED) {
		data = cpu_to_be32(BROADCAST_CHANNEL_INITIAL |
				   BROADCAST_CHANNEL_VALID);
		fw_run_transaction(card, TCODE_WRITE_QUADLET_REQUEST,
				device->node_id, generation, device->max_speed,
				CSR_REGISTER_BASE + CSR_BROADCAST_CHANNEL,
				&data, 4);
	}
}

int fw_device_set_broadcast_channel(struct device *dev, void *gen)
{
	if (is_fw_device(dev))
		set_broadcast_channel(fw_device(dev), (long)gen);

	return 0;
}

static void fw_device_init(struct work_struct *work)
{
	struct fw_device *device =
		container_of(work, struct fw_device, work.work);
	struct device *revived_dev;
	int minor, ret;

	

	if (read_bus_info_block(device, device->generation) < 0) {
		if (device->config_rom_retries < MAX_RETRIES &&
		    atomic_read(&device->state) == FW_DEVICE_INITIALIZING) {
			device->config_rom_retries++;
			schedule_delayed_work(&device->work, RETRY_DELAY);
		} else {
			fw_notify("giving up on config rom for node id %x\n",
				  device->node_id);
			if (device->node == device->card->root_node)
				fw_schedule_bm_work(device->card, 0);
			fw_device_release(&device->device);
		}
		return;
	}

	revived_dev = device_find_child(device->card->device,
					device, lookup_existing_device);
	if (revived_dev) {
		put_device(revived_dev);
		fw_device_release(&device->device);

		return;
	}

	device_initialize(&device->device);

	fw_device_get(device);
	down_write(&fw_device_rwsem);
	ret = idr_pre_get(&fw_device_idr, GFP_KERNEL) ?
	      idr_get_new(&fw_device_idr, device, &minor) :
	      -ENOMEM;
	up_write(&fw_device_rwsem);

	if (ret < 0)
		goto error;

	device->device.bus = &fw_bus_type;
	device->device.type = &fw_device_type;
	device->device.parent = device->card->device;
	device->device.devt = MKDEV(fw_cdev_major, minor);
	dev_set_name(&device->device, "fw%d", minor);

	BUILD_BUG_ON(ARRAY_SIZE(device->attribute_group.attrs) <
			ARRAY_SIZE(fw_device_attributes) +
			ARRAY_SIZE(config_rom_attributes));
	init_fw_attribute_group(&device->device,
				fw_device_attributes,
				&device->attribute_group);

	if (device_add(&device->device)) {
		fw_error("Failed to add device.\n");
		goto error_with_cdev;
	}

	create_units(device);

	
	if (atomic_cmpxchg(&device->state,
			   FW_DEVICE_INITIALIZING,
			   FW_DEVICE_RUNNING) == FW_DEVICE_GONE) {
		PREPARE_DELAYED_WORK(&device->work, fw_device_shutdown);
		schedule_delayed_work(&device->work, SHUTDOWN_DELAY);
	} else {
		if (device->config_rom_retries)
			fw_notify("created device %s: GUID %08x%08x, S%d00, "
				  "%d config ROM retries\n",
				  dev_name(&device->device),
				  device->config_rom[3], device->config_rom[4],
				  1 << device->max_speed,
				  device->config_rom_retries);
		else
			fw_notify("created device %s: GUID %08x%08x, S%d00\n",
				  dev_name(&device->device),
				  device->config_rom[3], device->config_rom[4],
				  1 << device->max_speed);
		device->config_rom_retries = 0;

		set_broadcast_channel(device, device->generation);
	}

	
	if (device->node == device->card->root_node)
		fw_schedule_bm_work(device->card, 0);

	return;

 error_with_cdev:
	down_write(&fw_device_rwsem);
	idr_remove(&fw_device_idr, minor);
	up_write(&fw_device_rwsem);
 error:
	fw_device_put(device);		

	put_device(&device->device);	
}

enum {
	REREAD_BIB_ERROR,
	REREAD_BIB_GONE,
	REREAD_BIB_UNCHANGED,
	REREAD_BIB_CHANGED,
};


static int reread_bus_info_block(struct fw_device *device, int generation)
{
	u32 q;
	int i;

	for (i = 0; i < 6; i++) {
		if (read_rom(device, generation, i, &q) != RCODE_COMPLETE)
			return REREAD_BIB_ERROR;

		if (i == 0 && q == 0)
			return REREAD_BIB_GONE;

		if (q != device->config_rom[i])
			return REREAD_BIB_CHANGED;
	}

	return REREAD_BIB_UNCHANGED;
}

static void fw_device_refresh(struct work_struct *work)
{
	struct fw_device *device =
		container_of(work, struct fw_device, work.work);
	struct fw_card *card = device->card;
	int node_id = device->node_id;

	switch (reread_bus_info_block(device, device->generation)) {
	case REREAD_BIB_ERROR:
		if (device->config_rom_retries < MAX_RETRIES / 2 &&
		    atomic_read(&device->state) == FW_DEVICE_INITIALIZING) {
			device->config_rom_retries++;
			schedule_delayed_work(&device->work, RETRY_DELAY / 2);

			return;
		}
		goto give_up;

	case REREAD_BIB_GONE:
		goto gone;

	case REREAD_BIB_UNCHANGED:
		if (atomic_cmpxchg(&device->state,
				   FW_DEVICE_INITIALIZING,
				   FW_DEVICE_RUNNING) == FW_DEVICE_GONE)
			goto gone;

		fw_device_update(work);
		device->config_rom_retries = 0;
		goto out;

	case REREAD_BIB_CHANGED:
		break;
	}

	
	device_for_each_child(&device->device, NULL, shutdown_unit);

	if (read_bus_info_block(device, device->generation) < 0) {
		if (device->config_rom_retries < MAX_RETRIES &&
		    atomic_read(&device->state) == FW_DEVICE_INITIALIZING) {
			device->config_rom_retries++;
			schedule_delayed_work(&device->work, RETRY_DELAY);

			return;
		}
		goto give_up;
	}

	create_units(device);

	
	kobject_uevent(&device->device.kobj, KOBJ_CHANGE);

	if (atomic_cmpxchg(&device->state,
			   FW_DEVICE_INITIALIZING,
			   FW_DEVICE_RUNNING) == FW_DEVICE_GONE)
		goto gone;

	fw_notify("refreshed device %s\n", dev_name(&device->device));
	device->config_rom_retries = 0;
	goto out;

 give_up:
	fw_notify("giving up on refresh of device %s\n", dev_name(&device->device));
 gone:
	atomic_set(&device->state, FW_DEVICE_GONE);
	PREPARE_DELAYED_WORK(&device->work, fw_device_shutdown);
	schedule_delayed_work(&device->work, SHUTDOWN_DELAY);
 out:
	if (node_id == card->root_node->node_id)
		fw_schedule_bm_work(card, 0);
}

void fw_node_event(struct fw_card *card, struct fw_node *node, int event)
{
	struct fw_device *device;

	switch (event) {
	case FW_NODE_CREATED:
	case FW_NODE_LINK_ON:
		if (!node->link_on)
			break;
 create:
		device = kzalloc(sizeof(*device), GFP_ATOMIC);
		if (device == NULL)
			break;

		
		atomic_set(&device->state, FW_DEVICE_INITIALIZING);
		device->card = fw_card_get(card);
		device->node = fw_node_get(node);
		device->node_id = node->node_id;
		device->generation = card->generation;
		device->is_local = node == card->local_node;
		mutex_init(&device->client_list_mutex);
		INIT_LIST_HEAD(&device->client_list);

		
		node->data = device;

		
		INIT_DELAYED_WORK(&device->work, fw_device_init);
		schedule_delayed_work(&device->work, INITIAL_DELAY);
		break;

	case FW_NODE_INITIATED_RESET:
		device = node->data;
		if (device == NULL)
			goto create;

		device->node_id = node->node_id;
		smp_wmb();  
		device->generation = card->generation;
		if (atomic_cmpxchg(&device->state,
			    FW_DEVICE_RUNNING,
			    FW_DEVICE_INITIALIZING) == FW_DEVICE_RUNNING) {
			PREPARE_DELAYED_WORK(&device->work, fw_device_refresh);
			schedule_delayed_work(&device->work,
				device->is_local ? 0 : INITIAL_DELAY);
		}
		break;

	case FW_NODE_UPDATED:
		if (!node->link_on || node->data == NULL)
			break;

		device = node->data;
		device->node_id = node->node_id;
		smp_wmb();  
		device->generation = card->generation;
		if (atomic_read(&device->state) == FW_DEVICE_RUNNING) {
			PREPARE_DELAYED_WORK(&device->work, fw_device_update);
			schedule_delayed_work(&device->work, 0);
		}
		break;

	case FW_NODE_DESTROYED:
	case FW_NODE_LINK_OFF:
		if (!node->data)
			break;

		
		device = node->data;
		if (atomic_xchg(&device->state,
				FW_DEVICE_GONE) == FW_DEVICE_RUNNING) {
			PREPARE_DELAYED_WORK(&device->work, fw_device_shutdown);
			schedule_delayed_work(&device->work,
				list_empty(&card->link) ? 0 : SHUTDOWN_DELAY);
		}
		break;
	}
}
