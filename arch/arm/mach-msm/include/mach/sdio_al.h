



#ifndef __SDIO_AL__
#define __SDIO_AL__

struct sdio_channel; 


#define SDIO_EVENT_DATA_READ_AVAIL      0x01
#define SDIO_EVENT_DATA_WRITE_AVAIL     0x02


int sdio_open(const char *name, struct sdio_channel **ch, void *priv,
	     void (*notify)(void *priv, unsigned channel_event));



int sdio_close(struct sdio_channel *ch);


int sdio_read(struct sdio_channel *ch, void *data, int len);


int sdio_write(struct sdio_channel *ch, const void *data, int len);


int sdio_write_avail(struct sdio_channel *ch);


int sdio_read_avail(struct sdio_channel *ch);


int sdio_set_write_threshold(struct sdio_channel *ch, int threshold);


int sdio_set_read_threshold(struct sdio_channel *ch, int threshold);


int sdio_set_poll_time(struct sdio_channel *ch, int poll_delay_msec);

#endif 
