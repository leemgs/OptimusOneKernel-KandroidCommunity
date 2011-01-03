

#ifndef __HW_H__
#define __HW_H__

#include "global.h"


#define IGA1_HOR_TOTAL_FORMULA(x)           (((x)/8)-5)
#define IGA1_HOR_ADDR_FORMULA(x)            (((x)/8)-1)
#define IGA1_HOR_BLANK_START_FORMULA(x)     (((x)/8)-1)
#define IGA1_HOR_BLANK_END_FORMULA(x, y)     (((x+y)/8)-1)
#define IGA1_HOR_SYNC_START_FORMULA(x)      ((x)/8)
#define IGA1_HOR_SYNC_END_FORMULA(x, y)      ((x+y)/8)

#define IGA1_VER_TOTAL_FORMULA(x)           ((x)-2)
#define IGA1_VER_ADDR_FORMULA(x)            ((x)-1)
#define IGA1_VER_BLANK_START_FORMULA(x)     ((x)-1)
#define IGA1_VER_BLANK_END_FORMULA(x, y)     ((x+y)-1)
#define IGA1_VER_SYNC_START_FORMULA(x)      ((x)-1)
#define IGA1_VER_SYNC_END_FORMULA(x, y)      ((x+y)-1)


#define IGA2_HOR_TOTAL_FORMULA(x)           ((x)-1)
#define IGA2_HOR_ADDR_FORMULA(x)            ((x)-1)
#define IGA2_HOR_BLANK_START_FORMULA(x)     ((x)-1)
#define IGA2_HOR_BLANK_END_FORMULA(x, y)     ((x+y)-1)
#define IGA2_HOR_SYNC_START_FORMULA(x)      ((x)-1)
#define IGA2_HOR_SYNC_END_FORMULA(x, y)      ((x+y)-1)

#define IGA2_VER_TOTAL_FORMULA(x)           ((x)-1)
#define IGA2_VER_ADDR_FORMULA(x)            ((x)-1)
#define IGA2_VER_BLANK_START_FORMULA(x)     ((x)-1)
#define IGA2_VER_BLANK_END_FORMULA(x, y)     ((x+y)-1)
#define IGA2_VER_SYNC_START_FORMULA(x)      ((x)-1)
#define IGA2_VER_SYNC_END_FORMULA(x, y)      ((x+y)-1)




#define IGA2_HOR_TOTAL_SHADOW_FORMULA(x)           ((x/8)-5)
#define IGA2_HOR_BLANK_END_SHADOW_FORMULA(x, y)     (((x+y)/8)-1)
#define IGA2_VER_TOTAL_SHADOW_FORMULA(x)           ((x)-2)
#define IGA2_VER_ADDR_SHADOW_FORMULA(x)            ((x)-1)
#define IGA2_VER_BLANK_START_SHADOW_FORMULA(x)     ((x)-1)
#define IGA2_VER_BLANK_END_SHADOW_FORMULA(x, y)     ((x+y)-1)
#define IGA2_VER_SYNC_START_SHADOW_FORMULA(x)      (x)
#define IGA2_VER_SYNC_END_SHADOW_FORMULA(x, y)      (x+y)




#define IGA1_HOR_TOTAL_REG_NUM		2

#define IGA1_HOR_ADDR_REG_NUM		1

#define IGA1_HOR_BLANK_START_REG_NUM    1

#define IGA1_HOR_BLANK_END_REG_NUM	3

#define IGA1_HOR_SYNC_START_REG_NUM	2

#define IGA1_HOR_SYNC_END_REG_NUM       1

#define IGA1_VER_TOTAL_REG_NUM          4

#define IGA1_VER_ADDR_REG_NUM           4

#define IGA1_VER_BLANK_START_REG_NUM    4

#define IGA1_VER_BLANK_END_REG_NUM      1

#define IGA1_VER_SYNC_START_REG_NUM     4

#define IGA1_VER_SYNC_END_REG_NUM       1




#define IGA2_SHADOW_HOR_TOTAL_REG_NUM       2

#define IGA2_SHADOW_HOR_BLANK_END_REG_NUM   1

#define IGA2_SHADOW_VER_TOTAL_REG_NUM       2

