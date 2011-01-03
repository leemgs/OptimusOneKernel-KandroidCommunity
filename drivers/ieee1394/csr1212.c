




#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/string.h>
#include <asm/bug.h>
#include <asm/byteorder.h>

#include "csr1212.h"



#define __I (1 << CSR1212_KV_TYPE_IMMEDIATE)
#define __C (1 << CSR1212_KV_TYPE_CSR_OFFSET)
#define __D (1 << CSR1212_KV_TYPE_DIRECTORY)
#define __L (1 << CSR1212_KV_TYPE_LEAF)
static const u8 csr1212_key_id_type_map[0x30] = {
	__C,			
	__D | __L,		
	__I | __D | __L,	
	__I | __D | __L,	
	__I,			
	0, 0,			
	__D | __L | __I,	
	__I, 0, 0, 0,		
	__I,			
	__L,			
	0, 0, 0,		
	__D,			
	__I,			
	__I,			
	__I | __C | __D | __L,	
	__L,			
	0,			
	__I,			
	__D,			
	__L,			
	__D,			
	__L,			
	__I,			
	__I,			
	__I | __C | __D | __L,	
	__L,			
	__I,			
	__I,			
};
#undef __I
#undef __C
#undef __D
#undef __L


#define quads_to_bytes(_q) ((_q) * sizeof(u32))
#define bytes_to_quads(_b) DIV_ROUND_UP(_b, sizeof(u32))

static void free_keyval(struct csr1212_keyval *kv)
{
	if ((kv->key.type == CSR1212_KV_TYPE_LEAF) &&
	    (kv->key.id != CSR1212_KV_ID_EXTENDED_ROM))
		CSR1212_FREE(kv->value.leaf.data);

	CSR1212_FREE(kv);
}

static u16 csr1212_crc16(const u32 *buffer, size_t length)
{
	int shift;
	u32 data;
	u16 sum, crc = 0;

	for (; length; length--) {
		data = be32_to_cpu(*buffer);
		buffer++;
		for (shift = 28; shift >= 0; shift -= 4 ) {
			sum = ((crc >> 12) ^ (data >> shift)) & 0xf;
			crc = (crc << 4) ^ (sum << 12) ^ (sum << 5) ^ (sum);
		}
		crc &= 0xffff;
	}

	return cpu_to_be16(crc);
}


static u16 csr1212_msft_crc16(const u32 *buffer, size_t length)
{
	int shift;
	u32 data;
	u16 sum, crc = 0;

	for (; length; length--) {
		data = le32_to_cpu(*buffer);
		buffer++;
		for (shift = 28; shift >= 0; shift -= 4 ) {
			sum = ((crc >> 12) ^ (data >> shift)) & 0xf;
			crc = (crc << 4) ^ (sum << 12) ^ (sum << 5) ^ (sum);
		}
		crc &= 0xffff;
	}

	return cpu_to_be16(crc);
}

static struct csr1212_dentry *
csr1212_find_keyval(struct csr1212_keyval *dir, struct csr1212_keyval *kv)
{
	struct csr1212_dentry *pos;

	for (pos = dir->value.directory.dentries_head;
	     pos != NULL; pos = pos->next)
		if (pos->kv == kv)
			return pos;
	return NULL;
}

static struct csr1212_keyval *
csr1212_find_keyval_offset(struct csr1212_keyval *kv_list, u32 offset)
{
	struct csr1212_keyval *kv;

	for (kv = kv_list->next; kv && (kv != kv_list); kv = kv->next)
		if (kv->offset == offset)
			return kv;
	return NULL;
}




struct csr1212_csr *csr1212_create_csr(struct csr1212_bus_ops *ops,
				       size_t bus_info_size, void *private)
{
	struct csr1212_csr *csr;

	csr = CSR1212_MALLOC(sizeof(*csr));
	if (!csr)
		return NULL;

	csr->cache_head =
		csr1212_rom_cache_malloc(CSR1212_CONFIG_ROM_SPACE_OFFSET,
					 CSR1212_CONFIG_ROM_SPACE_SIZE);
	if (!csr->cache_head) {
		CSR1212_FREE(csr);
		return NULL;
	}

	
	csr->root_kv = csr1212_new_directory(CSR1212_KV_ID_VENDOR);
	if (!csr->root_kv) {
		CSR1212_FREE(csr->cache_head);
		CSR1212_FREE(csr);
		return NULL;
	}

	csr->bus_info_data = csr->cache_head->data;
	csr->bus_info_len = bus_info_size;
	csr->crc_len = bus_info_size;
	csr->ops = ops;
	csr->private = private;
	csr->cache_tail = csr->cache_head;

	return csr;
}

void csr1212_init_local_csr(struct csr1212_csr *csr,
			    const u32 *bus_info_data, int max_rom)
{
	static const int mr_map[] = { 4, 64, 1024, 0 };

	BUG_ON(max_rom & ~0x3);
	csr->max_rom = mr_map[max_rom];
	memcpy(csr->bus_info_data, bus_info_data, csr->bus_info_len);
}

static struct csr1212_keyval *csr1212_new_keyval(u8 type, u8 key)
{
	struct csr1212_keyval *kv;

	if (key < 0x30 && ((csr1212_key_id_type_map[key] & (1 << type)) == 0))
		return NULL;

	kv = CSR1212_MALLOC(sizeof(*kv));
	if (!kv)
		return NULL;

