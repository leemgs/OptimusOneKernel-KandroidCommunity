

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/log2.h>
#include <linux/pci-aspm.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <asm/dma.h>	
#include <linux/device.h>
#include <asm/setup.h>
#include "pci.h"

const char *pci_power_names[] = {
	"error", "D0", "D1", "D2", "D3hot", "D3cold", "unknown",
};
EXPORT_SYMBOL_GPL(pci_power_names);

unsigned int pci_pm_d3_delay = PCI_PM_D3_WAIT;

#ifdef CONFIG_PCI_DOMAINS
int pci_domains_supported = 1;
#endif

#define DEFAULT_CARDBUS_IO_SIZE		(256)
#define DEFAULT_CARDBUS_MEM_SIZE	(64*1024*1024)

unsigned long pci_cardbus_io_size = DEFAULT_CARDBUS_IO_SIZE;
unsigned long pci_cardbus_mem_size = DEFAULT_CARDBUS_MEM_SIZE;

#define DEFAULT_HOTPLUG_IO_SIZE		(256)
#define DEFAULT_HOTPLUG_MEM_SIZE	(2*1024*1024)

unsigned long pci_hotplug_io_size  = DEFAULT_HOTPLUG_IO_SIZE;
unsigned long pci_hotplug_mem_size = DEFAULT_HOTPLUG_MEM_SIZE;


unsigned char pci_bus_max_busnr(struct pci_bus* bus)
{
	struct list_head *tmp;
	unsigned char max, n;

	max = bus->subordinate;
	list_for_each(tmp, &bus->children) {
		n = pci_bus_max_busnr(pci_bus_b(tmp));
		if(n > max)
			max = n;
	}
	return max;
}
EXPORT_SYMBOL_GPL(pci_bus_max_busnr);

#ifdef CONFIG_HAS_IOMEM
void __iomem *pci_ioremap_bar(struct pci_dev *pdev, int bar)
{
	
	if (!(pci_resource_flags(pdev, bar) & IORESOURCE_MEM)) {
		WARN_ON(1);
		return NULL;
	}
	return ioremap_nocache(pci_resource_start(pdev, bar),
				     pci_resource_len(pdev, bar));
}
EXPORT_SYMBOL_GPL(pci_ioremap_bar);
#endif

#if 0

unsigned char __devinit
pci_max_busnr(void)
{
	struct pci_bus *bus = NULL;
	unsigned char max, n;

	max = 0;
	while ((bus = pci_find_next_bus(bus)) != NULL) {
		n = pci_bus_max_busnr(bus);
		if(n > max)
			max = n;
	}
	return max;
}

#endif  

#define PCI_FIND_CAP_TTL	48

static int __pci_find_next_cap_ttl(struct pci_bus *bus, unsigned int devfn,
				   u8 pos, int cap, int *ttl)
{
	u8 id;

	while ((*ttl)--) {
		pci_bus_read_config_byte(bus, devfn, pos, &pos);
		if (pos < 0x40)
			break;
		pos &= ~3;
		pci_bus_read_config_byte(bus, devfn, pos + PCI_CAP_LIST_ID,
					 &id);
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos += PCI_CAP_LIST_NEXT;
	}
	return 0;
}

static int __pci_find_next_cap(struct pci_bus *bus, unsigned int devfn,
			       u8 pos, int cap)
{
	int ttl = PCI_FIND_CAP_TTL;

	return __pci_find_next_cap_ttl(bus, devfn, pos, cap, &ttl);
}

int pci_find_next_capability(struct pci_dev *dev, u8 pos, int cap)
{
	return __pci_find_next_cap(dev->bus, dev->devfn,
				   pos + PCI_CAP_LIST_NEXT, cap);
}
EXPORT_SYMBOL_GPL(pci_find_next_capability);

static int __pci_bus_find_cap_start(struct pci_bus *bus,
				    unsigned int devfn, u8 hdr_type)
{
	u16 status;

	pci_bus_read_config_word(bus, devfn, PCI_STATUS, &status);
	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;

	switch (hdr_type) {
	case PCI_HEADER_TYPE_NORMAL:
	case PCI_HEADER_TYPE_BRIDGE:
		return PCI_CAPABILITY_LIST;
	case PCI_HEADER_TYPE_CARDBUS:
		return PCI_CB_CAPABILITY_LIST;
	default:
		return 0;
	}

	return 0;
}


int pci_find_capability(struct pci_dev *dev, int cap)
{
	int pos;

	pos = __pci_bus_find_cap_start(dev->bus, dev->devfn, dev->hdr_type);
	if (pos)
		pos = __pci_find_next_cap(dev->bus, dev->devfn, pos, cap);

	return pos;
}


int pci_bus_find_capability(struct pci_bus *bus, unsigned int devfn, int cap)
{
	int pos;
	u8 hdr_type;

	pci_bus_read_config_byte(bus, devfn, PCI_HEADER_TYPE, &hdr_type);

	pos = __pci_bus_find_cap_start(bus, devfn, hdr_type & 0x7f);
	if (pos)
		pos = __pci_find_next_cap(bus, devfn, pos, cap);

	return pos;
}


