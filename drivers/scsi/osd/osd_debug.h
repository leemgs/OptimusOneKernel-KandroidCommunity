
#ifndef __OSD_DEBUG_H__
#define __OSD_DEBUG_H__

#define OSD_ERR(fmt, a...) printk(KERN_ERR "osd: " fmt, ##a)
#define OSD_INFO(fmt, a...) printk(KERN_NOTICE "osd: " fmt, ##a)

#ifdef CONFIG_SCSI_OSD_DEBUG
#define OSD_DEBUG(fmt, a...) \
	printk(KERN_NOTICE "osd @%s:%d: " fmt, __func__, __LINE__, ##a)
#else
#define OSD_DEBUG(fmt, a...) do {} while (0)
#endif


#define _LLU(x) (unsigned long long)(x)

#endif 
