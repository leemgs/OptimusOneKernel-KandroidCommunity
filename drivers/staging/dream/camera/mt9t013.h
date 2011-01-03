

#ifndef MT9T013_H
#define MT9T013_H

#include <linux/types.h>

struct reg_struct {
	uint16_t vt_pix_clk_div;        
	uint16_t vt_sys_clk_div;        
	uint16_t pre_pll_clk_div;       
	uint16_t pll_multiplier;        
	uint16_t op_pix_clk_div;        
	uint16_t op_sys_clk_div;        
	uint16_t scale_m;               
	uint16_t row_speed;             
	uint16_t x_addr_start;          
	uint16_t x_addr_end;            
	uint16_t y_addr_start;        	
	uint16_t y_addr_end;            
	uint16_t read_mode;             
	uint16_t x_output_size;         
	uint16_t y_output_size;         
	uint16_t line_length_pck;       
	uint16_t frame_length_lines;	
	uint16_t coarse_int_time; 		
	uint16_t fine_int_time;   		
};

struct mt9t013_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};

struct mt9t013_reg {
	struct reg_struct *reg_pat;
	uint16_t reg_pat_size;
	struct mt9t013_i2c_reg_conf *ttbl;
	uint16_t ttbl_size;
	struct mt9t013_i2c_reg_conf *lctbl;
	uint16_t lctbl_size;
	struct mt9t013_i2c_reg_conf *rftbl;
	uint16_t rftbl_size;
};

#endif 