int pci_find_ext_capability(struct pci_dev *dev, int cap)
{
	u32 header;
	int ttl;
	int pos = PCI_CFG_SPACE_SIZE;

	
	ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

	if (dev->cfg_size <= PCI_CFG_SPACE_SIZE)
		return 0;

	if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
		return 0;

	
	if (header == 0)
		return 0;

	while (ttl-- > 0) {
		if (PCI_EXT_CAP_ID(header) == cap)
			return pos;

		pos = PCI_EXT_CAP_NEXT(header);
		if (pos < PCI_CFG_SPACE_SIZE)
			break;

		if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
			break;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(pci_find_ext_capability);

static int __pci_find_next_ht_cap(struct pci_dev *dev, int pos, int ht_cap)
{
	int rc, ttl = PCI_FIND_CAP_TTL;
	u8 cap, mask;

	if (ht_cap == HT_CAPTYPE_SLAVE || ht_cap == HT_CAPTYPE_HOST)
		mask = HT_3BIT_CAP_MASK;
	else
		mask = HT_5BIT_CAP_MASK;

	pos = __pci_find_next_cap_ttl(dev->bus, dev->devfn, pos,
				      PCI_CAP_ID_HT, &ttl);
	while (pos) {
		rc = pci_read_config_byte(dev, pos + 3, &cap);
		if (rc != PCIBIOS_SUCCESSFUL)
			return 0;

		if ((cap & mask) == ht_cap)
			return pos;

		pos = __pci_find_next_cap_ttl(dev->bus, dev->devfn,
					      pos + PCI_CAP_LIST_NEXT,
					      PCI_CAP_ID_HT, &ttl);
	}

	return 0;
}

int pci_find_next_ht_capability(struct pci_dev *dev, int pos, int ht_cap)
{
	return __pci_find_next_ht_cap(dev, pos + PCI_CAP_LIST_NEXT, ht_cap);
}
EXPORT_SYMBOL_GPL(pci_find_next_ht_capability);


int pci_find_ht_capability(struct pci_dev *dev, int ht_cap)
{
	int pos;

	pos = __pci_bus_find_cap_start(dev->bus, dev->devfn, dev->hdr_type);
	if (pos)
		pos = __pci_find_next_ht_cap(dev, pos, ht_cap);

	return pos;
}
EXPORT_SYMBOL_GPL(pci_find_ht_capability);


struct resource *
pci_find_parent_resource(const struct pci_dev *dev, struct resource *res)
{
	const struct pci_bus *bus = dev->bus;
	int i;
	struct resource *best = NULL;

	for(i = 0; i < PCI_BUS_NUM_RESOURCES; i++) {
		struct resource *r = bus->resource[i];
		if (!r)
			continue;
		if (res->start && !(res->start >= r->start && res->end <= r->end))
			continue;	
		if ((res->flags ^ r->flags) & (IORESOURCE_IO | IORESOURCE_MEM))
			continue;	
		if (!((res->flags ^ r->flags) & IORESOURCE_PREFETCH))
			return r;	
		if ((res->flags & IORESOURCE_PREFETCH) && !(r->flags & IORESOURCE_PREFETCH))
			best = r;	
	}
	return best;
}


static void
pci_restore_bars(struct pci_dev *dev)
{
	int i;

	for (i = 0; i < PCI_BRIDGE_RESOURCES; i++)
		pci_update_resource(dev, i);
}

static struct pci_platform_pm_ops *pci_platform_pm;

int pci_set_platform_pm(struct pci_platform_pm_ops *ops)
{
	if (!ops->is_manageable || !ops->set_state || !ops->choose_state
	    || !ops->sleep_wake || !ops->can_wakeup)
		return -EINVAL;
	pci_platform_pm = ops;
	return 0;
}

static inline bool platform_pci_power_manageable(struct pci_dev *dev)
{
	return pci_platform_pm ? pci_platform_pm->is_manageable(dev) : false;
}

static inline int platform_pci_set_power_state(struct pci_dev *dev,
                                                pci_power_t t)
{
	return pci_platform_pm ? pci_platform_pm->set_state(dev, t) : -ENOSYS;
}

static inline pci_power_t platform_pci_choose_state(struct pci_dev *dev)
{
	return pci_platform_pm ?
			pci_platform_pm->choose_state(dev) : PCI_POWER_ERROR;
}

static inline bool platform_pci_can_wakeup(struct pci_dev *dev)
{
	return pci_platform_pm ? pci_platform_pm->can_wakeup(dev) : false;
}

static inline int platform_pci_sleep_wake(struct pci_dev *dev, bool enable)
{
	return pci_platform_pm ?
			pci_platform_pm->sleep_wake(dev, enable) : -ENODEV;
}


static int pci_raw_set_power_state(struct pci_dev *dev, pci_power_t state)
{
	u16 pmcsr;
	bool need_restore = false;

	
	if (dev->current_state == state)
		return 0;

	if (!dev->pm_cap)
		return -EIO;

	if (state < PCI_D0 || state > PCI_D3hot)
		return -EINVAL;

	
	if (state != PCI_D0 && dev->current_state <= PCI_D3cold
	    && dev->current_state > state) {
		dev_err(&dev->dev, "invalid power transition "
			"(from state %d to %d)\n", dev->current_state, state);
		return -EINVAL;
	}

	
	if ((state == PCI_D1 && !dev->d1_support)
	   || (state == PCI_D2 && !dev->d2_support))
		return -EIO;

	pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &pmcsr);

	
	switch (dev->current_state) {
	case PCI_D0:
	case PCI_D1:
	case PCI_D2:
		pmcsr &= ~PCI_PM_CTRL_STATE_MASK;
		pmcsr |= state;
		break;
	case PCI_D3hot:
	case PCI_D3cold:
	case PCI_UNKNOWN: 
		if ((pmcsr & PCI_PM_CTRL_STATE_MASK) == PCI_D3hot
		 && !(pmcsr & PCI_PM_CTRL_NO_SOFT_RESET))
			need_restore = true;
		
	default:
		pmcsr = 0;
		break;
	}

	
	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, pmcsr);

	
	
	if (state == PCI_D3hot || dev->current_state == PCI_D3hot)
		msleep(pci_pm_d3_delay);
	else if (state == PCI_D2 || dev->current_state == PCI_D2)
		udelay(PCI_PM_D2_DELAY);

	pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &pmcsr);
	dev->current_state = (pmcsr & PCI_PM_CTRL_STATE_MASK);
	if (dev->current_state != state && printk_ratelimit())
		dev_info(&dev->dev, "Refused to change power state, "
			"currently in D%d\n", dev->current_state);

	
	if (need_restore)
		pci_restore_bars(dev);

	if (dev->bus->self)
		pcie_aspm_pm_state_change(dev->bus->self);

	return 0;
}


void pci_update_current_state(struct pci_dev *dev, pci_power_t state)
{
	if (dev->pm_cap) {
		u16 pmcsr;

		pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &pmcsr);
		dev->current_state = (pmcsr & PCI_PM_CTRL_STATE_MASK);
	} else {
		dev->current_state = state;
	}
}


static int pci_platform_power_transition(struct pci_dev *dev, pci_power_t state)
{
	int error;

	if (platform_pci_power_manageable(dev)) {
		error = platform_pci_set_power_state(dev, state);
		if (!error)
			pci_update_current_state(dev, state);
	} else {
		error = -ENODEV;
		
		if (!dev->pm_cap)
			dev->current_state = PCI_D0;
	}

	return error;
}


static void __pci_start_power_transition(struct pci_dev *dev, pci_power_t state)
{
	if (state == PCI_D0)
		pci_platform_power_transition(dev, PCI_D0);
}


int __pci_complete_power_transition(struct pci_dev *dev, pci_power_t state)
{
	return state > PCI_D0 ?
			pci_platform_power_transition(dev, state) : -EINVAL;
}
EXPORT_SYMBOL_GPL(__pci_complete_power_transition);


int pci_set_power_state(struct pci_dev *dev, pci_power_t state)
{
	int error;

	
	if (state > PCI_D3hot)
		state = PCI_D3hot;
	else if (state < PCI_D0)
		state = PCI_D0;
	else if ((state == PCI_D1 || state == PCI_D2) && pci_no_d1d2(dev))
		
		return 0;

	
	if (dev->current_state == state)
		return 0;

	__pci_start_power_transition(dev, state);

	
	if (state == PCI_D3hot && (dev->dev_flags & PCI_DEV_FLAGS_NO_D3))
		return 0;

	error = pci_raw_set_power_state(dev, state);

	if (!__pci_complete_power_transition(dev, state))
		error = 0;

	return error;
}



pci_power_t pci_choose_state(struct pci_dev *dev, pm_message_t state)
{
	pci_power_t ret;

	if (!pci_find_capability(dev, PCI_CAP_ID_PM))
		return PCI_D0;

	ret = platform_pci_choose_state(dev);
	if (ret != PCI_POWER_ERROR)
		return ret;

	switch (state.event) {
	case PM_EVENT_ON:
		return PCI_D0;
	case PM_EVENT_FREEZE:
	case PM_EVENT_PRETHAW:
		
	case PM_EVENT_SUSPEND:
	case PM_EVENT_HIBERNATE:
		return PCI_D3hot;
	default:
		dev_info(&dev->dev, "unrecognized suspend event %d\n",
			 state.event);
		BUG();
	}
	return PCI_D0;
}

EXPORT_SYMBOL(pci_choose_state);

#define PCI_EXP_SAVE_REGS	7

#define pcie_cap_has_devctl(type, flags)	1
#define pcie_cap_has_lnkctl(type, flags)		\
		((flags & PCI_EXP_FLAGS_VERS) > 1 ||	\
		 (type == PCI_EXP_TYPE_ROOT_PORT ||	\
		  type == PCI_EXP_TYPE_ENDPOINT ||	\
		  type == PCI_EXP_TYPE_LEG_END))
#define pcie_cap_has_sltctl(type, flags)		\
		((flags & PCI_EXP_FLAGS_VERS) > 1 ||	\
		 ((type == PCI_EXP_TYPE_ROOT_PORT) ||	\
		  (type == PCI_EXP_TYPE_DOWNSTREAM &&	\
		   (flags & PCI_EXP_FLAGS_SLOT))))
