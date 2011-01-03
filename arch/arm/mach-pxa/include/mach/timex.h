



#if defined(CONFIG_PXA25x)

#define CLOCK_TICK_RATE 3686400
#elif defined(CONFIG_PXA27x)

#ifdef CONFIG_MACH_MAINSTONE
#define CLOCK_TICK_RATE 3249600
#else
#define CLOCK_TICK_RATE 3250000
#endif
#else
#define CLOCK_TICK_RATE 3250000
#endif
