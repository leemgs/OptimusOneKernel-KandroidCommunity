

#ifndef __CSR1212_H__
#define __CSR1212_H__

#include <linux/types.h>
#include <linux/slab.h>
#include <asm/atomic.h>

#define CSR1212_MALLOC(size)	kmalloc((size), GFP_KERNEL)
#define CSR1212_FREE(ptr)	kfree(ptr)

#define CSR1212_SUCCESS (0)



#define CSR1212_KV_TYPE_IMMEDIATE		0
#define CSR1212_KV_TYPE_CSR_OFFSET		1
#define CSR1212_KV_TYPE_LEAF			2
#define CSR1212_KV_TYPE_DIRECTORY		3



#define CSR1212_KV_ID_DESCRIPTOR		0x01
#define CSR1212_KV_ID_BUS_DEPENDENT_INFO	0x02
#define CSR1212_KV_ID_VENDOR			0x03
#define CSR1212_KV_ID_HARDWARE_VERSION		0x04
#define CSR1212_KV_ID_MODULE			0x07
#define CSR1212_KV_ID_NODE_CAPABILITIES		0x0C
#define CSR1212_KV_ID_EUI_64			0x0D
#define CSR1212_KV_ID_UNIT			0x11
#define CSR1212_KV_ID_SPECIFIER_ID		0x12
#define CSR1212_KV_ID_VERSION			0x13
#define CSR1212_KV_ID_DEPENDENT_INFO		0x14
#define CSR1212_KV_ID_UNIT_LOCATION		0x15
#define CSR1212_KV_ID_MODEL			0x17
#define CSR1212_KV_ID_INSTANCE			0x18
#define CSR1212_KV_ID_KEYWORD			0x19
#define CSR1212_KV_ID_FEATURE			0x1A
#define CSR1212_KV_ID_EXTENDED_ROM		0x1B
#define CSR1212_KV_ID_EXTENDED_KEY_SPECIFIER_ID	0x1C
#define CSR1212_KV_ID_EXTENDED_KEY		0x1D
#define CSR1212_KV_ID_EXTENDED_DATA		0x1E
#define CSR1212_KV_ID_MODIFIABLE_DESCRIPTOR	0x1F
#define CSR1212_KV_ID_DIRECTORY_ID		0x20
#define CSR1212_KV_ID_REVISION			0x21



#define CSR1212_ALL_SPACE_BASE			(0x000000000000ULL)
#define CSR1212_ALL_SPACE_SIZE			(1ULL << 48)
#define CSR1212_ALL_SPACE_END			(CSR1212_ALL_SPACE_BASE + CSR1212_ALL_SPACE_SIZE)

#define  CSR1212_MEMORY_SPACE_BASE		(0x000000000000ULL)
#define  CSR1212_MEMORY_SPACE_SIZE		((256ULL * (1ULL << 40)) - (512ULL * (1ULL << 20)))
#define  CSR1212_MEMORY_SPACE_END		(CSR1212_MEMORY_SPACE_BASE + CSR1212_MEMORY_SPACE_SIZE)

#define  CSR1212_PRIVATE_SPACE_BASE		(0xffffe0000000ULL)
#define  CSR1212_PRIVATE_SPACE_SIZE		(256ULL * (1ULL << 20))
#define  CSR1212_PRIVATE_SPACE_END		(CSR1212_PRIVATE_SPACE_BASE + CSR1212_PRIVATE_SPACE_SIZE)

#define  CSR1212_REGISTER_SPACE_BASE		(0xfffff0000000ULL)
#define  CSR1212_REGISTER_SPACE_SIZE		(256ULL * (1ULL << 20))
#define  CSR1212_REGISTER_SPACE_END		(CSR1212_REGISTER_SPACE_BASE + CSR1212_REGISTER_SPACE_SIZE)

#define  CSR1212_CSR_ARCH_REG_SPACE_BASE	(0xfffff0000000ULL)
#define  CSR1212_CSR_ARCH_REG_SPACE_SIZE	(512)
#define  CSR1212_CSR_ARCH_REG_SPACE_END		(CSR1212_CSR_ARCH_REG_SPACE_BASE + CSR1212_CSR_ARCH_REG_SPACE_SIZE)
#define  CSR1212_CSR_ARCH_REG_SPACE_OFFSET	(CSR1212_CSR_ARCH_REG_SPACE_BASE - CSR1212_REGISTER_SPACE_BASE)

