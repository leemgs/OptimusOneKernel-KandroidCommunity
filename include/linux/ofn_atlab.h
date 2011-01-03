


struct ofn_function1 {
	bool no_motion1_en;
	bool touch_sensor_en;
	bool ofn_en;
	u16 clock_select_khz;
	u32 cpi_selection;
};

struct ofn_function2 {
	bool invert_y;
	bool invert_x;
	bool swap_x_y;
	bool hold_a_b_en;
	bool motion_filter_en;
};

struct ofn_atlab_platform_data {
	int irq_button_l;
	int irq_button_r;
	int gpio_button_l;
	int gpio_button_r;
	int rotate_xy;
	int (*gpio_setup)(void);
	void (*gpio_release)(void);
	int (*optnav_on)(void);
	void (*optnav_off)(void);
	struct ofn_function1 function1;
	struct ofn_function2 function2;
};