	atomic_set(&kv->refcnt, 1);
	kv->key.type = type;
	kv->key.id = key;
	kv->associate = NULL;
	kv->next = NULL;
	kv->prev = NULL;
	kv->offset = 0;
	kv->valid = 0;
	return kv;
}

struct csr1212_keyval *csr1212_new_immediate(u8 key, u32 value)
{
	struct csr1212_keyval *kv;

	kv = csr1212_new_keyval(CSR1212_KV_TYPE_IMMEDIATE, key);
	if (!kv)
		return NULL;

	kv->value.immediate = value;
	kv->valid = 1;
	return kv;
}

static struct csr1212_keyval *
csr1212_new_leaf(u8 key, const void *data, size_t data_len)
{
	struct csr1212_keyval *kv;

	kv = csr1212_new_keyval(CSR1212_KV_TYPE_LEAF, key);
	if (!kv)
		return NULL;

	if (data_len > 0) {
		kv->value.leaf.data = CSR1212_MALLOC(data_len);
		if (!kv->value.leaf.data) {
			CSR1212_FREE(kv);
			return NULL;
		}

		if (data)
			memcpy(kv->value.leaf.data, data, data_len);
	} else {
		kv->value.leaf.data = NULL;
	}

	kv->value.leaf.len = bytes_to_quads(data_len);
	kv->offset = 0;
	kv->valid = 1;

	return kv;
}

static struct csr1212_keyval *
csr1212_new_csr_offset(u8 key, u32 csr_offset)
{
	struct csr1212_keyval *kv;

	kv = csr1212_new_keyval(CSR1212_KV_TYPE_CSR_OFFSET, key);
	if (!kv)
		return NULL;

	kv->value.csr_offset = csr_offset;

	kv->offset = 0;
	kv->valid = 1;
	return kv;
}

struct csr1212_keyval *csr1212_new_directory(u8 key)
{
	struct csr1212_keyval *kv;

	kv = csr1212_new_keyval(CSR1212_KV_TYPE_DIRECTORY, key);
	if (!kv)
		return NULL;

	kv->value.directory.len = 0;
	kv->offset = 0;
	kv->value.directory.dentries_head = NULL;
	kv->value.directory.dentries_tail = NULL;
	kv->valid = 1;
	return kv;
}

void csr1212_associate_keyval(struct csr1212_keyval *kv,
			      struct csr1212_keyval *associate)
{
	BUG_ON(!kv || !associate || kv->key.id == CSR1212_KV_ID_DESCRIPTOR ||
	       (associate->key.id != CSR1212_KV_ID_DESCRIPTOR &&
		associate->key.id != CSR1212_KV_ID_DEPENDENT_INFO &&
		associate->key.id != CSR1212_KV_ID_EXTENDED_KEY &&
		associate->key.id != CSR1212_KV_ID_EXTENDED_DATA &&
		associate->key.id < 0x30) ||
	       (kv->key.id == CSR1212_KV_ID_EXTENDED_KEY_SPECIFIER_ID &&
		associate->key.id != CSR1212_KV_ID_EXTENDED_KEY) ||
	       (kv->key.id == CSR1212_KV_ID_EXTENDED_KEY &&
		associate->key.id != CSR1212_KV_ID_EXTENDED_DATA) ||
	       (associate->key.id == CSR1212_KV_ID_EXTENDED_KEY &&
		kv->key.id != CSR1212_KV_ID_EXTENDED_KEY_SPECIFIER_ID) ||
	       (associate->key.id == CSR1212_KV_ID_EXTENDED_DATA &&
		kv->key.id != CSR1212_KV_ID_EXTENDED_KEY));

	if (kv->associate)
		csr1212_release_keyval(kv->associate);

	csr1212_keep_keyval(associate);
	kv->associate = associate;
}

static int __csr1212_attach_keyval_to_directory(struct csr1212_keyval *dir,
						struct csr1212_keyval *kv,
						bool keep_keyval)
{
	struct csr1212_dentry *dentry;

	BUG_ON(!kv || !dir || dir->key.type != CSR1212_KV_TYPE_DIRECTORY);

	dentry = CSR1212_MALLOC(sizeof(*dentry));
	if (!dentry)
		return -ENOMEM;

	if (keep_keyval)
		csr1212_keep_keyval(kv);
	dentry->kv = kv;

	dentry->next = NULL;
	dentry->prev = dir->value.directory.dentries_tail;

	if (!dir->value.directory.dentries_head)
		dir->value.directory.dentries_head = dentry;

	if (dir->value.directory.dentries_tail)
		dir->value.directory.dentries_tail->next = dentry;
	dir->value.directory.dentries_tail = dentry;

	return CSR1212_SUCCESS;
}

int csr1212_attach_keyval_to_directory(struct csr1212_keyval *dir,
				       struct csr1212_keyval *kv)
{
	return __csr1212_attach_keyval_to_directory(dir, kv, true);
}

#define CSR1212_DESCRIPTOR_LEAF_DATA(kv) \
	(&((kv)->value.leaf.data[1]))

#define CSR1212_DESCRIPTOR_LEAF_SET_TYPE(kv, type) \
	((kv)->value.leaf.data[0] = \
	 cpu_to_be32(CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) | \
		     ((type) << CSR1212_DESCRIPTOR_LEAF_TYPE_SHIFT)))
#define CSR1212_DESCRIPTOR_LEAF_SET_SPECIFIER_ID(kv, spec_id) \
	((kv)->value.leaf.data[0] = \
	 cpu_to_be32((CSR1212_DESCRIPTOR_LEAF_TYPE(kv) << \
		      CSR1212_DESCRIPTOR_LEAF_TYPE_SHIFT) | \
		     ((spec_id) & CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID_MASK)))

