#ifndef _LINUX_IRQ_H
#define _LINUX_IRQ_H



#include <linux/smp.h>

#ifndef CONFIG_S390

#include <linux/linkage.h>
#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/cpumask.h>
#include <linux/gfp.h>
#include <linux/irqreturn.h>
#include <linux/irqnr.h>
#include <linux/errno.h>
#include <linux/topology.h>
#include <linux/wait.h>

#include <asm/irq.h>
#include <asm/ptrace.h>
#include <asm/irq_regs.h>

struct irq_desc;
typedef	void (*irq_flow_handler_t)(unsigned int irq,
					    struct irq_desc *desc);



#define IRQ_TYPE_NONE		0x00000000	
#define IRQ_TYPE_EDGE_RISING	0x00000001	
#define IRQ_TYPE_EDGE_FALLING	0x00000002	
#define IRQ_TYPE_EDGE_BOTH (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH	0x00000004	
#define IRQ_TYPE_LEVEL_LOW	0x00000008	
#define IRQ_TYPE_SENSE_MASK	0x0000000f	
#define IRQ_TYPE_PROBE		0x00000010	


#define IRQ_INPROGRESS		0x00000100	
#define IRQ_DISABLED		0x00000200	
#define IRQ_PENDING		0x00000400	
#define IRQ_REPLAY		0x00000800	
#define IRQ_AUTODETECT		0x00001000	
#define IRQ_WAITING		0x00002000	
#define IRQ_LEVEL		0x00004000	
#define IRQ_MASKED		0x00008000	
#define IRQ_PER_CPU		0x00010000	
#define IRQ_NOPROBE		0x00020000	
#define IRQ_NOREQUEST		0x00040000	
#define IRQ_NOAUTOEN		0x00080000	
#define IRQ_WAKEUP		0x00100000	
#define IRQ_MOVE_PENDING	0x00200000	
#define IRQ_NO_BALANCING	0x00400000	
#define IRQ_SPURIOUS_DISABLED	0x00800000	
#define IRQ_MOVE_PCNTXT		0x01000000	
#define IRQ_AFFINITY_SET	0x02000000	
#define IRQ_SUSPENDED		0x04000000	
#define IRQ_ONESHOT		0x08000000	
#define IRQ_NESTED_THREAD	0x10000000	

#ifdef CONFIG_IRQ_PER_CPU
# define CHECK_IRQ_PER_CPU(var) ((var) & IRQ_PER_CPU)
# define IRQ_NO_BALANCING_MASK	(IRQ_PER_CPU | IRQ_NO_BALANCING)
#else
# define CHECK_IRQ_PER_CPU(var) 0
# define IRQ_NO_BALANCING_MASK	IRQ_NO_BALANCING
#endif

struct proc_dir_entry;
struct msi_desc;


struct irq_chip {
	const char	*name;
	unsigned int	(*startup)(unsigned int irq);
	void		(*shutdown)(unsigned int irq);
	void		(*enable)(unsigned int irq);
	void		(*disable)(unsigned int irq);

	void		(*ack)(unsigned int irq);
	void		(*mask)(unsigned int irq);
	void		(*mask_ack)(unsigned int irq);
	void		(*unmask)(unsigned int irq);
	void		(*eoi)(unsigned int irq);

	void		(*end)(unsigned int irq);
	int		(*set_affinity)(unsigned int irq,
					const struct cpumask *dest);
	int		(*retrigger)(unsigned int irq);
	int		(*set_type)(unsigned int irq, unsigned int flow_type);
	int		(*set_wake)(unsigned int irq, unsigned int on);

	void		(*bus_lock)(unsigned int irq);
	void		(*bus_sync_unlock)(unsigned int irq);

	
#ifdef CONFIG_IRQ_RELEASE_METHOD
	void		(*release)(unsigned int irq, void *dev_id);
#endif
	
	const char	*typename;
};

struct timer_rand_state;
struct irq_2_iommu;

