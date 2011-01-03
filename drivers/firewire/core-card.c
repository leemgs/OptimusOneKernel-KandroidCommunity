

#include <linux/bug.h>
#include <linux/completion.h>
#include <linux/crc-itu-t.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/firewire.h>
#include <linux/firewire-constants.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <asm/atomic.h>
#include <asm/byteorder.h>

#include "core.h"

int fw_compute_block_crc(u32 *block)
{
	__be32 be32_block[256];
	int i, length;

	length = (*block >> 16) & 0xff;
	for (i = 0; i < length; i++)
		be32_block[i] = cpu_to_be32(block[i + 1]);
	*block |= crc_itu_t(0, (u8 *) be32_block, length * 4);

	return length;
}

static DEFINE_MUTEX(card_mutex);
static LIST_HEAD(card_list);

static LIST_HEAD(descriptor_list);
static int descriptor_count;


static size_t config_rom_length = 1 + 4 + 1 + 1;

#define BIB_CRC(v)		((v) <<  0)
#define BIB_CRC_LENGTH(v)	((v) << 16)
#define BIB_INFO_LENGTH(v)	((v) << 24)

#define BIB_LINK_SPEED(v)	((v) <<  0)
#define BIB_GENERATION(v)	((v) <<  4)
#define BIB_MAX_ROM(v)		((v) <<  8)
#define BIB_MAX_RECEIVE(v)	((v) << 12)
#define BIB_CYC_CLK_ACC(v)	((v) << 16)
#define BIB_PMC			((1) << 27)
#define BIB_BMC			((1) << 28)
#define BIB_ISC			((1) << 29)
#define BIB_CMC			((1) << 30)
#define BIB_IMC			((1) << 31)

static u32 *generate_config_rom(struct fw_card *card)
{
	struct fw_descriptor *desc;
	static u32 config_rom[256];
	int i, j, length;

	

	memset(config_rom, 0, sizeof(config_rom));
	config_rom[0] = BIB_CRC_LENGTH(4) | BIB_INFO_LENGTH(4) | BIB_CRC(0);
	config_rom[1] = 0x31333934;

	config_rom[2] =
		BIB_LINK_SPEED(card->link_speed) |
		BIB_GENERATION(card->config_rom_generation++ % 14 + 2) |
		BIB_MAX_ROM(2) |
		BIB_MAX_RECEIVE(card->max_receive) |
		BIB_BMC | BIB_ISC | BIB_CMC | BIB_IMC;
	config_rom[3] = card->guid >> 32;
	config_rom[4] = card->guid;

	
	i = 5;
	config_rom[i++] = 0;
	config_rom[i++] = 0x0c0083c0; 
	j = i + descriptor_count;

	
	list_for_each_entry (desc, &descriptor_list, link) {
		if (desc->immediate > 0)
			config_rom[i++] = desc->immediate;
		config_rom[i] = desc->key | (j - i);
		i++;
		j += desc->length;
	}

	
	config_rom[5] = (i - 5 - 1) << 16;

	
	list_for_each_entry (desc, &descriptor_list, link) {
		memcpy(&config_rom[i], desc->data, desc->length * 4);
		i += desc->length;
	}

	
	for (i = 0; i < j; i += length + 1)
		length = fw_compute_block_crc(config_rom + i);

	WARN_ON(j != config_rom_length);

	return config_rom;
}

static void update_config_roms(void)
{
	struct fw_card *card;
	u32 *config_rom;

	list_for_each_entry (card, &card_list, link) {
		config_rom = generate_config_rom(card);
		card->driver->set_config_rom(card, config_rom,
					     config_rom_length);
	}
}

static size_t required_space(struct fw_descriptor *desc)
{
	
	return desc->length + 1 + (desc->immediate > 0 ? 1 : 0);
}

int fw_core_add_descriptor(struct fw_descriptor *desc)
{
	size_t i;
	int ret;

	
	i = 0;
	while (i < desc->length)
		i += (desc->data[i] >> 16) + 1;

	if (i != desc->length)
		return -EINVAL;

	mutex_lock(&card_mutex);

	if (config_rom_length + required_space(desc) > 256) {
		ret = -EBUSY;
	} else {
		list_add_tail(&desc->link, &descriptor_list);
		config_rom_length += required_space(desc);
		descriptor_count++;
		if (desc->immediate > 0)
			descriptor_count++;
		update_config_roms();
		ret = 0;
	}

	mutex_unlock(&card_mutex);

	return ret;
}
EXPORT_SYMBOL(fw_core_add_descriptor);

