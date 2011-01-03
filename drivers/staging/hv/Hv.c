
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include "osd.h"
#include "logging.h"
#include "VmbusPrivate.h"


struct hv_context gHvContext = {
	.SynICInitialized	= false,
	.HypercallPage		= NULL,
	.SignalEventParam	= NULL,
	.SignalEventBuffer	= NULL,
};


static int HvQueryHypervisorPresence(void)
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int op;

	eax = 0;
	ebx = 0;
	ecx = 0;
	edx = 0;
	op = HvCpuIdFunctionVersionAndFeatures;
	cpuid(op, &eax, &ebx, &ecx, &edx);

	return ecx & HV_PRESENT_BIT;
}


static int HvQueryHypervisorInfo(void)
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int maxLeaf;
	unsigned int op;

	
	eax = 0;
	ebx = 0;
	ecx = 0;
	edx = 0;
	op = HvCpuIdFunctionHvVendorAndMaxFunction;
	cpuid(op, &eax, &ebx, &ecx, &edx);

	DPRINT_INFO(VMBUS, "Vendor ID: %c%c%c%c%c%c%c%c%c%c%c%c",
		    (ebx & 0xFF),
		    ((ebx >> 8) & 0xFF),
		    ((ebx >> 16) & 0xFF),
		    ((ebx >> 24) & 0xFF),
		    (ecx & 0xFF),
		    ((ecx >> 8) & 0xFF),
		    ((ecx >> 16) & 0xFF),
		    ((ecx >> 24) & 0xFF),
		    (edx & 0xFF),
		    ((edx >> 8) & 0xFF),
		    ((edx >> 16) & 0xFF),
		    ((edx >> 24) & 0xFF));

	maxLeaf = eax;
	eax = 0;
	ebx = 0;
	ecx = 0;
	edx = 0;
	op = HvCpuIdFunctionHvInterface;
	cpuid(op, &eax, &ebx, &ecx, &edx);

	DPRINT_INFO(VMBUS, "Interface ID: %c%c%c%c",
		    (eax & 0xFF),
		    ((eax >> 8) & 0xFF),
		    ((eax >> 16) & 0xFF),
		    ((eax >> 24) & 0xFF));

	if (maxLeaf >= HvCpuIdFunctionMsHvVersion) {
		eax = 0;
		ebx = 0;
		ecx = 0;
		edx = 0;
		op = HvCpuIdFunctionMsHvVersion;
		cpuid(op, &eax, &ebx, &ecx, &edx);
		DPRINT_INFO(VMBUS, "OS Build:%d-%d.%d-%d-%d.%d",\
			    eax,
			    ebx >> 16,
			    ebx & 0xFFFF,
			    ecx,
			    edx >> 24,
			    edx & 0xFFFFFF);
	}
	return maxLeaf;
}


static u64 HvDoHypercall(u64 Control, void *Input, void *Output)
{
#ifdef CONFIG_X86_64
	u64 hvStatus = 0;
	u64 inputAddress = (Input) ? virt_to_phys(Input) : 0;
	u64 outputAddress = (Output) ? virt_to_phys(Output) : 0;
	volatile void *hypercallPage = gHvContext.HypercallPage;

	DPRINT_DBG(VMBUS, "Hypercall <control %llx input phys %llx virt %p "
		   "output phys %llx virt %p hypercall %p>",
		   Control, inputAddress, Input,
		   outputAddress, Output, hypercallPage);

	__asm__ __volatile__("mov %0, %%r8" : : "r" (outputAddress) : "r8");
	__asm__ __volatile__("call *%3" : "=a" (hvStatus) :
			     "c" (Control), "d" (inputAddress),
			     "m" (hypercallPage));

	DPRINT_DBG(VMBUS, "Hypercall <return %llx>",  hvStatus);

	return hvStatus;

#else

	u32 controlHi = Control >> 32;
	u32 controlLo = Control & 0xFFFFFFFF;
	u32 hvStatusHi = 1;
	u32 hvStatusLo = 1;
	u64 inputAddress = (Input) ? virt_to_phys(Input) : 0;
	u32 inputAddressHi = inputAddress >> 32;
	u32 inputAddressLo = inputAddress & 0xFFFFFFFF;
	u64 outputAddress = (Output) ? virt_to_phys(Output) : 0;
	u32 outputAddressHi = outputAddress >> 32;
	u32 outputAddressLo = outputAddress & 0xFFFFFFFF;
	volatile void *hypercallPage = gHvContext.HypercallPage;

	DPRINT_DBG(VMBUS, "Hypercall <control %llx input %p output %p>",
		   Control, Input, Output);

	__asm__ __volatile__ ("call *%8" : "=d"(hvStatusHi),
			      "=a"(hvStatusLo) : "d" (controlHi),
			      "a" (controlLo), "b" (inputAddressHi),
			      "c" (inputAddressLo), "D"(outputAddressHi),
			      "S"(outputAddressLo), "m" (hypercallPage));

	DPRINT_DBG(VMBUS, "Hypercall <return %llx>",
		   hvStatusLo | ((u64)hvStatusHi << 32));

	return hvStatusLo | ((u64)hvStatusHi << 32);
#endif 
}


