
#ifndef PCM_FUNCS_H
#define PCM_FUNCS_H

long pcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
void audpp_cmd_cfg_pcm_params(struct audio *audio);

#endif 