static struct csr1212_keyval *
csr1212_new_descriptor_leaf(u8 dtype, u32 specifier_id,
			    const void *data, size_t data_len)
{
	struct csr1212_keyval *kv;

	kv = csr1212_new_leaf(CSR1212_KV_ID_DESCRIPTOR, NULL,
			      data_len + CSR1212_DESCRIPTOR_LEAF_OVERHEAD);
	if (!kv)
		return NULL;

	kmemcheck_annotate_variable(kv->value.leaf.data[0]);
	CSR1212_DESCRIPTOR_LEAF_SET_TYPE(kv, dtype);
	CSR1212_DESCRIPTOR_LEAF_SET_SPECIFIER_ID(kv, specifier_id);

	if (data)
		memcpy(CSR1212_DESCRIPTOR_LEAF_DATA(kv), data, data_len);

	return kv;
}


static int csr1212_check_minimal_ascii(const char *s)
{
	static const char minimal_ascii_table[] = {
					
		128,			
		4 + 16 + 32,		
		0,			
		0,			
		255 - 8 - 16,		
		255,			
		255,			
		255,			
		255,			
		255,			
		255,			
		1 + 2 + 4 + 128,	
		255 - 1,		
		255,			
		255,			
		1 + 2 + 4,		
	};
	int i, j;

	for (; *s; s++) {
		i = *s >> 3;		
		j = 1 << (*s & 3);	

		if (i >= ARRAY_SIZE(minimal_ascii_table) ||
		    !(minimal_ascii_table[i] & j))
			return -EINVAL;
	}
	return 0;
}


struct csr1212_keyval *csr1212_new_string_descriptor_leaf(const char *s)
{
	struct csr1212_keyval *kv;
	u32 *text;
	size_t str_len, quads;

	if (!s || !*s || csr1212_check_minimal_ascii(s))
		return NULL;

	str_len = strlen(s);
	quads = bytes_to_quads(str_len);
	kv = csr1212_new_descriptor_leaf(0, 0, NULL, quads_to_bytes(quads) +
				      CSR1212_TEXTUAL_DESCRIPTOR_LEAF_OVERHEAD);
	if (!kv)
		return NULL;

	kv->value.leaf.data[1] = 0;	
	text = CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(kv);
	text[quads - 1] = 0;		
	memcpy(text, s, str_len);

	return kv;
}




void csr1212_detach_keyval_from_directory(struct csr1212_keyval *dir,
					  struct csr1212_keyval *kv)
{
	struct csr1212_dentry *dentry;

	if (!kv || !dir || dir->key.type != CSR1212_KV_TYPE_DIRECTORY)
		return;

	dentry = csr1212_find_keyval(dir, kv);

	if (!dentry)
		return;

	if (dentry->prev)
		dentry->prev->next = dentry->next;
	if (dentry->next)
		dentry->next->prev = dentry->prev;
	if (dir->value.directory.dentries_head == dentry)
		dir->value.directory.dentries_head = dentry->next;
	if (dir->value.directory.dentries_tail == dentry)
		dir->value.directory.dentries_tail = dentry->prev;

	CSR1212_FREE(dentry);

	csr1212_release_keyval(kv);
}


void csr1212_release_keyval(struct csr1212_keyval *kv)
{
	struct csr1212_keyval *k, *a;
	struct csr1212_dentry dentry;
	struct csr1212_dentry *head, *tail;

	if (!atomic_dec_and_test(&kv->refcnt))
		return;

	dentry.kv = kv;
	dentry.next = NULL;
	dentry.prev = NULL;

	head = &dentry;
	tail = head;

	while (head) {
		k = head->kv;

		while (k) {
			
			if (k != kv && !atomic_dec_and_test(&k->refcnt))
				break;

			a = k->associate;

			if (k->key.type == CSR1212_KV_TYPE_DIRECTORY) {
				
				if (k->value.directory.dentries_head) {
					tail->next =
					    k->value.directory.dentries_head;
					k->value.directory.dentries_head->prev =
					    tail;
					tail = k->value.directory.dentries_tail;
				}
			}
			free_keyval(k);
			k = a;
		}

		head = head->next;
		if (head) {
			if (head->prev && head->prev != &dentry)
				CSR1212_FREE(head->prev);
			head->prev = NULL;
		} else if (tail != &dentry) {
			CSR1212_FREE(tail);
		}
	}
}

void csr1212_destroy_csr(struct csr1212_csr *csr)
{
	struct csr1212_csr_rom_cache *c, *oc;
	struct csr1212_cache_region *cr, *ocr;

	csr1212_release_keyval(csr->root_kv);

	c = csr->cache_head;
	while (c) {
		oc = c;
		cr = c->filled_head;
		while (cr) {
			ocr = cr;
			cr = cr->next;
			CSR1212_FREE(ocr);
		}
		c = c->next;
		CSR1212_FREE(oc);
	}

	CSR1212_FREE(csr);
}