#define pcie_cap_has_rtctl(type, flags)			\
		((flags & PCI_EXP_FLAGS_VERS) > 1 ||	\
		 (type == PCI_EXP_TYPE_ROOT_PORT ||	\
		  type == PCI_EXP_TYPE_RC_EC))
#define pcie_cap_has_devctl2(type, flags)		\
		((flags & PCI_EXP_FLAGS_VERS) > 1)
#define pcie_cap_has_lnkctl2(type, flags)		\
		((flags & PCI_EXP_FLAGS_VERS) > 1)
#define pcie_cap_has_sltctl2(type, flags)		\
		((flags & PCI_EXP_FLAGS_VERS) > 1)

static int pci_save_pcie_state(struct pci_dev *dev)
{
	int pos, i = 0;
	struct pci_cap_saved_state *save_state;
	u16 *cap;
	u16 flags;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (pos <= 0)
		return 0;

	save_state = pci_find_saved_cap(dev, PCI_CAP_ID_EXP);
	if (!save_state) {
		dev_err(&dev->dev, "buffer not found in %s\n", __func__);
		return -ENOMEM;
	}
	cap = (u16 *)&save_state->data[0];

	pci_read_config_word(dev, pos + PCI_EXP_FLAGS, &flags);

	if (pcie_cap_has_devctl(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_DEVCTL, &cap[i++]);
	if (pcie_cap_has_lnkctl(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_LNKCTL, &cap[i++]);
	if (pcie_cap_has_sltctl(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_SLTCTL, &cap[i++]);
	if (pcie_cap_has_rtctl(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_RTCTL, &cap[i++]);
	if (pcie_cap_has_devctl2(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_DEVCTL2, &cap[i++]);
	if (pcie_cap_has_lnkctl2(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_LNKCTL2, &cap[i++]);
	if (pcie_cap_has_sltctl2(dev->pcie_type, flags))
		pci_read_config_word(dev, pos + PCI_EXP_SLTCTL2, &cap[i++]);

	return 0;
}

static void pci_restore_pcie_state(struct pci_dev *dev)
{
	int i = 0, pos;
	struct pci_cap_saved_state *save_state;
	u16 *cap;
	u16 flags;

	save_state = pci_find_saved_cap(dev, PCI_CAP_ID_EXP);
	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!save_state || pos <= 0)
		return;
	cap = (u16 *)&save_state->data[0];

	pci_read_config_word(dev, pos + PCI_EXP_FLAGS, &flags);

	if (pcie_cap_has_devctl(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_DEVCTL, cap[i++]);
	if (pcie_cap_has_lnkctl(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_LNKCTL, cap[i++]);
	if (pcie_cap_has_sltctl(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_SLTCTL, cap[i++]);
	if (pcie_cap_has_rtctl(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_RTCTL, cap[i++]);
	if (pcie_cap_has_devctl2(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_DEVCTL2, cap[i++]);
	if (pcie_cap_has_lnkctl2(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_LNKCTL2, cap[i++]);
	if (pcie_cap_has_sltctl2(dev->pcie_type, flags))
		pci_write_config_word(dev, pos + PCI_EXP_SLTCTL2, cap[i++]);
}


static int pci_save_pcix_state(struct pci_dev *dev)
{
	int pos;
	struct pci_cap_saved_state *save_state;

	pos = pci_find_capability(dev, PCI_CAP_ID_PCIX);
	if (pos <= 0)
		return 0;

	save_state = pci_find_saved_cap(dev, PCI_CAP_ID_PCIX);
	if (!save_state) {
		dev_err(&dev->dev, "buffer not found in %s\n", __func__);
		return -ENOMEM;
	}

	pci_read_config_word(dev, pos + PCI_X_CMD, (u16 *)save_state->data);

	return 0;
}

static void pci_restore_pcix_state(struct pci_dev *dev)
{
	int i = 0, pos;
	struct pci_cap_saved_state *save_state;
	u16 *cap;

	save_state = pci_find_saved_cap(dev, PCI_CAP_ID_PCIX);
	pos = pci_find_capability(dev, PCI_CAP_ID_PCIX);
	if (!save_state || pos <= 0)
		return;
	cap = (u16 *)&save_state->data[0];

	pci_write_config_word(dev, pos + PCI_X_CMD, cap[i++]);
}



int
pci_save_state(struct pci_dev *dev)
{
	int i;
	
	for (i = 0; i < 16; i++)
		pci_read_config_dword(dev, i * 4,&dev->saved_config_space[i]);
	dev->state_saved = true;
	if ((i = pci_save_pcie_state(dev)) != 0)
		return i;
	if ((i = pci_save_pcix_state(dev)) != 0)
		return i;
	return 0;
}


int 
pci_restore_state(struct pci_dev *dev)
{
	int i;
	u32 val;

	if (!dev->state_saved)
		return 0;

	
	pci_restore_pcie_state(dev);

	
	for (i = 15; i >= 0; i--) {
		pci_read_config_dword(dev, i * 4, &val);
		if (val != dev->saved_config_space[i]) {
			dev_printk(KERN_DEBUG, &dev->dev, "restoring config "
				"space at offset %#x (was %#x, writing %#x)\n",
				i, val, (int)dev->saved_config_space[i]);
			pci_write_config_dword(dev,i * 4,
				dev->saved_config_space[i]);
		}
	}
	pci_restore_pcix_state(dev);
	pci_restore_msi_state(dev);
	pci_restore_iov_state(dev);

	dev->state_saved = false;

	return 0;
}

static int do_pci_enable_device(struct pci_dev *dev, int bars)
{
	int err;

	err = pci_set_power_state(dev, PCI_D0);
	if (err < 0 && err != -EIO)
		return err;
	err = pcibios_enable_device(dev, bars);
	if (err < 0)
		return err;
	pci_fixup_device(pci_fixup_enable, dev);

	return 0;
}


int pci_reenable_device(struct pci_dev *dev)
{
	if (pci_is_enabled(dev))
		return do_pci_enable_device(dev, (1 << PCI_NUM_RESOURCES) - 1);
	return 0;
}

static int __pci_enable_device_flags(struct pci_dev *dev,
				     resource_size_t flags)
{
	int err;
	int i, bars = 0;

	if (atomic_add_return(1, &dev->enable_cnt) > 1)
		return 0;		

	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++)
		if (dev->resource[i].flags & flags)
			bars |= (1 << i);

	err = do_pci_enable_device(dev, bars);
	if (err < 0)
		atomic_dec(&dev->enable_cnt);
	return err;
}


int pci_enable_device_io(struct pci_dev *dev)
{
	return __pci_enable_device_flags(dev, IORESOURCE_IO);
}


int pci_enable_device_mem(struct pci_dev *dev)
{
	return __pci_enable_device_flags(dev, IORESOURCE_MEM);
}


int pci_enable_device(struct pci_dev *dev)
{
	return __pci_enable_device_flags(dev, IORESOURCE_MEM | IORESOURCE_IO);
}


struct pci_devres {
	unsigned int enabled:1;
	unsigned int pinned:1;
	unsigned int orig_intx:1;
	unsigned int restore_intx:1;
	u32 region_mask;
};

static void pcim_release(struct device *gendev, void *res)
{
	struct pci_dev *dev = container_of(gendev, struct pci_dev, dev);
	struct pci_devres *this = res;
	int i;

	if (dev->msi_enabled)
		pci_disable_msi(dev);
	if (dev->msix_enabled)
		pci_disable_msix(dev);

	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++)
		if (this->region_mask & (1 << i))
			pci_release_region(dev, i);

	if (this->restore_intx)
		pci_intx(dev, this->orig_intx);

	if (this->enabled && !this->pinned)
		pci_disable_device(dev);
}

static struct pci_devres * get_pci_dr(struct pci_dev *pdev)
{
	struct pci_devres *dr, *new_dr;

	dr = devres_find(&pdev->dev, pcim_release, NULL, NULL);
	if (dr)
		return dr;

	new_dr = devres_alloc(pcim_release, sizeof(*new_dr), GFP_KERNEL);
	if (!new_dr)
		return NULL;
	return devres_get(&pdev->dev, new_dr, NULL, NULL);
}

static struct pci_devres * find_pci_dr(struct pci_dev *pdev)
{
	if (pci_is_managed(pdev))
		return devres_find(&pdev->dev, pcim_release, NULL, NULL);
	return NULL;
}


int pcim_enable_device(struct pci_dev *pdev)
{
	struct pci_devres *dr;
	int rc;

	dr = get_pci_dr(pdev);
	if (unlikely(!dr))
		return -ENOMEM;
	if (dr->enabled)
		return 0;

	rc = pci_enable_device(pdev);
	if (!rc) {
		pdev->is_managed = 1;
		dr->enabled = 1;
	}
	return rc;
}


void pcim_pin_device(struct pci_dev *pdev)
{
	struct pci_devres *dr;

	dr = find_pci_dr(pdev);
	WARN_ON(!dr || !dr->enabled);
	if (dr)
		dr->pinned = 1;
}


void __attribute__ ((weak)) pcibios_disable_device (struct pci_dev *dev) {}

static void do_pci_disable_device(struct pci_dev *dev)
{
	u16 pci_command;

	pci_read_config_word(dev, PCI_COMMAND, &pci_command);
	if (pci_command & PCI_COMMAND_MASTER) {
		pci_command &= ~PCI_COMMAND_MASTER;
		pci_write_config_word(dev, PCI_COMMAND, pci_command);
	}

	pcibios_disable_device(dev);
}


void pci_disable_enabled_device(struct pci_dev *dev)
{
	if (pci_is_enabled(dev))
		do_pci_disable_device(dev);
}


void
pci_disable_device(struct pci_dev *dev)
{
	struct pci_devres *dr;

	dr = find_pci_dr(dev);
	if (dr)
		dr->enabled = 0;

	if (atomic_sub_return(1, &dev->enable_cnt) != 0)
		return;

	do_pci_disable_device(dev);

	dev->is_busmaster = 0;
}


int __attribute__ ((weak)) pcibios_set_pcie_reset_state(struct pci_dev *dev,
							enum pcie_reset_state state)
{
	return -EINVAL;
}


int pci_set_pcie_reset_state(struct pci_dev *dev, enum pcie_reset_state state)
{
	return pcibios_set_pcie_reset_state(dev, state);
}


bool pci_pme_capable(struct pci_dev *dev, pci_power_t state)
{
	if (!dev->pm_cap)
		return false;

	return !!(dev->pme_support & (1 << state));
}


void pci_pme_active(struct pci_dev *dev, bool enable)
{
	u16 pmcsr;

	if (!dev->pm_cap)
		return;

	pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &pmcsr);
	
	pmcsr |= PCI_PM_CTRL_PME_STATUS | PCI_PM_CTRL_PME_ENABLE;
	if (!enable)
		pmcsr &= ~PCI_PM_CTRL_PME_ENABLE;

	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, pmcsr);

	dev_printk(KERN_INFO, &dev->dev, "PME# %s\n",
			enable ? "enabled" : "disabled");
}


int pci_enable_wake(struct pci_dev *dev, pci_power_t state, bool enable)
{
	int ret = 0;

	if (enable && !device_may_wakeup(&dev->dev))
		return -EINVAL;

	
	if (!!enable == !!dev->wakeup_prepared)
		return 0;

	

	if (enable) {
		int error;

		if (pci_pme_capable(dev, state))
			pci_pme_active(dev, true);
		else
			ret = 1;
		error = platform_pci_sleep_wake(dev, true);
		if (ret)
			ret = error;
		if (!ret)
			dev->wakeup_prepared = true;
	} else {
		platform_pci_sleep_wake(dev, false);
		pci_pme_active(dev, false);
		dev->wakeup_prepared = false;
	}

	return ret;
}


int pci_wake_from_d3(struct pci_dev *dev, bool enable)
{
	return pci_pme_capable(dev, PCI_D3cold) ?
			pci_enable_wake(dev, PCI_D3cold, enable) :
			pci_enable_wake(dev, PCI_D3hot, enable);
}


pci_power_t pci_target_state(struct pci_dev *dev)
{
	pci_power_t target_state = PCI_D3hot;

	if (platform_pci_power_manageable(dev)) {
		
		pci_power_t state = platform_pci_choose_state(dev);

		switch (state) {
		case PCI_POWER_ERROR:
		case PCI_UNKNOWN:
			break;
		case PCI_D1:
		case PCI_D2:
			if (pci_no_d1d2(dev))
				break;
		default:
			target_state = state;
		}
	} else if (!dev->pm_cap) {
		target_state = PCI_D0;
	} else if (device_may_wakeup(&dev->dev)) {
		
		if (dev->pme_support) {
			while (target_state
			      && !(dev->pme_support & (1 << target_state)))
				target_state--;
		}
	}

	return target_state;
}


int pci_prepare_to_sleep(struct pci_dev *dev)
{
	pci_power_t target_state = pci_target_state(dev);
	int error;

	if (target_state == PCI_POWER_ERROR)
		return -EIO;

	pci_enable_wake(dev, target_state, device_may_wakeup(&dev->dev));

	error = pci_set_power_state(dev, target_state);

	if (error)
		pci_enable_wake(dev, target_state, false);

	return error;
}


int pci_back_from_sleep(struct pci_dev *dev)
{
	pci_enable_wake(dev, PCI_D0, false);
	return pci_set_power_state(dev, PCI_D0);
}


void pci_pm_init(struct pci_dev *dev)
{
	int pm;
	u16 pmc;

	dev->wakeup_prepared = false;
	dev->pm_cap = 0;

	
	pm = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (!pm)
		return;
	
	pci_read_config_word(dev, pm + PCI_PM_PMC, &pmc);

	if ((pmc & PCI_PM_CAP_VER_MASK) > 3) {
		dev_err(&dev->dev, "unsupported PM cap regs version (%u)\n",
			pmc & PCI_PM_CAP_VER_MASK);
		return;
	}

	dev->pm_cap = pm;

	dev->d1_support = false;
	dev->d2_support = false;
	if (!pci_no_d1d2(dev)) {
		if (pmc & PCI_PM_CAP_D1)
			dev->d1_support = true;
		if (pmc & PCI_PM_CAP_D2)
			dev->d2_support = true;

		if (dev->d1_support || dev->d2_support)
			dev_printk(KERN_DEBUG, &dev->dev, "supports%s%s\n",
				   dev->d1_support ? " D1" : "",
				   dev->d2_support ? " D2" : "");
	}

	pmc &= PCI_PM_CAP_PME_MASK;
	if (pmc) {
		dev_info(&dev->dev, "PME# supported from%s%s%s%s%s\n",
			 (pmc & PCI_PM_CAP_PME_D0) ? " D0" : "",
			 (pmc & PCI_PM_CAP_PME_D1) ? " D1" : "",
			 (pmc & PCI_PM_CAP_PME_D2) ? " D2" : "",
			 (pmc & PCI_PM_CAP_PME_D3) ? " D3hot" : "",
			 (pmc & PCI_PM_CAP_PME_D3cold) ? " D3cold" : "");
		dev->pme_support = pmc >> PCI_PM_CAP_PME_SHIFT;
		
		device_set_wakeup_capable(&dev->dev, true);
		device_set_wakeup_enable(&dev->dev, false);
		
		pci_pme_active(dev, false);
	} else {
		dev->pme_support = 0;
	}
}


void platform_pci_wakeup_init(struct pci_dev *dev)
{
	if (!platform_pci_can_wakeup(dev))
		return;

	device_set_wakeup_capable(&dev->dev, true);
	device_set_wakeup_enable(&dev->dev, false);
	platform_pci_sleep_wake(dev, false);
}


static int pci_add_cap_save_buffer(
	struct pci_dev *dev, char cap, unsigned int size)
{
	int pos;
	struct pci_cap_saved_state *save_state;

	pos = pci_find_capability(dev, cap);
	if (pos <= 0)
		return 0;

	save_state = kzalloc(sizeof(*save_state) + size, GFP_KERNEL);
	if (!save_state)
		return -ENOMEM;

	save_state->cap_nr = cap;
	pci_add_saved_cap(dev, save_state);

	return 0;
}


void pci_allocate_cap_save_buffers(struct pci_dev *dev)
{
	int error;

	error = pci_add_cap_save_buffer(dev, PCI_CAP_ID_EXP,
					PCI_EXP_SAVE_REGS * sizeof(u16));
	if (error)
		dev_err(&dev->dev,
			"unable to preallocate PCI Express save buffer\n");

	error = pci_add_cap_save_buffer(dev, PCI_CAP_ID_PCIX, sizeof(u16));
	if (error)
		dev_err(&dev->dev,
			"unable to preallocate PCI-X save buffer\n");
}


void pci_enable_ari(struct pci_dev *dev)
{
	int pos;
	u32 cap;
	u16 ctrl;
	struct pci_dev *bridge;

	if (!dev->is_pcie || dev->devfn)
		return;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ARI);
	if (!pos)
		return;

	bridge = dev->bus->self;
	if (!bridge || !bridge->is_pcie)
		return;

	pos = pci_find_capability(bridge, PCI_CAP_ID_EXP);
	if (!pos)
		return;

	pci_read_config_dword(bridge, pos + PCI_EXP_DEVCAP2, &cap);
	if (!(cap & PCI_EXP_DEVCAP2_ARI))
		return;

	pci_read_config_word(bridge, pos + PCI_EXP_DEVCTL2, &ctrl);
	ctrl |= PCI_EXP_DEVCTL2_ARI;
	pci_write_config_word(bridge, pos + PCI_EXP_DEVCTL2, ctrl);

	bridge->ari_enabled = 1;
}


u8 pci_swizzle_interrupt_pin(struct pci_dev *dev, u8 pin)
{
	int slot;

	if (pci_ari_enabled(dev->bus))
		slot = 0;
	else
		slot = PCI_SLOT(dev->devfn);

	return (((pin - 1) + slot) % 4) + 1;
}

int
pci_get_interrupt_pin(struct pci_dev *dev, struct pci_dev **bridge)
{
	u8 pin;

	pin = dev->pin;
	if (!pin)
		return -1;

	while (!pci_is_root_bus(dev->bus)) {
		pin = pci_swizzle_interrupt_pin(dev, pin);
		dev = dev->bus->self;
	}
	*bridge = dev;
	return pin;
}


u8 pci_common_swizzle(struct pci_dev *dev, u8 *pinp)
{
	u8 pin = *pinp;

	while (!pci_is_root_bus(dev->bus)) {
		pin = pci_swizzle_interrupt_pin(dev, pin);
		dev = dev->bus->self;
	}
	*pinp = pin;
	return PCI_SLOT(dev->devfn);
}


void pci_release_region(struct pci_dev *pdev, int bar)
{
	struct pci_devres *dr;

	if (pci_resource_len(pdev, bar) == 0)
		return;
	if (pci_resource_flags(pdev, bar) & IORESOURCE_IO)
		release_region(pci_resource_start(pdev, bar),
				pci_resource_len(pdev, bar));
	else if (pci_resource_flags(pdev, bar) & IORESOURCE_MEM)
		release_mem_region(pci_resource_start(pdev, bar),
				pci_resource_len(pdev, bar));

	dr = find_pci_dr(pdev);
	if (dr)
		dr->region_mask &= ~(1 << bar);
}


static int __pci_request_region(struct pci_dev *pdev, int bar, const char *res_name,
									int exclusive)
{
	struct pci_devres *dr;

	if (pci_resource_len(pdev, bar) == 0)
		return 0;
		
	if (pci_resource_flags(pdev, bar) & IORESOURCE_IO) {
		if (!request_region(pci_resource_start(pdev, bar),
			    pci_resource_len(pdev, bar), res_name))
			goto err_out;
	}
	else if (pci_resource_flags(pdev, bar) & IORESOURCE_MEM) {
		if (!__request_mem_region(pci_resource_start(pdev, bar),
					pci_resource_len(pdev, bar), res_name,
					exclusive))
			goto err_out;
	}

	dr = find_pci_dr(pdev);
	if (dr)
		dr->region_mask |= 1 << bar;

	return 0;

err_out:
	dev_warn(&pdev->dev, "BAR %d: can't reserve %s region %pR\n",
		 bar,
		 pci_resource_flags(pdev, bar) & IORESOURCE_IO ? "I/O" : "mem",
		 &pdev->resource[bar]);
	return -EBUSY;
}


int pci_request_region(struct pci_dev *pdev, int bar, const char *res_name)
{
	return __pci_request_region(pdev, bar, res_name, 0);
}


int pci_request_region_exclusive(struct pci_dev *pdev, int bar, const char *res_name)
{
	return __pci_request_region(pdev, bar, res_name, IORESOURCE_EXCLUSIVE);
}

void pci_release_selected_regions(struct pci_dev *pdev, int bars)
{
	int i;

	for (i = 0; i < 6; i++)
		if (bars & (1 << i))
			pci_release_region(pdev, i);
}

int __pci_request_selected_regions(struct pci_dev *pdev, int bars,
				 const char *res_name, int excl)
{
	int i;

	for (i = 0; i < 6; i++)
		if (bars & (1 << i))
			if (__pci_request_region(pdev, i, res_name, excl))
				goto err_out;
	return 0;

err_out:
	while(--i >= 0)
		if (bars & (1 << i))
			pci_release_region(pdev, i);

	return -EBUSY;
}



int pci_request_selected_regions(struct pci_dev *pdev, int bars,
				 const char *res_name)
{
	return __pci_request_selected_regions(pdev, bars, res_name, 0);
}

int pci_request_selected_regions_exclusive(struct pci_dev *pdev,
				 int bars, const char *res_name)
{
	return __pci_request_selected_regions(pdev, bars, res_name,
			IORESOURCE_EXCLUSIVE);
}



void pci_release_regions(struct pci_dev *pdev)
{
	pci_release_selected_regions(pdev, (1 << 6) - 1);
}


int pci_request_regions(struct pci_dev *pdev, const char *res_name)
{
	return pci_request_selected_regions(pdev, ((1 << 6) - 1), res_name);
}


int pci_request_regions_exclusive(struct pci_dev *pdev, const char *res_name)
{
	return pci_request_selected_regions_exclusive(pdev,
					((1 << 6) - 1), res_name);
}

static void __pci_set_master(struct pci_dev *dev, bool enable)
{
	u16 old_cmd, cmd;

	pci_read_config_word(dev, PCI_COMMAND, &old_cmd);
	if (enable)
		cmd = old_cmd | PCI_COMMAND_MASTER;
	else
		cmd = old_cmd & ~PCI_COMMAND_MASTER;
	if (cmd != old_cmd) {
		dev_dbg(&dev->dev, "%s bus mastering\n",
			enable ? "enabling" : "disabling");
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	dev->is_busmaster = enable;
}


void pci_set_master(struct pci_dev *dev)
{
	__pci_set_master(dev, true);
	pcibios_set_master(dev);
}


void pci_clear_master(struct pci_dev *dev)
{
	__pci_set_master(dev, false);
}

#ifdef PCI_DISABLE_MWI
int pci_set_mwi(struct pci_dev *dev)
{
	return 0;
}

int pci_try_set_mwi(struct pci_dev *dev)
{
	return 0;
}

void pci_clear_mwi(struct pci_dev *dev)
{
}

#else

#ifndef PCI_CACHE_LINE_BYTES
#define PCI_CACHE_LINE_BYTES L1_CACHE_BYTES
#endif



u8 pci_cache_line_size = PCI_CACHE_LINE_BYTES / 4;


static int
pci_set_cacheline_size(struct pci_dev *dev)
{
	u8 cacheline_size;

	if (!pci_cache_line_size)
		return -EINVAL;		

	
	pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &cacheline_size);
	if (cacheline_size >= pci_cache_line_size &&
	    (cacheline_size % pci_cache_line_size) == 0)
		return 0;

	
	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, pci_cache_line_size);
	
	pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &cacheline_size);
	if (cacheline_size == pci_cache_line_size)
		return 0;

	dev_printk(KERN_DEBUG, &dev->dev, "cache line size of %d is not "
		   "supported\n", pci_cache_line_size << 2);

	return -EINVAL;
}


int
pci_set_mwi(struct pci_dev *dev)
{
	int rc;
	u16 cmd;

	rc = pci_set_cacheline_size(dev);
	if (rc)
		return rc;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	if (! (cmd & PCI_COMMAND_INVALIDATE)) {
		dev_dbg(&dev->dev, "enabling Mem-Wr-Inval\n");
		cmd |= PCI_COMMAND_INVALIDATE;
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	
	return 0;
}


int pci_try_set_mwi(struct pci_dev *dev)
{
	int rc = pci_set_mwi(dev);
	return rc;
}


void
pci_clear_mwi(struct pci_dev *dev)
{
	u16 cmd;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	if (cmd & PCI_COMMAND_INVALIDATE) {
		cmd &= ~PCI_COMMAND_INVALIDATE;
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
}
#endif 


void
pci_intx(struct pci_dev *pdev, int enable)
{
	u16 pci_command, new;

	pci_read_config_word(pdev, PCI_COMMAND, &pci_command);

	if (enable) {
		new = pci_command & ~PCI_COMMAND_INTX_DISABLE;
	} else {
		new = pci_command | PCI_COMMAND_INTX_DISABLE;
	}

	if (new != pci_command) {
		struct pci_devres *dr;

		pci_write_config_word(pdev, PCI_COMMAND, new);

		dr = find_pci_dr(pdev);
		if (dr && !dr->restore_intx) {
			dr->restore_intx = 1;
			dr->orig_intx = !enable;
		}
	}
}


void pci_msi_off(struct pci_dev *dev)
{
	int pos;
	u16 control;

	pos = pci_find_capability(dev, PCI_CAP_ID_MSI);
	if (pos) {
		pci_read_config_word(dev, pos + PCI_MSI_FLAGS, &control);
		control &= ~PCI_MSI_FLAGS_ENABLE;
		pci_write_config_word(dev, pos + PCI_MSI_FLAGS, control);
	}
	pos = pci_find_capability(dev, PCI_CAP_ID_MSIX);
	if (pos) {
		pci_read_config_word(dev, pos + PCI_MSIX_FLAGS, &control);
		control &= ~PCI_MSIX_FLAGS_ENABLE;
		pci_write_config_word(dev, pos + PCI_MSIX_FLAGS, control);
	}
}

#ifndef HAVE_ARCH_PCI_SET_DMA_MASK

int
pci_set_dma_mask(struct pci_dev *dev, u64 mask)
{
	if (!pci_dma_supported(dev, mask))
		return -EIO;

	dev->dma_mask = mask;

	return 0;
}
    
int
pci_set_consistent_dma_mask(struct pci_dev *dev, u64 mask)
{
	if (!pci_dma_supported(dev, mask))
		return -EIO;

	dev->dev.coherent_dma_mask = mask;

	return 0;
}
#endif

#ifndef HAVE_ARCH_PCI_SET_DMA_MAX_SEGMENT_SIZE
int pci_set_dma_max_seg_size(struct pci_dev *dev, unsigned int size)
{
	return dma_set_max_seg_size(&dev->dev, size);
}
EXPORT_SYMBOL(pci_set_dma_max_seg_size);
#endif

#ifndef HAVE_ARCH_PCI_SET_DMA_SEGMENT_BOUNDARY
int pci_set_dma_seg_boundary(struct pci_dev *dev, unsigned long mask)
{
	return dma_set_seg_boundary(&dev->dev, mask);
}
EXPORT_SYMBOL(pci_set_dma_seg_boundary);
#endif

static int pcie_flr(struct pci_dev *dev, int probe)
{
	int i;
	int pos;
	u32 cap;
	u16 status;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return -ENOTTY;

	pci_read_config_dword(dev, pos + PCI_EXP_DEVCAP, &cap);
	if (!(cap & PCI_EXP_DEVCAP_FLR))
		return -ENOTTY;

	if (probe)
		return 0;

	
	for (i = 0; i < 4; i++) {
		if (i)
			msleep((1 << (i - 1)) * 100);

		pci_read_config_word(dev, pos + PCI_EXP_DEVSTA, &status);
		if (!(status & PCI_EXP_DEVSTA_TRPND))
			goto clear;
	}

	dev_err(&dev->dev, "transaction is not cleared; "
			"proceeding with reset anyway\n");

clear:
	pci_write_config_word(dev, pos + PCI_EXP_DEVCTL,
				PCI_EXP_DEVCTL_BCR_FLR);
	msleep(100);

	return 0;
}

static int pci_af_flr(struct pci_dev *dev, int probe)
{
	int i;
	int pos;
	u8 cap;
	u8 status;

	pos = pci_find_capability(dev, PCI_CAP_ID_AF);
	if (!pos)
		return -ENOTTY;

	pci_read_config_byte(dev, pos + PCI_AF_CAP, &cap);
	if (!(cap & PCI_AF_CAP_TP) || !(cap & PCI_AF_CAP_FLR))
		return -ENOTTY;

	if (probe)
		return 0;

	
	for (i = 0; i < 4; i++) {
		if (i)
			msleep((1 << (i - 1)) * 100);

		pci_read_config_byte(dev, pos + PCI_AF_STATUS, &status);
		if (!(status & PCI_AF_STATUS_TP))
			goto clear;
	}

	dev_err(&dev->dev, "transaction is not cleared; "
			"proceeding with reset anyway\n");

clear:
	pci_write_config_byte(dev, pos + PCI_AF_CTRL, PCI_AF_CTRL_FLR);
	msleep(100);

	return 0;
}

static int pci_pm_reset(struct pci_dev *dev, int probe)
{
	u16 csr;

	if (!dev->pm_cap)
		return -ENOTTY;

	pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &csr);
	if (csr & PCI_PM_CTRL_NO_SOFT_RESET)
		return -ENOTTY;

	if (probe)
		return 0;

	if (dev->current_state != PCI_D0)
		return -EINVAL;

	csr &= ~PCI_PM_CTRL_STATE_MASK;
	csr |= PCI_D3hot;
	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, csr);
	msleep(pci_pm_d3_delay);

	csr &= ~PCI_PM_CTRL_STATE_MASK;
	csr |= PCI_D0;
	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, csr);
	msleep(pci_pm_d3_delay);

	return 0;
}

static int pci_parent_bus_reset(struct pci_dev *dev, int probe)
{
	u16 ctrl;
	struct pci_dev *pdev;

	if (pci_is_root_bus(dev->bus) || dev->subordinate || !dev->bus->self)
		return -ENOTTY;

	list_for_each_entry(pdev, &dev->bus->devices, bus_list)
		if (pdev != dev)
			return -ENOTTY;

	if (probe)
		return 0;

	pci_read_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, &ctrl);
	ctrl |= PCI_BRIDGE_CTL_BUS_RESET;
	pci_write_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, ctrl);
	msleep(100);

	ctrl &= ~PCI_BRIDGE_CTL_BUS_RESET;
	pci_write_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, ctrl);
	msleep(100);

	return 0;
}