#define  CSR1212_CSR_BUS_DEP_REG_SPACE_BASE	(0xfffff0000200ULL)
#define  CSR1212_CSR_BUS_DEP_REG_SPACE_SIZE	(512)
#define  CSR1212_CSR_BUS_DEP_REG_SPACE_END	(CSR1212_CSR_BUS_DEP_REG_SPACE_BASE + CSR1212_CSR_BUS_DEP_REG_SPACE_SIZE)
#define  CSR1212_CSR_BUS_DEP_REG_SPACE_OFFSET	(CSR1212_CSR_BUS_DEP_REG_SPACE_BASE - CSR1212_REGISTER_SPACE_BASE)

#define  CSR1212_CONFIG_ROM_SPACE_BASE		(0xfffff0000400ULL)
#define  CSR1212_CONFIG_ROM_SPACE_SIZE		(1024)
#define  CSR1212_CONFIG_ROM_SPACE_END		(CSR1212_CONFIG_ROM_SPACE_BASE + CSR1212_CONFIG_ROM_SPACE_SIZE)
#define  CSR1212_CONFIG_ROM_SPACE_OFFSET	(CSR1212_CONFIG_ROM_SPACE_BASE - CSR1212_REGISTER_SPACE_BASE)

#define  CSR1212_UNITS_SPACE_BASE		(0xfffff0000800ULL)
#define  CSR1212_UNITS_SPACE_SIZE		((256ULL * (1ULL << 20)) - 2048)
#define  CSR1212_UNITS_SPACE_END		(CSR1212_UNITS_SPACE_BASE + CSR1212_UNITS_SPACE_SIZE)
#define  CSR1212_UNITS_SPACE_OFFSET		(CSR1212_UNITS_SPACE_BASE - CSR1212_REGISTER_SPACE_BASE)

#define  CSR1212_INVALID_ADDR_SPACE		-1



struct csr1212_bus_info_block_img {
	u8 length;
	u8 crc_length;
	u16 crc;

	
	u32 data[0];	
};

struct csr1212_leaf {
	int len;
	u32 *data;
};

struct csr1212_dentry {
	struct csr1212_dentry *next, *prev;
	struct csr1212_keyval *kv;
};

struct csr1212_directory {
	int len;
	struct csr1212_dentry *dentries_head, *dentries_tail;
};

struct csr1212_keyval {
	struct {
		u8 type;
		u8 id;
	} key;
	union {
		u32 immediate;
		u32 csr_offset;
		struct csr1212_leaf leaf;
		struct csr1212_directory directory;
	} value;
	struct csr1212_keyval *associate;
	atomic_t refcnt;

	
	struct csr1212_keyval *next, *prev;	
	u32 offset;	
	u8 valid;	
};


struct csr1212_cache_region {
	struct csr1212_cache_region *next, *prev;
	u32 offset_start;	
	u32 offset_end;		
};

struct csr1212_csr_rom_cache {
	struct csr1212_csr_rom_cache *next, *prev;
	struct csr1212_cache_region *filled_head, *filled_tail;
	struct csr1212_keyval *layout_head, *layout_tail;
	size_t size;
	u32 offset;
	struct csr1212_keyval *ext_rom;
	size_t len;

	
	u32 data[0];	
};

struct csr1212_csr {
	size_t bus_info_len;	
	size_t crc_len;		
	__be32 *bus_info_data;	

	void *private;		
	struct csr1212_bus_ops *ops;

	struct csr1212_keyval *root_kv;

	int max_rom;		

	
	struct csr1212_csr_rom_cache *cache_head, *cache_tail;
};

struct csr1212_bus_ops {
	
	int (*bus_read) (struct csr1212_csr *csr, u64 addr,
			 void *buffer, void *private);

	
	u64 (*allocate_addr_range) (u64 size, u32 alignment, void *private);

	
	void (*release_addr) (u64 addr, void *private);
};



#define CSR1212_DESCRIPTOR_LEAF_TYPE_SHIFT 24
#define CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID_MASK 0xffffff
#define CSR1212_DESCRIPTOR_LEAF_OVERHEAD (1 * sizeof(u32))