#define IGA2_SHADOW_VER_ADDR_REG_NUM        2

#define IGA2_SHADOW_VER_BLANK_START_REG_NUM 2

#define IGA2_SHADOW_VER_BLANK_END_REG_NUM   2

#define IGA2_SHADOW_VER_SYNC_START_REG_NUM  2

#define IGA2_SHADOW_VER_SYNC_END_REG_NUM    1




#define IGA2_HOR_TOTAL_REG_NUM          2

#define IGA2_HOR_ADDR_REG_NUM           2

#define IGA2_HOR_BLANK_START_REG_NUM    2


#define IGA2_HOR_BLANK_END_REG_NUM      3


#define IGA2_HOR_SYNC_START_REG_NUM     4


#define IGA2_HOR_SYNC_END_REG_NUM       2

#define IGA2_VER_TOTAL_REG_NUM          2

#define IGA2_VER_ADDR_REG_NUM           2

#define IGA2_VER_BLANK_START_REG_NUM    2

#define IGA2_VER_BLANK_END_REG_NUM      2

#define IGA2_VER_SYNC_START_REG_NUM     2

#define IGA2_VER_SYNC_END_REG_NUM       1




#define IGA1_FETCH_COUNT_REG_NUM        2

#define IGA1_FETCH_COUNT_ALIGN_BYTE     16

#define IGA1_FETCH_COUNT_PATCH_VALUE    4
#define IGA1_FETCH_COUNT_FORMULA(x, y)   \
	(((x*y)/IGA1_FETCH_COUNT_ALIGN_BYTE) + IGA1_FETCH_COUNT_PATCH_VALUE)


#define IGA2_FETCH_COUNT_REG_NUM        2
#define IGA2_FETCH_COUNT_ALIGN_BYTE     16
#define IGA2_FETCH_COUNT_PATCH_VALUE    0
#define IGA2_FETCH_COUNT_FORMULA(x, y)   \
	(((x*y)/IGA2_FETCH_COUNT_ALIGN_BYTE) + IGA2_FETCH_COUNT_PATCH_VALUE)




#define IGA1_STARTING_ADDR_REG_NUM      4

#define IGA2_STARTING_ADDR_REG_NUM      3




#define K800_IGA1_FIFO_MAX_DEPTH                384

#define K800_IGA1_FIFO_THRESHOLD                328

#define K800_IGA1_FIFO_HIGH_THRESHOLD           296

				
#define K800_IGA1_DISPLAY_QUEUE_EXPIRE_NUM      0


#define K800_IGA2_FIFO_MAX_DEPTH                384

#define K800_IGA2_FIFO_THRESHOLD                328

#define K800_IGA2_FIFO_HIGH_THRESHOLD           296

#define K800_IGA2_DISPLAY_QUEUE_EXPIRE_NUM      128


#define P880_IGA1_FIFO_MAX_DEPTH                192

#define P880_IGA1_FIFO_THRESHOLD                128

#define P880_IGA1_FIFO_HIGH_THRESHOLD           64

				
#define P880_IGA1_DISPLAY_QUEUE_EXPIRE_NUM      0


#define P880_IGA2_FIFO_MAX_DEPTH                96

#define P880_IGA2_FIFO_THRESHOLD                64

#define P880_IGA2_FIFO_HIGH_THRESHOLD           32

#define P880_IGA2_DISPLAY_QUEUE_EXPIRE_NUM      128




#define CN700_IGA1_FIFO_MAX_DEPTH               96

#define CN700_IGA1_FIFO_THRESHOLD               80

#define CN700_IGA1_FIFO_HIGH_THRESHOLD          64

#define CN700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     0

#define CN700_IGA2_FIFO_MAX_DEPTH               96

#define CN700_IGA2_FIFO_THRESHOLD               80

#define CN700_IGA2_FIFO_HIGH_THRESHOLD          32

#define CN700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     128



#define CX700_IGA1_FIFO_MAX_DEPTH               192

#define CX700_IGA1_FIFO_THRESHOLD               128

#define CX700_IGA1_FIFO_HIGH_THRESHOLD          128

#define CX700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     124


#define CX700_IGA2_FIFO_MAX_DEPTH               96

