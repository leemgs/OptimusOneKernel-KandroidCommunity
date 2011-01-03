#ifndef _CU3088_H
#define _CU3088_H


enum channel_types {
        
	channel_type_none,

        
	channel_type_parallel,

	
	channel_type_escon,

	
	channel_type_ficon,

	
	channel_type_osa2,

	
	channel_type_claw,

	
	channel_type_unknown,

	
	channel_type_unsupported,

	
	num_channel_types
};

extern const char *cu3088_type[num_channel_types];
extern int register_cu3088_discipline(struct ccwgroup_driver *);
extern void unregister_cu3088_discipline(struct ccwgroup_driver *);

#endif
