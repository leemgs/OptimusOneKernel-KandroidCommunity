



#include "boot.h"
#include <linux/utsrelease.h>
#include <linux/compile.h>

const char kernel_version[] =
	UTS_RELEASE " (" LINUX_COMPILE_BY "@" LINUX_COMPILE_HOST ") "
	UTS_VERSION;
