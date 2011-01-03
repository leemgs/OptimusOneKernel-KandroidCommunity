
#ifndef __CY8C8CTS_H__
#define __CY8C8CTS_H__



struct cy8c_ts_platform_data {
	int (*power_on)(int on);
	const char *ts_name;
	u32 dis_min_x; 
	u32 dis_max_x;
	u32 dis_min_y;
	u32 dis_max_y;
	u32 min_touch; 
	u32 max_touch;
	u32 min_tid; 
	u32 max_tid;
	u32 min_width;
	u32 max_width;
	u32 res_x; 
	u32 res_y;
	u32 swap_xy;
	u32 flags;
	u16 invert_x;
	u16 invert_y;
	u8 nfingers;
	u8 use_polling;
};

#endif
