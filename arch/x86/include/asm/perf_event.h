#ifndef _ASM_X86_PERF_EVENT_H
#define _ASM_X86_PERF_EVENT_H



#define X86_PMC_MAX_GENERIC					8
#define X86_PMC_MAX_FIXED					3

#define X86_PMC_IDX_GENERIC				        0
#define X86_PMC_IDX_FIXED				       32
#define X86_PMC_IDX_MAX					       64

#define MSR_ARCH_PERFMON_PERFCTR0			      0xc1
#define MSR_ARCH_PERFMON_PERFCTR1			      0xc2

#define MSR_ARCH_PERFMON_EVENTSEL0			     0x186
#define MSR_ARCH_PERFMON_EVENTSEL1			     0x187

#define ARCH_PERFMON_EVENTSEL0_ENABLE			  (1 << 22)
#define ARCH_PERFMON_EVENTSEL_INT			  (1 << 20)
#define ARCH_PERFMON_EVENTSEL_OS			  (1 << 17)
#define ARCH_PERFMON_EVENTSEL_USR			  (1 << 16)


#define ARCH_PERFMON_EVENT_MASK				    0xffff

#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_SEL		      0x3c
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_UMASK		(0x00 << 8)
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_INDEX 		 0
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_PRESENT \
		(1 << (ARCH_PERFMON_UNHALTED_CORE_CYCLES_INDEX))

#define ARCH_PERFMON_BRANCH_MISSES_RETIRED			 6


union cpuid10_eax {
	struct {
		unsigned int version_id:8;
		unsigned int num_events:8;
		unsigned int bit_width:8;
		unsigned int mask_length:8;
	} split;
	unsigned int full;
};

union cpuid10_edx {
	struct {
		unsigned int num_events_fixed:4;
		unsigned int reserved:28;
	} split;
	unsigned int full;
};





#define MSR_ARCH_PERFMON_FIXED_CTR_CTRL			0x38d




#define MSR_ARCH_PERFMON_FIXED_CTR0			0x309
#define X86_PMC_IDX_FIXED_INSTRUCTIONS			(X86_PMC_IDX_FIXED + 0)


#define MSR_ARCH_PERFMON_FIXED_CTR1			0x30a
#define X86_PMC_IDX_FIXED_CPU_CYCLES			(X86_PMC_IDX_FIXED + 1)


#define MSR_ARCH_PERFMON_FIXED_CTR2			0x30b
#define X86_PMC_IDX_FIXED_BUS_CYCLES			(X86_PMC_IDX_FIXED + 2)


#define X86_PMC_IDX_FIXED_BTS				(X86_PMC_IDX_FIXED + 16)


#ifdef CONFIG_PERF_EVENTS
extern void init_hw_perf_events(void);
extern void perf_events_lapic_init(void);

#define PERF_EVENT_INDEX_OFFSET			0

#else
static inline void init_hw_perf_events(void)		{ }
static inline void perf_events_lapic_init(void)	{ }
#endif

#endif 