static int csr1212_append_new_cache(struct csr1212_csr *csr, size_t romsize)
{
	struct csr1212_csr_rom_cache *cache;
	u64 csr_addr;

	BUG_ON(!csr || !csr->ops || !csr->ops->allocate_addr_range ||
	       !csr->ops->release_addr || csr->max_rom < 1);

	
	romsize = (romsize + (csr->max_rom - 1)) & ~(csr->max_rom - 1);

	csr_addr = csr->ops->allocate_addr_range(romsize, csr->max_rom,
						 csr->private);
	if (csr_addr == CSR1212_INVALID_ADDR_SPACE)
		return -ENOMEM;

	if (csr_addr < CSR1212_REGISTER_SPACE_BASE) {
		
		csr->ops->release_addr(csr_addr, csr->private);
		return -ENOMEM;
	}

	cache = csr1212_rom_cache_malloc(csr_addr - CSR1212_REGISTER_SPACE_BASE,
					 romsize);
	if (!cache) {
		csr->ops->release_addr(csr_addr, csr->private);
		return -ENOMEM;
	}

	cache->ext_rom = csr1212_new_keyval(CSR1212_KV_TYPE_LEAF,
					    CSR1212_KV_ID_EXTENDED_ROM);
	if (!cache->ext_rom) {
		csr->ops->release_addr(csr_addr, csr->private);
		CSR1212_FREE(cache);
		return -ENOMEM;
	}

	if (csr1212_attach_keyval_to_directory(csr->root_kv, cache->ext_rom) !=
	    CSR1212_SUCCESS) {
		csr1212_release_keyval(cache->ext_rom);
		csr->ops->release_addr(csr_addr, csr->private);
		CSR1212_FREE(cache);
		return -ENOMEM;
	}
	cache->ext_rom->offset = csr_addr - CSR1212_REGISTER_SPACE_BASE;
	cache->ext_rom->value.leaf.len = -1;
	cache->ext_rom->value.leaf.data = cache->data;

	
	cache->prev = csr->cache_tail;
	csr->cache_tail->next = cache;
	csr->cache_tail = cache;
	return CSR1212_SUCCESS;
}

static void csr1212_remove_cache(struct csr1212_csr *csr,
				 struct csr1212_csr_rom_cache *cache)
{
	if (csr->cache_head == cache)
		csr->cache_head = cache->next;
	if (csr->cache_tail == cache)
		csr->cache_tail = cache->prev;

	if (cache->prev)
		cache->prev->next = cache->next;
	if (cache->next)
		cache->next->prev = cache->prev;

	if (cache->ext_rom) {
		csr1212_detach_keyval_from_directory(csr->root_kv,
						     cache->ext_rom);
		csr1212_release_keyval(cache->ext_rom);
	}

	CSR1212_FREE(cache);
}

static int csr1212_generate_layout_subdir(struct csr1212_keyval *dir,
					  struct csr1212_keyval **layout_tail)
{
	struct csr1212_dentry *dentry;
	struct csr1212_keyval *dkv;
	struct csr1212_keyval *last_extkey_spec = NULL;
	struct csr1212_keyval *last_extkey = NULL;
	int num_entries = 0;

	for (dentry = dir->value.directory.dentries_head; dentry;
	     dentry = dentry->next) {
		for (dkv = dentry->kv; dkv; dkv = dkv->associate) {
			
			if (dkv->key.id ==
			    CSR1212_KV_ID_EXTENDED_KEY_SPECIFIER_ID) {
				if (last_extkey_spec == NULL)
					last_extkey_spec = dkv;
				else if (dkv->value.immediate !=
					 last_extkey_spec->value.immediate)
					last_extkey_spec = dkv;
				else
					continue;
			
			} else if (dkv->key.id == CSR1212_KV_ID_EXTENDED_KEY) {
				if (last_extkey == NULL)
					last_extkey = dkv;
				else if (dkv->value.immediate !=
					 last_extkey->value.immediate)
					last_extkey = dkv;
				else
					continue;
			}

			num_entries += 1;

			switch (dkv->key.type) {
			default:
			case CSR1212_KV_TYPE_IMMEDIATE:
			case CSR1212_KV_TYPE_CSR_OFFSET:
				break;
			case CSR1212_KV_TYPE_LEAF:
			case CSR1212_KV_TYPE_DIRECTORY:
				
				if (dkv->prev && (dkv->prev->next == dkv))
					dkv->prev->next = dkv->next;
				if (dkv->next && (dkv->next->prev == dkv))
					dkv->next->prev = dkv->prev;
				
				

				
				if (dkv->key.id == CSR1212_KV_ID_EXTENDED_ROM) {
					dkv->value.leaf.len = -1;
					
					break;
				}

				
				dkv->next = NULL;
				dkv->prev = *layout_tail;
				(*layout_tail)->next = dkv;
				*layout_tail = dkv;
				break;
			}
		}
	}
	return num_entries;
}

static size_t csr1212_generate_layout_order(struct csr1212_keyval *kv)
{
	struct csr1212_keyval *ltail = kv;
	size_t agg_size = 0;

	while (kv) {
		switch (kv->key.type) {
		case CSR1212_KV_TYPE_LEAF:
			
			agg_size += kv->value.leaf.len + 1;
			break;

		case CSR1212_KV_TYPE_DIRECTORY:
			kv->value.directory.len =
				csr1212_generate_layout_subdir(kv, &ltail);
			
			agg_size += kv->value.directory.len + 1;
			break;
		}
		kv = kv->next;
	}
	return quads_to_bytes(agg_size);
}

static struct csr1212_keyval *
csr1212_generate_positions(struct csr1212_csr_rom_cache *cache,
			   struct csr1212_keyval *start_kv, int start_pos)
{
	struct csr1212_keyval *kv = start_kv;
	struct csr1212_keyval *okv = start_kv;
	int pos = start_pos;
	int kv_len = 0, okv_len = 0;

