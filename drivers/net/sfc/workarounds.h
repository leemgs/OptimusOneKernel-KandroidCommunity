

#ifndef EFX_WORKAROUNDS_H
#define EFX_WORKAROUNDS_H



#define EFX_WORKAROUND_ALWAYS(efx) 1
#define EFX_WORKAROUND_FALCON_A(efx) (falcon_rev(efx) <= FALCON_REV_A1)
#define EFX_WORKAROUND_10G(efx) EFX_IS10G(efx)
#define EFX_WORKAROUND_SFT9001(efx) ((efx)->phy_type == PHY_TYPE_SFT9001A || \
				     (efx)->phy_type == PHY_TYPE_SFT9001B)


#define EFX_WORKAROUND_5147 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_7575 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_7884 EFX_WORKAROUND_10G

#define EFX_WORKAROUND_9141 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_10727 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_11482 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_11557 EFX_WORKAROUND_ALWAYS

#define EFX_WORKAROUND_15592 EFX_WORKAROUND_ALWAYS


#define EFX_WORKAROUND_5129 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_5583 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_5676 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_6555 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_7244 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_7803 EFX_WORKAROUND_FALCON_A

#define EFX_WORKAROUND_8071 EFX_WORKAROUND_FALCON_A


#define EFX_WORKAROUND_13204 EFX_WORKAROUND_SFT9001

#define EFX_WORKAROUND_15195 EFX_WORKAROUND_SFT9001

#endif 
