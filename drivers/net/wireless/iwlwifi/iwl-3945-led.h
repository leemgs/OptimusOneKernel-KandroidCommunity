

#ifndef IWL3945_LEDS_H
#define IWL3945_LEDS_H

struct iwl_priv;

#ifdef CONFIG_IWLWIFI_LEDS

#include "iwl-led.h"

extern int iwl3945_led_register(struct iwl_priv *priv);
extern void iwl3945_led_unregister(struct iwl_priv *priv);
extern void iwl3945_led_background(struct iwl_priv *priv);

#else
static inline int iwl3945_led_register(struct iwl_priv *priv) { return 0; }
static inline void iwl3945_led_unregister(struct iwl_priv *priv) {}
static inline void iwl3945_led_background(struct iwl_priv *priv) {}

#endif 
#endif 