	cache->layout_head = kv;

	while (kv && pos < cache->size) {
		
		if (kv->key.id != CSR1212_KV_ID_EXTENDED_ROM)
			kv->offset = cache->offset + pos;

		switch (kv->key.type) {
		case CSR1212_KV_TYPE_LEAF:
			kv_len = kv->value.leaf.len;
			break;

		case CSR1212_KV_TYPE_DIRECTORY:
			kv_len = kv->value.directory.len;
			break;

		default:
			
			WARN_ON(1);
			break;
		}

		pos += quads_to_bytes(kv_len + 1);

		if (pos <= cache->size) {
			okv = kv;
			okv_len = kv_len;
			kv = kv->next;
		}
	}

	cache->layout_tail = okv;
	cache->len = okv->offset - cache->offset + quads_to_bytes(okv_len + 1);

	return kv;
}

#define CSR1212_KV_KEY_SHIFT		24
#define CSR1212_KV_KEY_TYPE_SHIFT	6
#define CSR1212_KV_KEY_ID_MASK		0x3f
#define CSR1212_KV_KEY_TYPE_MASK	0x3	

static void
csr1212_generate_tree_subdir(struct csr1212_keyval *dir, u32 *data_buffer)
{
	struct csr1212_dentry *dentry;
	struct csr1212_keyval *last_extkey_spec = NULL;
	struct csr1212_keyval *last_extkey = NULL;
	int index = 0;

	for (dentry = dir->value.directory.dentries_head;
	     dentry;
	     dentry = dentry->next) {
		struct csr1212_keyval *a;

		for (a = dentry->kv; a; a = a->associate) {
			u32 value = 0;

			
			if (a->key.id ==
			    CSR1212_KV_ID_EXTENDED_KEY_SPECIFIER_ID) {
				if (last_extkey_spec == NULL)
					last_extkey_spec = a;
				else if (a->value.immediate !=
					 last_extkey_spec->value.immediate)
					last_extkey_spec = a;
				else
					continue;

			
			} else if (a->key.id == CSR1212_KV_ID_EXTENDED_KEY) {
				if (last_extkey == NULL)
					last_extkey = a;
				else if (a->value.immediate !=
					 last_extkey->value.immediate)
					last_extkey = a;
				else
					continue;
			}

			switch (a->key.type) {
			case CSR1212_KV_TYPE_IMMEDIATE:
				value = a->value.immediate;
				break;
			case CSR1212_KV_TYPE_CSR_OFFSET:
				value = a->value.csr_offset;
				break;
			case CSR1212_KV_TYPE_LEAF:
				value = a->offset;
				value -= dir->offset + quads_to_bytes(1+index);
				value = bytes_to_quads(value);
				break;
			case CSR1212_KV_TYPE_DIRECTORY:
				value = a->offset;
				value -= dir->offset + quads_to_bytes(1+index);
				value = bytes_to_quads(value);
				break;
			default:
				
				WARN_ON(1);
				break;
			}

			value |= (a->key.id & CSR1212_KV_KEY_ID_MASK) <<
				 CSR1212_KV_KEY_SHIFT;
			value |= (a->key.type & CSR1212_KV_KEY_TYPE_MASK) <<
				 (CSR1212_KV_KEY_SHIFT +
				  CSR1212_KV_KEY_TYPE_SHIFT);
			data_buffer[index] = cpu_to_be32(value);
			index++;
		}
	}
}

struct csr1212_keyval_img {
	u16 length;
	u16 crc;

	
	u32 data[0];	
};

static void csr1212_fill_cache(struct csr1212_csr_rom_cache *cache)
{
	struct csr1212_keyval *kv, *nkv;
	struct csr1212_keyval_img *kvi;

	for (kv = cache->layout_head;
	     kv != cache->layout_tail->next;
	     kv = nkv) {
		kvi = (struct csr1212_keyval_img *)(cache->data +
				bytes_to_quads(kv->offset - cache->offset));
		switch (kv->key.type) {
		default:
		case CSR1212_KV_TYPE_IMMEDIATE:
		case CSR1212_KV_TYPE_CSR_OFFSET:
			
			WARN_ON(1);
			break;

		case CSR1212_KV_TYPE_LEAF:
			
			if (kv->key.id != CSR1212_KV_ID_EXTENDED_ROM)
				memcpy(kvi->data, kv->value.leaf.data,
				       quads_to_bytes(kv->value.leaf.len));

			kvi->length = cpu_to_be16(kv->value.leaf.len);
			kvi->crc = csr1212_crc16(kvi->data, kv->value.leaf.len);
			break;

		case CSR1212_KV_TYPE_DIRECTORY:
			csr1212_generate_tree_subdir(kv, kvi->data);

			kvi->length = cpu_to_be16(kv->value.directory.len);
			kvi->crc = csr1212_crc16(kvi->data,
						 kv->value.directory.len);
			break;
		}

		nkv = kv->next;
		if (kv->prev)
			kv->prev->next = NULL;
		if (kv->next)
			kv->next->prev = NULL;
		kv->prev = NULL;
		kv->next = NULL;
	}
}


#define CSR1212_EXTENDED_ROM_SIZE (2048 - sizeof(struct csr1212_csr_rom_cache))

