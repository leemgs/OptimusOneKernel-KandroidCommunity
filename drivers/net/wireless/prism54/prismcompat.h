



#ifdef PRISM54_COMPAT24
#include "prismcompat24.h"
#else	

#ifndef _PRISM_COMPAT_H
#define _PRISM_COMPAT_H

#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/moduleparam.h>
#include <linux/workqueue.h>
#include <linux/compiler.h>

#ifndef __iomem
#define __iomem
#endif

#define PRISM_FW_PDEV		&priv->pdev->dev

#endif				
#endif				
