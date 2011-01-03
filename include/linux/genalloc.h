



struct gen_pool {
	rwlock_t lock;
	struct list_head chunks;	
	int min_alloc_order;		
};


struct gen_pool_chunk {
	spinlock_t lock;
	struct list_head next_chunk;	
	unsigned long start_addr;	
	unsigned long end_addr;		
	unsigned long bits[0];		
};

extern struct gen_pool *gen_pool_create(int, int);
extern int gen_pool_add(struct gen_pool *, unsigned long, size_t, int);
extern void gen_pool_destroy(struct gen_pool *);
extern unsigned long gen_pool_alloc(struct gen_pool *, size_t);
extern void gen_pool_free(struct gen_pool *, unsigned long, size_t);
