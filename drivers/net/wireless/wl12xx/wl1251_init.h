

#ifndef __WL1251_INIT_H__
#define __WL1251_INIT_H__

#include "wl1251.h"

int wl1251_hw_init_hwenc_config(struct wl1251 *wl);
int wl1251_hw_init_templates_config(struct wl1251 *wl);
int wl1251_hw_init_rx_config(struct wl1251 *wl, u32 config, u32 filter);
int wl1251_hw_init_phy_config(struct wl1251 *wl);
int wl1251_hw_init_beacon_filter(struct wl1251 *wl);
int wl1251_hw_init_pta(struct wl1251 *wl);
int wl1251_hw_init_energy_detection(struct wl1251 *wl);
int wl1251_hw_init_beacon_broadcast(struct wl1251 *wl);
int wl1251_hw_init_power_auth(struct wl1251 *wl);
int wl1251_hw_init_mem_config(struct wl1251 *wl);
int wl1251_hw_init(struct wl1251 *wl);

#endif
