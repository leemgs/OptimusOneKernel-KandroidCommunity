
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <linux/miscdevice.h>
#include <linux/pm.h>
#include <linux/agp_backend.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include "agp.h"

__u32 *agp_gatt_table;
int agp_memory_reserved;


EXPORT_SYMBOL_GPL(agp_memory_reserved);



void agp_free_key(int key)
{
	if (key < 0)
		return;

	if (key < MAXKEY)
		clear_bit(key, agp_bridge->key_list);
}
EXPORT_SYMBOL(agp_free_key);


static int agp_get_key(void)
{
	int bit;

	bit = find_first_zero_bit(agp_bridge->key_list, MAXKEY);
	if (bit < MAXKEY) {
		set_bit(bit, agp_bridge->key_list);
		return bit;
	}
	return -1;
}

void agp_flush_chipset(struct agp_bridge_data *bridge)
{
	if (bridge->driver->chipset_flush)
		bridge->driver->chipset_flush(bridge);
}
EXPORT_SYMBOL(agp_flush_chipset);



void agp_alloc_page_array(size_t size, struct agp_memory *mem)
{
	mem->pages = NULL;
	mem->vmalloc_flag = false;

	if (size <= 2*PAGE_SIZE)
		mem->pages = kmalloc(size, GFP_KERNEL | __GFP_NORETRY);
	if (mem->pages == NULL) {
		mem->pages = vmalloc(size);
		mem->vmalloc_flag = true;
	}
}
EXPORT_SYMBOL(agp_alloc_page_array);

void agp_free_page_array(struct agp_memory *mem)
{
	if (mem->vmalloc_flag) {
		vfree(mem->pages);
	} else {
		kfree(mem->pages);
	}
}
EXPORT_SYMBOL(agp_free_page_array);


static struct agp_memory *agp_create_user_memory(unsigned long num_agp_pages)
{
	struct agp_memory *new;
	unsigned long alloc_size = num_agp_pages*sizeof(struct page *);

	new = kzalloc(sizeof(struct agp_memory), GFP_KERNEL);
	if (new == NULL)
		return NULL;

	new->key = agp_get_key();

	if (new->key < 0) {
		kfree(new);
		return NULL;
	}

	agp_alloc_page_array(alloc_size, new);

	if (new->pages == NULL) {
		agp_free_key(new->key);
		kfree(new);
		return NULL;
	}
	new->num_scratch_pages = 0;
	return new;
}

struct agp_memory *agp_create_memory(int scratch_pages)
{
	struct agp_memory *new;

	new = kzalloc(sizeof(struct agp_memory), GFP_KERNEL);
	if (new == NULL)
		return NULL;

	new->key = agp_get_key();

	if (new->key < 0) {
		kfree(new);
		return NULL;
	}

	agp_alloc_page_array(PAGE_SIZE * scratch_pages, new);

	if (new->pages == NULL) {
		agp_free_key(new->key);
		kfree(new);
		return NULL;
	}
	new->num_scratch_pages = scratch_pages;
	new->type = AGP_NORMAL_MEMORY;
	return new;
}
EXPORT_SYMBOL(agp_create_memory);


void agp_free_memory(struct agp_memory *curr)
{
	size_t i;

	if (curr == NULL)
		return;

	if (curr->is_bound)
		agp_unbind_memory(curr);

	if (curr->type >= AGP_USER_TYPES) {
		agp_generic_free_by_type(curr);
		return;
	}

	if (curr->type != 0) {
		curr->bridge->driver->free_by_type(curr);
		return;
	}
	if (curr->page_count != 0) {
		if (curr->bridge->driver->agp_destroy_pages) {
			curr->bridge->driver->agp_destroy_pages(curr);
		} else {

			for (i = 0; i < curr->page_count; i++) {
				curr->bridge->driver->agp_destroy_page(
					curr->pages[i],
					AGP_PAGE_DESTROY_UNMAP);
			}
			for (i = 0; i < curr->page_count; i++) {
				curr->bridge->driver->agp_destroy_page(
					curr->pages[i],
					AGP_PAGE_DESTROY_FREE);
			}
		}
	}
	agp_free_key(curr->key);
	agp_free_page_array(curr);
	kfree(curr);
}
EXPORT_SYMBOL(agp_free_memory);

#define ENTRIES_PER_PAGE		(PAGE_SIZE / sizeof(unsigned long))


