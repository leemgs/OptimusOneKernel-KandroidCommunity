

#ifndef __SEP_DRIVER_CONFIG_H__
#define __SEP_DRIVER_CONFIG_H__





#define SEP_DRIVER_POLLING_MODE                         1


#define SEP_DRIVER_RECONFIG_MESSAGE_AREA                0


#define SEP_DRIVER_ARM_DEBUG_MODE                       0




#define SEP_DRIVER_IN_FLAG                              0


#define SEP_DRIVER_OUT_FLAG                             1


#define SEP_DRIVER_ENTRIES_PER_TABLE_IN_SEP             8







#define SEP_DRIVER_MAX_MESSAGE_SIZE_IN_BYTES                  (8 * 1024)


#define SEP_DRIVER_MESSAGE_SHARED_AREA_SIZE_IN_BYTES          (8 * 1024)


#define SEP_DRIVER_STATIC_AREA_SIZE_IN_BYTES                  (4 * 1024)


#define SEP_DRIVER_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES        (12 * 1024)


#define SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES   (1024 * 5)



#define SEP_DRIVER_FLOW_DMA_TABLES_AREA_SIZE_IN_BYTES         (1024 * 4)


#define SEP_DRIVER_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES           100



#define SEP_DRIVER_MMMAP_AREA_SIZE                            (1024 * 24)





#define SEP_DRIVER_MESSAGE_AREA_OFFSET_IN_BYTES               0


#define SEP_DRIVER_STATIC_AREA_OFFSET_IN_BYTES \
		(SEP_DRIVER_MESSAGE_SHARED_AREA_SIZE_IN_BYTES)


#define SEP_DRIVER_DATA_POOL_AREA_OFFSET_IN_BYTES \
	(SEP_DRIVER_STATIC_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_STATIC_AREA_SIZE_IN_BYTES)


#define SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_OFFSET_IN_BYTES \
	(SEP_DRIVER_DATA_POOL_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES)


#define SEP_DRIVER_FLOW_DMA_TABLES_AREA_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES)


#define SEP_DRIVER_SYSTEM_DATA_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_FLOW_DMA_TABLES_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_FLOW_DMA_TABLES_AREA_SIZE_IN_BYTES)


#define SEP_DRIVER_SYSTEM_TIME_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYSTEM_DATA_MEMORY_OFFSET_IN_BYTES)




#define SEP_IO_MEM_REGION_START_ADDRESS                       0x80000000


#define SEP_IO_MEM_REGION_SIZE                                (2 * 0x100000)


#define SEP_DIRVER_IRQ_NUM                                    1


#define SEP_MAX_NUM_ADD_BUFFERS                               100


#define SEP_DRIVER_NUM_FLOWS                                  4


#define SEP_DRIVER_MAX_FLOW_NUM_ENTRIES_IN_TABLE              25


#define SEP_NUM_ENTRIES_OFFSET_IN_BITS                        24


#define SEP_INT_FLAG_OFFSET_IN_BITS                           31


#define SEP_TABLE_DATA_SIZE_MASK                              0xFFFFFF


#define SEP_NUM_ENTRIES_MASK                                  0x7F


#define SEP_FREE_FLOW_ID                                      0xFFFFFFFF


#define SEP_TEMP_FLOW_ID                   (SEP_DRIVER_NUM_FLOWS + 1)


#define SEP_MAX_ADD_MESSAGE_LENGTH_IN_BYTES                   (7 * 4)


#define SEP_MAX_VIRT_BUFFERS_CONCURRENT                       100


#define SEP_TIME_VAL_TOKEN                                    0x12345678


#define SEP_DEBUG_LEVEL_BASIC       0x1

#define SEP_DEBUG_LEVEL_EXTENDED    0x4




#define dbg(fmt, args...) \
do {\
	if (debug & SEP_DEBUG_LEVEL_BASIC) \
		printk(KERN_DEBUG fmt, ##args); \
} while(0);

#define edbg(fmt, args...) \
do { \
	if (debug & SEP_DEBUG_LEVEL_EXTENDED) \
		printk(KERN_DEBUG fmt, ##args); \
} while(0);



#endif