#define CX700_IGA2_FIFO_THRESHOLD               64

#define CX700_IGA2_FIFO_HIGH_THRESHOLD          32

#define CX700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     128



#define K8M890_IGA1_FIFO_MAX_DEPTH               360

#define K8M890_IGA1_FIFO_THRESHOLD               328

#define K8M890_IGA1_FIFO_HIGH_THRESHOLD          296

#define K8M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     124


#define K8M890_IGA2_FIFO_MAX_DEPTH               360

#define K8M890_IGA2_FIFO_THRESHOLD               328

#define K8M890_IGA2_FIFO_HIGH_THRESHOLD          296

#define K8M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     124



#define P4M890_IGA1_FIFO_MAX_DEPTH               96

#define P4M890_IGA1_FIFO_THRESHOLD               76

#define P4M890_IGA1_FIFO_HIGH_THRESHOLD          64

#define P4M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     32

#define P4M890_IGA2_FIFO_MAX_DEPTH               96

#define P4M890_IGA2_FIFO_THRESHOLD               76

#define P4M890_IGA2_FIFO_HIGH_THRESHOLD          64

#define P4M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     32



#define P4M900_IGA1_FIFO_MAX_DEPTH               96

#define P4M900_IGA1_FIFO_THRESHOLD               76

#define P4M900_IGA1_FIFO_HIGH_THRESHOLD          76

#define P4M900_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     32

#define P4M900_IGA2_FIFO_MAX_DEPTH               96

#define P4M900_IGA2_FIFO_THRESHOLD               76

#define P4M900_IGA2_FIFO_HIGH_THRESHOLD          76

#define P4M900_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     32



#define VX800_IGA1_FIFO_MAX_DEPTH               192

#define VX800_IGA1_FIFO_THRESHOLD               152

#define VX800_IGA1_FIFO_HIGH_THRESHOLD          152

#define VX800_IGA1_DISPLAY_QUEUE_EXPIRE_NUM      64

#define VX800_IGA2_FIFO_MAX_DEPTH               96

#define VX800_IGA2_FIFO_THRESHOLD               64

#define VX800_IGA2_FIFO_HIGH_THRESHOLD          32

#define VX800_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     128


#define VX855_IGA1_FIFO_MAX_DEPTH               400
#define VX855_IGA1_FIFO_THRESHOLD               320
#define VX855_IGA1_FIFO_HIGH_THRESHOLD          320
#define VX855_IGA1_DISPLAY_QUEUE_EXPIRE_NUM     160

#define VX855_IGA2_FIFO_MAX_DEPTH               200
#define VX855_IGA2_FIFO_THRESHOLD               160
#define VX855_IGA2_FIFO_HIGH_THRESHOLD          160
#define VX855_IGA2_DISPLAY_QUEUE_EXPIRE_NUM     320

#define IGA1_FIFO_DEPTH_SELECT_REG_NUM          1
#define IGA1_FIFO_THRESHOLD_REG_NUM             2
#define IGA1_FIFO_HIGH_THRESHOLD_REG_NUM        2
#define IGA1_DISPLAY_QUEUE_EXPIRE_NUM_REG_NUM   1

#define IGA2_FIFO_DEPTH_SELECT_REG_NUM          3
#define IGA2_FIFO_THRESHOLD_REG_NUM             2
#define IGA2_FIFO_HIGH_THRESHOLD_REG_NUM        2
#define IGA2_DISPLAY_QUEUE_EXPIRE_NUM_REG_NUM   1

#define IGA1_FIFO_DEPTH_SELECT_FORMULA(x)                   ((x/2)-1)
#define IGA1_FIFO_THRESHOLD_FORMULA(x)                      (x/4)
#define IGA1_DISPLAY_QUEUE_EXPIRE_NUM_FORMULA(x)            (x/4)
#define IGA1_FIFO_HIGH_THRESHOLD_FORMULA(x)                 (x/4)
#define IGA2_FIFO_DEPTH_SELECT_FORMULA(x)                   (((x/2)/4)-1)
#define IGA2_FIFO_THRESHOLD_FORMULA(x)                      (x/4)
#define IGA2_DISPLAY_QUEUE_EXPIRE_NUM_FORMULA(x)            (x/4)
#define IGA2_FIFO_HIGH_THRESHOLD_FORMULA(x)                 (x/4)






