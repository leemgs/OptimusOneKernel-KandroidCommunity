
#ifndef _ASM_X86_TOPOLOGY_H
#define _ASM_X86_TOPOLOGY_H

#ifdef CONFIG_X86_32
# ifdef CONFIG_X86_HT
#  define ENABLE_TOPO_DEFINES
# endif
#else
# ifdef CONFIG_SMP
#  define ENABLE_TOPO_DEFINES
# endif
#endif


#define NUMA_NO_NODE	(-1)

#ifdef CONFIG_NUMA
#include <linux/cpumask.h>
#include <asm/mpspec.h>

#ifdef CONFIG_X86_32


extern int cpu_to_node_map[];


static inline int cpu_to_node(int cpu)
{
	return cpu_to_node_map[cpu];
}
#define early_cpu_to_node(cpu)	cpu_to_node(cpu)

#else 


DECLARE_EARLY_PER_CPU(int, x86_cpu_to_node_map);


DECLARE_PER_CPU(int, node_number);
#define numa_node_id()		percpu_read(node_number)

#ifdef CONFIG_DEBUG_PER_CPU_MAPS
extern int cpu_to_node(int cpu);
extern int early_cpu_to_node(int cpu);

#else	


static inline int cpu_to_node(int cpu)
{
	return per_cpu(x86_cpu_to_node_map, cpu);
}


static inline int early_cpu_to_node(int cpu)
{
	return early_per_cpu(x86_cpu_to_node_map, cpu);
}

#endif 

#endif 


extern cpumask_var_t node_to_cpumask_map[MAX_NUMNODES];

#ifdef CONFIG_DEBUG_PER_CPU_MAPS
extern const struct cpumask *cpumask_of_node(int node);
#else

static inline const struct cpumask *cpumask_of_node(int node)
{
	return node_to_cpumask_map[node];
}
#endif

extern void setup_node_to_cpumask_map(void);


#define parent_node(node) (node)

#define pcibus_to_node(bus) __pcibus_to_node(bus)

#ifdef CONFIG_X86_32
extern unsigned long node_start_pfn[];
extern unsigned long node_end_pfn[];
extern unsigned long node_remap_size[];
#define node_has_online_mem(nid) (node_start_pfn[nid] != node_end_pfn[nid])

# define SD_CACHE_NICE_TRIES	1
# define SD_IDLE_IDX		1

#else

# define SD_CACHE_NICE_TRIES	2
# define SD_IDLE_IDX		2

#endif


#define SD_NODE_INIT (struct sched_domain) {				\
	.min_interval		= 8,					\
	.max_interval		= 32,					\
	.busy_factor		= 32,					\
	.imbalance_pct		= 125,					\
	.cache_nice_tries	= SD_CACHE_NICE_TRIES,			\
	.busy_idx		= 3,					\
	.idle_idx		= SD_IDLE_IDX,				\
	.newidle_idx		= 0,					\
	.wake_idx		= 0,					\
	.forkexec_idx		= 0,					\
									\
	.flags			= 1*SD_LOAD_BALANCE			\
				| 1*SD_BALANCE_NEWIDLE			\
				| 1*SD_BALANCE_EXEC			\
				| 1*SD_BALANCE_FORK			\
				| 0*SD_BALANCE_WAKE			\
				| 1*SD_WAKE_AFFINE			\
				| 0*SD_PREFER_LOCAL			\
				| 0*SD_SHARE_CPUPOWER			\
				| 0*SD_POWERSAVINGS_BALANCE		\
				| 0*SD_SHARE_PKG_RESOURCES		\
				| 1*SD_SERIALIZE			\
				| 0*SD_PREFER_SIBLING			\
				,					\
	.last_balance		= jiffies,				\
	.balance_interval	= 1,					\
}

#ifdef CONFIG_X86_64_ACPI_NUMA
extern int __node_distance(int, int);
#define node_distance(a, b) __node_distance(a, b)
#endif

#else 

static inline int numa_node_id(void)
{
	return 0;
}

static inline int early_cpu_to_node(int cpu)
{
	return 0;
}

static inline void setup_node_to_cpumask_map(void) { }

#endif

#include <asm-generic/topology.h>

extern const struct cpumask *cpu_coregroup_mask(int cpu);

#ifdef ENABLE_TOPO_DEFINES
#define topology_physical_package_id(cpu)	(cpu_data(cpu).phys_proc_id)
#define topology_core_id(cpu)			(cpu_data(cpu).cpu_core_id)
#define topology_core_cpumask(cpu)		(per_cpu(cpu_core_map, cpu))
#define topology_thread_cpumask(cpu)		(per_cpu(cpu_sibling_map, cpu))


#define arch_provides_topology_pointers		yes
#endif

static inline void arch_fix_phys_package_id(int num, u32 slot)
{
}

struct pci_bus;
void x86_pci_root_bus_res_quirks(struct pci_bus *b);

#ifdef CONFIG_SMP
#define mc_capable()	((boot_cpu_data.x86_max_cores > 1) && \
			(cpumask_weight(cpu_core_mask(0)) != nr_cpu_ids))
#define smt_capable()			(smp_num_siblings > 1)
#endif

#ifdef CONFIG_NUMA
extern int get_mp_bus_to_node(int busnum);
extern void set_mp_bus_to_node(int busnum, int node);
#else
static inline int get_mp_bus_to_node(int busnum)
{
	return 0;
}
static inline void set_mp_bus_to_node(int busnum, int node)
{
}
#endif

#endif 