int csr1212_generate_csr_image(struct csr1212_csr *csr)
{
	struct csr1212_bus_info_block_img *bi;
	struct csr1212_csr_rom_cache *cache;
	struct csr1212_keyval *kv;
	size_t agg_size;
	int ret;
	int init_offset;

	BUG_ON(!csr);

	cache = csr->cache_head;

	bi = (struct csr1212_bus_info_block_img*)cache->data;

	bi->length = bytes_to_quads(csr->bus_info_len) - 1;
	bi->crc_length = bi->length;
	bi->crc = csr1212_crc16(bi->data, bi->crc_length);

	csr->root_kv->next = NULL;
	csr->root_kv->prev = NULL;

	agg_size = csr1212_generate_layout_order(csr->root_kv);

	init_offset = csr->bus_info_len;

	for (kv = csr->root_kv, cache = csr->cache_head;
	     kv;
	     cache = cache->next) {
		if (!cache) {
			
			int est_c = agg_size / (CSR1212_EXTENDED_ROM_SIZE -
						(2 * sizeof(u32))) + 1;

			
			for (; est_c; est_c--) {
				ret = csr1212_append_new_cache(csr,
						CSR1212_EXTENDED_ROM_SIZE);
				if (ret != CSR1212_SUCCESS)
					return ret;
			}
			
			agg_size = csr1212_generate_layout_order(csr->root_kv);
			kv = csr->root_kv;
			cache = csr->cache_head;
			init_offset = csr->bus_info_len;
		}
		kv = csr1212_generate_positions(cache, kv, init_offset);
		agg_size -= cache->len;
		init_offset = sizeof(u32);
	}

	
	while (cache) {
		struct csr1212_csr_rom_cache *oc = cache;

		cache = cache->next;
		csr1212_remove_cache(csr, oc);
	}

	
	for (cache = csr->cache_tail; cache; cache = cache->prev) {
		
		if (cache->ext_rom) {
			int leaf_size;

			
			BUG_ON(csr->max_rom < 1);
			leaf_size = (cache->len + (csr->max_rom - 1)) &
				~(csr->max_rom - 1);

			
			memset(cache->data + bytes_to_quads(cache->len), 0x00,
			       leaf_size - cache->len);

			
			leaf_size -= sizeof(u32);

			
			cache->ext_rom->value.leaf.len =
				bytes_to_quads(leaf_size);
		} else {
			
			memset(cache->data + bytes_to_quads(cache->len), 0x00,
			       cache->size - cache->len);
		}

		
		csr1212_fill_cache(cache);

		if (cache != csr->cache_head) {
			
			struct csr1212_keyval_img *kvi =
				(struct csr1212_keyval_img*)cache->data;
			u16 len = bytes_to_quads(cache->len) - 1;

			kvi->length = cpu_to_be16(len);
			kvi->crc = csr1212_crc16(kvi->data, len);
		}
	}

	return CSR1212_SUCCESS;
}

int csr1212_read(struct csr1212_csr *csr, u32 offset, void *buffer, u32 len)
{
	struct csr1212_csr_rom_cache *cache;

	for (cache = csr->cache_head; cache; cache = cache->next)
		if (offset >= cache->offset &&
		    (offset + len) <= (cache->offset + cache->size)) {
			memcpy(buffer, &cache->data[
					bytes_to_quads(offset - cache->offset)],
			       len);
			return CSR1212_SUCCESS;
		}

	return -ENOENT;
}


static void
csr1212_check_crc(const u32 *buffer, size_t length, u16 crc, __be32 *guid)
{
	static u64 last_bad_eui64;
	u64 eui64 = ((u64)be32_to_cpu(guid[0]) << 32) | be32_to_cpu(guid[1]);

	if (csr1212_crc16(buffer, length) == crc ||
	    csr1212_msft_crc16(buffer, length) == crc ||
	    eui64 == last_bad_eui64)
		return;

	printk(KERN_DEBUG "ieee1394: config ROM CRC error\n");
	last_bad_eui64 = eui64;
}



static int csr1212_parse_bus_info_block(struct csr1212_csr *csr)
{
	struct csr1212_bus_info_block_img *bi;
	struct csr1212_cache_region *cr;
	int i;
	int ret;

	for (i = 0; i < csr->bus_info_len; i += sizeof(u32)) {
		ret = csr->ops->bus_read(csr, CSR1212_CONFIG_ROM_SPACE_BASE + i,
				&csr->cache_head->data[bytes_to_quads(i)],
				csr->private);
		if (ret != CSR1212_SUCCESS)
			return ret;

		
		if (i == 0 &&
		    be32_to_cpu(csr->cache_head->data[0]) >> 24 !=
		    bytes_to_quads(csr->bus_info_len) - 1)
			return -EINVAL;
	}

	bi = (struct csr1212_bus_info_block_img*)csr->cache_head->data;
	csr->crc_len = quads_to_bytes(bi->crc_length);

	
	for (i = csr->bus_info_len; i <= csr->crc_len; i += sizeof(u32)) {
		ret = csr->ops->bus_read(csr, CSR1212_CONFIG_ROM_SPACE_BASE + i,
				&csr->cache_head->data[bytes_to_quads(i)],
				csr->private);
		if (ret != CSR1212_SUCCESS)
			return ret;
	}

	csr1212_check_crc(bi->data, bi->crc_length, bi->crc,
			  &csr->bus_info_data[3]);

	cr = CSR1212_MALLOC(sizeof(*cr));
	if (!cr)
		return -ENOMEM;

	cr->next = NULL;
	cr->prev = NULL;
	cr->offset_start = 0;
	cr->offset_end = csr->crc_len + 4;

	csr->cache_head->filled_head = cr;
	csr->cache_head->filled_tail = cr;

	return CSR1212_SUCCESS;
}

