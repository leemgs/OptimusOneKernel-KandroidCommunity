

#ifndef __I2400M_H__
#define __I2400M_H__

#include <linux/usb.h>
#include <linux/netdevice.h>
#include <linux/completion.h>
#include <linux/rwsem.h>
#include <asm/atomic.h>
#include <net/wimax.h>
#include <linux/wimax/i2400m.h>
#include <asm/byteorder.h>


enum {
	
	I2400M_BOOT_RETRIES = 3,
	I3200_BOOT_RETRIES = 3,
	
	I2400M_BM_CMD_BUF_SIZE = 16 * 1024,
	I2400M_BM_ACK_BUF_SIZE = 256,
};


struct i2400m_poke_table{
	__le32 address;
	__le32 data;
};

#define I2400M_FW_POKE(a, d) {		\
	.address = cpu_to_le32(a),	\
	.data = cpu_to_le32(d)		\
}



enum i2400m_reset_type {
	I2400M_RT_WARM,	
	I2400M_RT_COLD,	
	I2400M_RT_BUS,	
};

struct i2400m_reset_ctx;
struct i2400m_roq;


struct i2400m {
	struct wimax_dev wimax_dev;	

	unsigned updown:1;		
	unsigned boot_mode:1;		
	unsigned sboot:1;		
	unsigned ready:1;		
	unsigned rx_reorder:1;		
	u8 trace_msg_from_user;		
					
	enum i2400m_system_state state;
	wait_queue_head_t state_wq;	

	size_t bus_tx_block_size;
	size_t bus_pl_size_max;
	unsigned bus_bm_retries;

	int (*bus_dev_start)(struct i2400m *);
	void (*bus_dev_stop)(struct i2400m *);
	void (*bus_tx_kick)(struct i2400m *);
	int (*bus_reset)(struct i2400m *, enum i2400m_reset_type);
	ssize_t (*bus_bm_cmd_send)(struct i2400m *,
				   const struct i2400m_bootrom_header *,
				   size_t, int flags);
	ssize_t (*bus_bm_wait_for_ack)(struct i2400m *,
				       struct i2400m_bootrom_header *, size_t);
	const char **bus_fw_names;
	unsigned bus_bm_mac_addr_impaired:1;
	const struct i2400m_poke_table *bus_bm_pokes_table;

	spinlock_t tx_lock;		
	void *tx_buf;
	size_t tx_in, tx_out;
	struct i2400m_msg_hdr *tx_msg;
	size_t tx_sequence, tx_msg_size;
	
	unsigned tx_pl_num, tx_pl_max, tx_pl_min,
		tx_num, tx_size_acc, tx_size_min, tx_size_max;

	
	spinlock_t rx_lock;		
	unsigned rx_pl_num, rx_pl_max, rx_pl_min,
		rx_num, rx_size_acc, rx_size_min, rx_size_max;
	struct i2400m_roq *rx_roq;	
	u8 src_mac_addr[ETH_HLEN];

	struct mutex msg_mutex;		
	struct completion msg_completion;
	struct sk_buff *ack_skb;	

	void *bm_ack_buf;		
	void *bm_cmd_buf;		

	struct workqueue_struct *work_queue;

	struct mutex init_mutex;	
	struct i2400m_reset_ctx *reset_ctx;	

	struct work_struct wake_tx_ws;
	struct sk_buff *wake_tx_skb;

	struct dentry *debugfs_dentry;
	const char *fw_name;		
	unsigned long fw_version;	
};



static inline
void i2400m_init(struct i2400m *i2400m)
{
	wimax_dev_init(&i2400m->wimax_dev);

	i2400m->boot_mode = 1;
	i2400m->rx_reorder = 1;
	init_waitqueue_head(&i2400m->state_wq);

	spin_lock_init(&i2400m->tx_lock);
	i2400m->tx_pl_min = UINT_MAX;
	i2400m->tx_size_min = UINT_MAX;

	spin_lock_init(&i2400m->rx_lock);
	i2400m->rx_pl_min = UINT_MAX;
	i2400m->rx_size_min = UINT_MAX;

	mutex_init(&i2400m->msg_mutex);
	init_completion(&i2400m->msg_completion);

	mutex_init(&i2400m->init_mutex);
	
}




static inline
struct i2400m *wimax_dev_to_i2400m(struct wimax_dev *wimax_dev)
{
	return container_of(wimax_dev, struct i2400m, wimax_dev);
}

static inline
struct i2400m *net_dev_to_i2400m(struct net_device *net_dev)
{
	return wimax_dev_to_i2400m(netdev_priv(net_dev));
}




