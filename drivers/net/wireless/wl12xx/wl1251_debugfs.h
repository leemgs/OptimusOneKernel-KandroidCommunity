

#ifndef WL1251_DEBUGFS_H
#define WL1251_DEBUGFS_H

#include "wl1251.h"

int wl1251_debugfs_init(struct wl1251 *wl);
void wl1251_debugfs_exit(struct wl1251 *wl);
void wl1251_debugfs_reset(struct wl1251 *wl);

#endif 
