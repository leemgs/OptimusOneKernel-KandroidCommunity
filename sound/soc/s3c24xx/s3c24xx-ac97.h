

#ifndef S3C24XXAC97_H_
#define S3C24XXAC97_H_

#define AC_CMD_ADDR(x) (x << 16)
#define AC_CMD_DATA(x) (x & 0xffff)

extern struct snd_soc_dai s3c2443_ac97_dai[];

#endif 
