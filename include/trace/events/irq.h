#undef TRACE_SYSTEM
#define TRACE_SYSTEM irq

#if !defined(_TRACE_IRQ_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_IRQ_H

#include <linux/tracepoint.h>
#include <linux/interrupt.h>

#define softirq_name(sirq) { sirq##_SOFTIRQ, #sirq }
#define show_softirq_name(val)				\
	__print_symbolic(val,				\
			 softirq_name(HI),		\
			 softirq_name(TIMER),		\
			 softirq_name(NET_TX),		\
			 softirq_name(NET_RX),		\
			 softirq_name(BLOCK),		\
			 softirq_name(BLOCK_IOPOLL),	\
			 softirq_name(TASKLET),		\
			 softirq_name(SCHED),		\
			 softirq_name(HRTIMER),		\
			 softirq_name(RCU))


TRACE_EVENT(irq_handler_entry,

	TP_PROTO(int irq, struct irqaction *action),

	TP_ARGS(irq, action),

	TP_STRUCT__entry(
		__field(	int,	irq		)
		__string(	name,	action->name	)
	),

	TP_fast_assign(
		__entry->irq = irq;
		__assign_str(name, action->name);
	),

	TP_printk("irq=%d handler=%s", __entry->irq, __get_str(name))
);


TRACE_EVENT(irq_handler_exit,

	TP_PROTO(int irq, struct irqaction *action, int ret),

	TP_ARGS(irq, action, ret),

	TP_STRUCT__entry(
		__field(	int,	irq	)
		__field(	int,	ret	)
	),

	TP_fast_assign(
		__entry->irq	= irq;
		__entry->ret	= ret;
	),

	TP_printk("irq=%d return=%s",
		  __entry->irq, __entry->ret ? "handled" : "unhandled")
);


TRACE_EVENT(softirq_entry,

	TP_PROTO(struct softirq_action *h, struct softirq_action *vec),

	TP_ARGS(h, vec),

	TP_STRUCT__entry(
		__field(	int,	vec			)
	),

	TP_fast_assign(
		__entry->vec = (int)(h - vec);
	),

	TP_printk("softirq=%d action=%s", __entry->vec,
		  show_softirq_name(__entry->vec))
);


TRACE_EVENT(softirq_exit,

	TP_PROTO(struct softirq_action *h, struct softirq_action *vec),

	TP_ARGS(h, vec),

	TP_STRUCT__entry(
		__field(	int,	vec			)
	),

	TP_fast_assign(
		__entry->vec = (int)(h - vec);
	),

	TP_printk("softirq=%d action=%s", __entry->vec,
		  show_softirq_name(__entry->vec))
);

#endif 


#include <trace/define_trace.h>