#define LCD_POWER_SEQ_TD0               500000

#define LCD_POWER_SEQ_TD1               50000

#define LCD_POWER_SEQ_TD2               0

#define LCD_POWER_SEQ_TD3               210000

#define CLE266_POWER_SEQ_UNIT           71

#define K800_POWER_SEQ_UNIT             142

#define P880_POWER_SEQ_UNIT             572

#define CLE266_POWER_SEQ_FORMULA(x)     ((x)/CLE266_POWER_SEQ_UNIT)
#define K800_POWER_SEQ_FORMULA(x)       ((x)/K800_POWER_SEQ_UNIT)
#define P880_POWER_SEQ_FORMULA(x)       ((x)/P880_POWER_SEQ_UNIT)


#define LCD_POWER_SEQ_TD0_REG_NUM       2

#define LCD_POWER_SEQ_TD1_REG_NUM       2

#define LCD_POWER_SEQ_TD2_REG_NUM       2

#define LCD_POWER_SEQ_TD3_REG_NUM       2






#define CLE266_LCD_HOR_SCF_FORMULA(x, y)   (((x-1)*1024)/(y-1))

#define CLE266_LCD_VER_SCF_FORMULA(x, y)   (((x-1)*1024)/(y-1))

#define K800_LCD_HOR_SCF_FORMULA(x, y)     (((x-1)*4096)/(y-1))

#define K800_LCD_VER_SCF_FORMULA(x, y)     (((x-1)*2048)/(y-1))


#define LCD_HOR_SCALING_FACTOR_REG_NUM  3

#define LCD_VER_SCALING_FACTOR_REG_NUM  3

#define LCD_HOR_SCALING_FACTOR_REG_NUM_CLE  2

#define LCD_VER_SCALING_FACTOR_REG_NUM_CLE  2


struct io_register {
	u8 io_addr;
	u8 start_bit;
	u8 end_bit;
};


struct iga1_hor_total {
	int reg_num;
	struct io_register reg[IGA1_HOR_TOTAL_REG_NUM];
};


struct iga1_hor_addr {
	int reg_num;
	struct io_register reg[IGA1_HOR_ADDR_REG_NUM];
};


struct iga1_hor_blank_start {
	int reg_num;
	struct io_register reg[IGA1_HOR_BLANK_START_REG_NUM];
};


struct iga1_hor_blank_end {
	int reg_num;
	struct io_register reg[IGA1_HOR_BLANK_END_REG_NUM];
};


struct iga1_hor_sync_start {
	int reg_num;
	struct io_register reg[IGA1_HOR_SYNC_START_REG_NUM];
};


struct iga1_hor_sync_end {
	int reg_num;
	struct io_register reg[IGA1_HOR_SYNC_END_REG_NUM];
};


struct iga1_ver_total {
	int reg_num;
	struct io_register reg[IGA1_VER_TOTAL_REG_NUM];
};


struct iga1_ver_addr {
	int reg_num;
	struct io_register reg[IGA1_VER_ADDR_REG_NUM];
};


struct iga1_ver_blank_start {
	int reg_num;
	struct io_register reg[IGA1_VER_BLANK_START_REG_NUM];
};


struct iga1_ver_blank_end {
	int reg_num;
	struct io_register reg[IGA1_VER_BLANK_END_REG_NUM];
};


struct iga1_ver_sync_start {
	int reg_num;
	struct io_register reg[IGA1_VER_SYNC_START_REG_NUM];
};


struct iga1_ver_sync_end {
	int reg_num;
	struct io_register reg[IGA1_VER_SYNC_END_REG_NUM];
};




struct iga2_shadow_hor_total {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_HOR_TOTAL_REG_NUM];
};


struct iga2_shadow_hor_blank_end {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_HOR_BLANK_END_REG_NUM];
};


struct iga2_shadow_ver_total {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_TOTAL_REG_NUM];
};


struct iga2_shadow_ver_addr {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_ADDR_REG_NUM];
};


struct iga2_shadow_ver_blank_start {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_BLANK_START_REG_NUM];
};


