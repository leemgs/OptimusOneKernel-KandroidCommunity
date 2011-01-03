

#include <linux/pci.h>
#include <linux/acpi.h>
#include <linux/gfp.h>
#include <linux/list.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/msi.h>
#include <asm/pci-direct.h>
#include <asm/amd_iommu_types.h>
#include <asm/amd_iommu.h>
#include <asm/iommu.h>
#include <asm/gart.h>


#define IVRS_HEADER_LENGTH 48

#define ACPI_IVHD_TYPE                  0x10
#define ACPI_IVMD_TYPE_ALL              0x20
#define ACPI_IVMD_TYPE                  0x21
#define ACPI_IVMD_TYPE_RANGE            0x22

#define IVHD_DEV_ALL                    0x01
#define IVHD_DEV_SELECT                 0x02
#define IVHD_DEV_SELECT_RANGE_START     0x03
#define IVHD_DEV_RANGE_END              0x04
#define IVHD_DEV_ALIAS                  0x42
#define IVHD_DEV_ALIAS_RANGE            0x43
#define IVHD_DEV_EXT_SELECT             0x46
#define IVHD_DEV_EXT_SELECT_RANGE       0x47

#define IVHD_FLAG_HT_TUN_EN_MASK        0x01
#define IVHD_FLAG_PASSPW_EN_MASK        0x02
#define IVHD_FLAG_RESPASSPW_EN_MASK     0x04
#define IVHD_FLAG_ISOC_EN_MASK          0x08

#define IVMD_FLAG_EXCL_RANGE            0x08
#define IVMD_FLAG_UNITY_MAP             0x01

#define ACPI_DEVFLAG_INITPASS           0x01
#define ACPI_DEVFLAG_EXTINT             0x02
#define ACPI_DEVFLAG_NMI                0x04
#define ACPI_DEVFLAG_SYSMGT1            0x10
#define ACPI_DEVFLAG_SYSMGT2            0x20
#define ACPI_DEVFLAG_LINT0              0x40
#define ACPI_DEVFLAG_LINT1              0x80
#define ACPI_DEVFLAG_ATSDIS             0x10000000




struct ivhd_header {
	u8 type;
	u8 flags;
	u16 length;
	u16 devid;
	u16 cap_ptr;
	u64 mmio_phys;
	u16 pci_seg;
	u16 info;
	u32 reserved;
} __attribute__((packed));


struct ivhd_entry {
	u8 type;
	u16 devid;
	u8 flags;
	u32 ext;
} __attribute__((packed));


struct ivmd_header {
	u8 type;
	u8 flags;
	u16 length;
	u16 devid;
	u16 aux;
	u64 resv;
	u64 range_start;
	u64 range_length;
} __attribute__((packed));

bool amd_iommu_dump;

static int __initdata amd_iommu_detected;

u16 amd_iommu_last_bdf;			
LIST_HEAD(amd_iommu_unity_map);		
#ifdef CONFIG_IOMMU_STRESS
bool amd_iommu_isolate = false;
#else
bool amd_iommu_isolate = true;		
#endif

bool amd_iommu_unmap_flush;		

LIST_HEAD(amd_iommu_list);		


static bool amd_iommu_initialized;


struct dev_table_entry *amd_iommu_dev_table;


u16 *amd_iommu_alias_table;


struct amd_iommu **amd_iommu_rlookup_table;


struct protection_domain **amd_iommu_pd_table;


unsigned long *amd_iommu_pd_alloc_bitmap;

static u32 dev_table_size;	
static u32 alias_table_size;	
static u32 rlookup_table_size;	

static inline void update_last_devid(u16 devid)
{
	if (devid > amd_iommu_last_bdf)
		amd_iommu_last_bdf = devid;
}

static inline unsigned long tbl_size(int entry_size)
{
	unsigned shift = PAGE_SHIFT +
			 get_order(((int)amd_iommu_last_bdf + 1) * entry_size);

	return 1UL << shift;
}




static void iommu_set_exclusion_range(struct amd_iommu *iommu)
{
	u64 start = iommu->exclusion_start & PAGE_MASK;
	u64 limit = (start + iommu->exclusion_length) & PAGE_MASK;
	u64 entry;

	if (!iommu->exclusion_start)
		return;

	entry = start | MMIO_EXCL_ENABLE_MASK;
	memcpy_toio(iommu->mmio_base + MMIO_EXCL_BASE_OFFSET,
			&entry, sizeof(entry));

	entry = limit;
	memcpy_toio(iommu->mmio_base + MMIO_EXCL_LIMIT_OFFSET,
			&entry, sizeof(entry));
}