struct agp_memory *agp_allocate_memory(struct agp_bridge_data *bridge,
					size_t page_count, u32 type)
{
	int scratch_pages;
	struct agp_memory *new;
	size_t i;

	if (!bridge)
		return NULL;

	if ((atomic_read(&bridge->current_memory_agp) + page_count) > bridge->max_memory_agp)
		return NULL;

	if (type >= AGP_USER_TYPES) {
		new = agp_generic_alloc_user(page_count, type);
		if (new)
			new->bridge = bridge;
		return new;
	}

	if (type != 0) {
		new = bridge->driver->alloc_by_type(page_count, type);
		if (new)
			new->bridge = bridge;
		return new;
	}

	scratch_pages = (page_count + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;

	new = agp_create_memory(scratch_pages);

	if (new == NULL)
		return NULL;

	if (bridge->driver->agp_alloc_pages) {
		if (bridge->driver->agp_alloc_pages(bridge, new, page_count)) {
			agp_free_memory(new);
			return NULL;
		}
		new->bridge = bridge;
		return new;
	}

	for (i = 0; i < page_count; i++) {
		struct page *page = bridge->driver->agp_alloc_page(bridge);

		if (page == NULL) {
			agp_free_memory(new);
			return NULL;
		}
		new->pages[i] = page;
		new->page_count++;
	}
	new->bridge = bridge;

	return new;
}
EXPORT_SYMBOL(agp_allocate_memory);





static int agp_return_size(void)
{
	int current_size;
	void *temp;

	temp = agp_bridge->current_size;

	switch (agp_bridge->driver->size_type) {
	case U8_APER_SIZE:
		current_size = A_SIZE_8(temp)->size;
		break;
	case U16_APER_SIZE:
		current_size = A_SIZE_16(temp)->size;
		break;
	case U32_APER_SIZE:
		current_size = A_SIZE_32(temp)->size;
		break;
	case LVL2_APER_SIZE:
		current_size = A_SIZE_LVL2(temp)->size;
		break;
	case FIXED_APER_SIZE:
		current_size = A_SIZE_FIX(temp)->size;
		break;
	default:
		current_size = 0;
		break;
	}

	current_size -= (agp_memory_reserved / (1024*1024));
	if (current_size <0)
		current_size = 0;
	return current_size;
}


int agp_num_entries(void)
{
	int num_entries;
	void *temp;

	temp = agp_bridge->current_size;

	switch (agp_bridge->driver->size_type) {
	case U8_APER_SIZE:
		num_entries = A_SIZE_8(temp)->num_entries;
		break;
	case U16_APER_SIZE:
		num_entries = A_SIZE_16(temp)->num_entries;
		break;
	case U32_APER_SIZE:
		num_entries = A_SIZE_32(temp)->num_entries;
		break;
	case LVL2_APER_SIZE:
		num_entries = A_SIZE_LVL2(temp)->num_entries;
		break;
	case FIXED_APER_SIZE:
		num_entries = A_SIZE_FIX(temp)->num_entries;
		break;
	default:
		num_entries = 0;
		break;
	}

	num_entries -= agp_memory_reserved>>PAGE_SHIFT;
	if (num_entries<0)
		num_entries = 0;
	return num_entries;
}
EXPORT_SYMBOL_GPL(agp_num_entries);



int agp_copy_info(struct agp_bridge_data *bridge, struct agp_kern_info *info)
{
	memset(info, 0, sizeof(struct agp_kern_info));
	if (!bridge) {
		info->chipset = NOT_SUPPORTED;
		return -EIO;
	}

	info->version.major = bridge->version->major;
	info->version.minor = bridge->version->minor;
	info->chipset = SUPPORTED;
	info->device = bridge->dev;
	if (bridge->mode & AGPSTAT_MODE_3_0)
		info->mode = bridge->mode & ~AGP3_RESERVED_MASK;
	else
		info->mode = bridge->mode & ~AGP2_RESERVED_MASK;
	info->aper_base = bridge->gart_bus_addr;
	info->aper_size = agp_return_size();
	info->max_memory = bridge->max_memory_agp;
	info->current_memory = atomic_read(&bridge->current_memory_agp);
	info->cant_use_aperture = bridge->driver->cant_use_aperture;
	info->vm_ops = bridge->vm_ops;
	info->page_mask = ~0UL;
	return 0;
}
EXPORT_SYMBOL(agp_copy_info);






int agp_bind_memory(struct agp_memory *curr, off_t pg_start)
{
	int ret_val;

	if (curr == NULL)
		return -EINVAL;

	if (curr->is_bound) {
		printk(KERN_INFO PFX "memory %p is already bound!\n", curr);
		return -EINVAL;
	}
	if (!curr->is_flushed) {
		curr->bridge->driver->cache_flush();
		curr->is_flushed = true;
	}

	if (curr->bridge->driver->agp_map_memory) {
		ret_val = curr->bridge->driver->agp_map_memory(curr);
		if (ret_val)
			return ret_val;
	}
	ret_val = curr->bridge->driver->insert_memory(curr, pg_start, curr->type);

	if (ret_val != 0)
		return ret_val;

	curr->is_bound = true;
	curr->pg_start = pg_start;
	spin_lock(&agp_bridge->mapped_lock);
	list_add(&curr->mapped_list, &agp_bridge->mapped_list);
	spin_unlock(&agp_bridge->mapped_lock);

	return 0;
}
EXPORT_SYMBOL(agp_bind_memory);



int agp_unbind_memory(struct agp_memory *curr)
{
	int ret_val;

	if (curr == NULL)
		return -EINVAL;

	if (!curr->is_bound) {
		printk(KERN_INFO PFX "memory %p was not bound!\n", curr);
		return -EINVAL;
	}

	ret_val = curr->bridge->driver->remove_memory(curr, curr->pg_start, curr->type);

	if (ret_val != 0)
		return ret_val;

	if (curr->bridge->driver->agp_unmap_memory)
		curr->bridge->driver->agp_unmap_memory(curr);

	curr->is_bound = false;
	curr->pg_start = 0;
	spin_lock(&curr->bridge->mapped_lock);
	list_del(&curr->mapped_list);
	spin_unlock(&curr->bridge->mapped_lock);
	return 0;
}
EXPORT_SYMBOL(agp_unbind_memory);


int agp_rebind_memory(void)
{
	struct agp_memory *curr;
	int ret_val = 0;

	spin_lock(&agp_bridge->mapped_lock);
	list_for_each_entry(curr, &agp_bridge->mapped_list, mapped_list) {
		ret_val = curr->bridge->driver->insert_memory(curr,
							      curr->pg_start,
							      curr->type);
		if (ret_val != 0)
			break;
	}
	spin_unlock(&agp_bridge->mapped_lock);
	return ret_val;
}
EXPORT_SYMBOL(agp_rebind_memory);





static void agp_v2_parse_one(u32 *requested_mode, u32 *bridge_agpstat, u32 *vga_agpstat)
{
	u32 tmp;

	if (*requested_mode & AGP2_RESERVED_MASK) {
		printk(KERN_INFO PFX "reserved bits set (%x) in mode 0x%x. Fixed.\n",
			*requested_mode & AGP2_RESERVED_MASK, *requested_mode);
		*requested_mode &= ~AGP2_RESERVED_MASK;
	}

	
	switch (*bridge_agpstat & 7) {
	case 4:
		*bridge_agpstat |= (AGPSTAT2_2X | AGPSTAT2_1X);
		printk(KERN_INFO PFX "BIOS bug. AGP bridge claims to only support x4 rate"
			"Fixing up support for x2 & x1\n");
		break;
	case 2:
		*bridge_agpstat |= AGPSTAT2_1X;
		printk(KERN_INFO PFX "BIOS bug. AGP bridge claims to only support x2 rate"
			"Fixing up support for x1\n");
		break;
	default:
		break;
	}

	
	tmp = *requested_mode & 7;
	switch (tmp) {
		case 0:
			printk(KERN_INFO PFX "%s tried to set rate=x0. Setting to x1 mode.\n", current->comm);
			*requested_mode |= AGPSTAT2_1X;
			break;
		case 1:
		case 2:
			break;
		case 3:
			*requested_mode &= ~(AGPSTAT2_1X);	
			break;
		case 4:
			break;
		case 5:
		case 6:
		case 7:
			*requested_mode &= ~(AGPSTAT2_1X|AGPSTAT2_2X); 
			break;
	}

	
	if (!((*bridge_agpstat & AGPSTAT_SBA) && (*vga_agpstat & AGPSTAT_SBA) && (*requested_mode & AGPSTAT_SBA)))
		*bridge_agpstat &= ~AGPSTAT_SBA;

	
	if (!((*bridge_agpstat & AGPSTAT2_4X) && (*vga_agpstat & AGPSTAT2_4X) && (*requested_mode & AGPSTAT2_4X)))
		*bridge_agpstat &= ~AGPSTAT2_4X;

	if (!((*bridge_agpstat & AGPSTAT2_2X) && (*vga_agpstat & AGPSTAT2_2X) && (*requested_mode & AGPSTAT2_2X)))
		*bridge_agpstat &= ~AGPSTAT2_2X;

	if (!((*bridge_agpstat & AGPSTAT2_1X) && (*vga_agpstat & AGPSTAT2_1X) && (*requested_mode & AGPSTAT2_1X)))
		*bridge_agpstat &= ~AGPSTAT2_1X;

	
	if (*bridge_agpstat & AGPSTAT2_4X)
		*bridge_agpstat &= ~(AGPSTAT2_1X | AGPSTAT2_2X);	

	if (*bridge_agpstat & AGPSTAT2_2X)
		*bridge_agpstat &= ~(AGPSTAT2_1X | AGPSTAT2_4X);	

	if (*bridge_agpstat & AGPSTAT2_1X)
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);	

	
	if (agp_bridge->flags & AGP_ERRATA_FASTWRITES)
		*bridge_agpstat &= ~AGPSTAT_FW;

	if (agp_bridge->flags & AGP_ERRATA_SBA)
		*bridge_agpstat &= ~AGPSTAT_SBA;

	if (agp_bridge->flags & AGP_ERRATA_1X) {
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);
		*bridge_agpstat |= AGPSTAT2_1X;
	}

	
	if (*bridge_agpstat & AGPSTAT2_1X)
		*bridge_agpstat &= ~AGPSTAT_FW;
}