struct iga2_shadow_ver_blank_end {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_BLANK_END_REG_NUM];
};


struct iga2_shadow_ver_sync_start {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_SYNC_START_REG_NUM];
};


struct iga2_shadow_ver_sync_end {
	int reg_num;
	struct io_register reg[IGA2_SHADOW_VER_SYNC_END_REG_NUM];
};




struct iga2_hor_total {
	int reg_num;
	struct io_register reg[IGA2_HOR_TOTAL_REG_NUM];
};


struct iga2_hor_addr {
	int reg_num;
	struct io_register reg[IGA2_HOR_ADDR_REG_NUM];
};


struct iga2_hor_blank_start {
	int reg_num;
	struct io_register reg[IGA2_HOR_BLANK_START_REG_NUM];
};


struct iga2_hor_blank_end {
	int reg_num;
	struct io_register reg[IGA2_HOR_BLANK_END_REG_NUM];
};


struct iga2_hor_sync_start {
	int reg_num;
	struct io_register reg[IGA2_HOR_SYNC_START_REG_NUM];
};


struct iga2_hor_sync_end {
	int reg_num;
	struct io_register reg[IGA2_HOR_SYNC_END_REG_NUM];
};


struct iga2_ver_total {
	int reg_num;
	struct io_register reg[IGA2_VER_TOTAL_REG_NUM];
};


struct iga2_ver_addr {
	int reg_num;
	struct io_register reg[IGA2_VER_ADDR_REG_NUM];
};


struct iga2_ver_blank_start {
	int reg_num;
	struct io_register reg[IGA2_VER_BLANK_START_REG_NUM];
};


struct iga2_ver_blank_end {
	int reg_num;
	struct io_register reg[IGA2_VER_BLANK_END_REG_NUM];
};


struct iga2_ver_sync_start {
	int reg_num;
	struct io_register reg[IGA2_VER_SYNC_START_REG_NUM];
};


struct iga2_ver_sync_end {
	int reg_num;
	struct io_register reg[IGA2_VER_SYNC_END_REG_NUM];
};


struct iga1_fetch_count {
	int reg_num;
	struct io_register reg[IGA1_FETCH_COUNT_REG_NUM];
};


struct iga2_fetch_count {
	int reg_num;
	struct io_register reg[IGA2_FETCH_COUNT_REG_NUM];
};

struct fetch_count {
	struct iga1_fetch_count iga1_fetch_count_reg;
	struct iga2_fetch_count iga2_fetch_count_reg;
};


struct iga1_starting_addr {
	int reg_num;
	struct io_register reg[IGA1_STARTING_ADDR_REG_NUM];
};

struct iga2_starting_addr {
	int reg_num;
	struct io_register reg[IGA2_STARTING_ADDR_REG_NUM];
};

struct starting_addr {
	struct iga1_starting_addr iga1_starting_addr_reg;
	struct iga2_starting_addr iga2_starting_addr_reg;
};


struct lcd_pwd_seq_td0 {
	int reg_num;
	struct io_register reg[LCD_POWER_SEQ_TD0_REG_NUM];
};

struct lcd_pwd_seq_td1 {
	int reg_num;
	struct io_register reg[LCD_POWER_SEQ_TD1_REG_NUM];
};

struct lcd_pwd_seq_td2 {
	int reg_num;
	struct io_register reg[LCD_POWER_SEQ_TD2_REG_NUM];
};

struct lcd_pwd_seq_td3 {
	int reg_num;
	struct io_register reg[LCD_POWER_SEQ_TD3_REG_NUM];
};

struct _lcd_pwd_seq_timer {
	struct lcd_pwd_seq_td0 td0;
	struct lcd_pwd_seq_td1 td1;
	struct lcd_pwd_seq_td2 td2;
	struct lcd_pwd_seq_td3 td3;
};


struct _lcd_hor_scaling_factor {
	int reg_num;
	struct io_register reg[LCD_HOR_SCALING_FACTOR_REG_NUM];
};

struct _lcd_ver_scaling_factor {
	int reg_num;
	struct io_register reg[LCD_VER_SCALING_FACTOR_REG_NUM];
};