#define CSR1212_KV_KEY(q)	(be32_to_cpu(q) >> CSR1212_KV_KEY_SHIFT)
#define CSR1212_KV_KEY_TYPE(q)	(CSR1212_KV_KEY(q) >> CSR1212_KV_KEY_TYPE_SHIFT)
#define CSR1212_KV_KEY_ID(q)	(CSR1212_KV_KEY(q) & CSR1212_KV_KEY_ID_MASK)
#define CSR1212_KV_VAL_MASK	0xffffff
#define CSR1212_KV_VAL(q)	(be32_to_cpu(q) & CSR1212_KV_VAL_MASK)

static int
csr1212_parse_dir_entry(struct csr1212_keyval *dir, u32 ki, u32 kv_pos)
{
	int ret = CSR1212_SUCCESS;
	struct csr1212_keyval *k = NULL;
	u32 offset;
	bool keep_keyval = true;

	switch (CSR1212_KV_KEY_TYPE(ki)) {
	case CSR1212_KV_TYPE_IMMEDIATE:
		k = csr1212_new_immediate(CSR1212_KV_KEY_ID(ki),
					  CSR1212_KV_VAL(ki));
		if (!k) {
			ret = -ENOMEM;
			goto out;
		}
		
		keep_keyval = false;
		break;

	case CSR1212_KV_TYPE_CSR_OFFSET:
		k = csr1212_new_csr_offset(CSR1212_KV_KEY_ID(ki),
					   CSR1212_KV_VAL(ki));
		if (!k) {
			ret = -ENOMEM;
			goto out;
		}
		
		keep_keyval = false;
		break;

	default:
		
		offset = quads_to_bytes(CSR1212_KV_VAL(ki)) + kv_pos;
		if (offset == kv_pos) {
			
			ret = -EIO;
			goto out;
		}

		k = csr1212_find_keyval_offset(dir, offset);

		if (k)
			break;		

		if (CSR1212_KV_KEY_TYPE(ki) == CSR1212_KV_TYPE_DIRECTORY)
			k = csr1212_new_directory(CSR1212_KV_KEY_ID(ki));
		else
			k = csr1212_new_leaf(CSR1212_KV_KEY_ID(ki), NULL, 0);

		if (!k) {
			ret = -ENOMEM;
			goto out;
		}
		
		keep_keyval = false;
		
		k->valid = 0;
		k->offset = offset;

		k->prev = dir;
		k->next = dir->next;
		dir->next->prev = k;
		dir->next = k;
	}
	ret = __csr1212_attach_keyval_to_directory(dir, k, keep_keyval);
out:
	if (ret != CSR1212_SUCCESS && k != NULL)
		free_keyval(k);
	return ret;
}

int csr1212_parse_keyval(struct csr1212_keyval *kv,
			 struct csr1212_csr_rom_cache *cache)
{
	struct csr1212_keyval_img *kvi;
	int i;
	int ret = CSR1212_SUCCESS;
	int kvi_len;

	kvi = (struct csr1212_keyval_img*)
		&cache->data[bytes_to_quads(kv->offset - cache->offset)];
	kvi_len = be16_to_cpu(kvi->length);

	
	csr1212_check_crc(kvi->data, kvi_len, kvi->crc, &cache->data[3]);

	switch (kv->key.type) {
	case CSR1212_KV_TYPE_DIRECTORY:
		for (i = 0; i < kvi_len; i++) {
			u32 ki = kvi->data[i];

			
			if (ki == 0x0)
				continue;
			ret = csr1212_parse_dir_entry(kv, ki,
					kv->offset + quads_to_bytes(i + 1));
		}
		kv->value.directory.len = kvi_len;
		break;

	case CSR1212_KV_TYPE_LEAF:
		if (kv->key.id != CSR1212_KV_ID_EXTENDED_ROM) {
			size_t size = quads_to_bytes(kvi_len);

			kv->value.leaf.data = CSR1212_MALLOC(size);
			if (!kv->value.leaf.data) {
				ret = -ENOMEM;
				goto out;
			}

			kv->value.leaf.len = kvi_len;
			memcpy(kv->value.leaf.data, kvi->data, size);
		}
		break;
	}

	kv->valid = 1;
out:
	return ret;
}

