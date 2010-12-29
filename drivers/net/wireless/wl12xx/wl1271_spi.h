

#ifndef __WL1271_SPI_H__
#define __WL1271_SPI_H__

#include "wl1271_reg.h"

#define HW_ACCESS_MEMORY_MAX_RANGE		0x1FFC0

#define HW_ACCESS_PART0_SIZE_ADDR           0x1FFC0
#define HW_ACCESS_PART0_START_ADDR          0x1FFC4
#define HW_ACCESS_PART1_SIZE_ADDR           0x1FFC8
#define HW_ACCESS_PART1_START_ADDR          0x1FFCC

#define HW_ACCESS_REGISTER_SIZE             4

#define HW_ACCESS_PRAM_MAX_RANGE		0x3c000

#define WSPI_CMD_READ                 0x40000000
#define WSPI_CMD_WRITE                0x00000000
#define WSPI_CMD_FIXED                0x20000000
#define WSPI_CMD_BYTE_LENGTH          0x1FFE0000
#define WSPI_CMD_BYTE_LENGTH_OFFSET   17
#define WSPI_CMD_BYTE_ADDR            0x0001FFFF

#define WSPI_INIT_CMD_CRC_LEN       5

#define WSPI_INIT_CMD_START         0x00
#define WSPI_INIT_CMD_TX            0x40

#define WSPI_INIT_CMD_BYPASS_BIT    0x80
#define WSPI_INIT_CMD_FIXEDBUSY_LEN 0x07
#define WSPI_INIT_CMD_EN_FIXEDBUSY  0x80
#define WSPI_INIT_CMD_DIS_FIXEDBUSY 0x00
#define WSPI_INIT_CMD_IOD           0x40
#define WSPI_INIT_CMD_IP            0x20
#define WSPI_INIT_CMD_CS            0x10
#define WSPI_INIT_CMD_WS            0x08
#define WSPI_INIT_CMD_WSPI          0x01
#define WSPI_INIT_CMD_END           0x01

#define WSPI_INIT_CMD_LEN           8

#define HW_ACCESS_WSPI_FIXED_BUSY_LEN \
		((WL1271_BUSY_WORD_LEN - 4) / sizeof(u32))
#define HW_ACCESS_WSPI_INIT_CMD_MASK  0



void wl1271_spi_write(struct wl1271 *wl, int addr, void *buf,
		      size_t len, bool fixed);
void wl1271_spi_read(struct wl1271 *wl, int addr, void *buf,
		     size_t len, bool fixed);


void wl1271_spi_mem_read(struct wl1271 *wl, int addr, void *buf, size_t len);
void wl1271_spi_mem_write(struct wl1271 *wl, int addr, void *buf, size_t len);
u32 wl1271_mem_read32(struct wl1271 *wl, int addr);
void wl1271_mem_write32(struct wl1271 *wl, int addr, u32 val);


void wl1271_spi_reg_read(struct wl1271 *wl, int addr, void *buf, size_t len,
			 bool fixed);
void wl1271_spi_reg_write(struct wl1271 *wl, int addr, void *buf, size_t len,
			  bool fixed);
u32 wl1271_reg_read32(struct wl1271 *wl, int addr);
void wl1271_reg_write32(struct wl1271 *wl, int addr, u32 val);


void wl1271_spi_reset(struct wl1271 *wl);
void wl1271_spi_init(struct wl1271 *wl);
int wl1271_set_partition(struct wl1271 *wl,
			 u32 part_start, u32 part_size,
			 u32 reg_start,  u32 reg_size);

static inline u32 wl1271_read32(struct wl1271 *wl, int addr)
{
	wl1271_spi_read(wl, addr, &wl->buffer_32,
			sizeof(wl->buffer_32), false);

	return wl->buffer_32;
}

static inline void wl1271_write32(struct wl1271 *wl, int addr, u32 val)
{
	wl->buffer_32 = val;
	wl1271_spi_write(wl, addr, &wl->buffer_32,
			 sizeof(wl->buffer_32), false);
}

#endif 