static int pci_dev_reset(struct pci_dev *dev, int probe)
{
	int rc;

	might_sleep();

	if (!probe) {
		pci_block_user_cfg_access(dev);
		
		down(&dev->dev.sem);
	}

	rc = pcie_flr(dev, probe);
	if (rc != -ENOTTY)
		goto done;

	rc = pci_af_flr(dev, probe);
	if (rc != -ENOTTY)
		goto done;

	rc = pci_pm_reset(dev, probe);
	if (rc != -ENOTTY)
		goto done;

	rc = pci_parent_bus_reset(dev, probe);
done:
	if (!probe) {
		up(&dev->dev.sem);
		pci_unblock_user_cfg_access(dev);
	}

	return rc;
}


int __pci_reset_function(struct pci_dev *dev)
{
	return pci_dev_reset(dev, 0);
}
EXPORT_SYMBOL_GPL(__pci_reset_function);


int pci_probe_reset_function(struct pci_dev *dev)
{
	return pci_dev_reset(dev, 1);
}


int pci_reset_function(struct pci_dev *dev)
{
	int rc;

	rc = pci_dev_reset(dev, 1);
	if (rc)
		return rc;

	pci_save_state(dev);

	
	pci_write_config_word(dev, PCI_COMMAND, PCI_COMMAND_INTX_DISABLE);

	rc = pci_dev_reset(dev, 0);

	pci_restore_state(dev);

	return rc;
}
EXPORT_SYMBOL_GPL(pci_reset_function);