struct irq_desc {
	unsigned int		irq;
	struct timer_rand_state *timer_rand_state;
	unsigned int            *kstat_irqs;
#ifdef CONFIG_INTR_REMAP
	struct irq_2_iommu      *irq_2_iommu;
#endif
	irq_flow_handler_t	handle_irq;
	struct irq_chip		*chip;
	struct msi_desc		*msi_desc;
	void			*handler_data;
	void			*chip_data;
	struct irqaction	*action;	
	unsigned int		status;		

	unsigned int		depth;		
	unsigned int		wake_depth;	
	unsigned int		irq_count;	
	unsigned long		last_unhandled;	
	unsigned int		irqs_unhandled;
	spinlock_t		lock;
#ifdef CONFIG_SMP
	cpumask_var_t		affinity;
	unsigned int		node;
#ifdef CONFIG_GENERIC_PENDING_IRQ
	cpumask_var_t		pending_mask;
#endif
#endif
	atomic_t		threads_active;
	wait_queue_head_t       wait_for_threads;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry	*dir;
#endif
	const char		*name;
} ____cacheline_internodealigned_in_smp;

extern void arch_init_copy_chip_data(struct irq_desc *old_desc,
					struct irq_desc *desc, int node);
extern void arch_free_chip_data(struct irq_desc *old_desc, struct irq_desc *desc);

#ifndef CONFIG_SPARSE_IRQ
extern struct irq_desc irq_desc[NR_IRQS];
#endif

#ifdef CONFIG_NUMA_IRQ_DESC
extern struct irq_desc *move_irq_desc(struct irq_desc *old_desc, int node);
#else
static inline struct irq_desc *move_irq_desc(struct irq_desc *desc, int node)
{
	return desc;
}
#endif

extern struct irq_desc *irq_to_desc_alloc_node(unsigned int irq, int node);


#include <asm/hw_irq.h>

extern int setup_irq(unsigned int irq, struct irqaction *new);
extern void remove_irq(unsigned int irq, struct irqaction *act);

#ifdef CONFIG_GENERIC_HARDIRQS

#ifdef CONFIG_SMP

#ifdef CONFIG_GENERIC_PENDING_IRQ

void move_native_irq(int irq);
void move_masked_irq(int irq);

#else 

static inline void move_irq(int irq)
{
}

static inline void move_native_irq(int irq)
{
}

static inline void move_masked_irq(int irq)
{
}

#endif 

#else 

#define move_native_irq(x)
#define move_masked_irq(x)

#endif 

extern int no_irq_affinity;

static inline int irq_balancing_disabled(unsigned int irq)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	return desc->status & IRQ_NO_BALANCING_MASK;
}


extern irqreturn_t handle_IRQ_event(unsigned int irq, struct irqaction *action);


extern void handle_level_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_fasteoi_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_edge_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_simple_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_percpu_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_bad_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_nested_irq(unsigned int irq);


#ifndef CONFIG_GENERIC_HARDIRQS_NO__DO_IRQ
extern unsigned int __do_IRQ(unsigned int irq);
#endif


static inline void generic_handle_irq_desc(unsigned int irq, struct irq_desc *desc)
{
#ifdef CONFIG_GENERIC_HARDIRQS_NO__DO_IRQ
	desc->handle_irq(irq, desc);
#else
	if (likely(desc->handle_irq))
		desc->handle_irq(irq, desc);
	else
		__do_IRQ(irq);
#endif
}

static inline void generic_handle_irq(unsigned int irq)
{
	generic_handle_irq_desc(irq, irq_to_desc(irq));
}


extern void note_interrupt(unsigned int irq, struct irq_desc *desc,
			   irqreturn_t action_ret);


void check_irq_resend(struct irq_desc *desc, unsigned int irq);


extern int noirqdebug_setup(char *str);


extern int can_request_irq(unsigned int irq, unsigned long irqflags);


extern struct irq_chip no_irq_chip;
extern struct irq_chip dummy_irq_chip;

extern void
set_irq_chip_and_handler(unsigned int irq, struct irq_chip *chip,
			 irq_flow_handler_t handle);
extern void
set_irq_chip_and_handler_name(unsigned int irq, struct irq_chip *chip,
			      irq_flow_handler_t handle, const char *name);

