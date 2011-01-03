

#ifndef _ASM_X86_UV_UV_HUB_H
#define _ASM_X86_UV_UV_HUB_H

#ifdef CONFIG_X86_64
#include <linux/numa.h>
#include <linux/percpu.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <asm/types.h>
#include <asm/percpu.h>
#include <asm/uv/uv_mmrs.h>
#include <asm/irq_vectors.h>
#include <asm/io_apic.h>






#define UV_MAX_NUMALINK_BLADES	16384


#define UV_MAX_SSI_BLADES	256


#define UV_MAX_NASID_VALUE	(UV_MAX_NUMALINK_BLADES * 2)

struct uv_scir_s {
	struct timer_list timer;
	unsigned long	offset;
	unsigned long	last;
	unsigned long	idle_on;
	unsigned long	idle_off;
	unsigned char	state;
	unsigned char	enabled;
};


struct uv_hub_info_s {
	unsigned long		global_mmr_base;
	unsigned long		gpa_mask;
	unsigned int		gnode_extra;
	unsigned long		gnode_upper;
	unsigned long		lowmem_remap_top;
	unsigned long		lowmem_remap_base;
	unsigned short		pnode;
	unsigned short		pnode_mask;
	unsigned short		coherency_domain_number;
	unsigned short		numa_blade_id;
	unsigned char		blade_processor_id;
	unsigned char		m_val;
	unsigned char		n_val;
	struct uv_scir_s	scir;
};

DECLARE_PER_CPU(struct uv_hub_info_s, __uv_hub_info);
#define uv_hub_info		(&__get_cpu_var(__uv_hub_info))
#define uv_cpu_hub_info(cpu)	(&per_cpu(__uv_hub_info, cpu))


#define UV_NASID_TO_PNODE(n)		(((n) >> 1) & uv_hub_info->pnode_mask)
#define UV_PNODE_TO_GNODE(p)		((p) |uv_hub_info->gnode_extra)
#define UV_PNODE_TO_NASID(p)		(UV_PNODE_TO_GNODE(p) << 1)

#define UV_LOCAL_MMR_BASE		0xf4000000UL
#define UV_GLOBAL_MMR32_BASE		0xf8000000UL
#define UV_GLOBAL_MMR64_BASE		(uv_hub_info->global_mmr_base)
#define UV_LOCAL_MMR_SIZE		(64UL * 1024 * 1024)
#define UV_GLOBAL_MMR32_SIZE		(64UL * 1024 * 1024)

#define UV_GLOBAL_MMR32_PNODE_SHIFT	15
#define UV_GLOBAL_MMR64_PNODE_SHIFT	26

#define UV_GLOBAL_MMR32_PNODE_BITS(p)	((p) << (UV_GLOBAL_MMR32_PNODE_SHIFT))

#define UV_GLOBAL_MMR64_PNODE_BITS(p)					\
	(((unsigned long)(p)) << UV_GLOBAL_MMR64_PNODE_SHIFT)

#define UV_APIC_PNODE_SHIFT	6


#define LOCAL_BUS_BASE		0x1c00000
#define LOCAL_BUS_SIZE		(4 * 1024 * 1024)


#define SCIR_WINDOW_COUNT	64
#define SCIR_LOCAL_MMR_BASE	(LOCAL_BUS_BASE + \
				 LOCAL_BUS_SIZE - \
				 SCIR_WINDOW_COUNT)

#define SCIR_CPU_HEARTBEAT	0x01	
#define SCIR_CPU_ACTIVITY	0x02	
#define SCIR_CPU_HB_INTERVAL	(HZ)	


#define for_each_possible_blade(bid)		\
	for ((bid) = 0; (bid) < uv_num_possible_blades(); (bid)++)




static inline unsigned long uv_soc_phys_ram_to_gpa(unsigned long paddr)
{
	if (paddr < uv_hub_info->lowmem_remap_top)
		paddr |= uv_hub_info->lowmem_remap_base;
	return paddr | uv_hub_info->gnode_upper;
}



static inline unsigned long uv_gpa(void *v)
{
	return uv_soc_phys_ram_to_gpa(__pa(v));
}


static inline unsigned long uv_gpa_to_gnode(unsigned long gpa)
{
	return gpa >> uv_hub_info->m_val;
}


static inline int uv_gpa_to_pnode(unsigned long gpa)
{
	unsigned long n_mask = (1UL << uv_hub_info->n_val) - 1;

	return uv_gpa_to_gnode(gpa) & n_mask;
}


static inline void *uv_pnode_offset_to_vaddr(int pnode, unsigned long offset)
{
	return __va(((unsigned long)pnode << uv_hub_info->m_val) | offset);
}



static inline int uv_apicid_to_pnode(int apicid)
{
	return (apicid >> UV_APIC_PNODE_SHIFT);
}


static inline unsigned long *uv_global_mmr32_address(int pnode, unsigned long offset)
{
	return __va(UV_GLOBAL_MMR32_BASE |
		       UV_GLOBAL_MMR32_PNODE_BITS(pnode) | offset);
}

