

#ifndef __PMIC8058_KEYPAD_H__
#define __PMIC8058_KEYPAD_H__

#define MATRIX_MAX_ROWS		18
#define MATRIX_MAX_COLS		8

#define MATRIX_MAX_SIZE		(MATRIX_MAX_ROWS * MATRIX_MAX_COLS)

#define KEY_COL(val)	(((val) >> 16) & 0xff)
#define KEY_ROW(val)	(((val) >> 24) & 0xff)
#define KEY_VAL(val)	((val) & 0xffff)

#define KEY(row, col, val)	(((row % (MATRIX_MAX_ROWS)) << 24) |\
				 ((col % (MATRIX_MAX_COLS)) << 16)  |\
				 (val & 0xffff))

#define MAX_PM8058_REVS		0x5

struct pmic8058_keypad_data {
	const char *input_name;
	const char *input_phys_device;

	unsigned int num_cols;
	unsigned int num_rows;

	unsigned int rows_gpio_start;
	unsigned int cols_gpio_start;

	unsigned int debounce_ms[MAX_PM8058_REVS];
	unsigned int scan_delay_ms;
	unsigned int row_hold_ns;

	int keymap_size;
	const unsigned int *keymap;

	unsigned int wakeup;
	unsigned int rep;
};

#endif 