static void __init iommu_set_device_table(struct amd_iommu *iommu)
{
	u64 entry;

	BUG_ON(iommu->mmio_base == NULL);

	entry = virt_to_phys(amd_iommu_dev_table);
	entry |= (dev_table_size >> 12) - 1;
	memcpy_toio(iommu->mmio_base + MMIO_DEV_TABLE_OFFSET,
			&entry, sizeof(entry));
}


static void iommu_feature_enable(struct amd_iommu *iommu, u8 bit)
{
	u32 ctrl;

	ctrl = readl(iommu->mmio_base + MMIO_CONTROL_OFFSET);
	ctrl |= (1 << bit);
	writel(ctrl, iommu->mmio_base + MMIO_CONTROL_OFFSET);
}

static void iommu_feature_disable(struct amd_iommu *iommu, u8 bit)
{
	u32 ctrl;

	ctrl = readl(iommu->mmio_base + MMIO_CONTROL_OFFSET);
	ctrl &= ~(1 << bit);
	writel(ctrl, iommu->mmio_base + MMIO_CONTROL_OFFSET);
}


static void iommu_enable(struct amd_iommu *iommu)
{
	printk(KERN_INFO "AMD-Vi: Enabling IOMMU at %s cap 0x%hx\n",
	       dev_name(&iommu->dev->dev), iommu->cap_ptr);

	iommu_feature_enable(iommu, CONTROL_IOMMU_EN);
}

static void iommu_disable(struct amd_iommu *iommu)
{
	
	iommu_feature_disable(iommu, CONTROL_CMDBUF_EN);

	
	iommu_feature_disable(iommu, CONTROL_EVT_INT_EN);
	iommu_feature_disable(iommu, CONTROL_EVT_LOG_EN);

	
	iommu_feature_disable(iommu, CONTROL_IOMMU_EN);
}


static u8 * __init iommu_map_mmio_space(u64 address)
{
	u8 *ret;

	if (!request_mem_region(address, MMIO_REGION_LENGTH, "amd_iommu"))
		return NULL;

	ret = ioremap_nocache(address, MMIO_REGION_LENGTH);
	if (ret != NULL)
		return ret;

	release_mem_region(address, MMIO_REGION_LENGTH);

	return NULL;
}

static void __init iommu_unmap_mmio_space(struct amd_iommu *iommu)
{
	if (iommu->mmio_base)
		iounmap(iommu->mmio_base);
	release_mem_region(iommu->mmio_phys, MMIO_REGION_LENGTH);
}




static inline int ivhd_entry_length(u8 *ivhd)
{
	return 0x04 << (*ivhd >> 6);
}


static int __init find_last_devid_on_pci(int bus, int dev, int fn, int cap_ptr)
{
	u32 cap;

	cap = read_pci_config(bus, dev, fn, cap_ptr+MMIO_RANGE_OFFSET);
	update_last_devid(calc_devid(MMIO_GET_BUS(cap), MMIO_GET_LD(cap)));

	return 0;
}


static int __init find_last_devid_from_ivhd(struct ivhd_header *h)
{
	u8 *p = (void *)h, *end = (void *)h;
	struct ivhd_entry *dev;

	p += sizeof(*h);
	end += h->length;

	find_last_devid_on_pci(PCI_BUS(h->devid),
			PCI_SLOT(h->devid),
			PCI_FUNC(h->devid),
			h->cap_ptr);

	while (p < end) {
		dev = (struct ivhd_entry *)p;
		switch (dev->type) {
		case IVHD_DEV_SELECT:
		case IVHD_DEV_RANGE_END:
		case IVHD_DEV_ALIAS:
		case IVHD_DEV_EXT_SELECT:
			
			update_last_devid(dev->devid);
			break;
		default:
			break;
		}
		p += ivhd_entry_length(p);
	}

	WARN_ON(p != end);

	return 0;
}