int HvInit(void)
{
	int ret = 0;
	int maxLeaf;
	union hv_x64_msr_hypercall_contents hypercallMsr;
	void *virtAddr = NULL;

	DPRINT_ENTER(VMBUS);

	memset(gHvContext.synICEventPage, 0, sizeof(void *) * MAX_NUM_CPUS);
	memset(gHvContext.synICMessagePage, 0, sizeof(void *) * MAX_NUM_CPUS);

	if (!HvQueryHypervisorPresence()) {
		DPRINT_ERR(VMBUS, "No Windows hypervisor detected!!");
		goto Cleanup;
	}

	DPRINT_INFO(VMBUS,
		    "Windows hypervisor detected! Retrieving more info...");

	maxLeaf = HvQueryHypervisorInfo();
	

	
	rdmsrl(HV_X64_MSR_GUEST_OS_ID, gHvContext.GuestId);
	if (gHvContext.GuestId == 0) {
		
		wrmsrl(HV_X64_MSR_GUEST_OS_ID, HV_LINUX_GUEST_ID);
		gHvContext.GuestId = HV_LINUX_GUEST_ID;
	}

	
	rdmsrl(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
	if (gHvContext.GuestId == HV_LINUX_GUEST_ID) {
		
		
		virtAddr = osd_VirtualAllocExec(PAGE_SIZE);

		if (!virtAddr) {
			DPRINT_ERR(VMBUS,
				   "unable to allocate hypercall page!!");
			goto Cleanup;
		}

		hypercallMsr.Enable = 1;
		
		hypercallMsr.GuestPhysicalAddress = vmalloc_to_pfn(virtAddr);
		wrmsrl(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);

		
		hypercallMsr.AsUINT64 = 0;
		rdmsrl(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
		if (!hypercallMsr.Enable) {
			DPRINT_ERR(VMBUS, "unable to set hypercall page!!");
			goto Cleanup;
		}

		gHvContext.HypercallPage = virtAddr;
	} else {
		DPRINT_ERR(VMBUS, "Unknown guest id (0x%llx)!!",
				gHvContext.GuestId);
		goto Cleanup;
	}

	DPRINT_INFO(VMBUS, "Hypercall page VA=%p, PA=0x%0llx",
		    gHvContext.HypercallPage,
		    (u64)hypercallMsr.GuestPhysicalAddress << PAGE_SHIFT);

	
	gHvContext.SignalEventBuffer =
			kmalloc(sizeof(struct hv_input_signal_event_buffer),
				GFP_KERNEL);
	if (!gHvContext.SignalEventBuffer)
		goto Cleanup;

	gHvContext.SignalEventParam =
		(struct hv_input_signal_event *)
			(ALIGN_UP((unsigned long)gHvContext.SignalEventBuffer,
				  HV_HYPERCALL_PARAM_ALIGN));
	gHvContext.SignalEventParam->ConnectionId.Asu32 = 0;
	gHvContext.SignalEventParam->ConnectionId.u.Id =
						VMBUS_EVENT_CONNECTION_ID;
	gHvContext.SignalEventParam->FlagNumber = 0;
	gHvContext.SignalEventParam->RsvdZ = 0;

	

	DPRINT_EXIT(VMBUS);

	return ret;

Cleanup:
	if (virtAddr) {
		if (hypercallMsr.Enable) {
			hypercallMsr.AsUINT64 = 0;
			wrmsrl(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
		}

		vfree(virtAddr);
	}
	ret = -1;
	DPRINT_EXIT(VMBUS);

	return ret;
}


void HvCleanup(void)
{
	union hv_x64_msr_hypercall_contents hypercallMsr;

	DPRINT_ENTER(VMBUS);

	if (gHvContext.SignalEventBuffer) {
		gHvContext.SignalEventBuffer = NULL;
		gHvContext.SignalEventParam = NULL;
		kfree(gHvContext.SignalEventBuffer);
	}

	if (gHvContext.GuestId == HV_LINUX_GUEST_ID) {
		if (gHvContext.HypercallPage) {
			hypercallMsr.AsUINT64 = 0;
			wrmsrl(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
			vfree(gHvContext.HypercallPage);
			gHvContext.HypercallPage = NULL;
		}
	}

	DPRINT_EXIT(VMBUS);

}


u16 HvPostMessage(union hv_connection_id connectionId,
		  enum hv_message_type messageType,
		  void *payload, size_t payloadSize)
{
	struct alignedInput {
		u64 alignment8;
		struct hv_input_post_message msg;
	};

	struct hv_input_post_message *alignedMsg;
	u16 status;
	unsigned long addr;

	if (payloadSize > HV_MESSAGE_PAYLOAD_BYTE_COUNT)
		return -1;

	addr = (unsigned long)kmalloc(sizeof(struct alignedInput), GFP_ATOMIC);
	if (!addr)
		return -1;

	alignedMsg = (struct hv_input_post_message *)
			(ALIGN_UP(addr, HV_HYPERCALL_PARAM_ALIGN));

	alignedMsg->ConnectionId = connectionId;
	alignedMsg->MessageType = messageType;
	alignedMsg->PayloadSize = payloadSize;
	memcpy((void *)alignedMsg->Payload, payload, payloadSize);

	status = HvDoHypercall(HvCallPostMessage, alignedMsg, NULL) & 0xFFFF;

	kfree((void *)addr);

	return status;
}



u16 HvSignalEvent(void)
{
	u16 status;

	status = HvDoHypercall(HvCallSignalEvent, gHvContext.SignalEventParam,
			       NULL) & 0xFFFF;
	return status;
}


void HvSynicInit(void *irqarg)
{
	u64 version;
	union hv_synic_simp simp;
	union hv_synic_siefp siefp;
	union hv_synic_sint sharedSint;
	union hv_synic_scontrol sctrl;
	u64 guestID;
	u32 irqVector = *((u32 *)(irqarg));
	int cpu = smp_processor_id();

	DPRINT_ENTER(VMBUS);

	if (!gHvContext.HypercallPage) {
		DPRINT_EXIT(VMBUS);
		return;
	}

	
	rdmsrl(HV_X64_MSR_SVERSION, version);

	DPRINT_INFO(VMBUS, "SynIC version: %llx", version);

	
	if (gHvContext.GuestId == HV_XENLINUX_GUEST_ID) {
		DPRINT_INFO(VMBUS, "Skipping SIMP and SIEFP setup since "
				"it is already set.");

		rdmsrl(HV_X64_MSR_SIMP, simp.AsUINT64);
		rdmsrl(HV_X64_MSR_SIEFP, siefp.AsUINT64);

		DPRINT_DBG(VMBUS, "Simp: %llx, Sifep: %llx",
			   simp.AsUINT64, siefp.AsUINT64);

		
		rdmsrl(HV_X64_MSR_GUEST_OS_ID, guestID);
		if (guestID == HV_LINUX_GUEST_ID) {
			gHvContext.synICMessagePage[cpu] =
				phys_to_virt(simp.BaseSimpGpa << PAGE_SHIFT);
			gHvContext.synICEventPage[cpu] =
				phys_to_virt(siefp.BaseSiefpGpa << PAGE_SHIFT);
		} else {
			DPRINT_ERR(VMBUS, "unknown guest id!!");
			goto Cleanup;
		}
		DPRINT_DBG(VMBUS, "MAPPED: Simp: %p, Sifep: %p",
			   gHvContext.synICMessagePage[cpu],
			   gHvContext.synICEventPage[cpu]);
	} else {
		gHvContext.synICMessagePage[cpu] = (void *)get_zeroed_page(GFP_ATOMIC);
		if (gHvContext.synICMessagePage[cpu] == NULL) {
			DPRINT_ERR(VMBUS,
				   "unable to allocate SYNIC message page!!");
			goto Cleanup;
		}

		gHvContext.synICEventPage[cpu] = (void *)get_zeroed_page(GFP_ATOMIC);
		if (gHvContext.synICEventPage[cpu] == NULL) {
			DPRINT_ERR(VMBUS,
				   "unable to allocate SYNIC event page!!");
			goto Cleanup;
		}

		
		rdmsrl(HV_X64_MSR_SIMP, simp.AsUINT64);
		simp.SimpEnabled = 1;
		simp.BaseSimpGpa = virt_to_phys(gHvContext.synICMessagePage[cpu])
					>> PAGE_SHIFT;

		DPRINT_DBG(VMBUS, "HV_X64_MSR_SIMP msr set to: %llx",
				simp.AsUINT64);

		wrmsrl(HV_X64_MSR_SIMP, simp.AsUINT64);

		
		rdmsrl(HV_X64_MSR_SIEFP, siefp.AsUINT64);
		siefp.SiefpEnabled = 1;
		siefp.BaseSiefpGpa = virt_to_phys(gHvContext.synICEventPage[cpu])
					>> PAGE_SHIFT;

		DPRINT_DBG(VMBUS, "HV_X64_MSR_SIEFP msr set to: %llx",
				siefp.AsUINT64);

		wrmsrl(HV_X64_MSR_SIEFP, siefp.AsUINT64);
	}

	
	
	

	
	rdmsrl(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	sharedSint.AsUINT64 = 0;
	sharedSint.Vector = irqVector; 
	sharedSint.Masked = false;
	sharedSint.AutoEoi = true;

	DPRINT_DBG(VMBUS, "HV_X64_MSR_SINT1 msr set to: %llx",
		   sharedSint.AsUINT64);

	wrmsrl(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	
	rdmsrl(HV_X64_MSR_SCONTROL, sctrl.AsUINT64);
	sctrl.Enable = 1;

	wrmsrl(HV_X64_MSR_SCONTROL, sctrl.AsUINT64);

	gHvContext.SynICInitialized = true;

	DPRINT_EXIT(VMBUS);

	return;

Cleanup:
	if (gHvContext.GuestId == HV_LINUX_GUEST_ID) {
		if (gHvContext.synICEventPage[cpu])
			osd_PageFree(gHvContext.synICEventPage[cpu], 1);

		if (gHvContext.synICMessagePage[cpu])
			osd_PageFree(gHvContext.synICMessagePage[cpu], 1);
	}

	DPRINT_EXIT(VMBUS);
	return;
}


void HvSynicCleanup(void *arg)
{
	union hv_synic_sint sharedSint;
	union hv_synic_simp simp;
	union hv_synic_siefp siefp;
	int cpu = smp_processor_id();

	DPRINT_ENTER(VMBUS);

	if (!gHvContext.SynICInitialized) {
		DPRINT_EXIT(VMBUS);
		return;
	}

	rdmsrl(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	sharedSint.Masked = 1;

	
	
	wrmsrl(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	
	if (gHvContext.GuestId == HV_LINUX_GUEST_ID) {
		rdmsrl(HV_X64_MSR_SIMP, simp.AsUINT64);
		simp.SimpEnabled = 0;
		simp.BaseSimpGpa = 0;

		wrmsrl(HV_X64_MSR_SIMP, simp.AsUINT64);

		rdmsrl(HV_X64_MSR_SIEFP, siefp.AsUINT64);
		siefp.SiefpEnabled = 0;
		siefp.BaseSiefpGpa = 0;

		wrmsrl(HV_X64_MSR_SIEFP, siefp.AsUINT64);

		osd_PageFree(gHvContext.synICMessagePage[cpu], 1);
		osd_PageFree(gHvContext.synICEventPage[cpu], 1);
	}

	DPRINT_EXIT(VMBUS);
}
