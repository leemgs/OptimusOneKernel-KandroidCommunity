


#ifndef		_OSD_DEFS_H
#define		_OSD_DEFS_H






  
#if (defined(__linux__))
# define _DPT_LINUX
#elif (defined(__bsdi__))
# define _DPT_BSDI
#elif (defined(__FreeBSD__))
# define _DPT_FREE_BSD
#else
# define _DPT_SCO
#endif

#if defined (ZIL_CURSES)
#define		_DPT_CURSES
#else
#define         _DPT_MOTIF
#endif

  
#define		far

  
#define		SEMAPHORE_T	unsigned int *
  
#define		DLL_HANDLE_T	unsigned int *

#endif
