


#ifndef __ASM_ARM_ARCH_TPA2018D1_H
#define __ASM_ARM_ARCH_TPA2018D1_H

#define TPA2018D1_I2C_NAME "tpa2018d1"
#define TPA2018D1_CMD_LEN 8

struct tpa2018d1_platform_data {
	uint32_t gpio_tpa2018_spk_en;
};

struct tpa2018d1_config_data {
	unsigned char *cmd_data;  
	unsigned int mode_num;
	unsigned int data_len;
};

extern void tpa2018d1_set_speaker_amp(int on);

#endif 
