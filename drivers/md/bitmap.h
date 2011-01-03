
#ifndef BITMAP_H
#define BITMAP_H 1

#define BITMAP_MAJOR_LO 3

#define BITMAP_MAJOR_HI 4
#define	BITMAP_MAJOR_HOSTENDIAN 3

#define BITMAP_MINOR 39



#ifdef __KERNEL__

#define PAGE_BITS (PAGE_SIZE << 3)
#define PAGE_BIT_SHIFT (PAGE_SHIFT + 3)

typedef __u16 bitmap_counter_t;
#define COUNTER_BITS 16
#define COUNTER_BIT_SHIFT 4
#define COUNTER_BYTE_RATIO (COUNTER_BITS / 8)
#define COUNTER_BYTE_SHIFT (COUNTER_BIT_SHIFT - 3)

#define NEEDED_MASK ((bitmap_counter_t) (1 << (COUNTER_BITS - 1)))
#define RESYNC_MASK ((bitmap_counter_t) (1 << (COUNTER_BITS - 2)))
#define COUNTER_MAX ((bitmap_counter_t) RESYNC_MASK - 1)
#define NEEDED(x) (((bitmap_counter_t) x) & NEEDED_MASK)
#define RESYNC(x) (((bitmap_counter_t) x) & RESYNC_MASK)
#define COUNTER(x) (((bitmap_counter_t) x) & COUNTER_MAX)


#define PAGE_COUNTER_RATIO (PAGE_BITS / COUNTER_BITS)

#define PAGE_COUNTER_SHIFT (PAGE_BIT_SHIFT - COUNTER_BIT_SHIFT)

#define PAGE_COUNTER_MASK  (PAGE_COUNTER_RATIO - 1)

#define BITMAP_BLOCK_SIZE 512
#define BITMAP_BLOCK_SHIFT 9


#define CHUNK_BLOCK_RATIO(bitmap) ((bitmap)->chunksize >> BITMAP_BLOCK_SHIFT)
#define CHUNK_BLOCK_SHIFT(bitmap) ((bitmap)->chunkshift - BITMAP_BLOCK_SHIFT)
#define CHUNK_BLOCK_MASK(bitmap) (CHUNK_BLOCK_RATIO(bitmap) - 1)



#define PAGEPTR_BLOCK_RATIO(bitmap) \
			(CHUNK_BLOCK_RATIO(bitmap) << PAGE_COUNTER_SHIFT >> 1)
#define PAGEPTR_BLOCK_SHIFT(bitmap) \
			(CHUNK_BLOCK_SHIFT(bitmap) + PAGE_COUNTER_SHIFT - 1)
#define PAGEPTR_BLOCK_MASK(bitmap) (PAGEPTR_BLOCK_RATIO(bitmap) - 1)




#define CHUNK_BIT_OFFSET(chunk) ((chunk) + (sizeof(bitmap_super_t) << 3))

#endif



#define BITMAP_MAGIC 0x6d746962


enum bitmap_state {
	BITMAP_STALE  = 0x002,  
	BITMAP_WRITE_ERROR = 0x004, 
	BITMAP_HOSTENDIAN = 0x8000,
};


typedef struct bitmap_super_s {
	__le32 magic;        
	__le32 version;      
	__u8  uuid[16];      
	__le64 events;       
	__le64 events_cleared;
	__le64 sync_size;    
	__le32 state;        
	__le32 chunksize;    
	__le32 daemon_sleep; 
	__le32 write_behind; 

	__u8  pad[256 - 64]; 
} bitmap_super_t;



#ifdef __KERNEL__


struct bitmap_page {
	
	char *map;
	
	unsigned int hijacked:1;
	
	unsigned int  count:31;
};


struct page_list {
	struct list_head list;
	struct page *page;
};


struct bitmap {
	struct bitmap_page *bp;
	unsigned long pages; 
	unsigned long missing_pages; 

	mddev_t *mddev; 

	int counter_bits; 

	
	unsigned long chunksize;
	unsigned long chunkshift; 
	unsigned long chunks; 

	
	unsigned long syncchunk;

	__u64	events_cleared;
	int need_sync;

	
	spinlock_t lock;

	long offset; 
	struct file *file; 
	struct page *sb_page; 
	struct page **filemap; 
	unsigned long *filemap_attr; 
	unsigned long file_pages; 
	int last_page_size; 

	unsigned long flags;

	int allclean;

	unsigned long max_write_behind; 
	atomic_t behind_writes;

	
	unsigned long daemon_lastrun; 
	unsigned long daemon_sleep; 
	unsigned long last_end_sync; 

	atomic_t pending_writes; 
	wait_queue_head_t write_wait;
	wait_queue_head_t overflow_wait;

};




int  bitmap_create(mddev_t *mddev);
void bitmap_flush(mddev_t *mddev);
void bitmap_destroy(mddev_t *mddev);

void bitmap_print_sb(struct bitmap *bitmap);
void bitmap_update_sb(struct bitmap *bitmap);

int  bitmap_setallbits(struct bitmap *bitmap);
void bitmap_write_all(struct bitmap *bitmap);

void bitmap_dirty_bits(struct bitmap *bitmap, unsigned long s, unsigned long e);


int bitmap_startwrite(struct bitmap *bitmap, sector_t offset,
			unsigned long sectors, int behind);
void bitmap_endwrite(struct bitmap *bitmap, sector_t offset,
			unsigned long sectors, int success, int behind);
int bitmap_start_sync(struct bitmap *bitmap, sector_t offset, int *blocks, int degraded);
void bitmap_end_sync(struct bitmap *bitmap, sector_t offset, int *blocks, int aborted);
void bitmap_close_sync(struct bitmap *bitmap);
void bitmap_cond_end_sync(struct bitmap *bitmap, sector_t sector);

void bitmap_unplug(struct bitmap *bitmap);
void bitmap_daemon_work(mddev_t *mddev);
#endif

#endif
