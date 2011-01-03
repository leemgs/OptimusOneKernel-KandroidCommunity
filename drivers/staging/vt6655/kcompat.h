

#ifndef _KCOMPAT_H
#define _KCOMPAT_H

#include <linux/version.h>

#ifndef HAVE_NETDEV_PRIV
#define netdev_priv(dev) (dev->priv)
#endif

#endif

