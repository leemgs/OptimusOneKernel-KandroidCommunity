

#ifndef _TDISC_SHINETSU_H_
#define _TDISC_SHINETSU_H_

struct tdisc_abs_values {
	int	x_max;
	int	y_max;
	int	x_min;
	int	y_min;
	int	pressure_max;
	int	pressure_min;
};

struct tdisc_platform_data {
	int	(*tdisc_setup) (void);
	void	(*tdisc_release) (void);
	int	(*tdisc_enable) (void);
	void	(*tdisc_disable)(void);
	int	tdisc_wakeup;
	int	tdisc_gpio;
	struct	tdisc_abs_values *tdisc_abs;
};

#endif 
