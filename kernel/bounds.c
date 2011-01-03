

#define __GENERATING_BOUNDS_H

#include <linux/page-flags.h>
#include <linux/mmzone.h>
#include <linux/kbuild.h>

void foo(void)
{
	
	DEFINE(NR_PAGEFLAGS, __NR_PAGEFLAGS);
	DEFINE(MAX_NR_ZONES, __MAX_NR_ZONES);
	
}