static void agp_v3_parse_one(u32 *requested_mode, u32 *bridge_agpstat, u32 *vga_agpstat)
{
	u32 origbridge=*bridge_agpstat, origvga=*vga_agpstat;
	u32 tmp;

	if (*requested_mode & AGP3_RESERVED_MASK) {
		printk(KERN_INFO PFX "reserved bits set (%x) in mode 0x%x. Fixed.\n",
			*requested_mode & AGP3_RESERVED_MASK, *requested_mode);
		*requested_mode &= ~AGP3_RESERVED_MASK;
	}

	
	tmp = *requested_mode & 7;
	if (tmp == 0) {
		printk(KERN_INFO PFX "%s tried to set rate=x0. Setting to AGP3 x4 mode.\n", current->comm);
		*requested_mode |= AGPSTAT3_4X;
	}
	if (tmp >= 3) {
		printk(KERN_INFO PFX "%s tried to set rate=x%d. Setting to AGP3 x8 mode.\n", current->comm, tmp * 4);
		*requested_mode = (*requested_mode & ~7) | AGPSTAT3_8X;
	}

	
	*bridge_agpstat = ((*bridge_agpstat & ~AGPSTAT_ARQSZ) |
		max_t(u32,(*bridge_agpstat & AGPSTAT_ARQSZ),(*vga_agpstat & AGPSTAT_ARQSZ)));

	
	*bridge_agpstat = ((*bridge_agpstat & ~AGPSTAT_CAL_MASK) |
		min_t(u32,(*bridge_agpstat & AGPSTAT_CAL_MASK),(*vga_agpstat & AGPSTAT_CAL_MASK)));

	
	*bridge_agpstat |= AGPSTAT_SBA;

	
	if (*requested_mode & AGPSTAT_MODE_3_0) {
		
		if (*requested_mode & AGPSTAT2_4X) {
			printk(KERN_INFO PFX "%s passes broken AGP3 flags (%x). Fixed.\n",
						current->comm, *requested_mode);
			*requested_mode &= ~AGPSTAT2_4X;
			*requested_mode |= AGPSTAT3_4X;
		}
	} else {
		
		printk(KERN_INFO PFX "%s passes broken AGP2 flags (%x) in AGP3 mode. Fixed.\n",
					current->comm, *requested_mode);
		*requested_mode &= ~(AGPSTAT2_4X | AGPSTAT2_2X | AGPSTAT2_1X);
		*requested_mode |= AGPSTAT3_4X;
	}

	if (*requested_mode & AGPSTAT3_8X) {
		if (!(*bridge_agpstat & AGPSTAT3_8X)) {
			*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
			*bridge_agpstat |= AGPSTAT3_4X;
			printk(KERN_INFO PFX "%s requested AGPx8 but bridge not capable.\n", current->comm);
			return;
		}
		if (!(*vga_agpstat & AGPSTAT3_8X)) {
			*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
			*bridge_agpstat |= AGPSTAT3_4X;
			printk(KERN_INFO PFX "%s requested AGPx8 but graphic card not capable.\n", current->comm);
			return;
		}
		
		*bridge_agpstat &= ~(AGPSTAT3_4X | AGPSTAT3_RSVD);
		goto done;

	} else if (*requested_mode & AGPSTAT3_4X) {
		*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
		*bridge_agpstat |= AGPSTAT3_4X;
		goto done;

	} else {

		
		if ((*bridge_agpstat & AGPSTAT3_8X) && (*vga_agpstat & AGPSTAT3_8X)) {
			printk(KERN_INFO PFX "No AGP mode specified. Setting to highest mode "
				"supported by bridge & card (x8).\n");
			*bridge_agpstat &= ~(AGPSTAT3_4X | AGPSTAT3_RSVD);
			*vga_agpstat &= ~(AGPSTAT3_4X | AGPSTAT3_RSVD);
		} else {
			printk(KERN_INFO PFX "Fell back to AGPx4 mode because");
			if (!(*bridge_agpstat & AGPSTAT3_8X)) {
				printk(KERN_INFO PFX "bridge couldn't do x8. bridge_agpstat:%x (orig=%x)\n",
					*bridge_agpstat, origbridge);
				*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
				*bridge_agpstat |= AGPSTAT3_4X;
			}
			if (!(*vga_agpstat & AGPSTAT3_8X)) {
				printk(KERN_INFO PFX "graphics card couldn't do x8. vga_agpstat:%x (orig=%x)\n",
					*vga_agpstat, origvga);
				*vga_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
				*vga_agpstat |= AGPSTAT3_4X;
			}
		}
	}

done:
	
	if (agp_bridge->flags & AGP_ERRATA_FASTWRITES)
		*bridge_agpstat &= ~AGPSTAT_FW;

	if (agp_bridge->flags & AGP_ERRATA_SBA)
		*bridge_agpstat &= ~AGPSTAT_SBA;

	if (agp_bridge->flags & AGP_ERRATA_1X) {
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);
		*bridge_agpstat |= AGPSTAT2_1X;
	}
}



