#ifndef __ASM_GENERIC_IRQFLAGS_H
#define __ASM_GENERIC_IRQFLAGS_H


#ifndef RAW_IRQ_DISABLED
#define RAW_IRQ_DISABLED 0
#define RAW_IRQ_ENABLED 1
#endif


#ifndef __raw_local_save_flags
unsigned long __raw_local_save_flags(void);
#endif


#ifndef raw_local_irq_restore
void raw_local_irq_restore(unsigned long flags);
#endif


#ifndef __raw_local_irq_save
static inline unsigned long __raw_local_irq_save(void)
{
	unsigned long flags;
	flags = __raw_local_save_flags();
	raw_local_irq_restore(RAW_IRQ_DISABLED);
	return flags;
}
#endif


#ifndef raw_irqs_disabled_flags
static inline int raw_irqs_disabled_flags(unsigned long flags)
{
	return flags == RAW_IRQ_DISABLED;
}
#endif


#ifndef raw_local_irq_enable
static inline void raw_local_irq_enable(void)
{
	raw_local_irq_restore(RAW_IRQ_ENABLED);
}
#endif


#ifndef raw_local_irq_disable
static inline void raw_local_irq_disable(void)
{
	raw_local_irq_restore(RAW_IRQ_DISABLED);
}
#endif


#ifndef raw_irqs_disabled
static inline int raw_irqs_disabled(void)
{
	return raw_irqs_disabled_flags(__raw_local_save_flags());
}
#endif

#define raw_local_save_flags(flags) \
	do { (flags) = __raw_local_save_flags(); } while (0)

#define raw_local_irq_save(flags) \
	do { (flags) = __raw_local_irq_save(); } while (0)

#endif 
