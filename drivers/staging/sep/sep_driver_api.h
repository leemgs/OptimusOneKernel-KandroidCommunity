

#ifndef __SEP_DRIVER_API_H__
#define __SEP_DRIVER_API_H__






#define SEP_IOC_MAGIC_NUMBER                           's'


#define SEP_IOCSENDSEPCOMMAND                 _IO(SEP_IOC_MAGIC_NUMBER , 0)


#define SEP_IOCSENDSEPRPLYCOMMAND             _IO(SEP_IOC_MAGIC_NUMBER , 1)


#define SEP_IOCALLOCDATAPOLL                  _IO(SEP_IOC_MAGIC_NUMBER , 2)


#define SEP_IOCWRITEDATAPOLL                  _IO(SEP_IOC_MAGIC_NUMBER , 3)


#define SEP_IOCREADDATAPOLL                   _IO(SEP_IOC_MAGIC_NUMBER , 4)


#define SEP_IOCCREATESYMDMATABLE              _IO(SEP_IOC_MAGIC_NUMBER , 5)


#define SEP_IOCCREATEFLOWDMATABLE             _IO(SEP_IOC_MAGIC_NUMBER , 6)


#define SEP_IOCFREEDMATABLEDATA                _IO(SEP_IOC_MAGIC_NUMBER , 7)


#define SEP_IOCGETSTATICPOOLADDR               _IO(SEP_IOC_MAGIC_NUMBER , 8)


#define SEP_IOCSETFLOWID                       _IO(SEP_IOC_MAGIC_NUMBER , 9)


#define SEP_IOCADDFLOWTABLE                    _IO(SEP_IOC_MAGIC_NUMBER , 10)


#define SEP_IOCADDFLOWMESSAGE                  _IO(SEP_IOC_MAGIC_NUMBER , 11)


#define SEP_IOCSEPSTART                        _IO(SEP_IOC_MAGIC_NUMBER , 12)


#define SEP_IOCSEPINIT                         _IO(SEP_IOC_MAGIC_NUMBER , 13)


#define SEP_IOCENDTRANSACTION                  _IO(SEP_IOC_MAGIC_NUMBER , 15)


#define SEP_IOCREALLOCCACHERES                 _IO(SEP_IOC_MAGIC_NUMBER , 16)


#define SEP_IOCGETMAPPEDADDROFFSET             _IO(SEP_IOC_MAGIC_NUMBER , 17)


#define SEP_IOCGETIME                          _IO(SEP_IOC_MAGIC_NUMBER , 19)




struct sep_driver_init_t {
	
	unsigned long message_addr;

	
	unsigned long message_size_in_words;

};



struct sep_driver_realloc_cache_resident_t {
	
	u64 new_cache_addr;
	
	u64 new_resident_addr;
	
	u64  new_shared_area_addr;
	
	u64 new_base_addr;
};

struct sep_driver_alloc_t {
	
	unsigned long offset;

	
	unsigned long phys_address;

	
	unsigned long num_bytes;
};


struct sep_driver_write_t {
	
	unsigned long app_address;

	
	unsigned long datapool_address;

	
	unsigned long num_bytes;
};


struct sep_driver_read_t {
	
	unsigned long app_address;

	
	unsigned long datapool_address;

	
	unsigned long num_bytes;
};


struct sep_driver_build_sync_table_t {
	
	unsigned long app_in_address;

	
	unsigned long data_in_size;

	
	unsigned long app_out_address;

	
	unsigned long block_size;

	
	unsigned long in_table_address;

	
	unsigned long in_table_num_entries;

	
	unsigned long out_table_address;

	
	unsigned long out_table_num_entries;

	
	unsigned long table_data_size;

	
	bool isKernelVirtualAddress;

};


struct sep_driver_build_flow_table_t {
	
	unsigned long flow_type;

	
	unsigned long input_output_flag;

	
	unsigned long virt_buff_data_addr;

	
	unsigned long num_virtual_buffers;

	
	unsigned long first_table_addr;

	
	unsigned long first_table_num_entries;

	
	unsigned long first_table_data_size;

	
	bool isKernelVirtualAddress;
};


struct sep_driver_add_flow_table_t {
	
	unsigned long flow_id;

	
	unsigned long inputOutputFlag;

	
	unsigned long virt_buff_data_addr;

	
	unsigned long num_virtual_buffers;

	
	unsigned long first_table_addr;

	
	unsigned long first_table_num_entries;

	
	unsigned long first_table_data_size;

	
	bool isKernelVirtualAddress;

};


struct sep_driver_set_flow_id_t {
	
	unsigned long flow_id;
};



struct sep_driver_add_message_t {
	
	unsigned long flow_id;

	
	unsigned long message_size_in_bytes;

	
	unsigned long message_address;
};


struct sep_driver_static_pool_addr_t {
	
	unsigned long physical_static_address;

	
	unsigned long virtual_static_address;
};


struct sep_driver_get_mapped_offset_t {
	
	unsigned long physical_address;

	
	unsigned long offset;
};


struct sep_driver_get_time_t {
	
	unsigned long time_physical_address;

	
	unsigned long time_value;
};



struct sep_lli_entry_t {
	
	unsigned long physical_address;

	
	unsigned long block_size;
};


struct sep_lli_prepare_table_data_t {
	
	struct sep_lli_entry_t *lli_entry_ptr;

	
	struct sep_lli_entry_t *lli_array_ptr;

	
	int lli_array_size;

	
	int num_table_entries;

	
	int num_array_entries_processed;

	
	int lli_table_total_data_size;
};


struct sep_lli_table_t {
	
	unsigned long num_pages;
	
	struct page **table_page_array_ptr;

	
	struct sep_lli_entry_t lli_entries[SEP_DRIVER_MAX_FLOW_NUM_ENTRIES_IN_TABLE];
};



struct sep_flow_buffer_data {
	
	struct page **page_array_ptr;

	
	unsigned long num_pages;

	
	unsigned long last_page_array_flag;
};


struct sep_flow_context_t {
	
	struct work_struct flow_wq;

	
	unsigned long flow_id;

	
	unsigned long input_tables_flag;

	
	unsigned long output_tables_flag;

	
	struct sep_lli_entry_t first_input_table;

	
	struct sep_lli_entry_t first_output_table;

	
	struct sep_lli_entry_t last_input_table;

	
	struct sep_lli_entry_t last_output_table;

	
	struct sep_lli_entry_t input_tables_in_process;

	
	struct sep_lli_entry_t output_tables_in_process;

	
	unsigned long message_size_in_bytes;

	
	unsigned char message[SEP_MAX_ADD_MESSAGE_LENGTH_IN_BYTES];
};


#endif