static int __init find_last_devid_acpi(struct acpi_table_header *table)
{
	int i;
	u8 checksum = 0, *p = (u8 *)table, *end = (u8 *)table;
	struct ivhd_header *h;

	
	for (i = 0; i < table->length; ++i)
		checksum += p[i];
	if (checksum != 0)
		
		return -ENODEV;

	p += IVRS_HEADER_LENGTH;

	end += table->length;
	while (p < end) {
		h = (struct ivhd_header *)p;
		switch (h->type) {
		case ACPI_IVHD_TYPE:
			find_last_devid_from_ivhd(h);
			break;
		default:
			break;
		}
		p += h->length;
	}
	WARN_ON(p != end);

	return 0;
}




static u8 * __init alloc_command_buffer(struct amd_iommu *iommu)
{
	u8 *cmd_buf = (u8 *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
			get_order(CMD_BUFFER_SIZE));

	if (cmd_buf == NULL)
		return NULL;

	iommu->cmd_buf_size = CMD_BUFFER_SIZE;

	return cmd_buf;
}


void amd_iommu_reset_cmd_buffer(struct amd_iommu *iommu)
{
	iommu_feature_disable(iommu, CONTROL_CMDBUF_EN);

	writel(0x00, iommu->mmio_base + MMIO_CMD_HEAD_OFFSET);
	writel(0x00, iommu->mmio_base + MMIO_CMD_TAIL_OFFSET);

	iommu_feature_enable(iommu, CONTROL_CMDBUF_EN);
}


static void iommu_enable_command_buffer(struct amd_iommu *iommu)
{
	u64 entry;

	BUG_ON(iommu->cmd_buf == NULL);

	entry = (u64)virt_to_phys(iommu->cmd_buf);
	entry |= MMIO_CMD_SIZE_512;

	memcpy_toio(iommu->mmio_base + MMIO_CMD_BUF_OFFSET,
		    &entry, sizeof(entry));

	amd_iommu_reset_cmd_buffer(iommu);
}

static void __init free_command_buffer(struct amd_iommu *iommu)
{
	free_pages((unsigned long)iommu->cmd_buf,
		   get_order(iommu->cmd_buf_size));
}


static u8 * __init alloc_event_buffer(struct amd_iommu *iommu)
{
	iommu->evt_buf = (u8 *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
						get_order(EVT_BUFFER_SIZE));

	if (iommu->evt_buf == NULL)
		return NULL;

	iommu->evt_buf_size = EVT_BUFFER_SIZE;

	return iommu->evt_buf;
}

static void iommu_enable_event_buffer(struct amd_iommu *iommu)
{
	u64 entry;

	BUG_ON(iommu->evt_buf == NULL);

	entry = (u64)virt_to_phys(iommu->evt_buf) | EVT_LEN_MASK;

	memcpy_toio(iommu->mmio_base + MMIO_EVT_BUF_OFFSET,
		    &entry, sizeof(entry));

	
	writel(0x00, iommu->mmio_base + MMIO_EVT_HEAD_OFFSET);
	writel(0x00, iommu->mmio_base + MMIO_EVT_TAIL_OFFSET);

	iommu_feature_enable(iommu, CONTROL_EVT_LOG_EN);
}

static void __init free_event_buffer(struct amd_iommu *iommu)
{
	free_pages((unsigned long)iommu->evt_buf, get_order(EVT_BUFFER_SIZE));
}


static void set_dev_entry_bit(u16 devid, u8 bit)
{
	int i = (bit >> 5) & 0x07;
	int _bit = bit & 0x1f;

	amd_iommu_dev_table[devid].data[i] |= (1 << _bit);
}

static int get_dev_entry_bit(u16 devid, u8 bit)
{
	int i = (bit >> 5) & 0x07;
	int _bit = bit & 0x1f;

	return (amd_iommu_dev_table[devid].data[i] & (1 << _bit)) >> _bit;
}


void amd_iommu_apply_erratum_63(u16 devid)
{
	int sysmgt;

	sysmgt = get_dev_entry_bit(devid, DEV_ENTRY_SYSMGT1) |
		 (get_dev_entry_bit(devid, DEV_ENTRY_SYSMGT2) << 1);

	if (sysmgt == 0x01)
		set_dev_entry_bit(devid, DEV_ENTRY_IW);
}


static void __init set_iommu_for_device(struct amd_iommu *iommu, u16 devid)
{
	amd_iommu_rlookup_table[devid] = iommu;
}