void fw_core_remove_descriptor(struct fw_descriptor *desc)
{
	mutex_lock(&card_mutex);

	list_del(&desc->link);
	config_rom_length -= required_space(desc);
	descriptor_count--;
	if (desc->immediate > 0)
		descriptor_count--;
	update_config_roms();

	mutex_unlock(&card_mutex);
}
EXPORT_SYMBOL(fw_core_remove_descriptor);

static void allocate_broadcast_channel(struct fw_card *card, int generation)
{
	int channel, bandwidth = 0;

	fw_iso_resource_manage(card, generation, 1ULL << 31, &channel,
			       &bandwidth, true, card->bm_transaction_data);
	if (channel == 31) {
		card->broadcast_channel_allocated = true;
		device_for_each_child(card->device, (void *)(long)generation,
				      fw_device_set_broadcast_channel);
	}
}

static const char gap_count_table[] = {
	63, 5, 7, 8, 10, 13, 16, 18, 21, 24, 26, 29, 32, 35, 37, 40
};

void fw_schedule_bm_work(struct fw_card *card, unsigned long delay)
{
	int scheduled;

	fw_card_get(card);
	scheduled = schedule_delayed_work(&card->work, delay);
	if (!scheduled)
		fw_card_put(card);
}

static void fw_card_bm_work(struct work_struct *work)
{
	struct fw_card *card = container_of(work, struct fw_card, work.work);
	struct fw_device *root_device;
	struct fw_node *root_node;
	unsigned long flags;
	int root_id, new_root_id, irm_id, local_id;
	int gap_count, generation, grace, rcode;
	bool do_reset = false;
	bool root_device_is_running;
	bool root_device_is_cmc;

	spin_lock_irqsave(&card->lock, flags);

	if (card->local_node == NULL) {
		spin_unlock_irqrestore(&card->lock, flags);
		goto out_put_card;
	}

	generation = card->generation;
	root_node = card->root_node;
	fw_node_get(root_node);
	root_device = root_node->data;
	root_device_is_running = root_device &&
			atomic_read(&root_device->state) == FW_DEVICE_RUNNING;
	root_device_is_cmc = root_device && root_device->cmc;
	root_id  = root_node->node_id;
	irm_id   = card->irm_node->node_id;
	local_id = card->local_node->node_id;

	grace = time_after(jiffies, card->reset_jiffies + DIV_ROUND_UP(HZ, 8));

	if (is_next_generation(generation, card->bm_generation) ||
	    (card->bm_generation != generation && grace)) {
		

		if (!card->irm_node->link_on) {
			new_root_id = local_id;
			fw_notify("IRM has link off, making local node (%02x) root.\n",
				  new_root_id);
			goto pick_me;
		}

		card->bm_transaction_data[0] = cpu_to_be32(0x3f);
		card->bm_transaction_data[1] = cpu_to_be32(local_id);

		spin_unlock_irqrestore(&card->lock, flags);

		rcode = fw_run_transaction(card, TCODE_LOCK_COMPARE_SWAP,
				irm_id, generation, SCODE_100,
				CSR_REGISTER_BASE + CSR_BUS_MANAGER_ID,
				card->bm_transaction_data,
				sizeof(card->bm_transaction_data));

		if (rcode == RCODE_GENERATION)
			
			goto out;

		if (rcode == RCODE_COMPLETE &&
		    card->bm_transaction_data[0] != cpu_to_be32(0x3f)) {

			
			if (local_id == irm_id)
				allocate_broadcast_channel(card, generation);

			goto out;
		}

		spin_lock_irqsave(&card->lock, flags);

		if (rcode != RCODE_COMPLETE) {
			
			new_root_id = local_id;
			fw_notify("BM lock failed, making local node (%02x) root.\n",
				  new_root_id);
			goto pick_me;
		}
	} else if (card->bm_generation != generation) {
		
		spin_unlock_irqrestore(&card->lock, flags);
		fw_schedule_bm_work(card, DIV_ROUND_UP(HZ, 8));
		goto out;
	}

	
	card->bm_generation = generation;

	if (root_device == NULL) {
		
		new_root_id = local_id;
	} else if (!root_device_is_running) {
		
		spin_unlock_irqrestore(&card->lock, flags);
		goto out;
	} else if (root_device_is_cmc) {
		
		new_root_id = root_id;
	} else {
		
		new_root_id = local_id;
	}

 pick_me:
	
	if (!card->beta_repeaters_present &&
	    root_node->max_hops < ARRAY_SIZE(gap_count_table))
		gap_count = gap_count_table[root_node->max_hops];
	else
		gap_count = 63;

	

	if (card->bm_retries++ < 5 &&
	    (card->gap_count != gap_count || new_root_id != root_id))
		do_reset = true;

	spin_unlock_irqrestore(&card->lock, flags);

	if (do_reset) {
		fw_notify("phy config: card %d, new root=%x, gap_count=%d\n",
			  card->index, new_root_id, gap_count);
		fw_send_phy_config(card, new_root_id, generation, gap_count);
		fw_core_initiate_bus_reset(card, 1);
		
	} else {
		if (local_id == irm_id)
			allocate_broadcast_channel(card, generation);
	}

 out:
	fw_node_put(root_node);
 out_put_card:
	fw_card_put(card);
}

