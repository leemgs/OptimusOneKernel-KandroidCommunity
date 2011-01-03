

struct stmmac_timer {
	void (*timer_start) (unsigned int new_freq);
	void (*timer_stop) (void);
	unsigned int freq;
	unsigned int enable;
};


int stmmac_open_ext_timer(struct net_device *dev, struct stmmac_timer *tm);

int stmmac_close_ext_timer(void);

void stmmac_schedule(struct net_device *dev);

#if defined(CONFIG_STMMAC_TMU_TIMER)
extern int tmu2_register_user(void *fnt, void *data);
extern void tmu2_unregister_user(void);
#endif
