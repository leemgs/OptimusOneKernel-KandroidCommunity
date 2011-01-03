
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__

#include    "rtmp_type.h"
#ifdef LINUX
#include	"rt_linux.h"
#endif
#include    "rtmp_def.h"
#include    "rt28xx.h"

#ifdef RT2860
#include	"rt2860.h"
#endif
#ifdef RT2870
#include	"../rt2870/rt2870.h"
#endif 

#include    "oid.h"
#include    "mlme.h"
#include    "wpa.h"
#include    "md5.h"
#include    "rtmp.h"
#include	"ap.h"
#include	"dfs.h"
#include	"chlist.h"
#include	"spectrum.h"

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif 

#endif	

