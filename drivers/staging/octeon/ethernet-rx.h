

irqreturn_t cvm_oct_do_interrupt(int cpl, void *dev_id);
void cvm_oct_poll_controller(struct net_device *dev);
void cvm_oct_tasklet_rx(unsigned long unused);

void cvm_oct_rx_initialize(void);
void cvm_oct_rx_shutdown(void);
