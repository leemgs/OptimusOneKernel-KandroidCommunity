

#include <linux/oprofile.h>
#include <linux/slab.h>
#include <asm/ptrace.h>
#include <asm/msr.h>
#include <asm/apic.h>
#include <asm/nmi.h>

#include "op_x86_model.h"
#include "op_counter.h"

static int num_counters = 2;
static int counter_width = 32;

#define MSR_PPRO_EVENTSEL_RESERVED	((0xFFFFFFFFULL<<32)|(1ULL<<21))

static u64 *reset_value;

static void ppro_fill_in_addresses(struct op_msrs * const msrs)
{
	int i;

	for (i = 0; i < num_counters; i++) {
		if (reserve_perfctr_nmi(MSR_P6_PERFCTR0 + i))
			msrs->counters[i].addr = MSR_P6_PERFCTR0 + i;
		else
			msrs->counters[i].addr = 0;
	}

	for (i = 0; i < num_counters; i++) {
		if (reserve_evntsel_nmi(MSR_P6_EVNTSEL0 + i))
			msrs->controls[i].addr = MSR_P6_EVNTSEL0 + i;
		else
			msrs->controls[i].addr = 0;
	}
}


static void ppro_setup_ctrs(struct op_x86_model_spec const *model,
			    struct op_msrs const * const msrs)
{
	u64 val;
	int i;

	if (!reset_value) {
		reset_value = kmalloc(sizeof(reset_value[0]) * num_counters,
					GFP_ATOMIC);
		if (!reset_value)
			return;
	}

	if (cpu_has_arch_perfmon) {
		union cpuid10_eax eax;
		eax.full = cpuid_eax(0xa);

		
		if (!(eax.split.version_id == 0 &&
			current_cpu_data.x86 == 6 &&
				current_cpu_data.x86_model == 15)) {

			if (counter_width < eax.split.bit_width)
				counter_width = eax.split.bit_width;
		}
	}

	
	for (i = 0; i < num_counters; ++i) {
		if (unlikely(!msrs->controls[i].addr))
			continue;
		rdmsrl(msrs->controls[i].addr, val);
		val &= model->reserved;
		wrmsrl(msrs->controls[i].addr, val);
	}

	
	for (i = 0; i < num_counters; ++i) {
		if (unlikely(!msrs->counters[i].addr))
			continue;
		wrmsrl(msrs->counters[i].addr, -1LL);
	}

	
	for (i = 0; i < num_counters; ++i) {
		if (counter_config[i].enabled && msrs->counters[i].addr) {
			reset_value[i] = counter_config[i].count;
			wrmsrl(msrs->counters[i].addr, -reset_value[i]);
			rdmsrl(msrs->controls[i].addr, val);
			val &= model->reserved;
			val |= op_x86_get_ctrl(model, &counter_config[i]);
			wrmsrl(msrs->controls[i].addr, val);
		} else {
			reset_value[i] = 0;
		}
	}
}


static int ppro_check_ctrs(struct pt_regs * const regs,
			   struct op_msrs const * const msrs)
{
	u64 val;
	int i;

	
	if (unlikely(!reset_value))
		goto out;

	for (i = 0; i < num_counters; ++i) {
		if (!reset_value[i])
			continue;
		rdmsrl(msrs->counters[i].addr, val);
		if (val & (1ULL << (counter_width - 1)))
			continue;
		oprofile_add_sample(regs, i);
		wrmsrl(msrs->counters[i].addr, -reset_value[i]);
	}

out:
	
	apic_write(APIC_LVTPC, apic_read(APIC_LVTPC) & ~APIC_LVT_MASKED);

	
	return 1;
}


static void ppro_start(struct op_msrs const * const msrs)
{
	u64 val;
	int i;

	if (!reset_value)
		return;
	for (i = 0; i < num_counters; ++i) {
		if (reset_value[i]) {
			rdmsrl(msrs->controls[i].addr, val);
			val |= ARCH_PERFMON_EVENTSEL0_ENABLE;
			wrmsrl(msrs->controls[i].addr, val);
		}
	}
}


static void ppro_stop(struct op_msrs const * const msrs)
{
	u64 val;
	int i;

	if (!reset_value)
		return;
	for (i = 0; i < num_counters; ++i) {
		if (!reset_value[i])
			continue;
		rdmsrl(msrs->controls[i].addr, val);
		val &= ~ARCH_PERFMON_EVENTSEL0_ENABLE;
		wrmsrl(msrs->controls[i].addr, val);
	}
}

static void ppro_shutdown(struct op_msrs const * const msrs)
{
	int i;

	for (i = 0; i < num_counters; ++i) {
		if (msrs->counters[i].addr)
			release_perfctr_nmi(MSR_P6_PERFCTR0 + i);
	}
	for (i = 0; i < num_counters; ++i) {
		if (msrs->controls[i].addr)
			release_evntsel_nmi(MSR_P6_EVNTSEL0 + i);
	}
	if (reset_value) {
		kfree(reset_value);
		reset_value = NULL;
	}
}


struct op_x86_model_spec op_ppro_spec = {
	.num_counters		= 2,
	.num_controls		= 2,
	.reserved		= MSR_PPRO_EVENTSEL_RESERVED,
	.fill_in_addresses	= &ppro_fill_in_addresses,
	.setup_ctrs		= &ppro_setup_ctrs,
	.check_ctrs		= &ppro_check_ctrs,
	.start			= &ppro_start,
	.stop			= &ppro_stop,
	.shutdown		= &ppro_shutdown
};



static void arch_perfmon_setup_counters(void)
{
	union cpuid10_eax eax;

	eax.full = cpuid_eax(0xa);

	
	if (eax.split.version_id == 0 && current_cpu_data.x86 == 6 &&
		current_cpu_data.x86_model == 15) {
		eax.split.version_id = 2;
		eax.split.num_events = 2;
		eax.split.bit_width = 40;
	}

	num_counters = eax.split.num_events;

	op_arch_perfmon_spec.num_counters = num_counters;
	op_arch_perfmon_spec.num_controls = num_counters;
}

static int arch_perfmon_init(struct oprofile_operations *ignore)
{
	arch_perfmon_setup_counters();
	return 0;
}

struct op_x86_model_spec op_arch_perfmon_spec = {
	.reserved		= MSR_PPRO_EVENTSEL_RESERVED,
	.init			= &arch_perfmon_init,
	
	.fill_in_addresses	= &ppro_fill_in_addresses,
	
	.setup_ctrs		= &ppro_setup_ctrs,
	.check_ctrs		= &ppro_check_ctrs,
	.start			= &ppro_start,
	.stop			= &ppro_stop,
	.shutdown		= &ppro_shutdown
};