int pcix_get_max_mmrbc(struct pci_dev *dev)
{
	int err, cap;
	u32 stat;

	cap = pci_find_capability(dev, PCI_CAP_ID_PCIX);
	if (!cap)
		return -EINVAL;

	err = pci_read_config_dword(dev, cap + PCI_X_STATUS, &stat);
	if (err)
		return -EINVAL;

	return (stat & PCI_X_STATUS_MAX_READ) >> 12;
}
EXPORT_SYMBOL(pcix_get_max_mmrbc);


int pcix_get_mmrbc(struct pci_dev *dev)
{
	int ret, cap;
	u32 cmd;

	cap = pci_find_capability(dev, PCI_CAP_ID_PCIX);
	if (!cap)
		return -EINVAL;

	ret = pci_read_config_dword(dev, cap + PCI_X_CMD, &cmd);
	if (!ret)
		ret = 512 << ((cmd & PCI_X_CMD_MAX_READ) >> 2);

	return ret;
}
EXPORT_SYMBOL(pcix_get_mmrbc);


int pcix_set_mmrbc(struct pci_dev *dev, int mmrbc)
{
	int cap, err = -EINVAL;
	u32 stat, cmd, v, o;

	if (mmrbc < 512 || mmrbc > 4096 || !is_power_of_2(mmrbc))
		goto out;

	v = ffs(mmrbc) - 10;

	cap = pci_find_capability(dev, PCI_CAP_ID_PCIX);
	if (!cap)
		goto out;

	err = pci_read_config_dword(dev, cap + PCI_X_STATUS, &stat);
	if (err)
		goto out;

	if (v > (stat & PCI_X_STATUS_MAX_READ) >> 21)
		return -E2BIG;

	err = pci_read_config_dword(dev, cap + PCI_X_CMD, &cmd);
	if (err)
		goto out;

	o = (cmd & PCI_X_CMD_MAX_READ) >> 2;
	if (o != v) {
		if (v > o && dev->bus &&
		   (dev->bus->bus_flags & PCI_BUS_FLAGS_NO_MMRBC))
			return -EIO;

		cmd &= ~PCI_X_CMD_MAX_READ;
		cmd |= v << 2;
		err = pci_write_config_dword(dev, cap + PCI_X_CMD, cmd);
	}
out:
	return err;
}
EXPORT_SYMBOL(pcix_set_mmrbc);