#define CSR1212_DESCRIPTOR_LEAF_TYPE(kv) \
	(be32_to_cpu((kv)->value.leaf.data[0]) >> \
	 CSR1212_DESCRIPTOR_LEAF_TYPE_SHIFT)
#define CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) \
	(be32_to_cpu((kv)->value.leaf.data[0]) & \
	 CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID_MASK)



#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH_SHIFT 28
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH_MASK 0xf 
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET_SHIFT 16
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET_MASK 0xfff  
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE_MASK 0xffff
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_OVERHEAD (1 * sizeof(u32))

#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH(kv) \
	(be32_to_cpu((kv)->value.leaf.data[1]) >> \
	 CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH_SHIFT)
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET(kv) \
	((be32_to_cpu((kv)->value.leaf.data[1]) >> \
	  CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET_SHIFT) & \
	 CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET_MASK)
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE(kv) \
	(be32_to_cpu((kv)->value.leaf.data[1]) & \
	 CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE_MASK)
#define CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(kv) \
	(&((kv)->value.leaf.data[2]))



extern struct csr1212_csr *csr1212_create_csr(struct csr1212_bus_ops *ops,
					      size_t bus_info_size,
					      void *private);
extern void csr1212_init_local_csr(struct csr1212_csr *csr,
				   const u32 *bus_info_data, int max_rom);



extern void csr1212_destroy_csr(struct csr1212_csr *csr);



extern struct csr1212_keyval *csr1212_new_immediate(u8 key, u32 value);
extern struct csr1212_keyval *csr1212_new_directory(u8 key);
extern struct csr1212_keyval *csr1212_new_string_descriptor_leaf(const char *s);



extern void csr1212_associate_keyval(struct csr1212_keyval *kv,
				     struct csr1212_keyval *associate);



extern int csr1212_attach_keyval_to_directory(struct csr1212_keyval *dir,
					      struct csr1212_keyval *kv);
extern void csr1212_detach_keyval_from_directory(struct csr1212_keyval *dir,
						 struct csr1212_keyval *kv);



extern int csr1212_generate_csr_image(struct csr1212_csr *csr);



extern int csr1212_read(struct csr1212_csr *csr, u32 offset, void *buffer,
			u32 len);



extern int csr1212_parse_keyval(struct csr1212_keyval *kv,
				struct csr1212_csr_rom_cache *cache);
extern int csr1212_parse_csr(struct csr1212_csr *csr);



static inline struct csr1212_csr_rom_cache *
csr1212_rom_cache_malloc(u32 offset, size_t size)
{
	struct csr1212_csr_rom_cache *cache;

	cache = CSR1212_MALLOC(sizeof(*cache) + size);
	if (!cache)
		return NULL;

	cache->next = NULL;
	cache->prev = NULL;
	cache->filled_head = NULL;
	cache->filled_tail = NULL;
	cache->layout_head = NULL;
	cache->layout_tail = NULL;
	cache->offset = offset;
	cache->size = size;
	cache->ext_rom = NULL;

	return cache;
}



extern struct csr1212_keyval *
csr1212_get_keyval(struct csr1212_csr *csr, struct csr1212_keyval *kv);



static inline void csr1212_keep_keyval(struct csr1212_keyval *kv)
{
	atomic_inc(&kv->refcnt);
	smp_mb__after_atomic_inc();
}



extern void csr1212_release_keyval(struct csr1212_keyval *kv);



#define csr1212_for_each_dir_entry(_csr, _kv, _dir, _pos)		\
	for (csr1212_get_keyval((_csr), (_dir)),			\
	     _pos = (_dir)->value.directory.dentries_head,		\
	     _kv = (_pos) ? csr1212_get_keyval((_csr), _pos->kv) : NULL;\
	     (_kv) && (_pos);						\
	     (_kv->associate == NULL) ?					\
		     ((_pos = _pos->next), (_kv = (_pos) ?		\
				csr1212_get_keyval((_csr), _pos->kv) :	\
				NULL)) :				\
		     (_kv = csr1212_get_keyval((_csr), _kv->associate)))

#endif 