u32 agp_collect_device_status(struct agp_bridge_data *bridge, u32 requested_mode, u32 bridge_agpstat)
{
	struct pci_dev *device = NULL;
	u32 vga_agpstat;
	u8 cap_ptr;

	for (;;) {
		device = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, device);
		if (!device) {
			printk(KERN_INFO PFX "Couldn't find an AGP VGA controller.\n");
			return 0;
		}
		cap_ptr = pci_find_capability(device, PCI_CAP_ID_AGP);
		if (cap_ptr)
			break;
	}

	
	pci_read_config_dword(device, cap_ptr+PCI_AGP_STATUS, &vga_agpstat);

	
	bridge_agpstat = ((bridge_agpstat & ~AGPSTAT_RQ_DEPTH) |
	     min_t(u32, (requested_mode & AGPSTAT_RQ_DEPTH),
		 min_t(u32, (bridge_agpstat & AGPSTAT_RQ_DEPTH), (vga_agpstat & AGPSTAT_RQ_DEPTH))));

	
	if (!((bridge_agpstat & AGPSTAT_FW) &&
		 (vga_agpstat & AGPSTAT_FW) &&
		 (requested_mode & AGPSTAT_FW)))
		bridge_agpstat &= ~AGPSTAT_FW;

	
	if (agp_bridge->mode & AGPSTAT_MODE_3_0)
		agp_v3_parse_one(&requested_mode, &bridge_agpstat, &vga_agpstat);
	else
		agp_v2_parse_one(&requested_mode, &bridge_agpstat, &vga_agpstat);

	pci_dev_put(device);
	return bridge_agpstat;
}
EXPORT_SYMBOL(agp_collect_device_status);


