
#ifndef __LINUX_LGE_BORADCAST_H
#define __LINUX_LGE_BORADCAST_H

#include <linux/types.h>
#include <asm/sizes.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>

#include <linux/broadcast/broadcast_tdmb_typedef.h>


#define LGE_BROADCAST_TDMB_IOCTL_MAGIC 'B'

#define LGE_BROADCAST_TDMB_IOCTL_ON \
	_IO(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 0)

#define LGE_BROADCAST_TDMB_IOCTL_OFF \
	_IO(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 1)

#define LGE_BROADCAST_TDMB_IOCTL_OPEN \
	_IO(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 2)

#define LGE_BROADCAST_TDMB_IOCTL_CLOSE \
	_IO(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 3)
	
#define LGE_BROADCAST_TDMB_IOCTL_TUNE \
	_IOW(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 4, int*)

#define LGE_BROADCAST_TDMB_IOCTL_SET_CH \
	_IOW(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 5, struct broadcast_tdmb_set_ch_info*)

#define LGE_BROADCAST_TDMB_IOCTL_RESYNC \
	_IOW(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 6, int32)

#define LGE_BROADCAST_TDMB_IOCTL_DETECT_SYNC \
	_IOR(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 7, int32*)

#define LGE_BROADCAST_TDMB_IOCTL_GET_SIG_INFO \
	_IOR(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 8, struct broadcast_tdmb_sig_info*)

#define LGE_BROADCAST_TDMB_IOCTL_GET_CH_INFO \
	_IOR(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 9, struct broadcast_tdmb_get_ch_info*)

#define LGE_BROADCAST_TDMB_IOCTL_RESET_CH \
	_IO(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 10)

#define LGE_BROADCAST_TDMB_IOCTL_USER_STOP \
	_IOW(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 11, int*)

#define LGE_BROADCAST_TDMB_IOCTL_GET_DMB_DATA \
	_IOW(LGE_BROADCAST_TDMB_IOCTL_MAGIC, 12, struct broadcast_tdmb_get_dmb_data_info*)

struct broadcast_tdmb_set_ch_info
{
	uint32	mode;
	uint32	ch_num;
	uint32	sub_ch_id;
};

struct broadcast_tdmb_sig_info
{
	uint32	dab_ok;
	uint32	msc_ber;
	uint32	sync_lock;
	uint32	afc_ok;
	uint32	cir;
	uint32	fic_ber;
	uint32	tp_lock;
	uint32	sch_ber;
	uint32	tp_err_cnt;
	uint32	va_ber;
	uint32	srv_state_flag;
};



struct broadcast_tdmb_get_ch_info
{
	unsigned char*  ch_buf;
	unsigned int	buf_len;
};

struct broadcast_tdmb_get_dmb_data_info
{
	uint8*	data_buf;
	uint32	buf_len;
	uint32  packet_cnt;
};

extern int broadcast_tdmb_drv_start(void);
extern int broadcast_tdmb_get_stop_mode(void);
#endif