struct _lcd_scaling_factor {
	struct _lcd_hor_scaling_factor lcd_hor_scaling_factor;
	struct _lcd_ver_scaling_factor lcd_ver_scaling_factor;
};

struct pll_map {
	u32 clk;
	u32 cle266_pll;
	u32 k800_pll;
	u32 cx700_pll;
	u32 vx855_pll;
};

struct rgbLUT {
	u8 red;
	u8 green;
	u8 blue;
};

struct lcd_pwd_seq_timer {
	u16 td0;
	u16 td1;
	u16 td2;
	u16 td3;
};


struct iga1_fifo_depth_select {
	int reg_num;
	struct io_register reg[IGA1_FIFO_DEPTH_SELECT_REG_NUM];
};

struct iga1_fifo_threshold_select {
	int reg_num;
	struct io_register reg[IGA1_FIFO_THRESHOLD_REG_NUM];
};

struct iga1_fifo_high_threshold_select {
	int reg_num;
	struct io_register reg[IGA1_FIFO_HIGH_THRESHOLD_REG_NUM];
};

struct iga1_display_queue_expire_num {
	int reg_num;
	struct io_register reg[IGA1_DISPLAY_QUEUE_EXPIRE_NUM_REG_NUM];
};

struct iga2_fifo_depth_select {
	int reg_num;
	struct io_register reg[IGA2_FIFO_DEPTH_SELECT_REG_NUM];
};

struct iga2_fifo_threshold_select {
	int reg_num;
	struct io_register reg[IGA2_FIFO_THRESHOLD_REG_NUM];
};

struct iga2_fifo_high_threshold_select {
	int reg_num;
	struct io_register reg[IGA2_FIFO_HIGH_THRESHOLD_REG_NUM];
};

struct iga2_display_queue_expire_num {
	int reg_num;
	struct io_register reg[IGA2_DISPLAY_QUEUE_EXPIRE_NUM_REG_NUM];
};

struct fifo_depth_select {
	struct iga1_fifo_depth_select iga1_fifo_depth_select_reg;
	struct iga2_fifo_depth_select iga2_fifo_depth_select_reg;
};

struct fifo_threshold_select {
	struct iga1_fifo_threshold_select iga1_fifo_threshold_select_reg;
	struct iga2_fifo_threshold_select iga2_fifo_threshold_select_reg;
};

struct fifo_high_threshold_select {
	struct iga1_fifo_high_threshold_select
	 iga1_fifo_high_threshold_select_reg;
	struct iga2_fifo_high_threshold_select
	 iga2_fifo_high_threshold_select_reg;
};

struct display_queue_expire_num {
	struct iga1_display_queue_expire_num
	 iga1_display_queue_expire_num_reg;
	struct iga2_display_queue_expire_num
	 iga2_display_queue_expire_num_reg;
};

struct iga1_crtc_timing {
	struct iga1_hor_total hor_total;
	struct iga1_hor_addr hor_addr;
	struct iga1_hor_blank_start hor_blank_start;
	struct iga1_hor_blank_end hor_blank_end;
	struct iga1_hor_sync_start hor_sync_start;
	struct iga1_hor_sync_end hor_sync_end;
	struct iga1_ver_total ver_total;
	struct iga1_ver_addr ver_addr;
	struct iga1_ver_blank_start ver_blank_start;
	struct iga1_ver_blank_end ver_blank_end;
	struct iga1_ver_sync_start ver_sync_start;
	struct iga1_ver_sync_end ver_sync_end;
};

struct iga2_shadow_crtc_timing {
	struct iga2_shadow_hor_total hor_total_shadow;
	struct iga2_shadow_hor_blank_end hor_blank_end_shadow;
	struct iga2_shadow_ver_total ver_total_shadow;
	struct iga2_shadow_ver_addr ver_addr_shadow;
	struct iga2_shadow_ver_blank_start ver_blank_start_shadow;
	struct iga2_shadow_ver_blank_end ver_blank_end_shadow;
	struct iga2_shadow_ver_sync_start ver_sync_start_shadow;
	struct iga2_shadow_ver_sync_end ver_sync_end_shadow;
};