static void __init set_dev_entry_from_acpi(struct amd_iommu *iommu,
					   u16 devid, u32 flags, u32 ext_flags)
{
	if (flags & ACPI_DEVFLAG_INITPASS)
		set_dev_entry_bit(devid, DEV_ENTRY_INIT_PASS);
	if (flags & ACPI_DEVFLAG_EXTINT)
		set_dev_entry_bit(devid, DEV_ENTRY_EINT_PASS);
	if (flags & ACPI_DEVFLAG_NMI)
		set_dev_entry_bit(devid, DEV_ENTRY_NMI_PASS);
	if (flags & ACPI_DEVFLAG_SYSMGT1)
		set_dev_entry_bit(devid, DEV_ENTRY_SYSMGT1);
	if (flags & ACPI_DEVFLAG_SYSMGT2)
		set_dev_entry_bit(devid, DEV_ENTRY_SYSMGT2);
	if (flags & ACPI_DEVFLAG_LINT0)
		set_dev_entry_bit(devid, DEV_ENTRY_LINT0_PASS);
	if (flags & ACPI_DEVFLAG_LINT1)
		set_dev_entry_bit(devid, DEV_ENTRY_LINT1_PASS);

	amd_iommu_apply_erratum_63(devid);

	set_iommu_for_device(iommu, devid);
}


static void __init set_device_exclusion_range(u16 devid, struct ivmd_header *m)
{
	struct amd_iommu *iommu = amd_iommu_rlookup_table[devid];

	if (!(m->flags & IVMD_FLAG_EXCL_RANGE))
		return;

	if (iommu) {
		
		set_dev_entry_bit(m->devid, DEV_ENTRY_EX);
		iommu->exclusion_start = m->range_start;
		iommu->exclusion_length = m->range_length;
	}
}


static void __init init_iommu_from_pci(struct amd_iommu *iommu)
{
	int cap_ptr = iommu->cap_ptr;
	u32 range, misc;

	pci_read_config_dword(iommu->dev, cap_ptr + MMIO_CAP_HDR_OFFSET,
			      &iommu->cap);
	pci_read_config_dword(iommu->dev, cap_ptr + MMIO_RANGE_OFFSET,
			      &range);
	pci_read_config_dword(iommu->dev, cap_ptr + MMIO_MISC_OFFSET,
			      &misc);

	iommu->first_device = calc_devid(MMIO_GET_BUS(range),
					 MMIO_GET_FD(range));
	iommu->last_device = calc_devid(MMIO_GET_BUS(range),
					MMIO_GET_LD(range));
	iommu->evt_msi_num = MMIO_MSI_NUM(misc);
}


static void __init init_iommu_from_acpi(struct amd_iommu *iommu,
					struct ivhd_header *h)
{
	u8 *p = (u8 *)h;
	u8 *end = p, flags = 0;
	u16 dev_i, devid = 0, devid_start = 0, devid_to = 0;
	u32 ext_flags = 0;
	bool alias = false;
	struct ivhd_entry *e;

	
	h->flags & IVHD_FLAG_HT_TUN_EN_MASK ?
		iommu_feature_enable(iommu, CONTROL_HT_TUN_EN) :
		iommu_feature_disable(iommu, CONTROL_HT_TUN_EN);

	h->flags & IVHD_FLAG_PASSPW_EN_MASK ?
		iommu_feature_enable(iommu, CONTROL_PASSPW_EN) :
		iommu_feature_disable(iommu, CONTROL_PASSPW_EN);

	h->flags & IVHD_FLAG_RESPASSPW_EN_MASK ?
		iommu_feature_enable(iommu, CONTROL_RESPASSPW_EN) :
		iommu_feature_disable(iommu, CONTROL_RESPASSPW_EN);

	h->flags & IVHD_FLAG_ISOC_EN_MASK ?
		iommu_feature_enable(iommu, CONTROL_ISOC_EN) :
		iommu_feature_disable(iommu, CONTROL_ISOC_EN);

	
	iommu_feature_enable(iommu, CONTROL_COHERENT_EN);

	
	p += sizeof(struct ivhd_header);
	end += h->length;


