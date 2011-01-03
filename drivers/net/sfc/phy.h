

#ifndef EFX_PHY_H
#define EFX_PHY_H


extern struct efx_phy_operations falcon_sfx7101_phy_ops;
extern struct efx_phy_operations falcon_sft9001_phy_ops;

extern void tenxpress_phy_blink(struct efx_nic *efx, bool blink);


extern int sft9001_wait_boot(struct efx_nic *efx);


extern struct efx_phy_operations falcon_xfp_phy_ops;


#define QUAKE_LED_LINK_INVAL	(0)
#define QUAKE_LED_LINK_STAT	(1)
#define QUAKE_LED_LINK_ACT	(2)
#define QUAKE_LED_LINK_ACTSTAT	(3)
#define QUAKE_LED_OFF		(4)
#define QUAKE_LED_ON		(5)
#define QUAKE_LED_LINK_INPUT	(6)	

#define QUAKE_LED_TXLINK	(0)
#define QUAKE_LED_RXLINK	(8)

extern void xfp_set_led(struct efx_nic *p, int led, int state);

#endif