static void flush_timer_callback(unsigned long data)
{
	struct fw_card *card = (struct fw_card *)data;

	fw_flush_transactions(card);
}

void fw_card_initialize(struct fw_card *card,
			const struct fw_card_driver *driver,
			struct device *device)
{
	static atomic_t index = ATOMIC_INIT(-1);

	card->index = atomic_inc_return(&index);
	card->driver = driver;
	card->device = device;
	card->current_tlabel = 0;
	card->tlabel_mask = 0;
	card->color = 0;
	card->broadcast_channel = BROADCAST_CHANNEL_INITIAL;

	kref_init(&card->kref);
	init_completion(&card->done);
	INIT_LIST_HEAD(&card->transaction_list);
	spin_lock_init(&card->lock);
	setup_timer(&card->flush_timer,
		    flush_timer_callback, (unsigned long)card);

	card->local_node = NULL;

	INIT_DELAYED_WORK(&card->work, fw_card_bm_work);
}
EXPORT_SYMBOL(fw_card_initialize);

int fw_card_add(struct fw_card *card,
		u32 max_receive, u32 link_speed, u64 guid)
{
	u32 *config_rom;
	int ret;

	card->max_receive = max_receive;
	card->link_speed = link_speed;
	card->guid = guid;

	mutex_lock(&card_mutex);

	config_rom = generate_config_rom(card);
	ret = card->driver->enable(card, config_rom, config_rom_length);
	if (ret == 0)
		list_add_tail(&card->link, &card_list);

	mutex_unlock(&card_mutex);

	return ret;
}
EXPORT_SYMBOL(fw_card_add);




static int dummy_enable(struct fw_card *card, u32 *config_rom, size_t length)
{
	BUG();
	return -1;
}

static int dummy_update_phy_reg(struct fw_card *card, int address,
				int clear_bits, int set_bits)
{
	return -ENODEV;
}

static int dummy_set_config_rom(struct fw_card *card,
				u32 *config_rom, size_t length)
{
	
	BUG();
	return -1;
}

static void dummy_send_request(struct fw_card *card, struct fw_packet *packet)
{
	packet->callback(packet, card, -ENODEV);
}

static void dummy_send_response(struct fw_card *card, struct fw_packet *packet)
{
	packet->callback(packet, card, -ENODEV);
}

static int dummy_cancel_packet(struct fw_card *card, struct fw_packet *packet)
{
	return -ENOENT;
}

static int dummy_enable_phys_dma(struct fw_card *card,
				 int node_id, int generation)
{
	return -ENODEV;
}

static const struct fw_card_driver dummy_driver_template = {
	.enable          = dummy_enable,
	.update_phy_reg  = dummy_update_phy_reg,
	.set_config_rom  = dummy_set_config_rom,
	.send_request    = dummy_send_request,
	.cancel_packet   = dummy_cancel_packet,
	.send_response   = dummy_send_response,
	.enable_phys_dma = dummy_enable_phys_dma,
};

void fw_card_release(struct kref *kref)
{
	struct fw_card *card = container_of(kref, struct fw_card, kref);

	complete(&card->done);
}

void fw_core_remove_card(struct fw_card *card)
{
	struct fw_card_driver dummy_driver = dummy_driver_template;

	card->driver->update_phy_reg(card, 4,
				     PHY_LINK_ACTIVE | PHY_CONTENDER, 0);
	fw_core_initiate_bus_reset(card, 1);

	mutex_lock(&card_mutex);
	list_del_init(&card->link);
	mutex_unlock(&card_mutex);

	
	dummy_driver.free_iso_context	= card->driver->free_iso_context;
	dummy_driver.stop_iso		= card->driver->stop_iso;
	card->driver = &dummy_driver;

	fw_destroy_nodes(card);

	
	fw_card_put(card);
	wait_for_completion(&card->done);

	WARN_ON(!list_empty(&card->transaction_list));
	del_timer_sync(&card->flush_timer);
}
EXPORT_SYMBOL(fw_core_remove_card);

int fw_core_initiate_bus_reset(struct fw_card *card, int short_reset)
{
	int reg = short_reset ? 5 : 1;
	int bit = short_reset ? PHY_BUS_SHORT_RESET : PHY_BUS_RESET;

	return card->driver->update_phy_reg(card, reg, 0, bit);
}
EXPORT_SYMBOL(fw_core_initiate_bus_reset);
