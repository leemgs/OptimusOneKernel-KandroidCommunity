

#ifndef __BFA_DEFS_TSENSOR_H__
#define __BFA_DEFS_TSENSOR_H__

#include <bfa_os_inc.h>
#include <defs/bfa_defs_types.h>


enum bfa_tsensor_status {
	BFA_TSENSOR_STATUS_UNKNOWN   = 1,   
	BFA_TSENSOR_STATUS_FAULTY    = 2,   
	BFA_TSENSOR_STATUS_BELOW_MIN = 3,   
	BFA_TSENSOR_STATUS_NOMINAL   = 4,   
	BFA_TSENSOR_STATUS_ABOVE_MAX = 5,   
};


struct bfa_tsensor_attr_s {
	enum bfa_tsensor_status status;	
	u32        	value;	
};

#endif 
