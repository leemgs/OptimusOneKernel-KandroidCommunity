
#ifndef ASMARM_ARCH_KEYPAD_H
#define ASMARM_ARCH_KEYPAD_H

#warning: Please update the board to use matrix_keypad.h instead

struct omap_kp_platform_data {
	int rows;
	int cols;
	int *keymap;
	unsigned int keymapsize;
	unsigned int rep:1;
	unsigned long delay;
	unsigned int dbounce:1;
	
	unsigned int *row_gpios;
	unsigned int *col_gpios;
};


#define GROUP_0		(0 << 16)
#define GROUP_1		(1 << 16)
#define GROUP_2		(2 << 16)
#define GROUP_3		(3 << 16)
#define GROUP_MASK	GROUP_3

#define KEY_PERSISTENT		0x00800000
#define KEYNUM_MASK		0x00EFFFFF
#define KEY(col, row, val) (((col) << 28) | ((row) << 24) | (val))
#define PERSISTENT_KEY(col, row) (((col) << 28) | ((row) << 24) | \
						KEY_PERSISTENT)

#endif

