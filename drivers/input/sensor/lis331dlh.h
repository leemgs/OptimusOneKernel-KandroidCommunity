



#ifndef __LIS331DLH_H__
#define __LIS331DLH_H__

#include <linux/ioctl.h>  





#define LIS331DLH_IOCTL_BASE 'B'


#define LIS331DLH_IOCTL_SET_DELAY	_IOW(LIS331DLH_IOCTL_BASE, 0, int)
#define LIS331DLH_IOCTL_GET_DELAY	_IOWR(LIS331DLH_IOCTL_BASE, 1, int)
#define LIS331DLH_IOCTL_SET_ENABLE	_IOWR(LIS331DLH_IOCTL_BASE, 2, int)
#define LIS331DLH_IOCTL_GET_ENABLE	_IOWR(LIS331DLH_IOCTL_BASE, 3, int)
#define LIS331DLH_IOCTL_SET_G_RANGE	_IOWR(LIS331DLH_IOCTL_BASE, 4, int)
#define LIS331DLH_IOCTL_READ_ACCEL_XYZ _IOWR(LIS331DLH_IOCTL_BASE, 5, int)


#define LIS331DLH_G_2G 0x00
#define LIS331DLH_G_4G 0x10
#define LIS331DLH_G_8G 0x30

#ifdef __KERNEL__
struct lis331dlh_platform_data {
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*lis_init)(void);
	void (*lis_exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);

};

typedef struct  {
		short x, 
			 y, 
			 z; 
} lis331dlh_acc_t;

#endif 

#endif  