struct iga2_crtc_timing {
	struct iga2_hor_total hor_total;
	struct iga2_hor_addr hor_addr;
	struct iga2_hor_blank_start hor_blank_start;
	struct iga2_hor_blank_end hor_blank_end;
	struct iga2_hor_sync_start hor_sync_start;
	struct iga2_hor_sync_end hor_sync_end;
	struct iga2_ver_total ver_total;
	struct iga2_ver_addr ver_addr;
	struct iga2_ver_blank_start ver_blank_start;
	struct iga2_ver_blank_end ver_blank_end;
	struct iga2_ver_sync_start ver_sync_start;
	struct iga2_ver_sync_end ver_sync_end;
};


#define CLE266              0x3123
#define KM400               0x3205
#define CN400_FUNCTION2     0x2259
#define CN400_FUNCTION3     0x3259

#define CN700_FUNCTION2     0x2314
#define CN700_FUNCTION3     0x3208

#define CX700_FUNCTION2     0x2324
#define CX700_FUNCTION3     0x3324

#define KM800_FUNCTION3      0x3204

#define KM890_FUNCTION3      0x3336

#define P4M890_FUNCTION3     0x3327

#define CN750_FUNCTION3     0x3208

#define P4M900_FUNCTION3    0x3364

#define VX800_FUNCTION3     0x3353

#define VX855_FUNCTION3     0x3409

#define NUM_TOTAL_PLL_TABLE ARRAY_SIZE(pll_value)

struct IODATA {
	u8 Index;
	u8 Mask;
	u8 Data;
};

struct pci_device_id_info {
	u32 vendor;
	u32 device;
	u32 chip_index;
};

extern unsigned int viafb_second_virtual_xres;
extern unsigned int viafb_second_offset;
extern int viafb_second_size;
extern int viafb_SAMM_ON;
extern int viafb_dual_fb;
extern int viafb_LCD2_ON;
extern int viafb_LCD_ON;
extern int viafb_DVI_ON;
extern int viafb_hotplug;

void viafb_write_reg_mask(u8 index, int io_port, u8 data, u8 mask);
void viafb_set_output_path(int device, int set_iga,
	int output_interface);
void viafb_fill_crtc_timing(struct crt_mode_table *crt_table,
		      int mode_index, int bpp_byte, int set_iga);

void viafb_set_vclock(u32 CLK, int set_iga);
void viafb_load_reg(int timing_value, int viafb_load_reg_num,
	struct io_register *reg,
	      int io_type);
void viafb_crt_disable(void);
void viafb_crt_enable(void);
void init_ad9389(void);

void viafb_write_reg(u8 index, u16 io_port, u8 data);
u8 viafb_read_reg(int io_port, u8 index);
void viafb_lock_crt(void);
void viafb_unlock_crt(void);
void viafb_load_fetch_count_reg(int h_addr, int bpp_byte, int set_iga);
void viafb_write_regx(struct io_reg RegTable[], int ItemNum);
struct VideoModeTable *viafb_get_modetbl_pointer(int Index);
u32 viafb_get_clk_value(int clk);
void viafb_load_FIFO_reg(int set_iga, int hor_active, int ver_active);
void viafb_set_color_depth(int bpp_byte, int set_iga);
void viafb_set_dpa_gfx(int output_interface, struct GFX_DPA_SETTING\
					*p_gfx_dpa_setting);

int viafb_setmode(int vmode_index, int hor_res, int ver_res,
	    int video_bpp, int vmode_index1, int hor_res1,
	    int ver_res1, int video_bpp1);
void viafb_init_chip_info(struct pci_dev *pdev,
			  const struct pci_device_id *pdi);
void viafb_init_dac(int set_iga);
int viafb_get_pixclock(int hres, int vres, int vmode_refresh);
int viafb_get_refresh(int hres, int vres, u32 float_refresh);
void viafb_update_device_setting(int hres, int vres, int bpp,
			   int vmode_refresh, int flag);

int viafb_get_fb_size_from_pci(void);
void viafb_set_iga_path(void);
void viafb_set_primary_address(u32 addr);
void viafb_set_secondary_address(u32 addr);
void viafb_set_primary_pitch(u32 pitch);
void viafb_set_secondary_pitch(u32 pitch);
void viafb_get_fb_info(unsigned int *fb_base, unsigned int *fb_len);

#endif 
