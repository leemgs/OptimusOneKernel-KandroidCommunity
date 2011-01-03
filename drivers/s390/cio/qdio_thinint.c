
#include <linux/io.h>
#include <asm/atomic.h>
#include <asm/debug.h>
#include <asm/qdio.h>
#include <asm/airq.h>
#include <asm/isc.h>

#include "cio.h"
#include "ioasm.h"
#include "qdio.h"
#include "qdio_debug.h"
#include "qdio_perf.h"


#define TIQDIO_NR_NONSHARED_IND		63
#define TIQDIO_NR_INDICATORS		(TIQDIO_NR_NONSHARED_IND + 1)
#define TIQDIO_SHARED_IND		63


static LIST_HEAD(tiq_list);
DEFINE_MUTEX(tiq_list_lock);


static unsigned char *tiqdio_alsi;


struct indicator_t {
	u32 ind;	
	atomic_t count; 
};
static struct indicator_t *q_indicators;

static int css_qdio_omit_svs;

static inline unsigned long do_clear_global_summary(void)
{
	register unsigned long __fn asm("1") = 3;
	register unsigned long __tmp asm("2");
	register unsigned long __time asm("3");

	asm volatile(
		"	.insn	rre,0xb2650000,2,0"
		: "+d" (__fn), "=d" (__tmp), "=d" (__time));
	return __time;
}


static u32 *get_indicator(void)
{
	int i;

	for (i = 0; i < TIQDIO_NR_NONSHARED_IND; i++)
		if (!atomic_read(&q_indicators[i].count)) {
			atomic_set(&q_indicators[i].count, 1);
			return &q_indicators[i].ind;
		}

	
	atomic_inc(&q_indicators[TIQDIO_SHARED_IND].count);
	return &q_indicators[TIQDIO_SHARED_IND].ind;
}

static void put_indicator(u32 *addr)
{
	int i;

	if (!addr)
		return;
	i = ((unsigned long)addr - (unsigned long)q_indicators) /
		sizeof(struct indicator_t);
	atomic_dec(&q_indicators[i].count);
}

void tiqdio_add_input_queues(struct qdio_irq *irq_ptr)
{
	struct qdio_q *q;
	int i;

	
	if (!css_qdio_omit_svs && irq_ptr->siga_flag.sync)
		css_qdio_omit_svs = 1;

	mutex_lock(&tiq_list_lock);
	for_each_input_queue(irq_ptr, q, i)
		list_add_rcu(&q->entry, &tiq_list);
	mutex_unlock(&tiq_list_lock);
	xchg(irq_ptr->dsci, 1);
}

void tiqdio_remove_input_queues(struct qdio_irq *irq_ptr)
{
	struct qdio_q *q;
	int i;

	for (i = 0; i < irq_ptr->nr_input_qs; i++) {
		q = irq_ptr->input_qs[i];
		
		if (!q || !q->entry.prev || !q->entry.next)
			continue;

		mutex_lock(&tiq_list_lock);
		list_del_rcu(&q->entry);
		mutex_unlock(&tiq_list_lock);
		synchronize_rcu();
	}
}

static inline int shared_ind(struct qdio_irq *irq_ptr)
{
	return irq_ptr->dsci == &q_indicators[TIQDIO_SHARED_IND].ind;
}


static void tiqdio_thinint_handler(void *ind, void *drv_data)
{
	struct qdio_q *q;

	qdio_perf_stat_inc(&perf_stats.thin_int);

	
	if (!css_qdio_omit_svs)
		do_clear_global_summary();

	
	xchg((u8 *)ind, 0);

	
	rcu_read_lock();

	
	list_for_each_entry_rcu(q, &tiq_list, entry)
		
		if (*q->irq_ptr->dsci) {

			
			if (!shared_ind(q->irq_ptr))
				xchg(q->irq_ptr->dsci, 0);
			
			tasklet_schedule(&q->tasklet);
		}

	rcu_read_unlock();

	
	if (atomic_read(&q_indicators[TIQDIO_SHARED_IND].count)) {
		xchg(&q_indicators[TIQDIO_SHARED_IND].ind, 0);

		
		if (*tiqdio_alsi)
			xchg(&q_indicators[TIQDIO_SHARED_IND].ind, 1);
	}
}