void agp_device_command(u32 bridge_agpstat, bool agp_v3)
{
	struct pci_dev *device = NULL;
	int mode;

	mode = bridge_agpstat & 0x7;
	if (agp_v3)
		mode *= 4;

	for_each_pci_dev(device) {
		u8 agp = pci_find_capability(device, PCI_CAP_ID_AGP);
		if (!agp)
			continue;

		dev_info(&device->dev, "putting AGP V%d device into %dx mode\n",
			 agp_v3 ? 3 : 2, mode);
		pci_write_config_dword(device, agp + PCI_AGP_COMMAND, bridge_agpstat);
	}
}
EXPORT_SYMBOL(agp_device_command);


void get_agp_version(struct agp_bridge_data *bridge)
{
	u32 ncapid;

	
	if (bridge->major_version != 0)
		return;

	pci_read_config_dword(bridge->dev, bridge->capndx, &ncapid);
	bridge->major_version = (ncapid >> AGP_MAJOR_VERSION_SHIFT) & 0xf;
	bridge->minor_version = (ncapid >> AGP_MINOR_VERSION_SHIFT) & 0xf;
}
EXPORT_SYMBOL(get_agp_version);


void agp_generic_enable(struct agp_bridge_data *bridge, u32 requested_mode)
{
	u32 bridge_agpstat, temp;

	get_agp_version(agp_bridge);

	dev_info(&agp_bridge->dev->dev, "AGP %d.%d bridge\n",
		 agp_bridge->major_version, agp_bridge->minor_version);

	pci_read_config_dword(agp_bridge->dev,
		      agp_bridge->capndx + PCI_AGP_STATUS, &bridge_agpstat);

	bridge_agpstat = agp_collect_device_status(agp_bridge, requested_mode, bridge_agpstat);
	if (bridge_agpstat == 0)
		
		return;

	bridge_agpstat |= AGPSTAT_AGP_ENABLE;

	
	if (bridge->major_version >= 3) {
		if (bridge->mode & AGPSTAT_MODE_3_0) {
			
			if (bridge->minor_version >= 5)
				agp_3_5_enable(bridge);
			agp_device_command(bridge_agpstat, true);
			return;
		} else {
		    
		    bridge_agpstat &= ~(7<<10) ;
		    pci_read_config_dword(bridge->dev,
					bridge->capndx+AGPCTRL, &temp);
		    temp |= (1<<9);
		    pci_write_config_dword(bridge->dev,
					bridge->capndx+AGPCTRL, temp);

		    dev_info(&bridge->dev->dev, "bridge is in legacy mode, falling back to 2.x\n");
		}
	}

	
	agp_device_command(bridge_agpstat, false);
}
EXPORT_SYMBOL(agp_generic_enable);


