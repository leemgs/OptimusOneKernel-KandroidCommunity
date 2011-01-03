
#ifndef OP_MODEL_ARM11_CORE_H
#define OP_MODEL_ARM11_CORE_H


#define PMCR_E		(1 << 0)	
#define PMCR_P		(1 << 1)	
#define PMCR_C		(1 << 2)	
#define PMCR_D		(1 << 3)	
#define PMCR_IEN_PMN0	(1 << 4)	
#define PMCR_IEN_PMN1	(1 << 5)	
#define PMCR_IEN_CCNT	(1 << 6)	
#define PMCR_OFL_PMN0	(1 << 8)	
#define PMCR_OFL_PMN1	(1 << 9)	
#define PMCR_OFL_CCNT	(1 << 10)	

#define PMN0 0
#define PMN1 1
#define CCNT 2

#define CPU_COUNTER(cpu, counter)	((cpu) * 3 + (counter))

int arm11_setup_pmu(void);
int arm11_start_pmu(void);
int arm11_stop_pmu(void);
int arm11_request_interrupts(int *, int);
void arm11_release_interrupts(int *, int);

#endif