int pcie_get_readrq(struct pci_dev *dev)
{
	int ret, cap;
	u16 ctl;

	cap = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!cap)
		return -EINVAL;

	ret = pci_read_config_word(dev, cap + PCI_EXP_DEVCTL, &ctl);
	if (!ret)
	ret = 128 << ((ctl & PCI_EXP_DEVCTL_READRQ) >> 12);

	return ret;
}
EXPORT_SYMBOL(pcie_get_readrq);


int pcie_set_readrq(struct pci_dev *dev, int rq)
{
	int cap, err = -EINVAL;
	u16 ctl, v;

	if (rq < 128 || rq > 4096 || !is_power_of_2(rq))
		goto out;

	v = (ffs(rq) - 8) << 12;

	cap = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!cap)
		goto out;

	err = pci_read_config_word(dev, cap + PCI_EXP_DEVCTL, &ctl);
	if (err)
		goto out;

	if ((ctl & PCI_EXP_DEVCTL_READRQ) != v) {
		ctl &= ~PCI_EXP_DEVCTL_READRQ;
		ctl |= v;
		err = pci_write_config_dword(dev, cap + PCI_EXP_DEVCTL, ctl);
	}

out:
	return err;
}
EXPORT_SYMBOL(pcie_set_readrq);


int pci_select_bars(struct pci_dev *dev, unsigned long flags)
{
	int i, bars = 0;
	for (i = 0; i < PCI_NUM_RESOURCES; i++)
		if (pci_resource_flags(dev, i) & flags)
			bars |= (1 << i);
	return bars;
}