int agp_generic_create_gatt_table(struct agp_bridge_data *bridge)
{
	char *table;
	char *table_end;
	int size;
	int page_order;
	int num_entries;
	int i;
	void *temp;
	struct page *page;

	
	if (bridge->driver->size_type == LVL2_APER_SIZE)
		return -EINVAL;

	table = NULL;
	i = bridge->aperture_size_idx;
	temp = bridge->current_size;
	size = page_order = num_entries = 0;

	if (bridge->driver->size_type != FIXED_APER_SIZE) {
		do {
			switch (bridge->driver->size_type) {
			case U8_APER_SIZE:
				size = A_SIZE_8(temp)->size;
				page_order =
				    A_SIZE_8(temp)->page_order;
				num_entries =
				    A_SIZE_8(temp)->num_entries;
				break;
			case U16_APER_SIZE:
				size = A_SIZE_16(temp)->size;
				page_order = A_SIZE_16(temp)->page_order;
				num_entries = A_SIZE_16(temp)->num_entries;
				break;
			case U32_APER_SIZE:
				size = A_SIZE_32(temp)->size;
				page_order = A_SIZE_32(temp)->page_order;
				num_entries = A_SIZE_32(temp)->num_entries;
				break;
				
			case FIXED_APER_SIZE:
			case LVL2_APER_SIZE:
			default:
				size = page_order = num_entries = 0;
				break;
			}

			table = alloc_gatt_pages(page_order);

			if (table == NULL) {
				i++;
				switch (bridge->driver->size_type) {
				case U8_APER_SIZE:
					bridge->current_size = A_IDX8(bridge);
					break;
				case U16_APER_SIZE:
					bridge->current_size = A_IDX16(bridge);
					break;
				case U32_APER_SIZE:
					bridge->current_size = A_IDX32(bridge);
					break;
				
				case FIXED_APER_SIZE:
				case LVL2_APER_SIZE:
				default:
					break;
				}
				temp = bridge->current_size;
			} else {
				bridge->aperture_size_idx = i;
			}
		} while (!table && (i < bridge->driver->num_aperture_sizes));
	} else {
		size = ((struct aper_size_info_fixed *) temp)->size;
		page_order = ((struct aper_size_info_fixed *) temp)->page_order;
		num_entries = ((struct aper_size_info_fixed *) temp)->num_entries;
		table = alloc_gatt_pages(page_order);
	}

	if (table == NULL)
		return -ENOMEM;

	table_end = table + ((PAGE_SIZE * (1 << page_order)) - 1);

	for (page = virt_to_page(table); page <= virt_to_page(table_end); page++)
		SetPageReserved(page);

	bridge->gatt_table_real = (u32 *) table;
	agp_gatt_table = (void *)table;

	bridge->driver->cache_flush();
#ifdef CONFIG_X86
	set_memory_uc((unsigned long)table, 1 << page_order);
	bridge->gatt_table = (void *)table;
#else
	bridge->gatt_table = ioremap_nocache(virt_to_phys(table),
					(PAGE_SIZE * (1 << page_order)));
	bridge->driver->cache_flush();
#endif

	if (bridge->gatt_table == NULL) {
		for (page = virt_to_page(table); page <= virt_to_page(table_end); page++)
			ClearPageReserved(page);

		free_gatt_pages(table, page_order);

		return -ENOMEM;
	}
	bridge->gatt_bus_addr = virt_to_phys(bridge->gatt_table_real);

	
	for (i = 0; i < num_entries; i++) {
		writel(bridge->scratch_page, bridge->gatt_table+i);
		readl(bridge->gatt_table+i);	
	}

	return 0;
}
EXPORT_SYMBOL(agp_generic_create_gatt_table);

int agp_generic_free_gatt_table(struct agp_bridge_data *bridge)
{
	int page_order;
	char *table, *table_end;
	void *temp;
	struct page *page;

	temp = bridge->current_size;

	switch (bridge->driver->size_type) {
	case U8_APER_SIZE:
		page_order = A_SIZE_8(temp)->page_order;
		break;
	case U16_APER_SIZE:
		page_order = A_SIZE_16(temp)->page_order;
		break;
	case U32_APER_SIZE:
		page_order = A_SIZE_32(temp)->page_order;
		break;
	case FIXED_APER_SIZE:
		page_order = A_SIZE_FIX(temp)->page_order;
		break;
	case LVL2_APER_SIZE:
		
		return -EINVAL;
		break;
	default:
		page_order = 0;
		break;
	}

	

#ifdef CONFIG_X86
	set_memory_wb((unsigned long)bridge->gatt_table, 1 << page_order);
#else
	iounmap(bridge->gatt_table);
#endif
	table = (char *) bridge->gatt_table_real;
	table_end = table + ((PAGE_SIZE * (1 << page_order)) - 1);

	for (page = virt_to_page(table); page <= virt_to_page(table_end); page++)
		ClearPageReserved(page);

	free_gatt_pages(bridge->gatt_table_real, page_order);

	agp_gatt_table = NULL;
	bridge->gatt_table = NULL;
	bridge->gatt_table_real = NULL;
	bridge->gatt_bus_addr = 0;

	return 0;
}
EXPORT_SYMBOL(agp_generic_free_gatt_table);


