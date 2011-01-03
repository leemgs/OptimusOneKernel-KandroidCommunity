#ifndef _XT_CONNMARK_H
#define _XT_CONNMARK_H

#include <linux/types.h>



struct xt_connmark_mtinfo1 {
	__u32 mark, mask;
	__u8 invert;
};

#endif 