int pci_resource_bar(struct pci_dev *dev, int resno, enum pci_bar_type *type)
{
	int reg;

	if (resno < PCI_ROM_RESOURCE) {
		*type = pci_bar_unknown;
		return PCI_BASE_ADDRESS_0 + 4 * resno;
	} else if (resno == PCI_ROM_RESOURCE) {
		*type = pci_bar_mem32;
		return dev->rom_base_reg;
	} else if (resno < PCI_BRIDGE_RESOURCES) {
		
		reg = pci_iov_resource_bar(dev, resno, type);
		if (reg)
			return reg;
	}

	dev_err(&dev->dev, "BAR: invalid resource #%d\n", resno);
	return 0;
}


int pci_set_vga_state(struct pci_dev *dev, bool decode,
		      unsigned int command_bits, bool change_bridge)
{
	struct pci_bus *bus;
	struct pci_dev *bridge;
	u16 cmd;

	WARN_ON(command_bits & ~(PCI_COMMAND_IO|PCI_COMMAND_MEMORY));

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	if (decode == true)
		cmd |= command_bits;
	else
		cmd &= ~command_bits;
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	if (change_bridge == false)
		return 0;

	bus = dev->bus;
	while (bus) {
		bridge = bus->self;
		if (bridge) {
			pci_read_config_word(bridge, PCI_BRIDGE_CONTROL,
					     &cmd);
			if (decode == true)
				cmd |= PCI_BRIDGE_CTL_VGA;
			else
				cmd &= ~PCI_BRIDGE_CTL_VGA;
			pci_write_config_word(bridge, PCI_BRIDGE_CONTROL,
					      cmd);
		}
		bus = bus->parent;
	}
	return 0;
}