extern void
__set_irq_handler(unsigned int irq, irq_flow_handler_t handle, int is_chained,
		  const char *name);


static inline void __set_irq_handler_unlocked(int irq,
					      irq_flow_handler_t handler)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	desc->handle_irq = handler;
}


static inline void
set_irq_handler(unsigned int irq, irq_flow_handler_t handle)
{
	__set_irq_handler(irq, handle, 0, NULL);
}


static inline void
set_irq_chained_handler(unsigned int irq,
			irq_flow_handler_t handle)
{
	__set_irq_handler(irq, handle, 1, NULL);
}

extern void set_irq_nested_thread(unsigned int irq, int nest);

extern void set_irq_noprobe(unsigned int irq);
extern void set_irq_probe(unsigned int irq);


extern unsigned int create_irq_nr(unsigned int irq_want, int node);
extern int create_irq(void);
extern void destroy_irq(unsigned int irq);


static inline int irq_has_action(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	return desc->action != NULL;
}


extern void dynamic_irq_init(unsigned int irq);
extern void dynamic_irq_cleanup(unsigned int irq);


extern int set_irq_chip(unsigned int irq, struct irq_chip *chip);
extern int set_irq_data(unsigned int irq, void *data);
extern int set_irq_chip_data(unsigned int irq, void *data);
extern int set_irq_type(unsigned int irq, unsigned int type);
extern int set_irq_msi(unsigned int irq, struct msi_desc *entry);

#define get_irq_chip(irq)	(irq_to_desc(irq)->chip)
#define get_irq_chip_data(irq)	(irq_to_desc(irq)->chip_data)
#define get_irq_data(irq)	(irq_to_desc(irq)->handler_data)
#define get_irq_msi(irq)	(irq_to_desc(irq)->msi_desc)

#define get_irq_desc_chip(desc)		((desc)->chip)
#define get_irq_desc_chip_data(desc)	((desc)->chip_data)
#define get_irq_desc_data(desc)		((desc)->handler_data)
#define get_irq_desc_msi(desc)		((desc)->msi_desc)

#endif 

#endif 

#ifdef CONFIG_SMP

static inline bool alloc_desc_masks(struct irq_desc *desc, int node,
							bool boot)
{
	gfp_t gfp = GFP_ATOMIC;

	if (boot)
		gfp = GFP_NOWAIT;

#ifdef CONFIG_CPUMASK_OFFSTACK
	if (!alloc_cpumask_var_node(&desc->affinity, gfp, node))
		return false;

#ifdef CONFIG_GENERIC_PENDING_IRQ
	if (!alloc_cpumask_var_node(&desc->pending_mask, gfp, node)) {
		free_cpumask_var(desc->affinity);
		return false;
	}
#endif
#endif
	return true;
}

static inline void init_desc_masks(struct irq_desc *desc)
{
	cpumask_setall(desc->affinity);
#ifdef CONFIG_GENERIC_PENDING_IRQ
	cpumask_clear(desc->pending_mask);
#endif
}



static inline void init_copy_desc_masks(struct irq_desc *old_desc,
					struct irq_desc *new_desc)
{
#ifdef CONFIG_CPUMASK_OFFSTACK
	cpumask_copy(new_desc->affinity, old_desc->affinity);

#ifdef CONFIG_GENERIC_PENDING_IRQ
	cpumask_copy(new_desc->pending_mask, old_desc->pending_mask);
#endif
#endif
}

static inline void free_desc_masks(struct irq_desc *old_desc,
				   struct irq_desc *new_desc)
{
	free_cpumask_var(old_desc->affinity);

#ifdef CONFIG_GENERIC_PENDING_IRQ
	free_cpumask_var(old_desc->pending_mask);
#endif
}

#else 

static inline bool alloc_desc_masks(struct irq_desc *desc, int node,
								bool boot)
{
	return true;
}

static inline void init_desc_masks(struct irq_desc *desc)
{
}

static inline void init_copy_desc_masks(struct irq_desc *old_desc,
					struct irq_desc *new_desc)
{
}

static inline void free_desc_masks(struct irq_desc *old_desc,
				   struct irq_desc *new_desc)
{
}
#endif	

#endif 