enum i2400m_bm_cmd_flags {
	I2400M_BM_CMD_RAW	= 1 << 2,
};


enum i2400m_bri {
	I2400M_BRI_SOFT       = 1 << 1,
	I2400M_BRI_NO_REBOOT  = 1 << 2,
	I2400M_BRI_MAC_REINIT = 1 << 3,
};

extern void i2400m_bm_cmd_prepare(struct i2400m_bootrom_header *);
extern int i2400m_dev_bootstrap(struct i2400m *, enum i2400m_bri);
extern int i2400m_read_mac_addr(struct i2400m *);
extern int i2400m_bootrom_init(struct i2400m *, enum i2400m_bri);



static inline
__le32 i2400m_brh_command(enum i2400m_brh_opcode opcode, unsigned use_checksum,
			  unsigned direct_access)
{
	return cpu_to_le32(
		I2400M_BRH_SIGNATURE
		| (direct_access ? I2400M_BRH_DIRECT_ACCESS : 0)
		| I2400M_BRH_RESPONSE_REQUIRED 
		| (use_checksum ? I2400M_BRH_USE_CHECKSUM : 0)
		| (opcode & I2400M_BRH_OPCODE_MASK));
}

static inline
void i2400m_brh_set_opcode(struct i2400m_bootrom_header *hdr,
			   enum i2400m_brh_opcode opcode)
{
	hdr->command = cpu_to_le32(
		(le32_to_cpu(hdr->command) & ~I2400M_BRH_OPCODE_MASK)
		| (opcode & I2400M_BRH_OPCODE_MASK));
}

static inline
unsigned i2400m_brh_get_opcode(const struct i2400m_bootrom_header *hdr)
{
	return le32_to_cpu(hdr->command) & I2400M_BRH_OPCODE_MASK;
}

static inline
unsigned i2400m_brh_get_response(const struct i2400m_bootrom_header *hdr)
{
	return (le32_to_cpu(hdr->command) & I2400M_BRH_RESPONSE_MASK)
		>> I2400M_BRH_RESPONSE_SHIFT;
}

static inline
unsigned i2400m_brh_get_use_checksum(const struct i2400m_bootrom_header *hdr)
{
	return le32_to_cpu(hdr->command) & I2400M_BRH_USE_CHECKSUM;
}

static inline
unsigned i2400m_brh_get_response_required(
	const struct i2400m_bootrom_header *hdr)
{
	return le32_to_cpu(hdr->command) & I2400M_BRH_RESPONSE_REQUIRED;
}

static inline
unsigned i2400m_brh_get_direct_access(const struct i2400m_bootrom_header *hdr)
{
	return le32_to_cpu(hdr->command) & I2400M_BRH_DIRECT_ACCESS;
}

static inline
unsigned i2400m_brh_get_signature(const struct i2400m_bootrom_header *hdr)
{
	return (le32_to_cpu(hdr->command) & I2400M_BRH_SIGNATURE_MASK)
		>> I2400M_BRH_SIGNATURE_SHIFT;
}



extern void i2400m_netdev_setup(struct net_device *net_dev);
extern int i2400m_sysfs_setup(struct device_driver *);
extern void i2400m_sysfs_release(struct device_driver *);
extern int i2400m_tx_setup(struct i2400m *);
extern void i2400m_wake_tx_work(struct work_struct *);
extern void i2400m_tx_release(struct i2400m *);

extern int i2400m_rx_setup(struct i2400m *);
extern void i2400m_rx_release(struct i2400m *);

extern void i2400m_net_rx(struct i2400m *, struct sk_buff *, unsigned,
			  const void *, int);
extern void i2400m_net_erx(struct i2400m *, struct sk_buff *,
			   enum i2400m_cs);
enum i2400m_pt;
extern int i2400m_tx(struct i2400m *, const void *, size_t, enum i2400m_pt);

#ifdef CONFIG_DEBUG_FS
extern int i2400m_debugfs_add(struct i2400m *);
extern void i2400m_debugfs_rm(struct i2400m *);
#else
static inline int i2400m_debugfs_add(struct i2400m *i2400m)
{
	return 0;
}
static inline void i2400m_debugfs_rm(struct i2400m *i2400m) {}
#endif


extern int i2400m_dev_initialize(struct i2400m *);
extern void i2400m_dev_shutdown(struct i2400m *);

extern struct attribute_group i2400m_dev_attr_group;

extern int i2400m_schedule_work(struct i2400m *,
				void (*)(struct work_struct *), gfp_t);



static inline
size_t i2400m_pld_size(const struct i2400m_pld *pld)
{
	return I2400M_PLD_SIZE_MASK & le32_to_cpu(pld->val);
}

