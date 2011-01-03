

#include <asm/processor.h>
#include <asm/vmware.h>
#include <asm/hypervisor.h>

static inline void __cpuinit
detect_hypervisor_vendor(struct cpuinfo_x86 *c)
{
	if (vmware_platform())
		c->x86_hyper_vendor = X86_HYPER_VENDOR_VMWARE;
	else
		c->x86_hyper_vendor = X86_HYPER_VENDOR_NONE;
}

static inline void __cpuinit
hypervisor_set_feature_bits(struct cpuinfo_x86 *c)
{
	if (boot_cpu_data.x86_hyper_vendor == X86_HYPER_VENDOR_VMWARE) {
		vmware_set_feature_bits(c);
		return;
	}
}

void __cpuinit init_hypervisor(struct cpuinfo_x86 *c)
{
	detect_hypervisor_vendor(c);
	hypervisor_set_feature_bits(c);
}

void __init init_hypervisor_platform(void)
{
	init_hypervisor(&boot_cpu_data);
	if (boot_cpu_data.x86_hyper_vendor == X86_HYPER_VENDOR_VMWARE)
		vmware_platform_setup();
}
