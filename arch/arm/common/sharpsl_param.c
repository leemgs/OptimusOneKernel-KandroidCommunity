

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm/mach/sharpsl_param.h>


#ifdef CONFIG_ARCH_SA1100
#define PARAM_BASE	0xe8ffc000
#else
#define PARAM_BASE	0xa0000a00
#endif
#define MAGIC_CHG(a,b,c,d) ( ( d << 24 ) | ( c << 16 )  | ( b << 8 ) | a )

#define COMADJ_MAGIC	MAGIC_CHG('C','M','A','D')
#define UUID_MAGIC	MAGIC_CHG('U','U','I','D')
#define TOUCH_MAGIC	MAGIC_CHG('T','U','C','H')
#define AD_MAGIC	MAGIC_CHG('B','V','A','D')
#define PHAD_MAGIC	MAGIC_CHG('P','H','A','D')

struct sharpsl_param_info sharpsl_param;
EXPORT_SYMBOL(sharpsl_param);

void sharpsl_save_param(void)
{
	memcpy(&sharpsl_param, (void *)PARAM_BASE, sizeof(struct sharpsl_param_info));

	if (sharpsl_param.comadj_keyword != COMADJ_MAGIC)
		sharpsl_param.comadj=-1;

	if (sharpsl_param.phad_keyword != PHAD_MAGIC)
		sharpsl_param.phadadj=-1;

	if (sharpsl_param.uuid_keyword != UUID_MAGIC)
		sharpsl_param.uuid[0]=-1;

	if (sharpsl_param.touch_keyword != TOUCH_MAGIC)
		sharpsl_param.touch_xp=-1;

	if (sharpsl_param.adadj_keyword != AD_MAGIC)
		sharpsl_param.adadj=-1;
}


