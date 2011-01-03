#include <linux/init.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <asm/processor-cyrix.h>
#include <asm/processor-flags.h>
#include <asm/mtrr.h>
#include <asm/msr.h>

#include "mtrr.h"


void set_mtrr_prepare_save(struct set_mtrr_context *ctxt)
{
	unsigned int cr0;

	
	local_irq_save(ctxt->flags);

	if (use_intel() || is_cpu(CYRIX)) {

		
		if (cpu_has_pge) {
			ctxt->cr4val = read_cr4();
			write_cr4(ctxt->cr4val & ~X86_CR4_PGE);
		}

		
		cr0 = read_cr0() | X86_CR0_CD;
		wbinvd();
		write_cr0(cr0);
		wbinvd();

		if (use_intel()) {
			
			rdmsr(MSR_MTRRdefType, ctxt->deftype_lo, ctxt->deftype_hi);
		} else {
			
			ctxt->ccr3 = getCx86(CX86_CCR3);
		}
	}
}

void set_mtrr_cache_disable(struct set_mtrr_context *ctxt)
{
	if (use_intel()) {
		
		mtrr_wrmsr(MSR_MTRRdefType, ctxt->deftype_lo & 0xf300UL,
		      ctxt->deftype_hi);
	} else {
		if (is_cpu(CYRIX)) {
			
			setCx86(CX86_CCR3, (ctxt->ccr3 & 0x0f) | 0x10);
		}
	}
}


void set_mtrr_done(struct set_mtrr_context *ctxt)
{
	if (use_intel() || is_cpu(CYRIX)) {

		
		wbinvd();

		
		if (use_intel()) {
			
			mtrr_wrmsr(MSR_MTRRdefType, ctxt->deftype_lo,
				   ctxt->deftype_hi);
		} else {
			
			setCx86(CX86_CCR3, ctxt->ccr3);
		}

		
		write_cr0(read_cr0() & 0xbfffffff);

		
		if (cpu_has_pge)
			write_cr4(ctxt->cr4val);
	}
	
	local_irq_restore(ctxt->flags);
}
