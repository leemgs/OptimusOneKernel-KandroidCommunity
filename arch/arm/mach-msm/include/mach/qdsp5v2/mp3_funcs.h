
#ifndef MP3_FUNCS_H
#define MP3_FUNCS_H


long mp3_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
void audpp_cmd_cfg_mp3_params(struct audio *audio);

#endif 
