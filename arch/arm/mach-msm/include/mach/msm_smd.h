

#ifndef __ASM_ARCH_MSM_SMD_H
#define __ASM_ARCH_MSM_SMD_H

typedef struct smd_channel smd_channel_t;


int smd_open(const char *name, smd_channel_t **ch, void *priv,
	     void (*notify)(void *priv, unsigned event));

#define SMD_EVENT_DATA 1
#define SMD_EVENT_OPEN 2
#define SMD_EVENT_CLOSE 3

int smd_close(smd_channel_t *ch);


int smd_read(smd_channel_t *ch, void *data, int len);
int smd_read_from_cb(smd_channel_t *ch, void *data, int len);


int smd_write(smd_channel_t *ch, const void *data, int len);

int smd_write_avail(smd_channel_t *ch);
int smd_read_avail(smd_channel_t *ch);


int smd_cur_packet_size(smd_channel_t *ch);


#if 0

int smd_wait_until_readable(smd_channel_t *ch, int bytes);
int smd_wait_until_writable(smd_channel_t *ch, int bytes);
#endif


int smd_tiocmget(smd_channel_t *ch);
int smd_tiocmset(smd_channel_t *ch, unsigned int set, unsigned int clear);

#if defined(CONFIG_MSM_N_WAY_SMD)
enum {
	SMD_APPS_MODEM = 0,
	SMD_APPS_QDSP,
	SMD_MODEM_QDSP,
	SMD_LOOPBACK_TYPE = 100,

};
#else
enum {
	SMD_APPS_MODEM = 0,
	SMD_LOOPBACK_TYPE = 100,
};
#endif

int smd_named_open_on_edge(const char *name, uint32_t edge, smd_channel_t **_ch,
			   void *priv, void (*notify)(void *, unsigned));

#endif
