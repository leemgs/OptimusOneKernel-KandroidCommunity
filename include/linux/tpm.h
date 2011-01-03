
#ifndef __LINUX_TPM_H__
#define __LINUX_TPM_H__


#define	TPM_ANY_NUM 0xFFFF

#if defined(CONFIG_TCG_TPM)

extern int tpm_pcr_read(u32 chip_num, int pcr_idx, u8 *res_buf);
extern int tpm_pcr_extend(u32 chip_num, int pcr_idx, const u8 *hash);
#endif
#endif
