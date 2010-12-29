

#ifndef _MACH_MSM_QDSP6_ADIE_
#define _MACH_MSM_QDSP6_ADIE_

#include "../dal.h"

#define ADIE_DAL_DEVICE		0x02000029
#define ADIE_DAL_PORT		"DAL_AM_AUD"
#define ADIE_DAL_VERSION	0x00010000

enum {
	ADIE_OP_SET_PATH =  DAL_OP_FIRST_DEVICE_API,
	ADIE_OP_PROCEED_TO_STAGE,
	ADIE_OP_IOCTL
};


#define ADIE_PATH_HANDSET_TX			0x010740f6
#define ADIE_PATH_HANDSET_RX			0x010740f7
#define ADIE_PATH_HEADSET_MONO_TX		0x010740f8
#define ADIE_PATH_HEADSET_STEREO_TX		0x010740f9
#define ADIE_PATH_HEADSET_MONO_RX		0x010740fa
#define ADIE_PATH_HEADSET_STEREO_RX		0x010740fb
#define ADIE_PATH_SPEAKER_TX			0x010740fc
#define ADIE_PATH_SPEAKER_RX			0x010740fd
#define ADIE_PATH_SPEAKER_STEREO_RX		0x01074101


#define ADIE_PATH_TTY_HEADSET_TX		0x010740fe
#define ADIE_PATH_TTY_HEADSET_RX		0x010740ff


#define ADIE_PATH_FTM_MIC1_TX			0x01074108
#define ADIE_PATH_FTM_MIC2_TX			0x01074107
#define ADIE_PATH_FTM_HPH_L_RX			0x01074106
#define ADIE_PATH_FTM_HPH_R_RX			0x01074104
#define ADIE_PATH_FTM_EAR_RX			0x01074103
#define ADIE_PATH_FTM_SPKR_RX			0x01074102



#define ADIE_PATH_AUXPGA_LINEOUT_STEREO_LB	0x01074100

#define ADIE_PATH_AUXPGA_LINEOUT_MONO_LB	0x01073d82

#define ADIE_PATH_AUXPGA_HDPH_STEREO_LB		0x01074109

#define ADIE_PATH_AUXPGA_HDPH_MONO_LB		0x01073d85

#define ADIE_PATH_AUXPGA_EAP_LB			0x01073d81

#define ADIE_PATH_AUXPGA_AUXOUT_LB		0x01073d86


#define ADIE_PATH_SPKR_STEREO_HDPH_MONO_RX	0x01073d83
#define ADIE_PATH_SPKR_MONO_HDPH_MONO_RX	0x01073d84
#define ADIE_PATH_SPKR_MONO_HDPH_STEREO_RX	0x01073d88
#define ADIE_PATH_SPKR_STEREO_HDPH_STEREO_RX	0x01073d89


#define ADIE_STAGE_PATH_OFF			0x0050
#define ADIE_STAGE_DIGITAL_READY		0x0100
#define ADIE_STAGE_DIGITAL_ANALOG_READY		0x1000
#define ADIE_STAGE_ANALOG_OFF			0x0750
#define ADIE_STAGE_DIGITAL_OFF			0x0600


#define ADIE_PATH_RX		0
#define ADIE_PATH_TX		1
#define ADIE_PATH_LOOPBACK	2


#define ADIE_MUTE_OFF		0
#define ADIE_MUTE_ON		1


#endif
