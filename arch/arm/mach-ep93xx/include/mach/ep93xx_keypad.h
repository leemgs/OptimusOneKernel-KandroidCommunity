

#ifndef __ASM_ARCH_EP93XX_KEYPAD_H
#define __ASM_ARCH_EP93XX_KEYPAD_H

#define MAX_MATRIX_KEY_ROWS		(8)
#define MAX_MATRIX_KEY_COLS		(8)


#define EP93XX_KEYPAD_DISABLE_3_KEY	(1<<0)	
#define EP93XX_KEYPAD_DIAG_MODE		(1<<1)	
#define EP93XX_KEYPAD_BACK_DRIVE	(1<<2)	
#define EP93XX_KEYPAD_TEST_MODE		(1<<3)	
#define EP93XX_KEYPAD_KDIV		(1<<4)	
#define EP93XX_KEYPAD_AUTOREPEAT	(1<<5)	


struct ep93xx_keypad_platform_data {
	unsigned int	matrix_key_rows;
	unsigned int	matrix_key_cols;
	unsigned int	*matrix_key_map;
	int		matrix_key_map_size;
	unsigned int	debounce;
	unsigned int	prescale;
	unsigned int	flags;
};


#define KEY(row, col, val)	(((row) << 28) | ((col) << 24) | (val))

#endif	
