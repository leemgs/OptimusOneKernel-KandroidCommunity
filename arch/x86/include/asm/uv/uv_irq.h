

#ifndef _ASM_X86_UV_UV_IRQ_H
#define _ASM_X86_UV_UV_IRQ_H


struct uv_IO_APIC_route_entry {
	__u64	vector		:  8,
		delivery_mode	:  3,
		dest_mode	:  1,
		delivery_status	:  1,
		polarity	:  1,
		__reserved_1	:  1,
		trigger		:  1,
		mask		:  1,
		__reserved_2	: 15,
		dest		: 32;
};

extern struct irq_chip uv_irq_chip;

extern int arch_enable_uv_irq(char *, unsigned int, int, int, unsigned long);
extern void arch_disable_uv_irq(int, unsigned long);

extern int uv_setup_irq(char *, int, int, unsigned long);
extern void uv_teardown_irq(unsigned int, int, unsigned long);

#endif 