	while (p < end) {
		e = (struct ivhd_entry *)p;
		switch (e->type) {
		case IVHD_DEV_ALL:

			DUMP_printk("  DEV_ALL\t\t\t first devid: %02x:%02x.%x"
				    " last device %02x:%02x.%x flags: %02x\n",
				    PCI_BUS(iommu->first_device),
				    PCI_SLOT(iommu->first_device),
				    PCI_FUNC(iommu->first_device),
				    PCI_BUS(iommu->last_device),
				    PCI_SLOT(iommu->last_device),
				    PCI_FUNC(iommu->last_device),
				    e->flags);

			for (dev_i = iommu->first_device;
					dev_i <= iommu->last_device; ++dev_i)
				set_dev_entry_from_acpi(iommu, dev_i,
							e->flags, 0);
			break;
		case IVHD_DEV_SELECT:

			DUMP_printk("  DEV_SELECT\t\t\t devid: %02x:%02x.%x "
				    "flags: %02x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags);

			devid = e->devid;
			set_dev_entry_from_acpi(iommu, devid, e->flags, 0);
			break;
		case IVHD_DEV_SELECT_RANGE_START:

			DUMP_printk("  DEV_SELECT_RANGE_START\t "
				    "devid: %02x:%02x.%x flags: %02x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags);

			devid_start = e->devid;
			flags = e->flags;
			ext_flags = 0;
			alias = false;
			break;
		case IVHD_DEV_ALIAS:

			DUMP_printk("  DEV_ALIAS\t\t\t devid: %02x:%02x.%x "
				    "flags: %02x devid_to: %02x:%02x.%x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags,
				    PCI_BUS(e->ext >> 8),
				    PCI_SLOT(e->ext >> 8),
				    PCI_FUNC(e->ext >> 8));

			devid = e->devid;
			devid_to = e->ext >> 8;
			set_dev_entry_from_acpi(iommu, devid   , e->flags, 0);
			set_dev_entry_from_acpi(iommu, devid_to, e->flags, 0);
			amd_iommu_alias_table[devid] = devid_to;
			break;
		case IVHD_DEV_ALIAS_RANGE:

			DUMP_printk("  DEV_ALIAS_RANGE\t\t "
				    "devid: %02x:%02x.%x flags: %02x "
				    "devid_to: %02x:%02x.%x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags,
				    PCI_BUS(e->ext >> 8),
				    PCI_SLOT(e->ext >> 8),
				    PCI_FUNC(e->ext >> 8));

			devid_start = e->devid;
			flags = e->flags;
			devid_to = e->ext >> 8;
			ext_flags = 0;
			alias = true;
			break;
		case IVHD_DEV_EXT_SELECT:

			DUMP_printk("  DEV_EXT_SELECT\t\t devid: %02x:%02x.%x "
				    "flags: %02x ext: %08x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags, e->ext);

			devid = e->devid;
			set_dev_entry_from_acpi(iommu, devid, e->flags,
						e->ext);
			break;
		case IVHD_DEV_EXT_SELECT_RANGE:

			DUMP_printk("  DEV_EXT_SELECT_RANGE\t devid: "
				    "%02x:%02x.%x flags: %02x ext: %08x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid),
				    e->flags, e->ext);

			devid_start = e->devid;
			flags = e->flags;
			ext_flags = e->ext;
			alias = false;
			break;
		case IVHD_DEV_RANGE_END:

			DUMP_printk("  DEV_RANGE_END\t\t devid: %02x:%02x.%x\n",
				    PCI_BUS(e->devid),
				    PCI_SLOT(e->devid),
				    PCI_FUNC(e->devid));

			devid = e->devid;
			for (dev_i = devid_start; dev_i <= devid; ++dev_i) {
				if (alias) {
					amd_iommu_alias_table[dev_i] = devid_to;
					set_dev_entry_from_acpi(iommu,
						devid_to, flags, ext_flags);
				}
				set_dev_entry_from_acpi(iommu, dev_i,
							flags, ext_flags);
			}
			break;
		default:
			break;
		}

		p += ivhd_entry_length(p);
	}
}


static int __init init_iommu_devices(struct amd_iommu *iommu)
{
	u16 i;

	for (i = iommu->first_device; i <= iommu->last_device; ++i)
		set_iommu_for_device(iommu, i);

	return 0;
}

static void __init free_iommu_one(struct amd_iommu *iommu)
{
	free_command_buffer(iommu);
	free_event_buffer(iommu);
	iommu_unmap_mmio_space(iommu);
}

static void __init free_iommu_all(void)
{
	struct amd_iommu *iommu, *next;

	for_each_iommu_safe(iommu, next) {
		list_del(&iommu->list);
		free_iommu_one(iommu);
		kfree(iommu);
	}
}


static int __init init_iommu_one(struct amd_iommu *iommu, struct ivhd_header *h)
{
	spin_lock_init(&iommu->lock);
	list_add_tail(&iommu->list, &amd_iommu_list);

	
	iommu->dev = pci_get_bus_and_slot(PCI_BUS(h->devid), h->devid & 0xff);
	if (!iommu->dev)
		return 1;

	iommu->cap_ptr = h->cap_ptr;
	iommu->pci_seg = h->pci_seg;
	iommu->mmio_phys = h->mmio_phys;
	iommu->mmio_base = iommu_map_mmio_space(h->mmio_phys);
	if (!iommu->mmio_base)
		return -ENOMEM;

	iommu->cmd_buf = alloc_command_buffer(iommu);
	if (!iommu->cmd_buf)
		return -ENOMEM;

	iommu->evt_buf = alloc_event_buffer(iommu);
	if (!iommu->evt_buf)
		return -ENOMEM;

	iommu->int_enabled = false;

	init_iommu_from_pci(iommu);
	init_iommu_from_acpi(iommu, h);
	init_iommu_devices(iommu);

	return pci_enable_device(iommu->dev);
}


static int __init init_iommu_all(struct acpi_table_header *table)
{
	u8 *p = (u8 *)table, *end = (u8 *)table;
	struct ivhd_header *h;
	struct amd_iommu *iommu;
	int ret;

	end += table->length;
	p += IVRS_HEADER_LENGTH;

	while (p < end) {
		h = (struct ivhd_header *)p;
		switch (*p) {
		case ACPI_IVHD_TYPE:

			DUMP_printk("device: %02x:%02x.%01x cap: %04x "
				    "seg: %d flags: %01x info %04x\n",
				    PCI_BUS(h->devid), PCI_SLOT(h->devid),
				    PCI_FUNC(h->devid), h->cap_ptr,
				    h->pci_seg, h->flags, h->info);
			DUMP_printk("       mmio-addr: %016llx\n",
				    h->mmio_phys);

			iommu = kzalloc(sizeof(struct amd_iommu), GFP_KERNEL);
			if (iommu == NULL)
				return -ENOMEM;
			ret = init_iommu_one(iommu, h);
			if (ret)
				return ret;
			break;
		default:
			break;
		}
		p += h->length;

	}
	WARN_ON(p != end);

	amd_iommu_initialized = true;

	return 0;
}



static int iommu_setup_msi(struct amd_iommu *iommu)
{
	int r;

	if (pci_enable_msi(iommu->dev))
		return 1;

	r = request_irq(iommu->dev->irq, amd_iommu_int_handler,
			IRQF_SAMPLE_RANDOM,
			"AMD-Vi",
			NULL);

	if (r) {
		pci_disable_msi(iommu->dev);
		return 1;
	}

	iommu->int_enabled = true;
	iommu_feature_enable(iommu, CONTROL_EVT_INT_EN);

	return 0;
}

static int iommu_init_msi(struct amd_iommu *iommu)
{
	if (iommu->int_enabled)
		return 0;

	if (pci_find_capability(iommu->dev, PCI_CAP_ID_MSI))
		return iommu_setup_msi(iommu);

	return 1;
}



static void __init free_unity_maps(void)
{
	struct unity_map_entry *entry, *next;

	list_for_each_entry_safe(entry, next, &amd_iommu_unity_map, list) {
		list_del(&entry->list);
		kfree(entry);
	}
}


static int __init init_exclusion_range(struct ivmd_header *m)
{
	int i;

	switch (m->type) {
	case ACPI_IVMD_TYPE:
		set_device_exclusion_range(m->devid, m);
		break;
	case ACPI_IVMD_TYPE_ALL:
		for (i = 0; i <= amd_iommu_last_bdf; ++i)
			set_device_exclusion_range(i, m);
		break;
	case ACPI_IVMD_TYPE_RANGE:
		for (i = m->devid; i <= m->aux; ++i)
			set_device_exclusion_range(i, m);
		break;
	default:
		break;
	}

	return 0;
}


static int __init init_unity_map_range(struct ivmd_header *m)
{
	struct unity_map_entry *e = 0;
	char *s;

	e = kzalloc(sizeof(*e), GFP_KERNEL);
	if (e == NULL)
		return -ENOMEM;

	switch (m->type) {
	default:
		kfree(e);
		return 0;
	case ACPI_IVMD_TYPE:
		s = "IVMD_TYPEi\t\t\t";
		e->devid_start = e->devid_end = m->devid;
		break;
	case ACPI_IVMD_TYPE_ALL:
		s = "IVMD_TYPE_ALL\t\t";
		e->devid_start = 0;
		e->devid_end = amd_iommu_last_bdf;
		break;
	case ACPI_IVMD_TYPE_RANGE:
		s = "IVMD_TYPE_RANGE\t\t";
		e->devid_start = m->devid;
		e->devid_end = m->aux;
		break;
	}
	e->address_start = PAGE_ALIGN(m->range_start);
	e->address_end = e->address_start + PAGE_ALIGN(m->range_length);
	e->prot = m->flags >> 1;

	DUMP_printk("%s devid_start: %02x:%02x.%x devid_end: %02x:%02x.%x"
		    " range_start: %016llx range_end: %016llx flags: %x\n", s,
		    PCI_BUS(e->devid_start), PCI_SLOT(e->devid_start),
		    PCI_FUNC(e->devid_start), PCI_BUS(e->devid_end),
		    PCI_SLOT(e->devid_end), PCI_FUNC(e->devid_end),
		    e->address_start, e->address_end, m->flags);

	list_add_tail(&e->list, &amd_iommu_unity_map);

	return 0;
}


static int __init init_memory_definitions(struct acpi_table_header *table)
{
	u8 *p = (u8 *)table, *end = (u8 *)table;
	struct ivmd_header *m;

	end += table->length;
	p += IVRS_HEADER_LENGTH;

	while (p < end) {
		m = (struct ivmd_header *)p;
		if (m->flags & IVMD_FLAG_EXCL_RANGE)
			init_exclusion_range(m);
		else if (m->flags & IVMD_FLAG_UNITY_MAP)
			init_unity_map_range(m);

		p += m->length;
	}

	return 0;
}


static void init_device_table(void)
{
	u16 devid;

	for (devid = 0; devid <= amd_iommu_last_bdf; ++devid) {
		set_dev_entry_bit(devid, DEV_ENTRY_VALID);
		set_dev_entry_bit(devid, DEV_ENTRY_TRANSLATION);
	}
}


static void enable_iommus(void)
{
	struct amd_iommu *iommu;

	for_each_iommu(iommu) {
		iommu_disable(iommu);
		iommu_set_device_table(iommu);
		iommu_enable_command_buffer(iommu);
		iommu_enable_event_buffer(iommu);
		iommu_set_exclusion_range(iommu);
		iommu_init_msi(iommu);
		iommu_enable(iommu);
	}
}

static void disable_iommus(void)
{
	struct amd_iommu *iommu;

	for_each_iommu(iommu)
		iommu_disable(iommu);
}



static int amd_iommu_resume(struct sys_device *dev)
{
	
	enable_iommus();

	
	amd_iommu_flush_all_devices();
	amd_iommu_flush_all_domains();

	return 0;
}

static int amd_iommu_suspend(struct sys_device *dev, pm_message_t state)
{
	
	disable_iommus();

	return 0;
}

static struct sysdev_class amd_iommu_sysdev_class = {
	.name = "amd_iommu",
	.suspend = amd_iommu_suspend,
	.resume = amd_iommu_resume,
};

static struct sys_device device_amd_iommu = {
	.id = 0,
	.cls = &amd_iommu_sysdev_class,
};


int __init amd_iommu_init(void)
{
	int i, ret = 0;


	if (no_iommu) {
		printk(KERN_INFO "AMD-Vi disabled by kernel command line\n");
		return 0;
	}

	if (!amd_iommu_detected)
		return -ENODEV;

	
	if (acpi_table_parse("IVRS", find_last_devid_acpi) != 0)
		return -ENODEV;

	dev_table_size     = tbl_size(DEV_TABLE_ENTRY_SIZE);
	alias_table_size   = tbl_size(ALIAS_TABLE_ENTRY_SIZE);
	rlookup_table_size = tbl_size(RLOOKUP_TABLE_ENTRY_SIZE);

	ret = -ENOMEM;

	
	amd_iommu_dev_table = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
				      get_order(dev_table_size));
	if (amd_iommu_dev_table == NULL)
		goto out;

	
	amd_iommu_alias_table = (void *)__get_free_pages(GFP_KERNEL,
			get_order(alias_table_size));
	if (amd_iommu_alias_table == NULL)
		goto free;

	
	amd_iommu_rlookup_table = (void *)__get_free_pages(
			GFP_KERNEL | __GFP_ZERO,
			get_order(rlookup_table_size));
	if (amd_iommu_rlookup_table == NULL)
		goto free;

	
	amd_iommu_pd_table = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
				     get_order(rlookup_table_size));
	if (amd_iommu_pd_table == NULL)
		goto free;

	amd_iommu_pd_alloc_bitmap = (void *)__get_free_pages(
					    GFP_KERNEL | __GFP_ZERO,
					    get_order(MAX_DOMAIN_ID/8));
	if (amd_iommu_pd_alloc_bitmap == NULL)
		goto free;

	
	init_device_table();

	
	for (i = 0; i <= amd_iommu_last_bdf; ++i)
		amd_iommu_alias_table[i] = i;

	
	amd_iommu_pd_alloc_bitmap[0] = 1;

	
	ret = -ENODEV;
	if (acpi_table_parse("IVRS", init_iommu_all) != 0)
		goto free;

	if (!amd_iommu_initialized)
		goto free;

	if (acpi_table_parse("IVRS", init_memory_definitions) != 0)
		goto free;

	ret = sysdev_class_register(&amd_iommu_sysdev_class);
	if (ret)
		goto free;

	ret = sysdev_register(&device_amd_iommu);
	if (ret)
		goto free;

	if (iommu_pass_through)
		ret = amd_iommu_init_passthrough();
	else
		ret = amd_iommu_init_dma_ops();

	if (ret)
		goto free;

	amd_iommu_init_api();

	enable_iommus();

	if (iommu_pass_through)
		goto out;

	printk(KERN_INFO "AMD-Vi: device isolation ");
	if (amd_iommu_isolate)
		printk("enabled\n");
	else
		printk("disabled\n");

	if (amd_iommu_unmap_flush)
		printk(KERN_INFO "AMD-Vi: IO/TLB flush on unmap enabled\n");
	else
		printk(KERN_INFO "AMD-Vi: Lazy IO/TLB flushing enabled\n");

out:
	return ret;

free:
	free_pages((unsigned long)amd_iommu_pd_alloc_bitmap,
		   get_order(MAX_DOMAIN_ID/8));

	free_pages((unsigned long)amd_iommu_pd_table,
		   get_order(rlookup_table_size));

	free_pages((unsigned long)amd_iommu_rlookup_table,
		   get_order(rlookup_table_size));

	free_pages((unsigned long)amd_iommu_alias_table,
		   get_order(alias_table_size));

	free_pages((unsigned long)amd_iommu_dev_table,
		   get_order(dev_table_size));

	free_iommu_all();

	free_unity_maps();

	goto out;
}

void amd_iommu_shutdown(void)
{
	disable_iommus();
}


static int __init early_amd_iommu_detect(struct acpi_table_header *table)
{
	return 0;
}

void __init amd_iommu_detect(void)
{
	if (swiotlb || no_iommu || (iommu_detected && !gart_iommu_aperture))
		return;

	if (acpi_table_parse("IVRS", early_amd_iommu_detect) == 0) {
		iommu_detected = 1;
		amd_iommu_detected = 1;
#ifdef CONFIG_GART_IOMMU
		gart_iommu_aperture_disabled = 1;
		gart_iommu_aperture = 0;
#endif
	}
}



static int __init parse_amd_iommu_dump(char *str)
{
	amd_iommu_dump = true;

	return 1;
}

static int __init parse_amd_iommu_options(char *str)
{
	for (; *str; ++str) {
		if (strncmp(str, "isolate", 7) == 0)
			amd_iommu_isolate = true;
		if (strncmp(str, "share", 5) == 0)
			amd_iommu_isolate = false;
		if (strncmp(str, "fullflush", 9) == 0)
			amd_iommu_unmap_flush = true;
	}

	return 1;
}

__setup("amd_iommu_dump", parse_amd_iommu_dump);
__setup("amd_iommu=", parse_amd_iommu_options);
