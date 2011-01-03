

#ifndef _CTCM_DBUG_H_
#define _CTCM_DBUG_H_



#include <asm/debug.h>

#ifdef DEBUG
 #define do_debug 1
#else
 #define do_debug 0
#endif
#ifdef DEBUGCCW
 #define do_debug_ccw 1
 #define DEBUGDATA 1
#else
 #define do_debug_ccw 0
#endif
#ifdef DEBUGDATA
 #define do_debug_data 1
#else
 #define do_debug_data 0
#endif


#define	CTC_DBF_ALWAYS	0	
#define	CTC_DBF_EMERG	0	
#define	CTC_DBF_ALERT	1	
#define	CTC_DBF_CRIT	2	
#define	CTC_DBF_ERROR	3	
#define	CTC_DBF_WARN	4	
#define	CTC_DBF_NOTICE	5	
#define	CTC_DBF_INFO	5	
#define	CTC_DBF_DEBUG	6	

enum ctcm_dbf_names {
	CTCM_DBF_SETUP,
	CTCM_DBF_ERROR,
	CTCM_DBF_TRACE,
	CTCM_DBF_MPC_SETUP,
	CTCM_DBF_MPC_ERROR,
	CTCM_DBF_MPC_TRACE,
	CTCM_DBF_INFOS	
};

struct ctcm_dbf_info {
	char name[DEBUG_MAX_NAME_LEN];
	int pages;
	int areas;
	int len;
	int level;
	debug_info_t *id;
};

extern struct ctcm_dbf_info ctcm_dbf[CTCM_DBF_INFOS];

int ctcm_register_dbf_views(void);
void ctcm_unregister_dbf_views(void);
void ctcm_dbf_longtext(enum ctcm_dbf_names dbf_nix, int level, char *text, ...);

static inline const char *strtail(const char *s, int n)
{
	int l = strlen(s);
	return (l > n) ? s + (l - n) : s;
}

#define CTCM_FUNTAIL strtail((char *)__func__, 16)

#define CTCM_DBF_TEXT(name, level, text) \
	do { \
		debug_text_event(ctcm_dbf[CTCM_DBF_##name].id, level, text); \
	} while (0)

#define CTCM_DBF_HEX(name, level, addr, len) \
	do { \
		debug_event(ctcm_dbf[CTCM_DBF_##name].id, \
					level, (void *)(addr), len); \
	} while (0)

#define CTCM_DBF_TEXT_(name, level, text...) \
	ctcm_dbf_longtext(CTCM_DBF_##name, level, text)


#define CTCM_DBF_DEV_NAME(cat, dev, text) \
	do { \
		CTCM_DBF_TEXT_(cat, CTC_DBF_INFO, "%s(%s) :- %s", \
			CTCM_FUNTAIL, dev->name, text); \
	} while (0)

#define MPC_DBF_DEV_NAME(cat, dev, text) \
	do { \
		CTCM_DBF_TEXT_(MPC_##cat, CTC_DBF_INFO, "%s(%s) := %s", \
			CTCM_FUNTAIL, dev->name, text); \
	} while (0)

#define CTCMY_DBF_DEV_NAME(cat, dev, text) \
	do { \
		if (IS_MPCDEV(dev)) \
			MPC_DBF_DEV_NAME(cat, dev, text); \
		else \
			CTCM_DBF_DEV_NAME(cat, dev, text); \
	} while (0)


#define CTCM_DBF_DEV(cat, dev, text) \
	do { \
		CTCM_DBF_TEXT_(cat, CTC_DBF_INFO, "%s(%p) :-: %s", \
			CTCM_FUNTAIL, dev, text); \
	} while (0)

#define MPC_DBF_DEV(cat, dev, text) \
	do { \
		CTCM_DBF_TEXT_(MPC_##cat, CTC_DBF_INFO, "%s(%p) :=: %s", \
			CTCM_FUNTAIL, dev, text); \
	} while (0)

#define CTCMY_DBF_DEV(cat, dev, text) \
	do { \
		if (IS_MPCDEV(dev)) \
			MPC_DBF_DEV(cat, dev, text); \
		else \
			CTCM_DBF_DEV(cat, dev, text); \
	} while (0)

#endif