static inline void uv_write_global_mmr32(int pnode, unsigned long offset, unsigned long val)
{
	writeq(val, uv_global_mmr32_address(pnode, offset));
}

static inline unsigned long uv_read_global_mmr32(int pnode, unsigned long offset)
{
	return readq(uv_global_mmr32_address(pnode, offset));
}


static inline unsigned long *uv_global_mmr64_address(int pnode, unsigned long offset)
{
	return __va(UV_GLOBAL_MMR64_BASE |
		    UV_GLOBAL_MMR64_PNODE_BITS(pnode) | offset);
}

static inline void uv_write_global_mmr64(int pnode, unsigned long offset, unsigned long val)
{
	writeq(val, uv_global_mmr64_address(pnode, offset));
}

static inline unsigned long uv_read_global_mmr64(int pnode, unsigned long offset)
{
	return readq(uv_global_mmr64_address(pnode, offset));
}

static inline void uv_write_global_mmr8(int pnode, unsigned long offset, unsigned char val)
{
	writeb(val, uv_global_mmr64_address(pnode, offset));
}

static inline unsigned char uv_read_global_mmr8(int pnode, unsigned long offset)
{
	return readb(uv_global_mmr64_address(pnode, offset));
}


static inline unsigned long *uv_local_mmr_address(unsigned long offset)
{
	return __va(UV_LOCAL_MMR_BASE | offset);
}

static inline unsigned long uv_read_local_mmr(unsigned long offset)
{
	return readq(uv_local_mmr_address(offset));
}

static inline void uv_write_local_mmr(unsigned long offset, unsigned long val)
{
	writeq(val, uv_local_mmr_address(offset));
}

static inline unsigned char uv_read_local_mmr8(unsigned long offset)
{
	return readb(uv_local_mmr_address(offset));
}

static inline void uv_write_local_mmr8(unsigned long offset, unsigned char val)
{
	writeb(val, uv_local_mmr_address(offset));
}


struct uv_blade_info {
	unsigned short	nr_possible_cpus;
	unsigned short	nr_online_cpus;
	unsigned short	pnode;
	short		memory_nid;
};
extern struct uv_blade_info *uv_blade_info;
extern short *uv_node_to_blade;
extern short *uv_cpu_to_blade;
extern short uv_possible_blades;


static inline int uv_blade_processor_id(void)
{
	return uv_hub_info->blade_processor_id;
}


static inline int uv_numa_blade_id(void)
{
	return uv_hub_info->numa_blade_id;
}


static inline int uv_cpu_to_blade_id(int cpu)
{
	return uv_cpu_to_blade[cpu];
}


static inline int uv_node_to_blade_id(int nid)
{
	return uv_node_to_blade[nid];
}


static inline int uv_blade_to_pnode(int bid)
{
	return uv_blade_info[bid].pnode;
}


static inline int uv_blade_to_memory_nid(int bid)
{
	return uv_blade_info[bid].memory_nid;
}


static inline int uv_blade_nr_possible_cpus(int bid)
{
	return uv_blade_info[bid].nr_possible_cpus;
}


static inline int uv_blade_nr_online_cpus(int bid)
{
	return uv_blade_info[bid].nr_online_cpus;
}


static inline int uv_cpu_to_pnode(int cpu)
{
	return uv_blade_info[uv_cpu_to_blade_id(cpu)].pnode;
}


static inline int uv_node_to_pnode(int nid)
{
	return uv_blade_info[uv_node_to_blade_id(nid)].pnode;
}


static inline int uv_num_possible_blades(void)
{
	return uv_possible_blades;
}


static inline void uv_set_scir_bits(unsigned char value)
{
	if (uv_hub_info->scir.state != value) {
		uv_hub_info->scir.state = value;
		uv_write_local_mmr8(uv_hub_info->scir.offset, value);
	}
}

static inline unsigned long uv_scir_offset(int apicid)
{
	return SCIR_LOCAL_MMR_BASE | (apicid & 0x3f);
}

static inline void uv_set_cpu_scir_bits(int cpu, unsigned char value)
{
	if (uv_cpu_hub_info(cpu)->scir.state != value) {
		uv_write_global_mmr8(uv_cpu_to_pnode(cpu),
				uv_cpu_hub_info(cpu)->scir.offset, value);
		uv_cpu_hub_info(cpu)->scir.state = value;
	}
}

static inline void uv_hub_send_ipi(int pnode, int apicid, int vector)
{
	unsigned long val;
	unsigned long dmode = dest_Fixed;

	if (vector == NMI_VECTOR)
		dmode = dest_NMI;

	val = (1UL << UVH_IPI_INT_SEND_SHFT) |
			((apicid) << UVH_IPI_INT_APIC_ID_SHFT) |
			(dmode << UVH_IPI_INT_DELIVERY_MODE_SHFT) |
			(vector << UVH_IPI_INT_VECTOR_SHFT);
	uv_write_global_mmr64(pnode, UVH_IPI_INT, val);
}

#endif 
#endif 
