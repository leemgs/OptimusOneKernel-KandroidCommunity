

#ifndef MSM_GEMINI_COMMON_H
#define MSM_GEMINI_COMMON_H

#ifdef MSM_GEMINI_DEBUG
#define GMN_DBG(fmt, args...) printk(KERN_INFO "gemini: " fmt, ##args)
#else
#define GMN_DBG(fmt, args...) do { } while (0)
#endif

#define GMN_PR_ERR   pr_err

enum GEMINI_MODE {
	GEMINI_MODE_DISABLE,
	GEMINI_MODE_OFFLINE,
	GEMINI_MODE_REALTIME,
	GEMINI_MODE_REALTIME_ROTATION
};

enum GEMINI_ROTATION {
	GEMINI_ROTATION_0,
	GEMINI_ROTATION_90,
	GEMINI_ROTATION_180,
	GEMINI_ROTATION_270
};

#endif 
