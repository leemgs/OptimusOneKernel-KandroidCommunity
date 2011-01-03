

#ifndef _WM_HUBS_H
#define _WM_HUBS_H

struct snd_soc_codec;

extern const unsigned int wm_hubs_spkmix_tlv[];

extern int wm_hubs_add_analogue_controls(struct snd_soc_codec *);
extern int wm_hubs_add_analogue_routes(struct snd_soc_codec *, int, int);

#endif