static int set_subchannel_ind(struct qdio_irq *irq_ptr, int reset)
{
	struct scssc_area *scssc_area;
	int rc;

	scssc_area = (struct scssc_area *)irq_ptr->chsc_page;
	memset(scssc_area, 0, PAGE_SIZE);

	if (reset) {
		scssc_area->summary_indicator_addr = 0;
		scssc_area->subchannel_indicator_addr = 0;
	} else {
		scssc_area->summary_indicator_addr = virt_to_phys(tiqdio_alsi);
		scssc_area->subchannel_indicator_addr =
			virt_to_phys(irq_ptr->dsci);
	}

	scssc_area->request = (struct chsc_header) {
		.length = 0x0fe0,
		.code	= 0x0021,
	};
	scssc_area->operation_code = 0;
	scssc_area->ks = PAGE_DEFAULT_KEY;
	scssc_area->kc = PAGE_DEFAULT_KEY;
	scssc_area->isc = QDIO_AIRQ_ISC;
	scssc_area->schid = irq_ptr->schid;

	
	if (css_general_characteristics.aif_tdd)
		scssc_area->word_with_d_bit = 0x10000000;

	rc = chsc(scssc_area);
	if (rc)
		return -EIO;

	rc = chsc_error_from_response(scssc_area->response.code);
	if (rc) {
		DBF_ERROR("%4x SSI r:%4x", irq_ptr->schid.sch_no,
			  scssc_area->response.code);
		DBF_ERROR_HEX(&scssc_area->response, sizeof(void *));
		return rc;
	}

	DBF_EVENT("setscind");
	DBF_HEX(&scssc_area->summary_indicator_addr, sizeof(unsigned long));
	DBF_HEX(&scssc_area->subchannel_indicator_addr,	sizeof(unsigned long));
	return 0;
}


int __init tiqdio_allocate_memory(void)
{
	q_indicators = kzalloc(sizeof(struct indicator_t) * TIQDIO_NR_INDICATORS,
			     GFP_KERNEL);
	if (!q_indicators)
		return -ENOMEM;
	return 0;
}

void tiqdio_free_memory(void)
{
	kfree(q_indicators);
}

int __init tiqdio_register_thinints(void)
{
	isc_register(QDIO_AIRQ_ISC);
	tiqdio_alsi = s390_register_adapter_interrupt(&tiqdio_thinint_handler,
						      NULL, QDIO_AIRQ_ISC);
	if (IS_ERR(tiqdio_alsi)) {
		DBF_EVENT("RTI:%lx", PTR_ERR(tiqdio_alsi));
		tiqdio_alsi = NULL;
		isc_unregister(QDIO_AIRQ_ISC);
		return -ENOMEM;
	}
	return 0;
}

int qdio_establish_thinint(struct qdio_irq *irq_ptr)
{
	if (!is_thinint_irq(irq_ptr))
		return 0;

	
	if (css_general_characteristics.aif_tdd)
		css_qdio_omit_svs = 1;
	return set_subchannel_ind(irq_ptr, 0);
}

void qdio_setup_thinint(struct qdio_irq *irq_ptr)
{
	if (!is_thinint_irq(irq_ptr))
		return;
	irq_ptr->dsci = get_indicator();
	DBF_HEX(&irq_ptr->dsci, sizeof(void *));
}

void qdio_shutdown_thinint(struct qdio_irq *irq_ptr)
{
	if (!is_thinint_irq(irq_ptr))
		return;

	
	put_indicator(irq_ptr->dsci);
	set_subchannel_ind(irq_ptr, 1);
}

void __exit tiqdio_unregister_thinints(void)
{
	WARN_ON(!list_empty(&tiq_list));

	if (tiqdio_alsi) {
		s390_unregister_adapter_interrupt(tiqdio_alsi, QDIO_AIRQ_ISC);
		isc_unregister(QDIO_AIRQ_ISC);
	}
}
