


struct msm_touchpad_platform_data {
	int gpioirq;
	int gpiosuspend;
	int (*gpio_setup) (void);
	void (*gpio_shutdown)(void);
};
