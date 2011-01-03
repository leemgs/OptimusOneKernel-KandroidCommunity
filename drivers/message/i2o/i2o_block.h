

#ifndef I2O_BLOCK_OSM_H
#define I2O_BLOCK_OSM_H

#define I2O_BLOCK_RETRY_TIME HZ/4
#define I2O_BLOCK_MAX_OPEN_REQUESTS 50


#define I2O_BLOCK_REQ_MEMPOOL_SIZE		32

#define KERNEL_SECTOR_SHIFT 9
#define KERNEL_SECTOR_SIZE (1 << KERNEL_SECTOR_SHIFT)


struct i2o_block_mempool {
	struct kmem_cache *slab;
	mempool_t *pool;
};


struct i2o_block_device {
	struct i2o_device *i2o_dev;	
	struct gendisk *gd;
	spinlock_t lock;	
	struct list_head open_queue;	
	unsigned int open_queue_depth;	

	int rcache;		
	int wcache;		
	int flags;
	u16 power;		
	int media_change_flag;	
};


struct i2o_block_request {
	struct list_head queue;
	struct request *req;	
	struct i2o_block_device *i2o_blk_dev;	
	struct device *dev;	
	int sg_nents;		
	struct scatterlist sg_table[I2O_MAX_PHYS_SEGMENTS];	
};


struct i2o_block_delayed_request {
	struct delayed_work work;
	struct request_queue *queue;
};

#endif