#define RESOURCE_ALIGNMENT_PARAM_SIZE COMMAND_LINE_SIZE
static char resource_alignment_param[RESOURCE_ALIGNMENT_PARAM_SIZE] = {0};
spinlock_t resource_alignment_lock = SPIN_LOCK_UNLOCKED;


resource_size_t pci_specified_resource_alignment(struct pci_dev *dev)
{
	int seg, bus, slot, func, align_order, count;
	resource_size_t align = 0;
	char *p;

	spin_lock(&resource_alignment_lock);
	p = resource_alignment_param;
	while (*p) {
		count = 0;
		if (sscanf(p, "%d%n", &align_order, &count) == 1 &&
							p[count] == '@') {
			p += count + 1;
		} else {
			align_order = -1;
		}
		if (sscanf(p, "%x:%x:%x.%x%n",
			&seg, &bus, &slot, &func, &count) != 4) {
			seg = 0;
			if (sscanf(p, "%x:%x.%x%n",
					&bus, &slot, &func, &count) != 3) {
				
				printk(KERN_ERR "PCI: Can't parse resource_alignment parameter: %s\n",
					p);
				break;
			}
		}
		p += count;
		if (seg == pci_domain_nr(dev->bus) &&
			bus == dev->bus->number &&
			slot == PCI_SLOT(dev->devfn) &&
			func == PCI_FUNC(dev->devfn)) {
			if (align_order == -1) {
				align = PAGE_SIZE;
			} else {
				align = 1 << align_order;
			}
			
			break;
		}
		if (*p != ';' && *p != ',') {
			
			break;
		}
		p++;
	}
	spin_unlock(&resource_alignment_lock);
	return align;
}


int pci_is_reassigndev(struct pci_dev *dev)
{
	return (pci_specified_resource_alignment(dev) != 0);
}

ssize_t pci_set_resource_alignment_param(const char *buf, size_t count)
{
	if (count > RESOURCE_ALIGNMENT_PARAM_SIZE - 1)
		count = RESOURCE_ALIGNMENT_PARAM_SIZE - 1;
	spin_lock(&resource_alignment_lock);
	strncpy(resource_alignment_param, buf, count);
	resource_alignment_param[count] = '\0';
	spin_unlock(&resource_alignment_lock);
	return count;
}

ssize_t pci_get_resource_alignment_param(char *buf, size_t size)
{
	size_t count;
	spin_lock(&resource_alignment_lock);
	count = snprintf(buf, size, "%s", resource_alignment_param);
	spin_unlock(&resource_alignment_lock);
	return count;
}

static ssize_t pci_resource_alignment_show(struct bus_type *bus, char *buf)
{
	return pci_get_resource_alignment_param(buf, PAGE_SIZE);
}

static ssize_t pci_resource_alignment_store(struct bus_type *bus,
					const char *buf, size_t count)
{
	return pci_set_resource_alignment_param(buf, count);
}

BUS_ATTR(resource_alignment, 0644, pci_resource_alignment_show,
					pci_resource_alignment_store);

static int __init pci_resource_alignment_sysfs_init(void)
{
	return bus_create_file(&pci_bus_type,
					&bus_attr_resource_alignment);
}

late_initcall(pci_resource_alignment_sysfs_init);

static void __devinit pci_no_domains(void)
{
#ifdef CONFIG_PCI_DOMAINS
	pci_domains_supported = 0;
#endif
}


int __attribute__ ((weak)) pci_ext_cfg_avail(struct pci_dev *dev)
{
	return 1;
}

void __weak pci_fixup_cardbus(struct pci_bus *bus)
{
}
EXPORT_SYMBOL(pci_fixup_cardbus);

static int __init pci_setup(char *str)
{
	while (str) {
		char *k = strchr(str, ',');
		if (k)
			*k++ = 0;
		if (*str && (str = pcibios_setup(str)) && *str) {
			if (!strcmp(str, "nomsi")) {
				pci_no_msi();
			} else if (!strcmp(str, "noaer")) {
				pci_no_aer();
			} else if (!strcmp(str, "nodomains")) {
				pci_no_domains();
			} else if (!strncmp(str, "cbiosize=", 9)) {
				pci_cardbus_io_size = memparse(str + 9, &str);
			} else if (!strncmp(str, "cbmemsize=", 10)) {
				pci_cardbus_mem_size = memparse(str + 10, &str);
			} else if (!strncmp(str, "resource_alignment=", 19)) {
				pci_set_resource_alignment_param(str + 19,
							strlen(str + 19));
			} else if (!strncmp(str, "ecrc=", 5)) {
				pcie_ecrc_get_policy(str + 5);
			} else if (!strncmp(str, "hpiosize=", 9)) {
				pci_hotplug_io_size = memparse(str + 9, &str);
			} else if (!strncmp(str, "hpmemsize=", 10)) {
				pci_hotplug_mem_size = memparse(str + 10, &str);
			} else {
				printk(KERN_ERR "PCI: Unknown option `%s'\n",
						str);
			}
		}
		str = k;
	}
	return 0;
}
early_param("pci", pci_setup);

EXPORT_SYMBOL(pci_reenable_device);
EXPORT_SYMBOL(pci_enable_device_io);
EXPORT_SYMBOL(pci_enable_device_mem);
EXPORT_SYMBOL(pci_enable_device);
EXPORT_SYMBOL(pcim_enable_device);
EXPORT_SYMBOL(pcim_pin_device);
EXPORT_SYMBOL(pci_disable_device);
EXPORT_SYMBOL(pci_find_capability);
EXPORT_SYMBOL(pci_bus_find_capability);
EXPORT_SYMBOL(pci_release_regions);
EXPORT_SYMBOL(pci_request_regions);
EXPORT_SYMBOL(pci_request_regions_exclusive);
EXPORT_SYMBOL(pci_release_region);
EXPORT_SYMBOL(pci_request_region);
EXPORT_SYMBOL(pci_request_region_exclusive);
EXPORT_SYMBOL(pci_release_selected_regions);
EXPORT_SYMBOL(pci_request_selected_regions);
EXPORT_SYMBOL(pci_request_selected_regions_exclusive);
EXPORT_SYMBOL(pci_set_master);
EXPORT_SYMBOL(pci_clear_master);
EXPORT_SYMBOL(pci_set_mwi);
EXPORT_SYMBOL(pci_try_set_mwi);
EXPORT_SYMBOL(pci_clear_mwi);
EXPORT_SYMBOL_GPL(pci_intx);
EXPORT_SYMBOL(pci_set_dma_mask);
EXPORT_SYMBOL(pci_set_consistent_dma_mask);
EXPORT_SYMBOL(pci_assign_resource);
EXPORT_SYMBOL(pci_find_parent_resource);
EXPORT_SYMBOL(pci_select_bars);

EXPORT_SYMBOL(pci_set_power_state);
EXPORT_SYMBOL(pci_save_state);
EXPORT_SYMBOL(pci_restore_state);
EXPORT_SYMBOL(pci_pme_capable);
EXPORT_SYMBOL(pci_pme_active);
EXPORT_SYMBOL(pci_enable_wake);
EXPORT_SYMBOL(pci_wake_from_d3);
EXPORT_SYMBOL(pci_target_state);
EXPORT_SYMBOL(pci_prepare_to_sleep);
EXPORT_SYMBOL(pci_back_from_sleep);
EXPORT_SYMBOL_GPL(pci_set_pcie_reset_state);

