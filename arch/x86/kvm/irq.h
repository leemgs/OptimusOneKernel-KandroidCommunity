

#ifndef __IRQ_H
#define __IRQ_H

#include <linux/mm_types.h>
#include <linux/hrtimer.h>
#include <linux/kvm_host.h>
#include <linux/spinlock.h>

#include "iodev.h"
#include "ioapic.h"
#include "lapic.h"

#define PIC_NUM_PINS 16
#define SELECT_PIC(irq) \
	((irq) < 8 ? KVM_IRQCHIP_PIC_MASTER : KVM_IRQCHIP_PIC_SLAVE)

struct kvm;
struct kvm_vcpu;

typedef void irq_request_func(void *opaque, int level);

struct kvm_kpic_state {
	u8 last_irr;	
	u8 irr;		
	u8 imr;		
	u8 isr;		
	u8 isr_ack;	
	u8 priority_add;	
	u8 irq_base;
	u8 read_reg_select;
	u8 poll;
	u8 special_mask;
	u8 init_state;
	u8 auto_eoi;
	u8 rotate_on_auto_eoi;
	u8 special_fully_nested_mode;
	u8 init4;		
	u8 elcr;		
	u8 elcr_mask;
	struct kvm_pic *pics_state;
};

struct kvm_pic {
	spinlock_t lock;
	unsigned pending_acks;
	struct kvm *kvm;
	struct kvm_kpic_state pics[2]; 
	irq_request_func *irq_request;
	void *irq_request_opaque;
	int output;		
	struct kvm_io_device dev;
	void (*ack_notifier)(void *opaque, int irq);
};

struct kvm_pic *kvm_create_pic(struct kvm *kvm);
int kvm_pic_read_irq(struct kvm *kvm);
void kvm_pic_update_irq(struct kvm_pic *s);
void kvm_pic_clear_isr_ack(struct kvm *kvm);

static inline struct kvm_pic *pic_irqchip(struct kvm *kvm)
{
	return kvm->arch.vpic;
}

static inline int irqchip_in_kernel(struct kvm *kvm)
{
	return pic_irqchip(kvm) != NULL;
}

void kvm_pic_reset(struct kvm_kpic_state *s);

void kvm_inject_pending_timer_irqs(struct kvm_vcpu *vcpu);
void kvm_inject_apic_timer_irqs(struct kvm_vcpu *vcpu);
void kvm_apic_nmi_wd_deliver(struct kvm_vcpu *vcpu);
void __kvm_migrate_apic_timer(struct kvm_vcpu *vcpu);
void __kvm_migrate_pit_timer(struct kvm_vcpu *vcpu);
void __kvm_migrate_timers(struct kvm_vcpu *vcpu);

int pit_has_pending_timer(struct kvm_vcpu *vcpu);
int apic_has_pending_timer(struct kvm_vcpu *vcpu);

#endif
