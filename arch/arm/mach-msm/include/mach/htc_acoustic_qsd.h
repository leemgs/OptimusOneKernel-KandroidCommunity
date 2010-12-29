
#ifndef _ARCH_ARM_MACH_MSM_HTC_ACOUSTIC_QSD_H_
#define _ARCH_ARM_MACH_MSM_HTC_ACOUSTIC_QSD_H_

struct qsd_acoustic_ops {
	void (*enable_mic_bias)(int en);
};

void acoustic_register_ops(struct qsd_acoustic_ops *ops);

int turn_mic_bias_on(int on);
int force_headset_speaker_on(int enable);
int enable_aux_loopback(uint32_t enable);

#endif