int agp_generic_insert_memory(struct agp_memory * mem, off_t pg_start, int type)
{
	int num_entries;
	size_t i;
	off_t j;
	void *temp;
	struct agp_bridge_data *bridge;
	int mask_type;

	bridge = mem->bridge;
	if (!bridge)
		return -EINVAL;

	if (mem->page_count == 0)
		return 0;

	temp = bridge->current_size;

	switch (bridge->driver->size_type) {
	case U8_APER_SIZE:
		num_entries = A_SIZE_8(temp)->num_entries;
		break;
	case U16_APER_SIZE:
		num_entries = A_SIZE_16(temp)->num_entries;
		break;
	case U32_APER_SIZE:
		num_entries = A_SIZE_32(temp)->num_entries;
		break;
	case FIXED_APER_SIZE:
		num_entries = A_SIZE_FIX(temp)->num_entries;
		break;
	case LVL2_APER_SIZE:
		
		return -EINVAL;
		break;
	default:
		num_entries = 0;
		break;
	}

	num_entries -= agp_memory_reserved/PAGE_SIZE;
	if (num_entries < 0) num_entries = 0;

	if (type != mem->type)
		return -EINVAL;

	mask_type = bridge->driver->agp_type_to_mask_type(bridge, type);
	if (mask_type != 0) {
		
		return -EINVAL;
	}

	
	if ((pg_start + mem->page_count) > num_entries)
		return -EINVAL;

	j = pg_start;

	while (j < (pg_start + mem->page_count)) {
		if (!PGE_EMPTY(bridge, readl(bridge->gatt_table+j)))
			return -EBUSY;
		j++;
	}

	if (!mem->is_flushed) {
		bridge->driver->cache_flush();
		mem->is_flushed = true;
	}

	for (i = 0, j = pg_start; i < mem->page_count; i++, j++) {
		writel(bridge->driver->mask_memory(bridge,
						   page_to_phys(mem->pages[i]),
						   mask_type),
		       bridge->gatt_table+j);
	}
	readl(bridge->gatt_table+j-1);	

	bridge->driver->tlb_flush(mem);
	return 0;
}
EXPORT_SYMBOL(agp_generic_insert_memory);


int agp_generic_remove_memory(struct agp_memory *mem, off_t pg_start, int type)
{
	size_t i;
	struct agp_bridge_data *bridge;
	int mask_type;

	bridge = mem->bridge;
	if (!bridge)
		return -EINVAL;

	if (mem->page_count == 0)
		return 0;

	if (type != mem->type)
		return -EINVAL;

	mask_type = bridge->driver->agp_type_to_mask_type(bridge, type);
	if (mask_type != 0) {
		
		return -EINVAL;
	}

	
	for (i = pg_start; i < (mem->page_count + pg_start); i++) {
		writel(bridge->scratch_page, bridge->gatt_table+i);
	}
	readl(bridge->gatt_table+i-1);	

	bridge->driver->tlb_flush(mem);
	return 0;
}
EXPORT_SYMBOL(agp_generic_remove_memory);

struct agp_memory *agp_generic_alloc_by_type(size_t page_count, int type)
{
	return NULL;
}
EXPORT_SYMBOL(agp_generic_alloc_by_type);

void agp_generic_free_by_type(struct agp_memory *curr)
{
	agp_free_page_array(curr);
	agp_free_key(curr->key);
	kfree(curr);
}
EXPORT_SYMBOL(agp_generic_free_by_type);

struct agp_memory *agp_generic_alloc_user(size_t page_count, int type)
{
	struct agp_memory *new;
	int i;
	int pages;

	pages = (page_count + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;
	new = agp_create_user_memory(page_count);
	if (new == NULL)
		return NULL;

	for (i = 0; i < page_count; i++)
		new->pages[i] = 0;
	new->page_count = 0;
	new->type = type;
	new->num_scratch_pages = pages;

	return new;
}
EXPORT_SYMBOL(agp_generic_alloc_user);



int agp_generic_alloc_pages(struct agp_bridge_data *bridge, struct agp_memory *mem, size_t num_pages)
{
	struct page * page;
	int i, ret = -ENOMEM;

	for (i = 0; i < num_pages; i++) {
		page = alloc_page(GFP_KERNEL | GFP_DMA32 | __GFP_ZERO);
		
		if (page == NULL)
			goto out;

#ifndef CONFIG_X86
		map_page_into_agp(page);
#endif
		get_page(page);
		atomic_inc(&agp_bridge->current_memory_agp);

		mem->pages[i] = page;
		mem->page_count++;
	}

#ifdef CONFIG_X86
	set_pages_array_uc(mem->pages, num_pages);
#endif
	ret = 0;
out:
	return ret;
}
EXPORT_SYMBOL(agp_generic_alloc_pages);

struct page *agp_generic_alloc_page(struct agp_bridge_data *bridge)
{
	struct page * page;

	page = alloc_page(GFP_KERNEL | GFP_DMA32 | __GFP_ZERO);
	if (page == NULL)
		return NULL;

	map_page_into_agp(page);

