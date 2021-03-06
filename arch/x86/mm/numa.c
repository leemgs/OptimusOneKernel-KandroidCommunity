
#include <linux/topology.h>
#include <linux/module.h>
#include <linux/bootmem.h>

#ifdef CONFIG_DEBUG_PER_CPU_MAPS
# define DBG(x...) printk(KERN_DEBUG x)
#else
# define DBG(x...)
#endif


cpumask_var_t node_to_cpumask_map[MAX_NUMNODES];
EXPORT_SYMBOL(node_to_cpumask_map);


void __init setup_node_to_cpumask_map(void)
{
	unsigned int node, num = 0;

	
	if (nr_node_ids == MAX_NUMNODES) {
		for_each_node_mask(node, node_possible_map)
			num = node;
		nr_node_ids = num + 1;
	}

	
	for (node = 0; node < nr_node_ids; node++)
		alloc_bootmem_cpumask_var(&node_to_cpumask_map[node]);

	
	pr_debug("Node to cpumask map for %d nodes\n", nr_node_ids);
}

#ifdef CONFIG_DEBUG_PER_CPU_MAPS

const struct cpumask *cpumask_of_node(int node)
{
	if (node >= nr_node_ids) {
		printk(KERN_WARNING
			"cpumask_of_node(%d): node > nr_node_ids(%d)\n",
			node, nr_node_ids);
		dump_stack();
		return cpu_none_mask;
	}
	if (node_to_cpumask_map[node] == NULL) {
		printk(KERN_WARNING
			"cpumask_of_node(%d): no node_to_cpumask_map!\n",
			node);
		dump_stack();
		return cpu_online_mask;
	}
	return node_to_cpumask_map[node];
}
EXPORT_SYMBOL(cpumask_of_node);
#endif
