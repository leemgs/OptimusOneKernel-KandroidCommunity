

#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/threads.h>
#include <linux/cpumask.h>
#include <linux/seqlock.h>


#define MAX_RCU_LVLS 3
#define RCU_FANOUT	      (CONFIG_RCU_FANOUT)
#define RCU_FANOUT_SQ	      (RCU_FANOUT * RCU_FANOUT)
#define RCU_FANOUT_CUBE	      (RCU_FANOUT_SQ * RCU_FANOUT)

#if NR_CPUS <= RCU_FANOUT
#  define NUM_RCU_LVLS	      1
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      (NR_CPUS)
#  define NUM_RCU_LVL_2	      0
#  define NUM_RCU_LVL_3	      0
#elif NR_CPUS <= RCU_FANOUT_SQ
#  define NUM_RCU_LVLS	      2
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT)
#  define NUM_RCU_LVL_2	      (NR_CPUS)
#  define NUM_RCU_LVL_3	      0
#elif NR_CPUS <= RCU_FANOUT_CUBE
#  define NUM_RCU_LVLS	      3
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_SQ)
#  define NUM_RCU_LVL_2	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT)
#  define NUM_RCU_LVL_3	      NR_CPUS
#else
# error "CONFIG_RCU_FANOUT insufficient for NR_CPUS"
#endif 

#define RCU_SUM (NUM_RCU_LVL_0 + NUM_RCU_LVL_1 + NUM_RCU_LVL_2 + NUM_RCU_LVL_3)
#define NUM_RCU_NODES (RCU_SUM - NR_CPUS)


struct rcu_dynticks {
	int dynticks_nesting;	
	int dynticks;		
	int dynticks_nmi;	
				
				
};


struct rcu_node {
	spinlock_t lock;	
				
	long	gpnum;		
				
				
	long	completed;	
				
				
	unsigned long qsmask;	
				
				
				
				
				
	unsigned long qsmaskinit;
				
	unsigned long grpmask;	
				
	int	grplo;		
	int	grphi;		
	u8	grpnum;		
	u8	level;		
	struct rcu_node *parent;
	struct list_head blocked_tasks[2];
				
				
				
				
} ____cacheline_internodealigned_in_smp;


#define rcu_for_each_node_breadth_first(rsp, rnp) \
	for ((rnp) = &(rsp)->node[0]; \
	     (rnp) < &(rsp)->node[NUM_RCU_NODES]; (rnp)++)

#define rcu_for_each_leaf_node(rsp, rnp) \
	for ((rnp) = (rsp)->level[NUM_RCU_LVLS - 1]; \
	     (rnp) < &(rsp)->node[NUM_RCU_NODES]; (rnp)++)


#define RCU_DONE_TAIL		0	
#define RCU_WAIT_TAIL		1	
#define RCU_NEXT_READY_TAIL	2	
#define RCU_NEXT_TAIL		3
#define RCU_NEXT_SIZE		4


struct rcu_data {
	
	long		completed;	
					
	long		gpnum;		
					
	long		passed_quiesc_completed;
					
	bool		passed_quiesc;	
	bool		qs_pending;	
	bool		beenonline;	
	bool		preemptable;	
	struct rcu_node *mynode;	
	unsigned long grpmask;		

	
	
	struct rcu_head *nxtlist;
	struct rcu_head **nxttail[RCU_NEXT_SIZE];
	long		qlen;		
	long		qlen_last_fqs_check;
					
	unsigned long	n_force_qs_snap;
					
	long		blimit;		

#ifdef CONFIG_NO_HZ
	
	struct rcu_dynticks *dynticks;	
	int dynticks_snap;		
	int dynticks_nmi_snap;		
#endif 

	
#ifdef CONFIG_NO_HZ
	unsigned long dynticks_fqs;	
#endif 
	unsigned long offline_fqs;	
	unsigned long resched_ipi;	

	
	long n_rcu_pending;		
	long n_rp_qs_pending;
	long n_rp_cb_ready;
	long n_rp_cpu_needs_gp;
	long n_rp_gp_completed;
	long n_rp_gp_started;
	long n_rp_need_fqs;
	long n_rp_need_nothing;