	get_page(page);
	atomic_inc(&agp_bridge->current_memory_agp);
	return page;
}
EXPORT_SYMBOL(agp_generic_alloc_page);

void agp_generic_destroy_pages(struct agp_memory *mem)
{
	int i;
	struct page *page;

	if (!mem)
		return;

#ifdef CONFIG_X86
	set_pages_array_wb(mem->pages, mem->page_count);
#endif

	for (i = 0; i < mem->page_count; i++) {
		page = mem->pages[i];

#ifndef CONFIG_X86
		unmap_page_from_agp(page);
#endif
		put_page(page);
		__free_page(page);
		atomic_dec(&agp_bridge->current_memory_agp);
		mem->pages[i] = NULL;
	}
}
EXPORT_SYMBOL(agp_generic_destroy_pages);

void agp_generic_destroy_page(struct page *page, int flags)
{
	if (page == NULL)
		return;

	if (flags & AGP_PAGE_DESTROY_UNMAP)
		unmap_page_from_agp(page);

	if (flags & AGP_PAGE_DESTROY_FREE) {
		put_page(page);
		__free_page(page);
		atomic_dec(&agp_bridge->current_memory_agp);
	}
}
EXPORT_SYMBOL(agp_generic_destroy_page);





void agp_enable(struct agp_bridge_data *bridge, u32 mode)
{
	if (!bridge)
		return;
	bridge->driver->agp_enable(bridge, mode);
}
EXPORT_SYMBOL(agp_enable);



struct agp_bridge_data *agp_generic_find_bridge(struct pci_dev *pdev)
{
	if (list_empty(&agp_bridges))
		return NULL;

	return agp_bridge;
}

static void ipi_handler(void *null)
{
	flush_agp_cache();
}

void global_cache_flush(void)
{
	if (on_each_cpu(ipi_handler, NULL, 1) != 0)
		panic(PFX "timed out waiting for the other CPUs!\n");
}
EXPORT_SYMBOL(global_cache_flush);

unsigned long agp_generic_mask_memory(struct agp_bridge_data *bridge,
				      dma_addr_t addr, int type)
{
	
	if (bridge->driver->masks)
		return addr | bridge->driver->masks[0].mask;
	else
		return addr;
}
EXPORT_SYMBOL(agp_generic_mask_memory);

int agp_generic_type_to_mask_type(struct agp_bridge_data *bridge,
				  int type)
{
	if (type >= AGP_USER_TYPES)
		return 0;
	return type;
}
EXPORT_SYMBOL(agp_generic_type_to_mask_type);



int agp3_generic_fetch_size(void)
{
	u16 temp_size;
	int i;
	struct aper_size_info_16 *values;

	pci_read_config_word(agp_bridge->dev, agp_bridge->capndx+AGPAPSIZE, &temp_size);
	values = A_SIZE_16(agp_bridge->driver->aperture_sizes);

	for (i = 0; i < agp_bridge->driver->num_aperture_sizes; i++) {
		if (temp_size == values[i].size_value) {
			agp_bridge->previous_size =
				agp_bridge->current_size = (void *) (values + i);

			agp_bridge->aperture_size_idx = i;
			return values[i].size;
		}
	}
	return 0;
}
EXPORT_SYMBOL(agp3_generic_fetch_size);

void agp3_generic_tlbflush(struct agp_memory *mem)
{
	u32 ctrl;
	pci_read_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, &ctrl);
	pci_write_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, ctrl & ~AGPCTRL_GTLBEN);
	pci_write_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, ctrl);
}
EXPORT_SYMBOL(agp3_generic_tlbflush);

int agp3_generic_configure(void)
{
	u32 temp;
	struct aper_size_info_16 *current_size;

	current_size = A_SIZE_16(agp_bridge->current_size);

	pci_read_config_dword(agp_bridge->dev, AGP_APBASE, &temp);
	agp_bridge->gart_bus_addr = (temp & PCI_BASE_ADDRESS_MEM_MASK);

	
	pci_write_config_word(agp_bridge->dev, agp_bridge->capndx+AGPAPSIZE, current_size->size_value);
	
	pci_write_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPGARTLO, agp_bridge->gatt_bus_addr);
	
	pci_read_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, &temp);
	pci_write_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, temp | AGPCTRL_APERENB | AGPCTRL_GTLBEN);
	return 0;
}
EXPORT_SYMBOL(agp3_generic_configure);

void agp3_generic_cleanup(void)
{
	u32 ctrl;
	pci_read_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, &ctrl);
	pci_write_config_dword(agp_bridge->dev, agp_bridge->capndx+AGPCTRL, ctrl & ~AGPCTRL_APERENB);
}
EXPORT_SYMBOL(agp3_generic_cleanup);

const struct aper_size_info_16 agp3_generic_sizes[AGP_GENERIC_SIZES_ENTRIES] =
{
	{4096, 1048576, 10,0x000},
	{2048,  524288, 9, 0x800},
	{1024,  262144, 8, 0xc00},
	{ 512,  131072, 7, 0xe00},
	{ 256,   65536, 6, 0xf00},
	{ 128,   32768, 5, 0xf20},
	{  64,   16384, 4, 0xf30},
	{  32,    8192, 3, 0xf38},
	{  16,    4096, 2, 0xf3c},
	{   8,    2048, 1, 0xf3e},
	{   4,    1024, 0, 0xf3f}
};
EXPORT_SYMBOL(agp3_generic_sizes);