static inline
enum i2400m_pt i2400m_pld_type(const struct i2400m_pld *pld)
{
	return (I2400M_PLD_TYPE_MASK & le32_to_cpu(pld->val))
		>> I2400M_PLD_TYPE_SHIFT;
}

static inline
void i2400m_pld_set(struct i2400m_pld *pld, size_t size,
		    enum i2400m_pt type)
{
	pld->val = cpu_to_le32(
		((type << I2400M_PLD_TYPE_SHIFT) & I2400M_PLD_TYPE_MASK)
		|  (size & I2400M_PLD_SIZE_MASK));
}




static inline
struct i2400m *i2400m_get(struct i2400m *i2400m)
{
	dev_hold(i2400m->wimax_dev.net_dev);
	return i2400m;
}

static inline
void i2400m_put(struct i2400m *i2400m)
{
	dev_put(i2400m->wimax_dev.net_dev);
}

extern int i2400m_dev_reset_handle(struct i2400m *);


extern int i2400m_setup(struct i2400m *, enum i2400m_bri bm_flags);
extern void i2400m_release(struct i2400m *);

extern int i2400m_rx(struct i2400m *, struct sk_buff *);
extern struct i2400m_msg_hdr *i2400m_tx_msg_get(struct i2400m *, size_t *);
extern void i2400m_tx_msg_sent(struct i2400m *);

static const __le32 i2400m_NBOOT_BARKER[4] = {
	cpu_to_le32(I2400M_NBOOT_BARKER),
	cpu_to_le32(I2400M_NBOOT_BARKER),
	cpu_to_le32(I2400M_NBOOT_BARKER),
	cpu_to_le32(I2400M_NBOOT_BARKER)
};

static const __le32 i2400m_SBOOT_BARKER[4] = {
	cpu_to_le32(I2400M_SBOOT_BARKER),
	cpu_to_le32(I2400M_SBOOT_BARKER),
	cpu_to_le32(I2400M_SBOOT_BARKER),
	cpu_to_le32(I2400M_SBOOT_BARKER)
};

extern int i2400m_power_save_disabled;



static inline
struct device *i2400m_dev(struct i2400m *i2400m)
{
	return i2400m->wimax_dev.net_dev->dev.parent;
}


struct i2400m_work {
	struct work_struct ws;
	struct i2400m *i2400m;
	u8 pl[0];
};
extern int i2400m_queue_work(struct i2400m *,
			     void (*)(struct work_struct *), gfp_t,
				const void *, size_t);

extern int i2400m_msg_check_status(const struct i2400m_l3l4_hdr *,
				   char *, size_t);
extern int i2400m_msg_size_check(struct i2400m *,
				 const struct i2400m_l3l4_hdr *, size_t);
extern struct sk_buff *i2400m_msg_to_dev(struct i2400m *, const void *, size_t);
extern void i2400m_msg_to_dev_cancel_wait(struct i2400m *, int);
extern void i2400m_msg_ack_hook(struct i2400m *,
				const struct i2400m_l3l4_hdr *, size_t);
extern void i2400m_report_hook(struct i2400m *,
			       const struct i2400m_l3l4_hdr *, size_t);
extern int i2400m_cmd_enter_powersave(struct i2400m *);
extern int i2400m_cmd_get_state(struct i2400m *);
extern int i2400m_cmd_exit_idle(struct i2400m *);
extern struct sk_buff *i2400m_get_device_info(struct i2400m *);
extern int i2400m_firmware_check(struct i2400m *);
extern int i2400m_set_init_config(struct i2400m *,
				  const struct i2400m_tlv_hdr **, size_t);
extern int i2400m_set_idle_timeout(struct i2400m *, unsigned);

static inline
struct usb_endpoint_descriptor *usb_get_epd(struct usb_interface *iface, int ep)
{
	return &iface->cur_altsetting->endpoint[ep].desc;
}

extern int i2400m_op_rfkill_sw_toggle(struct wimax_dev *,
				      enum wimax_rf_state);
extern void i2400m_report_tlv_rf_switches_status(
	struct i2400m *, const struct i2400m_tlv_rf_switches_status *);


static inline
unsigned i2400m_le_v1_3(struct i2400m *i2400m)
{
	
	return i2400m->fw_version <= 0x00090001;
}

static inline
unsigned i2400m_ge_v1_4(struct i2400m *i2400m)
{
	
	return i2400m->fw_version >= 0x00090002;
}



static inline
void __i2400m_msleep(unsigned ms)
{
#if 1
#else
	msleep(ms);
#endif
}



extern int i2400m_idle_mode_disabled;
extern int i2400m_rx_reorder_disabled;


#endif 
