
#ifndef __WL1251_IO_H__
#define __WL1251_IO_H__

#include "wl1251.h"

#define HW_ACCESS_MEMORY_MAX_RANGE		0x1FFC0

#define HW_ACCESS_PART0_SIZE_ADDR           0x1FFC0
#define HW_ACCESS_PART0_START_ADDR          0x1FFC4
#define HW_ACCESS_PART1_SIZE_ADDR           0x1FFC8
#define HW_ACCESS_PART1_START_ADDR          0x1FFCC

#define HW_ACCESS_REGISTER_SIZE             4

#define HW_ACCESS_PRAM_MAX_RANGE		0x3c000

static inline u32 wl1251_read32(struct wl1251 *wl, int addr)
{
	u32 response;

	wl->if_ops->read(wl, addr, &response, sizeof(u32));

	return response;
}

static inline void wl1251_write32(struct wl1251 *wl, int addr, u32 val)
{
	wl->if_ops->write(wl, addr, &val, sizeof(u32));
}


void wl1251_mem_read(struct wl1251 *wl, int addr, void *buf, size_t len);
void wl1251_mem_write(struct wl1251 *wl, int addr, void *buf, size_t len);
u32 wl1251_mem_read32(struct wl1251 *wl, int addr);
void wl1251_mem_write32(struct wl1251 *wl, int addr, u32 val);

u32 wl1251_reg_read32(struct wl1251 *wl, int addr);
void wl1251_reg_write32(struct wl1251 *wl, int addr, u32 val);

void wl1251_set_partition(struct wl1251 *wl,
			  u32 part_start, u32 part_size,
			  u32 reg_start,  u32 reg_size);

#endif
