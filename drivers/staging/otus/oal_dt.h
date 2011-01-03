










#ifndef _OAL_DT_H
#define _OAL_DT_H



#include <linux/netdevice.h>

typedef     unsigned long long  u64_t;
typedef     unsigned int        u32_t;
typedef     unsigned short      u16_t;
typedef     unsigned char       u8_t;
typedef     long long           s64_t;
typedef     long                s32_t;
typedef     short               s16_t;
typedef     char                s8_t;

#ifndef     TRUE
#define     TRUE                (1==1)
#endif

#ifndef     FALSE
#define     FALSE               (1==0)
#endif

#ifndef     NULL
#define     NULL                0
#endif


typedef     struct sk_buff      zbuf_t;


typedef     struct net_device   zdev_t;

#endif 
