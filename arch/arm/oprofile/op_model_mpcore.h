
#ifndef OP_MODEL_MPCORE_H
#define OP_MODEL_MPCORE_H

struct eventmonitor {
	unsigned long PMCR;
	unsigned char MCEB[8];
	unsigned long MC[8];
};



#define COUNTER_CPU0_PMN0 0
#define COUNTER_CPU0_PMN1 1
#define COUNTER_CPU0_CCNT 2

#define COUNTER_CPU1_PMN0 3
#define COUNTER_CPU1_PMN1 4
#define COUNTER_CPU1_CCNT 5

#define COUNTER_CPU2_PMN0 6
#define COUNTER_CPU2_PMN1 7
#define COUNTER_CPU2_CCNT 8

#define COUNTER_CPU3_PMN0 9
#define COUNTER_CPU3_PMN1 10
#define COUNTER_CPU3_CCNT 11

#define COUNTER_SCU_MN0 12
#define COUNTER_SCU_MN1 13
#define COUNTER_SCU_MN2 14
#define COUNTER_SCU_MN3 15
#define COUNTER_SCU_MN4 16
#define COUNTER_SCU_MN5 17
#define COUNTER_SCU_MN6 18
#define COUNTER_SCU_MN7 19
#define NUM_SCU_COUNTERS 8

#define SCU_COUNTER(number)	((number) + COUNTER_SCU_MN0)

#define MPCORE_NUM_COUNTERS	SCU_COUNTER(NUM_SCU_COUNTERS)

#endif