static int
csr1212_read_keyval(struct csr1212_csr *csr, struct csr1212_keyval *kv)
{
	struct csr1212_cache_region *cr, *ncr, *newcr = NULL;
	struct csr1212_keyval_img *kvi = NULL;
	struct csr1212_csr_rom_cache *cache;
	int cache_index;
	u64 addr;
	u32 *cache_ptr;
	u16 kv_len = 0;

	BUG_ON(!csr || !kv || csr->max_rom < 1);

	
	for (cache = csr->cache_head; cache; cache = cache->next)
		if (kv->offset >= cache->offset &&
		    kv->offset < (cache->offset + cache->size))
			break;

	if (!cache) {
		u32 q, cache_size;

		
		if (kv->key.id != CSR1212_KV_ID_EXTENDED_ROM)
			return -EINVAL;

		if (csr->ops->bus_read(csr,
				       CSR1212_REGISTER_SPACE_BASE + kv->offset,
				       &q, csr->private))
			return -EIO;

		kv->value.leaf.len = be32_to_cpu(q) >> 16;

		cache_size = (quads_to_bytes(kv->value.leaf.len + 1) +
			      (csr->max_rom - 1)) & ~(csr->max_rom - 1);

		cache = csr1212_rom_cache_malloc(kv->offset, cache_size);
		if (!cache)
			return -ENOMEM;

		kv->value.leaf.data = &cache->data[1];
		csr->cache_tail->next = cache;
		cache->prev = csr->cache_tail;
		cache->next = NULL;
		csr->cache_tail = cache;
		cache->filled_head =
			CSR1212_MALLOC(sizeof(*cache->filled_head));
		if (!cache->filled_head)
			return -ENOMEM;

		cache->filled_head->offset_start = 0;
		cache->filled_head->offset_end = sizeof(u32);
		cache->filled_tail = cache->filled_head;
		cache->filled_head->next = NULL;
		cache->filled_head->prev = NULL;
		cache->data[0] = q;

		
		return csr1212_parse_keyval(kv, cache);
	}

	cache_index = kv->offset - cache->offset;

	
	for (cr = cache->filled_head; cr; cr = cr->next) {
		if (cache_index < cr->offset_start) {
			newcr = CSR1212_MALLOC(sizeof(*newcr));
			if (!newcr)
				return -ENOMEM;

			newcr->offset_start = cache_index & ~(csr->max_rom - 1);
			newcr->offset_end = newcr->offset_start;
			newcr->next = cr;
			newcr->prev = cr->prev;
			cr->prev = newcr;
			cr = newcr;
			break;
		} else if ((cache_index >= cr->offset_start) &&
			   (cache_index < cr->offset_end)) {
			kvi = (struct csr1212_keyval_img*)
				(&cache->data[bytes_to_quads(cache_index)]);
			kv_len = quads_to_bytes(be16_to_cpu(kvi->length) + 1);
			break;
		} else if (cache_index == cr->offset_end) {
			break;
		}
	}

	if (!cr) {
		cr = cache->filled_tail;
		newcr = CSR1212_MALLOC(sizeof(*newcr));
		if (!newcr)
			return -ENOMEM;

		newcr->offset_start = cache_index & ~(csr->max_rom - 1);
		newcr->offset_end = newcr->offset_start;
		newcr->prev = cr;
		newcr->next = cr->next;
		cr->next = newcr;
		cr = newcr;
		cache->filled_tail = newcr;
	}

	while(!kvi || cr->offset_end < cache_index + kv_len) {
		cache_ptr = &cache->data[bytes_to_quads(cr->offset_end &
							~(csr->max_rom - 1))];

		addr = (CSR1212_CSR_ARCH_REG_SPACE_BASE + cache->offset +
			cr->offset_end) & ~(csr->max_rom - 1);

		if (csr->ops->bus_read(csr, addr, cache_ptr, csr->private))
			return -EIO;

		cr->offset_end += csr->max_rom - (cr->offset_end &
						  (csr->max_rom - 1));

		if (!kvi && (cr->offset_end > cache_index)) {
			kvi = (struct csr1212_keyval_img*)
				(&cache->data[bytes_to_quads(cache_index)]);
			kv_len = quads_to_bytes(be16_to_cpu(kvi->length) + 1);
		}

		if ((kv_len + (kv->offset - cache->offset)) > cache->size) {
			
			return -EIO;
		}

		ncr = cr->next;

		if (ncr && (cr->offset_end >= ncr->offset_start)) {
			
			ncr->offset_start = cr->offset_start;

			if (cr->prev)
				cr->prev->next = cr->next;
			ncr->prev = cr->prev;
			if (cache->filled_head == cr)
				cache->filled_head = ncr;
			CSR1212_FREE(cr);
			cr = ncr;
		}
	}

	return csr1212_parse_keyval(kv, cache);
}

struct csr1212_keyval *
csr1212_get_keyval(struct csr1212_csr *csr, struct csr1212_keyval *kv)
{
	if (!kv)
		return NULL;
	if (!kv->valid)
		if (csr1212_read_keyval(csr, kv) != CSR1212_SUCCESS)
			return NULL;
	return kv;
}

int csr1212_parse_csr(struct csr1212_csr *csr)
{
	struct csr1212_dentry *dentry;
	int ret;

	BUG_ON(!csr || !csr->ops || !csr->ops->bus_read);

	ret = csr1212_parse_bus_info_block(csr);
	if (ret != CSR1212_SUCCESS)
		return ret;

	
	csr->max_rom = 4;

	csr->cache_head->layout_head = csr->root_kv;
	csr->cache_head->layout_tail = csr->root_kv;

	csr->root_kv->offset = (CSR1212_CONFIG_ROM_SPACE_BASE & 0xffff) +
		csr->bus_info_len;

	csr->root_kv->valid = 0;
	csr->root_kv->next = csr->root_kv;
	csr->root_kv->prev = csr->root_kv;
	ret = csr1212_read_keyval(csr, csr->root_kv);
	if (ret != CSR1212_SUCCESS)
		return ret;

	
	for (dentry = csr->root_kv->value.directory.dentries_head;
	     dentry; dentry = dentry->next) {
		if (dentry->kv->key.id == CSR1212_KV_ID_EXTENDED_ROM &&
			!dentry->kv->valid) {
			ret = csr1212_read_keyval(csr, dentry->kv);
			if (ret != CSR1212_SUCCESS)
				return ret;
		}
	}

	return CSR1212_SUCCESS;
}