	int cpu;
};


#define RCU_GP_IDLE		0	
#define RCU_GP_INIT		1	
#define RCU_SAVE_DYNTICK	2	
#define RCU_SAVE_COMPLETED	3	
#define RCU_FORCE_QS		4	
#ifdef CONFIG_NO_HZ
#define RCU_SIGNAL_INIT		RCU_SAVE_DYNTICK
#else 
#define RCU_SIGNAL_INIT		RCU_SAVE_COMPLETED
#endif 

#define RCU_JIFFIES_TILL_FORCE_QS	 3	
#ifdef CONFIG_RCU_CPU_STALL_DETECTOR
#define RCU_SECONDS_TILL_STALL_CHECK   (10 * HZ)  
#define RCU_SECONDS_TILL_STALL_RECHECK (30 * HZ)  
#define RCU_STALL_RAT_DELAY		2	  
						  
						  
						  

#endif 


struct rcu_state {
	struct rcu_node node[NUM_RCU_NODES];	
	struct rcu_node *level[NUM_RCU_LVLS];	
	u32 levelcnt[MAX_RCU_LVLS + 1];		
	u8 levelspread[NUM_RCU_LVLS];		
	struct rcu_data *rda[NR_CPUS];		

	

	u8	signaled ____cacheline_internodealigned_in_smp;
						
	long	gpnum;				
	long	completed;			

	

	spinlock_t onofflock;			
						
						
						
	struct rcu_head *orphan_cbs_list;	
						
						
						
	struct rcu_head **orphan_cbs_tail;	
	long orphan_qlen;			
	spinlock_t fqslock;			
						
	unsigned long jiffies_force_qs;		
						
	unsigned long n_force_qs;		
						
	unsigned long n_force_qs_lh;		
						
	unsigned long n_force_qs_ngp;		
						
#ifdef CONFIG_RCU_CPU_STALL_DETECTOR
	unsigned long gp_start;			
						
	unsigned long jiffies_stall;		
						
#endif 
	long dynticks_completed;		
						
};

#ifdef RCU_TREE_NONCORE


extern struct rcu_state rcu_sched_state;
DECLARE_PER_CPU(struct rcu_data, rcu_sched_data);

extern struct rcu_state rcu_bh_state;
DECLARE_PER_CPU(struct rcu_data, rcu_bh_data);

#ifdef CONFIG_TREE_PREEMPT_RCU
extern struct rcu_state rcu_preempt_state;
DECLARE_PER_CPU(struct rcu_data, rcu_preempt_data);
#endif 

#else 


static void rcu_bootup_announce(void);
long rcu_batches_completed(void);
static void rcu_preempt_note_context_switch(int cpu);
static int rcu_preempted_readers(struct rcu_node *rnp);
#ifdef CONFIG_RCU_CPU_STALL_DETECTOR
static void rcu_print_task_stall(struct rcu_node *rnp);
#endif 
static void rcu_preempt_check_blocked_tasks(struct rcu_node *rnp);
#ifdef CONFIG_HOTPLUG_CPU
static int rcu_preempt_offline_tasks(struct rcu_state *rsp,
				     struct rcu_node *rnp,
				     struct rcu_data *rdp);
static void rcu_preempt_offline_cpu(int cpu);
#endif 
static void rcu_preempt_check_callbacks(int cpu);
static void rcu_preempt_process_callbacks(void);
void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu));
static int rcu_preempt_pending(int cpu);
static int rcu_preempt_needs_cpu(int cpu);
static void __cpuinit rcu_preempt_init_percpu_data(int cpu);
static void rcu_preempt_send_cbs_to_orphanage(void);
static void __init __rcu_init_preempt(void);

#endif 
